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

#include <AppMain.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

// v1.5 binding cluster includes
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>

#include <app/CommandSender.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/InvokeInteraction.h>
// Command cluster includes for actual command sending
// Using cluster objects directly from generated code

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

// Interaction model includes for command sending
// #include <app/InteractionModelEngine.h>
// #include <app/data-model/Encode.h>
// #include <messaging/ExchangeManager.h>

#include <lib/dnssd/Resolver.h>
#include <platform/CHIPDeviceLayer.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui.h>
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/basic_information.h>
#include <imgui_ui/windows/qrcode.h>

#include <cinttypes>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declarations for custom ImGui windows
class LightSwitchBasicInformation;
class LightSwitchControlWindow;
class DeviceConnectionWindow;

// Device Manager for handling command sending to known devices
class LightSwitchDeviceManager
{
public:
    static LightSwitchDeviceManager & GetInstance()
    {
        static LightSwitchDeviceManager instance;
        return instance;
    }

    CHIP_ERROR Init();
    void Shutdown();

    // Command sending to known devices
    CHIP_ERROR SendOnOffCommand(chip::NodeId nodeId, chip::EndpointId endpointId, bool onOff);
    CHIP_ERROR SendLevelCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t level);
    CHIP_ERROR SendColorHueCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t hue);
    CHIP_ERROR SendColorSaturationCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t saturation);
    CHIP_ERROR SendColorTemperatureCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint16_t colorTemp);
    CHIP_ERROR SendIdentifyCommand(chip::NodeId nodeId, chip::EndpointId endpointId);

    // Binding-based commands now handled by BindingManager system

    // Group command sending
    CHIP_ERROR SendGroupOnOffCommand(chip::GroupId groupId, bool onOff);
    CHIP_ERROR SendGroupToggleCommand(chip::GroupId groupId);
    CHIP_ERROR SendGroupIdentifyCommand(chip::GroupId groupId, uint16_t identifyTime);

    // Simple device list for UI testing (in real use, devices would be managed by M2 app)
    std::vector<chip::NodeId> GetKnownDevices() const { return mKnownDevices; }
    void AddKnownDevice(chip::NodeId nodeId);
    void RemoveKnownDevice(chip::NodeId nodeId);

    // Connection management (stub methods for compilation)
    CHIP_ERROR ConnectToDevice(chip::NodeId nodeId)
    {
        ChipLogProgress(AppServer, "ConnectToDevice called for node %" PRIx64, nodeId);
        // TODO: Implement actual connection logic
        return CHIP_NO_ERROR;
    }

    void DisconnectFromDevice(chip::NodeId nodeId)
    {
        ChipLogProgress(AppServer, "DisconnectFromDevice called for node %" PRIx64, nodeId);
        // TODO: Implement actual disconnection logic
    }

private:
    LightSwitchDeviceManager() = default;

    // Simple list of known device node IDs for testing
    std::vector<chip::NodeId> mKnownDevices;

    // Command sending helpers using binding cluster
    template <typename CommandType>
    CHIP_ERROR SendCommandViaBinding(chip::EndpointId localEndpoint, const CommandType & command);

    // Helper method to send commands to specific devices
    template <typename CommandType>
    CHIP_ERROR SendCommandToDevice(chip::NodeId nodeId, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                   const CommandType & command);

    // Helper method to send commands to groups
    template <typename CommandType>
    CHIP_ERROR SendCommandToGroup(chip::GroupId groupId, chip::ClusterId clusterId, const CommandType & command);
};

// Custom BasicInformation window for light switch
class LightSwitchBasicInformation : public example::Ui::Windows::BasicInformation
{
public:
    LightSwitchBasicInformation(chip::EndpointId endpointId) : BasicInformation(endpointId) {}

protected:
    std::string GetDeviceProductName() const override { return "Light Switch"; }
    uint16_t GetDeviceProductID() const override { return 0x8006; }
    std::string GetDeviceNodeLabel() const override { return ""; }
    std::string GetDeviceProductLabel() const override { return "Light Switch"; }
    std::string GetDevicePartNumber() const override { return "SWITCH-001"; }
    std::string GetDeviceSerialNumber() const override { return "SWITCH-123456"; }
    std::string GetDeviceUniqueID() const override { return "SWITCH-UID-123456"; }
};

