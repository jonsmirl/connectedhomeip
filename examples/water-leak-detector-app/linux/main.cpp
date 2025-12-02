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

#include "WaterLeakDetectorAppAttrUpdateDelegate.h"
#include "WaterLeakDetectorAppBasicInformation.h"
#include "WaterLeakDetectorManager.h"

#include <AppMain.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <platform/CHIPDeviceConfig.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/boolean_state.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for water leak detector
class WaterLeakDetectorBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    WaterLeakDetectorBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Water Leak Detector"; }
    uint16_t GetDeviceProductID() const override { return 0x800B; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Water Leak Detector"; }
    std::string GetDevicePartNumber() const override { return "WLD-001"; }
    std::string GetDeviceSerialNumber() const override { return "WLD-123456"; }
    std::string GetDeviceUniqueID() const override { return "WLD-UID-123456"; }
};
#endif

static constexpr chip::EndpointId sWaterLeakDetectorEndpointId = 1;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
NamedPipeCommands sChipNamedPipeCommands;
WaterLeakDetectorAppAttrUpdateDelegate sWaterLeakDetectorAppAttrUpdateDelegate;

// Custom BasicInformation AttributeAccessInterface for NodeLabel support
chip::app::Clusters::BasicInformation::WaterLeakDetectorAppBasicInformationAttrAccess gBasicInformationAttrAccess;
} // namespace

void ApplicationInit()
{
    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");

    WaterLeakDetectorManager::InitInstance(sWaterLeakDetectorEndpointId);
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sWaterLeakDetectorAppAttrUpdateDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<WaterLeakDetectorBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::BooleanState>(chip::EndpointId(1), "Water Leak"));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
