# AeroSLR (simple, lightweight renderer)
AeroSLR is a basic 3D renderer written in C++ using Dear ImGUI, OpenGL 3.3, GLFW, and GLAD. It provides a GUI-based development environment for 3D rendering with windows for viewport, scene hierarchy, console, inspector, and properties.

**Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the information here.**

## Working Effectively

### Bootstrap and Build (Windows - Primary Platform)
The project is designed to work out-of-the-box on Windows with Visual Studio 2022:

1. **Prerequisites**: 
   - Visual Studio 2022 with C++ development tools (v143 toolset)
   - CMake 3.5.0+ (though 3.10+ recommended to avoid deprecation warnings)
   - All dependencies (ImGUI, GLAD, GLFW) are included in the repository

2. **Build Commands**:
   ```bash
   # Create build directory
   mkdir build
   cd build
   
   # Configure CMake (Windows)
   cmake .. -G "Visual Studio 17 2022" -A x64 -T v143
   
   # Build (NEVER CANCEL - Build takes ~10-15 seconds, set timeout to 60+ seconds)
   cmake --build . --config Debug
   # OR for Release:
   cmake --build . --config Release
   ```

3. **Expected Build Time**: 
   - Configuration: ~1-2 seconds
   - Compilation: ~8-15 seconds 
   - **NEVER CANCEL builds or long-running commands**
   - Total build time: <20 seconds

### Bootstrap and Build (Linux - Requires Dependencies)
The project can be built on Linux but requires system GLFW libraries:

1. **Prerequisites**:
   ```bash
   # Install dependencies
   sudo apt-get update
   sudo apt-get install -y libglfw3-dev libgl1-mesa-dev build-essential cmake
   ```

2. **Build Commands**:
   ```bash
   # Linux requires modified CMakeLists.txt - Windows version will fail at linking
   # The Windows CMakeLists.txt links to pre-built Windows GLFW libraries
   mkdir build
   cd build
   cmake ..
   cmake --build .  # NEVER CANCEL - takes ~8 seconds, set timeout to 60+ seconds
   ```

### Running the Application
```bash
# Windows
cd build/Debug  # or build/Release
./AeroSLR.exe

# Linux  
cd build
./AeroSLR
```

**Expected Behavior**: Application opens with ImGUI interface showing:
- Main toolbar
- Viewport window with OpenGL canvas (renders white triangles)
- Scene Hierarchy window
- Console window
- Inspector window
- Properties window

## Validation Requirements

### CRITICAL: Manual Validation After Changes
**ALWAYS manually validate any changes by running through complete scenarios**:

1. **Build Validation**:
   - Clean build must complete without errors in <20 seconds
   - No linking errors 
   - Executable is created in correct location
   - Only acceptable warning: format warning in main.cpp:437

2. **Runtime Validation** (GUI Application):
   - Application starts without crashes
   - All 5 GUI windows appear:
     - **Viewport**: OpenGL canvas with rendered content
     - **Scene Hierarchy**: Tree view of scene objects
     - **Console**: Debug/log output window
     - **Inspector**: Object properties panel  
     - **Properties**: Additional property controls
   - **OpenGL Rendering**: White triangles visible in viewport
   - Application responds to window interactions (resize, drag)
   - Application can be closed cleanly without crashes

3. **Code Quality**:
   - No new compiler warnings introduced 
   - Changes follow existing code style (no specific linter configured)
   - C++17 compliance maintained

### Quick Validation Commands
For rapid validation during development:

```bash
# Clean build test (Windows)
rmdir /s build & mkdir build & cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
Debug\AeroSLR.exe

# Clean build test (Linux - requires GLFW)
rm -rf build && mkdir build && cd build
cmake .. && cmake --build .
./AeroSLR
```

## Build System Details

### CMake Configuration
- **Minimum CMake**: 3.5.0 (3.10+ recommended)
- **C++ Standard**: C++17 (required)
- **Target**: Single executable `AeroSLR`

### Dependencies (All Included)
- **ImGUI**: UI framework (dependencies/ImGUI/)
- **GLAD**: OpenGL loader (dependencies/glad/)
- **GLFW**: Windowing (Windows: pre-built lib, Linux: system package required)
- **OpenGL**: Graphics API (system provided)

