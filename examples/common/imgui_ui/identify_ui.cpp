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
#include "identify_ui.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/identify-server/identify-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

#include <imgui.h>
#include <iomanip>
#include <sstream>

namespace example {
namespace Ui {

IdentifyDialog::IdentifyDialog()
{
    ChipLogProgress(AppServer, "IdentifyDialog created");
}

void IdentifyDialog::Show(chip::EndpointId endpoint)
{
    mVisibleDialogs[endpoint] = true;
    // Initialize with default values - actual time will be read in Update() method
    mLastIdentifyTimes[endpoint]    = 10; // Default 10 seconds, will be updated
    mInitialIdentifyTimes[endpoint] = 0;  // Will be set on first Update() call
    ChipLogProgress(AppServer, "Showing identify dialog for endpoint %d", endpoint);
}

void IdentifyDialog::Hide(chip::EndpointId endpoint)
{
    mVisibleDialogs.erase(endpoint);
    mLastIdentifyTimes.erase(endpoint);
    mInitialIdentifyTimes.erase(endpoint);
    ChipLogProgress(AppServer, "Hiding identify dialog for endpoint %d", endpoint);
}

void IdentifyDialog::Update()
{
    // Create a copy of visible dialogs to avoid iterator corruption
    // from concurrent modifications by callbacks
    std::map<chip::EndpointId, bool> visibleDialogsCopy;
    std::map<chip::EndpointId, uint16_t> cachedTimesCopy;

    // Copy the maps to avoid race conditions
    visibleDialogsCopy = mVisibleDialogs;
    cachedTimesCopy    = mLastIdentifyTimes;

    // Update cached time values for visible dialogs
    for (const auto & pair : visibleDialogsCopy)
    {
        chip::EndpointId endpoint = pair.first;
        bool isVisible            = pair.second;

        if (isVisible)
        {
            // Safely read current identify time and update cache
            uint16_t currentTime         = GetCurrentIdentifyTime(endpoint);
            mLastIdentifyTimes[endpoint] = currentTime;

            // Set initial time on first update (when it's still 0)
            if (mInitialIdentifyTimes[endpoint] == 0 && currentTime > 0)
            {
                mInitialIdentifyTimes[endpoint] = currentTime;
                ChipLogProgress(AppServer, "Set initial identify time for endpoint %d to %d", endpoint, currentTime);
            }

            // Use the updated time for rendering
            RenderDialog(endpoint, currentTime);

            // Auto-hide if identify time reached zero
            if (currentTime == 0)
            {
                ChipLogProgress(AppServer, "Auto-hiding identify dialog for endpoint %d (time expired)", endpoint);
                Hide(endpoint);
            }
        }
    }
}

bool IdentifyDialog::IsVisible() const
{
    for (const auto & pair : mVisibleDialogs)
    {
        if (pair.second)
        {
            return true;
        }
    }
    return false;
}

void IdentifyDialog::HideAll()
{
    mVisibleDialogs.clear();
    mLastIdentifyTimes.clear();
    mInitialIdentifyTimes.clear();
    ChipLogProgress(AppServer, "All identify dialogs hidden");
}

void IdentifyDialog::RenderDialog(chip::EndpointId endpoint, uint16_t identifyTime)
{
    // Create a unique popup ID for this endpoint
    std::string popupId = "Identify Device##" + std::to_string(endpoint);

    // Set popup to appear in center of screen
    ImGuiIO & io  = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(DIALOG_WIDTH, DIALOG_HEIGHT), ImGuiCond_Appearing);

    // Open popup if not already open
    if (!ImGui::IsPopupOpen(popupId.c_str()))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    // Render the popup
    if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // Set focus to this window to bring it to the front
        ImGui::SetWindowFocus();
        // Header with endpoint info
        ImGui::Text("Device Identification Active");
        ImGui::Separator();

        // Endpoint information
        std::string endpointName = GetEndpointName(endpoint);
        ImGui::Text("Endpoint: %s", endpointName.c_str());

        ImGui::Spacing();

        // Countdown timer with visual emphasis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(IDENTIFY_COLOR_R, IDENTIFY_COLOR_G, IDENTIFY_COLOR_B, IDENTIFY_COLOR_A));
        std::string timeStr = FormatTime(identifyTime);
        ImGui::Text("Time remaining: %s", timeStr.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Progress bar showing time remaining based on initial time
        float progress = 0.0f;
        if (mInitialIdentifyTimes.find(endpoint) != mInitialIdentifyTimes.end())
        {
            uint16_t initialTime = mInitialIdentifyTimes[endpoint];
            if (initialTime > 0)
            {
                // Progress: 1.0 (full) when identifyTime == initialTime, 0.0 (empty) when identifyTime == 0
                progress = static_cast<float>(identifyTime) / static_cast<float>(initialTime);
            }
        }

        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");

        ImGui::Spacing();

        // Manual dismiss button
        if (ImGui::Button("Dismiss", ImVec2(120, 0)))
        {
            DismissDialog(endpoint);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Info text
        ImGui::Text("(Auto-dismisses when timer expires)");

        ImGui::EndPopup();
    }
}

uint16_t IdentifyDialog::GetCurrentIdentifyTime(chip::EndpointId endpoint)
{
    uint16_t identifyTime = 0;

    // Acquire CHIP stack lock before reading attributes
    chip::DeviceLayer::StackLock lock;

    chip::Protocols::InteractionModel::Status status =
        chip::app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);

    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        return 0;
    }

    return identifyTime;
}

std::string IdentifyDialog::FormatTime(uint16_t seconds)
{
    if (seconds == 0)
    {
        return "0:00";
    }

    uint16_t minutes          = seconds / 60;
    uint16_t remainingSeconds = seconds % 60;

    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << remainingSeconds;
    return oss.str();
}

// Static function to handle the identify stop work on the CHIP thread
static void StopIdentifyWork(intptr_t endpointArg)
{
    chip::EndpointId endpoint = static_cast<chip::EndpointId>(endpointArg);

    // No need for StackLock here - ScheduleWork ensures we're on the CHIP thread
    chip::Protocols::InteractionModel::Status status = chip::app::Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, 0);

    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(AppServer, "Failed to set IdentifyTime to 0 for endpoint %d: %d", endpoint, static_cast<int>(status));
    }
    else
    {
        ChipLogProgress(AppServer, "Successfully stopped identify process for endpoint %d", endpoint);
    }
}

void IdentifyDialog::DismissDialog(chip::EndpointId endpoint)
{
    ChipLogProgress(AppServer, "Manually dismissing identify dialog for endpoint %d", endpoint);

    // Schedule the attribute write on the CHIP main thread to avoid thread safety issues
    chip::DeviceLayer::PlatformMgr().ScheduleWork(StopIdentifyWork, static_cast<intptr_t>(endpoint));

    // Remove from visible dialogs immediately (this is safe from UI thread)
    mVisibleDialogs.erase(endpoint);
    mLastIdentifyTimes.erase(endpoint);
}

std::string IdentifyDialog::GetEndpointName(chip::EndpointId endpoint)
{
    switch (endpoint)
    {
    case 0:
        return "Root Device (EP0)";
    case 1:
        return "Primary Device (EP1)";
    default:
        return "Endpoint " + std::to_string(endpoint);
    }
}

} // namespace Ui
} // namespace example
