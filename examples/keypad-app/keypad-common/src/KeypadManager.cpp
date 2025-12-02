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

#include "KeypadManager.h"
#include <algorithm>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <cmath>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

// Include attribute accessors for direct cluster attribute manipulation
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

CHIP_ERROR KeypadManager::Init()
{
    if (mInitialized)
    {
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(AppServer, "KeypadManager: Initializing dual-endpoint device");

    // Initialize centralized actual light state with ColorXY coordinates
    mActualLightState.isOn         = false;
    mActualLightState.level        = 0;
    mActualLightState.colorX       = 0x616B; // Default to warm white (0.3806, 0.3769)
    mActualLightState.colorY       = 0x607D;
    mActualLightState.colorMode    = 0x01; // ColorXY mode
    mActualLightState.uiHue        = 35;   // Warm white hue
    mActualLightState.uiSaturation = 84;   // Warm white saturation

    // TODO: Initialize endpoint configurations
    // This will be done via ZAP files in a subsequent step
    // For now, we just set up the basic state management

    mInitialized = true;

    ChipLogProgress(AppServer, "KeypadManager: Initialization complete");
    ChipLogProgress(AppServer, "  - Light endpoint: %d", kLightEndpointId);
    ChipLogProgress(AppServer, "  - Switch endpoint: %d", kSwitchEndpointId);

    return CHIP_NO_ERROR;
}

void KeypadManager::Shutdown()
{
    if (!mInitialized)
    {
        return;
    }

    ChipLogProgress(AppServer, "KeypadManager: Shutting down");
    mInitialized = false;
}

void KeypadManager::SetLightOnOff(bool isOn)
{
    if (mActualLightState.isOn != isOn)
    {
        // Update the centralized actual light state
        mActualLightState.isOn = isOn;
        ChipLogProgress(AppServer, "KeypadManager: Actual Light OnOff changed to %s", isOn ? "ON" : "OFF");

        // Schedule EP1 attribute synchronization on the Matter main loop thread for thread safety
        DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                bool newValue = static_cast<bool>(context);

                // Update EP1 cluster attribute to reflect actual light state
                Protocols::InteractionModel::Status status = OnOff::Attributes::OnOff::Set(kLightEndpointId, newValue);
                if (status != Protocols::InteractionModel::Status::Success)
                {
                    ChipLogError(AppServer, "KeypadManager: Failed to sync EP1 OnOff attribute: %x", to_underlying(status));
                }
                else
                {
                    ChipLogProgress(AppServer, "KeypadManager: EP1 OnOff attribute synchronized to %s", newValue ? "ON" : "OFF");

                    // Trigger attribute change notification for proper Matter compliance
                    GetInstance().TriggerAttributeChangeNotification(kLightEndpointId, OnOff::Id, OnOff::Attributes::OnOff::Id);
                }
            },
            static_cast<intptr_t>(isOn));
    }
}

void KeypadManager::SetLightLevel(uint8_t level)
{
    if (mActualLightState.level != level)
    {
        // Update the centralized actual light state
        mActualLightState.level = level;
        ChipLogProgress(AppServer, "KeypadManager: Actual Light Level changed to %d", level);

        // Schedule EP1 attribute synchronization on the Matter main loop thread for thread safety
        DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                uint8_t newLevel = static_cast<uint8_t>(context);

                // Update EP1 cluster attribute to reflect actual light state
                DataModel::Nullable<uint8_t> nullableLevel(newLevel);
                Protocols::InteractionModel::Status status =
                    LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, nullableLevel);
                if (status != Protocols::InteractionModel::Status::Success)
                {
                    ChipLogError(AppServer, "KeypadManager: Failed to sync EP1 CurrentLevel attribute: %x", to_underlying(status));
                }
                else
                {
                    ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentLevel attribute synchronized to %d", newLevel);

                    // Trigger attribute change notification for proper Matter compliance
                    GetInstance().TriggerAttributeChangeNotification(kLightEndpointId, LevelControl::Id,
                                                                     LevelControl::Attributes::CurrentLevel::Id);
                }
            },
            static_cast<intptr_t>(level));
    }
}

