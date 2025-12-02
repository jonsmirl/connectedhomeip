/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include "window.h"

#include <stdint.h>
#include <string>
#include <vector>

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/BitMask.h>

namespace example {
namespace Ui {
namespace Windows {

/**
 * Displays GroupKeyManagement cluster data and management controls for the root node (endpoint 0)
 */
class GroupManagement : public Window
{
public:
    GroupManagement(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Group Key Management"; }

private:
    chip::EndpointId mEndpointId;

    // GroupKeyManagement cluster attributes
    uint16_t mMaxGroupsPerFabric    = 0;
    uint16_t mMaxGroupKeysPerFabric = 0;

    // GroupKeyManagement state
    struct GroupKeySetInfo
    {
        uint16_t groupKeySetId;
        uint8_t groupKeySecurityPolicy;
        chip::ByteSpan epochKey0;
        uint64_t epochStartTime0;
        chip::ByteSpan epochKey1;
        uint64_t epochStartTime1;
        chip::ByteSpan epochKey2;
        uint64_t epochStartTime2;
        bool isValid;
    };

    struct GroupTableEntry
    {
        uint16_t groupId;
        std::vector<chip::EndpointId> endpoints;
        uint16_t groupKeySetId;
        std::string groupName;
        bool isValid;
    };

    std::vector<GroupKeySetInfo> mGroupKeySets;
    std::vector<GroupTableEntry> mGroupTable;

    // UI state
    int mNewGroupKeySetId        = 1;
    int mNewGroupId              = 1;
    int mSelectedKeySetIndex     = -1;
    int mSelectedGroupIndex      = -1;
    bool mShowAddKeySetDialog    = false;
    bool mShowRemoveKeySetDialog = false;
    bool mShowAddGroupDialog     = false;
    bool mShowRemoveGroupDialog  = false;

    // Command sending methods
    void SendKeySetWriteCommand(uint16_t groupKeySetId);
    void SendKeySetRemoveCommand(uint16_t groupKeySetId);
    void SendKeySetReadCommand(uint16_t groupKeySetId);
    void SendKeySetReadAllIndicesCommand();

    // Helper methods
    void RefreshGroupKeyData();
    void RenderClusterAttributes();
    void RenderGroupKeySets();
    void RenderGroupTable();
    void RenderKeySetControls();
    void RenderGroupControls();
    void RenderAddKeySetDialog();
    void RenderRemoveKeySetDialog();
    void RenderAddGroupDialog();
    void RenderRemoveGroupDialog();
    std::string GetSecurityPolicyString(uint8_t policy);
};

} // namespace Windows
} // namespace Ui
} // namespace example
