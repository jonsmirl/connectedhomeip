// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanLocation (cluster code: 4294114306/0xFFF2FC02)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LowpanLocation/Ids.h>
#include <clusters/LowpanLocation/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LowpanLocation::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LowpanLocation::Attributes;
        switch (attributeId)
        {
        case Latitude::Id:
            return Latitude::kMetadataEntry;
        case Longitude::Id:
            return Longitude::kMetadataEntry;
        case Timezone::Id:
            return Timezone::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LowpanLocation::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LowpanLocation::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
