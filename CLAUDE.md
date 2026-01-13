# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is the **Matter SDK** (connectedhomeip), an open-source implementation of the Matter smart home protocol. Matter is a unified connectivity standard for IoT devices built on IP, compatible with Thread and Wi-Fi networks.

## Build System

Matter uses **GN (Generate Ninja)** as its primary build system, with CMake for some platform-specific builds (ESP32, nRF Connect, Zephyr).

### Environment Setup

```bash
# First time or when builds fail - full bootstrap
source scripts/bootstrap.sh

# Quick activation (after bootstrap)
source scripts/activate.sh
```

### Build Commands

```bash
# Build for host (Linux/macOS)
source scripts/activate.sh
gn gen out/host
ninja -C out/host

# Release build
gn gen out/host --args='is_debug=false'
ninja -C out/host

# Unified build (all platforms)
gn gen out/unified --args='is_debug=true target_os="all"'
ninja -C out/unified all
```

### Using build_examples.py (Recommended)

```bash
# List available targets
./scripts/build/build_examples.py targets

# Build Linux examples
./scripts/build/build_examples.py --target linux-x64-tests build
./scripts/build/build_examples.py --target linux-x64-chip-tool build
./scripts/build/build_examples.py --target linux-x64-light-no-ble build

# Build ESP32 example
./scripts/build/build_examples.py --target esp32-m5stack-all-clusters build

# Build nRF example
./scripts/build/build_examples.py --target nrf-nrf5340dk-pump build
```

### Building Examples Standalone

```bash
cd examples/chip-tool
gn gen out/debug
ninja -C out/debug
```

## Testing

The Matter SDK has multiple testing frameworks for different purposes:

| Test Type | Framework | Location | Use Case |
|-----------|-----------|----------|----------|
| Unit Tests | pw_unit_test (GoogleTest) | `src/*/tests/` | Fast, isolated component testing |
| YAML Tests | Custom parser → chip-tool | `src/app/tests/suites/` | Human-readable integration tests |
| Python Tests | Mobly + ChipDeviceCtrl | `src/python_testing/` | Complex certification tests |
| Cirque Tests | Docker containers | `src/test_driver/linux-cirque/` | Multi-device network topology tests |
| Fuzz Tests | libFuzzer / pw_fuzzer | Various | Security and robustness testing |

### Unit Tests

Unit tests use Pigweed's pw_unit_test framework (GoogleTest compatible). Tests are in `src/*/tests/` directories.

```bash
# Build and run all unit tests
./gn_build.sh

# Run all tests via ninja
ninja -C out/host check

# Run specific test directory
ninja -C out/host src/inet/tests:tests_run

# Run a single unit test (platform path from out/debug subdirectory)
ninja -C out/debug linux_x64_clang/tests/TestSessionManagerDispatch
# OR with full path:
ninja -C out/debug linux_x64_clang/phony/src/transport/tests/TestSessionManagerDispatch.run

# Test binaries location
# out/debug/<host_compiler>/tests/  (e.g., out/debug/linux_x64_clang/tests/)
```

#### Writing Unit Tests

Tests are defined in BUILD.gn using `chip_test_suite`:

```gn
chip_test_suite("tests") {
    output_name = "libSomethingTests"
    test_sources = [
        "TestFoo.cpp",
        "TestBar.cpp",
    ]
    public_deps = [
        "${chip_root}/src/lib/core",
        "${chip_root}/src/lib/support:testing",
    ]
}
```

Simple test pattern:
```cpp
#include <pw_unit_test/framework.h>

TEST(YourTestFunction) {
    SomeType foo;
    EXPECT_EQ(foo.GetValue(), 42);      // Non-fatal assertion
    ASSERT_NE(foo.GetPointer(), nullptr); // Fatal - aborts test on failure
}
```

