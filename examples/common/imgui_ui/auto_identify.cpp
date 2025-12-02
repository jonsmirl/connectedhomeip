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
#include "auto_identify.h"
#include "identify_manager.h"
#include "ui.h"
#include "visual_effects.h"

#include <lib/support/logging/CHIPLogging.h>

#include <map>

namespace example {
namespace Ui {

AutoIdentifyRegistration::AutoIdentifyRegistration(ImguiUi * ui, bool createDefaultClusters)
{
    Initialize(ui, createDefaultClusters);
}

AutoIdentifyRegistration::AutoIdentifyRegistration(bool createDefaultClusters)
{
    Initialize(nullptr, createDefaultClusters);
}

AutoIdentifyRegistration::~AutoIdentifyRegistration()
{
    Cleanup();
}

void AutoIdentifyRegistration::Initialize(ImguiUi * ui, bool createDefaultClusters)
{
    if (mInitialized)
    {
        ChipLogError(AppServer, "AutoIdentifyRegistration already initialized");
        return;
    }

    mUi = ui;

    // Initialize the IdentifyManager
    IdentifyManager::GetInstance().Initialize(mUi);

    // Create visual effects system if enabled
    if (mVisualEffectsEnabled)
    {
        mVisualEffects = std::make_unique<VisualEffects>();
    }

    // Create default clusters if requested
    if (createDefaultClusters)
    {
        CreateDefaultClusters();
    }

    mInitialized = true;

    if (mUi != nullptr)
    {
        ChipLogProgress(AppServer, "AutoIdentifyRegistration initialized with UI support");
    }
    else
    {
        ChipLogProgress(AppServer, "AutoIdentifyRegistration initialized in headless mode");
    }
}

void AutoIdentifyRegistration::CreateDefaultClusters()
{
    // Create identify clusters for common endpoints

    // Endpoint 0 (Root device)
    if (AddIdentifyCluster(0, chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator))
    {
        ChipLogProgress(AppServer, "Created default identify cluster for endpoint 0");
    }
    else
    {
        ChipLogError(AppServer, "Failed to create identify cluster for endpoint 0");
    }

    // Endpoint 1 (Primary device)
    if (AddIdentifyCluster(1, chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator))
    {
        ChipLogProgress(AppServer, "Created default identify cluster for endpoint 1");
    }
    else
    {
        ChipLogError(AppServer, "Failed to create identify cluster for endpoint 1");
    }
}

bool AutoIdentifyRegistration::AddIdentifyCluster(chip::EndpointId endpoint,
                                                  chip::app::Clusters::Identify::IdentifyTypeEnum identifyType)
{
    // Check if cluster already exists
    if (HasIdentifyCluster(endpoint))
    {
        ChipLogProgress(AppServer, "Identify cluster for endpoint %d already exists", endpoint);
        return true;
    }

    // Create the cluster
    auto cluster = CreateIdentifyCluster(endpoint, identifyType);
    if (cluster == nullptr)
    {
        ChipLogError(AppServer, "Failed to create identify cluster for endpoint %d", endpoint);
        return false;
    }

    // Register with IdentifyManager
    IdentifyManager::GetInstance().RegisterIdentifyCluster(cluster.get());

    // Enable visual effects monitoring with thread safety fixes
    if (mVisualEffects != nullptr)
    {
        mVisualEffects->StartMonitoring(endpoint);
    }

    // Store the cluster
    mIdentifyClusters[endpoint] = std::move(cluster);

    ChipLogProgress(AppServer, "Added identify cluster for endpoint %d", endpoint);
    return true;
}

void AutoIdentifyRegistration::RemoveIdentifyCluster(chip::EndpointId endpoint)
{
    auto it = mIdentifyClusters.find(endpoint);
    if (it == mIdentifyClusters.end())
    {
        return;
    }

    // Unregister from IdentifyManager
    IdentifyManager::GetInstance().UnregisterIdentifyCluster(it->second.get());

    // Visual effects monitoring disabled
    // if (mVisualEffects != nullptr)
    // {
    //     mVisualEffects->StopMonitoring(endpoint);
    // }

    // Remove the cluster
    mIdentifyClusters.erase(it);

    ChipLogProgress(AppServer, "Removed identify cluster for endpoint %d", endpoint);
}

bool AutoIdentifyRegistration::HasIdentifyCluster(chip::EndpointId endpoint) const
{
    return mIdentifyClusters.find(endpoint) != mIdentifyClusters.end();
}

::Identify * AutoIdentifyRegistration::GetIdentifyCluster(chip::EndpointId endpoint) const
{
    auto it = mIdentifyClusters.find(endpoint);
    if (it != mIdentifyClusters.end())
    {
        return it->second.get();
    }
    return nullptr;
}

std::vector<chip::EndpointId> AutoIdentifyRegistration::GetRegisteredEndpoints() const
{
    std::vector<chip::EndpointId> endpoints;
    for (const auto & pair : mIdentifyClusters)
    {
        endpoints.push_back(pair.first);
    }
    return endpoints;
}

void AutoIdentifyRegistration::SetVisualEffectsEnabled(bool enabled)
{
    if (mVisualEffectsEnabled == enabled)
    {
        return;
    }

    mVisualEffectsEnabled = enabled;

    if (enabled && mVisualEffects == nullptr)
    {
        // Create visual effects system
        mVisualEffects = std::make_unique<VisualEffects>();

        // Start monitoring all existing clusters with thread safety fixes
        for (const auto & pair : mIdentifyClusters)
        {
            mVisualEffects->StartMonitoring(pair.first);
        }

        ChipLogProgress(AppServer, "Visual effects enabled");
    }
    else if (!enabled && mVisualEffects != nullptr)
    {
        // Destroy visual effects system
        mVisualEffects.reset();
        ChipLogProgress(AppServer, "Visual effects disabled");
    }
}

bool AutoIdentifyRegistration::IsVisualEffectsEnabled() const
{
    return mVisualEffectsEnabled && (mVisualEffects != nullptr);
}

void AutoIdentifyRegistration::Cleanup()
{
    if (!mInitialized)
    {
        return;
    }

    // Remove all clusters
    while (!mIdentifyClusters.empty())
    {
        RemoveIdentifyCluster(mIdentifyClusters.begin()->first);
    }

    // Cleanup visual effects
    mVisualEffects.reset();

    // Shutdown IdentifyManager
    IdentifyManager::GetInstance().Shutdown();

    mUi          = nullptr;
    mInitialized = false;

    ChipLogProgress(AppServer, "AutoIdentifyRegistration cleanup complete");
}

std::unique_ptr<::Identify>
AutoIdentifyRegistration::CreateIdentifyCluster(chip::EndpointId endpoint,
                                                chip::app::Clusters::Identify::IdentifyTypeEnum identifyType)
{
    auto cluster = std::make_unique<::Identify>(endpoint, IdentifyManager::OnIdentifyStart, IdentifyManager::OnIdentifyStop,
                                                identifyType, IdentifyManager::OnTriggerEffect);

    if (cluster == nullptr)
    {
        ChipLogError(AppServer, "Failed to create identify cluster for endpoint %d", endpoint);
        return nullptr;
    }

    ChipLogProgress(AppServer, "Created identify cluster for endpoint %d with type %d", endpoint, static_cast<int>(identifyType));
    return cluster;
}

} // namespace Ui
} // namespace example
