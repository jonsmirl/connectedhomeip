/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 * @file CustomClusterCallbacks.cpp
 * @brief Stub implementations for Lowpan custom cluster callbacks in the rocker-app.
 */

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

#include <clusters/LowpanHardwareMode/Commands.h>
#include <clusters/LowpanProxyMode/Commands.h>

using namespace chip;
using namespace chip::app;

// ============================================================================
// Lowpan BLE Sensor extern stubs
// ============================================================================

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanBleSensor {

CHIP_ERROR AddSensorEp(const char * macAddress)
{
    ChipLogProgress(Zcl, "LowpanBleSensor::AddSensorEp stub called for MAC: %s", macAddress);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RemoveSensorEp(EndpointId endpointId)
{
    ChipLogProgress(Zcl, "LowpanBleSensor::RemoveSensorEp stub called for endpoint: %u", endpointId);
    return CHIP_NO_ERROR;
}

} // namespace LowpanBleSensor
} // namespace Clusters
} // namespace app
} // namespace chip

// ============================================================================
// Lowpan Hardware Mode Cluster Callbacks
// ============================================================================

bool emberAfLowpanHardwareModeClusterChangeToModeCallback(
    CommandHandler * commandHandler,
    const ConcreteCommandPath & commandPath,
    const Clusters::LowpanHardwareMode::Commands::ChangeToMode::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "LowpanHardwareMode: ChangeToMode stub, mode=%d, endpoint=%d",
                    commandData.newMode, commandPath.mEndpointId);
    commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

void MatterLowpanHardwareModePluginServerInitCallback() {}

// ============================================================================
// Lowpan Proxy Mode Cluster Callbacks
// ============================================================================

bool emberAfLowpanProxyModeClusterChangeToModeCallback(
    CommandHandler * commandHandler,
    const ConcreteCommandPath & commandPath,
    const Clusters::LowpanProxyMode::Commands::ChangeToMode::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "LowpanProxyMode: ChangeToMode stub, mode=%d, endpoint=%d",
                    commandData.newMode, commandPath.mEndpointId);
    commandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

void MatterLowpanProxyModePluginServerInitCallback() {}
