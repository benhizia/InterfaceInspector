# Running SwitchSpy Tests

This directory contains 6 comprehensive end-to-end black box tests for SwitchSpy. Each test has dedicated build and run scripts for easy "double-click" execution.

## Quick Start

### Windows

**Option 1: Interactive Menu (Recommended)**
```cmd
run_test_menu.cmd
```
This launches an interactive menu where you can select which test to run.

**Option 2: Build All Tests**
```cmd
build_all.cmd
```

**Option 3: Run Individual Test**
```cmd
cd 01_tcp_basic
build.cmd
run_test.cmd
```

### Linux/macOS

**Option 1: Interactive Menu (Recommended)**
```bash
./run_test_menu.sh
```

**Option 2: Build All Tests**
```bash
./build_all.sh
```

**Option 3: Run Individual Test**
```bash
cd 01_tcp_basic
./build.sh
./run_test.sh
```

## Test Overview

| Test | Focus | Data Rate | Key Feature |
|------|-------|-----------|-------------|
| **01_tcp_basic** | Basic TCP relay | 2 msg/s | Simple validation |
| **02_udp_relay** | UDP datagram forwarding | 1 msg/s | Connectionless, CRC |
| **03_multicast** | Passive multicast spy | 1 msg/s | Non-intrusive monitoring |
| **04_high_throughput** | Stress test | 10,000 msg/s | Performance validation |
| **05_rate_mismatch** | Buffer overflow | 100→50 msg/s | Flow control |
| **06_complex_structs** | Nested structures | 2 msg/s | Tree view validation |

## Scripts in Each Test Folder

### Build Scripts
- **`build.cmd`** (Windows) - Compiles sender.cpp and receiver.cpp
- **`build.sh`** (Linux/macOS) - Compiles sender.cpp and receiver.cpp

### Run Scripts
- **`run_test.cmd`** (Windows) - Launches test with instructions
- **`run_test.sh`** (Linux/macOS) - Launches test with instructions

### Other Files
- **`shared.hpp`** - Data structure definitions
- **`sender.cpp`** - Data producer
- **`receiver.cpp`** - Data consumer
- **`generate_metadata.cmd`** - Metadata generation script
- **`switchspy.ini`** - SwitchSpy configuration
- **`README.md`** - Detailed test documentation

## How Tests Work

Each run script follows this pattern:

1. **Check Prerequisites**
   - Verify sender/receiver are built
   - Check for metadata file (generate if missing)

2. **Start Receiver**
   - Opens in separate window/terminal
   - Listens for data on designated port

3. **Prompt for SwitchSpy**
   - Displays command to launch SwitchSpy GUI
   - Waits for user confirmation

4. **Start Sender**
   - Opens in separate window/terminal
   - Begins sending data to SwitchSpy relay

5. **Monitor**
   - User validates GUI behavior per test README

## Requirements

### Windows
- **Compiler**: MinGW-w64 or MSYS2 with g++
- **SwitchSpy**: Built GUI executable
- **Python**: For metadata generation (header_inspector.py)

### Linux/macOS
- **Compiler**: g++ (GNU C++ compiler)
- **SwitchSpy**: Built GUI executable
- **Python**: For metadata generation

## Test Execution Flow

```
┌─────────────┐
│   Sender    │ ──────┐
│ (sends data)│       │
└─────────────┘       │
                      ▼
                ┌───────────┐         ┌──────────────┐
                │ SwitchSpy │────────▶│     GUI      │
                │   Relay   │         │ Visualization│
                └───────────┘         └──────────────┘
                      │
                      ▼
                ┌───────────┐
                │ Receiver  │
                │(validates)│
                └───────────┘
```

## Building Individual Tests

### Windows
```cmd
cd <test_folder>
build.cmd
```

Example:
```cmd
cd 01_tcp_basic
build.cmd
```

### Linux/macOS
```bash
cd <test_folder>
./build.sh
```

Example:
```bash
cd 01_tcp_basic
./build.sh
```

## Running Individual Tests

### Windows
```cmd
cd <test_folder>
run_test.cmd
```

The script will:
- Open receiver in new window
- Show SwitchSpy launch instructions
- Open sender in new window

### Linux/macOS
```bash
cd <test_folder>
./run_test.sh
```

