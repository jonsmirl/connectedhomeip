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

#include "CustomClusterIds.h"
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

/**
 * @file CustomClusterCallbacks.h
 * @brief Forward declarations for custom cluster command callbacks
 *
 * This file declares callback functions for the four Lowpan custom clusters:
 * - Lowpan BLE Sensor (0xFFF3FC03): Commands for BLE device management
 * - Lowpan Group (0xFFF3FC01): Commands for group endpoint management
 * - Lowpan Location (0xFFF3FC02): No commands (attribute-only cluster)
 * - Lowpan Web Server (0xFFF3FC04): No commands (attribute-only cluster)
 */

// ============================================================================
// Lowpan BLE Sensor Cluster Callbacks (0xFFF3FC03)
// ============================================================================

/**
 * @brief Callback for AddSensorEp command
 * @param commandHandler The command handler
 * @param commandPath The command path
 * @param commandData Decoded command data containing macAddress (char_string)
 * @return true if handled, false otherwise
 */
bool emberAfLowpanBleSensorClusterAddSensorEpCallback(
    chip::app::CommandHandler * commandHandler,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::LowpanBleSensor::Commands::AddSensorEp::DecodableType & commandData);

/**
 * @brief Callback for RemoveSensorEp command
 * @param commandHandler The command handler
 * @param commandPath The command path
 * @param commandData Decoded command data containing endpoint (int64u)
 * @return true if handled, false otherwise
 */
bool emberAfLowpanBleSensorClusterRemoveSensorEpCallback(
    chip::app::CommandHandler * commandHandler,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::LowpanBleSensor::Commands::RemoveSensorEp::DecodableType & commandData);

// ============================================================================
// Lowpan Group Cluster Callbacks (0xFFF3FC01)
// ============================================================================

/**
 * @brief Callback for AddGroupEp command
 * @param commandHandler The command handler
 * @param commandPath The command path
 * @param commandData Decoded command data containing endpoint (int16u) and groupName (char_string)
 * @return true if handled, false otherwise
 */
bool emberAfLowpanGroupClusterAddGroupEpCallback(
    chip::app::CommandHandler * commandHandler,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::LowpanGroup::Commands::AddGroupEp::DecodableType & commandData);

/**
 * @brief Callback for RemoveGroupEp command
 * @param commandHandler The command handler
 * @param commandPath The command path
 * @param commandData Decoded command data containing endpoint (int16u)
 * @return true if handled, false otherwise
 */
bool emberAfLowpanGroupClusterRemoveGroupEpCallback(
    chip::app::CommandHandler * commandHandler,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::LowpanGroup::Commands::RemoveGroupEp::DecodableType & commandData);

/**
 * @brief Callback for Configuration command
 * @param commandHandler The command handler
 * @param commandPath The command path
 * @param commandData Decoded command data (no parameters)
 * @return true if handled, false otherwise
 */
bool emberAfLowpanGroupClusterConfigurationCallback(
    chip::app::CommandHandler * commandHandler,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::LowpanGroup::Commands::Configuration::DecodableType & commandData);

// ============================================================================
// Lowpan Location Cluster (0xFFF3FC02)
// ============================================================================
// No callbacks needed - this cluster has no commands, only attributes:
// - Latitude (int32s)
// - Longitude (int32s)
// - Timezone (char_string, length 100)

// ============================================================================
// Lowpan Web Server Cluster (0xFFF3FC04)
// ============================================================================
// No callbacks needed - this cluster has no commands, only attributes:
// - Enable (boolean)
// - Hostname (char_string, length 32)
