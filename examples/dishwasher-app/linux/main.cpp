/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "operational-state-delegate-impl.h"
#include "DishwasherAppBasicInformation.h"
#include <AppMain.h>
#include <app/AttributeAccessInterfaceRegistry.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for dishwasher
class DishwasherBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    DishwasherBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Dishwasher"; }
    uint16_t GetDeviceProductID() const override { return 0x800C; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Dishwasher"; }
    std::string GetDevicePartNumber() const override { return "DW-001"; }
    std::string GetDeviceSerialNumber() const override { return "DW-123456"; }
    std::string GetDeviceUniqueID() const override { return "DW-UID-123456"; }
};
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// Custom BasicInformation AttributeAccessInterface for NodeLabel support
chip::app::Clusters::BasicInformation::DishwasherAppBasicInformationAttrAccess gBasicInformationAttrAccess;

void ApplicationInit()
{
    // Register custom BasicInformation AttributeAccessInterface for NodeLabel support
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gBasicInformationAttrAccess);
    ChipLogProgress(NotSpecified, "Registered custom BasicInformation AttributeAccessInterface for NodeLabel support");
}

void ApplicationShutdown()
{
    OperationalState::Shutdown();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<DishwasherBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
