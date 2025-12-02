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
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include <memory>
#include <vector>

// Forward declarations
namespace example {
namespace Ui {
class ImguiUi;
class IdentifyDialog;
} // namespace Ui
} // namespace example

namespace example {
namespace Ui {

/**
 * Centralized manager for identify cluster functionality and UI coordination.
 *
 * This singleton class bridges between Matter identify cluster callbacks and
 * the ImGui UI system, providing automatic identify dialog management across
 * all Linux Matter device apps.
 *
 * Key Features:
 * - Automatic identify cluster registration for common endpoints
 * - Reactive monitoring of IdentifyTime attribute changes
 * - Centralized UI coordination for identify dialogs
 * - Thread-safe callback handling
 */
class IdentifyManager
{
public:
    /**
     * Get the singleton instance of IdentifyManager.
     */
    static IdentifyManager & GetInstance();

    /**
     * Initialize the identify manager with a UI instance.
     * This should be called once during application startup.
     *
     * @param ui Pointer to the ImguiUi instance (can be nullptr for headless mode)
     */
    void Initialize(ImguiUi * ui = nullptr);

    /**
     * Update identify dialogs. This should be called from the UI render loop.
     */
    void UpdateDialogs();

    /**
     * Shutdown the identify manager and cleanup resources.
     */
    void Shutdown();

    /**
     * Register an identify cluster instance with the manager.
     * This is called automatically by AutoIdentifyRegistration.
     *
     * @param identify Pointer to the identify cluster instance
     */
    void RegisterIdentifyCluster(::Identify * identify);

    /**
     * Unregister an identify cluster instance from the manager.
     *
     * @param identify Pointer to the identify cluster instance
     */
    void UnregisterIdentifyCluster(::Identify * identify);

    /**
     * Called when identify starts on an endpoint.
     * This is used as the callback for identify cluster instances.
     *
     * @param identify Pointer to the identify cluster instance
     */
    static void OnIdentifyStart(::Identify * identify);

    /**
     * Called when identify stops on an endpoint.
     * This is used as the callback for identify cluster instances.
     *
     * @param identify Pointer to the identify cluster instance
     */
    static void OnIdentifyStop(::Identify * identify);

    /**
     * Called when identify effect is triggered.
     * This is used as the callback for identify cluster instances.
     *
     * @param identify Pointer to the identify cluster instance
     */
    static void OnTriggerEffect(::Identify * identify);

    /**
     * Get the current identify time for an endpoint.
     *
     * @param endpoint The endpoint to query
     * @return Current identify time in seconds, or 0 if not identifying
     */
    uint16_t GetCurrentIdentifyTime(chip::EndpointId endpoint);

    /**
     * Check if any endpoint is currently identifying.
     *
     * @return true if any endpoint is identifying, false otherwise
     */
    bool IsAnyEndpointIdentifying();

    /**
     * Get a list of all endpoints that are currently identifying.
     *
     * @return Vector of endpoint IDs that are currently identifying
     */
    std::vector<chip::EndpointId> GetIdentifyingEndpoints();

private:
    IdentifyManager()  = default;
    ~IdentifyManager() = default;

    // Prevent copying
    IdentifyManager(const IdentifyManager &)             = delete;
    IdentifyManager & operator=(const IdentifyManager &) = delete;

    /**
     * Internal method to handle identify start events.
     *
     * @param identify Pointer to the identify cluster instance
     */
    void HandleIdentifyStart(::Identify * identify);

    /**
     * Internal method to handle identify stop events.
     *
     * @param identify Pointer to the identify cluster instance
     */
    void HandleIdentifyStop(::Identify * identify);

    /**
     * Internal method to handle trigger effect events.
     *
     * @param identify Pointer to the identify cluster instance
     */
    void HandleTriggerEffect(::Identify * identify);

    /**
     * Show the identify dialog for the given endpoint.
     *
     * @param endpoint The endpoint that is identifying
     */
    void ShowIdentifyDialog(chip::EndpointId endpoint);

    /**
     * Hide the identify dialog for the given endpoint.
     *
     * @param endpoint The endpoint that stopped identifying
     */
    void HideIdentifyDialog(chip::EndpointId endpoint);

    // Member variables
    ImguiUi * mUi = nullptr;
    std::vector<::Identify *> mRegisteredClusters;
    std::unique_ptr<IdentifyDialog> mIdentifyDialog;
    bool mInitialized = false;
};

} // namespace Ui
} // namespace example
