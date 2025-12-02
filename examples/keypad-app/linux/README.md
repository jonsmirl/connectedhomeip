# CHIP Linux Keypad App Example

The Keypad App is a dual-endpoint Matter device that combines the functionality of both a lighting device and a light switch device in a single application. This demonstrates how to implement a multi-endpoint device using the Matter/CHIP framework.

## Device Architecture

The Keypad device implements two distinct endpoints:

### Endpoint 1: Light Device (Server)
- **Device Type**: Extended Color Light (0x010C)
- **Clusters**: 
  - OnOff Server
  - LevelControl Server  
  - ColorControl Server
  - Identify Server
  - Groups Server
  - Scenes Server
  - Basic Information
  - Descriptor

### Endpoint 2: Light Switch Device (Client)
- **Device Type**: Color Dimmer Switch (0x0105)
- **Clusters**:
  - OnOff Client
  - LevelControl Client
  - ColorControl Client
  - Identify Client
  - Groups Client
  - Scenes Client
  - Binding
  - Basic Information
  - Descriptor

## Concept

The "Keypad" concept represents a physical device that can both:
1. **Act as a light** - responding to commands from other devices
2. **Control other lights** - sending commands to bound devices

This is useful for scenarios like:
- A wall switch that has built-in lighting
- A smart lamp that can also control other lamps
- A bridge device that provides both local lighting and remote control

## Building

### Prerequisites

- Linux development environment
- CHIP/Matter SDK properly set up
- GN build system
- ImGui dependencies (for UI)

### Build Commands

```bash
# From the connectedhomeip root directory

# Standard build
gn gen out/debug --args='import("//examples/keypad-app/linux/args.gni")'
ninja -C out/debug chip-keypad-app

# Build with PW RPC support
gn gen out/debug --args='import("//examples/keypad-app/linux/with_pw_rpc.gni")'
ninja -C out/debug chip-keypad-app
```

### Build Arguments

The keypad app supports the following build arguments in `args.gni`:

- `chip_examples_enable_imgui_ui = true` - Enable ImGui UI for debugging
- `chip_enable_pw_rpc = false` - Enable Pigweed RPC support
- Various cluster enables for both client and server functionality

## Running

```bash
# Run the keypad app
./out/debug/chip-keypad-app

# Run with custom parameters
./out/debug/chip-keypad-app --secured-device-port 5540 --discriminator 3840
```

### Command Line Options

The keypad app supports standard CHIP application options:
- `--secured-device-port` - Port for secure device communication
- `--discriminator` - Device discriminator for commissioning
- `--passcode` - Setup passcode for commissioning

## User Interface

When built with ImGui support (`chip_examples_enable_imgui_ui = true`), the application provides:

### Light Control Window
- On/Off toggle for the light endpoint
- Brightness slider
- Color controls (hue/saturation)
- Real-time status display

### Switch Control Window  
- Send On/Off commands to bound devices
- Send level commands to bound devices
- Send color commands to bound devices
- Send identify commands to bound devices

### Basic Information Window
- Device information display
- QR code for commissioning
- Network status

## Commissioning

The keypad device can be commissioned like any other Matter device:

1. **Start the application**
2. **Commission using a Matter controller**:
   ```bash
   # Using chip-tool
   chip-tool pairing onnetwork 1 20202021
   ```
3. **Configure bindings** for the switch endpoint to control other devices

## Binding Configuration

To use the switch functionality, you need to configure bindings:

```bash
# Bind switch endpoint to another light device
chip-tool binding write binding '[{"fabricIndex": 1, "node": 2, "endpoint": 1, "cluster": 6}]' 1 2

# Where:
# - 1 = keypad device node ID
# - 2 = keypad switch endpoint
# - 2 = target light device node ID  
# - 1 = target light endpoint
# - 6 = OnOff cluster ID
```

## Development Status

### Current Implementation
- ✅ Basic dual-endpoint framework
- ✅ KeypadManager for state management
- ✅ ImGui UI for both endpoints
- ✅ Build system integration
- ✅ Placeholder cluster implementations

### TODO (Requires ZAP Configuration)
- ⏳ ZAP files for cluster configuration
- ⏳ Actual cluster attribute implementations
- ⏳ Binding cluster command sending
- ⏳ Group messaging support
- ⏳ Scene management

## ZAP File Generation

The cluster configurations will be defined using ZAP files:
- `keypad-app.zap` - Main device configuration with both endpoints
- Generated files will provide the actual cluster implementations

## File Structure

```
examples/keypad-app/
├── linux/
│   ├── .gn                     # GN configuration
│   ├── BUILD.gn               # Build configuration  
│   ├── args.gni               # Build arguments
│   ├── with_pw_rpc.gni        # PW RPC build arguments
│   ├── main.cpp               # Main application
│   ├── include/
│   │   └── CHIPProjectAppConfig.h
│   ├── build_overrides/       # Symlink to ../../build_overrides
│   └── third_party/
│       └── connectedhomeip    # Symlink to ../../../../
└── keypad-common/
    ├── BUILD.gn               # Common library build
    ├── include/
    │   └── KeypadManager.h    # Dual-endpoint manager
    └── src/
        └── KeypadManager.cpp  # Implementation
```

## Contributing

When extending this example:
1. Follow the existing dual-endpoint pattern
2. Update both light and switch functionality together
3. Maintain separation between server and client cluster logic
4. Update documentation for new features

## License

This example is licensed under the Apache License 2.0.