void KeypadManager::SetLightColor(uint8_t hue, uint8_t saturation)
{
    // Update cached UI values and convert to ColorXY coordinates for Matter compliance
    mActualLightState.uiHue        = hue;
    mActualLightState.uiSaturation = saturation;

    uint16_t newColorX, newColorY;
    HueSaturationToColorXY(hue, saturation, newColorX, newColorY);

    bool changed = false;
    if (mActualLightState.colorX != newColorX || mActualLightState.colorY != newColorY)
    {
        mActualLightState.colorX    = newColorX;
        mActualLightState.colorY    = newColorY;
        mActualLightState.colorMode = 0x01; // ColorXY mode
        changed                     = true;
    }

    if (changed)
    {
        ChipLogProgress(AppServer, "KeypadManager: Actual Light Color changed to ColorXY X:0x%04X Y:0x%04X (from H:%d S:%d)",
                        newColorX, newColorY, hue, saturation);

        // Schedule EP1 attribute synchronization on the Matter main loop thread for thread safety
        DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                // Extract colorX and colorY from context (packed into 64-bit)
                uint64_t packedColors = static_cast<uint64_t>(context);
                uint16_t colorX       = static_cast<uint16_t>(packedColors & 0xFFFF);
                uint16_t colorY       = static_cast<uint16_t>((packedColors >> 16) & 0xFFFF);

                // Set ColorMode to ColorXY first
                Protocols::InteractionModel::Status status =
                    ColorControl::Attributes::ColorMode::Set(kLightEndpointId, static_cast<ColorControl::ColorModeEnum>(0x01));
                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(AppServer, "KeypadManager: EP1 ColorMode set to ColorXY");
                    GetInstance().TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                                     ColorControl::Attributes::ColorMode::Id);
                }

                // Set CurrentX attribute
                status = ColorControl::Attributes::CurrentX::Set(kLightEndpointId, colorX);
                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentX synchronized to 0x%04X", colorX);
                    GetInstance().TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                                     ColorControl::Attributes::CurrentX::Id);
                }

                // Set CurrentY attribute
                status = ColorControl::Attributes::CurrentY::Set(kLightEndpointId, colorY);
                if (status == Protocols::InteractionModel::Status::Success)
                {
                    ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentY synchronized to 0x%04X", colorY);
                    GetInstance().TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                                     ColorControl::Attributes::CurrentY::Id);
                }
            },
            static_cast<intptr_t>((static_cast<uint64_t>(newColorY) << 16) | newColorX));
    }
}

uint8_t KeypadManager::GetLightHue() const
{
    // Return cached UI hue to avoid conversion drift
    return mActualLightState.uiHue;
}

uint8_t KeypadManager::GetLightSaturation() const
{
    // Return cached UI saturation to avoid conversion drift
    return mActualLightState.uiSaturation;
}

void KeypadManager::UpdateActualLightHue(uint8_t hue)
{
    // Update cached UI hue and convert to ColorXY with current saturation
    mActualLightState.uiHue = hue;
    uint16_t newColorX, newColorY;
    HueSaturationToColorXY(mActualLightState.uiHue, mActualLightState.uiSaturation, newColorX, newColorY);

    mActualLightState.colorX    = newColorX;
    mActualLightState.colorY    = newColorY;
    mActualLightState.colorMode = 0x01; // ColorXY mode
}

void KeypadManager::UpdateActualLightSaturation(uint8_t saturation)
{
    // Update cached UI saturation and convert to ColorXY with current hue
    mActualLightState.uiSaturation = saturation;
    uint16_t newColorX, newColorY;
    HueSaturationToColorXY(mActualLightState.uiHue, mActualLightState.uiSaturation, newColorX, newColorY);

    mActualLightState.colorX    = newColorX;
    mActualLightState.colorY    = newColorY;
    mActualLightState.colorMode = 0x01; // ColorXY mode
}

void KeypadManager::UpdateActualLightColorXY(uint16_t colorX, uint16_t colorY)
{
    mActualLightState.colorX    = colorX;
    mActualLightState.colorY    = colorY;
    mActualLightState.colorMode = 0x01; // ColorXY mode

    // Update cached UI values by converting from ColorXY
    ColorXYToHueSaturation(colorX, colorY, mActualLightState.uiHue, mActualLightState.uiSaturation);

    ChipLogProgress(AppServer, "KeypadManager: Actual Light ColorXY changed to X:0x%04X Y:0x%04X", colorX, colorY);

    // Schedule EP1 synchronization on the Matter main loop
    DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t context) { KeypadManager::GetInstance().SynchronizeEP1ColorXYAttributes(); });
}

