/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "RockerManager.h"
#include <AppMain.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
// v1.5 binding cluster includes
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/clusters/color-control-server/color-control-server.h>
// Identify cluster for visual indication
#include <app/clusters/identify-server/identify-server.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

// Global identify state for imgui visual feedback
// Using mActive member from each Identify struct to track state
static Identify * gIdentifyPtrs[3] = { nullptr, nullptr, nullptr };

void OnIdentifyStart(Identify * identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(Identify * identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

// Helper to check if identify is active on an endpoint
static bool IsIdentifyActive(chip::EndpointId ep)
{
    if (ep < 3 && gIdentifyPtrs[ep] != nullptr)
    {
        return gIdentifyPtrs[ep]->mActive;
    }
    return false;
}

void OnTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "OnTriggerEffect: Blink");
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "OnTriggerEffect: Breathe");
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "OnTriggerEffect: Okay");
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "OnTriggerEffect: ChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "OnTriggerEffect: Unknown");
        break;
    }
}

// Identify structs for each endpoint with Identify cluster
static Identify gIdentify0(
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop,
    chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnTriggerEffect);

static Identify gIdentify1(
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop,
    chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnTriggerEffect);

// Initialize identify pointers (called from ApplicationInit)
static void InitIdentifyPointers()
{
    gIdentifyPtrs[0] = &gIdentify0;
    gIdentifyPtrs[1] = &gIdentify1;
}

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui.h>
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/color_control_debug.h>
#include <imgui_ui/windows/group_management.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for rocker device with identify support
class RockerBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    RockerBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId), mEndpointId(endpointId) {}

    void Render() override
    {
        // Show identify indicator if active
        if (IsIdentifyActive(mEndpointId))
        {
            // Blink effect using time
            double time     = ImGui::GetTime();
            bool showBlink  = (static_cast<int>(time * 4) % 2) == 0; // Blink 2Hz
            ImVec4 blinkColor = showBlink ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, blinkColor);
            ImGui::Text("*** IDENTIFYING - Endpoint %d ***", mEndpointId);
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        // Call base class render
        BasicInformation::Render();
    }

protected:
    std::string GetDeviceProductName() const override { return "Rocker Device"; }
    uint16_t GetDeviceProductID() const override { return 0x8007; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Rocker Device"; }
    std::string GetDevicePartNumber() const override { return "ROCKER-001"; }
    std::string GetDeviceSerialNumber() const override { return "ROCKER-123456"; }
    std::string GetDeviceUniqueID() const override { return "ROCKER-UID-123456"; }

private:
    chip::EndpointId mEndpointId;
};

// Custom Light window for the light endpoint with identify support
class RockerLightWindow : public example::Ui::Windows::Light
{
public:
    RockerLightWindow(chip::EndpointId endpointId) : Light(endpointId), mEndpointId(endpointId) {}
    const char * GetDisplayName() const override { return "Rocker Light (Endpoint 1)"; }

    void Render() override
    {
        // Show identify indicator if active
        if (IsIdentifyActive(mEndpointId))
        {
            // Blink effect using time
            double time     = ImGui::GetTime();
            bool showBlink  = (static_cast<int>(time * 4) % 2) == 0; // Blink 2Hz
            ImVec4 blinkColor = showBlink ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, blinkColor);
            ImGui::Text("*** IDENTIFYING - Endpoint %d ***", mEndpointId);
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        // Call base class render
        Light::Render();
    }

private:
    chip::EndpointId mEndpointId;
};

// Custom Switch Control window for the switch endpoint
class RockerSwitchWindow : public example::Ui::Window
{
public:
    RockerSwitchWindow() = default;
    const char * GetDisplayName() const override { return "Rocker Switch (Endpoint 2)"; }

