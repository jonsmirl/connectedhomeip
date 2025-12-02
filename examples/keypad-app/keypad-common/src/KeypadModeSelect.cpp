/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "KeypadModeSelect.h"
#include "KeypadManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadModeSelect;
using namespace chip::DeviceLayer;

template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static KeypadModeSelectDelegate * gKeypadModeSelectDelegate = nullptr;
static ModeBase::Instance * gKeypadModeSelectInstance       = nullptr;

ModeBase::Instance * KeypadModeSelect::Instance()
{
    return gKeypadModeSelectInstance;
}

void KeypadModeSelect::Shutdown()
{
    if (gKeypadModeSelectInstance != nullptr)
    {
        delete gKeypadModeSelectInstance;
        gKeypadModeSelectInstance = nullptr;
    }
    if (gKeypadModeSelectDelegate != nullptr)
    {
        delete gKeypadModeSelectDelegate;
        gKeypadModeSelectDelegate = nullptr;
    }
}

// ModeSelect cluster initialization callback
void emberAfModeSelectClusterInitCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "KeypadModeSelect: Initializing ModeSelect cluster on endpoint %d", endpointId);

    // Only initialize for EP2 (kSwitchEndpointId)
    if (endpointId != KeypadManager::kSwitchEndpointId)
    {
        ChipLogProgress(AppServer, "KeypadModeSelect: Skipping initialization for endpoint %d (not switch endpoint)", endpointId);
        return;
    }

    VerifyOrDie(gKeypadModeSelectDelegate == nullptr && gKeypadModeSelectInstance == nullptr);

    gKeypadModeSelectDelegate = new KeypadModeSelectDelegate;
    gKeypadModeSelectInstance = new ModeBase::Instance(gKeypadModeSelectDelegate, endpointId, ModeSelect::Id, 0);
    gKeypadModeSelectInstance->Init();

    ChipLogProgress(AppServer, "KeypadModeSelect: Successfully initialized ModeSelect cluster on endpoint %d", endpointId);

    // Set initial mode to "None" (mode 0)
    ModeSelect::Attributes::CurrentMode::Set(endpointId, ModeNone);

    // Initialize KeypadManager mode state
    KeypadManager::GetInstance().SetCurrentMode(ModeNone);

    ChipLogProgress(AppServer, "KeypadModeSelect: Set initial mode to %d (None)", ModeNone);
}

// ModeSelect cluster shutdown callback
void emberAfModeSelectClusterShutdownCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "KeypadModeSelect: Shutting down ModeSelect cluster on endpoint %d", endpointId);

    // Only handle shutdown for EP2 (kSwitchEndpointId)
    if (endpointId != KeypadManager::kSwitchEndpointId)
    {
        return;
    }

    KeypadModeSelect::Shutdown();
}

// ModeSelect ChangeToMode command callback
bool emberAfModeSelectClusterChangeToModeCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                  const ModeSelect::Commands::ChangeToMode::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "KeypadModeSelect: ChangeToMode command received on endpoint %d, newMode=%d",
                    commandPath.mEndpointId, commandData.newMode);

    // Only handle commands for EP2 (kSwitchEndpointId)
    if (commandPath.mEndpointId != KeypadManager::kSwitchEndpointId)
    {
        ChipLogError(AppServer, "KeypadModeSelect: ChangeToMode command received on wrong endpoint %d", commandPath.mEndpointId);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::UnsupportedEndpoint);
        return true;
    }

    // Validate the new mode
    if (commandData.newMode > ModeButton)
    {
        ChipLogError(AppServer, "KeypadModeSelect: Invalid mode %d requested", commandData.newMode);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }

    // Get current mode
    uint8_t currentMode = 0;
    ModeSelect::Attributes::CurrentMode::Get(commandPath.mEndpointId, &currentMode);

    if (currentMode == commandData.newMode)
    {
        ChipLogProgress(AppServer, "KeypadModeSelect: Already in mode %d, no change needed", commandData.newMode);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        return true;
    }

    // Update the current mode attribute
    ModeSelect::Attributes::CurrentMode::Set(commandPath.mEndpointId, commandData.newMode);

    // Update KeypadManager state
    KeypadManager::GetInstance().HandleModeChange(commandData.newMode);

    ChipLogProgress(AppServer, "KeypadModeSelect: Successfully changed mode from %d to %d", currentMode, commandData.newMode);

    commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

// KeypadModeSelectDelegate implementation
CHIP_ERROR KeypadModeSelectDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void KeypadModeSelectDelegate::HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(AppServer, "KeypadModeSelectDelegate: HandleChangeToMode to mode %d", mode);

    // Validate the new mode
    if (mode > ModeButton)
    {
        ChipLogError(AppServer, "KeypadModeSelectDelegate: Invalid mode %d requested", mode);
        response.status = to_underlying(ModeBase::StatusCode::kUnsupportedMode);
        return;
    }

    // Update KeypadManager state
    KeypadManager::GetInstance().HandleModeChange(mode);

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
    ChipLogProgress(AppServer, "KeypadModeSelectDelegate: Successfully changed to mode %d", mode);
}

CHIP_ERROR KeypadModeSelectDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR KeypadModeSelectDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadModeSelectDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}
