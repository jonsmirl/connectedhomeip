// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LowpanGroup (cluster code: 4294114305/0xFFF2FC01)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowpanGroup {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace AddGroupEp {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddGroupEp

namespace RemoveGroupEp {
inline constexpr CommandId Id = 0x00000001;
} // namespace RemoveGroupEp

namespace Configuration {
inline constexpr CommandId Id = 0x00000002;
} // namespace Configuration

} // namespace Commands
} // namespace LowpanGroup
} // namespace Clusters
} // namespace app
} // namespace chip
