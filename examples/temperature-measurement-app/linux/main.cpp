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

#include "TemperatureMeasurementAppAttrUpdateDelegate.h"
#include "TemperatureMeasurementAppBasicInformation.h"
#include "TemperatureMeasurementManager.h"

#include <AppMain.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <platform/PlatformManager.h>

#include <app/reporting/reporting.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cmath>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <app-common/zap-generated/attributes/Accessors.h>

#include <imgui.h>
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/qrcode.h>
#include <imgui_ui/windows/window.h>

// Custom BasicInformation window for temperature sensor
class TemperatureSensorBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    TemperatureSensorBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Temperature Sensor"; }
    uint16_t GetDeviceProductID() const override { return 0x800A; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Temperature Sensor"; }
    std::string GetDevicePartNumber() const override { return "TEMP-001"; }
    std::string GetDeviceSerialNumber() const override { return "TEMP-123456"; }
    std::string GetDeviceUniqueID() const override { return "TEMP-UID-123456"; }
};

// Custom Temperature Measurement control window
class TemperatureMeasurementControlWindow : public example::Ui::Window
{
public:
    TemperatureMeasurementControlWindow(chip::EndpointId endpointId) : mEndpointId(endpointId)
    {
        // Initialize cached values
        UpdateCachedValues();
    }

    const char * GetDisplayName() const override { return "Temperature Measurement"; }

