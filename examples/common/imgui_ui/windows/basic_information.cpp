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

#include "basic_information.h"

#include <imgui.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace example {
namespace Ui {
namespace Windows {

namespace {
// Empty namespace - no helper functions needed for static implementation
} // namespace

void BasicInformation::UpdateState()
{
    using namespace chip::app::Clusters::BasicInformation;

    // For BasicInformation cluster, many attributes are handled by a custom AttributeAccessInterface
    // and don't have standard generated accessors. We'll populate with static/default values
    // that are typical for a lighting device.

    // Get device-specific information (can be overridden by derived classes)
    mDataModelRevision     = GetDeviceDataModelRevision();
    mVendorName            = GetDeviceVendorName();
    mVendorID              = GetDeviceVendorID();
    mProductName           = GetDeviceProductName();
    mProductID             = GetDeviceProductID();
    mNodeLabel             = GetDeviceNodeLabel();
    mLocation              = GetDeviceLocation();
    mHardwareVersion       = GetDeviceHardwareVersion();
    mHardwareVersionString = GetDeviceHardwareVersionString();
    mSoftwareVersion       = GetDeviceSoftwareVersion();
    mSoftwareVersionString = GetDeviceSoftwareVersionString();
    mManufacturingDate     = GetDeviceManufacturingDate();
    mPartNumber            = GetDevicePartNumber();
    mProductURL            = GetDeviceProductURL();
    mProductLabel          = GetDeviceProductLabel();
    mSerialNumber          = GetDeviceSerialNumber();
    mLocalConfigDisabled   = GetDeviceLocalConfigDisabled();
    mReachable             = GetDeviceReachable();
    mUniqueID              = GetDeviceUniqueID();
    mCapabilityMinima      = GetDeviceCapabilityMinima();
    mSpecificationVersion  = GetDeviceSpecificationVersion();
    mMaxPathsPerInvoke     = GetDeviceMaxPathsPerInvoke();

    // Note: BasicInformation cluster uses a custom AttributeAccessInterface
    // so standard attribute accessors are not available. Using static values above.
}

void BasicInformation::Render()
{
    ImGui::Begin("Device Information");
    RenderContent();
    ImGui::End();
}

void BasicInformation::RenderContent()
{
    ImGui::Text("Basic Information Cluster (Endpoint %d)", mEndpointId);
    ImGui::Separator();

    // Device Identity Section
    ImGui::Text("Device Identity:");
    ImGui::Indent();
    ImGui::Text("Vendor Name:            %s", mVendorName.c_str());
    ImGui::Text("Vendor ID:              %d (0x%04X)", mVendorID, mVendorID);
    ImGui::Text("Product Name:           %s", mProductName.c_str());
    ImGui::Text("Product ID:             %d (0x%04X)", mProductID, mProductID);
    ImGui::Text("Product Label:          %s", mProductLabel.c_str());
    ImGui::Text("Product URL:            %s", mProductURL.c_str());
    ImGui::Text("Serial Number:          %s", mSerialNumber.c_str());
    ImGui::Text("Part Number:            %s", mPartNumber.c_str());
    ImGui::Text("Unique ID:              %s", mUniqueID.c_str());
    ImGui::Unindent();

    ImGui::Separator();

    // Version Information Section
    ImGui::Text("Version Information:");
    ImGui::Indent();
    ImGui::Text("Data Model Revision:    %s", mDataModelRevision.c_str());
    ImGui::Text("Hardware Version:       %d (%s)", mHardwareVersion, mHardwareVersionString.c_str());
    ImGui::Text("Software Version:       %d (%s)", mSoftwareVersion, mSoftwareVersionString.c_str());
    ImGui::Text("Specification Version:  0x%08X", mSpecificationVersion);
    ImGui::Text("Manufacturing Date:     %s", mManufacturingDate.c_str());
    ImGui::Unindent();

    ImGui::Separator();

    // Configuration Section
    ImGui::Text("Configuration:");
    ImGui::Indent();
    ImGui::Text("Node Label:             %s", mNodeLabel.c_str());
    ImGui::Text("Location:               %s", mLocation.c_str());
    ImGui::Text("Local Config Disabled:  %s", mLocalConfigDisabled ? "Yes" : "No");
    ImGui::Text("Reachable:              %s", mReachable ? "Yes" : "No");
    ImGui::Unindent();

    ImGui::Separator();

    // Capabilities Section
    ImGui::Text("Capabilities:");
    ImGui::Indent();
    ImGui::Text("Capability Minima:      0x%08X", mCapabilityMinima);
    ImGui::Text("Max Paths Per Invoke:   %d", mMaxPathsPerInvoke);
    ImGui::Unindent();
}

} // namespace Windows
} // namespace Ui
} // namespace example