// Light Switch Control Window
class LightSwitchControlWindow : public example::Ui::Window
{
public:
    LightSwitchControlWindow() = default;

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Light Switch Control"; }

private:
    // Target device information
    chip::NodeId mTargetNodeId         = 0;
    chip::EndpointId mTargetEndpointId = 1;
    bool mDeviceConnected              = false;

    // Control state
    bool mOnOffState         = false;
    uint8_t mLevelValue      = 128;
    uint8_t mHueValue        = 0;
    uint8_t mSaturationValue = 100;
    uint16_t mColorTempValue = 250;
    uint8_t mColorMode       = 0; // 0=HSV, 1=XY, 2=ColorTemp

    // UI state
    bool mShowAdvancedControls = false;
    char mNodeIdInput[32]      = "1";
    char mEndpointInput[8]     = "1";

    // Binding and group UI state
    bool mShowBindingControls   = false;
    char mLocalEndpointInput[8] = "1";
    char mGroupIdInput[8]       = "1";
    uint16_t mIdentifyTime      = 10;

    // Command sending methods
    void SendOnOffCommand(bool onOff);
    void SendLevelCommand(uint8_t level);
    void SendColorHueCommand(uint8_t hue);
    void SendColorSaturationCommand(uint8_t saturation);
    void SendColorTemperatureCommand(uint16_t colorTemp);
    void SendIdentifyCommand();

    // Binding-based command methods
    void SendBoundOnOffCommand(bool onOff);
    void SendBoundToggleCommand();
    void SendBoundIdentifyCommand();

    // Group command methods
    void SendGroupOnOffCommand(bool onOff);
    void SendGroupToggleCommand();
    void SendGroupIdentifyCommand();

    // Connection management
    void ConnectToDevice();
    void DisconnectFromDevice();
};

// Device Connection Window
class DeviceConnectionWindow : public example::Ui::Window
{
public:
    DeviceConnectionWindow() = default;

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Device Connection"; }

private:
    char mNodeIdBuffer[32]      = "1";
    bool mIsConnected           = false;
    chip::NodeId mCurrentNodeId = 0;

    // Simplified - no discovery since M2 app handles that
    std::vector<chip::NodeId> mKnownDevices;
    chip::NodeId mSelectedNodeId = 0;
};

// Device Management Window
class DeviceManagementWindow : public example::Ui::Window
{
public:
    DeviceManagementWindow() = default;

    void UpdateState() override;
    void Render() override;
    void RenderContent() override;
    const char * GetDisplayName() const override { return "Device Management"; }

private:
    // Device management state
    std::vector<chip::NodeId> mKnownDevices;
    char mNodeIdInput[32]      = "1";
    std::string mStatusMessage = "Ready";

    // UI methods
    void AddDevice();
    void RemoveDevice(chip::NodeId nodeId);
    void RefreshDeviceList();
};

#endif // CHIP_IMGUI_ENABLED

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// Binding command data structure for passing context to binding manager
struct BindingCommandData
{
    chip::EndpointId localEndpointId = 1;
    chip::CommandId commandId;
    chip::ClusterId clusterId;
    bool isGroup = false;
};

// Forward declarations for binding handlers
void ProcessOnOffUnicastBindingCommand(CommandId commandId, const Binding::TableEntry & binding,
                                       Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle);
void ProcessOnOffGroupBindingCommand(CommandId commandId, const Binding::TableEntry & binding);
void LightSwitchChangedHandler(const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device, void * context);
void LightSwitchContextReleaseHandler(void * context);
void SwitchWorkerFunction(intptr_t context);

namespace {

// Global instances
#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
std::unique_ptr<LightSwitchControlWindow> gLightSwitchControl;
std::unique_ptr<DeviceManagementWindow> gDeviceManagement;
#endif

} // namespace

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Light Switch App starting...");

    // Initialize device manager
    CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to initialize device manager: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Initialize binding manager (v1.5 API)
    auto & server = chip::Server::GetInstance();
    chip::app::Clusters::Binding::Manager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::app::Clusters::Binding::Manager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    chip::app::Clusters::Binding::Manager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);

    ChipLogProgress(NotSpecified, "Light Switch App initialized");
}

void ApplicationShutdown()
{
    ChipLogProgress(NotSpecified, "Light Switch App shutting down...");
    LightSwitchDeviceManager::GetInstance().Shutdown();
}

