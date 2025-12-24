// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanWebServer (cluster code: 4294114308/0xFFF2FC04)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LowpanWebServer/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanWebServer {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Jwt {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Jwt::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Jwt
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    Jwt::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace LowpanWebServer
} // namespace Clusters
} // namespace app
} // namespace chip
