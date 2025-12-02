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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>

class TemperatureManager
{
public:
    static TemperatureManager & GetInstance() { return sTempMgr; }

    CHIP_ERROR Init();
    void AttributeChangeHandler(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value, uint16_t size);

    uint8_t GetMode();
    void SetMode(uint8_t mode);
    int8_t GetCurrentTemp();
    void SetCurrentTemp(int8_t temperature);
    int8_t GetHeatingSetPoint();
    int8_t GetCoolingSetPoint();
    uint16_t GetRelayState();

    // UI synchronization helpers
    bool HasExternalChanges();
    void ClearExternalChanges();

private:
    static TemperatureManager sTempMgr;

    int8_t ConvertToPrintableTemp(int16_t temperature);

    uint8_t mThermMode             = 0; // SystemMode: 0=Off, 1=Auto, 3=Cool, 4=Heat, etc.
    int8_t mCurrentTempCelsius     = 21;
    int8_t mHeatingCelsiusSetPoint = 20;
    int8_t mCoolingCelsiusSetPoint = 26;
    uint16_t mRelayState           = 0; // ThermostatRunningState bitmap

    // Flag to track external changes that require UI sync
    bool mHasExternalChanges = false;
};
