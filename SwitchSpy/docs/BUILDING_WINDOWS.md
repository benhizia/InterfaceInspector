# Building SwitchSpy on Windows

This guide covers building SwitchSpy on Windows using Visual Studio 2022 and CMake.

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Required workloads:
     - Desktop development with C++
     - C++ CMake tools for Windows

2. **CMake 3.21 or later**
   - Download from: https://cmake.org/download/
   - Or install via Visual Studio Installer
   - Make sure `cmake` is in your PATH

3. **Git** (for cloning dependencies)
   - Download from: https://git-scm.com/download/win

### Optional Software

- **GLFW3** (for ImGui backend)
  - Will be fetched automatically as a submodule
  - Or install via vcpkg: `vcpkg install glfw3`

## Quick Start

### Option 1: Quick Build Script (Recommended)

The easiest way to build:

```cmd
quick-build.cmd
```

This will:
1. Configure the project (x64 Release by default)
2. Build the project
3. Optionally run SwitchSpy

**Custom configurations:**
```cmd
quick-build.cmd x64 debug      # x64 Debug build
quick-build.cmd x86 release    # x86 (32-bit) Release build
```

### Option 2: Step-by-Step Build

```cmd
# Configure x64 Debug
build.cmd x64 debug configure

# Build
build.cmd x64 debug build

# Run
build.cmd x64 debug run
```

## Build Script Usage

The `build.cmd` script provides full control over the build process.

### Syntax

```cmd
build.cmd [architecture] [configuration] [action]
```

**Arguments:**
- `architecture`: `x64` (default), `x86`, `32`, `64`
- `configuration`: `debug` (default), `release`
- `action`: `configure`, `build`, `rebuild`, `clean`, `test`, `run`

### Examples

```cmd
# Configure x64 Debug
build.cmd x64 debug configure

# Build x64 Release
build.cmd x64 release build

# Build x86 (32-bit) Debug
build.cmd x86 debug build

# Clean and rebuild x64 Release
build.cmd x64 release rebuild

# Run tests
build.cmd x64 debug test

# Run the application
build.cmd x64 release run

# Clean build directory
build.cmd x64 debug clean
```

### Workflow Example

```cmd
# 1. First-time setup: fetch dependencies
git submodule update --init --recursive

# 2. Configure
build.cmd x64 release configure

# 3. Build
build.cmd x64 release build

# 4. Run
build.cmd x64 release run
```

## CMake Presets

SwitchSpy uses CMake Presets for configuration. The available presets are:

### Configure Presets
- `windows-x64-debug` - Windows x64 Debug
- `windows-x64-release` - Windows x64 Release
- `windows-x86-debug` - Windows x86 (32-bit) Debug
- `windows-x86-release` - Windows x86 (32-bit) Release

### Build Presets
- `windows-x64-debug`
- `windows-x64-release`
- `windows-x86-debug`
- `windows-x86-release`

### Direct CMake Usage

If you prefer to use CMake directly:

```cmd
# Configure
cmake --preset windows-x64-release

# Build
cmake --build --preset windows-x64-release --config Release

# Test
ctest --preset windows-x64-release --config Release
```

## Build Output

After building, you'll find:

```
SwitchSpy/
├── build/
│   └── windows-x64-release/        # Build directory
│       ├── Release/
│       │   ├── switchspy.exe       # Main executable
│       │   └── switchspy_lib.lib   # Static library
│       └── SwitchSpy.sln          # Visual Studio solution
```

## Opening in Visual Studio

You can open the generated solution in Visual Studio:

```cmd
# After configuring
start build\windows-x64-release\SwitchSpy.sln
```

Or use Visual Studio's "Open a local folder" feature to open the SwitchSpy directory directly, and it will automatically detect CMakePresets.json.

## Dependencies

### Automatic Dependencies (Submodules)

These are automatically fetched:
- **ImGui** - GUI framework
- **nlohmann/json** - JSON parsing
- **inih** - INI file parsing

Fetch with:
```cmd
git submodule update --init --recursive
```

### System Dependencies

- **OpenGL** - Usually included with GPU drivers
- **GLFW3** - Fetched as submodule or via vcpkg

## Troubleshooting

### CMake not found
- Make sure CMake is installed and in your PATH
- Try closing and reopening the command prompt
- Reinstall CMake and check "Add to PATH" during installation

### Visual Studio 2022 not found
- The script looks for VS2022 in standard locations
- If installed elsewhere, CMake should still find it via the Visual Studio generator

### GLFW3 not found
- Run: `git submodule update --init --recursive`
- Or install via vcpkg: `vcpkg install glfw3:x64-windows`

### ImGui compilation errors
- Make sure submodules are initialized: `git submodule update --init --recursive`
- Try cleaning and rebuilding: `build.cmd x64 release clean && build.cmd x64 release configure`

### Link errors
- Ensure you're building the same configuration (Debug/Release) consistently
- Clean and rebuild: `build.cmd x64 release rebuild`

## Build Configurations

### Debug vs Release

**Debug:**
- Includes debug symbols
- No optimization
- Assertions enabled
- Larger executable
- Use for development and debugging

**Release:**
- Optimized for performance
- No debug symbols (smaller executable)
- Assertions disabled
- Use for production/distribution

### x64 vs x86

**x64 (64-bit):**
- Recommended for modern systems
- Can address more than 4GB of memory
- Better performance for large datasets

**x86 (32-bit):**
- For compatibility with 32-bit systems
- Smaller memory footprint
- Limited to ~4GB address space

## Advanced Usage

### Custom Build Directory

Edit `CMakePresets.json` to change build directories:

```json
"binaryDir": "${sourceDir}/build/${presetName}"
```

### Adding Build Options

Pass additional CMake variables in the preset:

```json
"cacheVariables": {
  "CMAKE_BUILD_TYPE": "Release",
  "MY_CUSTOM_OPTION": "ON"
}
```

### Parallel Builds

CMake uses all CPU cores by default with Visual Studio. To limit:

```cmd
cmake --build --preset windows-x64-release --config Release --parallel 4
```

## Building from Visual Studio IDE

1. Open Visual Studio 2022
2. File → Open → Folder
3. Select the `SwitchSpy` directory
4. Visual Studio will detect `CMakePresets.json`
5. Select a preset from the dropdown (e.g., "windows-x64-release")
6. Build → Build All (Ctrl+Shift+B)

## Running Tests

```cmd
# Run all tests
build.cmd x64 debug test

# Or with CMake directly
ctest --preset windows-x64-debug --config Debug --output-on-failure
```

## Creating a Standalone Distribution

After building in Release mode:

1. Copy the executable:
   ```cmd
   copy build\windows-x64-release\Release\switchspy.exe dist\
   ```

2. Copy required DLLs (if any):
   - GLFW3.dll (if dynamically linked)
   - Any other runtime dependencies

3. Copy configuration:
   ```cmd
   xcopy config dist\config\ /E /I
   ```

4. Copy documentation:
   ```cmd
   copy README.md dist\
   copy docs\*.md dist\docs\
   ```

## Common Commands Reference

```cmd
# Clean build
build.cmd x64 release clean
build.cmd x64 release configure
build.cmd x64 release build

# Quick rebuild
build.cmd x64 release rebuild

# Build all configurations
build.cmd x64 debug build
build.cmd x64 release build
build.cmd x86 debug build
build.cmd x86 release build

# Run with custom config
build\windows-x64-release\Release\switchspy.exe my-config.ini
```

## Getting Help

```cmd
# Show build script help
build.cmd --help
```

For more information, see:
- Main README: `../README.md`
- Architecture documentation: `ARCHITECTURE.md`