CHIP_ERROR KeypadManager::SendOnOffCommand(bool isOn)
{
    ChipLogProgress(AppServer, "KeypadManager: Sending OnOff command: %s", isOn ? "ON" : "OFF");

    // Implement local device control: directly update the light endpoint
    // This simulates the switch endpoint controlling the light endpoint within the same device
    SetLightOnOff(isOn);

    ChipLogProgress(AppServer, "KeypadManager: Local OnOff command completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadManager::SendLevelCommand(uint8_t level)
{
    ChipLogProgress(AppServer, "KeypadManager: Sending Level command: %d", level);

    // Implement local device control: directly update the light endpoint
    // This simulates the switch endpoint controlling the light endpoint within the same device
    SetLightLevel(level);

    ChipLogProgress(AppServer, "KeypadManager: Local Level command completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadManager::SendColorCommand(uint8_t hue, uint8_t saturation)
{
    ChipLogProgress(AppServer, "KeypadManager: Sending Color command: H:%d S:%d", hue, saturation);

    // Implement local device control: directly update the light endpoint
    // This simulates the switch endpoint controlling the light endpoint within the same device
    SetLightColor(hue, saturation);

    ChipLogProgress(AppServer, "KeypadManager: Local Color command completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadManager::SendColorXYCommand(uint16_t colorX, uint16_t colorY)
{
    ChipLogProgress(AppServer, "KeypadManager: Sending ColorXY command: X:0x%04X Y:0x%04X", colorX, colorY);

    // Implement local device control: directly update the light endpoint with ColorXY
    // This bypasses hue/saturation conversion entirely for perfect precision
    UpdateActualLightColorXY(colorX, colorY);

    ChipLogProgress(AppServer, "KeypadManager: Local ColorXY command completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadManager::SendRGBCommand(uint8_t red, uint8_t green, uint8_t blue)
{
    ChipLogProgress(AppServer, "KeypadManager: Sending RGB command: R:%d G:%d B:%d", red, green, blue);

    // Convert RGB to ColorXY using the original RGB values (not scaled)
    // This ensures color information is preserved even when level is 0
    uint16_t colorX, colorY;
    RGBToColorXY(red, green, blue, colorX, colorY);

    // Update the color coordinates
    UpdateActualLightColorXY(colorX, colorY);

    uint8_t currentLevel = mActualLightState.level;
    ChipLogProgress(AppServer, "KeypadManager: RGB command set ColorXY:X:0x%04X Y:0x%04X (Level preserved: %d)", colorX, colorY,
                    currentLevel);
    ChipLogProgress(AppServer, "KeypadManager: Local RGB command completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadManager::SendIdentifyCommand()
{
    ChipLogProgress(AppServer, "KeypadManager: Sending Identify command to EP%d", kLightEndpointId);

    // For local device identify, we trigger the identify effect on the light endpoint
    // The identify cluster server handles the actual identify behavior
    // External binding-based identify would go through BindingManager
    constexpr uint16_t kIdentifyTimeSeconds = 10;

    ChipLogProgress(AppServer, "KeypadManager: Identify command - would trigger %d seconds identify on EP%d", kIdentifyTimeSeconds,
                    kLightEndpointId);
    ChipLogProgress(AppServer, "KeypadManager: Note: For external devices, use binding cluster to configure targets");

    return CHIP_NO_ERROR;
}

void KeypadManager::TriggerAttributeChangeNotification(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
{
    // Create a concrete attribute path for the changed attribute
    ConcreteAttributePath attributePath(endpointId, clusterId, attributeId);

    // Trigger attribute reporting to notify subscribed controllers
    // This ensures that external Matter controllers receive attribute update notifications
    MatterReportingAttributeChangeCallback(attributePath);

    ChipLogProgress(AppServer,
                    "KeypadManager: Triggered attribute change notification for Endpoint=%d, Cluster=0x%08lX, Attribute=0x%08lX",
                    endpointId, static_cast<unsigned long>(clusterId), static_cast<unsigned long>(attributeId));
}

void KeypadManager::UpdateActualLightStateAndSyncEP1(bool isOn, uint8_t level, uint8_t hue, uint8_t saturation)
{
    bool stateChanged = false;

    // Update actual light state
    if (mActualLightState.isOn != isOn)
    {
        mActualLightState.isOn = isOn;
        stateChanged           = true;
    }
    if (mActualLightState.level != level)
    {
        mActualLightState.level = level;
        stateChanged            = true;
    }

    // Update cached UI values and convert hue/saturation to ColorXY for color state updates
    if (mActualLightState.uiHue != hue || mActualLightState.uiSaturation != saturation)
    {
        mActualLightState.uiHue        = hue;
        mActualLightState.uiSaturation = saturation;

        uint16_t newColorX, newColorY;
        HueSaturationToColorXY(hue, saturation, newColorX, newColorY);
        mActualLightState.colorX    = newColorX;
        mActualLightState.colorY    = newColorY;
        mActualLightState.colorMode = 0x01; // ColorXY mode
        stateChanged                = true;
    }

    if (stateChanged)
    {
        ChipLogProgress(AppServer,
                        "KeypadManager: Actual Light State updated - OnOff:%s Level:%d ColorXY X:0x%04X Y:0x%04X (from H:%d S:%d)",
                        isOn ? "ON" : "OFF", level, mActualLightState.colorX, mActualLightState.colorY, hue, saturation);

        // Synchronize EP1 attributes with the new actual state
        SynchronizeEP1WithActualState();
    }
}

void KeypadManager::SynchronizeEP1WithActualState()
{
    ChipLogProgress(AppServer, "KeypadManager: Synchronizing EP1 attributes with actual light state");

    // Schedule all EP1 attribute updates on the Matter main loop thread for thread safety
    DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t context) {
            KeypadManager & manager        = GetInstance();
            const ActualLightState & state = manager.mActualLightState;

            // Update OnOff attribute
            Protocols::InteractionModel::Status status = OnOff::Attributes::OnOff::Set(kLightEndpointId, state.isOn);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 OnOff synchronized to %s", state.isOn ? "ON" : "OFF");
                manager.TriggerAttributeChangeNotification(kLightEndpointId, OnOff::Id, OnOff::Attributes::OnOff::Id);
            }

            // Update Level attribute
            DataModel::Nullable<uint8_t> nullableLevel(state.level);
            status = LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, nullableLevel);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 Level synchronized to %d", state.level);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, LevelControl::Id,
                                                           LevelControl::Attributes::CurrentLevel::Id);
            }

            // Set ColorMode to ColorXY
            status = ColorControl::Attributes::ColorMode::Set(kLightEndpointId,
                                                              static_cast<ColorControl::ColorModeEnum>(state.colorMode));
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 ColorMode synchronized to %d", state.colorMode);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::ColorMode::Id);
            }

            // Update CurrentX attribute
            status = ColorControl::Attributes::CurrentX::Set(kLightEndpointId, state.colorX);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentX synchronized to 0x%04X", state.colorX);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::CurrentX::Id);
            }

            // Update CurrentY attribute
            status = ColorControl::Attributes::CurrentY::Set(kLightEndpointId, state.colorY);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentY synchronized to 0x%04X", state.colorY);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::CurrentY::Id);
            }
        },
        0);
}

void KeypadManager::SynchronizeEP1ColorXYAttributes()
{
    ChipLogProgress(AppServer, "KeypadManager: Synchronizing EP1 ColorXY attributes with actual light state");

    // Schedule ColorXY attribute updates on the Matter main loop thread for thread safety
    DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t context) {
            KeypadManager & manager        = GetInstance();
            const ActualLightState & state = manager.mActualLightState;

            // Set ColorMode to ColorXY
            Protocols::InteractionModel::Status status = ColorControl::Attributes::ColorMode::Set(
                kLightEndpointId, static_cast<ColorControl::ColorModeEnum>(state.colorMode));
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 ColorMode set to ColorXY");
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::ColorMode::Id);
            }

            // Update CurrentX attribute
            status = ColorControl::Attributes::CurrentX::Set(kLightEndpointId, state.colorX);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentX synchronized to 0x%04X", state.colorX);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::CurrentX::Id);
            }

            // Update CurrentY attribute
            status = ColorControl::Attributes::CurrentY::Set(kLightEndpointId, state.colorY);
            if (status == Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(AppServer, "KeypadManager: EP1 CurrentY synchronized to 0x%04X", state.colorY);
                manager.TriggerAttributeChangeNotification(kLightEndpointId, ColorControl::Id,
                                                           ColorControl::Attributes::CurrentY::Id);
            }
        },
        0);
}

