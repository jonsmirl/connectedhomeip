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

#include "KeypadManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/color-control-server/color-control-server.h>
#include <app/clusters/level-control/level-control.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/data-model/Nullable.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

/**
 * @brief Matter attribute change callback
 *
 * This callback is triggered whenever a cluster attribute changes on any endpoint.
 * We use it to detect changes and trigger synchronization between EP1 and EP2.
 */
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ChipLogProgress(AppServer, "ZclCallbacks: Attribute changed - Endpoint:%d Cluster:0x%04X Attribute:0x%04X",
                    attributePath.mEndpointId, attributePath.mClusterId, attributePath.mAttributeId);

    // Handle OnOff cluster attribute changes
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        bool isOn = *reinterpret_cast<bool *>(value);
        ChipLogProgress(AppServer, "ZclCallbacks: OnOff attribute changed to %s on endpoint %d", isOn ? "ON" : "OFF",
                        attributePath.mEndpointId);

        // Update actual light state and synchronize other endpoint
        KeypadManager::GetInstance().SetLightOnOff(isOn);
    }
    // Handle Level Control cluster attribute changes
    else if (attributePath.mClusterId == LevelControl::Id &&
             attributePath.mAttributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        uint8_t level = *reinterpret_cast<uint8_t *>(value);
        ChipLogProgress(AppServer, "ZclCallbacks: CurrentLevel attribute changed to %d on endpoint %d", level,
                        attributePath.mEndpointId);

        // Update actual light state and synchronize other endpoint
        KeypadManager::GetInstance().SetLightLevel(level);
    }
    // Handle Color Control cluster attribute changes
    else if (attributePath.mClusterId == ColorControl::Id)
    {
        if (attributePath.mAttributeId == ColorControl::Attributes::CurrentX::Id)
        {
            uint16_t colorX = *reinterpret_cast<uint16_t *>(value);
            ChipLogProgress(AppServer, "ZclCallbacks: CurrentX attribute changed to 0x%04X on endpoint %d", colorX,
                            attributePath.mEndpointId);

            // Get current Y coordinate and update both
            uint16_t colorY = 0;
            ColorControl::Attributes::CurrentY::Get(attributePath.mEndpointId, &colorY);
            KeypadManager::GetInstance().UpdateActualLightColorXY(colorX, colorY);
        }
        else if (attributePath.mAttributeId == ColorControl::Attributes::CurrentY::Id)
        {
            uint16_t colorY = *reinterpret_cast<uint16_t *>(value);
            ChipLogProgress(AppServer, "ZclCallbacks: CurrentY attribute changed to 0x%04X on endpoint %d", colorY,
                            attributePath.mEndpointId);

            // Get current X coordinate and update both
            uint16_t colorX = 0;
            ColorControl::Attributes::CurrentX::Get(attributePath.mEndpointId, &colorX);
            KeypadManager::GetInstance().UpdateActualLightColorXY(colorX, colorY);
        }
    }
    // Handle ModeSelect cluster attribute changes
    else if (attributePath.mClusterId == ModeSelect::Id && attributePath.mAttributeId == ModeSelect::Attributes::CurrentMode::Id)
    {
        uint8_t mode = *reinterpret_cast<uint8_t *>(value);
        ChipLogProgress(AppServer, "ZclCallbacks: ModeSelect CurrentMode attribute changed to %d on endpoint %d", mode,
                        attributePath.mEndpointId);

        // Update KeypadManager mode state
        KeypadManager::GetInstance().HandleModeChange(mode);
    }
}

/**
 * @brief OnOff cluster command callbacks
 */
bool emberAfOnOffClusterOnCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                   const OnOff::Commands::On::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: OnOff On command received on endpoint %d", commandPath.mEndpointId);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().SetLightOnOff(true);

    return true;
}

bool emberAfOnOffClusterOffCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                    const OnOff::Commands::Off::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: OnOff Off command received on endpoint %d", commandPath.mEndpointId);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().SetLightOnOff(false);

    return true;
}

bool emberAfOnOffClusterToggleCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                       const OnOff::Commands::Toggle::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: OnOff Toggle command received on endpoint %d", commandPath.mEndpointId);

    // Get current state and toggle it
    bool currentState = false;
    OnOff::Attributes::OnOff::Get(commandPath.mEndpointId, &currentState);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().SetLightOnOff(!currentState);

    return true;
}

/**
 * @brief Level Control cluster command callbacks
 */
bool emberAfLevelControlClusterMoveToLevelCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                   const LevelControl::Commands::MoveToLevel::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: LevelControl MoveToLevel command received on endpoint %d, level=%d",
                    commandPath.mEndpointId, commandData.level);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().SetLightLevel(commandData.level);

    return true;
}

/**
 * @brief Color Control cluster command callbacks
 */
bool emberAfColorControlClusterMoveToColorCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                   const ColorControl::Commands::MoveToColor::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: ColorControl MoveToColor command received on endpoint %d, X=0x%04X Y=0x%04X",
                    commandPath.mEndpointId, commandData.colorX, commandData.colorY);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().UpdateActualLightColorXY(commandData.colorX, commandData.colorY);

    return true;
}

bool emberAfColorControlClusterMoveToHueAndSaturationCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const ColorControl::Commands::MoveToHueAndSaturation::DecodableType & commandData)
{
    ChipLogProgress(AppServer, "ZclCallbacks: ColorControl MoveToHueAndSaturation command received on endpoint %d, H=%d S=%d",
                    commandPath.mEndpointId, commandData.hue, commandData.saturation);

    // Update actual light state and synchronize both endpoints
    KeypadManager::GetInstance().SetLightColor(commandData.hue, commandData.saturation);

    return true;
}
