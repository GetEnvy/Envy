# Developer Guide

This guide provides comprehensive information for developers working on the Envy P2P client project.

## 📋 Table of Contents

- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Development Workflow](#development-workflow)
- [Code Standards](#code-standards)
- [Testing](#testing)
- [Build System](#build-system)
- [Contributing](#contributing)

## 🚀 Getting Started

### Prerequisites

- **Visual Studio 2026** (18.x) or newer
  - MSVC toolset `v145` (as configured in the `.vcxproj` files)
  - Workloads/components: Desktop development with C++, MFC/ATL, Windows 10/11 SDK
- **Windows SDK** 10.0.x (projects target `10.0`)
- **CMake** 3.20+ (optional; incomplete, HashLib only)
- **Git** 2.30+

### Initial Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Mika3578/Envy.git
   cd Envy
   ```

2. **Open in Visual Studio:**
   - Open `Visual Studio\Envy.sln`
   - Select your preferred configuration (Debug/Release, x64/Win32)

3. **Build the project:**
   - Build → Build Solution (Ctrl+Shift+B)

### Recommended Tools

- **ClangFormat**: For code formatting (`.clang-format` provided)
- **GitHub Copilot**: AI code completion (see [AI Coding Guide](ai-coding-guide.md))
- **Cursor AI**: Project-specific rules live under `.cursor/rules/`

## 🏗️ Project Structure

```
Envy/
├── Envy/                    # Main application source (network, library, UI, protocols)
├── HashLib/                 # Hash algorithm library
├── Services/                # Bundled libs (SQLite, zlib, etc.)
├── Plugins/                 # Plugin implementations
├── Visual Studio/           # Solution and build configs
├── scripts/                 # Version and build scripts
├── tests/                   # Integration tests (manual runner)
└── docs/                    # Documentation
```

### Key Components

- **Network Layer**: TCP/UDP sockets, protocol implementations
- **Library System**: File management, metadata, search
- **Download Manager**: Multi-source downloads, hash verification
- **UI Framework**: MFC-based with skinning support
- **Plugin System**: COM-based extensibility

## 🔄 Development Workflow

### Daily Development

1. **Pull latest changes:**
   ```bash
   git pull origin develop
   ```

2. **Create feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make changes with frequent commits:**
   ```bash
   # Make small, focused changes
   git add specific-files
   git commit -m "Clear, descriptive commit message"
   ```

4. **Test your changes:**
   ```bash
   # Build all configurations/platforms
   .\build_all.ps1
   ```

5. **Format code:**
   ```bash
   # Use clang-format via your IDE or the clang-format executable
   # (configuration is in the repository root: .clang-format)
   ```

### Before Committing

- ✅ Code compiles without warnings
- ✅ `.\build_all.ps1` succeeds (or equivalent); run `tests\run_integration_tests.bat` if you touch protocol/hash code
- ✅ Code follows [standards](standards.md); format with clang-format
- ✅ Docs updated if behaviour or setup changes

### Pull Request Process

1. **Push your branch:**
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create Pull Request** on GitHub with:
   - Clear description of changes
   - Reference to any related issues
   - Screenshots for UI changes

3. **Address review feedback** and update as needed

## 📏 Code Standards

### Language Standards

- **C++ Standard**: C++17 (current baseline in project files; C++20 is a future target)
- **Character Set**: Unicode (UTF-16)
- **Framework**: Microsoft Foundation Classes (MFC)

### Naming Conventions

#### Classes
```cpp
class CDownloadTask;        // PascalCase with C prefix (MFC convention)
class CLibraryFile;
```

#### Member Variables
```cpp
class CExample
{
private:
    CString m_sFileName;        // strings use m_s* in this codebase
    int m_nCount;               // m_n prefix for numbers
    DWORD m_nFileSize;
    CFile* m_pFile;             // m_p prefix for pointers
    BOOL m_bIsActive;           // BOOL for MFC/Win32 compatibility
};
```

#### Functions
```cpp
void OnDownload();              // PascalCase
bool GetFileSize();
void UpdateProgress();
```

### Modern C++ Features

**✅ Use:**
- Smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- `nullptr` instead of `NULL`
- Range-based `for` loops
- `auto` for obvious types
- `constexpr` for compile-time constants
- `override` and `final` keywords

**❌ Avoid:**
- Raw `new`/`delete` (use smart pointers)
- `NULL` (use `nullptr`)
- C-style casts (use C++ casts)
- Manual memory management

See [Modern C++ Guide](modern-cpp-guide.md) for detailed examples.

### Code Organization

- **Header files**: Include guards, forward declarations, clear structure
- **Implementation files**: Logical function grouping, clear comments
- **Error handling**: Exceptions for exceptional cases, proper resource cleanup
- **Thread safety**: Document thread requirements, use appropriate synchronization

## 🧪 Testing

### Integration Tests (Current)

The repository contains a standalone integration test runner in `tests/` (not wired into the Visual Studio solution yet).

**Run via batch script:**
```batch
cd tests
run_integration_tests.bat
```

**Or compile manually (Developer Command Prompt):**
```batch
cd tests
cl /EHsc /I"../Envy" /I"." test_runner.cpp /Fe:test_runner.exe
test_runner.exe
```

### Testing Guidelines

- Write tests for new functionality
- Test edge cases and error conditions
- Ensure tests are fast and reliable
- Use descriptive test names
- Follow the existing test structure

See `tests/INTEGRATION_TEST_README.md` and `tests/MANUAL_CRYPTO_TESTING_GUIDE.md`.

## 🔨 Build System

### Visual Studio Builds

- **Solution**: `Visual Studio\Envy.sln`
- **Configurations**: Debug, Release
- **Platforms**: Win32, x64 (x64 recommended)
- **Toolset**: v145 (VS2026)

### CMake Builds (Modern)

```powershell
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release --parallel

# Install
cmake --install build --config Release
```

### Build Verification

**Test all configurations:**
```powershell
.\build_all.ps1
```

### Static Analysis

Static analysis is currently performed in CI (and via IDE tooling). Repo configs include `.clang-tidy` and `.cppcheck-suppressions`.

## 🤝 Contributing

### Contribution Guidelines

1. **Understand the codebase** before making changes
2. **Follow existing patterns** and conventions
3. **Make minimal changes** that solve the specific problem
4. **Test thoroughly** before submitting
5. **Update documentation** as needed

### Areas for Contribution

- **Protocol improvements**: BitTorrent, Gnutella2, eDonkey
- **Performance optimization**: Memory usage, CPU efficiency
- **UI enhancements**: Modern Windows features, accessibility
- **Testing**: Additional test coverage, integration tests
- **Documentation**: User guides, API documentation

### Getting Help

- **Issues**: [GitHub Issues](../../issues)
- **Discussions**: [GitHub Discussions](../../discussions)
- **Documentation**: Check this guide and related docs
- **Code**: Look for similar implementations in the codebase

## 📚 Related

- [Build](build.md) · [Status](status.md) · [Contributing](contributing.md) · [Standards](standards.md)
- [Architecture](../20_arch/architecture.md) · [AI Coding Guide](ai-coding-guide.md) · [Modern C++](modern-cpp-guide.md) · [Agents & Automation](agents-and-automation.md)

---

**Last Updated:** January 2026
