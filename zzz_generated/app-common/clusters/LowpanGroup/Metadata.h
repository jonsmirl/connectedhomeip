// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanGroup (cluster code: 4294114305/0xFFF2FC01)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LowpanGroup/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanGroup {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

constexpr std::array<DataModel::AttributeEntry, 0> kMandatoryMetadata = {

};

} // namespace Attributes

namespace Commands {

namespace AddGroupEp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddGroupEp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddGroupEp
namespace RemoveGroupEp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveGroupEp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace RemoveGroupEp
namespace Configuration {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Configuration::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Configuration

} // namespace Commands

namespace Events {} // namespace Events
} // namespace LowpanGroup
} // namespace Clusters
} // namespace app
} // namespace chip
