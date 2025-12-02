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

#include "color_control_debug.h"

#include <imgui.h>
#include <fstream>
#include <sstream>

namespace example {
namespace Ui {
namespace Windows {

void ColorControlDebug::UpdateState()
{
    LoadLogFile();
}

void ColorControlDebug::LoadLogFile()
{
    static size_t lastFileSize = 0;
    
    std::ifstream file("/tmp/lighting_app_color_debug.log");
    if (!file.is_open())
    {
        // File doesn't exist yet, clear logs if we had any
        if (!mLogLines.empty())
        {
            mLogLines.clear();
        }
        return;
    }
    
    // Get file size to check if it changed
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Only reload if file size changed
    if (fileSize != lastFileSize)
    {
        lastFileSize = fileSize;
        mLogLines.clear();
        
        std::string line;
        while (std::getline(file, line))
        {
            mLogLines.push_back(line);
        }
    }
}

void ColorControlDebug::ClearLogs()
{
    mLogLines.clear();
    // Also clear the log file
    std::ofstream file("/tmp/lighting_app_color_debug.log", std::ios::trunc);
    file.close();
}

void ColorControlDebug::Render()
{
    ImGui::Begin("Color Control Debug");
    RenderContent();
    ImGui::End();
}

void ColorControlDebug::RenderContent()
{
    // Control buttons
    if (ImGui::Button("Clear Logs"))
    {
        ClearLogs();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &mAutoScroll);
    ImGui::SameLine();
    ImGui::Checkbox("Show timestamps", &mShowTimestamps);
    
    ImGui::Separator();
    
    // Log display area
    ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (mLogLines.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No Color Control debug logs yet...");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Send RGB/HSV/XY commands to see debug output here.");
    }
    else
    {
        for (const auto& line : mLogLines)
        {
            // Color code different types of messages
            if (line.find("COMPUTE_PWM_FROM_HSV") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", line.c_str()); // Green for HSV
            }
            else if (line.find("COMPUTE_PWM_FROM_XY") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f), "%s", line.c_str()); // Blue for XY
            }
            else if (line.find("Failed") != std::string::npos || line.find("ERROR") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", line.c_str()); // Red for errors
            }
            else if (line.find("Successfully") != std::string::npos)
            {
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "%s", line.c_str()); // Dark green for success
            }
            else
            {
                ImGui::Text("%s", line.c_str()); // Default color
            }
        }
    }
    
    // Auto-scroll to bottom
    if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
    
    // Status info
    ImGui::Separator();
    ImGui::Text("Log lines: %zu", mLogLines.size());
    ImGui::SameLine();
    ImGui::Text("| Log file: /tmp/lighting_app_color_debug.log");
}

} // namespace Windows
} // namespace Ui
} // namespace example
