/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "spiffs-update-server.h"

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <clusters/LowpanSpiffsUpdate/ClusterId.h>
#include <clusters/LowpanSpiffsUpdate/CommandIds.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/core/TLV.h>
#include <tracing/macros.h>

// External implementation in LowpanSpiffsUpdate.cpp
namespace chip {
namespace app {
namespace Clusters {
namespace LowpanSpiffsUpdate {
extern CHIP_ERROR CheckForUpdate(const char * supabaseUrl);
extern CHIP_ERROR ForceUpdate(const char * supabaseUrl);
extern CHIP_ERROR SetDeviceToken(const char * jwt, uint64_t nodeId);
} // namespace LowpanSpiffsUpdate
} // namespace Clusters
} // namespace app
} // namespace chip

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::TLV;
using Protocols::InteractionModel::Status;

// Manual command dispatcher since we don't have ZAP-generated command parsing
bool emberAfLowpanSpiffsUpdateClusterServerCommandDispatchCallback(
    CommandHandler * commandObj,
    const ConcreteCommandPath & commandPath,
    TLVReader & commandDataReader)
{
    MATTER_TRACE_SCOPE("Dispatch", "LowpanSpiffsUpdate");

    if (commandPath.mClusterId != LowpanSpiffsUpdate::Id)
    {
        return false;
    }

    ChipLogProgress(Zcl, "LowpanSpiffsUpdate: Command %u received on endpoint %u",
                    (unsigned)commandPath.mCommandId, (unsigned)commandPath.mEndpointId);

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (commandPath.mCommandId)
    {
    case LowpanSpiffsUpdate::Commands::CheckForUpdate::Id: {
        // Parse supabaseUrl from TLV (tag 0)
        char url[256] = "";
        TLVType outerType;
        err = commandDataReader.EnterContainer(outerType);
        if (err == CHIP_NO_ERROR)
        {
            while ((err = commandDataReader.Next()) == CHIP_NO_ERROR)
            {
                Tag tag = commandDataReader.GetTag();
                if (IsContextTag(tag) && TagNumFromTag(tag) == 0)
                {
                    CharSpan urlSpan;
                    err = commandDataReader.Get(urlSpan);
                    if (err == CHIP_NO_ERROR)
                    {
                        size_t len = urlSpan.size() < sizeof(url) - 1 ? urlSpan.size() : sizeof(url) - 1;
                        memcpy(url, urlSpan.data(), len);
                        url[len] = '\0';
                    }
                }
            }
            commandDataReader.ExitContainer(outerType);
        }

        err = LowpanSpiffsUpdate::CheckForUpdate(url);
        commandObj->AddStatus(commandPath, err == CHIP_NO_ERROR ? Status::Success : Status::Failure);
        return true;
    }

    case LowpanSpiffsUpdate::Commands::ForceUpdate::Id: {
        // Parse supabaseUrl from TLV (tag 0)
        char url[256] = "";
        TLVType outerType;
        err = commandDataReader.EnterContainer(outerType);
        if (err == CHIP_NO_ERROR)
        {
            while ((err = commandDataReader.Next()) == CHIP_NO_ERROR)
            {
                Tag tag = commandDataReader.GetTag();
                if (IsContextTag(tag) && TagNumFromTag(tag) == 0)
                {
                    CharSpan urlSpan;
                    err = commandDataReader.Get(urlSpan);
                    if (err == CHIP_NO_ERROR)
                    {
                        size_t len = urlSpan.size() < sizeof(url) - 1 ? urlSpan.size() : sizeof(url) - 1;
                        memcpy(url, urlSpan.data(), len);
                        url[len] = '\0';
                    }
                }
            }
            commandDataReader.ExitContainer(outerType);
        }

        err = LowpanSpiffsUpdate::ForceUpdate(url);
        commandObj->AddStatus(commandPath, err == CHIP_NO_ERROR ? Status::Success : Status::Failure);
        return true;
    }

    case LowpanSpiffsUpdate::Commands::GetStatus::Id: {
        // No parameters needed
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }

    case LowpanSpiffsUpdate::Commands::SetDeviceToken::Id: {
        // Parse jwt (tag 0) and nodeId (tag 1) from TLV
        char jwt[2048] = "";
        uint64_t nodeId = 0;

        TLVType outerType;
        err = commandDataReader.EnterContainer(outerType);
        if (err == CHIP_NO_ERROR)
        {
            while ((err = commandDataReader.Next()) == CHIP_NO_ERROR)
            {
                Tag tag = commandDataReader.GetTag();
                if (IsContextTag(tag))
                {
                    uint32_t tagNum = TagNumFromTag(tag);
                    if (tagNum == 0)
                    {
                        CharSpan jwtSpan;
                        if (commandDataReader.Get(jwtSpan) == CHIP_NO_ERROR)
                        {
                            size_t len = jwtSpan.size() < sizeof(jwt) - 1 ? jwtSpan.size() : sizeof(jwt) - 1;
                            memcpy(jwt, jwtSpan.data(), len);
                            jwt[len] = '\0';
                        }
                    }
                    else if (tagNum == 1)
                    {
                        commandDataReader.Get(nodeId);
                    }
                }
            }
            commandDataReader.ExitContainer(outerType);
        }

        err = LowpanSpiffsUpdate::SetDeviceToken(jwt, nodeId);
        commandObj->AddStatus(commandPath, err == CHIP_NO_ERROR ? Status::Success : Status::Failure);
        return true;
    }

    default:
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return true;
    }

    return false;
}

void MatterLowpanSpiffsUpdatePluginServerInitCallback() {}

void MatterLowpanSpiffsUpdatePluginServerShutdownCallback() {}
