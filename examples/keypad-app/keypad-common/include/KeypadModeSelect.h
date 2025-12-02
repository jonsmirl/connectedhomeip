/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace KeypadModeSelect {

// Mode definitions for the keypad switch
const uint8_t ModeNone       = 0; // No operation mode
const uint8_t ModeLight      = 1; // Control single light device
const uint8_t ModeLightGroup = 2; // Control group of lights
const uint8_t ModeButton     = 3; // Simple button press mode

/// This is an application level delegate to handle ModeSelect commands according to the specific business logic.
class KeypadModeSelectDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

    // Semantic tags for each mode (using standard Matter semantic tags where applicable)
    ModeTagStructType modeTagsNone[1]       = { { .value = 0x0000 } }; // Generic/None
    ModeTagStructType modeTagsLight[1]      = { { .value = 0x0001 } }; // Auto/Single
    ModeTagStructType modeTagsLightGroup[1] = { { .value = 0x0007 } }; // Max/Group
    ModeTagStructType modeTagsButton[1]     = { { .value = 0x0001 } }; // Quick/Button

    const detail::Structs::ModeOptionStruct::Type kModeOptions[4] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("None"),
                                                 .mode     = ModeNone,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsNone) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Light"),
                                                 .mode     = ModeLight,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsLight) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Light Group"),
                                                 .mode     = ModeLightGroup,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsLightGroup) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Button"),
                                                 .mode     = ModeButton,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsButton) }
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, chip::app::DataModel::List<ModeTagStructType> & tags) override;

public:
    ~KeypadModeSelectDelegate() override = default;
};

// Global instance management
ModeBase::Instance * Instance();
void Shutdown();

} // namespace KeypadModeSelect

} // namespace Clusters
} // namespace app
} // namespace chip
