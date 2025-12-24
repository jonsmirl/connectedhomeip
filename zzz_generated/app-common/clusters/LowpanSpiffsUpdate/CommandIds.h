// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LowpanSpiffsUpdate (cluster code: 4294114311/0xFFF2FC07)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanSpiffsUpdate {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace CheckForUpdate {
inline constexpr CommandId Id = 0x00000000;
} // namespace CheckForUpdate

namespace ForceUpdate {
inline constexpr CommandId Id = 0x00000001;
} // namespace ForceUpdate

namespace GetStatus {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetStatus

namespace SetDeviceToken {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetDeviceToken

} // namespace Commands
} // namespace LowpanSpiffsUpdate
} // namespace Clusters
} // namespace app
} // namespace chip
