# Linux Light Switch App

This is a Linux implementation of a Matter light switch application that follows the same command patterns as the Espressif ESP-Matter light switch sample.

## Overview

The Linux light switch app provides a graphical user interface (ImGui-based) for controlling Matter lighting devices through binding and group functionality. This application is designed to work with devices that have already been commissioned and bound by the M2 app.

## Key Features

### Binding Support
- Send commands to bound devices using the binding cluster
- Support for both unicast (individual device) and multicast (group) bindings
- Commands are sent based on the binding table entries

### Group Support  
- Send commands to groups of devices
- Multicast command functionality for controlling multiple devices simultaneously

### Supported Commands
Following the Espressif light switch pattern, the app supports:

#### OnOff Cluster Commands
- **On** (0x01) - Turn device/group on
- **Off** (0x00) - Turn device/group off  
- **Toggle** (0x02) - Toggle device/group state

#### Identify Cluster Commands
- **Identify** (0x00) - Make device/group identify itself

## User Interface

The application provides three main windows:

### 1. Light Switch Control Window
- **Direct Device Control**: Connect to specific devices by Node ID and Endpoint
- **Binding Controls**: Send commands to bound devices from a local endpoint
- **Group Controls**: Send commands to device groups
- **Advanced Controls**: Scene recall and other advanced features

### 2. Device Connection Window  
- Simple device connection management
- Note: Device discovery and commissioning is handled by the M2 app

### 3. Device Management Window
- Add/remove known devices for testing
- View list of known device Node IDs

## Command Patterns

The application implements the same command patterns as the Espressif sample:

### Bound Device Commands
```cpp
// Send OnOff command to all devices bound to local endpoint 1
SendBoundOnOffCommand(1, true);  // Turn on
SendBoundOnOffCommand(1, false); // Turn off
SendBoundToggleCommand(1);       // Toggle

// Send Identify command to bound devices
SendBoundIdentifyCommand(1, 10); // Identify for 10 seconds
```

### Group Commands  
```cpp
// Send OnOff command to group 257
SendGroupOnOffCommand(257, true);  // Turn group on
SendGroupToggleCommand(257);       // Toggle group
SendGroupIdentifyCommand(257, 5);  // Group identify for 5 seconds
```

## Architecture

### Integration with M2 App
- The M2 app handles device commissioning and binding setup
- This light switch app only sends commands to already-commissioned devices
- Binding table is managed externally and read by this application

### Binding Cluster Integration
- Uses the Matter binding table to determine target devices/groups
- Iterates through binding entries to find matches for local endpoints
- Supports both MATTER_UNICAST_BINDING and MATTER_MULTICAST_BINDING types

### Command Flow
1. User interacts with UI controls
2. UI calls appropriate command method
3. Command method checks binding table for target devices/groups
4. Commands are sent via Matter controller to target devices

## Building

```bash
cd examples/light-switch-app/linux
gn gen out/debug
ninja -C out/debug
```

## Running

```bash
./out/debug/chip-light-switch-app
```

The application will start with the ImGui interface. Use the "Show Binding Controls" checkbox to access binding and group functionality.

## Implementation Notes

### Current Status
- ✅ UI framework and basic structure implemented
- ✅ Binding table integration for command routing
- ✅ Group command support structure
- ⚠️ Command sending currently logs actions (demo mode)
- ⚠️ Real Matter controller integration needed for actual device communication

### Next Steps for Full Implementation
1. **Implement Real Controller Integration**: Replace stub command methods with actual Matter controller calls
2. **Add Binding Cluster Support**: Implement actual binding cluster functionality for device-to-device communication  
3. **Integrate with M2 App**: Add proper interfaces for receiving commissioned device information
4. **Add Error Handling**: Implement proper error reporting and status feedback
5. **Testing**: Test with real Matter devices and binding scenarios

## Comparison with Espressif Sample

This Linux implementation provides the same core functionality as the Espressif ESP-Matter light switch:

| Feature | Espressif Sample | Linux Implementation |
|---------|------------------|---------------------|
| Bound device commands | ✅ Console commands | ✅ GUI controls |
| Group commands | ✅ Console commands | ✅ GUI controls |
| OnOff cluster | ✅ | ✅ |
| Identify cluster | ✅ | ✅ |
| Binding table integration | ✅ | ✅ |
| Physical button | ✅ | ➖ (GUI only) |
| Auto-subscription | ✅ | ⚠️ (planned) |

The key difference is that the Espressif sample uses console commands while this Linux version provides a graphical interface, but both follow the same underlying command patterns and binding mechanisms.
