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

#include "include/TemperatureManager.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr EndpointId kThermostatEndpoint = 1;

using namespace ::chip::app::Clusters::Thermostat;
namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;

TemperatureManager TemperatureManager::sTempMgr;

CHIP_ERROR TemperatureManager::Init()
{
    // Initialize with default values
    mCurrentTempCelsius     = 21;
    mHeatingCelsiusSetPoint = 20;
    mCoolingCelsiusSetPoint = 26;
    mThermMode              = 0; // Default to Off

    ChipLogProgress(Zcl, "TemperatureManager initialized: Mode=%d, Temp=%d, Heat=%d, Cool=%d", mThermMode, mCurrentTempCelsius,
                    mHeatingCelsiusSetPoint, mCoolingCelsiusSetPoint);

    return CHIP_NO_ERROR;
}

int8_t TemperatureManager::ConvertToPrintableTemp(int16_t temperature)
{
    constexpr uint8_t kRoundUpValue = 50;

    // Round up the temperature as we won't print decimals
    // Is it a negative temperature
    if (temperature < 0)
    {
        temperature -= kRoundUpValue;
    }
    else
    {
        temperature += kRoundUpValue;
    }

    return static_cast<int8_t>(temperature / 100);
}

void TemperatureManager::AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
    case ThermAttr::LocalTemperature::Id: {
        int8_t Temp = ConvertToPrintableTemp(*((int16_t *) value));
        ChipLogProgress(Zcl, "Local temp %d", Temp);
        mCurrentTempCelsius = Temp;
        mHasExternalChanges = true; // Mark that external change occurred
    }
    break;

    case ThermAttr::OccupiedCoolingSetpoint::Id: {
        int8_t coolingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        ChipLogProgress(Zcl, "CoolingSetpoint %d", coolingTemp);
        mCoolingCelsiusSetPoint = coolingTemp;
        mHasExternalChanges     = true; // Mark that external change occurred
    }
    break;

    case ThermAttr::OccupiedHeatingSetpoint::Id: {
        int8_t heatingTemp = ConvertToPrintableTemp(*((int16_t *) value));
        ChipLogProgress(Zcl, "HeatingSetpoint %d", heatingTemp);
        mHeatingCelsiusSetPoint = heatingTemp;
        mHasExternalChanges     = true; // Mark that external change occurred
    }
    break;

    case ThermAttr::SystemMode::Id: {
        ChipLogProgress(Zcl, "SystemMode changed to %d", static_cast<uint8_t>(*value));
        uint8_t mode = static_cast<uint8_t>(*value);
        if (mThermMode != mode)
        {
            mThermMode          = mode;
            mHasExternalChanges = true; // Mark that external change occurred
            ChipLogProgress(Zcl, "SystemMode updated to %d", mThermMode);
        }
    }
    break;

    default: {
        ChipLogProgress(Zcl, "Unhandled thermostat attribute %x", attributeId);
        return;
    }
    break;
    }
}

uint8_t TemperatureManager::GetMode()
{
    return mThermMode;
}

void TemperatureManager::SetMode(uint8_t mode)
{
    if (mThermMode != mode)
    {
        mThermMode = mode;
        ChipLogProgress(Zcl, "TemperatureManager: Mode set to %d", mThermMode);
    }
}

int8_t TemperatureManager::GetCurrentTemp()
{
    return mCurrentTempCelsius;
}

void TemperatureManager::SetCurrentTemp(int8_t temperature)
{
    ChipLogProgress(Zcl, "TemperatureManager: SetCurrentTemp called with %d", temperature);
    mCurrentTempCelsius = temperature;
}

int8_t TemperatureManager::GetHeatingSetPoint()
{
    return mHeatingCelsiusSetPoint;
}

int8_t TemperatureManager::GetCoolingSetPoint()
{
    return mCoolingCelsiusSetPoint;
}

uint16_t TemperatureManager::GetRelayState()
{
    // Calculate relay state based on current system mode and temperature
    uint16_t relayState = 0;

    switch (mThermMode)
    {
    case 0: // Off
        relayState = 0;
        break;
    case 1: // Auto
        if (mCurrentTempCelsius < mHeatingCelsiusSetPoint)
        {
            relayState |= 0x01; // Heat
            relayState |= 0x04; // Fan Stage 1
        }
        else if (mCurrentTempCelsius > mCoolingCelsiusSetPoint)
        {
            relayState |= 0x02; // Cool
            relayState |= 0x04; // Fan Stage 1
        }
        break;
    case 3: // Cool
        if (mCurrentTempCelsius > mCoolingCelsiusSetPoint)
        {
            relayState |= 0x02; // Cool
            relayState |= 0x04; // Fan Stage 1
        }
        break;
    case 4: // Heat
        if (mCurrentTempCelsius < mHeatingCelsiusSetPoint)
        {
            relayState |= 0x01; // Heat
            relayState |= 0x04; // Fan Stage 1
        }
        break;
    case 7:                 // Fan Only
        relayState |= 0x04; // Fan Stage 1
        break;
    }

    mRelayState = relayState;
    return mRelayState;
}

bool TemperatureManager::HasExternalChanges()
{
    return mHasExternalChanges;
}

void TemperatureManager::ClearExternalChanges()
{
    mHasExternalChanges = false;
}