### File Structure
```
src/
├── main.cpp          # Main application entry point (~700+ lines)
└── config.h          # Basic includes

dependencies/
├── ImGUI/            # Dear ImGUI library
├── glad/             # OpenGL loader
└── ...

CMakeLists.txt        # Windows-focused build configuration
.github/workflows/    # CI pipeline (Windows-only)
```

## CI/CD Pipeline
- **GitHub Actions**: `.github/workflows/cmake-multi-platform.yml`
- **Platforms Tested**: Windows only
- **Build Configuration**: Release only in CI
- **Artifacts**: Windows executable uploaded
- **No Tests**: CTest configuration is present but no tests are defined

## Common Tasks

### Debugging Build Issues
1. **GLFW Linking Errors on Linux** (most common):
   ```
   undefined reference to `glfwSetErrorCallback'
   undefined reference to `glfwInit'
   ```
   - **Solution**: Install system GLFW: `sudo apt-get install -y libglfw3-dev`
   - **Root Cause**: Windows CMakeLists.txt links to pre-built Windows GLFW library

2. **Missing OpenGL Headers**:
   - **Error**: `fatal error: GL/gl.h: No such file or directory`
   - **Solution**: Install Mesa development packages: `sudo apt-get install -y libgl1-mesa-dev`

3. **Visual Studio Toolset Issues** (Windows):
   - Ensure VS2022 with v143 toolset is installed
   - Use CMake preset: `cmake --preset windows-x64-debug`
   - Alternative: Specify generator explicitly: `cmake .. -G "Visual Studio 17 2022" -A x64`

4. **CMake Version Warnings**:
   - **Warning**: "Compatibility with CMake < 3.10 will be removed"
   - **Safe to Ignore**: Project builds successfully despite warning
   - **Fix**: Update `cmake_minimum_required(VERSION 3.10.0)` in CMakeLists.txt

### Code Navigation
- **Main Application Logic**: `src/main.cpp` (contains entire application)
- **UI Setup**: Lines ~120-180 in main.cpp
- **OpenGL Rendering**: Lines ~200-600 in main.cpp  
- **ImGUI Windows**: Lines ~400-500 in main.cpp
- **Shader Code**: Embedded in main.cpp around lines ~180-220

### Important Code Locations
- **Window Creation**: `glfwCreateWindow` call in main()
- **OpenGL Context**: GLAD initialization after GLFW setup
- **Render Loop**: Main while loop in main() 
- **ImGUI Setup**: ImGui::CreateContext() and style configuration
- **Viewport Rendering**: OpenGL scissor test and triangle rendering

## Limitations and Notes

### Platform Support
- **Full Support**: Windows with Visual Studio 2022
- **Limited Support**: Linux (requires manual dependency installation)
- **Unknown**: MacOS (ImGUI supports it, but not tested in this project)

### Testing
- **No Automated Tests**: Repository contains no unit tests or integration tests
- **Manual Testing Required**: All validation must be done by running the application
- **CI Validation**: Only build validation, no runtime testing

### Development Workflow
1. **Before Making Changes**: 
   - Perform clean build to establish baseline
   - Run application to verify current functionality
   - Note which GUI windows are working

2. **After Making Changes**:
   - **ALWAYS** perform clean rebuild: `rm -rf build && mkdir build`
   - **NEVER CANCEL** build operations - they complete quickly
   - **ALWAYS** run the application to validate GUI functionality
   - Test all 5 windows are present and functional
   - Verify OpenGL viewport renders correctly (white triangles should be visible)
   - Test window interactions (resize, move, close)

3. **For UI Changes**: Pay special attention to ImGUI window layout and rendering
4. **For OpenGL Changes**: Verify viewport shows expected rendering output
5. **For Build System Changes**: Test on both Debug and Release configurations

### Performance Expectations
- **Cold Build**: 8-15 seconds (includes all ImGUI compilation)  
- **Incremental Build**: 1-5 seconds (only changed files)
- **CMake Configuration**: <1 second
- **Application Launch**: 1-2 seconds with GUI appearing immediately

## Timing Expectations
- **CMake Configuration**: <1 second
- **Build Compilation**: 8-15 seconds 
- **Application Startup**: 1-2 seconds
- **NEVER CANCEL** any command - all operations complete quickly

**CRITICAL**: Set timeout values of 60+ seconds for all build commands to avoid premature cancellation, even though builds complete much faster.