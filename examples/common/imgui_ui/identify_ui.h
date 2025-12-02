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

#include <imgui.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace example {
namespace Ui {

/**
 * ImGui-based identify dialog that displays when devices are being identified.
 *
 * This class provides a visual popup dialog that shows:
 * - Which endpoint is being identified
 * - Countdown timer showing remaining identify time
 * - Auto-dismissal when identify time reaches zero
 * - Manual dismissal option
 *
 * The dialog is designed to be non-intrusive and automatically manages
 * its visibility based on identify state.
 */
class IdentifyDialog
{
public:
    IdentifyDialog();
    ~IdentifyDialog() = default;

    /**
     * Show the identify dialog for a specific endpoint.
     *
     * @param endpoint The endpoint that is being identified
     */
    void Show(chip::EndpointId endpoint);

    /**
     * Hide the identify dialog for a specific endpoint.
     *
     * @param endpoint The endpoint that stopped being identified
     */
    void Hide(chip::EndpointId endpoint);

    /**
     * Update the dialog state and render if visible.
     * This should be called every frame from the main UI loop.
     */
    void Update();

    /**
     * Check if the dialog is currently visible for any endpoint.
     *
     * @return true if dialog is visible, false otherwise
     */
    bool IsVisible() const;

    /**
     * Force hide all identify dialogs.
     * Useful for cleanup or emergency situations.
     */
    void HideAll();

private:
    /**
     * Render the identify dialog popup.
     *
     * @param endpoint The endpoint being identified
     * @param identifyTime Current identify time remaining
     */
    void RenderDialog(chip::EndpointId endpoint, uint16_t identifyTime);

    /**
     * Get the current identify time for an endpoint.
     *
     * @param endpoint The endpoint to query
     * @return Current identify time in seconds
     */
    uint16_t GetCurrentIdentifyTime(chip::EndpointId endpoint);

    /**
     * Manually dismiss the identify dialog for an endpoint.
     * This will stop the identify process by setting IdentifyTime to 0.
     *
     * @param endpoint The endpoint to stop identifying
     */
    void DismissDialog(chip::EndpointId endpoint);

    /**
     * Format time for display (e.g., "1:23" for 83 seconds).
     *
     * @param seconds Time in seconds
     * @return Formatted time string
     */
    std::string FormatTime(uint16_t seconds);

    /**
     * Get a user-friendly name for an endpoint.
     *
     * @param endpoint The endpoint ID
     * @return Human-readable endpoint name
     */
    std::string GetEndpointName(chip::EndpointId endpoint);

    // Track which endpoints are currently showing dialogs
    std::map<chip::EndpointId, bool> mVisibleDialogs;

    // Track last known identify times to detect changes
    std::map<chip::EndpointId, uint16_t> mLastIdentifyTimes;

    // Track initial identify times for progress calculation
    std::map<chip::EndpointId, uint16_t> mInitialIdentifyTimes;

    // Dialog styling
    static constexpr float DIALOG_WIDTH  = 300.0f;
    static constexpr float DIALOG_HEIGHT = 150.0f;

    // Colors for visual feedback
    static constexpr float IDENTIFY_COLOR_R = 0.2f;
    static constexpr float IDENTIFY_COLOR_G = 0.8f;
    static constexpr float IDENTIFY_COLOR_B = 0.2f;
    static constexpr float IDENTIFY_COLOR_A = 1.0f;
};

} // namespace Ui
} // namespace example
