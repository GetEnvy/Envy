# AI Agent Best Practices for Envy

This document provides comprehensive guidance for AI coding agents working on the Envy P2P filesharing and BitTorrent client project.

## Table of Contents
- [Project Overview](#project-overview)
- [Development Environment](#development-environment)
- [Code Standards and Conventions](#code-standards-and-conventions)
- [Architecture and Design](#architecture-and-design)
- [Security Best Practices](#security-best-practices)
- [Performance Guidelines](#performance-guidelines)
- [Testing and Quality Assurance](#testing-and-quality-assurance)
- [Common Patterns and Anti-Patterns](#common-patterns-and-anti-patterns)
- [Workflow and Process](#workflow-and-process)
- [Resources and References](#resources-and-references)

---

## Project Overview

### What is Envy?
Envy is a multi-network P2P filesharing and BitTorrent client for Windows, supporting:
- **BitTorrent**: Full-featured client with DHT and PEX support
- **Gnutella2**: Advanced query routing and ultra-peer capabilities
- **Gnutella**: Classic P2P network support
- **eDonkey2000**: Compatible with eDonkey and eMule networks

### Key Characteristics
- **Language**: C++20 (modern C++ features)
- **Framework**: Microsoft Foundation Classes (MFC)
- **Platform**: Windows (Win32/x64)
- **Build System**: Visual Studio 2026, MSBuild
- **Character Set**: Unicode (UTF-16)
- **License**: AGPL v3.0

### Project Structure
```
Envy/
├── Envy/           # Main application source code
├── Plugins/        # Plugin modules (COM-based)
├── Services/       # Utility libraries (zlib, SQLite, GeoIP, etc.)
├── HashLib/        # Hash calculation library
├── TorrentEnvy/    # BitTorrent wizard utilities
├── Skins/          # XML-based UI themes
├── Languages/      # Localization files
├── Schemas/        # XML schemas for data formats
└── Visual Studio/  # Solution and project files
```

### Core Subsystems
1. **Network Stack**: TCP/UDP sockets, protocol implementations
2. **Library System**: File management, metadata, search
3. **Download Manager**: Multi-source downloads, hash verification
4. **Upload Manager**: Sharing and bandwidth management
5. **UI Framework**: MFC-based with skinning support
6. **Plugin System**: COM-based extensibility

---

## Development Environment

### Required Software
- **Visual Studio 2026** (v18.0 or later)
  - Desktop development with C++ workload
  - Windows 10/11 SDK (10.0.19041.0 or later)
  - MFC and ATL support
  - C++ Clang tools for Windows (optional)

### Recommended Tools
- **ClangFormat**: Code formatting (`.clang-format` provided)
- **GitHub Copilot**: AI code completion (see `.github/copilot-instructions.md`)
- **Cursor AI**: Project rules live under `.cursor/rules/`

### Build Configurations
- **Debug**: Full debug symbols, no optimizations, verbose logging
- **Release**: Optimizations enabled, release runtime, smaller binary
- **Platforms**: Win32 (x86) and x64 (recommended)

### Important Files to Review
- `.github/copilot-instructions.md` - GitHub Copilot configuration
- `.cursor/rules/` - Cursor rules and project context
- `docs/00_index/CURSOR_INDEX.md` - Doc entry points
- `docs/10_dev/guide.md` - Developer workflow
- `docs/10_dev/build.md` - Build instructions
- `docs/10_dev/standards.md` - Code standards
- `.github/MODERN_CPP_GUIDE.md` - Modern C++ examples
- `docs/10_dev/roadmap.md` - Project roadmap and priorities

---

## Code Standards and Conventions

### Naming Conventions

#### Classes
```cpp
class CDownloadTask;        // PascalCase with C prefix (MFC convention)
class CLibraryFile;
class CNetworkConnection;
```

#### Member Variables
```cpp
class CExample
{
private:
    CString m_sFileName;        // codebase convention: m_s* for member strings
    int m_nCount;               // m_n prefix for numbers/counts
    DWORD m_nFileSize;
    CFile* m_pFile;             // m_p prefix for pointers
    bool m_bIsActive;           // m_b prefix for booleans
    std::unique_ptr<CData> m_pData;
};
```

#### Functions and Methods
```cpp
void OnDownload();              // PascalCase
bool GetFileSize();
void UpdateProgress();
```

#### Constants and Macros
```cpp
constexpr size_t MAX_BUFFER_SIZE = 8192;
constexpr DWORD DEFAULT_TIMEOUT = 30000;
#define ENVY_VERSION_MAJOR 1
```

#### Local Variables
```cpp
void ProcessFile()
{
    CString strPath;           // camelCase with type prefix
    int nCount = 0;
    auto pFile = GetFile();
}
```

### Modern C++20 Features

#### DO USE
```cpp
// Smart pointers
auto pFile = std::make_unique<CFragmentedFile>();
auto pShared = std::make_shared<CDownload>();

// nullptr instead of NULL
CConnection* pConn = nullptr;
if (pConn != nullptr) { ... }

// auto for obvious types
auto it = map.find(key);
auto result = CalculateHash();

// Range-based for loops
for (const auto& item : collection) { ... }

// constexpr for compile-time constants
constexpr size_t BUFFER_SIZE = 4096;

// override keyword for virtual functions
void OnUpdate() override;

// using instead of typedef
using FileMap = std::map<CString, CFile*>;

// Structured bindings
auto [success, value] = GetResult();

// std::optional for optional values
std::optional<CString> FindUser(DWORD id);

// Lambda expressions
std::sort(files.begin(), files.end(),
    [](CFile* a, CFile* b) { return a->GetSize() < b->GetSize(); });
```

#### DO NOT USE
```cpp
// Raw new/delete in new code - use smart pointers
CFile* p = new CFile();  // ❌ Avoid
delete p;

// NULL - use nullptr
if (ptr != NULL) { ... }  // ❌ Avoid

// typedef - use using
typedef std::map<CString, int> MyMap;  // ❌ Avoid

// C-style casts - use C++ casts
int n = (int)value;  // ❌ Avoid
int n = static_cast<int>(value);  // ✅ Correct
```

### Code Organization

#### Header Files
```cpp
#pragma once

// Include order:
#include "stdafx.h"          // PCH first (if exists)
#include "ProjectHeader.h"   // Project headers
#include <vector>            // System headers
#include <memory>

// Forward declarations
class CConnection;
class CDownload;

// Class definition
class CDownloadTask
{
public:
    // Public types and enums
    enum class State { Idle, Running, Completed, Failed };

    // Constructors/Destructor
    CDownloadTask();
    virtual ~CDownloadTask();

    // Public methods
    bool Start();
    void Cancel();
    State GetState() const;

protected:
    // Protected methods
    virtual void OnProgress(int nPercent);

private:
    // Private members
    State m_state = State::Idle;
    std::unique_ptr<CConnection> m_pConnection;
    CString m_sFileName;

    // Private methods
    void UpdateProgress();

    // Disable copy (if appropriate)
    CDownloadTask(const CDownloadTask&) = delete;
    CDownloadTask& operator=(const CDownloadTask&) = delete;
};
```

#### Implementation Files
```cpp
#include "stdafx.h"
#include "DownloadTask.h"
#include "Connection.h"

// Member initialization with member initializers
CDownloadTask::CDownloadTask()
    : m_state(State::Idle)
    , m_pConnection(nullptr)
{
}

// Use RAII for resource management
void CDownloadTask::Start()
{
    m_pConnection = std::make_unique<CConnection>();
    // Resources automatically cleaned up
}
```

### Memory Management

#### RAII Principles
```cpp
// ✅ Good: Resource automatically managed
class CFileHandler
{
private:
    std::unique_ptr<CFile> m_pFile;

public:
    CFileHandler(const CString& path)
        : m_pFile(std::make_unique<CFile>())
    {
        m_pFile->Open(path);
    }
    // Destructor automatically cleans up
};

// ❌ Bad: Manual resource management
class CFileHandler
{
private:
    CFile* m_pFile;

public:
    CFileHandler() : m_pFile(new CFile()) { }
    ~CFileHandler() { delete m_pFile; }  // Easy to forget
};
```

#### MFC Objects
```cpp
// MFC objects have specific ownership patterns
CWnd* pWnd = new CMyWindow();
pWnd->Create(...);  // MFC manages lifetime for windows

// For non-window MFC objects
CFile file;  // Stack allocation preferred
// or
std::unique_ptr<CFile> pFile(new CFile());
```

### Error Handling

```cpp
// Use exceptions for exceptional conditions
void LoadFile(const CString& path)
{
    CFile file;
    if (!file.Open(path, CFile::modeRead))
    {
        throw std::runtime_error("Failed to open file");
    }
    // Process file
}

// Check Win32 API return values
HANDLE hFile = CreateFile(...);
if (hFile == INVALID_HANDLE_VALUE)
{
    DWORD error = GetLastError();
    // Log error with context
    theApp.Message(MSG_ERROR, _T("CreateFile failed: %lu"), error);
    return false;
}

// Use ASSERT for debug-time checks
ASSERT(pFile != nullptr);
ASSERT(nSize > 0);

// Use VERIFY when expression needed in release
VERIFY(file.Open(path, CFile::modeRead));
```

### Thread Safety

```cpp
class CThreadSafeCache
{
private:
    mutable std::mutex m_mutex;
    std::map<CString, CData> m_cache;

public:
    void Add(const CString& key, const CData& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache[key] = data;
    }

    std::optional<CData> Get(const CString& key) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return it->second;
        return std::nullopt;
    }
};

// Document thread safety
/// <summary>
/// Thread-safe cache for file data.
/// All methods can be called from any thread.
/// </summary>
class CFileCache { ... };
```

---

## Architecture and Design

### MFC Patterns

#### Message Maps
```cpp
class CMyWindow : public CWnd
{
    DECLARE_MESSAGE_MAP()

protected:
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg LRESULT OnCustomMessage(WPARAM wParam, LPARAM lParam);
};

BEGIN_MESSAGE_MAP(CMyWindow, CWnd)
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_MESSAGE(WM_CUSTOM, &CMyWindow::OnCustomMessage)
END_MESSAGE_MAP()
```

#### Document/View Architecture
```cpp
// Document holds data
class CEnvyDoc : public CDocument
{
    // Data members
    // Serialization
};

// View displays data
class CEnvyView : public CView
{
    // UI rendering
    CEnvyDoc* GetDocument() const;
};
```

### Network Protocol Patterns

```cpp
// Packet base class
class CPacket
{
public:
    virtual void ToBuffer(CBuffer* pBuffer) = 0;
    virtual bool ReadBuffer(CBuffer* pBuffer) = 0;
    virtual CString GetType() const = 0;
};

// Specific protocol implementation
class CBTPacket : public CPacket
{
public:
    void ToBuffer(CBuffer* pBuffer) override;
    bool ReadBuffer(CBuffer* pBuffer) override;
    CString GetType() const override;
};
```

### File Management Patterns

```cpp
// Library file with metadata
class CLibraryFile
{
private:
    CString m_sPath;
    QWORD m_nSize;
    CSchemaPtr m_pSchema;
    CXMLElement* m_pMetadata;

public:
    bool Scan();                    // Scan file and extract metadata
    bool VerifyHash();              // Verify file integrity
    void UpdateMetadata(CXMLElement* pXML);
};
```

### Plugin System (COM)

```cpp
// Plugin interface
interface IPlugin : public IUnknown
{
    STDMETHOD(Initialize)(LPVOID pHost) PURE;
    STDMETHOD(Shutdown)() PURE;
    STDMETHOD(OnCommand)(UINT nCommandID) PURE;
};

// Plugin implementation
class CMyPlugin : public IPlugin
{
    // Implement interface methods
    STDMETHOD(Initialize)(LPVOID pHost) override;
    STDMETHOD(Shutdown)() override;
    STDMETHOD(OnCommand)(UINT nCommandID) override;
};
```

---

## Security Best Practices

### Input Validation

```cpp
// Validate network input
bool CConnection::ReadString(CString& strOutput, int nMaxLength)
{
    DWORD nLength;
    if (!Read(&nLength, sizeof(nLength)))
        return false;

    // Validate length to prevent buffer overflow
    if (nLength > nMaxLength || nLength > 1024 * 1024)
    {
        theApp.Message(MSG_ERROR, _T("Invalid string length: %lu"), nLength);
        return false;
    }

    // Safe read
    std::vector<BYTE> buffer(nLength);
    if (!Read(buffer.data(), nLength))
        return false;

    strOutput = CString(reinterpret_cast<LPCSTR>(buffer.data()), nLength);
    return true;
}
```

### File Path Sanitization

```cpp
bool ValidateFilePath(const CString& strPath)
{
    // Prevent directory traversal
    if (strPath.Find(_T("..")) >= 0)
        return false;

    // Check for invalid characters
    static const CString strInvalid = _T("<>:\"|?*");
    for (int i = 0; i < strInvalid.GetLength(); i++)
    {
        if (strPath.Find(strInvalid[i]) >= 0)
            return false;
    }

    // Validate path length
    if (strPath.GetLength() > MAX_PATH)
    {
        // Handle long paths appropriately
        return false;
    }

    return true;
}
```

### Secure String Operations

```cpp
// ❌ Bad: Unsafe string operations
char buffer[256];
strcpy(buffer, userInput);  // Buffer overflow risk
sprintf(buffer, "%s", data);  // Unsafe

// ✅ Good: Safe string operations
char buffer[256];
strncpy_s(buffer, sizeof(buffer), userInput, _TRUNCATE);
sprintf_s(buffer, sizeof(buffer), "%s", data);

// Or better: Use CString
CString str = userInput;
str.Format(_T("%s"), data);
```

### Cryptographic Security

```cpp
// Use secure random for security contexts
void GenerateSessionID(BYTE* pBuffer, size_t nLength)
{
    // Use cryptographically secure random
    HCRYPTPROV hProv;
    if (CryptAcquireContext(&hProv, nullptr, nullptr,
                           PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        CryptGenRandom(hProv, static_cast<DWORD>(nLength), pBuffer);
        CryptReleaseContext(hProv, 0);
    }
    else
    {
        // Fallback or error handling
        throw std::runtime_error("Failed to generate secure random");
    }
}

// Clear sensitive data after use
void ProcessPassword(CString& strPassword)
{
    // Use password
    AuthenticateUser(strPassword);

    // Clear from memory
    SecureZeroMemory(strPassword.GetBuffer(),
                    strPassword.GetLength() * sizeof(TCHAR));
    strPassword.ReleaseBuffer();
}
```

### Network Security

```cpp
// Always use HTTPS/TLS for network communications
bool CHttpRequest::SendRequest()
{
    // Prefer HTTPS
    if (m_sURL.Find(_T("http://")) == 0)
    {
        theApp.Message(MSG_WARNING,
            _T("Insecure HTTP connection to %s"), (LPCTSTR)m_sURL);
    }

    // Validate SSL/TLS certificates
    DWORD dwFlags = INTERNET_FLAG_SECURE;
    // Don't ignore certificate errors in production

    return Send(dwFlags);
}
```

---

## Performance Guidelines

### General Principles

```cpp
// Pass large objects by const reference
void ProcessFile(const CLibraryFile& file);  // ✅ Good
void ProcessFile(CLibraryFile file);         // ❌ Bad: copies

// Reserve capacity for vectors when size is known
std::vector<CString> files;
files.reserve(expectedCount);  // Avoid multiple reallocations
for (int i = 0; i < expectedCount; i++)
{
    files.push_back(GetFileName(i));
}

// Use move semantics for transferring ownership
CData CreateData()
{
    CData data;
    // ... populate data
    return data;  // Move, not copy (C++11+)
}

// Cache expensive computations
class CHashCache
{
private:
    std::map<CString, CString> m_cache;

public:
    CString GetHash(const CString& path)
    {
        auto it = m_cache.find(path);
        if (it != m_cache.end())
            return it->second;  // Cache hit

        CString hash = ComputeExpensiveHash(path);
        m_cache[path] = hash;
        return hash;
    }
};
```

### UI Thread Responsiveness

```cpp
// ❌ Bad: Blocking UI thread
void CMyDialog::OnDownload()
{
    for (int i = 0; i < 1000; i++)
    {
        ProcessFile(i);  // Long operation
    }
    UpdateDisplay();
}

// ✅ Good: Background thread with UI updates
void CMyDialog::OnDownload()
{
    // Start background task
    std::thread worker([this]()
    {
        for (int i = 0; i < 1000; i++)
        {
            ProcessFile(i);

            // Update UI on main thread
            PostMessage(WM_PROGRESS_UPDATE, i);
        }
    });
    worker.detach();
}

LRESULT CMyDialog::OnProgressUpdate(WPARAM wParam, LPARAM lParam)
{
    int progress = static_cast<int>(wParam);
    UpdateDisplay(progress);
    return 0;
}
```

### Hot Path Optimization

```cpp
// Critical path: network packet processing
class CPacketHandler
{
public:
    bool ProcessPacket(CBuffer* pBuffer)
    {
        // Minimize allocations in hot path
        static thread_local std::vector<BYTE> buffer;
        buffer.resize(pBuffer->m_nLength);

        // Avoid virtual function calls if possible
        // Use inline functions for small operations
        return ProcessData(buffer.data(), buffer.size());
    }
};
```

### Memory Optimization

```cpp
// Use appropriate data structures
// For lookup: std::map or std::unordered_map
std::unordered_map<CString, CFile*> fileMap;  // O(1) lookup

// For ordered iteration: std::map
std::map<DWORD, CDownload*> downloadsByID;  // Sorted by ID

// For simple array: std::vector
std::vector<CPacket*> packets;  // Contiguous memory

// Avoid unnecessary copies with string_view
void LogMessage(std::wstring_view message)  // No copy
{
    // Use message without copying
}
```

---

## Testing and Quality Assurance

### Unit Testing Principles

```cpp
// Test structure
void TestDownloadSpeed()
{
    // Arrange
    CDownload download;
    download.SetSpeed(1024 * 1024);  // 1 MB/s

    // Act
    DWORD speed = download.GetSpeed();

    // Assert
    ASSERT(speed == 1024 * 1024);
}

// Test edge cases
void TestEmptyFile()
{
    CFile file;
    ASSERT(file.GetSize() == 0);
    ASSERT(file.GetHash().IsEmpty());
}

// Test error conditions
void TestInvalidInput()
{
    CParser parser;
    bool result = parser.Parse(nullptr);
    ASSERT(result == false);
}
```

### Manual Testing Checklist

Before committing changes:
- [ ] Code compiles without errors or warnings
- [ ] Tested on both Debug and Release configurations
- [ ] Tested on both Win32 and x64 platforms
- [ ] No memory leaks (check with debug CRT)
- [ ] UI remains responsive
- [ ] No crashes or exceptions
- [ ] Functionality works as expected
- [ ] Edge cases handled properly
- [ ] Error messages are meaningful

### Build Verification

```bash
# Clean build
MSBuild /t:Clean,Build /p:Configuration=Debug /p:Platform=x64

# Check for warnings
MSBuild /p:Configuration=Release /p:Platform=x64 /v:minimal

# Run static analysis (if configured)
# Check Code Analysis in project properties
```

### Performance Testing

```cpp
// Profile critical paths
#include <chrono>

void ProfileFunction()
{
    auto start = std::chrono::high_resolution_clock::now();

    // Code to profile
    ProcessLargeFile();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);

    TRACE(_T("ProcessLargeFile took %lld ms\n"), duration.count());
}
```

---

## Common Patterns and Anti-Patterns

### ✅ Good Patterns

#### Singleton Pattern (for application-wide objects)
```cpp
class CSettings
{
private:
    static CSettings* s_pInstance;
    CSettings() = default;

public:
    static CSettings& Instance()
    {
        static CSettings instance;
        return instance;
    }

    // Prevent copying
    CSettings(const CSettings&) = delete;
    CSettings& operator=(const CSettings&) = delete;
};
```

#### Factory Pattern (for object creation)
```cpp
class CPacketFactory
{
public:
    static std::unique_ptr<CPacket> CreatePacket(BYTE nType)
    {
        switch (nType)
        {
        case PACKET_G2:
            return std::make_unique<CG2Packet>();
        case PACKET_BT:
            return std::make_unique<CBTPacket>();
        default:
            return nullptr;
        }
    }
};
```

#### Observer Pattern (for event notification)
```cpp
class IDownloadObserver
{
public:
    virtual void OnDownloadProgress(CDownload* pDownload, int nPercent) = 0;
    virtual void OnDownloadComplete(CDownload* pDownload) = 0;
};

class CDownload
{
private:
    std::vector<IDownloadObserver*> m_observers;

public:
    void AddObserver(IDownloadObserver* pObserver)
    {
        m_observers.push_back(pObserver);
    }

    void NotifyProgress(int nPercent)
    {
        for (auto pObserver : m_observers)
        {
            pObserver->OnDownloadProgress(this, nPercent);
        }
    }
};
```

### ❌ Anti-Patterns to Avoid

#### God Objects
```cpp
// ❌ Bad: One class does everything
class CApplication
{
    void ManageNetwork();
    void ManageDownloads();
    void ManageUI();
    void ManageFiles();
    void ManageEverything();
    // 5000+ lines of code
};

// ✅ Good: Separate concerns
class CNetworkManager { ... };
class CDownloadManager { ... };
class CUIManager { ... };
class CFileManager { ... };
```

#### Magic Numbers
```cpp
// ❌ Bad: Magic numbers
if (size > 1048576)  // What does this number mean?
    return false;

// ✅ Good: Named constants
constexpr size_t MAX_FILE_SIZE = 1024 * 1024;  // 1 MB
if (size > MAX_FILE_SIZE)
    return false;
```

#### Naked Pointers in New Code
```cpp
// ❌ Bad: Manual memory management
CFile* pFile = new CFile();
// ... might throw exception
delete pFile;  // Never reached if exception

// ✅ Good: Smart pointers
auto pFile = std::make_unique<CFile>();
// Automatically cleaned up even with exceptions
```

#### Ignoring Return Values
```cpp
// ❌ Bad: Ignoring important return values
file.Open(path, CFile::modeRead);  // Did it succeed?
CreateThread(...);  // Handle == nullptr?

// ✅ Good: Check return values
if (!file.Open(path, CFile::modeRead))
{
    theApp.Message(MSG_ERROR, _T("Failed to open file: %s"), (LPCTSTR)path);
    return false;
}
```

---

## Workflow and Process

### Before Starting Work

1. **Understand the issue**: Read the problem statement thoroughly
2. **Review related code**: Explore the codebase to understand context
3. **Check existing patterns**: Look for similar implementations
4. **Plan minimal changes**: Identify the smallest change needed

### During Development

1. **Make incremental changes**: Small, focused commits
2. **Test frequently**: Build and test after each significant change
3. **Follow conventions**: Match existing code style
4. **Document as you go**: Add comments for complex logic
5. **Check for side effects**: Consider impact on other components

### Before Committing

1. **Build verification**:
   ```bash
   # Clean build both configurations
   MSBuild /t:Clean,Build /p:Configuration=Debug /p:Platform=x64
   MSBuild /t:Clean,Build /p:Configuration=Release /p:Platform=x64
   ```

2. **Code review checklist**:
   - [ ] No compiler warnings
   - [ ] No memory leaks
   - [ ] Follows naming conventions
   - [ ] Proper error handling
   - [ ] Thread-safe if needed
   - [ ] Documentation updated

3. **Format code**:
   ```bash
   clang-format -i ModifiedFile.cpp
   ```

### Git Workflow

```bash
# Create feature branch
git checkout -b feature/my-feature

# Make changes and commit incrementally
git add ChangedFile.cpp
git commit -m "Add feature X with proper error handling"

# Keep commits focused and atomic
git add AnotherFile.cpp
git commit -m "Update documentation for feature X"

# Push and create pull request
git push origin feature/my-feature
```

### Code Review

When reviewing code:
- **Check correctness**: Does it solve the problem?
- **Check safety**: Memory leaks, buffer overflows, thread safety?
- **Check performance**: Any obvious bottlenecks?
- **Check style**: Follows project conventions?
- **Check tests**: Are edge cases covered?

---

## Resources and References

### Project Documentation
- [Documentation Index](../README.md) - Documentation overview
- [Development Guide](guide.md) - Development setup and guide
- [Contributing Guide](contributing.md) - Contribution guidelines
- [Modern C++ Guide](modern-cpp-guide.md) - C++20 modernization guide
- [Architecture](../20_arch/architecture.md) - System design
- [Agents and Automation](agents-and-automation.md) - Development automation tools
- [Modernization Summary](modernization-summary.md) - Recent improvements

### External Documentation
- [Microsoft C++ Documentation](https://docs.microsoft.com/cpp/)
- [MFC Reference](https://docs.microsoft.com/cpp/mfc/)
- [Windows API Reference](https://docs.microsoft.com/windows/win32/api/)
- [C++20 Reference](https://en.cppreference.com/w/cpp/20)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)

### BitTorrent Protocol
- [BitTorrent Protocol Specification](http://www.bittorrent.org/beps/bep_0003.html)
- [DHT Protocol](http://www.bittorrent.org/beps/bep_0005.html)
- [PEX Protocol](http://www.bittorrent.org/beps/bep_0011.html)

### Tools and Configuration
- `.clang-format` - Code formatting rules
- `.github/workflows/` - CI/CD configuration

---

## Key Reminders for AI Agents

### Critical Rules
1. **Minimal Changes**: Make the smallest change that solves the problem
2. **Don't Break Working Code**: Unless fixing a security issue
3. **Follow Existing Patterns**: Match the style of surrounding code
4. **Test Early and Often**: Build and test frequently
5. **Security First**: Validate all external input
6. **Document Complex Logic**: Explain "why" not "what"
7. **Respect MFC Patterns**: Follow MFC ownership and lifetime rules
8. **Unicode Always**: Use Unicode APIs (W suffix)
9. **Thread Safety**: Document and implement properly
10. **Performance Matters**: This is a real-time network application

### Common Mistakes to Avoid
- ❌ Using raw new/delete instead of smart pointers
- ❌ Blocking the UI thread with long operations
- ❌ Ignoring return values from critical functions
- ❌ Using NULL instead of nullptr
- ❌ Creating memory leaks with COM objects
- ❌ Not checking buffer sizes before copying
- ❌ Using unsafe string functions
- ❌ Introducing race conditions in multi-threaded code
- ❌ Making changes without understanding existing patterns
- ❌ Breaking backward compatibility without good reason

### When in Doubt
1. Look for similar code in the project
2. Check the existing documentation
3. Follow the principle of least surprise
4. Ask for clarification rather than guessing
5. Make conservative choices that preserve stability

---

**Related:** [Guide](guide.md) · [Standards](standards.md) · [Build](build.md)

**Last Updated:** January 2026