    void Render() override
    {
        // Update cached values periodically (every 60 frames ~1 second at 60fps)
        static int frameCounter = 0;
        if (++frameCounter >= 60)
        {
            frameCounter = 0;
            UpdateCachedValues();
        }

        ImGui::Text("Temperature Sensor Status");
        ImGui::Separator();

        // Display current temperature using cached values
        if (!mCachedMeasuredValue.IsNull())
        {
            float tempCelsius = static_cast<float>(mCachedMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Current Temperature: %.2f°C (%.2f°F)", tempCelsius, (tempCelsius * 9.0f / 5.0f) + 32.0f);
        }
        else
        {
            ImGui::Text("Current Temperature: N/A");
        }

        // Display min/max ranges using cached values
        if (!mCachedMinMeasuredValue.IsNull())
        {
            float minTempCelsius = static_cast<float>(mCachedMinMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Min Temperature: %.2f°C", minTempCelsius);
        }
        else
        {
            ImGui::Text("Min Temperature: N/A");
        }

        if (!mCachedMaxMeasuredValue.IsNull())
        {
            float maxTempCelsius = static_cast<float>(mCachedMaxMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Max Temperature: %.2f°C", maxTempCelsius);
        }
        else
        {
            ImGui::Text("Max Temperature: N/A");
        }

        ImGui::Separator();

        // Temperature simulation controls
        ImGui::Text("Temperature Simulation");

        static float simulatedTemp = 24.51f; // Default temperature in Celsius
        static bool autoMode       = false;
        static float autoModeTimer = 0.0f;

        if (ImGui::SliderFloat("Temperature (°C)", &simulatedTemp, -85.0f, 99.0f, "%.2f"))
        {
            SetTemperature(simulatedTemp);
        }

        ImGui::Checkbox("Auto Mode (Simulate changing temperature)", &autoMode);

        if (autoMode)
        {
            autoModeTimer += ImGui::GetIO().DeltaTime;
            // Simulate temperature variation using sine wave
            float baseTemp  = 24.0f;
            float variation = 5.0f * sinf(autoModeTimer * 0.5f); // Slow sine wave
            simulatedTemp   = baseTemp + variation;
            SetTemperature(simulatedTemp);
        }

        ImGui::Separator();

        // Quick temperature presets
        ImGui::Text("Quick Presets");

        if (ImGui::Button("Freezing (0°C)"))
        {
            simulatedTemp = 0.0f;
            SetTemperature(simulatedTemp);
        }
        ImGui::SameLine();

        if (ImGui::Button("Room Temp (22°C)"))
        {
            simulatedTemp = 22.0f;
            SetTemperature(simulatedTemp);
        }
        ImGui::SameLine();

        if (ImGui::Button("Hot (35°C)"))
        {
            simulatedTemp = 35.0f;
            SetTemperature(simulatedTemp);
        }

        if (ImGui::Button("Very Cold (-20°C)"))
        {
            simulatedTemp = -20.0f;
            SetTemperature(simulatedTemp);
        }
        ImGui::SameLine();

        if (ImGui::Button("Body Temp (37°C)"))
        {
            simulatedTemp = 37.0f;
            SetTemperature(simulatedTemp);
        }
        ImGui::SameLine();

        if (ImGui::Button("Very Hot (50°C)"))
        {
            simulatedTemp = 50.0f;
            SetTemperature(simulatedTemp);
        }

        ImGui::Separator();

        // Temperature unit conversion display
        ImGui::Text("Temperature Conversions");
        float fahrenheit = (simulatedTemp * 9.0f / 5.0f) + 32.0f;
        float kelvin     = simulatedTemp + 273.15f;
        ImGui::Text("Celsius: %.2f°C", simulatedTemp);
        ImGui::Text("Fahrenheit: %.2f°F", fahrenheit);
        ImGui::Text("Kelvin: %.2f K", kelvin);
    }

private:
    chip::EndpointId mEndpointId;

    // Cached attribute values to avoid direct access from ImGui thread
    chip::app::DataModel::Nullable<int16_t> mCachedMeasuredValue;
    chip::app::DataModel::Nullable<int16_t> mCachedMinMeasuredValue;
    chip::app::DataModel::Nullable<int16_t> mCachedMaxMeasuredValue;

    void SetTemperature(float temperatureCelsius)
    {
        // Convert to Matter format (hundredths of degrees Celsius)
        int16_t matterTemp = static_cast<int16_t>(temperatureCelsius * 100.0f);

        // Create a struct to pass data to the work function
        struct TemperatureUpdateData
        {
            chip::EndpointId endpointId;
            int16_t temperature;
        };

        TemperatureUpdateData * data = new TemperatureUpdateData{ mEndpointId, matterTemp };

        // Use DeviceLayer to safely schedule the attribute update on the Matter thread
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                TemperatureUpdateData * updateData = reinterpret_cast<TemperatureUpdateData *>(context);
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(updateData->endpointId,
                                                                                            updateData->temperature);
                delete updateData;
            },
            reinterpret_cast<intptr_t>(data));
    }

    void UpdateCachedValues()
    {
        // Schedule attribute reading on the Matter thread
        struct AttributeReadData
        {
            chip::EndpointId endpointId;
            chip::app::DataModel::Nullable<int16_t> * measuredValue;
            chip::app::DataModel::Nullable<int16_t> * minMeasuredValue;
            chip::app::DataModel::Nullable<int16_t> * maxMeasuredValue;
        };

        AttributeReadData * data =
            new AttributeReadData{ mEndpointId, &mCachedMeasuredValue, &mCachedMinMeasuredValue, &mCachedMaxMeasuredValue };

        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                AttributeReadData * readData = reinterpret_cast<AttributeReadData *>(context);
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Get(readData->endpointId,
                                                                                            *(readData->measuredValue));
                chip::app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Get(readData->endpointId,
                                                                                               *(readData->minMeasuredValue));
                chip::app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Get(readData->endpointId,
                                                                                               *(readData->maxMeasuredValue));
                delete readData;
            },
            reinterpret_cast<intptr_t>(data));
    }
};
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
// Custom BasicInformation AttributeAccessInterface for NodeLabel support
chip::app::Clusters::BasicInformation::TemperatureMeasurementAppBasicInformationAttrAccess gBasicInformationAttrAccess;
} // namespace

void ApplicationInit()
{
    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");

    // Initialize the temperature measurement manager
    TemperatureMeasurementManager::InitInstance(chip::EndpointId(1));

    ChipLogProgress(NotSpecified, "Temperature Measurement App initialized");
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface and optionally full-screen mode
    // Set the second parameter to true for full-screen mode
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<TemperatureSensorBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<TemperatureMeasurementControlWindow>(chip::EndpointId(1)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif
    return 0;
}
