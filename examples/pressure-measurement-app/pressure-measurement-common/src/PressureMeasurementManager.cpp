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

#include "PressureMeasurementManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

PressureMeasurementManager PressureMeasurementManager::sInstance;

PressureMeasurementManager * PressureMeasurementManager::GetInstance()
{
    return &sInstance;
}

void PressureMeasurementManager::InitInstance(EndpointId endpoint)
{
    sInstance.mEndpoint = endpoint;

    // Initialize pressure measurement attributes with reasonable defaults
    // According to Matter spec, these should be null by default, but for a working sensor
    // we initialize them to reasonable values representing atmospheric pressure range

    // Set default atmospheric pressure (101.3 kPa = 1013 in 10x kPa units)
    Status status = PressureMeasurement::Attributes::MeasuredValue::Set(endpoint, 1013);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set PressureMeasurement MeasuredValue attribute"));

    // Set minimum pressure (0.1 kPa = 1 in 10x kPa units - near vacuum)
    status = PressureMeasurement::Attributes::MinMeasuredValue::Set(endpoint, 1);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set PressureMeasurement MinMeasuredValue attribute"));

    // Set maximum pressure (327.6 kPa = 3276 in 10x kPa units - high pressure limit)
    status = PressureMeasurement::Attributes::MaxMeasuredValue::Set(endpoint, 3276);
    VerifyOrReturn(Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set PressureMeasurement MaxMeasuredValue attribute"));

    ChipLogProgress(NotSpecified, "Initialized PressureMeasurement attributes - Default: 101.3 kPa, Range: 0.1-327.6 kPa");
}
