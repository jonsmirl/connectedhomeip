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

#include "RockerModeSelect.h"
#include "RockerManager.h"

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
using namespace chip::app::Clusters::RockerModeSelect;
using namespace chip::DeviceLayer;

template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static RockerModeSelectDelegate * gRockerModeSelectDelegate = nullptr;
static ModeBase::Instance * gRockerModeSelectInstance       = nullptr;

ModeBase::Instance * RockerModeSelect::Instance()
{
    return gRockerModeSelectInstance;
}

void RockerModeSelect::Shutdown()
{
    if (gRockerModeSelectInstance != nullptr)
    {
        delete gRockerModeSelectInstance;
        gRockerModeSelectInstance = nullptr;
    }
    if (gRockerModeSelectDelegate != nullptr)
    {
        delete gRockerModeSelectDelegate;
        gRockerModeSelectDelegate = nullptr;
    }
}

// ModeSelect cluster initialization callback
void emberAfModeSelectClusterInitCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "RockerModeSelect: Initializing ModeSelect cluster on endpoint %d", endpointId);

    // Only initialize for EP2 (kSwitchEndpointId)
    if (endpointId != RockerManager::kSwitchEndpointId)
    {
        ChipLogProgress(AppServer, "RockerModeSelect: Skipping initialization for endpoint %d (not switch endpoint)", endpointId);
        return;
    }

    VerifyOrDie(gRockerModeSelectDelegate == nullptr && gRockerModeSelectInstance == nullptr);

    gRockerModeSelectDelegate = new RockerModeSelectDelegate;
    gRockerModeSelectInstance = new ModeBase::Instance(gRockerModeSelectDelegate, endpointId, ModeSelect::Id, 0);
    gRockerModeSelectInstance->Init();

    ChipLogProgress(AppServer, "RockerModeSelect: Successfully initialized ModeSelect cluster on endpoint %d", endpointId);

    // Set initial mode to "None" (mode 0)
    ModeSelect::Attributes::CurrentMode::Set(endpointId, ModeNone);

    // Initialize RockerManager mode state
    RockerManager::GetInstance().SetCurrentMode(ModeNone);

    ChipLogProgress(AppServer, "RockerModeSelect: Set initial mode to %d (None)", ModeNone);
}

// ModeSelect cluster shutdown callback
void emberAfModeSelectClusterShutdownCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "RockerModeSelect: Shutting down ModeSelect cluster on endpoint %d", endpointId);

    // Only handle shutdown for EP2 (kSwitchEndpointId)
    if (endpointId != RockerManager::kSwitchEndpointId)
    {
        return;
    }

    RockerModeSelect::Shutdown();
}

// ModeSelect ChangeToMode command callback
bool emberAfModeSelectClusterChangeToModeCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                  const ModeSelect::Commands::ChangeToMode::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "RockerModeSelect: ChangeToMode command received on endpoint %d, newMode=%d",
                    commandPath.mEndpointId, commandData.newMode);

    // Only handle commands for EP2 (kSwitchEndpointId)
    if (commandPath.mEndpointId != RockerManager::kSwitchEndpointId)
    {
        ChipLogError(AppServer, "RockerModeSelect: ChangeToMode command received on wrong endpoint %d", commandPath.mEndpointId);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::UnsupportedEndpoint);
        return true;
    }

    // Validate the new mode
    if (commandData.newMode > ModeButton)
    {
        ChipLogError(AppServer, "RockerModeSelect: Invalid mode %d requested", commandData.newMode);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }

    // Get current mode
    uint8_t currentMode = 0;
    ModeSelect::Attributes::CurrentMode::Get(commandPath.mEndpointId, &currentMode);

    if (currentMode == commandData.newMode)
    {
        ChipLogProgress(AppServer, "RockerModeSelect: Already in mode %d, no change needed", commandData.newMode);
        commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        return true;
    }

    // Update the current mode attribute
    ModeSelect::Attributes::CurrentMode::Set(commandPath.mEndpointId, commandData.newMode);

    // Update RockerManager state
    RockerManager::GetInstance().HandleModeChange(commandData.newMode);

    ChipLogProgress(AppServer, "RockerModeSelect: Successfully changed mode from %d to %d", currentMode, commandData.newMode);

    commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

// RockerModeSelectDelegate implementation
CHIP_ERROR RockerModeSelectDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RockerModeSelectDelegate::HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(AppServer, "RockerModeSelectDelegate: HandleChangeToMode to mode %d", mode);

    // Validate the new mode
    if (mode > ModeButton)
    {
        ChipLogError(AppServer, "RockerModeSelectDelegate: Invalid mode %d requested", mode);
        response.status = to_underlying(ModeBase::StatusCode::kUnsupportedMode);
        return;
    }

    // Update RockerManager state
    RockerManager::GetInstance().HandleModeChange(mode);

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
    ChipLogProgress(AppServer, "RockerModeSelectDelegate: Successfully changed to mode %d", mode);
}

CHIP_ERROR RockerModeSelectDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RockerModeSelectDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RockerModeSelectDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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
