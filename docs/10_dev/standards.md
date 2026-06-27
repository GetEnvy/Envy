# Code Standards and Guidelines

This document defines the coding standards and best practices for the Envy project.

## 📋 Table of Contents

- [Language Standards](#language-standards)
- [Naming Conventions](#naming-conventions)
- [Code Organization](#code-organization)
- [Error Handling](#error-handling)
- [Performance Guidelines](#performance-guidelines)
- [Documentation Standards](#documentation-standards)
- [Tools and Automation](#tools-and-automation)

## 🎯 Language Standards

### C++ Standard
- **Current baseline**: C++17 (configured as `stdcpp17` in the `.vcxproj` files)
- **Target**: C++20 (planned; adopt incrementally once the baseline is raised)
- **Migration**: Gradual adoption of modern features

### Platform and Framework
- **OS**: Windows (Win32/x64)
- **Framework**: Microsoft Foundation Classes (MFC)
- **Character Set**: Unicode (UTF-16)
- **Threading**: Windows threads with MFC synchronization

## 📝 Naming Conventions

### Classes and Structs
```cpp
class CDownloadTask;        // PascalCase with C prefix (MFC convention)
class CHashFunction;
struct FileMetadata;        // Plain structs without C prefix
```

### Member Variables
```cpp
class CExample
{
private:
    // String members
    CString m_sFileName;    // codebase convention: m_s* for member strings
    CString m_sPath;

    // Numeric members
    int m_nCount;
    DWORD m_nFileSize;
    size_t m_nBufferSize;

    // Boolean members
    BOOL m_bIsActive;       // BOOL for MFC compatibility
    BOOL m_bCompleted;

    // Pointer members
    CFile* m_pFile;
    std::unique_ptr<CData> m_pData;

    // Reference members (rare)
    CSettings& m_Settings;
};
```

### Functions and Methods
```cpp
// Public methods
void StartDownload();
bool IsCompleted() const;
CString GetFileName() const;

// Private methods
void UpdateProgress();
void CleanupResources();

// Event handlers (MFC style)
afx_msg void OnDownloadComplete();
afx_msg LRESULT OnProgressUpdate(WPARAM wParam, LPARAM lParam);
```

### Local Variables and Parameters
```cpp
void ProcessFile(const CString& strPath)
{
    // Local variables: camelCase with type prefix
    CString strName = GetFileName(strPath);
    int nSize = GetFileSize(strPath);
    bool bExists = FileExists(strPath);

    // Loop variables
    for (int i = 0; i < nCount; i++) { ... }

    // Pointers and references
    CFile* pFile = nullptr;
    auto& settings = GetSettings();
}
```

### Constants and Enums
```cpp
// Constants
constexpr size_t MAX_BUFFER_SIZE = 8192;
constexpr DWORD DEFAULT_TIMEOUT = 30000;
const int PROTOCOL_VERSION = 1;

// Enums
enum class DownloadState {
    Idle,
    Connecting,
    Downloading,
    Completed,
    Failed
};

// MFC-style enums (legacy)
enum {
    ID_DOWNLOAD_START = 1001,
    ID_DOWNLOAD_PAUSE = 1002,
    ID_DOWNLOAD_CANCEL = 1003
};
```

### Macros
```cpp
// Project-wide macros
#define ENVY_VERSION_MAJOR 4
#define ENVY_VERSION_MINOR 1

// Debug macros
#ifdef _DEBUG
#define TRACE_LINE() TRACE(_T("Line %d in %s\n"), __LINE__, __FILE__)
#endif
```

## 🗂️ Code Organization

### Header Files (.h/.hpp)

#### Structure
```cpp
#pragma once

// System includes (alphabetical)
#include <memory>
#include <string>
#include <vector>

// MFC includes
#include <afx.h>
#include <afxwin.h>

// Project includes
#include "DownloadTypes.h"
#include "HashFunction.h"

// Forward declarations (when possible)
class CConnection;
class CDownloadSource;

// Class definition
class CDownloadTask
{
    DECLARE_DYNAMIC(CDownloadTask)

public:
    // Public types and constants
    enum class Priority { Low, Normal, High };

    // Constructor/Destructor
    CDownloadTask();
    virtual ~CDownloadTask();

    // Public interface
    bool Start();
    void Cancel();
    Priority GetPriority() const;
    void SetPriority(Priority priority);

protected:
    // Protected interface (for derived classes)
    virtual void OnProgress(int nPercent);
    virtual void OnComplete();

private:
    // Private implementation
    void Initialize();
    void Cleanup();
    bool ValidateState() const;

    // Member variables (grouped logically)
    DownloadState m_state = DownloadState::Idle;
    Priority m_priority = Priority::Normal;
    CString m_sFileName;
    std::unique_ptr<CConnection> m_pConnection;
    std::vector<CDownloadSource*> m_Sources;
};
```

#### Implementation Files (.cpp)

```cpp
#include "stdafx.h"
#include "DownloadTask.h"
#include "Connection.h"
#include "DownloadSource.h"

// Static member initialization
const int CDownloadTask::DEFAULT_TIMEOUT = 30000;

// Constructor with member initialization
CDownloadTask::CDownloadTask()
    : m_state(DownloadState::Idle)
    , m_priority(Priority::Normal)
    , m_pConnection(nullptr)
{
}

// Public methods
bool CDownloadTask::Start()
{
    if (!ValidateState()) {
        return false;
    }

    try {
        m_pConnection = std::make_unique<CConnection>();
        m_state = DownloadState::Connecting;
        return true;
    }
    catch (const std::exception& e) {
        theApp.Message(MSG_ERROR, _T("Failed to start download: %hs"), e.what());
        return false;
    }
}

// Private methods
void CDownloadTask::Initialize()
{
    // Implementation
}

bool CDownloadTask::ValidateState() const
{
    // Implementation
    return true;
}
```

### File Naming
- **Headers**: `ClassName.h`
- **Implementation**: `ClassName.cpp`
- **Related files**: `ClassNameHelpers.cpp`, `ClassNameTests.cpp`

## ⚠️ Error Handling

### Exception Safety
```cpp
// ✅ Good: RAII and smart pointers
void ProcessFile(const CString& strPath)
{
    auto pFile = std::make_unique<CFile>();
    if (!pFile->Open(strPath)) {
        throw std::runtime_error("Failed to open file");
    }
    // File automatically closed on exception
}

// ❌ Bad: Manual cleanup needed
void ProcessFileBad(const CString& strPath)
{
    CFile* pFile = new CFile();
    if (!pFile->Open(strPath)) {
        delete pFile;  // Easy to forget
        throw std::runtime_error("Failed to open file");
    }
    delete pFile;
}
```

### MFC Error Handling
```cpp
// Win32 API error checking
HANDLE hFile = CreateFile(strPath, GENERIC_READ, 0, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
if (hFile == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();
    theApp.Message(MSG_ERROR, _T("CreateFile failed: %lu"), dwError);
    return false;
}

// MFC object validation
CFile file;
if (!file.Open(strPath, CFile::modeRead)) {
    theApp.Message(MSG_ERROR, _T("Failed to open file: %s"), (LPCTSTR)strPath);
    return false;
}
```

### Assertions and Debugging
```cpp
// Debug-time assertions
ASSERT(pConnection != nullptr);
ASSERT(nSize > 0);

// Release-safe checks
VERIFY(file.Open(strPath, CFile::modeRead));

// Debug logging
#ifdef _DEBUG
    TRACE(_T("Processing file: %s\n"), (LPCTSTR)strPath);
#endif
```

## ⚡ Performance Guidelines

### Memory Management
```cpp
// ✅ Good: Smart pointers
std::unique_ptr<CData> pData = std::make_unique<CData>();
auto pShared = std::make_shared<CResource>();

// ✅ Good: Reserve capacity
std::vector<CString> files;
files.reserve(expectedCount);

// ❌ Bad: Unnecessary copies
void ProcessData(CData data);  // Copies parameter
void ProcessData(CData& data); // ✅ Reference instead
```

### Loops and Algorithms
```cpp
// ✅ Good: Range-based for
for (const auto& file : files) {
    ProcessFile(file);
}

// ✅ Good: Standard algorithms
auto it = std::find_if(files.begin(), files.end(),
    [](const CFile& f) { return f.GetSize() > 1000000; });

// ❌ Bad: Manual loops when algorithms exist
for (size_t i = 0; i < files.size(); i++) {
    if (files[i].GetSize() > 1000000) {
        // Process
        break;
    }
}
```

### Hot Path Optimization
```cpp
// Critical network code: minimize allocations
class CPacketProcessor
{
public:
    bool ProcessPacket(CBuffer* pBuffer)
    {
        static thread_local std::vector<BYTE> tempBuffer;
        tempBuffer.resize(pBuffer->m_nLength);
        memcpy(tempBuffer.data(), pBuffer->m_pBuffer, pBuffer->m_nLength);
        return ProcessData(tempBuffer);
    }
};
```

## 📚 Documentation Standards

### Code Comments
```cpp
/**
 * @brief Downloads a file from multiple sources using P2P protocols
 *
 * This class manages the complete lifecycle of a file download,
 * including source discovery, connection management, and integrity
 * verification through hashing.
 */
class CDownloadTask
{
public:
    /**
     * @brief Starts the download process
     *
     * Initiates connections to available sources and begins
     * transferring file chunks. The download may be resumed
     * if partially complete.
     *
     * @return true if download started successfully, false otherwise
     *
     * @throws std::runtime_error if initialization fails
     * @throws NetworkException if no sources available
     */
    bool Start();

    /**
     * @brief Cancels the active download
     *
     * Stops all active transfers and releases resources.
     * The download can be resumed later if desired.
     */
    void Cancel();

private:
    /**
     * Validates the current download state
     * @return true if state is valid for operations
     */
    bool ValidateState() const;
};
```

### Inline Comments
```cpp
void CDownloadTask::ProcessChunk(const BYTE* pData, size_t nLength)
{
    // Verify chunk hash before processing
    if (!VerifyChunkHash(pData, nLength)) {
        LogError("Chunk hash verification failed");
        RequestChunkRetry();
        return;
    }

    // Update progress and write to file
    m_nDownloaded += nLength;
    WriteToFile(pData, nLength);

    // Notify observers of progress
    NotifyProgress(GetProgressPercent());
}
```

### TODO and FIXME Comments
```cpp
// TODO: Implement bandwidth throttling for slow connections
// FIXME: Race condition in multi-threaded source management
// HACK: Temporary workaround for protocol incompatibility
```

## 🛠️ Tools and Automation

### Code Formatting
- **Tool**: clang-format
- **Config**: `.clang-format` in repository root
- **Usage**: Run clang-format via your IDE or the clang-format executable

### Static Analysis
- **Tools/Configs**: `.clang-tidy`, `.cppcheck-suppressions` (plus IDE and CI tooling)
- **Primary execution**: CI workflows under `.github/workflows/`

### Build Verification
- **Script**: `.\build_all.ps1` (builds Debug/Release for Win32/x64)
- **Manual**: `msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64`
- **CI/CD**: GitHub Actions workflows

### Automated Checks
All changes must pass:
- ✅ Compilation without warnings
- ✅ Build on target platforms (`.\build_all.ps1` or CI)
- ✅ Code formatting (clang-format)
- ✅ Static analysis (CI where configured)

## Code review checklist

- [ ] Compiles without warnings; follows naming conventions
- [ ] Error handling robust; thread-safety considered
- [ ] Follows existing patterns; backward compatibility maintained
- [ ] Tests added/updated where applicable (e.g. integration tests in `tests/`)

---

**Related:** [Guide](guide.md) · [Build](build.md) · [Contributing](contributing.md)

**Last Updated:** January 2026