    void Render() override
    {
        ImGui::Text("Switch Endpoint (Endpoint 2)");
        ImGui::Separator();

        if (ImGui::Button("Send On Command"))
        {
            RockerManager::GetInstance().SendOnOffCommand(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Send Off Command"))
        {
            RockerManager::GetInstance().SendOnOffCommand(false);
        }

        // Use static variables that persist during drag operations
        static int level  = static_cast<int>(RockerManager::GetInstance().GetLightLevel());
        static int colorX = static_cast<int>(RockerManager::GetInstance().GetLightColorX());
        static int colorY = static_cast<int>(RockerManager::GetInstance().GetLightColorY());

        // Track which sliders are being actively dragged
        static bool levelActive  = false;
        static bool colorXActive = false;
        static bool colorYActive = false;

        // Update static variables when not actively dragging (sync with actual state)
        if (!levelActive && !colorXActive && !colorYActive)
        {
            level  = static_cast<int>(RockerManager::GetInstance().GetLightLevel());
            colorX = static_cast<int>(RockerManager::GetInstance().GetLightColorX());
            colorY = static_cast<int>(RockerManager::GetInstance().GetLightColorY());
        }

        if (ImGui::SliderInt("Level", &level, 0, 254))
        {
            levelActive = true;
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            levelActive = false;
            RockerManager::GetInstance().SendLevelCommand(static_cast<uint8_t>(level));
        }

        // ColorXY sliders for direct color control without conversion issues
        if (ImGui::SliderInt("Color X", &colorX, 0, 65535))
        {
            colorXActive = true;
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            colorXActive = false;
            // Send ColorXY command directly
            RockerManager::GetInstance().SendColorXYCommand(static_cast<uint16_t>(colorX), static_cast<uint16_t>(colorY));
        }

        if (ImGui::SliderInt("Color Y", &colorY, 0, 65535))
        {
            colorYActive = true;
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            colorYActive = false;
            // Send ColorXY command directly
            RockerManager::GetInstance().SendColorXYCommand(static_cast<uint16_t>(colorX), static_cast<uint16_t>(colorY));
        }

        ImGui::Separator();

        // Display current color as a color square
        uint8_t red, green, blue;
        RockerManager::GetInstance().ColorXYToRGB(static_cast<uint16_t>(colorX), static_cast<uint16_t>(colorY), red, green, blue);

        // Convert to ImGui color format (0.0-1.0 range)
        ImVec4 currentColor = ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);

        // Display color square
        ImGui::Text("Current Color:");
        ImGui::ColorButton("Current Color Display", currentColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
                           ImVec2(80, 80));

        ImGui::Separator();

        if (ImGui::Button("Send Identify"))
        {
            RockerManager::GetInstance().SendIdentifyCommand();
        }

        ImGui::Text("Note: Commands are sent to bound devices");
        ImGui::Text("Use binding cluster to configure targets");
    }
};

#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// Debug logging function that writes to file
void DebugLog(const std::string & message)
{
    std::ofstream debugFile("/tmp/rocker_app_debug.log", std::ios::app);
    if (debugFile.is_open())
    {
        auto now = std::time(nullptr);
        auto tm  = *std::localtime(&now);
        debugFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
        debugFile.close();
    }
}

} // namespace

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    EndpointId endpointId   = attributePath.mEndpointId;

    ChipLogProgress(Zcl, "Cluster callback: Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI " Attribute=" ChipLogFormatMEI,
                    endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId));

    // Handle light endpoint (endpoint 1) attribute changes
    if (endpointId == RockerManager::kLightEndpointId)
    {
        if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
        {
            bool onOffValue = *reinterpret_cast<bool *>(value);
            // Update actual light state when external Matter commands change EP1 attributes
            RockerManager::GetInstance().UpdateActualLightOnOff(onOffValue);
            DebugLog("External command: Light OnOff changed to: " + std::to_string(onOffValue));
        }
        else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
        {
            uint8_t levelValue = *reinterpret_cast<uint8_t *>(value);
            // Update actual light state when external Matter commands change EP1 attributes
            RockerManager::GetInstance().UpdateActualLightLevel(levelValue);
            DebugLog("External command: Light Level changed to: " + std::to_string(levelValue));
        }
        else if (clusterId == ColorControl::Id)
        {
            if (attributeId == ColorControl::Attributes::CurrentHue::Id)
            {
                uint8_t hueValue = *reinterpret_cast<uint8_t *>(value);
                // Update actual light state when external Matter commands change EP1 attributes
                // This converts hue to ColorXY internally for consistency
                RockerManager::GetInstance().UpdateActualLightHue(hueValue);
                DebugLog("External command: Light Hue changed to: " + std::to_string(hueValue));
            }
            else if (attributeId == ColorControl::Attributes::CurrentSaturation::Id)
            {
                uint8_t saturationValue = *reinterpret_cast<uint8_t *>(value);
                // Update actual light state when external Matter commands change EP1 attributes
                // This converts saturation to ColorXY internally for consistency
                RockerManager::GetInstance().UpdateActualLightSaturation(saturationValue);
                DebugLog("External command: Light Saturation changed to: " + std::to_string(saturationValue));
            }
            else if (attributeId == ColorControl::Attributes::CurrentX::Id)
            {
                uint16_t colorXValue = *reinterpret_cast<uint16_t *>(value);
                // Update actual light state with ColorXY coordinates
                uint16_t currentColorY = RockerManager::GetInstance().GetLightColorY();
                RockerManager::GetInstance().UpdateActualLightColorXY(colorXValue, currentColorY);
                DebugLog("External command: Light ColorX changed to: 0x" + std::to_string(colorXValue));
            }
            else if (attributeId == ColorControl::Attributes::CurrentY::Id)
            {
                uint16_t colorYValue = *reinterpret_cast<uint16_t *>(value);
                // Update actual light state with ColorXY coordinates
                uint16_t currentColorX = RockerManager::GetInstance().GetLightColorX();
                RockerManager::GetInstance().UpdateActualLightColorXY(currentColorX, colorYValue);
                DebugLog("External command: Light ColorY changed to: 0x" + std::to_string(colorYValue));
            }
        }
    }
    // Handle switch endpoint (endpoint 2) attribute changes
    else if (endpointId == RockerManager::kSwitchEndpointId)
    {
        // Switch endpoint primarily uses client clusters for sending commands
        // Binding cluster changes would be handled here
        DebugLog("Switch endpoint attribute change detected");
    }
}

void ApplicationInit()
{
    ChipLogProgress(AppServer, "Rocker App: ApplicationInit()");

    // Initialize identify pointers for imgui visual feedback
    InitIdentifyPointers();

    // Initialize the rocker manager
    CHIP_ERROR err = RockerManager::GetInstance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize RockerManager: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    DebugLog("Rocker application initialized successfully");
    ChipLogProgress(AppServer, "Rocker App: Dual-endpoint device ready");
    ChipLogProgress(AppServer, "  - Endpoint 1: Light device (server clusters)");
    ChipLogProgress(AppServer, "  - Endpoint 2: Switch device (client clusters)");
}

void ApplicationShutdown()
{
    ChipLogProgress(AppServer, "Rocker App: ApplicationShutdown()");
    RockerManager::GetInstance().Shutdown();
    DebugLog("Rocker application shutdown");
}

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

    ui.AddWindow(std::make_unique<RockerBasicInformation>(0)); // Root endpoint
    ui.AddWindow(std::make_unique<RockerLightWindow>(RockerManager::kLightEndpointId));
    ui.AddWindow(std::make_unique<RockerSwitchWindow>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif
    return 0;
}