Test with fixtures:
```cpp
class YourTestContext : public ::testing::Test {
public:
    static void SetUpTestSuite() { /* once per suite */ }
    static void TearDownTestSuite() { /* once per suite */ }
protected:
    void SetUp() override { /* before each test */ }
    void TearDown() override { /* after each test */ }
};

TEST_F(YourTestContext, YourTestFunction) {
    // Access fixture members via this->
}
```

#### Test Utilities

| Utility | Location | Purpose |
|---------|----------|---------|
| `Test::AppContext` | `src/app/tests/AppTestContext.h` | Loopback messaging with secure sessions |
| `MockClock` / `RAIIMockClock` | `src/system/SystemClock.h`, `RAIIMockClock.h` | Time-dependent testing |
| `TestPersistentStorageDelegate` | `src/lib/support/TestPersistentStorageDelegate.h` | In-memory storage |
| `*TestAccess` classes | Various | Friend classes for private member access |

### YAML Integration Tests

YAML tests are human-readable test definitions in `src/app/tests/suites/`. They're parsed and executed via chip-tool.

```yaml
name: Test Cluster Example
config:
    nodeId: 0x12344321
    cluster: "On/Off"
    endpoint: 1

tests:
    - label: "Wait for commissioning"
      cluster: "DelayCommands"
      command: "WaitForCommissionee"
      arguments:
          values:
              - name: "nodeId"
                value: nodeId

    - label: "Turn on the light"
      command: "On"

    - label: "Read OnOff attribute"
      command: "readAttribute"
      attribute: "OnOff"
      response:
          value: true
          constraints:
              type: boolean
```

Running YAML tests:
```bash
# Build prerequisites
./scripts/build/build_examples.py --target linux-x64-chip-tool --target linux-x64-all-clusters build

# Run a YAML test
scripts/tests/run_test_suite.py --runner chip_tool_python \
    --chip-tool out/linux-x64-chip-tool/chip-tool \
    --target TestOnOff run

# Run with BLE-WiFi commissioning mocking
scripts/tests/run_test_suite.py --runner chip_tool_python \
    --chip-tool out/linux-x64-chip-tool/chip-tool \
    --target TestOperationalState run --ble-wifi

# Using chiptool.py directly
./scripts/tests/chipyaml/chiptool.py tests Test_TC_OO_2_1 \
    --server_path ./out/linux-x64-chip-tool/chip-tool
```

### Python Framework Tests

Python tests use the Mobly framework with `MatterBaseTest` base class. Located in `src/python_testing/` (~200+ TC_*.py files).

```python
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#     factory-reset: true
# === END CI TEST ARGUMENTS ===

class TC_MYTEST_1_1(MatterBaseTest):
    @async_test_body
    async def test_TC_MYTEST_1_1(self):
        # Read an attribute
        vendor_name = await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorName,
        )
        asserts.assert_equal(vendor_name, "Test vendor")

        # Send a command
        await self.send_single_cmd(
            cmd=Clusters.OnOff.Commands.On(),
            endpoint=1,
        )

if __name__ == "__main__":
    default_matter_test_main()
```

Running Python tests:
```bash
# Setup Python environment
source scripts/activate.sh
./scripts/build_python.sh -i out/python_env
source out/python_env/bin/activate

# Run a test directly
python3 src/python_testing/TC_ACE_1_2.py \
    --commissioning-method on-network \
    --discriminator 1234 \
    --passcode 20202021

# Using run_python_test.py (manages app lifecycle)
./scripts/tests/run_python_test.py --factory-reset \
    --app ./out/linux-x64-all-clusters/chip-all-clusters-app \
    --script src/python_testing/TC_ACE_1_2.py \
    --script-args "--commissioning-method on-network --discriminator 1234"

# Build and run all Python tests locally
./scripts/tests/local.py build
./scripts/tests/local.py python-tests
```

Key `MatterBaseTest` methods:
- `read_single_attribute_check_success()` / `read_single_attribute_expect_error()`
- `send_single_cmd()`
- `step(num)` - Mark test plan steps for certification
- `check_pics(pics_str)` / `pics_guard(condition)` - PICS-gated execution

