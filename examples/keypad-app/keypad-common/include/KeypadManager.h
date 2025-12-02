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

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

/**
 * @brief Keypad Manager
 *
 * This class manages the dual-endpoint keypad device functionality:
 * - Endpoint 1: Light device (server clusters)
 * - Endpoint 2: Light switch device (client clusters)
 */
class KeypadManager
{
public:
    static KeypadManager & GetInstance()
    {
        static KeypadManager sInstance;
        return sInstance;
    }

    /**
     * @brief Initialize the keypad manager
     *
     * Sets up both light and switch endpoints with their respective clusters
     */
    CHIP_ERROR Init();

    /**
     * @brief Shutdown the keypad manager
     */
    void Shutdown();

    // Light endpoint (Endpoint 1) functionality
    /**
     * @brief Set the light on/off state
     */
    void SetLightOnOff(bool isOn);

    /**
     * @brief Get the actual light on/off state
     */
    bool GetLightOnOff() const { return mActualLightState.isOn; }

    /**
     * @brief Set the light level (brightness) - updates actual state and EP1 attributes
     */
    void SetLightLevel(uint8_t level);

    /**
     * @brief Get the actual light level
     */
    uint8_t GetLightLevel() const { return mActualLightState.level; }

    /**
     * @brief Set the light color (hue/saturation) - converts to ColorXY and updates actual state and EP1 attributes
     */
    void SetLightColor(uint8_t hue, uint8_t saturation);

    /**
     * @brief Get the actual light hue (converted from ColorXY for UI compatibility)
     */
    uint8_t GetLightHue() const;

    /**
     * @brief Get the actual light saturation (converted from ColorXY for UI compatibility)
     */
    uint8_t GetLightSaturation() const;

    /**
     * @brief Get the actual light ColorX coordinate
     */
    uint16_t GetLightColorX() const { return mActualLightState.colorX; }

    /**
     * @brief Get the actual light ColorY coordinate
     */
    uint16_t GetLightColorY() const { return mActualLightState.colorY; }

    /**
     * @brief Update actual light hue state (for external Matter commands) - converts to ColorXY
     */
    void UpdateActualLightHue(uint8_t hue);

    /**
     * @brief Update actual light saturation state (for external Matter commands) - converts to ColorXY
     */
    void UpdateActualLightSaturation(uint8_t saturation);

    /**
     * @brief Update actual light ColorXY state (for external Matter commands)
     */
    void UpdateActualLightColorXY(uint16_t colorX, uint16_t colorY);

    /**
     * @brief Update actual light OnOff state (for external Matter commands)
     */
    void UpdateActualLightOnOff(bool isOn) { mActualLightState.isOn = isOn; }

    /**
     * @brief Update actual light level state (for external Matter commands)
     */
    void UpdateActualLightLevel(uint8_t level) { mActualLightState.level = level; }

    /**
     * @brief Update actual light state and synchronize EP1 attributes
     * This is the core method for the three-layer architecture
     */
    void UpdateActualLightStateAndSyncEP1(bool isOn, uint8_t level, uint8_t hue, uint8_t saturation);

    /**
     * @brief Synchronize EP1 attributes with current actual light state
     * Updates Matter cluster attributes to reflect the actual light state
     */
    void SynchronizeEP1WithActualState();

    /**
     * @brief Synchronize EP1 ColorXY attributes with current actual light state
     * Updates Matter ColorXY cluster attributes to reflect the actual ColorXY state
     */
    void SynchronizeEP1ColorXYAttributes();

    // Switch endpoint (Endpoint 2) functionality
    /**
     * @brief Send on/off command to bound devices
     */
    CHIP_ERROR SendOnOffCommand(bool isOn);

    /**
     * @brief Send level command to bound devices
     */
    CHIP_ERROR SendLevelCommand(uint8_t level);

    /**
     * @brief Send color command to bound devices
     */
    CHIP_ERROR SendColorCommand(uint8_t hue, uint8_t saturation);

    /**
     * @brief Send ColorXY command to bound devices (direct ColorXY control)
     */
    CHIP_ERROR SendColorXYCommand(uint16_t colorX, uint16_t colorY);

    /**
     * @brief Send RGB command to bound devices (converts RGB to ColorXY)
     */
    CHIP_ERROR SendRGBCommand(uint8_t red, uint8_t green, uint8_t blue);

    /**
     * @brief Send identify command to bound devices
     */
    CHIP_ERROR SendIdentifyCommand();

    // Endpoint definitions
    static constexpr chip::EndpointId kLightEndpointId  = 1;
    static constexpr chip::EndpointId kSwitchEndpointId = 2;

    /**
     * @brief Convert RGB to ColorXY using symmetric CIE 1931 transformation
     * Uses the inverse of the standard sRGB to XYZ transformation matrix
     */
    static void RGBToColorXY(uint8_t red, uint8_t green, uint8_t blue, uint16_t & colorX, uint16_t & colorY);

    /**
     * @brief Convert ColorXY to RGB using symmetric CIE 1931 transformation
     * Uses the standard sRGB transformation matrix (same as lighting apps)
     */
    static void ColorXYToRGB(uint16_t colorX, uint16_t colorY, uint8_t & red, uint8_t & green, uint8_t & blue);

    // ModeSelect cluster management
    uint8_t GetCurrentMode() const { return mCurrentMode; }
    void SetCurrentMode(uint8_t mode);
    void HandleModeChange(uint8_t newMode);

private:
    KeypadManager() = default;

    /**
     * @brief Trigger attribute change notification for proper Matter compliance
     *
     * This ensures that attribute changes are properly reported to subscribed controllers
     * and that the MatterPostAttributeChangeCallback is triggered for UI updates
     */
    void TriggerAttributeChangeNotification(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId);

    /**
     * @brief Centralized actual light state - represents the true device state
     * This is the single source of truth that both EP1 and EP2 reference
     * Color is stored in ColorXY coordinates as per Matter specification for EP2 operation
     */
    struct ActualLightState
    {
        bool isOn         = false;
        uint8_t level     = 0;
        uint16_t colorX   = 0x616B; // Default to warm white (0.3806, 0.3769)
        uint16_t colorY   = 0x607D;
        uint8_t colorMode = 0x01; // ColorXY mode (ColorModeEnum::kCurrentXAndCurrentY)
        // UI-level hue/saturation cache to avoid conversion drift
        uint8_t uiHue        = 35; // Warm white hue
        uint8_t uiSaturation = 84; // Warm white saturation
    };

    /**
     * @brief Convert hue/saturation to ColorXY coordinates
     * Uses CIE 1931 color space conversion for Matter compliance
     */
    static void HueSaturationToColorXY(uint8_t hue, uint8_t saturation, uint16_t & colorX, uint16_t & colorY);

    /**
     * @brief Convert ColorXY coordinates to hue/saturation
     * For UI compatibility when displaying ColorXY values as hue/saturation
     */
    static void ColorXYToHueSaturation(uint16_t colorX, uint16_t colorY, uint8_t & hue, uint8_t & saturation);

private:
    ActualLightState mActualLightState;
    bool mInitialized = false;

    // ModeSelect state
    uint8_t mCurrentMode = 0; // Default to "None" mode
};
