/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

// Keypad device combines both light and switch functionality
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 1
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

// Enable commissionable device type and name
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

// Keypad device type - using a custom device type for dual endpoint device
// For now, we'll use the bridge device type as it supports multiple endpoints
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 14 // 0x000E = 14 = Bridge
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8007 // Keypad product ID
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Keypad Device"

// Enable binding cluster for switch functionality
#define CHIP_DEVICE_CONFIG_ENABLE_BINDING_CLUSTER 1

// Enable client clusters for switch endpoint
#define CHIP_DEVICE_CONFIG_ENABLE_ON_OFF_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_LEVEL_CONTROL_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_COLOR_CONTROL_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_SCENES_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_IDENTIFY_CLIENT 1

// Enable group messaging for scene control
#define CHIP_DEVICE_CONFIG_ENABLE_GROUP_MESSAGING 1

// Enable operational credentials
#define CHIP_DEVICE_CONFIG_ENABLE_OPERATIONAL_CREDENTIALS 1

// Enable commissioning
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING 1

// Enable port parameters for --secured-device-port option
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

// Enable ImGui UI
#define CHIP_IMGUI_ENABLED 1

// Use a default setup PIN code if none is specified.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_PAIRING_CODE "CHIPUS"

/**
 * CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
 *
 * Enables the use of a hard-coded default serial number if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "KEYPAD_SN"

/**
 * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
 *
 * 0xFFF1: Test vendor
 */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION
 *
 * The hardware version number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software versions.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION 1

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 * CHIP service currently expects the software version to be in the format
 * {MAJOR_VERSION}.{MINOR_VERSION}.{PATCH_VERSION}
 */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0.0"

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
 *
 * A uint32_t identifying the software version running on the device.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Disable support for Chip-over-BLE (CHIPoBLE) - using IP-only commissioning.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote Chip Time service
 * using the Chip Time Sync protocol.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

/**
 * CHIP_DEVICE_CONFIG_TEST_MANUFACTURING_DATE
 *
 * Enables the use of a hard-coded default manufacturing date if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_MANUFACTURING_DATE "2020-01-01"

/**
 * CHIP_DEVICE_CONFIG_TEST_PRODUCT_URL
 *
 * Enables the use of a hard-coded default product URL if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_PRODUCT_URL "https://www.example.com"

/**
 * CHIP_DEVICE_CONFIG_TEST_PRODUCT_LABEL
 *
 * Enables the use of a hard-coded default product label if none
 * is found in Chip NV storage.
 */
#define CHIP_DEVICE_CONFIG_TEST_PRODUCT_LABEL "Keypad Device"

// Enable default scene handlers for lighting clusters
#define CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS 1

// include the CHIPProjectConfig from config/standalone (at end to let our defines take precedence)
#include <CHIPProjectConfig.h>
