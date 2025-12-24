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

// External implementation in LowpanBleSensor.cpp
namespace chip {
namespace app {
namespace Clusters {
namespace LowpanBleSensor {
extern CHIP_ERROR AddSensorEp(const char * macAddress);
extern CHIP_ERROR RemoveSensorEp(EndpointId endpointId);
} // namespace LowpanBleSensor
} // namespace Clusters
} // namespace app
} // namespace chip

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using Protocols::InteractionModel::Status;

bool emberAfLowpanBleSensorClusterAddSensorEpCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                      const LowpanBleSensor::Commands::AddSensorEp::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddSensorEp", "LowpanBleSensor");

    ChipLogProgress(Zcl, "LowpanBleSensor: AddSensorEp command received on endpoint %u", commandPath.mEndpointId);

    // Extract MAC address
    CharSpan macSpan = commandData.macAddress;
    char mac[18] = {0};
    size_t len = (macSpan.size() < sizeof(mac) - 1) ? macSpan.size() : sizeof(mac) - 1;
    memcpy(mac, macSpan.data(), len);
    mac[len] = '\0';

    CHIP_ERROR err = LowpanBleSensor::AddSensorEp(mac);
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }
    return true;
}

bool emberAfLowpanBleSensorClusterRemoveSensorEpCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                         const LowpanBleSensor::Commands::RemoveSensorEp::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RemoveSensorEp", "LowpanBleSensor");

    ChipLogProgress(Zcl, "LowpanBleSensor: RemoveSensorEp command received on endpoint %u", commandPath.mEndpointId);

    EndpointId epId = static_cast<EndpointId>(commandData.endpoint);
    CHIP_ERROR err = LowpanBleSensor::RemoveSensorEp(epId);
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else if (err == CHIP_ERROR_NOT_FOUND)
    {
        commandObj->AddStatus(commandPath, Status::NotFound);
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }
    return true;
}

void MatterLowpanBleSensorPluginServerInitCallback() {}

void MatterLowpanBleSensorPluginServerShutdownCallback() {}
