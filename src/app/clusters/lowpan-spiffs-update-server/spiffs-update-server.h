/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

/**
 * @brief SPIFFS Update Cluster Server
 *
 * This is a manufacturer-specific cluster for managing SPIFFS firmware updates.
 * The cluster provides commands to check for updates, force updates, and set device tokens.
 */

// Command callback declarations
bool emberAfLowpanSpiffsUpdateClusterCheckForUpdateCallback(
    chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::CharSpan & supabaseUrl);

bool emberAfLowpanSpiffsUpdateClusterForceUpdateCallback(
    chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::CharSpan & supabaseUrl);

bool emberAfLowpanSpiffsUpdateClusterGetStatusCallback(
    chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath);

bool emberAfLowpanSpiffsUpdateClusterSetDeviceTokenCallback(
    chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::CharSpan & jwt,
    uint64_t nodeId);
