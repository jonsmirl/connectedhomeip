// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowpanBleSensor (cluster code: 4294114307/0xFFF2FC03)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LowpanBleSensor/Ids.h>
#include <clusters/LowpanBleSensor/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LowpanBleSensor::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LowpanBleSensor::Attributes;
        switch (attributeId)
        {
        case Sensors::Id:
            return Sensors::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LowpanBleSensor::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LowpanBleSensor::Commands;
        switch (commandId)
        {
        case AddSensorEp::Id:
            return AddSensorEp::kMetadataEntry;
        case RemoveSensorEp::Id:
            return RemoveSensorEp::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
