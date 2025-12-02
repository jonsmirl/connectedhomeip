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

/**
 * @file CustomClusterIds.h
 * @brief Definitions for custom/manufacturer-specific cluster IDs and command structures
 *
 * This file provides stub definitions for the four Lowpan clusters which are
 * manufacturer-specific clusters (vendor code 0xFFF3) referenced by the generated code.
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Lowpan BLE Sensor Cluster (0xFFF3FC03)
 *
 * Provides facilities to manage BLE sensor devices including discovery, bridging,
 * and monitoring of sensor data from BLE devices.
 */
namespace LowpanBleSensor {
// Cluster ID for Lowpan BLE Sensor (manufacturer-specific)
inline constexpr ClusterId Id = 0xFFF3FC03;

namespace Commands {
namespace AddSensorEp {
inline constexpr CommandId Id = 0x00;
struct DecodableType
{
    chip::CharSpan macAddress;
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        // TODO: Implement proper decoding of macAddress field
        return CHIP_NO_ERROR;
    }
};
} // namespace AddSensorEp

namespace RemoveSensorEp {
inline constexpr CommandId Id = 0x01;
struct DecodableType
{
    uint64_t endpoint;
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        // TODO: Implement proper decoding of endpoint field
        return CHIP_NO_ERROR;
    }
};
} // namespace RemoveSensorEp
} // namespace Commands
} // namespace LowpanBleSensor

/**
 * @brief Lowpan Group Cluster (0xFFF3FC01)
 *
 * Provides facilities to manage group endpoints including adding and removing
 * group endpoints with associated names.
 */
namespace LowpanGroup {
// Cluster ID for Lowpan Group (manufacturer-specific)
inline constexpr ClusterId Id = 0xFFF3FC01;

namespace Commands {
namespace AddGroupEp {
inline constexpr CommandId Id = 0x00;
struct DecodableType
{
    uint16_t endpoint;
    chip::CharSpan groupName;
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        // TODO: Implement proper decoding of endpoint and groupName fields
        return CHIP_NO_ERROR;
    }
};
} // namespace AddGroupEp

namespace RemoveGroupEp {
inline constexpr CommandId Id = 0x01;
struct DecodableType
{
    uint16_t endpoint;
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        // TODO: Implement proper decoding of endpoint field
        return CHIP_NO_ERROR;
    }
};
} // namespace RemoveGroupEp

namespace Configuration {
inline constexpr CommandId Id = 0x02;
struct DecodableType
{
    // No parameters for Configuration command
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Decode(TLV::TLVReader & reader) { return CHIP_NO_ERROR; }
};
} // namespace Configuration
} // namespace Commands
} // namespace LowpanGroup

/**
 * @brief Lowpan Location Cluster (0xFFF3FC02)
 *
 * Provides facilities to configure and retrieve geographical location information
 * including latitude, longitude, and timezone settings.
 *
 * This cluster has NO commands - only attributes (Latitude, Longitude, Timezone).
 */
namespace LowpanLocation {
// Cluster ID for Lowpan Location (manufacturer-specific)
inline constexpr ClusterId Id = 0xFFF3FC02;

// No commands - this cluster only has attributes
} // namespace LowpanLocation

/**
 * @brief Lowpan Web Server Cluster (0xFFF3FC04)
 *
 * Provides facilities to configure and control an embedded web server including
 * enable/disable functionality and hostname configuration with automatic mDNS registration.
 *
 * This cluster has NO commands - only attributes (Enable, Hostname).
 */
namespace LowpanWebServer {
// Cluster ID for Lowpan Web Server (manufacturer-specific)
inline constexpr ClusterId Id = 0xFFF3FC04;

// No commands - this cluster only has attributes
} // namespace LowpanWebServer

} // namespace Clusters
} // namespace app
} // namespace chip
