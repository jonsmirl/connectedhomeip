/*
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
#pragma once

#include "window.h"

#include <string>
#include <vector>

namespace example {
namespace Ui {
namespace Windows {

/**
 * Shows Color Control debug logs in real-time
 */
class ColorControlDebug : public Window
{
public:
    ColorControlDebug() = default;

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Color Control Debug"; }

private:
    std::vector<std::string> mLogLines;
    bool mAutoScroll = true;
    bool mShowTimestamps = true;
    
    void LoadLogFile();
    void ClearLogs();
};

} // namespace Windows
} // namespace Ui
} // namespace example
