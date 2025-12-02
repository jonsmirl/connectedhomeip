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

#include <AppMain.h>
#include <platform/CHIPDeviceConfig.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/boolean_state.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for contact sensor
class ContactSensorBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    ContactSensorBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Contact Sensor"; }
    uint16_t GetDeviceProductID() const override { return 0x800F; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Contact Sensor"; }
    std::string GetDevicePartNumber() const override { return "CS-001"; }
    std::string GetDeviceSerialNumber() const override { return "CS-123456"; }
    std::string GetDeviceUniqueID() const override { return "CS-UID-123456"; }
};
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void ApplicationInit() {}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface
    example::Ui::ImguiUi ui(true, false);

    ui.AddWindow(std::make_unique<ContactSensorBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::BooleanState>(chip::EndpointId(1), "Contact Sensor"));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
