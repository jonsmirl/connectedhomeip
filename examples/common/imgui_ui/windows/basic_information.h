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

#include "window.h"

#include <stdint.h>
#include <string>

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

namespace example {
namespace Ui {
namespace Windows {

/**
 * Displays BasicInformation cluster data for a given endpoint
 */
class BasicInformation : public Window
{
public:
    BasicInformation(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Device Information"; }
    std::string GetDeviceProductNameForTitle() const override { return GetDeviceProductName(); }

private:
    chip::EndpointId mEndpointId;

    // BasicInformation cluster attributes
    std::string mDataModelRevision;
    std::string mVendorName;
    uint16_t mVendorID = 0;
    std::string mProductName;
    uint16_t mProductID = 0;
    std::string mNodeLabel;
    std::string mLocation;
    uint16_t mHardwareVersion = 0;
    std::string mHardwareVersionString;
    uint32_t mSoftwareVersion = 0;
    std::string mSoftwareVersionString;
    std::string mManufacturingDate;
    std::string mPartNumber;
    std::string mProductURL;
    std::string mProductLabel;
    std::string mSerialNumber;
    bool mLocalConfigDisabled = false;
    bool mReachable           = true;
    std::string mUniqueID;
    uint32_t mCapabilityMinima     = 0;
    uint32_t mSpecificationVersion = 0;
    uint32_t mMaxPathsPerInvoke    = 0;

protected:
    // Virtual methods for device-specific information - can be overridden
    virtual std::string GetDeviceDataModelRevision() const { return "1"; }
    virtual std::string GetDeviceVendorName() const { return "CSA"; }
    virtual uint16_t GetDeviceVendorID() const { return 0xFFF1; }
    virtual std::string GetDeviceProductName() const { return "Generic Matter Device"; }
    virtual uint16_t GetDeviceProductID() const { return 0x8000; }
    virtual std::string GetDeviceNodeLabel() const { return "Generic Matter Device"; }
    virtual std::string GetDeviceLocation() const { return "US"; }
    virtual uint16_t GetDeviceHardwareVersion() const { return 1; }
    virtual std::string GetDeviceHardwareVersionString() const { return "1.0"; }
    virtual uint32_t GetDeviceSoftwareVersion() const { return 1; }
    virtual std::string GetDeviceSoftwareVersionString() const { return "1.0.0"; }
    virtual std::string GetDeviceManufacturingDate() const { return "20240101"; }
    virtual std::string GetDevicePartNumber() const { return "GENERIC-001"; }
    virtual std::string GetDeviceProductURL() const { return "https://csa-iot.org"; }
    virtual std::string GetDeviceProductLabel() const { return GetDeviceProductName(); }
    virtual std::string GetDeviceSerialNumber() const { return "GENERIC-123456"; }
    virtual bool GetDeviceLocalConfigDisabled() const { return false; }
    virtual bool GetDeviceReachable() const { return true; }
    virtual std::string GetDeviceUniqueID() const { return "GENERIC-UID-123456"; }
    virtual uint32_t GetDeviceCapabilityMinima() const { return 0x00000001; }
    virtual uint32_t GetDeviceSpecificationVersion() const { return 0x01040000; } // Matter 1.4
    virtual uint32_t GetDeviceMaxPathsPerInvoke() const { return 1; }
};

} // namespace Windows
} // namespace Ui
} // namespace example
