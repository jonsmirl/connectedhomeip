#!/bin/bash

# Build script for all ImGui-enabled Matter example applications
# This script builds each app with ImGui UI enabled and copies the binaries to the target directory

# Note: This script should be run from the connectedhomeip/examples directory
# Make sure you have activated the Matter build environment first:
#   source scripts/activate.sh

set -e  # Exit on any error

# Configuration
TARGET_DIR="$HOME/lowpan/matter-devices/bin"
BUILD_ARGS="chip_examples_enable_imgui_ui=true"
EXAMPLES_DIR="$(pwd)"
CHIP_ROOT="$(dirname "$(pwd)")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to kill running app instances
kill_running_apps() {
    local binary_name="$1"

    print_status "  Checking for running instances of $binary_name..."

    # Find all processes with the binary name
    local pids=$(pgrep -f "$binary_name" 2>/dev/null || true)

    if [ -n "$pids" ]; then
        print_warning "  Found running instances of $binary_name (PIDs: $pids)"
        print_status "  Terminating running instances..."

        # First try graceful termination (SIGTERM)
        for pid in $pids; do
            if kill -TERM "$pid" 2>/dev/null; then
                print_status "    Sent SIGTERM to PID $pid"
            fi
        done

        # Wait a moment for graceful shutdown
        sleep 2

        # Check if any processes are still running and force kill them
        local remaining_pids=$(pgrep -f "$binary_name" 2>/dev/null || true)
        if [ -n "$remaining_pids" ]; then
            print_warning "  Some instances still running, force killing..."
            for pid in $remaining_pids; do
                if kill -KILL "$pid" 2>/dev/null; then
                    print_status "    Sent SIGKILL to PID $pid"
                fi
            done
            sleep 1
        fi

        # Final check
        local final_pids=$(pgrep -f "$binary_name" 2>/dev/null || true)
        if [ -n "$final_pids" ]; then
            print_error "  Failed to terminate some instances of $binary_name (PIDs: $final_pids)"
            return 1
        else
            print_success "  All instances of $binary_name terminated successfully"
        fi
    else
        print_status "  No running instances of $binary_name found"
    fi

    return 0
}

# Function to kill all running Matter device apps
kill_all_matter_apps() {
    print_status "Checking for and terminating any running Matter device applications..."

    # List of all Matter app binary names
    local matter_apps=(
        "contact-sensor-app"
        "chip-dishwasher-app"
        "humidity-measurement-app"
        "chip-lighting-app"
        "chip-light-switch-app"
        "chip-microwave-oven-app"
        "occupancy-sensor-app"
        "pressure-measurement-app"
        "chip-rocker-app"
        "chip-keypad-app"
        "temperature-measurement-app"
        "thermostat-app"
        "water-leak-detector-app"
    )

    local killed_any=false

    for app in "${matter_apps[@]}"; do
        local pids=$(pgrep -f "$app" 2>/dev/null || true)
        if [ -n "$pids" ]; then
            killed_any=true
            print_warning "Found running $app (PIDs: $pids), terminating..."

            # Try graceful termination first
            for pid in $pids; do
                kill -TERM "$pid" 2>/dev/null || true
            done
        fi
    done

    if [ "$killed_any" = true ]; then
        print_status "Waiting for graceful shutdown..."
        sleep 3

        # Force kill any remaining processes
        for app in "${matter_apps[@]}"; do
            local remaining_pids=$(pgrep -f "$app" 2>/dev/null || true)
            if [ -n "$remaining_pids" ]; then
                print_warning "Force killing remaining $app instances (PIDs: $remaining_pids)..."
                for pid in $remaining_pids; do
                    kill -KILL "$pid" 2>/dev/null || true
                done
            fi
        done

        sleep 1
        print_success "All Matter device applications terminated"
    else
        print_status "No running Matter device applications found"
    fi
}

# Function to build an application
build_app() {
    local app_name="$1"
    local binary_name="$2"
    local app_dir="$3"
    
    print_status "Building $app_name..."
    
    if [ ! -d "$app_dir" ]; then
        print_error "Directory $app_dir does not exist, skipping $app_name"
        return 1
    fi
    
    cd "$app_dir/linux"
    
    # Generate build files
    print_status "  Generating build files for $app_name..."
    if ! gn gen out/debug --args="$BUILD_ARGS"; then
        print_error "  Failed to generate build files for $app_name"
        cd "$EXAMPLES_DIR"
        return 1
    fi
    
    # Build the application
    print_status "  Compiling $app_name..."
    if ! ninja -C out/debug; then
        print_error "  Failed to build $app_name"
        cd "$EXAMPLES_DIR"
        return 1
    fi
    
    # Check if binary exists
    if [ ! -f "out/debug/$binary_name" ]; then
        print_error "  Binary out/debug/$binary_name not found for $app_name"
        cd "$EXAMPLES_DIR"
        return 1
    fi

    # Kill any running instances before copying
    if ! kill_running_apps "$binary_name"; then
        print_warning "  Warning: Could not terminate all running instances of $binary_name"
        print_warning "  Proceeding with copy anyway..."
    fi

    # Copy to target directory
    print_status "  Copying $binary_name to $TARGET_DIR..."
    if ! cp "out/debug/$binary_name" "$TARGET_DIR/"; then
        print_error "  Failed to copy $binary_name to $TARGET_DIR"
        cd "$EXAMPLES_DIR"
        return 1
    fi

    # Strip the binary at the destination to reduce size
    print_status "  Stripping debug symbols from $binary_name..."
    if ! strip "$TARGET_DIR/$binary_name"; then
        print_warning "  Failed to strip $binary_name (continuing anyway)"
    else
        print_success "  Successfully stripped $binary_name"
    fi

    print_success "  Successfully built and copied $app_name"
    cd "$EXAMPLES_DIR"
    return 0
}

