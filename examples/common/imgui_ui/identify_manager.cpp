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
#include "identify_manager.h"
#include "identify_ui.h"
#include "ui.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>

namespace example {
namespace Ui {

IdentifyManager & IdentifyManager::GetInstance()
{
    static IdentifyManager instance;
    return instance;
}

void IdentifyManager::Initialize(ImguiUi * ui)
{
    if (mInitialized)
    {
        ChipLogProgress(AppServer, "IdentifyManager already initialized");
        return;
    }

    mUi = ui;

    // Create the identify dialog if we have a UI
    if (mUi != nullptr)
    {
        mIdentifyDialog = std::make_unique<IdentifyDialog>();
        ChipLogProgress(AppServer, "IdentifyManager initialized with UI support");
    }
    else
    {
        ChipLogProgress(AppServer, "IdentifyManager initialized in headless mode");
    }

    mInitialized = true;
}

void IdentifyManager::UpdateDialogs()
{
    if (mIdentifyDialog != nullptr)
    {
        mIdentifyDialog->Update();
    }
}

void IdentifyManager::Shutdown()
{
    if (!mInitialized)
    {
        return;
    }

    // Clear registered clusters
    mRegisteredClusters.clear();

    // Reset dialog
    mIdentifyDialog.reset();

    mUi          = nullptr;
    mInitialized = false;

    ChipLogProgress(AppServer, "IdentifyManager shutdown complete");
}

void IdentifyManager::RegisterIdentifyCluster(::Identify * identify)
{
    if (identify == nullptr)
    {
        ChipLogError(AppServer, "Cannot register null identify cluster");
        return;
    }

    // Check if already registered
    auto it = std::find(mRegisteredClusters.begin(), mRegisteredClusters.end(), identify);
    if (it != mRegisteredClusters.end())
    {
        ChipLogProgress(AppServer, "Identify cluster for endpoint %d already registered", identify->mEndpoint);
        return;
    }

    mRegisteredClusters.push_back(identify);
    ChipLogProgress(AppServer, "Registered identify cluster for endpoint %d", identify->mEndpoint);
}

void IdentifyManager::UnregisterIdentifyCluster(::Identify * identify)
{
    if (identify == nullptr)
    {
        return;
    }

    auto it = std::find(mRegisteredClusters.begin(), mRegisteredClusters.end(), identify);
    if (it != mRegisteredClusters.end())
    {
        mRegisteredClusters.erase(it);
        ChipLogProgress(AppServer, "Unregistered identify cluster for endpoint %d", identify->mEndpoint);
    }
}

void IdentifyManager::OnIdentifyStart(::Identify * identify)
{
    GetInstance().HandleIdentifyStart(identify);
}

void IdentifyManager::OnIdentifyStop(::Identify * identify)
{
    GetInstance().HandleIdentifyStop(identify);
}

void IdentifyManager::OnTriggerEffect(::Identify * identify)
{
    GetInstance().HandleTriggerEffect(identify);
}

void IdentifyManager::HandleIdentifyStart(::Identify * identify)
{
    if (identify == nullptr)
    {
        return;
    }

    ChipLogProgress(AppServer, "Identify started on endpoint %d", identify->mEndpoint);

    // Show identify dialog if UI is available
    if (mUi != nullptr && mIdentifyDialog != nullptr)
    {
        ShowIdentifyDialog(identify->mEndpoint);
    }
}

void IdentifyManager::HandleIdentifyStop(::Identify * identify)
{
    if (identify == nullptr)
    {
        return;
    }

    ChipLogProgress(AppServer, "Identify stopped on endpoint %d", identify->mEndpoint);

    // Hide identify dialog if UI is available
    if (mUi != nullptr && mIdentifyDialog != nullptr)
    {
        HideIdentifyDialog(identify->mEndpoint);
    }
}

void IdentifyManager::HandleTriggerEffect(::Identify * identify)
{
    if (identify == nullptr)
    {
        return;
    }

    ChipLogProgress(AppServer, "Identify effect triggered on endpoint %d", identify->mEndpoint);

    // For now, just log the effect. Could be extended to show different visual effects
}

uint16_t IdentifyManager::GetCurrentIdentifyTime(chip::EndpointId endpoint)
{
    uint16_t identifyTime = 0;

    // Use CHIP stack to get the current identify time
    chip::Protocols::InteractionModel::Status status =
        chip::app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);

    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        return 0;
    }

    return identifyTime;
}

bool IdentifyManager::IsAnyEndpointIdentifying()
{
    for (const auto & cluster : mRegisteredClusters)
    {
        if (GetCurrentIdentifyTime(cluster->mEndpoint) > 0)
        {
            return true;
        }
    }
    return false;
}

std::vector<chip::EndpointId> IdentifyManager::GetIdentifyingEndpoints()
{
    std::vector<chip::EndpointId> identifyingEndpoints;

    for (const auto & cluster : mRegisteredClusters)
    {
        if (GetCurrentIdentifyTime(cluster->mEndpoint) > 0)
        {
            identifyingEndpoints.push_back(cluster->mEndpoint);
        }
    }

    return identifyingEndpoints;
}

void IdentifyManager::ShowIdentifyDialog(chip::EndpointId endpoint)
{
    if (mIdentifyDialog != nullptr)
    {
        mIdentifyDialog->Show(endpoint);
    }
}

void IdentifyManager::HideIdentifyDialog(chip::EndpointId endpoint)
{
    if (mIdentifyDialog != nullptr)
    {
        mIdentifyDialog->Hide(endpoint);
    }
}

} // namespace Ui
} // namespace example
