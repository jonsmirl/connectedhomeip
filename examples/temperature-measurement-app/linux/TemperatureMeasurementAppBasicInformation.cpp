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

#include "include/TemperatureMeasurementAppBasicInformation.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;

namespace {
constexpr char kNodeLabelKey[]       = "temperature-measurement-app/node-label";
constexpr size_t kMaxNodeLabelLength = 32; // Matter specification limit
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {

CHIP_ERROR TemperatureMeasurementAppBasicInformationAttrAccess::Read(const ConcreteReadAttributePath & aPath,
                                                                     AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == BasicInformation::Id);

    switch (aPath.mAttributeId)
    {
    case BasicInformation::Attributes::NodeLabel::Id:
        return ReadNodeLabel(aEncoder);
    case BasicInformation::Attributes::ProductName::Id:
        return aEncoder.Encode(CharSpan::fromCharString("Temperature Sensor"));
    case BasicInformation::Attributes::ProductLabel::Id:
        return aEncoder.Encode(CharSpan::fromCharString("Temperature Sensor"));
    case BasicInformation::Attributes::PartNumber::Id:
        return aEncoder.Encode(CharSpan::fromCharString("TEMP-001"));
    case BasicInformation::Attributes::SerialNumber::Id:
        return aEncoder.Encode(CharSpan::fromCharString("TEMP-123456"));
    case BasicInformation::Attributes::UniqueID::Id:
        return aEncoder.Encode(CharSpan::fromCharString("TEMP-UID-123456"));
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementAppBasicInformationAttrAccess::Write(const ConcreteDataAttributePath & aPath,
                                                                      AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == BasicInformation::Id);

    switch (aPath.mAttributeId)
    {
    case BasicInformation::Attributes::NodeLabel::Id:
        return WriteNodeLabel(aDecoder);
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementAppBasicInformationAttrAccess::ReadNodeLabel(AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(NotSpecified, "Reading NodeLabel attribute");

    char nodeLabel[kMaxNodeLabelLength + 1] = { 0 };
    size_t nodeLabelLength                  = 0;

    CHIP_ERROR err =
        chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kNodeLabelKey, nodeLabel, sizeof(nodeLabel), &nodeLabelLength);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // If no stored value, return empty string
        ChipLogProgress(NotSpecified, "No stored NodeLabel found, returning empty string");
        return aEncoder.Encode(chip::CharSpan("", 0));
    }
    else if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to read NodeLabel from storage: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    // Ensure null termination
    nodeLabel[nodeLabelLength] = '\0';

    ChipLogProgress(NotSpecified, "Read NodeLabel: '%s' (length: %u)", nodeLabel, static_cast<unsigned>(nodeLabelLength));
    return aEncoder.Encode(chip::CharSpan(nodeLabel, nodeLabelLength));
}

CHIP_ERROR TemperatureMeasurementAppBasicInformationAttrAccess::WriteNodeLabel(AttributeValueDecoder & aDecoder)
{
    ChipLogProgress(NotSpecified, "Writing NodeLabel attribute");

    chip::CharSpan nodeLabel;
    ReturnErrorOnFailure(aDecoder.Decode(nodeLabel));

    // Validate length according to Matter specification
    if (nodeLabel.size() > kMaxNodeLabelLength)
    {
        ChipLogError(NotSpecified, "NodeLabel too long: %u bytes (max: %u)", static_cast<unsigned>(nodeLabel.size()),
                     static_cast<unsigned>(kMaxNodeLabelLength));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ChipLogProgress(NotSpecified, "Setting NodeLabel to: '%.*s' (length: %u)", static_cast<int>(nodeLabel.size()), nodeLabel.data(),
                    static_cast<unsigned>(nodeLabel.size()));

    // Store in persistent storage
    CHIP_ERROR err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kNodeLabelKey, nodeLabel.data(), nodeLabel.size());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to store NodeLabel: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(NotSpecified, "NodeLabel successfully stored");
    return CHIP_NO_ERROR;
}

} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
