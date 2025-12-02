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

#include "PressureMeasurementAppAttrUpdateDelegate.h"
#include "PressureMeasurementAppBasicInformation.h"
#include "PressureMeasurementManager.h"

#include <AppMain.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>

#include <cmath>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <app-common/zap-generated/attributes/Accessors.h>
#include <imgui.h>
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/qrcode.h>
#include <imgui_ui/windows/window.h>

// Custom BasicInformation window for pressure sensor
class PressureSensorBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    PressureSensorBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Pressure Sensor"; }
    uint16_t GetDeviceProductID() const override { return 0x800D; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Pressure Sensor"; }
    std::string GetDevicePartNumber() const override { return "PRESS-001"; }
    std::string GetDeviceSerialNumber() const override { return "PRESS-123456"; }
    std::string GetDeviceUniqueID() const override { return "PRESS-UID-123456"; }
};

// Custom Pressure Measurement control window
class PressureMeasurementControlWindow : public example::Ui::Window
{
public:
    PressureMeasurementControlWindow(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    const char * GetDisplayName() const override { return "Pressure Measurement"; }

    void Render() override
    {
        // Update cached values periodically (every 60 frames ~1 second at 60fps)
        static int frameCounter = 0;
        if (++frameCounter >= 60)
        {
            frameCounter = 0;
            UpdateCachedValues();
        }

        ImGui::Text("Pressure Sensor Status");
        ImGui::Separator();

        // Display current pressure using cached values
        if (!mCachedMeasuredValue.IsNull())
        {
            float pressureKPa = static_cast<float>(mCachedMeasuredValue.Value()) / 10.0f; // Convert from dPa to kPa
            float pressurePsi = pressureKPa * 0.145038f;                                  // Convert kPa to PSI
            ImGui::Text("Current Pressure: %.1f kPa (%.2f PSI)", pressureKPa, pressurePsi);
        }
        else
        {
            ImGui::Text("Current Pressure: N/A");
        }

        // Display min/max ranges using cached values
        if (!mCachedMinMeasuredValue.IsNull())
        {
            float minPressureKPa = static_cast<float>(mCachedMinMeasuredValue.Value()) / 10.0f;
            ImGui::Text("Min Pressure: %.1f kPa", minPressureKPa);
        }
        else
        {
            ImGui::Text("Min Pressure: N/A");
        }

        if (!mCachedMaxMeasuredValue.IsNull())
        {
            float maxPressureKPa = static_cast<float>(mCachedMaxMeasuredValue.Value()) / 10.0f;
            ImGui::Text("Max Pressure: %.1f kPa", maxPressureKPa);
        }
        else
        {
            ImGui::Text("Max Pressure: N/A");
        }

        ImGui::Separator();

        // Pressure simulation controls
        ImGui::Text("Pressure Simulation");

        static float simulatedPressure = 101.3f; // Default atmospheric pressure in kPa
        static bool autoMode           = false;
        static float autoModeTimer     = 0.0f;

        if (ImGui::SliderFloat("Pressure (kPa)", &simulatedPressure, 0.0f, 200.0f, "%.1f"))
        {
            SetPressure(simulatedPressure);
        }

        ImGui::Checkbox("Auto Mode (Simulate changing pressure)", &autoMode);

        if (autoMode)
        {
            autoModeTimer += ImGui::GetIO().DeltaTime;
            // Simulate pressure variation using sine wave
            float basePressure = 101.3f;                             // Atmospheric pressure
            float variation    = 10.0f * sinf(autoModeTimer * 0.3f); // Slow sine wave
            simulatedPressure  = basePressure + variation;
            SetPressure(simulatedPressure);
        }

        ImGui::Separator();

        // Quick pressure presets
        ImGui::Text("Quick Presets");

        if (ImGui::Button("Vacuum (0 kPa)"))
        {
            simulatedPressure = 0.0f;
            SetPressure(simulatedPressure);
        }
        ImGui::SameLine();

        if (ImGui::Button("Atmospheric (101.3 kPa)"))
        {
            simulatedPressure = 101.3f;
            SetPressure(simulatedPressure);
        }
        ImGui::SameLine();

        if (ImGui::Button("High (150 kPa)"))
        {
            simulatedPressure = 150.0f;
            SetPressure(simulatedPressure);
        }

        ImGui::Separator();

        // Pressure unit conversion display
        ImGui::Text("Pressure Conversions");
        float psi  = simulatedPressure * 0.145038f;
        float bar  = simulatedPressure / 100.0f;
        float mmHg = simulatedPressure * 7.50062f;
        ImGui::Text("kPa: %.1f", simulatedPressure);
        ImGui::Text("PSI: %.2f", psi);
        ImGui::Text("Bar: %.3f", bar);
        ImGui::Text("mmHg: %.1f", mmHg);
    }

private:
    chip::EndpointId mEndpointId;

    // Cached attribute values to avoid direct access from ImGui thread
    chip::app::DataModel::Nullable<int16_t> mCachedMeasuredValue;
    chip::app::DataModel::Nullable<int16_t> mCachedMinMeasuredValue;
    chip::app::DataModel::Nullable<int16_t> mCachedMaxMeasuredValue;

    void SetPressure(float pressureKPa)
    {
        // Convert to Matter format (dPa - decipascals)
        int16_t matterPressure = static_cast<int16_t>(pressureKPa * 10.0f);

        // Create a struct to pass data to the work function
        struct PressureUpdateData
        {
            chip::EndpointId endpointId;
            int16_t pressure;
        };

        PressureUpdateData * data = new PressureUpdateData{ mEndpointId, matterPressure };

        // Use DeviceLayer to safely schedule the attribute update on the Matter thread
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                PressureUpdateData * updateData = reinterpret_cast<PressureUpdateData *>(context);
                chip::app::Clusters::PressureMeasurement::Attributes::MeasuredValue::Set(updateData->endpointId,
                                                                                         updateData->pressure);
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
                chip::app::Clusters::PressureMeasurement::Attributes::MeasuredValue::Get(readData->endpointId,
                                                                                         *(readData->measuredValue));
                chip::app::Clusters::PressureMeasurement::Attributes::MinMeasuredValue::Get(readData->endpointId,
                                                                                            *(readData->minMeasuredValue));
                chip::app::Clusters::PressureMeasurement::Attributes::MaxMeasuredValue::Get(readData->endpointId,
                                                                                            *(readData->maxMeasuredValue));
                delete readData;
            },
            reinterpret_cast<intptr_t>(data));
    }
};
#endif

static constexpr chip::EndpointId sPressureMeasurementEndpointId = 1;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
NamedPipeCommands sChipNamedPipeCommands;
PressureMeasurementAppAttrUpdateDelegate sPressureMeasurementAppAttrUpdateDelegate;

// Custom BasicInformation AttributeAccessInterface for NodeLabel support
chip::app::Clusters::BasicInformation::PressureMeasurementAppBasicInformationAttrAccess gBasicInformationAttrAccess;
} // namespace

void ApplicationInit()
{
    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");

    PressureMeasurementManager::InitInstance(sPressureMeasurementEndpointId);
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sPressureMeasurementAppAttrUpdateDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface and optionally full-screen mode
    // Set the second parameter to true for full-screen mode
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<PressureSensorBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<PressureMeasurementControlWindow>(chip::EndpointId(1)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
