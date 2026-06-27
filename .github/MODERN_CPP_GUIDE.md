# Modern C++ Best Practices for Envy

This guide provides specific examples of how to use modern C++20 features in the Envy codebase.

## Table of Contents
- [Smart Pointers](#smart-pointers)
- [Auto Type Deduction](#auto-type-deduction)
- [Range-Based For Loops](#range-based-for-loops)
- [nullptr](#nullptr)
- [constexpr](#constexpr)
- [String View](#string-view)
- [Optional Values](#optional-values)
- [Structured Bindings](#structured-bindings)
- [Concepts (C++20)](#concepts-c20)
- [Ranges (C++20)](#ranges-c20)

## Smart Pointers

### Before (Old Style)
```cpp
class CDownload
{
private:
	CConnection* m_pConnection;

public:
	CDownload() : m_pConnection(NULL) {}

	~CDownload()
	{
		if (m_pConnection != NULL)
		{
			delete m_pConnection;
			m_pConnection = NULL;
		}
	}

	void CreateConnection()
	{
		m_pConnection = new CConnection();
	}
};
```

### After (Modern Style)
```cpp
class CDownload
{
private:
	std::unique_ptr<CConnection> m_pConnection;

public:
	CDownload() = default;
	~CDownload() = default; // Automatic cleanup

	void CreateConnection()
	{
		m_pConnection = std::make_unique<CConnection>();
	}
};
```

## Auto Type Deduction

### Before
```cpp
CString GetFileName()
{
	CString strFileName = _T("file.txt");
	return strFileName;
}

void ProcessFiles()
{
	CAtlList<CString> fileList;
	// ...
	POSITION pos = fileList.GetHeadPosition();
	while (pos)
	{
		CString strFile = fileList.GetNext(pos);
		// Process file
	}
}
```

### After
```cpp
CString GetFileName()
{
	auto strFileName = _T("file.txt");
	return strFileName;
}

void ProcessFiles()
{
	CAtlList<CString> fileList;
	// ...
	for (auto pos = fileList.GetHeadPosition(); pos != nullptr;)
	{
		auto strFile = fileList.GetNext(pos);
		// Process file
	}
}
```

## Range-Based For Loops

### Before
```cpp
void ProcessVector()
{
	std::vector<CFile*> files;
	// ...
	for (size_t i = 0; i < files.size(); i++)
	{
		CFile* pFile = files[i];
		pFile->Process();
	}
}
```

### After
```cpp
void ProcessVector()
{
	std::vector<CFile*> files;
	// ...
	for (auto pFile : files)
	{
		pFile->Process();
	}

	// Or with const reference for efficiency
	std::vector<CString> names;
	for (const auto& name : names)
	{
		// Use name without copying
	}
}
```

## nullptr

### Before
```cpp
CConnection* m_pConnection;

void Initialize()
{
	m_pConnection = NULL;
}

bool IsValid()
{
	return m_pConnection != NULL;
}
```

### After
```cpp
CConnection* m_pConnection;

void Initialize()
{
	m_pConnection = nullptr;
}

bool IsValid()
{
	return m_pConnection != nullptr;
}
```

## constexpr

### Before
```cpp
#define MAX_BUFFER_SIZE 8192
#define KILOBYTE 1024

class CBuffer
{
	static const int BufferSize = 8192;
};
```

### After
```cpp
constexpr size_t MAX_BUFFER_SIZE = 8192;
constexpr size_t KILOBYTE = 1024;
constexpr size_t MEGABYTE = KILOBYTE * 1024; // Compile-time calculation

class CBuffer
{
	static constexpr size_t BufferSize = 8192;
};

// Constexpr functions
constexpr size_t BytesToKB(size_t bytes)
{
	return bytes / KILOBYTE;
}
```

## String View

### Before
```cpp
bool StartsWith(const CString& str, const CString& prefix)
{
	if (str.GetLength() < prefix.GetLength())
		return false;
	return str.Left(prefix.GetLength()) == prefix;
}
```

### After
```cpp
bool StartsWith(std::wstring_view str, std::wstring_view prefix)
{
	if (str.length() < prefix.length())
		return false;
	return str.substr(0, prefix.length()) == prefix;
}

// Usage - no string copies
CString strFileName = _T("file.txt");
if (StartsWith(strFileName.GetString(), L"file"))
{
	// Process
}
```

## Optional Values

### Before
```cpp
CString FindUserName(DWORD nUserID)
{
	CString strName;
	if (LookupUser(nUserID, strName))
		return strName;
	return CString(); // Empty string = not found
}

// Caller has to check if empty
CString strName = FindUserName(123);
if (!strName.IsEmpty())
{
	// Use name
}
```

### After
```cpp
std::optional<CString> FindUserName(DWORD nUserID)
{
	CString strName;
	if (LookupUser(nUserID, strName))
		return strName;
	return std::nullopt;
}

// Caller clearly knows it's optional
if (auto userName = FindUserName(123))
{
	// Use userName.value()
	CString name = userName.value();
}
```

## Structured Bindings

### Before
```cpp
std::pair<bool, int> GetStatus()
{
	return std::make_pair(true, 42);
}

void CheckStatus()
{
	std::pair<bool, int> result = GetStatus();
	if (result.first)
	{
		int value = result.second;
		// Use value
	}
}
```

### After
```cpp
std::pair<bool, int> GetStatus()
{
	return {true, 42};
}

void CheckStatus()
{
	auto [success, value] = GetStatus();
	if (success)
	{
		// Use value directly
	}
}

// Works with maps too
std::map<CString, int> counts;
for (const auto& [key, value] : counts)
{
	// Use key and value directly
}
```

## Concepts (C++20)

### Before
```cpp
template<typename T>
bool CompareValues(T a, T b)
{
	return a < b; // Hope T has operator<
}
```

### After
```cpp
#include <concepts>

template<typename T>
requires std::totally_ordered<T>
bool CompareValues(T a, T b)
{
	return a < b;
}

// Or shorter syntax
template<std::totally_ordered T>
bool CompareValues(T a, T b)
{
	return a < b;
}
```

## Ranges (C++20)

### Before
```cpp
std::vector<int> GetSquares(const std::vector<int>& numbers)
{
	std::vector<int> result;
	for (int n : numbers)
	{
		if (n > 0)
		{
			result.push_back(n * n);
		}
	}
	return result;
}
```

### After
```cpp
#include <ranges>

auto GetSquares(const std::vector<int>& numbers)
{
	return numbers
		| std::views::filter([](int n) { return n > 0; })
		| std::views::transform([](int n) { return n * n; });
}

// Usage - lazy evaluation
auto squares = GetSquares({-1, 2, -3, 4, 5});
for (int sq : squares)
{
	// Process
}
```

## Lambda Expressions

### Before
```cpp
struct CompareBySize
{
	bool operator()(CFile* a, CFile* b) const
	{
		return a->GetSize() < b->GetSize();
	}
};

void SortFiles(std::vector<CFile*>& files)
{
	std::sort(files.begin(), files.end(), CompareBySize());
}
```

### After
```cpp
void SortFiles(std::vector<CFile*>& files)
{
	std::sort(files.begin(), files.end(),
		[](CFile* a, CFile* b) { return a->GetSize() < b->GetSize(); });
}

// With capture
void ProcessDownloads(int minSpeed)
{
	std::vector<CDownload*> downloads;
	// ...

	// Capture minSpeed by value
	auto fastEnough = [minSpeed](CDownload* d) {
		return d->GetSpeed() >= minSpeed;
	};

	auto it = std::find_if(downloads.begin(), downloads.end(), fastEnough);
}
```

## Override Keyword

### Before
```cpp
class CDownload : public CTransfer
{
public:
	virtual void OnComplete() // Is this really overriding?
	{
		// Implementation
	}
};
```

### After
```cpp
class CDownload : public CTransfer
{
public:
	void OnComplete() override // Compiler verifies this overrides
	{
		// Implementation
	}

	// Mark as final if shouldn't be overridden further
	void OnError() override final
	{
		// Implementation
	}
};
```

## Initialization

### Before
```cpp
class CConnection
{
private:
	CString m_strHost;
	WORD m_nPort;
	bool m_bConnected;

public:
	CConnection()
	{
		m_strHost = _T("");
		m_nPort = 0;
		m_bConnected = false;
	}
};
```

### After
```cpp
class CConnection
{
private:
	CString m_strHost;
	WORD m_nPort = 0;
	bool m_bConnected = false;

public:
	CConnection() = default; // Use member initializers

	// Or with constructor
	CConnection(const CString& host, WORD port)
		: m_strHost(host)
		, m_nPort(port)
		, m_bConnected(false)
	{
	}
};
```

## Default and Delete

### Before
```cpp
class CNonCopyable
{
private:
	CNonCopyable(const CNonCopyable&) {}
	CNonCopyable& operator=(const CNonCopyable&) { return *this; }
};
```

### After
```cpp
class CNonCopyable
{
public:
	CNonCopyable() = default;
	CNonCopyable(const CNonCopyable&) = delete;
	CNonCopyable& operator=(const CNonCopyable&) = delete;
	CNonCopyable(CNonCopyable&&) = default; // Allow move
	CNonCopyable& operator=(CNonCopyable&&) = default;
};
```

## Migration Strategy

1. **New Code**: Use modern C++ features from the start
2. **Refactoring**: Gradually update old code during maintenance
3. **Hot Paths**: Prioritize performance-critical sections
4. **Safety**: Focus on memory management improvements first
5. **Testing**: Thoroughly test after modernization

## Important Notes

- **MFC Compatibility**: Some MFC patterns don't mix well with modern C++ (e.g., CObject-derived classes)
- **Legacy Code**: Don't refactor working code without good reason
- **Performance**: Profile before and after changes
- **Compilation**: Ensure changes work on all build configurations
- **Team Agreement**: Discuss major style changes with team

## Resources

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Modern C++ Features](https://github.com/AnthonyCalandra/modern-cpp-features)
- [C++20 Standard](https://en.cppreference.com/w/cpp/20)
