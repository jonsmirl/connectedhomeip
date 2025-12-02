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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

/**
 * @brief Lowpan Group Cluster Server
 *
 * This is a manufacturer-specific cluster for managing Lowpan (6LoWPAN/Thread) groups.
 * The cluster provides commands to add, remove, and configure group endpoints.
 */

// Command callback declarations
bool emberAfLowpanGroupClusterAddGroupEpCallback(chip::app::CommandHandler * commandObj,
                                                 const chip::app::ConcreteCommandPath & commandPath,
                                                 const chip::app::Clusters::LowpanGroup::Commands::AddGroupEp::DecodableType & commandData);

bool emberAfLowpanGroupClusterRemoveGroupEpCallback(chip::app::CommandHandler * commandObj,
                                                    const chip::app::ConcreteCommandPath & commandPath,
                                                    const chip::app::Clusters::LowpanGroup::Commands::RemoveGroupEp::DecodableType & commandData);

bool emberAfLowpanGroupClusterConfigurationCallback(chip::app::CommandHandler * commandObj,
                                                    const chip::app::ConcreteCommandPath & commandPath,
                                                    const chip::app::Clusters::LowpanGroup::Commands::Configuration::DecodableType & commandData);
