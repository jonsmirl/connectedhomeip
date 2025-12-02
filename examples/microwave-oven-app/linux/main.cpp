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
#include "microwave-oven-device.h"
#include <AppMain.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/qrcode.h>

// Custom BasicInformation window for microwave oven
class MicrowaveOvenBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    MicrowaveOvenBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Microwave Oven"; }
    uint16_t GetDeviceProductID() const override { return 0x800D; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Microwave Oven"; }
    std::string GetDevicePartNumber() const override { return "MW-001"; }
    std::string GetDeviceSerialNumber() const override { return "MW-123456"; }
    std::string GetDeviceUniqueID() const override { return "MW-UID-123456"; }
};
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void ApplicationInit()
{
    MatterMicrowaveOvenServerInit();
}

void ApplicationShutdown()
{
    MatterMicrowaveOvenServerShutdown();
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

    ui.AddWindow(std::make_unique<MicrowaveOvenBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