The script will:
- Start receiver in background
- Show SwitchSpy launch instructions
- Start sender (Ctrl+C to stop)

## Metadata Generation

Each test requires metadata extracted from `shared.hpp`:

### Automatic (via run script)
Run scripts automatically call `generate_metadata.cmd` if `shared.meta.json` is missing.

### Manual
```cmd
cd <test_folder>
generate_metadata.cmd
```

Or manually:
```bash
cd <InterfaceInspector_root>
python header_inspector.py SwitchSpy/tests/<test_folder>/shared.hpp
cp MetaDataGen/shared.meta.json SwitchSpy/tests/<test_folder>/
```

## Test Validation

Each test has a comprehensive README.md with:
- **Setup instructions**
- **Execution steps**
- **What to observe in each GUI panel**
- **Expected values and thresholds**
- **Pass/fail criteria**
- **Troubleshooting guide**

**Read the README.md in each test folder for detailed validation steps!**

## Example: Running Test 01 (TCP Basic)

### Windows
1. Open `SwitchSpy/tests/`
2. Double-click `run_test_menu.cmd`
3. Press `1` and Enter
4. Follow on-screen instructions

Or directly:
1. Open `SwitchSpy/tests/01_tcp_basic/`
2. Double-click `build.cmd` (first time only)
3. Double-click `run_test.cmd`
4. Follow on-screen instructions

### Linux/macOS
```bash
cd SwitchSpy/tests
./run_test_menu.sh
# Press 1 and Enter
# Follow on-screen instructions
```

Or directly:
```bash
cd SwitchSpy/tests/01_tcp_basic
./build.sh          # First time only
./run_test.sh
# Follow on-screen instructions
```

## Troubleshooting

### "g++ not found"
**Windows**: Install MinGW-w64 or MSYS2
```
https://www.msys2.org/
```

**Linux**:
```bash
sudo apt install build-essential  # Debian/Ubuntu
sudo dnf install gcc-c++           # Fedora
```

**macOS**:
```bash
brew install gcc
```

### "Build failed"
Check compiler output for errors. Common issues:
- Missing headers (check #include paths)
- Linker errors (verify ws2_32 on Windows)
- C++17 support (use modern compiler)

### "Receiver/Sender not connecting"
1. Check firewall settings
2. Verify ports are not in use: `netstat -an | grep <port>`
3. Try running receiver separately to test

### "SwitchSpy not showing data"
1. Verify metadata file exists: `ls shared.meta.json`
2. Check SwitchSpy logs panel for errors
3. Verify ports in switchspy.ini match sender/receiver

### "Permission denied" (Linux/macOS)
Make scripts executable:
```bash
chmod +x *.sh
```

## Clean Build

To rebuild from scratch:

### Windows
```cmd
cd <test_folder>
del *.exe *.o
build.cmd
```

### Linux/macOS
```bash
cd <test_folder>
rm -f sender receiver *.o
./build.sh
```

## Performance Notes

### Test 04 (High Throughput)
- **CPU**: Expect 30-50% usage
- **Memory**: ~50-100MB
- **Drops**: 0-2% acceptable at 10K msg/s
- **Deferred mode REQUIRED**

### Test 05 (Rate Mismatch)
- **Expected**: Buffer warnings, packet drops
- **This is correct behavior** - validates flow control

## Additional Resources

- **Architecture**: `SwitchSpy/docs/ARCHITECTURE.md`
- **Building SwitchSpy**: `SwitchSpy/README.md`
- **Windows Build**: `SwitchSpy/docs/BUILDING_WINDOWS.md`
- **Test READMEs**: Each test folder has detailed README.md

## Getting Help

If tests fail or behave unexpectedly:
1. Read the test's README.md (500+ lines of detailed instructions)
2. Check SwitchSpy logs panel for errors
3. Verify prerequisites (compiler, Python, SwitchSpy built)
4. Try a simpler test first (Test 01)

## Summary

The test suite provides comprehensive validation through:
- ✅ Automated build scripts
- ✅ Easy-to-use run scripts
- ✅ Interactive test launcher
- ✅ Detailed documentation
- ✅ 6 different test scenarios
- ✅ Cross-platform support (Windows, Linux, macOS)

**Start with the interactive menu (`run_test_menu.cmd` or `./run_test_menu.sh`) for the easiest experience!**
