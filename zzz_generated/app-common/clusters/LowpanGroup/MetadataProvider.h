// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanGroup (cluster code: 4294114305/0xFFF2FC01)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LowpanGroup/Ids.h>
#include <clusters/LowpanGroup/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LowpanGroup::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LowpanGroup::Attributes;
        switch (attributeId)
        {
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LowpanGroup::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LowpanGroup::Commands;
        switch (commandId)
        {
        case AddGroupEp::Id:
            return AddGroupEp::kMetadataEntry;
        case RemoveGroupEp::Id:
            return RemoveGroupEp::kMetadataEntry;
        case Configuration::Id:
            return Configuration::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
