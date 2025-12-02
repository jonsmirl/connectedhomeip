// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanBleSensor (cluster code: 4294114307/0xFFF2FC03)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LowpanBleSensor/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanBleSensor {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Sensors {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Sensors::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Sensors
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    Sensors::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace AddSensorEp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddSensorEp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddSensorEp
namespace RemoveSensorEp {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveSensorEp::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace RemoveSensorEp

} // namespace Commands

namespace Events {} // namespace Events
} // namespace LowpanBleSensor
} // namespace Clusters
} // namespace app
} // namespace chip
