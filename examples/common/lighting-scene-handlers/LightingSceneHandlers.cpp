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

#include "LightingSceneHandlers.h"

#include <app/clusters/color-control-server/color-control-server.h>
#include <app/clusters/level-control/level-control.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/scenes-server/scenes-server.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LightingSceneHandlers {

CHIP_ERROR RegisterLightingSceneHandlers(chip::EndpointId endpoint)
{
    ChipLogProgress(AppServer, "LightingSceneHandlers: Registering scene handlers for endpoint %u", endpoint);

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    // Register OnOff cluster scene handler
    auto * onOffSceneHandler = OnOffServer::Instance().GetSceneHandler();
    if (onOffSceneHandler != nullptr)
    {
        ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, onOffSceneHandler);
        ChipLogProgress(AppServer, "LightingSceneHandlers: Registered OnOff scene handler for endpoint %u", endpoint);
    }
    else
    {
        ChipLogError(AppServer, "LightingSceneHandlers: Failed to get OnOff scene handler for endpoint %u", endpoint);
    }

    // Register LevelControl cluster scene handler
    auto * levelControlSceneHandler = LevelControlServer::GetSceneHandler();
    if (levelControlSceneHandler != nullptr)
    {
        ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, levelControlSceneHandler);
        ChipLogProgress(AppServer, "LightingSceneHandlers: Registered LevelControl scene handler for endpoint %u", endpoint);
    }
    else
    {
        ChipLogError(AppServer, "LightingSceneHandlers: Failed to get LevelControl scene handler for endpoint %u", endpoint);
    }

    // Register ColorControl cluster scene handler
    auto * colorControlSceneHandler = ColorControlServer::GetSceneHandler();
    if (colorControlSceneHandler != nullptr)
    {
        ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, colorControlSceneHandler);
        ChipLogProgress(AppServer, "LightingSceneHandlers: Registered ColorControl scene handler for endpoint %u", endpoint);
    }
    else
    {
        ChipLogError(AppServer, "LightingSceneHandlers: Failed to get ColorControl scene handler for endpoint %u", endpoint);
    }

    ChipLogProgress(AppServer, "LightingSceneHandlers: Successfully registered all available scene handlers for endpoint %u",
                    endpoint);
    return CHIP_NO_ERROR;

#else
    ChipLogError(AppServer, "LightingSceneHandlers: Scene management not enabled in build configuration");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR UnregisterLightingSceneHandlers(chip::EndpointId endpoint)
{
    ChipLogProgress(AppServer, "LightingSceneHandlers: Unregistering scene handlers for endpoint %u", endpoint);

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    // Note: The ScenesServer doesn't provide an unregister method in the current implementation
    // This is a placeholder for future implementation if needed
    ChipLogProgress(AppServer, "LightingSceneHandlers: Scene handler unregistration not implemented in current CHIP version");
    return CHIP_NO_ERROR;

#else
    ChipLogError(AppServer, "LightingSceneHandlers: Scene management not enabled in build configuration");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

bool AreLightingSceneHandlersRegistered(chip::EndpointId endpoint)
{
#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    // Check if OnOff scene handler is registered
    auto * onOffSceneHandler = OnOffServer::Instance().GetSceneHandler();
    if (onOffSceneHandler != nullptr &&
        !ScenesManagement::ScenesServer::Instance().IsHandlerRegistered(endpoint, onOffSceneHandler))
    {
        return false;
    }

    // Check if LevelControl scene handler is registered
    auto * levelControlSceneHandler = LevelControlServer::GetSceneHandler();
    if (levelControlSceneHandler != nullptr &&
        !ScenesManagement::ScenesServer::Instance().IsHandlerRegistered(endpoint, levelControlSceneHandler))
    {
        return false;
    }

    // Check if ColorControl scene handler is registered
    auto * colorControlSceneHandler = ColorControlServer::GetSceneHandler();
    if (colorControlSceneHandler != nullptr &&
        !ScenesManagement::ScenesServer::Instance().IsHandlerRegistered(endpoint, colorControlSceneHandler))
    {
        return false;
    }

    return true;

#else
    return false;
#endif
}

} // namespace LightingSceneHandlers
} // namespace Clusters
} // namespace app
} // namespace chip
