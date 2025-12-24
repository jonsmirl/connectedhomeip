// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LowpanSpiffsUpdate (cluster code: 4294114311/0xFFF2FC07)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanSpiffsUpdate {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 10;

namespace CurrentVersion {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentVersion

namespace LastCheckTime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace LastCheckTime

namespace UpdateStatus {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpdateStatus

namespace ErrorCode {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ErrorCode

namespace DownloadProgress {
inline constexpr AttributeId Id = 0x00000004;
} // namespace DownloadProgress

namespace Initialized {
inline constexpr AttributeId Id = 0x00000005;
} // namespace Initialized

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
} // namespace LowpanSpiffsUpdate
} // namespace Clusters
} // namespace app
} // namespace chip
