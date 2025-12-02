/*
 * Copyright (c) 2024 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LightingManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/color-control-server/color-control-server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace chip;
using namespace chip::app::Clusters::ColorControl;

// Debug logging function that writes to file
void DebugLogColorControl(const std::string & message)
{
    std::ofstream debugFile("/tmp/lighting_app_color_debug.log", std::ios::app);
    if (debugFile.is_open())
    {
        auto now = std::time(nullptr);
        auto tm  = *std::localtime(&now);
        debugFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
        debugFile.close();
    }
}

// Override the Color Control server's empty computePwmFromHsv function using strong symbol
// This function is called by the Color Control server when HSV commands are processed
extern "C" void ColorControlServer_computePwmFromHsv(chip::EndpointId endpoint) __attribute__((weak));
void ColorControlServer::computePwmFromHsv(chip::EndpointId endpoint)
{
    DebugLogColorControl("COMPUTE_PWM_FROM_HSV: Called for endpoint " + std::to_string(endpoint));

    uint8_t hue = 0, saturation = 0;

    // Get current HSV values from the cluster
    using namespace chip::app::Clusters::ColorControl::Attributes;
    chip::Protocols::InteractionModel::Status status;
    status = CurrentHue::Get(endpoint, &hue);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        DebugLogColorControl("COMPUTE_PWM_FROM_HSV: Failed to get CurrentHue, using default value 0");
        hue = 0; // Use default value
    }

    status = CurrentSaturation::Get(endpoint, &saturation);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        DebugLogColorControl("COMPUTE_PWM_FROM_HSV: Failed to get CurrentSaturation, using default value 0");
        saturation = 0; // Use default value
    }

    DebugLogColorControl("COMPUTE_PWM_FROM_HSV: hue=" + std::to_string(hue) + " saturation=" + std::to_string(saturation));

    // Set the color mode to HSV so the UI displays the color correctly
    ColorMode::Set(endpoint, chip::app::Clusters::ColorControl::ColorModeEnum::kCurrentHueAndCurrentSaturation);

    // Per Matter specification, Color Control cluster commands should NOT affect OnOff state
    // Color commands only change color attributes, they don't turn lights on/off
    // The Matter stack should have already updated the CurrentHue and CurrentSaturation attributes
    // before calling this function, so we don't need to set them again
    DebugLogColorControl("COMPUTE_PWM_FROM_HSV: Color processing completed - hue=" + std::to_string(hue) +
                         ", saturation=" + std::to_string(saturation) + " (OnOff state unchanged per Matter spec)");

    DebugLogColorControl("COMPUTE_PWM_FROM_HSV: Color control completed successfully");
}

// Override the Color Control server's empty computePwmFromXy function
// This function is called by the Color Control server when XY commands are processed
void ColorControlServer::computePwmFromXy(chip::EndpointId endpoint)
{
    DebugLogColorControl("COMPUTE_PWM_FROM_XY: Called for endpoint " + std::to_string(endpoint));

    uint16_t x = 0, y = 0;

    // Get current XY values from the cluster
    using namespace chip::app::Clusters::ColorControl::Attributes;
    chip::Protocols::InteractionModel::Status status;
    status = CurrentX::Get(endpoint, &x);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        DebugLogColorControl("COMPUTE_PWM_FROM_XY: Failed to get CurrentX, using default value 0");
        x = 0; // Use default value
    }

    status = CurrentY::Get(endpoint, &y);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        DebugLogColorControl("COMPUTE_PWM_FROM_XY: Failed to get CurrentY, using default value 0");
        y = 0; // Use default value
    }

    DebugLogColorControl("COMPUTE_PWM_FROM_XY: x=" + std::to_string(x) + " y=" + std::to_string(y));

    // Set the color mode to XY so the UI displays the color correctly
    ColorMode::Set(endpoint, chip::app::Clusters::ColorControl::ColorModeEnum::kCurrentXAndCurrentY);

    // Per Matter specification, Color Control cluster commands should NOT affect OnOff state
    // Color commands only change color attributes, they don't turn lights on/off
    // The Matter stack should have already updated the CurrentX and CurrentY attributes
    // before calling this function, so we don't need to set them again
    DebugLogColorControl("COMPUTE_PWM_FROM_XY: Color processing completed - x=" + std::to_string(x) + ", y=" + std::to_string(y) +
                         " (OnOff state unchanged per Matter spec)");

    DebugLogColorControl("COMPUTE_PWM_FROM_XY: Color control completed successfully");
}

// Override the Color Control server's empty computePwmFromTemp function
// This function is called by the Color Control server when color temperature commands are processed
void ColorControlServer::computePwmFromTemp(chip::EndpointId endpoint)
{
    DebugLogColorControl("COMPUTE_PWM_FROM_TEMP: Called for endpoint " + std::to_string(endpoint));

    uint16_t colorTemperatureMireds = 0;

    // Get current color temperature value from the cluster
    using namespace chip::app::Clusters::ColorControl::Attributes;
    chip::Protocols::InteractionModel::Status status;
    status = ColorTemperatureMireds::Get(endpoint, &colorTemperatureMireds);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        DebugLogColorControl("COMPUTE_PWM_FROM_TEMP: Failed to get ColorTemperatureMireds, using default value 0");
        colorTemperatureMireds = 0; // Use default value
    }

    DebugLogColorControl("COMPUTE_PWM_FROM_TEMP: colorTemperatureMireds=" + std::to_string(colorTemperatureMireds));

    // Set the color mode to ColorTemperature so the UI displays correctly
    ColorMode::Set(endpoint, chip::app::Clusters::ColorControl::ColorModeEnum::kColorTemperatureMireds);

    // Per Matter specification, Color Control cluster commands should NOT affect OnOff state
    // Color commands only change color attributes, they don't turn lights on/off
    // The Matter stack should have already updated the ColorTemperatureMireds attribute
    // before calling this function, so we don't need to set it again
    DebugLogColorControl("COMPUTE_PWM_FROM_TEMP: Color temperature processing completed - mireds=" +
                         std::to_string(colorTemperatureMireds) + " (OnOff state unchanged per Matter spec)");

    DebugLogColorControl("COMPUTE_PWM_FROM_TEMP: Color control completed successfully");
}
