/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/scenes-server/scenes-server.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Common lighting scene handlers for Matter devices with lighting clusters.
 * 
 * This provides scene support for OnOff, LevelControl, and ColorControl clusters
 * that is shared across lighting-app, rocker-app, and keypad-app.
 * 
 * Usage:
 *   1. Call RegisterLightingSceneHandlers(endpoint) during application initialization
 *   2. Ensure MATTER_DM_PLUGIN_SCENES_MANAGEMENT is defined in your build
 *   3. Ensure CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS is enabled
 */
namespace LightingSceneHandlers {

/**
 * @brief Register scene handlers for all lighting clusters on the specified endpoint.
 * 
 * This function registers scene handlers for:
 * - OnOff cluster (0x0006)
 * - LevelControl cluster (0x0008) 
 * - ColorControl cluster (0x0300)
 * 
 * @param endpoint The endpoint ID where lighting clusters are present
 * @return CHIP_NO_ERROR on success, appropriate error code on failure
 */
CHIP_ERROR RegisterLightingSceneHandlers(chip::EndpointId endpoint);

/**
 * @brief Unregister scene handlers for all lighting clusters on the specified endpoint.
 * 
 * @param endpoint The endpoint ID where lighting clusters are present
 * @return CHIP_NO_ERROR on success, appropriate error code on failure
 */
CHIP_ERROR UnregisterLightingSceneHandlers(chip::EndpointId endpoint);

/**
 * @brief Check if scene handlers are properly registered for lighting clusters.
 * 
 * @param endpoint The endpoint ID to check
 * @return true if all lighting scene handlers are registered, false otherwise
 */
bool AreLightingSceneHandlersRegistered(chip::EndpointId endpoint);

} // namespace LightingSceneHandlers
} // namespace Clusters
} // namespace app
} // namespace chip
