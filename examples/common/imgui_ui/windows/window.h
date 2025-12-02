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

#include <string>

namespace example {
namespace Ui {

/**
 * Represents a generic UI window for a ImGUI application.
 *
 * Provides callbacks of loading state, updating state and rendering the
 * actual UI.
 *
 * UI rendering is expected to be done using imgui.
 */
class Window
{
public:
    virtual ~Window() = default;

    // State updates will run in the chip main loop

    virtual void LoadInitialState() {}
    virtual void UpdateState() {}

    // Render the UI
    // MUST use Imgui rendering, generally within a Begin/End block to
    // create a window.
    virtual void Render() = 0;

    // Render content only (without Begin/End window) for tabbed interface
    // Default implementation calls Render() for backward compatibility
    virtual void RenderContent() { Render(); }

    // Get the display name for this window (used in tab titles)
    virtual const char * GetDisplayName() const = 0;

    // Get the device product name for window title (only implemented by BasicInformation windows)
    virtual std::string GetDeviceProductNameForTitle() const { return ""; }
};

} // namespace Ui
} // namespace example
