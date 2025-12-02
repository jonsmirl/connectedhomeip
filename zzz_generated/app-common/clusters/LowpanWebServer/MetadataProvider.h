// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanWebServer (cluster code: 4294114308/0xFFF2FC04)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LowpanWebServer/Ids.h>
#include <clusters/LowpanWebServer/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LowpanWebServer::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LowpanWebServer::Attributes;
        switch (attributeId)
        {
        case Enable::Id:
            return Enable::kMetadataEntry;
        case Hostname::Id:
            return Hostname::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LowpanWebServer::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LowpanWebServer::Commands;
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
