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

#include <app/clusters/identify-server/identify-server.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

#include <map>
#include <memory>
#include <vector>

// Forward declarations
namespace example {
namespace Ui {
class ImguiUi;
class VisualEffects;
} // namespace Ui
} // namespace example

namespace example {
namespace Ui {

/**
 * RAII class for automatic identify cluster registration and management.
 *
 * This class provides a one-line solution for enabling identify dialog
 * functionality in Linux Matter device apps. It automatically:
 * - Creates identify cluster instances for common endpoints
 * - Registers them with the IdentifyManager
 * - Integrates with the ImGui UI system
 * - Handles cleanup on destruction
 *
 * Usage:
 * ```cpp
 * #if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
 * example::Ui::ImguiUi ui(true, false);
 * example::Ui::AutoIdentifyRegistration identifySystem(&ui);
 * #else
 * example::Ui::AutoIdentifyRegistration identifySystem; // headless mode
 * #endif
 * ```
 */
class AutoIdentifyRegistration
{
public:
    /**
     * Constructor for GUI mode with ImGui UI integration.
     *
     * @param ui Pointer to the ImguiUi instance for dialog integration
     * @param createDefaultClusters Whether to create default identify clusters for EP0 and EP1
     */
    explicit AutoIdentifyRegistration(ImguiUi * ui = nullptr, bool createDefaultClusters = true);

    /**
     * Constructor for headless mode without UI.
     *
     * @param createDefaultClusters Whether to create default identify clusters for EP0 and EP1
     */
    explicit AutoIdentifyRegistration(bool createDefaultClusters);

    /**
     * Destructor - automatically cleans up all registered clusters.
     */
    ~AutoIdentifyRegistration();

    /**
     * Add a custom identify cluster for a specific endpoint.
     *
     * @param endpoint The endpoint ID for the cluster
     * @param identifyType The type of identification method
     * @return true if cluster was created successfully, false otherwise
     */
    bool AddIdentifyCluster(chip::EndpointId endpoint,
                            chip::app::Clusters::Identify::IdentifyTypeEnum identifyType =
                                chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator);

    /**
     * Remove an identify cluster for a specific endpoint.
     *
     * @param endpoint The endpoint ID to remove
     */
    void RemoveIdentifyCluster(chip::EndpointId endpoint);

    /**
     * Check if an identify cluster exists for an endpoint.
     *
     * @param endpoint The endpoint ID to check
     * @return true if cluster exists, false otherwise
     */
    bool HasIdentifyCluster(chip::EndpointId endpoint) const;

    /**
     * Get the identify cluster instance for an endpoint.
     *
     * @param endpoint The endpoint ID to get
     * @return Pointer to identify cluster, or nullptr if not found
     */
    ::Identify * GetIdentifyCluster(chip::EndpointId endpoint) const;

    /**
     * Get all registered endpoint IDs.
     *
     * @return Vector of endpoint IDs that have identify clusters
     */
    std::vector<chip::EndpointId> GetRegisteredEndpoints() const;

    /**
     * Enable or disable visual effects monitoring.
     *
     * @param enabled Whether to enable visual effects
     */
    void SetVisualEffectsEnabled(bool enabled);

    /**
     * Check if visual effects are enabled.
     *
     * @return true if visual effects are enabled, false otherwise
     */
    bool IsVisualEffectsEnabled() const;

private:
    /**
     * Initialize the identify system.
     *
     * @param ui Pointer to ImguiUi instance (can be nullptr)
     * @param createDefaultClusters Whether to create default clusters
     */
    void Initialize(ImguiUi * ui, bool createDefaultClusters);

    /**
     * Create default identify clusters for common endpoints.
     */
    void CreateDefaultClusters();

    /**
     * Cleanup all resources and unregister clusters.
     */
    void Cleanup();

    /**
     * Create an identify cluster for a specific endpoint.
     *
     * @param endpoint The endpoint ID
     * @param identifyType The identification type
     * @return Pointer to created cluster, or nullptr on failure
     */
    std::unique_ptr<::Identify> CreateIdentifyCluster(chip::EndpointId endpoint,
                                                      chip::app::Clusters::Identify::IdentifyTypeEnum identifyType);

    // Map of endpoint ID to identify cluster instance
    std::map<chip::EndpointId, std::unique_ptr<::Identify>> mIdentifyClusters;

    // UI integration
    ImguiUi * mUi = nullptr;

    // Visual effects system
    std::unique_ptr<VisualEffects> mVisualEffects;
    bool mVisualEffectsEnabled = true;

    // Initialization state
    bool mInitialized = false;
};

/**
 * Convenience macro for creating identify cluster instances with ImGui integration.
 * This macro creates an identify cluster with the standard callbacks for ImGui UI.
 */
#define IMGUI_IDENTIFY_CLUSTER_VISIBLE(endpoint)                                                                                   \
    ::Identify(chip::EndpointId(endpoint), example::Ui::IdentifyManager::OnIdentifyStart,                                          \
               example::Ui::IdentifyManager::OnIdentifyStop, chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,   \
               example::Ui::IdentifyManager::OnTriggerEffect)

/**
 * Convenience macro for creating identify cluster instances without visual indication.
 */
#define IMGUI_IDENTIFY_CLUSTER_NONE(endpoint)                                                                                      \
    ::Identify(chip::EndpointId(endpoint), example::Ui::IdentifyManager::OnIdentifyStart,                                          \
               example::Ui::IdentifyManager::OnIdentifyStop, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,               \
               example::Ui::IdentifyManager::OnTriggerEffect)

} // namespace Ui
} // namespace example
