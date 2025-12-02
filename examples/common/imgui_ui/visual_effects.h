/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace example {
namespace Ui {

/**
 * Visual effects system for identify functionality.
 *
 * This class provides reactive monitoring of IdentifyTime attributes
 * and coordinates visual effects during device identification.
 * It works in conjunction with IdentifyManager to provide a complete
 * identify experience.
 */
class VisualEffects
{
public:
    /**
     * Effect types that can be applied during identification.
     */
    enum class EffectType
    {
        None,
        Blink,
        Breathe,
        Okay,
        ChannelChange,
        FinishEffect,
        StopEffect
    };

    /**
     * Structure to track identify state for an endpoint.
     */
    struct IdentifyState
    {
        uint16_t identifyTime    = 0;
        EffectType currentEffect = EffectType::None;
        std::chrono::steady_clock::time_point lastUpdate;
        bool isActive = false;
    };

    VisualEffects();
    ~VisualEffects() = default;

    /**
     * Update visual effects for all monitored endpoints.
     * This should be called regularly from the main UI loop.
     */
    void Update();

    /**
     * Start monitoring an endpoint for identify changes.
     *
     * @param endpoint The endpoint to monitor
     */
    void StartMonitoring(chip::EndpointId endpoint);

    /**
     * Stop monitoring an endpoint.
     *
     * @param endpoint The endpoint to stop monitoring
     */
    void StopMonitoring(chip::EndpointId endpoint);

    /**
     * Get the current identify time for an endpoint.
     *
     * @param endpoint The endpoint to query
     * @return Current identify time in seconds
     */
    uint16_t GetCurrentIdentifyTime(chip::EndpointId endpoint);

    /**
     * Check if an endpoint is currently identifying.
     *
     * @param endpoint The endpoint to check
     * @return true if identifying, false otherwise
     */
    bool IsIdentifying(chip::EndpointId endpoint);

    /**
     * Get the current effect type for an endpoint.
     *
     * @param endpoint The endpoint to query
     * @return Current effect type
     */
    EffectType GetCurrentEffect(chip::EndpointId endpoint);

    /**
     * Set the effect type for an endpoint.
     *
     * @param endpoint The endpoint to set effect for
     * @param effect The effect type to set
     */
    void SetEffect(chip::EndpointId endpoint, EffectType effect);

    /**
     * Get the identify state for an endpoint.
     *
     * @param endpoint The endpoint to query
     * @return Identify state structure
     */
    const IdentifyState * GetIdentifyState(chip::EndpointId endpoint) const;

    /**
     * Clear all monitored endpoints and reset state.
     */
    void Reset();

private:
    /**
     * Update the identify state for a specific endpoint.
     *
     * @param endpoint The endpoint to update
     */
    void UpdateEndpointState(chip::EndpointId endpoint);

    /**
     * Handle identify time change for an endpoint.
     *
     * @param endpoint The endpoint that changed
     * @param oldTime Previous identify time
     * @param newTime New identify time
     */
    void HandleIdentifyTimeChange(chip::EndpointId endpoint, uint16_t oldTime, uint16_t newTime);

    /**
     * Apply visual effect for an endpoint.
     *
     * @param endpoint The endpoint to apply effect to
     * @param effect The effect to apply
     */
    void ApplyEffect(chip::EndpointId endpoint, EffectType effect);

    /**
     * Convert Matter effect identifier to internal effect type.
     *
     * @param effectId Matter effect identifier
     * @return Internal effect type
     */
    EffectType ConvertMatterEffect(uint8_t effectId);

    // Map of endpoint ID to identify state
    std::map<chip::EndpointId, IdentifyState> mEndpointStates;

    // Update interval for checking identify time changes
    static constexpr std::chrono::milliseconds UPDATE_INTERVAL{ 100 };
    std::chrono::steady_clock::time_point mLastGlobalUpdate;
};

} // namespace Ui
} // namespace example
