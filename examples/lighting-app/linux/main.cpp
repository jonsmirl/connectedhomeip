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

#include "LightingAppBasicInformation.h"
#include "LightingAppCommandDelegate.h"
#include "LightingManager.h"
#include <AppMain.h>
#include <app/AttributeAccessInterfaceRegistry.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/color-control-server/color-control-server.h>
#include <app/server/Server.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/color_control_debug.h>
#include <imgui_ui/windows/group_management.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for lighting
class LightingBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    LightingBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Extended Color Light"; }
    uint16_t GetDeviceProductID() const override { return 0x8005; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Extended Color Light"; }
    std::string GetDevicePartNumber() const override { return "LIGHT-001"; }
    std::string GetDeviceSerialNumber() const override { return "LIGHT-123456"; }
    std::string GetDeviceUniqueID() const override { return "LIGHT-UID-123456"; }
};

#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

NamedPipeCommands sChipNamedPipeCommands;
LightingAppCommandDelegate sLightingAppCommandDelegate;

// Custom BasicInformation AttributeAccessInterface for NodeLabel support
chip::app::Clusters::BasicInformation::LightingAppBasicInformationAttrAccess gBasicInformationAttrAccess;

// Debug logging function that writes to file
void DebugLog(const std::string & message)
{
    std::ofstream debugFile("/tmp/lighting_app_debug.log", std::ios::app);
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

    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
        // Note: LEVEL_ACTION was removed in v1.5 - level control is handled differently now
    }
    else if (clusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        std::stringstream ss;
        ss << "COLOR_CONTROL_ATTRIBUTE: clusterId=0x" << std::hex << clusterId << " attributeId=0x" << attributeId
           << " type=" << std::dec << (int) type << " value=" << (int) *value << " size=" << size;
        DebugLog(ss.str());

        // Handle color attributes based on current color mode
        if (attributeId == ColorControl::Attributes::CurrentHue::Id ||
            attributeId == ColorControl::Attributes::CurrentSaturation::Id ||
            attributeId == ColorControl::Attributes::CurrentX::Id || attributeId == ColorControl::Attributes::CurrentY::Id ||
            attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
        {
            // First, get the current color mode to determine which attributes to read
            ColorControl::ColorModeEnum colorMode;
            Protocols::InteractionModel::Status status =
                ColorControl::Attributes::ColorMode::Get(attributePath.mEndpointId, &colorMode);
            if (status != Protocols::InteractionModel::Status::Success)
            {
                ChipLogError(Zcl, "Failed to get ColorMode: 0x%02x", to_underlying(status));
                return;
            }

            DebugLog("COLOR_MODE: " + std::to_string(to_underlying(colorMode)) + " (0=HSV, 1=XY, 2=CT)");

            if (colorMode == ColorControl::ColorModeEnum::kCurrentHueAndCurrentSaturation)
            {
                uint8_t hue, saturation;

                if (attributeId == ColorControl::Attributes::CurrentHue::Id)
                {
                    hue    = *value;
                    status = ColorControl::Attributes::CurrentSaturation::Get(attributePath.mEndpointId, &saturation);
                }
                else if (attributeId == ColorControl::Attributes::CurrentSaturation::Id)
                {
                    saturation = *value;
                    status     = ColorControl::Attributes::CurrentHue::Get(attributePath.mEndpointId, &hue);
                }
                else
                {
                    // Get both values for other attribute changes in HSV mode
                    status = ColorControl::Attributes::CurrentHue::Get(attributePath.mEndpointId, &hue);
                    if (status == Protocols::InteractionModel::Status::Success)
                    {
                        status = ColorControl::Attributes::CurrentSaturation::Get(attributePath.mEndpointId, &saturation);
                    }
                }

                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(Zcl, "HSV Mode - Hue: %u, Saturation: %u", hue, saturation);
                    DebugLog("HSV_VALUES: hue=" + std::to_string(hue) + ", saturation=" + std::to_string(saturation));
                }
            }
            else if (colorMode == ColorControl::ColorModeEnum::kCurrentXAndCurrentY)
            {
                uint16_t x, y;

                if (attributeId == ColorControl::Attributes::CurrentX::Id)
                {
                    x      = *reinterpret_cast<uint16_t *>(value);
                    status = ColorControl::Attributes::CurrentY::Get(attributePath.mEndpointId, &y);
                }
                else if (attributeId == ColorControl::Attributes::CurrentY::Id)
                {
                    y      = *reinterpret_cast<uint16_t *>(value);
                    status = ColorControl::Attributes::CurrentX::Get(attributePath.mEndpointId, &x);
                }
                else
                {
                    // Get both values for other attribute changes in XY mode
                    status = ColorControl::Attributes::CurrentX::Get(attributePath.mEndpointId, &x);
                    if (status == Protocols::InteractionModel::Status::Success)
                    {
                        status = ColorControl::Attributes::CurrentY::Get(attributePath.mEndpointId, &y);
                    }
                }

                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(Zcl, "XY Mode - X: %u, Y: %u", x, y);
                    DebugLog("XY_VALUES: x=" + std::to_string(x) + ", y=" + std::to_string(y));
                }
            }
            else if (colorMode == ColorControl::ColorModeEnum::kColorTemperatureMireds)
            {
                uint16_t colorTemp;

                if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
                {
                    colorTemp = *reinterpret_cast<uint16_t *>(value);
                }
                else
                {
                    status = ColorControl::Attributes::ColorTemperatureMireds::Get(attributePath.mEndpointId, &colorTemp);
                }

                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(Zcl, "CT Mode - Color Temperature: %u mireds", colorTemp);
                    DebugLog("CT_VALUES: colorTemp=" + std::to_string(colorTemp));
                }
            }

            // Note: Linux LightingManager doesn't support color control yet
            ChipLogProgress(Zcl, "Color values received but not applied - Linux LightingManager needs color support");
        }
        else
        {
            ChipLogProgress(Zcl, "Unhandled ColorControl attribute: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
        }
    }
    else if (clusterId == Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

void ApplicationInit()
{
    // Debug log application startup
    DebugLog("=== LIGHTING APP STARTING ===");

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
    DebugLog("COLOR_CONTROL_HSV: ENABLED - HSV commands should work");
#else
    DebugLog("COLOR_CONTROL_HSV: DISABLED - HSV commands will be rejected");
#endif

    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");

    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sLightingAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

#ifdef __NuttX__
// NuttX requires the main function to be defined with C-linkage. However, marking
// the main as extern "C" is not strictly conformant with the C++ standard. Since
// clang >= 20 such code triggers -Wmain warning.
extern "C" {
#endif

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    CHIP_ERROR err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize lighting manager: %" CHIP_ERROR_FORMAT, err.Format());
        chip::DeviceLayer::PlatformMgr().Shutdown();
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface and optionally full-screen mode
    // Set the second parameter to true for full-screen mode
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<LightingBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::ColorControlDebug>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::GroupManagement>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::Light>(chip::EndpointId(1)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::OccupancySensing>(chip::EndpointId(1), "Occupancy"));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}

#ifdef __NuttX__
}
#endif