#ifdef __NuttX__
// NuttX requires the main function to be defined with C-linkage. However, marking
// the main as extern "C" is not strictly conformant with the C++ standard. Since
// clang >= 20 such code triggers -Wmain warning.
extern "C" {
#endif

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    // Enable tabbed interface
    example::Ui::ImguiUi ui(true, false);

    // Add windows
    ui.AddWindow(std::make_unique<LightSwitchBasicInformation>(chip::EndpointId(0)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());

    // Create and add custom windows
    gLightSwitchControl = std::make_unique<LightSwitchControlWindow>();
    gDeviceManagement   = std::make_unique<DeviceManagementWindow>();

    ui.AddWindow(std::move(gLightSwitchControl));
    ui.AddWindow(std::move(gDeviceManagement));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}

#ifdef __NuttX__
}
#endif

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED

// LightSwitchControlWindow Implementation
void LightSwitchControlWindow::UpdateState()
{
    // Update connection status and device state
    // This would typically query the device controller for current state
}

void LightSwitchControlWindow::Render()
{
    if (ImGui::Begin("Light Switch Control"))
    {
        RenderContent();
    }
    ImGui::End();
}

void LightSwitchControlWindow::RenderContent()
{
    // Matter Light Switch - Binding-based control
    ImGui::Text("Matter Light Switch");
    ImGui::Separator();
    ImGui::Text("Binding-based switch - controls bound devices via Matter binding table");
    ImGui::Spacing();

    ImGui::Text("Light Controls");

    // On/Off control
    if (ImGui::Checkbox("Light On/Off", &mOnOffState))
    {
        SendOnOffCommand(mOnOffState);
    }

    // Level control
    if (ImGui::SliderInt("Brightness", (int *) &mLevelValue, 0, 254))
    {
        SendLevelCommand(mLevelValue);
    }

    // Color mode selection
    const char * colorModes[] = { "Hue/Saturation", "XY", "Color Temperature" };
    if (ImGui::Combo("Color Mode", (int *) &mColorMode, colorModes, 3))
    {
        // Color mode changed - could send mode change command
    }

    // Color controls based on mode
    if (mColorMode == 0) // HSV
    {
        if (ImGui::SliderInt("Hue", (int *) &mHueValue, 0, 254))
        {
            SendColorHueCommand(mHueValue);
        }
        if (ImGui::SliderInt("Saturation", (int *) &mSaturationValue, 0, 254))
        {
            SendColorSaturationCommand(mSaturationValue);
        }
    }
    else if (mColorMode == 2) // Color Temperature
    {
        if (ImGui::SliderInt("Color Temperature", (int *) &mColorTempValue, 153, 500))
        {
            SendColorTemperatureCommand(mColorTempValue);
        }
    }

    ImGui::Spacing();

    // Additional controls
    if (ImGui::Button("Identify"))
    {
        SendIdentifyCommand();
    }

    ImGui::Checkbox("Show Advanced Controls", &mShowAdvancedControls);

    if (mShowAdvancedControls)
    {
        ImGui::Separator();
        ImGui::Text("Advanced Controls");

        if (ImGui::Button("Scene Recall 1"))
        {
            // Send scene recall command for scene 1
        }
        if (ImGui::Button("Scene Recall 2"))
        {
            // Send scene recall command for scene 2
        }
    }

    // ImGui::EndDisabled(); // Disabled for transmit-only switch behavior

    // Binding and Group Controls (always enabled - following Espressif pattern)
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Binding & Group Controls");

    ImGui::Checkbox("Show Binding Controls", &mShowBindingControls);

    if (mShowBindingControls)
    {
        ImGui::Separator();

        // Local endpoint input
        ImGui::InputText("Local Endpoint", mLocalEndpointInput, sizeof(mLocalEndpointInput));

        // Binding-based commands
        ImGui::Text("Bound Device Commands:");
        if (ImGui::Button("Bound ON"))
        {
            SendBoundOnOffCommand(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Bound OFF"))
        {
            SendBoundOnOffCommand(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Bound Toggle"))
        {
            SendBoundToggleCommand();
        }

        ImGui::SliderInt("Identify Time", (int *) &mIdentifyTime, 1, 60);
        if (ImGui::Button("Bound Identify"))
        {
            SendBoundIdentifyCommand();
        }

        ImGui::Spacing();

        // Group commands
        ImGui::InputText("Group ID", mGroupIdInput, sizeof(mGroupIdInput));
        ImGui::Text("Group Commands:");
        if (ImGui::Button("Group ON"))
        {
            SendGroupOnOffCommand(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Group OFF"))
        {
            SendGroupOnOffCommand(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Group Toggle"))
        {
            SendGroupToggleCommand();
        }

        if (ImGui::Button("Group Identify"))
        {
            SendGroupIdentifyCommand();
        }
    }
}

void LightSwitchControlWindow::SendOnOffCommand(bool onOff)
{
    ChipLogProgress(NotSpecified, "UI OnOff control changed: %s", onOff ? "ON" : "OFF");

    // For transmit-only switch: UI works locally, commands sent via bindings
    // Create binding command data and send via binding manager
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = onOff ? Clusters::OnOff::Commands::On::Id : Clusters::OnOff::Commands::Off::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->localEndpointId     = 1;     // Switch endpoint
    data->isGroup             = false; // Send to individual devices

    // Schedule work to send command via binding manager
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchControlWindow::SendLevelCommand(uint8_t level)
{
    ChipLogProgress(NotSpecified, "UI Level control changed: %d", level);

    // For transmit-only switch: UI works locally, commands sent via bindings
    // Level commands would be sent to bound devices via binding table
    ChipLogProgress(NotSpecified, "Level command (%d) would be sent to bound devices", level);

    // TODO: Implement level control via binding table when binding cluster supports it
    // For now, just log the local UI change
}

void LightSwitchControlWindow::SendColorHueCommand(uint8_t hue)
{
    ChipLogProgress(NotSpecified, "UI Color Hue control changed: %d", hue);

    // For transmit-only switch: UI works locally, commands sent via bindings
    // Color commands would be sent to bound devices via binding table
    ChipLogProgress(NotSpecified, "Color Hue command (%d) would be sent to bound devices", hue);

    // TODO: Implement color control via binding table when binding cluster supports it
    // For now, just log the local UI change
}

void LightSwitchControlWindow::SendColorSaturationCommand(uint8_t saturation)
{
    ChipLogProgress(NotSpecified, "UI Color Saturation control changed: %d", saturation);

    // For transmit-only switch: UI works locally, commands sent via bindings
    // Color commands would be sent to bound devices via binding table
    ChipLogProgress(NotSpecified, "Color Saturation command (%d) would be sent to bound devices", saturation);

    // TODO: Implement color control via binding table when binding cluster supports it
    // For now, just log the local UI change
}

void LightSwitchControlWindow::SendColorTemperatureCommand(uint16_t colorTemp)
{
    ChipLogProgress(NotSpecified, "UI Color Temperature control changed: %d", colorTemp);

    // For transmit-only switch: UI works locally, commands sent via bindings
    // Color commands would be sent to bound devices via binding table
    ChipLogProgress(NotSpecified, "Color Temperature command (%d) would be sent to bound devices", colorTemp);

    // TODO: Implement color control via binding table when binding cluster supports it
    // For now, just log the local UI change
}

void LightSwitchControlWindow::SendIdentifyCommand()
{
    ChipLogProgress(NotSpecified, "UI Identify button clicked");

    // For transmit-only switch: UI works locally, commands sent via bindings
    // TODO: Implement Identify command via binding manager
    // For now, just log the command - Identify commands need special parameter handling
    ChipLogProgress(NotSpecified, "Identify command (10 seconds) would be sent to bound devices from endpoint 1");
}

void LightSwitchControlWindow::ConnectToDevice()
{
    mTargetNodeId     = static_cast<chip::NodeId>(strtoul(mNodeIdInput, nullptr, 10));
    mTargetEndpointId = static_cast<chip::EndpointId>(strtoul(mEndpointInput, nullptr, 10));

    ChipLogProgress(NotSpecified, "Connecting to device NodeId: %" PRIu64 ", Endpoint: %d", mTargetNodeId, mTargetEndpointId);

    CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().ConnectToDevice(mTargetNodeId);
    if (err == CHIP_NO_ERROR)
    {
        mDeviceConnected = true;
        ChipLogProgress(NotSpecified, "Successfully connected to device");
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to connect to device: %" CHIP_ERROR_FORMAT, err.Format());
        mDeviceConnected = false;
    }
}

void LightSwitchControlWindow::DisconnectFromDevice()
{
    ChipLogProgress(NotSpecified, "Disconnecting from device");
    if (mTargetNodeId != 0)
    {
        LightSwitchDeviceManager::GetInstance().DisconnectFromDevice(mTargetNodeId);
    }
    mDeviceConnected = false;
}

// Binding-based UI command implementations
void LightSwitchControlWindow::SendBoundOnOffCommand(bool onOff)
{
    chip::EndpointId localEndpoint = static_cast<chip::EndpointId>(strtoul(mLocalEndpointInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending bound OnOff command from endpoint %d: %s", localEndpoint, onOff ? "ON" : "OFF");

    // Create binding command data and send via binding manager
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = onOff ? Clusters::OnOff::Commands::On::Id : Clusters::OnOff::Commands::Off::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->localEndpointId     = localEndpoint;
    data->isGroup             = false; // Send to individual devices

    // Schedule work to send command via binding manager
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchControlWindow::SendBoundToggleCommand()
{
    chip::EndpointId localEndpoint = static_cast<chip::EndpointId>(strtoul(mLocalEndpointInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending bound Toggle command from endpoint %d", localEndpoint);

    // Create binding command data and send via binding manager
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::Toggle::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->localEndpointId     = localEndpoint;
    data->isGroup             = false; // Send to individual devices

    // Schedule work to send command via binding manager
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchControlWindow::SendBoundIdentifyCommand()
{
    chip::EndpointId localEndpoint = static_cast<chip::EndpointId>(strtoul(mLocalEndpointInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending bound Identify command from endpoint %d, time=%d", localEndpoint, mIdentifyTime);

    // TODO: Implement Identify command via binding manager
    // For now, just log the command - Identify commands need special parameter handling
    ChipLogProgress(NotSpecified, "Identify command (time=%d) would be sent to bound devices from endpoint %d", mIdentifyTime,
                    localEndpoint);
}

// Group command UI implementations
void LightSwitchControlWindow::SendGroupOnOffCommand(bool onOff)
{
    chip::GroupId groupId = static_cast<chip::GroupId>(strtoul(mGroupIdInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending group OnOff command to group %d: %s", groupId, onOff ? "ON" : "OFF");

    CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().SendGroupOnOffCommand(groupId, onOff);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send group OnOff command: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void LightSwitchControlWindow::SendGroupToggleCommand()
{
    chip::GroupId groupId = static_cast<chip::GroupId>(strtoul(mGroupIdInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending group Toggle command to group %d", groupId);

    CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().SendGroupToggleCommand(groupId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send group Toggle command: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void LightSwitchControlWindow::SendGroupIdentifyCommand()
{
    chip::GroupId groupId = static_cast<chip::GroupId>(strtoul(mGroupIdInput, nullptr, 10));
    ChipLogProgress(NotSpecified, "Sending group Identify command to group %d, time=%d", groupId, mIdentifyTime);

    CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().SendGroupIdentifyCommand(groupId, mIdentifyTime);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send group Identify command: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

// DeviceConnectionWindow Implementation
void DeviceConnectionWindow::UpdateState()
{
    // Update discovery and connection state
    // This would typically query the device controller for discovered devices
}

void DeviceConnectionWindow::Render()
{
    if (ImGui::Begin("Device Connection"))
    {
        RenderContent();
    }
    ImGui::End();
}

void DeviceConnectionWindow::RenderContent()
{
    ImGui::Text("Device Connection Management");
    ImGui::Separator();

    // Note: Discovery and commissioning is handled by M2 app
    ImGui::Text("Note: Device discovery and commissioning is handled by the M2 app.");
    ImGui::Text("This window is for connecting to already commissioned devices.");

    ImGui::Spacing();
    ImGui::Separator();

    // Manual node ID input for testing
    ImGui::Text("Connect to Device by Node ID:");
    ImGui::InputText("Node ID", mNodeIdBuffer, sizeof(mNodeIdBuffer));

    if (ImGui::Button("Connect"))
    {
        chip::NodeId nodeId = static_cast<chip::NodeId>(strtoull(mNodeIdBuffer, nullptr, 10));
        if (nodeId > 0)
        {
            CHIP_ERROR err = LightSwitchDeviceManager::GetInstance().ConnectToDevice(nodeId);
            if (err == CHIP_NO_ERROR)
            {
                mCurrentNodeId = nodeId;
                mIsConnected   = true;
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Disconnect") && mIsConnected)
    {
        LightSwitchDeviceManager::GetInstance().DisconnectFromDevice(mCurrentNodeId);
        mIsConnected   = false;
        mCurrentNodeId = 0;
    }

    ImGui::Spacing();
    ImGui::Text("Connection Status: %s", mIsConnected ? "Connected" : "Disconnected");
    if (mIsConnected)
    {
        ImGui::Text("Connected to Node ID: %" PRIu64, mCurrentNodeId);
    }
}

// DeviceManagementWindow Implementation
void DeviceManagementWindow::UpdateState()
{
    // Update device list from device manager
    mKnownDevices = LightSwitchDeviceManager::GetInstance().GetKnownDevices();
}

void DeviceManagementWindow::Render()
{
    if (ImGui::Begin("Device Management"))
    {
        RenderContent();
    }
    ImGui::End();
}

void DeviceManagementWindow::RenderContent()
{
    ImGui::Text("Device Management");
    ImGui::Separator();

    // Add device section
    ImGui::Text("Add Device:");
    ImGui::InputText("Node ID", mNodeIdInput, sizeof(mNodeIdInput));

    if (ImGui::Button("Add Device"))
    {
        chip::NodeId nodeId = static_cast<chip::NodeId>(strtoull(mNodeIdInput, nullptr, 10));
        if (nodeId > 0)
        {
            LightSwitchDeviceManager::GetInstance().AddKnownDevice(nodeId);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Known devices list
    ImGui::Text("Known Devices:");

    if (mKnownDevices.empty())
    {
        ImGui::Text("No devices configured");
    }
    else
    {
        for (size_t i = 0; i < mKnownDevices.size(); i++)
        {
            char deviceLabel[64];
            snprintf(deviceLabel, sizeof(deviceLabel), "Device %" PRIu64, mKnownDevices[i]);

            ImGui::Text("%s", deviceLabel);
            ImGui::SameLine();

            char removeButtonId[64];
            snprintf(removeButtonId, sizeof(removeButtonId), "Remove##%zu", i);

            if (ImGui::Button(removeButtonId))
            {
                LightSwitchDeviceManager::GetInstance().RemoveKnownDevice(mKnownDevices[i]);
            }
        }
    }
}

// LightSwitchDeviceManager Implementation
CHIP_ERROR LightSwitchDeviceManager::Init()
{
    ChipLogProgress(AppServer, "Initializing LightSwitchDeviceManager");
    // TODO: Initialize device controller and other components
    return CHIP_NO_ERROR;
}

void LightSwitchDeviceManager::Shutdown()
{
    ChipLogProgress(AppServer, "Shutting down LightSwitchDeviceManager");
    // TODO: Cleanup device controller and other components
}

void LightSwitchDeviceManager::AddKnownDevice(chip::NodeId nodeId)
{
    auto it = std::find(mKnownDevices.begin(), mKnownDevices.end(), nodeId);
    if (it == mKnownDevices.end())
    {
        mKnownDevices.push_back(nodeId);
        ChipLogProgress(AppServer, "Added known device %" PRIu64, nodeId);
    }
}

void LightSwitchDeviceManager::RemoveKnownDevice(chip::NodeId nodeId)
{
    auto it = std::find(mKnownDevices.begin(), mKnownDevices.end(), nodeId);
    if (it != mKnownDevices.end())
    {
        mKnownDevices.erase(it);
        ChipLogProgress(NotSpecified, "Removed known device %" PRIu64, nodeId);
    }
}

CHIP_ERROR LightSwitchDeviceManager::SendOnOffCommand(chip::NodeId nodeId, chip::EndpointId endpointId, bool onOff)
{
    ChipLogProgress(NotSpecified, "Sending OnOff command to device %" PRIu64 ", endpoint %d: %s", nodeId, endpointId,
                    onOff ? "ON" : "OFF");

    // UI controls work locally - command would be sent to device via Matter controller
    ChipLogProgress(NotSpecified, "OnOff command (%s) would be sent to device %" PRIu64 " endpoint %d via binding cluster",
                    onOff ? "ON" : "OFF", nodeId, endpointId);

    // TODO: Implement actual command sending using Matter controller
    // This would use the binding cluster to route commands to bound devices

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendLevelCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t level)
{
    ChipLogProgress(NotSpecified, "Sending Level command to device %" PRIu64 ", endpoint %d: %d", nodeId, endpointId, level);

    // UI controls work locally - command would be sent to device via Matter controller
    ChipLogProgress(NotSpecified, "Level command (level=%d) would be sent to device %" PRIu64 " endpoint %d via binding cluster",
                    level, nodeId, endpointId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendColorHueCommand(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t hue)
{
    ChipLogProgress(NotSpecified, "Sending Color Hue command to device %" PRIu64 ", endpoint %d: %d", nodeId, endpointId, hue);

    ChipLogProgress(NotSpecified, "Color Hue command (hue=%d) would be sent to device %" PRIu64 " endpoint %d", hue, nodeId,
                    endpointId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendColorSaturationCommand(chip::NodeId nodeId, chip::EndpointId endpointId,
                                                                uint8_t saturation)
{
    ChipLogProgress(NotSpecified, "Sending Color Saturation command to device %" PRIu64 ", endpoint %d: %d", nodeId, endpointId,
                    saturation);

    ChipLogProgress(NotSpecified, "Color Saturation command (saturation=%d) would be sent to device %" PRIu64 " endpoint %d",
                    saturation, nodeId, endpointId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendColorTemperatureCommand(chip::NodeId nodeId, chip::EndpointId endpointId,
                                                                 uint16_t colorTemp)
{
    ChipLogProgress(NotSpecified, "Sending Color Temperature command to device %" PRIu64 ", endpoint %d: %d", nodeId, endpointId,
                    colorTemp);

    ChipLogProgress(NotSpecified, "Color Temperature command (temp=%d) would be sent to device %" PRIu64 " endpoint %d", colorTemp,
                    nodeId, endpointId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendIdentifyCommand(chip::NodeId nodeId, chip::EndpointId endpointId)
{
    ChipLogProgress(NotSpecified, "Sending Identify command to device %" PRIu64 ", endpoint %d", nodeId, endpointId);

    ChipLogProgress(NotSpecified, "Identify command (10 seconds) would be sent to device %" PRIu64 " endpoint %d", nodeId,
                    endpointId);

    return CHIP_NO_ERROR;
}

// Old binding methods removed - now using proper BindingManager system

// Group command implementations
CHIP_ERROR LightSwitchDeviceManager::SendGroupOnOffCommand(chip::GroupId groupId, bool onOff)
{
    ChipLogProgress(NotSpecified, "Sending group OnOff command to group %d: %s", groupId, onOff ? "ON" : "OFF");

    ChipLogProgress(NotSpecified, "Group OnOff command (%s) would be sent to group %d via groups cluster", onOff ? "ON" : "OFF",
                    groupId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendGroupToggleCommand(chip::GroupId groupId)
{
    ChipLogProgress(NotSpecified, "Sending group Toggle command to group %d", groupId);

    ChipLogProgress(NotSpecified, "Group Toggle command would be sent to group %d via groups cluster", groupId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LightSwitchDeviceManager::SendGroupIdentifyCommand(chip::GroupId groupId, uint16_t identifyTime)
{
    ChipLogProgress(NotSpecified, "Sending group Identify command to group %d, time=%d", groupId, identifyTime);

    ChipLogProgress(NotSpecified, "Group Identify command (time=%d) would be sent to group %d via groups cluster", identifyTime,
                    groupId);

    return CHIP_NO_ERROR;
}

// Helper methods removed - commands are now logged directly in each method

// Binding handler implementations
void ProcessOnOffUnicastBindingCommand(CommandId commandId, const Binding::TableEntry & binding,
                                       Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, onCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, offCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessOnOffGroupBindingCommand(CommandId commandId, const Binding::TableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, toggleCommand);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onCommand);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, offCommand);
        break;
    }
}

void LightSwitchChangedHandler(const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (binding.type == Binding::MATTER_MULTICAST_BINDING && data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            ProcessOnOffGroupBindingCommand(data->commandId, binding);
            break;
        }
    }
    else if (binding.type == Binding::MATTER_UNICAST_BINDING && !data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            ProcessOnOffUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(),
                                              peer_device->GetSecureSession().Value());
            break;
        }
    }
}

void LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "Invalid context for Light switch context release handler"));
    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);
    Binding::Manager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId, static_cast<void *>(data));
}

#endif // CHIP_IMGUI_ENABLED