// Color conversion functions for Matter ColorXY compliance

void KeypadManager::HueSaturationToColorXY(uint8_t hue, uint8_t saturation, uint16_t & colorX, uint16_t & colorY)
{
    // Convert Matter hue (0-254) and saturation (0-254) to CIE 1931 ColorXY coordinates
    // Matter hue: 0-254 maps to 0-360 degrees
    // Matter saturation: 0-254 maps to 0-100%

    // Convert to floating point for calculations
    float h = (static_cast<float>(hue) * 360.0f) / 254.0f; // Convert to degrees
    float s = static_cast<float>(saturation) / 254.0f;     // Convert to 0-1 range
    float v = 1.0f;                                        // Assume full brightness for color conversion

    // Convert HSV to RGB
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r, g, b;
    if (h >= 0 && h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h >= 60 && h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h >= 120 && h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h >= 180 && h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h >= 240 && h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    r += m;
    g += m;
    b += m;

    // Convert RGB to CIE XYZ using sRGB color space
    // Apply gamma correction (sRGB)
    auto gammaCorrect = [](float val) -> float { return (val <= 0.04045f) ? (val / 12.92f) : powf((val + 0.055f) / 1.055f, 2.4f); };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // sRGB to XYZ transformation matrix
    float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
    float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
    float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

    // Convert XYZ to xy chromaticity coordinates
    float sum = X + Y + Z;
    if (sum > 0.0f)
    {
        float x_coord = X / sum;
        float y_coord = Y / sum;

        // Convert to Matter ColorXY format (16-bit fixed point, 0-65535 maps to 0.0-1.0)
        colorX = static_cast<uint16_t>(x_coord * 65535.0f);
        colorY = static_cast<uint16_t>(y_coord * 65535.0f);
    }
    else
    {
        // Default to warm white if calculation fails
        colorX = 0x616B; // ~0.3806
        colorY = 0x607D; // ~0.3769
    }
}

void KeypadManager::ColorXYToHueSaturation(uint16_t colorX, uint16_t colorY, uint8_t & hue, uint8_t & saturation)
{
    // Convert Matter ColorXY coordinates to hue/saturation for UI display
    // This is an approximation for UI purposes

    // Convert from Matter format to floating point
    float x = static_cast<float>(colorX) / 65535.0f;
    float y = static_cast<float>(colorY) / 65535.0f;
    float z = 1.0f - x - y;

    // Convert xy to XYZ (assuming Y=1 for maximum brightness)
    float Y = 1.0f;
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    // XYZ to sRGB transformation matrix
    float r = 3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b = 0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;

    // Apply inverse gamma correction
    auto invGammaCorrect = [](float val) -> float {
        return (val <= 0.0031308f) ? (val * 12.92f) : (1.055f * powf(val, 1.0f / 2.4f) - 0.055f);
    };

    r = invGammaCorrect(r);
    g = invGammaCorrect(g);
    b = invGammaCorrect(b);

    // Clamp to valid range
    r = fmaxf(0.0f, fminf(1.0f, r));
    g = fmaxf(0.0f, fminf(1.0f, g));
    b = fmaxf(0.0f, fminf(1.0f, b));

    // Convert RGB to HSV
    float max_val = fmaxf(r, fmaxf(g, b));
    float min_val = fminf(r, fminf(g, b));
    float delta   = max_val - min_val;

    float h_deg = 0.0f;
    float s_val = (max_val > 0.0f) ? (delta / max_val) : 0.0f;

    if (delta > 0.0f)
    {
        if (max_val == r)
        {
            h_deg = 60.0f * fmodf((g - b) / delta, 6.0f);
        }
        else if (max_val == g)
        {
            h_deg = 60.0f * ((b - r) / delta + 2.0f);
        }
        else
        {
            h_deg = 60.0f * ((r - g) / delta + 4.0f);
        }
    }

    if (h_deg < 0.0f)
        h_deg += 360.0f;

    // Convert to Matter format
    hue        = static_cast<uint8_t>((h_deg * 254.0f) / 360.0f);
    saturation = static_cast<uint8_t>(s_val * 254.0f);
}

void KeypadManager::RGBToColorXY(uint8_t red, uint8_t green, uint8_t blue, uint16_t & colorX, uint16_t & colorY)
{
    // Convert RGB (0-255) to ColorXY using symmetric CIE 1931 transformation
    // This is the inverse of the ColorXYToRGB function for perfect symmetry

    // Convert RGB to floating point (0.0-1.0)
    float r = static_cast<float>(red) / 255.0f;
    float g = static_cast<float>(green) / 255.0f;
    float b = static_cast<float>(blue) / 255.0f;

    // Apply gamma correction (inverse of gamma 2.2)
    auto gammaCorrect = [](float val) -> float { return (val <= 0.04045f) ? (val / 12.92f) : powf((val + 0.055f) / 1.055f, 2.4f); };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // sRGB to XYZ transformation matrix (D65 illuminant)
    // This is the inverse of the matrix used in ColorXYToRGB
    float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
    float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
    float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

    // Convert XYZ to xy chromaticity coordinates
    float sum = X + Y + Z;
    float x, y;

    if (sum > 0.0f)
    {
        x = X / sum;
        y = Y / sum;
    }
    else
    {
        // Default to warm white if sum is zero
        x = 0.3127f; // D65 white point
        y = 0.3290f;
    }

    // Clamp to valid CIE 1931 gamut
    x = fmaxf(0.0f, fminf(1.0f, x));
    y = fmaxf(0.0f, fminf(1.0f, y));

    // Convert to Matter format (0-65535)
    colorX = static_cast<uint16_t>(x * 65535.0f);
    colorY = static_cast<uint16_t>(y * 65535.0f);
}

void KeypadManager::ColorXYToRGB(uint16_t colorX, uint16_t colorY, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    // Convert ColorXY to RGB using symmetric CIE 1931 transformation
    // This matches the algorithm used in ConnectedHomeIP lighting apps

    // Convert from Matter format to floating point
    float x = static_cast<float>(colorX) / 65535.0f;
    float y = static_cast<float>(colorY) / 65535.0f;
    float z = 1.0f - x - y;

    // Convert xy to XYZ (assuming Y=1 for maximum brightness)
    float Y = 1.0f;
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    // XYZ to sRGB transformation matrix (D65 illuminant)
    // These coefficients match the ConnectedHomeIP lighting apps
    float r = 3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b = 0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;

    // Apply gamma 2.2 correction
    auto gammaCorrect = [](float val) -> float {
        return (val <= 0.0031308f) ? (val * 12.92f) : (1.055f * powf(val, 1.0f / 2.4f) - 0.055f);
    };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // Clamp to valid range
    r = fmaxf(0.0f, fminf(1.0f, r));
    g = fmaxf(0.0f, fminf(1.0f, g));
    b = fmaxf(0.0f, fminf(1.0f, b));

    // Convert to RGB (0-255)
    red   = static_cast<uint8_t>(r * 255.0f);
    green = static_cast<uint8_t>(g * 255.0f);
    blue  = static_cast<uint8_t>(b * 255.0f);
}

void KeypadManager::SetCurrentMode(uint8_t mode)
{
    if (mCurrentMode != mode)
    {
        mCurrentMode = mode;
        ChipLogProgress(AppServer, "KeypadManager: Current mode changed to %d", mode);
    }
}

void KeypadManager::HandleModeChange(uint8_t newMode)
{
    ChipLogProgress(AppServer, "KeypadManager: Handling mode change to %d", newMode);

    // Update internal state
    SetCurrentMode(newMode);

    // Log the mode change for verification
    const char * modeNames[] = { "None", "Light", "Light Group", "Button" };
    const char * modeName    = (newMode < 4) ? modeNames[newMode] : "Unknown";

    ChipLogProgress(AppServer, "KeypadManager: Mode changed to %d (%s)", newMode, modeName);

    // TODO: In future implementation, this is where we would change the behavior
    // of how EP2 commands are processed based on the selected mode:
    // - Mode 0 (None): No operation
    // - Mode 1 (Light): Control single light device (current behavior)
    // - Mode 2 (Light Group): Control group of lights (future enhancement)
    // - Mode 3 (Button): Simple button press mode (future enhancement)

    // For now, we just log the mode change as this is framework setup only
}
