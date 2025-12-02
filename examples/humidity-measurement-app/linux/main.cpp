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

#include "HumidityMeasurementAppAttrUpdateDelegate.h"
#include "HumidityMeasurementAppBasicInformation.h"
#include "HumidityMeasurementManager.h"

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

// Custom BasicInformation window for humidity sensor
class HumiditySensorBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    HumiditySensorBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Humidity Sensor"; }
    uint16_t GetDeviceProductID() const override { return 0x800B; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Humidity Sensor"; }
    std::string GetDevicePartNumber() const override { return "HUM-001"; }
    std::string GetDeviceSerialNumber() const override { return "HUM-123456"; }
    std::string GetDeviceUniqueID() const override { return "HUM-UID-123456"; }
};

// Custom Humidity Measurement control window
class HumidityMeasurementControlWindow : public example::Ui::Window
{
public:
    HumidityMeasurementControlWindow(chip::EndpointId endpointId) : mEndpointId(endpointId)
    {
        // Initialize cached values
        UpdateCachedValues();
    }

    const char * GetDisplayName() const override { return "Humidity Measurement"; }

    void Render() override
    {
        // Update cached values periodically (every 60 frames ~1 second at 60fps)
        static int frameCounter = 0;
        if (++frameCounter >= 60)
        {
            frameCounter = 0;
            UpdateCachedValues();
        }

        ImGui::Text("Humidity Sensor Status");
        ImGui::Separator();

        // Display current humidity using cached values
        if (!mCachedMeasuredValue.IsNull())
        {
            float humidityPercent = static_cast<float>(mCachedMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Current Humidity: %.1f%%", humidityPercent);
        }
        else
        {
            ImGui::Text("Current Humidity: N/A");
        }

        // Display min/max ranges using cached values
        if (!mCachedMinMeasuredValue.IsNull())
        {
            float minHumidityPercent = static_cast<float>(mCachedMinMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Min Humidity: %.1f%%", minHumidityPercent);
        }
        else
        {
            ImGui::Text("Min Humidity: N/A");
        }

        if (!mCachedMaxMeasuredValue.IsNull())
        {
            float maxHumidityPercent = static_cast<float>(mCachedMaxMeasuredValue.Value()) / 100.0f;
            ImGui::Text("Max Humidity: %.1f%%", maxHumidityPercent);
        }
        else
        {
            ImGui::Text("Max Humidity: N/A");
        }

        ImGui::Separator();

        // Humidity simulation controls
        ImGui::Text("Humidity Simulation");

        static float simulatedHumidity = 45.0f; // Default humidity in percent
        static bool autoMode           = false;
        static float autoModeTimer     = 0.0f;

        if (ImGui::SliderFloat("Humidity (%)", &simulatedHumidity, 0.0f, 100.0f, "%.1f"))
        {
            SetHumidity(simulatedHumidity);
        }

        ImGui::Checkbox("Auto Mode (Simulate changing humidity)", &autoMode);

        if (autoMode)
        {
            autoModeTimer += ImGui::GetIO().DeltaTime;
            // Simulate humidity variation using sine wave
            float baseHumidity = 50.0f;                              // Base humidity
            float variation    = 20.0f * sinf(autoModeTimer * 0.2f); // Slow sine wave
            simulatedHumidity  = baseHumidity + variation;
            if (simulatedHumidity < 0.0f)
                simulatedHumidity = 0.0f;
            if (simulatedHumidity > 100.0f)
                simulatedHumidity = 100.0f;
            SetHumidity(simulatedHumidity);
        }

        ImGui::Separator();

        // Quick humidity presets
        ImGui::Text("Quick Presets");

        if (ImGui::Button("Dry (20%)"))
        {
            simulatedHumidity = 20.0f;
            SetHumidity(simulatedHumidity);
        }
        ImGui::SameLine();
        if (ImGui::Button("Comfortable (45%)"))
        {
            simulatedHumidity = 45.0f;
            SetHumidity(simulatedHumidity);
        }
        ImGui::SameLine();
        if (ImGui::Button("Humid (80%)"))
        {
            simulatedHumidity = 80.0f;
            SetHumidity(simulatedHumidity);
        }
    }

private:
    chip::EndpointId mEndpointId;

    // Cached attribute values to avoid direct access from ImGui thread
    chip::app::DataModel::Nullable<uint16_t> mCachedMeasuredValue;
    chip::app::DataModel::Nullable<uint16_t> mCachedMinMeasuredValue;
    chip::app::DataModel::Nullable<uint16_t> mCachedMaxMeasuredValue;

    void SetHumidity(float humidityPercent)
    {
        // Convert to Matter format (hundredths of percent)
        uint16_t matterHumidity = static_cast<uint16_t>(humidityPercent * 100.0f);

        // Create a struct to pass data to the work function
        struct HumidityUpdateData
        {
            chip::EndpointId endpointId;
            uint16_t humidity;
        };

        HumidityUpdateData * data = new HumidityUpdateData{ mEndpointId, matterHumidity };

        // Use DeviceLayer to safely schedule the attribute update on the Matter thread
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                HumidityUpdateData * updateData = reinterpret_cast<HumidityUpdateData *>(context);
                chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(updateData->endpointId,
                                                                                                 updateData->humidity);
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
            chip::app::DataModel::Nullable<uint16_t> * measuredValue;
            chip::app::DataModel::Nullable<uint16_t> * minMeasuredValue;
            chip::app::DataModel::Nullable<uint16_t> * maxMeasuredValue;
        };

        AttributeReadData * data =
            new AttributeReadData{ mEndpointId, &mCachedMeasuredValue, &mCachedMinMeasuredValue, &mCachedMaxMeasuredValue };

        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                AttributeReadData * readData = reinterpret_cast<AttributeReadData *>(context);
                chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Get(readData->endpointId,
                                                                                                 *(readData->measuredValue));
                chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Get(readData->endpointId,
                                                                                                    *(readData->minMeasuredValue));
                chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Get(readData->endpointId,
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
chip::app::Clusters::BasicInformation::HumidityMeasurementAppBasicInformationAttrAccess gBasicInformationAttrAccess;

// Named pipe command delegate for external control
HumidityMeasurementAppAttrUpdateDelegate sHumidityMeasurementAppAttrUpdateDelegate;
NamedPipeCommands sChipNamedPipeCommands;
} // namespace

void ApplicationInit()
{
    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");

    // Initialize the humidity measurement manager
    HumidityMeasurementManager::InitInstance(chip::EndpointId(1));

    ChipLogProgress(NotSpecified, "Humidity Measurement App initialized");
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sHumidityMeasurementAppAttrUpdateDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface and optionally full-screen mode
    // Set the second parameter to true for full-screen mode
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<HumiditySensorBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<HumidityMeasurementControlWindow>(chip::EndpointId(1)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
