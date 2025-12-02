// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LowpanLocation (cluster code: 4294114306/0xFFF2FC02)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanLocation {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 8;

namespace Latitude {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Latitude

namespace Longitude {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Longitude

namespace Timezone {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Timezone

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace LowpanLocation
} // namespace Clusters
} // namespace app
} // namespace chip
