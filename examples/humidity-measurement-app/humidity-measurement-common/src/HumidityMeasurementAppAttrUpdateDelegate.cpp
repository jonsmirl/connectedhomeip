/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "HumidityMeasurementAppAttrUpdateDelegate.h"

#include "HumidityMeasurementManager.h"

#include <json/json.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::DeviceLayer;

class HumidityAttrUpdateHandler
{
public:
    static HumidityAttrUpdateHandler * FromJSON(const char * json);
    static void HandleCommand(intptr_t context);

    explicit HumidityAttrUpdateHandler(Json::Value value) : mJsonValue(std::move(value)) {}

private:
    Json::Value mJsonValue;
};

HumidityAttrUpdateHandler * HumidityAttrUpdateHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(json, value) || !value.isMember("Name") || value["Name"].asString() != "Humidity")
    {
        return nullptr;
    }

    return Platform::New<HumidityAttrUpdateHandler>(std::move(value));
}

void HumidityAttrUpdateHandler::HandleCommand(intptr_t context)
{
    auto * self = reinterpret_cast<HumidityAttrUpdateHandler *>(context);
    int16_t temperature = static_cast<int16_t>(self->mJsonValue["NewValue"].asInt());

    HumidityMeasurementManager::GetInstance()->OnHumidityChanged(temperature);
    Platform::Delete(self);
}

void HumidityMeasurementAppAttrUpdateDelegate::OnEventCommandReceived(const char * json)
{
    auto * handler = HumidityAttrUpdateHandler::FromJSON(json);
    if (handler)
    {
        PlatformMgr().ScheduleWork(HumidityAttrUpdateHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to parse incoming JSON command.");
    }
}
