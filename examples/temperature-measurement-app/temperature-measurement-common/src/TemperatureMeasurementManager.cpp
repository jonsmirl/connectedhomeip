/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "TemperatureMeasurementManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

TemperatureMeasurementManager TemperatureMeasurementManager::sInstance;

TemperatureMeasurementManager * TemperatureMeasurementManager::GetInstance()
{
    return &sInstance;
}

void TemperatureMeasurementManager::InitInstance(EndpointId endpoint)
{
    sInstance.mEndpoint = endpoint;

    // Initialize with a default temperature of 22°C (2200 in 0.01°C units)
    int16_t defaultTemperature = 2200; // 22.00°C (room temperature)
    Status status = chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(endpoint, defaultTemperature);
    if (Status::Success == status)
    {
        ChipLogProgress(NotSpecified, "Initialized TemperatureMeasurement MeasuredValue to default: 22.00°C");
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to initialize TemperatureMeasurement MeasuredValue attribute");
    }

    // Set min and max measurement range (-40°C to 85°C, typical sensor range)
    chip::app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(endpoint, -4000); // -40°C
    chip::app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(endpoint, 8500);  // 85°C
}

void TemperatureMeasurementManager::OnTemperatureChanged(int16_t temperature)
{
    Status status = chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(mEndpoint, temperature);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "Temperature updated to: %d", temperature);
}
