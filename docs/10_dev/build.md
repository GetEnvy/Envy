# Building Envy

**Last Updated:** January 2026
**Primary:** Visual Studio (`Visual Studio\Envy.sln`, toolset `v145`, C++17)
**Secondary:** CMake (HashLib only)

## Quick reference

| Component | Status | Build |
|-----------|--------|-------|
| **Main app** | ✅ | Visual Studio |
| **HashLib** | ✅ | VS + CMake |
| **Services** | ✅ | Visual Studio |
| **Plugins** | ✅ | Visual Studio |
| **Tests** | 🟡 | Standalone runner in `tests/` (see [status](status.md)) |

## 🏗️ Primary Build: Visual Studio 2026

### Prerequisites

1. **Visual Studio 2026** (Community/Professional/Enterprise)
   - Version 18.0+ required
   - MSVC toolset `v145` (as configured in the `.vcxproj` files)
   - Windows 10/11 SDK (projects target `WindowsTargetPlatformVersion` = `10.0`)

2. **Windows Requirements**
   - Windows 10/11 (64-bit recommended)
   - Administrator privileges (for some operations)

3. **No External Dependencies**
   - All libraries bundled in `Services/`
   - Precompiled binaries included

### Build Steps

#### Step 1: Clone Repository
```bash
git clone https://github.com/Mika3578/Envy.git
cd Envy
```

#### Step 2: Open Solution
- Navigate to `Visual Studio/` directory
- Double-click `Envy.sln`
- Visual Studio 2026 will open the solution

#### Step 3: Select Configuration
- **Platform:** x64 (recommended) or Win32
- **Configuration:** Release (recommended) or Debug
- **Toolset:** v145 (automatically selected)

#### Step 4: Build Solution
- **Menu:** Build → Build Solution
- **Keyboard:** Ctrl+Shift+B
- **Status:** Monitor Output window for progress

#### Step 5: Verify Build
- Check the project output folder for `Envy.exe`
  - `Envy\Release x64\Envy.exe` (Release x64)
  - `Envy\Debug x64\Envy.exe` (Debug x64)
  - `Envy\Release Win32\Envy.exe` (Release Win32)
  - `Envy\Debug Win32\Envy.exe` (Debug Win32)

### Build Configurations

| Configuration | Purpose | Optimization | Symbols |
|---------------|---------|--------------|---------|
| **Release** | Production | /O2 (Max Speed) | No debug info |
| **Debug** | Development | /Od (Disabled) | Full debug info |

### Platform Differences

| Platform | Address Space | Recommended | Notes |
|----------|---------------|-------------|-------|
| **x64** | 64-bit | ✅ Primary | Better performance, larger files |
| **Win32** | 32-bit | ⚠️ Legacy | Limited to 2GB address space |

## 🔧 Secondary Build: CMake (Limited)

### Current Status
- **Supported:** HashLib library only
- **Missing:** Main application, services, plugins, MFC integration
- **Use Case:** Library development, cross-platform HashLib usage

### Prerequisites
- **CMake:** 3.20+
- **Visual Studio 2026:** For MSVC compiler

### Build Steps

#### Step 1: Configure
```bash
# Create build directory
mkdir build
cd build

# Configure for Visual Studio 2026
cmake .. -G "Visual Studio 18 2026" -A x64
```

#### Step 2: Build HashLib
```bash
# Build HashLib library
cmake --build . --config Release --target HashLib
```

#### Step 3: Optional - Build Tests
```bash
# Note: The repository currently contains standalone test programs under `tests/`,
# but the CMake `add_subdirectory(tests)` hook expects `tests/CMakeLists.txt`,
# which is not present. BUILD_TESTS is therefore not usable yet.
```

#### Step 4: Verify
- Check `build/Release/HashLib.dll` (library)
- Check `build/bin/Release/HashLibTests.exe` (if tests enabled)

## 🔍 Troubleshooting

### Common Build Errors

#### 1. Platform Toolset Mismatch
```
Error: MSB8036: The PlatformToolset version 'v145' is not supported by this version of Visual Studio.
```
**Solution:**
- Install a Visual Studio version that provides toolset `v145`, or retarget the solution/projects to the toolset you have installed.
- In Visual Studio: **Project → Retarget solution**, or update **Project Properties → General → Platform Toolset**.

#### 2. Windows SDK Missing
```
Error: Cannot find Windows SDK version 10.0.19041.0
```
**Solution:**
- Install Windows SDK 10.0.19041.0 or later
- VS Installer → Individual Components → Windows SDK

#### 3. MFC Dependencies
```
Error: Cannot open include file 'afxwin.h'
```
**Solution:**
- Ensure MFC is installed: VS Installer → Desktop development with C++ → MFC
- Check project settings: Configuration Properties → General → Use of MFC → Use MFC in a Static Library

#### 4. Unicode Configuration
```
Warning: Mixing Unicode and non-Unicode
```
**Solution:**
- All projects use Unicode character set
- Check: Configuration Properties → General → Character Set → Use Unicode Character Set

### Performance Issues

#### Slow Builds
- **Solution:** Use Release configuration (/O2 optimization)
- **Parallel Builds:** Tools → Options → Projects and Solutions → Build and Run → maximum number of parallel project builds

#### Large Binary Size
- **Expected:** Release build ~8-12MB (MFC statically linked)
- **Debug builds:** Much larger due to symbols

### Missing Dependencies

#### Precompiled Libraries
- **Location:** `Services/` directory
- **Required:** SQLite, zlib, GeoIP, BugTrap, MiniUPnP
- **Status:** All included, no external downloads needed

#### Plugin Dependencies
- **Build Order:** Services → HashLib → Main App → Plugins
- **COM Registration:** Some plugins require registration (admin rights)

## 🧪 Testing Build

### Manual Testing
1. **Launch Envy.exe**
2. **Basic Functionality:**
   - UI loads without crashes
   - Settings dialog accessible
   - Network configuration works

3. **Protocol Testing:**
   - G2 connections (should work)
   - Basic ED2K connections (limited functionality)
   - BT magnet links (basic support)

### Automated Testing (Future)
**Current:** CI builds the solution, but does not run tests.

**What exists today:** Standalone integration tests in `tests/`:
- Run via `tests\run_integration_tests.bat` (will compile `tests\test_runner.cpp` if `cl.exe` is on PATH)
- Or compile manually from a Visual Studio Developer Command Prompt

See `tests/INTEGRATION_TEST_README.md` and `tests/MANUAL_CRYPTO_TESTING_GUIDE.md`.

## 📁 Build Output Structure

```
Envy/
├── Envy/Release x64/           # Main application output
│   ├── Envy.exe
│   └── Envy.pdb (if generated)
├── Services/*/Release x64/     # Service libraries (per-project)
├── Plugins/*/Release x64/      # Plugin DLLs (per-project)
└── HashLib/Release x64/        # HashLib output
```

## 🔄 Build System Limitations

### Known Issues
1. **CMake Incomplete:** Only HashLib builds via CMake
2. **No Cross-Platform:** Windows-only (MFC dependency)
3. **Precompiled Binaries:** Services contain prebuilt libraries
4. **Large Solution:** 30+ projects, complex dependencies

### Future Improvements
- **CMake Completion:** Full application build support
- **Cross-Platform:** Qt migration for Linux/macOS
- **Reproducible Builds:** Source-only dependencies

## Support

- **Limitations / roadmap:** [status](status.md) · [roadmap](roadmap.md)
- **Workflow:** [guide](guide.md)
- **CI:** GitHub Actions (see Actions tab)

---

**Last Updated:** January 2026
