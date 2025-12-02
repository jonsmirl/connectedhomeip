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

#include "HumidityMeasurementManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

HumidityMeasurementManager HumidityMeasurementManager::sInstance;

HumidityMeasurementManager * HumidityMeasurementManager::GetInstance()
{
    return &sInstance;
}

void HumidityMeasurementManager::InitInstance(EndpointId endpoint)
{
    sInstance.mEndpoint = endpoint;

    // Initialize with a default humidity of 45% (4500 in 0.01% units)
    uint16_t defaultHumidity = 4500; // 45.00%
    Status status = chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(endpoint, defaultHumidity);
    if (Status::Success == status)
    {
        ChipLogProgress(NotSpecified, "Initialized RelativeHumidityMeasurement MeasuredValue to default: 45.00%%");
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to initialize RelativeHumidityMeasurement MeasuredValue attribute");
    }

    // Set min and max measurement range (0-100%)
    chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(endpoint, 0);     // 0%
    chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(endpoint, 10000); // 100%
}

void HumidityMeasurementManager::OnHumidityChanged(uint16_t humidity)
{
    Status status = chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(mEndpoint, humidity);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MeasuredValue attribute"));
    ChipLogDetail(NotSpecified, "Temperature updated to: %d", humidity);
}