### Cirque Tests (Multi-Device Network)

Cirque simulates complex network topologies using Docker containers. Each container has independent network stack supporting Thread, BLE, and WiFi.

```bash
# Bootstrap Cirque environment
scripts/tests/cirque_tests.sh bootstrap

# Run all Cirque tests
scripts/tests/cirque_tests.sh run_all_tests

# Run specific test
scripts/tests/cirque_tests.sh run_test CommissioningTest

# Setup topology for manual testing
scripts/tests/cirque_tests.sh run_test ManualTest -t topologies/three_node_with_thread.json
```

Available tests: EchoTest, CommissioningTest, FailsafeTest, MobileDeviceTest, IcdDeviceTest, SubscriptionResumptionTest, etc.

### Fuzz Testing

```bash
# libFuzzer targets
./scripts/build/build_examples.py --target linux-x64-tests-clang-asan-libfuzzer build

# pw_fuzzer FuzzTests (asan enabled by default)
./scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest build

# Run fuzz test in unit test mode (brief)
./out/linux-x64-tests-clang-pw-fuzztest/chip_pw_fuzztest/tests/fuzz-chip-cert-pw

# Continuous fuzzing
./fuzz-chip-cert-pw --fuzz=ChipCert.DecodeChipCertFuzzer

# Run for specific duration
./fuzz-chip-cert-pw --fuzz_for=10m
```

### Code Coverage

```bash
./scripts/build_coverage.sh                    # Core + unit tests
./scripts/build_coverage.sh --yaml             # Include YAML tests
./scripts/build_coverage.sh --python           # Include Python tests
./scripts/build_coverage.sh --code=all --yaml  # Full coverage (core + clusters)
./scripts/build_coverage.sh --target TestFoo.run  # Specific test

# Report location: out/coverage/coverage/html/index.html
```

### PICS (Protocol Implementation Conformance Statement)

PICS gates test execution based on device capabilities. CI PICS values: `src/app/tests/suites/certification/ci-pics-values`

YAML:
```yaml
- label: "Step gated on PICS"
  PICS: CLUSTER.S.A0001
  command: "readAttribute"
  attribute: "SomeAttribute"
```

Python:
```python
if self.pics_guard(self.check_pics('PICS.S.FEATURE')):
    # Conditional test code
```

### Test Utilities for Hard-to-Trigger Conditions

**TestEventTriggers** - Simulate conditions like smoke alarm:
```cpp
class MyHandler : public TestEventTriggerHandler {
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override { /* ... */ }
};
```

**NamedPipes** - CI simulation of manual actions

**Fault Injection**:
```cpp
CHIP_FAULT_INJECT(FaultInjection::kFault_CASEServerBusy, busy = true);
```

## ZAP Code Generation

ZAP generates cluster code from `.zap` files.

```bash
# Generate code from a .zap file
scripts/tools/zap/generate.py <path-to-zap-file>

# Regenerate all ZAP files
./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'

# Run ZAP GUI
scripts/tools/zap/run_zaptool.sh <path-to-zap-file>
```

## chip-tool (Matter Controller CLI)

```bash
# Build chip-tool
./scripts/build/build_examples.py --target linux-x64-chip-tool build

# Commission a device via BLE
./out/linux-x64-chip-tool/chip-tool pairing ble-wifi <node_id> <ssid> <password> <pin_code> <discriminator>

# Commission using setup code
./out/linux-x64-chip-tool/chip-tool pairing code <node_id> MT:-24J0AFN00KA0648G00

# Control a light
./out/linux-x64-chip-tool/chip-tool onoff on <node_id> <endpoint_id>
./out/linux-x64-chip-tool/chip-tool onoff off <node_id> <endpoint_id>
./out/linux-x64-chip-tool/chip-tool onoff read on-off <node_id> <endpoint_id>

# Interactive mode (maintains CASE sessions)
./out/linux-x64-chip-tool/chip-tool interactive start
```

## Source Architecture

