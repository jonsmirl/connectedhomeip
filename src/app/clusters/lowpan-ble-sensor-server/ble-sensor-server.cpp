/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ble-sensor-server.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/logging/CHIPLogging.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using Protocols::InteractionModel::Status;

bool emberAfLowpanBleSensorClusterAddSensorEpCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                      const LowpanBleSensor::Commands::AddSensorEp::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddSensorEp", "LowpanBleSensor");

    ChipLogProgress(Zcl, "LowpanBleSensor: AddSensorEp command received on endpoint %u (stub implementation)",
                    commandPath.mEndpointId);

    // TODO: Implement BLE sensor endpoint addition logic
    // This would typically:
    // 1. Validate the sensor parameters
    // 2. Create a dynamic endpoint for the sensor
    // 3. Configure the sensor attributes

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfLowpanBleSensorClusterRemoveSensorEpCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                         const LowpanBleSensor::Commands::RemoveSensorEp::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RemoveSensorEp", "LowpanBleSensor");

    ChipLogProgress(Zcl, "LowpanBleSensor: RemoveSensorEp command received on endpoint %u (stub implementation)",
                    commandPath.mEndpointId);

    // TODO: Implement BLE sensor endpoint removal logic
    // This would typically:
    // 1. Validate the sensor endpoint ID
    // 2. Remove the dynamic endpoint
    // 3. Clean up associated resources

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

void MatterLowpanBleSensorPluginServerInitCallback() {}

void MatterLowpanBleSensorPluginServerShutdownCallback() {}
