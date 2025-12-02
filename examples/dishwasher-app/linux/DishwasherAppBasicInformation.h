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

#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {

/**
 * @brief Custom AttributeAccessInterface for BasicInformation cluster to handle NodeLabel attribute
 * 
 * This class provides custom handling for the NodeLabel attribute in the BasicInformation cluster,
 * allowing persistent storage and retrieval of the device's user-assigned name.
 */
class DishwasherAppBasicInformationAttrAccess : public AttributeAccessInterface
{
public:
    DishwasherAppBasicInformationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), BasicInformation::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadNodeLabel(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteNodeLabel(AttributeValueDecoder & aDecoder);
};

} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
