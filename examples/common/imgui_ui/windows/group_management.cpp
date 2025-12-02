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

#include "group_management.h"

#include <algorithm>
#include <imgui.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace example {
namespace Ui {
namespace Windows {

void GroupManagement::UpdateState()
{
    using namespace chip::app::Clusters::GroupKeyManagement;

    // Read GroupKeyManagement cluster attributes
    // For simplicity, we'll use default values since this is a demonstration
    mMaxGroupsPerFabric    = 12; // Typical value for Matter devices
    mMaxGroupKeysPerFabric = 3;  // Typical value for Matter devices

    // In a real implementation, you would read from the device:
    // Attributes::MaxGroupsPerFabric::Get(mEndpointId, &mMaxGroupsPerFabric);
    // Attributes::MaxGroupKeysPerFabric::Get(mEndpointId, &mMaxGroupKeysPerFabric);

    // Refresh the group key data
    RefreshGroupKeyData();
}

void GroupManagement::Render()
{
    ImGui::Begin("Group Key Management");
    RenderContent();
    ImGui::End();
}

void GroupManagement::RenderContent()
{
    ImGui::Text("GroupKeyManagement Cluster (Endpoint %d)", mEndpointId);
    ImGui::Separator();

    // Cluster attributes
    RenderClusterAttributes();

    ImGui::Separator();

    // Group Key Sets
    RenderGroupKeySets();

    ImGui::Separator();

    // Group Table
    RenderGroupTable();

    ImGui::Separator();

    // Controls
    RenderKeySetControls();
    RenderGroupControls();

    // Dialogs
    RenderAddKeySetDialog();
    RenderRemoveKeySetDialog();
    RenderAddGroupDialog();
    RenderRemoveGroupDialog();
}

void GroupManagement::RenderClusterAttributes()
{
    ImGui::Text("Cluster Attributes:");
    ImGui::Indent();
    ImGui::Text("Max Groups Per Fabric: %d", mMaxGroupsPerFabric);
    ImGui::Text("Max Group Keys Per Fabric: %d", mMaxGroupKeysPerFabric);
    ImGui::Unindent();
}

void GroupManagement::RenderGroupKeySets()
{
    ImGui::Text("Group Key Sets:");
    ImGui::Indent();

    if (mGroupKeySets.empty())
    {
        ImGui::TextDisabled("No group key sets configured");
    }
    else
    {
        // Table header
        if (ImGui::BeginTable("KeySetsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Key Set ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Security Policy", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Epoch Keys", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < mGroupKeySets.size(); i++)
            {
                const auto & keySet = mGroupKeySets[i];
                ImGui::TableNextRow();

                // Key Set ID
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", keySet.groupKeySetId);

                // Security Policy
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", GetSecurityPolicyString(keySet.groupKeySecurityPolicy).c_str());

                // Epoch Keys
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("3 epoch keys configured");

                // Actions
                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(static_cast<int>(i));

                if (ImGui::Button("Read"))
                {
                    SendKeySetReadCommand(keySet.groupKeySetId);
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove"))
                {
                    mSelectedKeySetIndex    = static_cast<int>(i);
                    mShowRemoveKeySetDialog = true;
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    ImGui::Unindent();
}

void GroupManagement::RenderGroupTable()
{
    ImGui::Text("Group Table:");
    ImGui::Indent();

    if (mGroupTable.empty())
    {
        ImGui::TextDisabled("No groups in table");
    }
    else
    {
        // Table header
        if (ImGui::BeginTable("GroupTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Group ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Endpoints", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Key Set ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Group Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < mGroupTable.size(); i++)
            {
                const auto & entry = mGroupTable[i];
                ImGui::TableNextRow();

                // Group ID
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", entry.groupId);

                // Endpoints
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%zu EPs", entry.endpoints.size());

                // Key Set ID
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", entry.groupKeySetId);

                // Group Name
                ImGui::TableSetColumnIndex(3);
                if (entry.groupName.empty())
                {
                    ImGui::TextDisabled("(No name)");
                }
                else
                {
                    ImGui::Text("%s", entry.groupName.c_str());
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::Unindent();
}

void GroupManagement::RenderKeySetControls()
{
    ImGui::Text("Key Set Management:");
    ImGui::Indent();

    // Add Key Set button
    if (ImGui::Button("Add Key Set"))
    {
        mShowAddKeySetDialog = true;
    }
    ImGui::SameLine();

    // Read All Indices button
    if (ImGui::Button("Read All Indices"))
    {
        SendKeySetReadAllIndicesCommand();
    }

    ImGui::Unindent();
}

void GroupManagement::RenderGroupControls()
{
    ImGui::Text("Group Management:");
    ImGui::Indent();

    // Add Group button
    if (ImGui::Button("Add Group"))
    {
        mShowAddGroupDialog = true;
    }
    ImGui::SameLine();

    // Refresh button
    if (ImGui::Button("Refresh"))
    {
        RefreshGroupKeyData();
    }

    ImGui::Unindent();
}

void GroupManagement::RenderAddKeySetDialog()
{
    if (mShowAddKeySetDialog)
    {
        ImGui::OpenPopup("Add Key Set");
    }

    if (ImGui::BeginPopupModal("Add Key Set", &mShowAddKeySetDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Create a new group key set:");
        ImGui::Separator();

        ImGui::Text("Key Set ID:");
        ImGui::InputInt("##KeySetID", &mNewGroupKeySetId, 1, 10);
        if (mNewGroupKeySetId < 1)
            mNewGroupKeySetId = 1;
        if (mNewGroupKeySetId > 65535)
            mNewGroupKeySetId = 65535;

        ImGui::Separator();

        if (ImGui::Button("Create"))
        {
            SendKeySetWriteCommand(static_cast<uint16_t>(mNewGroupKeySetId));
            mShowAddKeySetDialog = false;
            mNewGroupKeySetId++;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            mShowAddKeySetDialog = false;
        }

        ImGui::EndPopup();
    }
}

void GroupManagement::RenderRemoveKeySetDialog()
{
    if (mShowRemoveKeySetDialog && mSelectedKeySetIndex >= 0 && mSelectedKeySetIndex < static_cast<int>(mGroupKeySets.size()))
    {
        ImGui::OpenPopup("Remove Key Set");
    }

    if (ImGui::BeginPopupModal("Remove Key Set", &mShowRemoveKeySetDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (mSelectedKeySetIndex >= 0 && mSelectedKeySetIndex < static_cast<int>(mGroupKeySets.size()))
        {
            const auto & keySet = mGroupKeySets[mSelectedKeySetIndex];
            ImGui::Text("Remove key set %d?", keySet.groupKeySetId);
            ImGui::Separator();

            if (ImGui::Button("Remove"))
            {
                SendKeySetRemoveCommand(keySet.groupKeySetId);
                mShowRemoveKeySetDialog = false;
                mSelectedKeySetIndex    = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                mShowRemoveKeySetDialog = false;
                mSelectedKeySetIndex    = -1;
            }
        }

        ImGui::EndPopup();
    }
}

void GroupManagement::RenderAddGroupDialog()
{
    if (mShowAddGroupDialog)
    {
        ImGui::OpenPopup("Add Group");
    }

    if (ImGui::BeginPopupModal("Add Group", &mShowAddGroupDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Add group to table:");
        ImGui::Separator();

        ImGui::Text("Group ID:");
        ImGui::InputInt("##GroupID", &mNewGroupId, 1, 10);
        if (mNewGroupId < 1)
            mNewGroupId = 1;
        if (mNewGroupId > 65535)
            mNewGroupId = 65535;

        ImGui::Separator();

        if (ImGui::Button("Add"))
        {
            // Add to group table (simplified)
            GroupTableEntry entry;
            entry.groupId = static_cast<uint16_t>(mNewGroupId);
            entry.endpoints.push_back(chip::EndpointId(1)); // Add endpoint 1
            entry.groupKeySetId = 1;                        // Default key set
            entry.groupName     = "Group " + std::to_string(mNewGroupId);
            entry.isValid       = true;
            mGroupTable.push_back(entry);

            mShowAddGroupDialog = false;
            mNewGroupId++;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            mShowAddGroupDialog = false;
        }

        ImGui::EndPopup();
    }
}

void GroupManagement::RenderRemoveGroupDialog()
{
    if (mShowRemoveGroupDialog && mSelectedGroupIndex >= 0 && mSelectedGroupIndex < static_cast<int>(mGroupTable.size()))
    {
        ImGui::OpenPopup("Remove Group");
    }

    if (ImGui::BeginPopupModal("Remove Group", &mShowRemoveGroupDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (mSelectedGroupIndex >= 0 && mSelectedGroupIndex < static_cast<int>(mGroupTable.size()))
        {
            const auto & entry = mGroupTable[mSelectedGroupIndex];
            ImGui::Text("Remove group %d (%s)?", entry.groupId, entry.groupName.empty() ? "(No name)" : entry.groupName.c_str());
            ImGui::Separator();

            if (ImGui::Button("Remove"))
            {
                mGroupTable.erase(mGroupTable.begin() + mSelectedGroupIndex);
                mShowRemoveGroupDialog = false;
                mSelectedGroupIndex    = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                mShowRemoveGroupDialog = false;
                mSelectedGroupIndex    = -1;
            }
        }

        ImGui::EndPopup();
    }
}

void GroupManagement::SendKeySetWriteCommand(uint16_t groupKeySetId)
{
    // Simplified implementation: directly update local state
    // In a real implementation, this would send the KeySetWrite command through the Matter stack

    GroupKeySetInfo newKeySet;
    newKeySet.groupKeySetId          = groupKeySetId;
    newKeySet.groupKeySecurityPolicy = 0; // TrustFirst policy
    newKeySet.epochStartTime0        = 0;
    newKeySet.epochStartTime1        = 0;
    newKeySet.epochStartTime2        = 0;
    newKeySet.isValid                = true;

    // Check if key set already exists
    for (auto & keySet : mGroupKeySets)
    {
        if (keySet.groupKeySetId == groupKeySetId)
        {
            ChipLogProgress(NotSpecified, "Updated Key Set %d", groupKeySetId);
            return;
        }
    }

    // Add new key set
    mGroupKeySets.push_back(newKeySet);
    ChipLogProgress(NotSpecified, "Added Key Set %d", groupKeySetId);
}

void GroupManagement::SendKeySetRemoveCommand(uint16_t groupKeySetId)
{
    // Simplified implementation: directly update local state
    // In a real implementation, this would send the KeySetRemove command through the Matter stack

    auto it = std::remove_if(mGroupKeySets.begin(), mGroupKeySets.end(),
                             [groupKeySetId](const GroupKeySetInfo & keySet) { return keySet.groupKeySetId == groupKeySetId; });

    if (it != mGroupKeySets.end())
    {
        mGroupKeySets.erase(it, mGroupKeySets.end());
        ChipLogProgress(NotSpecified, "Removed Key Set %d", groupKeySetId);
    }
}

void GroupManagement::SendKeySetReadCommand(uint16_t groupKeySetId)
{
    // Simplified implementation: just log the read request
    // In a real implementation, this would send the KeySetRead command

    ChipLogProgress(NotSpecified, "Key Set Read %d requested", groupKeySetId);
}

void GroupManagement::SendKeySetReadAllIndicesCommand()
{
    // Simplified implementation: refresh current state
    // In a real implementation, this would send the KeySetReadAllIndices command

    ChipLogProgress(NotSpecified, "Key Set Read All Indices requested");
    RefreshGroupKeyData();
}

void GroupManagement::RefreshGroupKeyData()
{
    // For demonstration purposes, we'll maintain the current data
    // In a real implementation, this would query the device for current group key data

    // If no key sets exist, add some sample data for demonstration
    if (mGroupKeySets.empty())
    {
        // Add a default key set for demonstration
        GroupKeySetInfo defaultKeySet;
        defaultKeySet.groupKeySetId          = 1;
        defaultKeySet.groupKeySecurityPolicy = 0; // TrustFirst
        defaultKeySet.epochStartTime0        = 0;
        defaultKeySet.epochStartTime1        = 0;
        defaultKeySet.epochStartTime2        = 0;
        defaultKeySet.isValid                = true;
        mGroupKeySets.push_back(defaultKeySet);
    }
}

std::string GroupManagement::GetSecurityPolicyString(uint8_t policy)
{
    switch (policy)
    {
    case 0:
        return "TrustFirst";
    case 1:
        return "CacheAndSync";
    default:
        return "Unknown";
    }
}

} // namespace Windows
} // namespace Ui
} // namespace example
