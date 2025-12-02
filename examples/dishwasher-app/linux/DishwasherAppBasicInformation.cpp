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

#include "DishwasherAppBasicInformation.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <clusters/BasicInformation/Attributes.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {

constexpr char kNodeLabelKey[]       = "dishwasher-app-node-label";
constexpr size_t kMaxNodeLabelLength = 32; // Matter specification limit

CHIP_ERROR DishwasherAppBasicInformationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::NodeLabel::Id:
        return ReadNodeLabel(aEncoder);
    case Attributes::ProductName::Id:
        return aEncoder.Encode(CharSpan::fromCharString("Dishwasher"));
    case Attributes::ProductLabel::Id:
        return aEncoder.Encode(CharSpan::fromCharString("Dishwasher"));
    case Attributes::PartNumber::Id:
        return aEncoder.Encode(CharSpan::fromCharString("DW-001"));
    case Attributes::SerialNumber::Id:
        return aEncoder.Encode(CharSpan::fromCharString("DW-123456"));
    case Attributes::UniqueID::Id:
        return aEncoder.Encode(CharSpan::fromCharString("DW-UID-123456"));
    default:
        // Let default implementation handle other attributes
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR DishwasherAppBasicInformationAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    // Only handle NodeLabel attribute
    if (aPath.mAttributeId == Attributes::NodeLabel::Id)
    {
        return WriteNodeLabel(aDecoder);
    }

    // For all other attributes, return CHIP_NO_ERROR to let the default implementation handle them
    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherAppBasicInformationAttrAccess::ReadNodeLabel(AttributeValueEncoder & aEncoder)
{
    char nodeLabel[kMaxNodeLabelLength + 1] = { 0 };
    size_t nodeLabelLen                     = 0;

    // Try to read from persistent storage
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(kNodeLabelKey, nodeLabel, sizeof(nodeLabel), &nodeLabelLen);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // If not found in storage, use empty string as default
        nodeLabel[0] = '\0';
        nodeLabelLen = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to read NodeLabel from storage: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    else
    {
        // Ensure null termination
        nodeLabel[std::min(nodeLabelLen, kMaxNodeLabelLength)] = '\0';
    }

    ChipLogProgress(Zcl, "Reading NodeLabel: '%s' (length: %u)", nodeLabel, static_cast<unsigned>(nodeLabelLen));

    return aEncoder.Encode(CharSpan(nodeLabel, strnlen(nodeLabel, kMaxNodeLabelLength)));
}

CHIP_ERROR DishwasherAppBasicInformationAttrAccess::WriteNodeLabel(AttributeValueDecoder & aDecoder)
{
    CharSpan nodeLabel;
    ReturnErrorOnFailure(aDecoder.Decode(nodeLabel));

    // Validate length according to Matter specification (max 32 characters)
    if (nodeLabel.size() > kMaxNodeLabelLength)
    {
        ChipLogError(Zcl, "Invalid NodeLabel length: %u (max %u)", static_cast<unsigned>(nodeLabel.size()),
                     static_cast<unsigned>(kMaxNodeLabelLength));
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Store the node label in persistent storage
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Put(kNodeLabelKey, nodeLabel.data(), nodeLabel.size());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to store NodeLabel: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(Zcl, "NodeLabel written successfully: '%.*s' (length: %u)", static_cast<int>(nodeLabel.size()),
                    nodeLabel.data(), static_cast<unsigned>(nodeLabel.size()));

    return CHIP_NO_ERROR;
}

} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
