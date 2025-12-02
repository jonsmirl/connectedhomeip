// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LowpanBleSensor (cluster code: 4294114307/0xFFF2FC03)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanBleSensor {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace AddSensorEp {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddSensorEp

namespace RemoveSensorEp {
inline constexpr CommandId Id = 0x00000001;
} // namespace RemoveSensorEp

} // namespace Commands
} // namespace LowpanBleSensor
} // namespace Clusters
} // namespace app
} // namespace chip