# Activate the Matter build environment
print_status "Activating Matter build environment..."
if [ -f "$CHIP_ROOT/scripts/activate.sh" ]; then
    source "$CHIP_ROOT/scripts/activate.sh"
    print_status "Matter build environment activated"
else
    print_error "Could not find activate.sh at $CHIP_ROOT/scripts/activate.sh"
    print_error "Please make sure you're running this script from the connectedhomeip/examples directory"
    exit 1
fi

# Kill any running Matter apps before starting
kill_all_matter_apps

# Main script
print_status "Starting build process for all ImGui-enabled Matter applications"
print_status "Target directory: $TARGET_DIR"
print_status "Build arguments: $BUILD_ARGS"
print_status "Examples directory: $EXAMPLES_DIR"

# Ensure we're in the right directory
if [ ! -d "rocker-app" ] || [ ! -d "lighting-app" ]; then
    print_error "This script must be run from the connectedhomeip/examples directory"
    exit 1
fi

# Create target directory if it doesn't exist
if [ ! -d "$TARGET_DIR" ]; then
    print_status "Creating target directory: $TARGET_DIR"
    mkdir -p "$TARGET_DIR"
fi

# Track build results
declare -a successful_builds=()
declare -a failed_builds=()

# Build all applications
# Format: build_app "Display Name" "binary-name" "source-directory"

build_app "Contact Sensor App" "contact-sensor-app" "contact-sensor-app"
if [ $? -eq 0 ]; then successful_builds+=("contact-sensor-app"); else failed_builds+=("contact-sensor-app"); fi

build_app "Dishwasher App" "chip-dishwasher-app" "dishwasher-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-dishwasher-app"); else failed_builds+=("chip-dishwasher-app"); fi

build_app "Humidity Measurement App" "humidity-measurement-app" "humidity-measurement-app"
if [ $? -eq 0 ]; then successful_builds+=("humidity-measurement-app"); else failed_builds+=("humidity-measurement-app"); fi

build_app "Lighting App" "chip-lighting-app" "lighting-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-lighting-app"); else failed_builds+=("chip-lighting-app"); fi

build_app "Light Switch App" "chip-light-switch-app" "light-switch-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-light-switch-app"); else failed_builds+=("chip-light-switch-app"); fi

build_app "Microwave Oven App" "chip-microwave-oven-app" "microwave-oven-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-microwave-oven-app"); else failed_builds+=("chip-microwave-oven-app"); fi

build_app "Occupancy Sensor App" "occupancy-sensor-app" "occupancy-sensor-app"
if [ $? -eq 0 ]; then successful_builds+=("occupancy-sensor-app"); else failed_builds+=("occupancy-sensor-app"); fi

build_app "Pressure Measurement App" "pressure-measurement-app" "pressure-measurement-app"
if [ $? -eq 0 ]; then successful_builds+=("pressure-measurement-app"); else failed_builds+=("pressure-measurement-app"); fi

build_app "Rocker App" "chip-rocker-app" "rocker-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-rocker-app"); else failed_builds+=("chip-rocker-app"); fi

build_app "Keypad App" "chip-keypad-app" "keypad-app"
if [ $? -eq 0 ]; then successful_builds+=("chip-keypad-app"); else failed_builds+=("chip-keypad-app"); fi

build_app "Temperature Measurement App" "temperature-measurement-app" "temperature-measurement-app"
if [ $? -eq 0 ]; then successful_builds+=("temperature-measurement-app"); else failed_builds+=("temperature-measurement-app"); fi

build_app "Thermostat App" "thermostat-app" "thermostat"
if [ $? -eq 0 ]; then successful_builds+=("thermostat-app"); else failed_builds+=("thermostat-app"); fi

build_app "Water Leak Detector App" "water-leak-detector-app" "water-leak-detector-app"
if [ $? -eq 0 ]; then successful_builds+=("water-leak-detector-app"); else failed_builds+=("water-leak-detector-app"); fi

# Print summary
echo
print_status "Build Summary:"
echo "=============="

if [ ${#successful_builds[@]} -gt 0 ]; then
    print_success "Successfully built ${#successful_builds[@]} applications:"
    for app in "${successful_builds[@]}"; do
        echo "  ✓ $app"
    done
fi

if [ ${#failed_builds[@]} -gt 0 ]; then
    print_warning "Failed to build ${#failed_builds[@]} applications:"
    for app in "${failed_builds[@]}"; do
        echo "  ✗ $app"
    done
fi

echo
print_status "All binaries copied to: $TARGET_DIR"
print_status "Build process complete!"

# Exit with error code if any builds failed
if [ ${#failed_builds[@]} -gt 0 ]; then
    exit 1
else
    exit 0
fi