| Directory | Purpose |
|-----------|---------|
| `src/app/` | Application layer, interaction model, clusters |
| `src/app/clusters/` | Cluster implementations (~118 clusters) |
| `src/controller/` | Matter controller implementations |
| `src/credentials/` | Credential management, attestation, PKI |
| `src/crypto/` | Cryptographic operations |
| `src/inet/` | IP networking layer |
| `src/lib/` | Core libraries (TLV, support, DNS-SD) |
| `src/messaging/` | Message transport |
| `src/platform/` | Platform abstraction layers (~30 platforms) |
| `src/protocols/` | Matter protocols (CASE, PASE, etc.) |
| `src/transport/` | Transport layer |
| `examples/` | Example device applications (~60 examples) |
| `zzz_generated/` | ZAP-generated code |
| `data_model/` | Matter spec XML files (1.2, 1.3, 1.4, 1.5) |

## Writing Clusters

Cluster implementations go in `src/app/clusters/<cluster-name-server>/`. Key files:

- `ClusterNameCluster.h/cpp` - `ServerClusterInterface` implementation
- `ClusterNameLogic.h/cpp` - Core business logic (for modular pattern)
- `CodegenIntegration.cpp` - Application integration bridge
- `tests/` - Unit tests

Recommended patterns:
- **Combined**: Logic and data in one class (simpler, smaller footprint)
- **Modular**: Separate `ClusterLogic` from `ClusterImplementation` (better testability)

Update these files when adding clusters:
- `src/app/zap_cluster_list.json` - Map cluster to directory
- `src/app/common/templates/config-data.yaml` - Enable callbacks
- `src/app/zap-templates/zcl/zcl.json` - Mark externally-handled attributes

## Matter PKI (Certificate Hierarchy)

Matter uses a three-level certificate hierarchy for device authentication:

| Certificate | Purpose | Scope |
|-------------|---------|-------|
| **RCAC** (Root CA Certificate) | Defines the fabric | Shared by all devices in a home/fabric |
| **ICAC** (Intermediate CA Certificate) | Identifies commissioning client | Unique per phone/app instance |
| **NOC** (Node Operational Certificate) | Identifies the device | Unique per commissioned device |

### Why Each Commissioning Client Needs Its Own ICAC

**Interoperability**: Devices can communicate if they share the same RCAC, regardless of which ICAC signed their NOC. The RCAC defines fabric membership.

**Revocation granularity**: Each commissioning client (phone/app) gets its own ICAC. This enables:
- Targeted revocation when a phone is lost/stolen or a user is removed
- Network continuity - devices commissioned by other clients remain operational
- Their NOCs chain to different ICACs but the same trusted RCAC

**Anti-pattern**: Do NOT share ICACs across commissioning clients. If ICACs were shared, revoking a compromised client would require re-commissioning every device on the fabric.

### Certificate Chain

```
RCAC (Root)
 └── ICAC-1 (Client A's phone)
 │    └── NOC (Device 1)
 │    └── NOC (Device 2)
 └── ICAC-2 (Client B's phone)
      └── NOC (Device 3)
      └── NOC (Device 4)
```

All four devices can communicate because they trust the same RCAC. If Client A's phone is compromised, revoke ICAC-1 - Devices 3 and 4 continue working normally.

## Key Development Guides

- `docs/guides/writing_clusters.md` - Cluster implementation
- `docs/guides/migrating_ember_cluster_to_code_driven.md` - Ember migration
- `docs/testing/unit_testing.md` - Unit testing patterns
- `docs/testing/integration_tests.md` - Integration testing
- `docs/development_controllers/chip-tool/chip_tool_guide.md` - chip-tool usage

## Code Review Guidelines

From `.github/copilot-instructions.md`:
- Do not comment on XML or `.matter` cluster content
- The SDK implements an in-progress Matter specification - do not make uninformed assumptions about the spec
- Match the prevailing style of existing code
- The SDK uses automated code formatting - don't comment on whitespace/formatting
- Look for common typos and suggest fixes
