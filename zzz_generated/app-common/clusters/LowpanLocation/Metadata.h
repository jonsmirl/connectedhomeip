// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanLocation (cluster code: 4294114306/0xFFF2FC02)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LowpanLocation/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanLocation {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Latitude {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Latitude::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Latitude
namespace Longitude {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Longitude::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Longitude
namespace Timezone {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Timezone::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Timezone
namespace Locality {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Locality::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Locality
namespace City {
inline constexpr DataModel::AttributeEntry kMetadataEntry(City::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace City
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    Latitude::kMetadataEntry, Longitude::kMetadataEntry, Timezone::kMetadataEntry, Locality::kMetadataEntry, City::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace LowpanLocation
} // namespace Clusters
} // namespace app
} // namespace chip
