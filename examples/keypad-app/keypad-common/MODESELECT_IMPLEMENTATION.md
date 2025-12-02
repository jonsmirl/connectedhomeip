# ModeSelect Cluster Implementation for Keypad-App

## Overview

This document describes the ModeSelect cluster implementation for EP2 (Smart Switch endpoint) in the keypad-app. The implementation provides a framework for selecting different operational modes that will control how EP2 commands are processed.

## Architecture

### Mode Definitions

The implementation defines exactly four modes as specified:

| Mode | Value | Label | Description | Semantic Tag |
|------|-------|-------|-------------|--------------|
| None | 0 | "None" | No operation mode | 0x0000 (Generic) |
| Light | 1 | "Light" | Control single light device | 0x0001 (Auto/Single) |
| Light Group | 2 | "Light Group" | Control group of lights | 0x0007 (Max/Group) |
| Button | 3 | "Button" | Simple button press mode | 0x0001 (Quick/Button) |

### Key Components

#### 1. KeypadModeSelect.h/cpp
- **KeypadModeSelectDelegate**: Implements `ModeSelect::SupportedModesManager::Delegate`
- **Mode Options**: Defines the four supported modes with labels and semantic tags
- **Cluster Callbacks**: Handles ModeSelect cluster initialization and commands
- **Global Instance Management**: Manages the ModeSelect cluster instance lifecycle

#### 2. KeypadManager Integration
- **Mode State Tracking**: Added `mCurrentMode` member to track current mode
- **Mode Change Handling**: `HandleModeChange()` method processes mode transitions
- **State Management**: `SetCurrentMode()` and `GetCurrentMode()` for mode access

#### 3. ZclCallbacks.cpp Enhancement
- **Attribute Change Detection**: Added ModeSelect cluster attribute monitoring
- **Command Processing**: Integrated ModeSelect commands with existing cluster callbacks
- **Synchronization**: Ensures mode changes are properly handled across the system

## Implementation Details

### Cluster Initialization

The ModeSelect cluster is initialized only on EP2 (kSwitchEndpointId) via the `emberAfModeSelectClusterInitCallback`:

```cpp
void emberAfModeSelectClusterInitCallback(chip::EndpointId endpointId)
{
    // Only initialize for EP2 (kSwitchEndpointId)
    if (endpointId != KeypadManager::kSwitchEndpointId) return;
    
    // Create delegate and supported modes manager
    gKeypadModeSelectDelegate = new KeypadModeSelectDelegate;
    gKeypadModeSelectInstance = new ModeSelect::SupportedModesManager(endpointId, gKeypadModeSelectDelegate);
    
    // Set initial mode to "None"
    ModeSelect::Attributes::CurrentMode::Set(endpointId, ModeNone);
    KeypadManager::GetInstance().SetCurrentMode(ModeNone);
}
```

### Command Handling

The `emberAfModeSelectClusterChangeToModeCallback` handles ChangeToMode commands:

1. **Validation**: Ensures command is for EP2 and mode is valid (0-3)
2. **State Update**: Updates the CurrentMode attribute
3. **Manager Notification**: Calls `KeypadManager::HandleModeChange()`
4. **Response**: Sends appropriate status response to client

### Mode Change Processing

When a mode change occurs, `KeypadManager::HandleModeChange()` is called:

```cpp
void KeypadManager::HandleModeChange(uint8_t newMode)
{
    // Update internal state
    SetCurrentMode(newMode);
    
    // Log the mode change for verification
    const char* modeNames[] = { "None", "Light", "Light Group", "Button" };
    const char* modeName = (newMode < 4) ? modeNames[newMode] : "Unknown";
    
    ChipLogProgress(AppServer, "KeypadManager: Mode changed to %d (%s)", newMode, modeName);
    
    // TODO: Future implementation will change EP2 command processing behavior
}
```

### Attribute Monitoring

The `MatterPostAttributeChangeCallback` monitors ModeSelect attribute changes:

```cpp
// Handle ModeSelect cluster attribute changes
else if (attributePath.mClusterId == ModeSelect::Id && 
         attributePath.mAttributeId == ModeSelect::Attributes::CurrentMode::Id)
{
    uint8_t mode = *reinterpret_cast<uint8_t *>(value);
    KeypadManager::GetInstance().HandleModeChange(mode);
}
```

## Matter Compliance

### Cluster Configuration
- **Cluster ID**: 0x0050 (ModeSelect)
- **Endpoint**: EP2 only (kSwitchEndpointId)
- **Attributes**: 
  - Description: "Keypad Switch Mode"
  - SupportedModes: Array of 4 mode options
  - CurrentMode: Current selected mode (0-3)
- **Commands**: ChangeToMode

### Semantic Tags
The implementation uses standard Matter semantic tags where applicable:
- **0x0000**: Generic/None for the "None" mode
- **0x0001**: Auto/Single/Quick for operational modes
- **0x0007**: Max/Group for group operations

## Usage Examples

### Reading Current Mode
```bash
chip-tool modeselect read current-mode 1 2
```

### Reading Supported Modes
```bash
chip-tool modeselect read supported-modes 1 2
```

### Changing Mode
```bash
# Change to Light mode (mode 1)
chip-tool modeselect change-to-mode 1 1 2

# Change to Light Group mode (mode 2)
chip-tool modeselect change-to-mode 2 1 2

# Change to Button mode (mode 3)
chip-tool modeselect change-to-mode 3 1 2

# Change to None mode (mode 0)
chip-tool modeselect change-to-mode 0 1 2
```

## Future Enhancements

This implementation provides the foundation for future behavioral changes:

### Mode 0 (None)
- EP2 commands will be ignored or return appropriate status
- No synchronization with EP1

### Mode 1 (Light) - Current Behavior
- EP2 commands control the single light device
- Full bidirectional synchronization with EP1
- Current proxy implementation behavior

### Mode 2 (Light Group) - Future
- EP2 commands will control a group of lights
- Group membership management
- Broadcast commands to group members

### Mode 3 (Button) - Future
- EP2 acts as simple button/switch
- Trigger events or scenes
- Minimal state synchronization

## Integration Points

### Build System
The implementation is integrated into the build via `BUILD.gn`:
```gn
sources = [
  "include/KeypadManager.h",
  "include/KeypadModeSelect.h",
  "src/KeypadManager.cpp",
  "src/KeypadModeSelect.cpp",
  "src/ZclCallbacks.cpp",
]
```

### ZAP Configuration
The ModeSelect cluster is configured in `keypad-app.zap` for EP2 with:
- Server cluster enabled
- ChangeToMode command handling
- Callback attributes for SupportedModes

## Testing Checklist

- [ ] ModeSelect cluster initializes correctly on EP2
- [ ] SupportedModes attribute returns 4 mode options
- [ ] CurrentMode attribute reads/writes correctly
- [ ] ChangeToMode command works for all valid modes (0-3)
- [ ] Invalid mode values are rejected appropriately
- [ ] Mode changes are logged correctly
- [ ] KeypadManager state is updated on mode changes
- [ ] Attribute change callbacks are triggered
- [ ] Commands on EP1 only work for EP1 (not affected by EP2 mode)

## Framework Only

**Important**: This implementation is framework setup only. The actual behavioral differences between modes are not implemented yet. The mode selection capability is established and mode changes are tracked, but EP2 command processing behavior remains the same regardless of the selected mode. Future enhancements will implement the actual mode-specific behaviors.
