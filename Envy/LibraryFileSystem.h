// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace EnvyFileSystem
{
inline std::wstring ApiPath(const std::wstring& path)
{
	if (path.size() < MAX_PATH - 4 || path.compare(0, 4, L"\\\\?\\") == 0)
		return path;

	if (path.compare(0, 2, L"\\\\") == 0)
		return L"\\\\?\\UNC" + path.substr(1);

	return L"\\\\?\\" + path;
}

// A finished listing ends with ERROR_NO_MORE_FILES. ERROR_FILE_NOT_FOUND is
// what FindFirstFile reports for a path with no matching entry, so it is the
// only other result that may justify pruning. Every other code (access denied,
// network loss, abort) leaves the listing incomplete and entries untouched.
inline bool EnumerationComplete(DWORD error)
{
	return error == ERROR_NO_MORE_FILES || error == ERROR_FILE_NOT_FOUND;
}

// Directory junctions and symlinks can leave the selected share or loop back
// into it, so scans never descend into directory reparse points.
inline bool IsDirectoryReparsePoint(DWORD attributes)
{
	const DWORD mask = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
	return (attributes & mask) == mask;
}

enum class PathRelation
{
	Unrelated,
	Same,
	Ancestor,  // The candidate contains the other path
	Descendant // The candidate lies inside the other path
};

// Case-insensitive comparison of path with prefix over the length of prefix.
inline bool PathPrefixEquals(const std::wstring& path, const std::wstring& prefix)
{
	const int length = static_cast<int>(prefix.size());
	if (path.size() < prefix.size())
		return false; // The comparison reads length characters from both strings
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
	// Ordinal, like NTFS: linguistic collation could equate distinct spellings.
	return CompareStringOrdinal(path.c_str(), length, prefix.c_str(), length, TRUE) == CSTR_EQUAL;
#else
	// Only legacy plugin units that compile Strings.cpp for _WIN32_WINNT 0x0501
	// reach this branch, where CompareStringOrdinal is not declared. They use
	// ApiPath alone and never call this function.
	return CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, path.c_str(), length,
	                      prefix.c_str(), length) == CSTR_EQUAL;
#endif
}

// Case-insensitive relation between two folder paths without trailing
// backslashes ("C:" for a drive root), as the library stores them.
inline PathRelation RelatePaths(const std::wstring& candidate, const std::wstring& other)
{
	if (candidate.empty() || other.empty())
		return PathRelation::Unrelated;
	if (candidate.size() == other.size())
		return PathPrefixEquals(candidate, other) ? PathRelation::Same : PathRelation::Unrelated;
	if (candidate.size() < other.size())
	{
		if (other[candidate.size()] == L'\\' && PathPrefixEquals(other, candidate))
			return PathRelation::Ancestor;
		return PathRelation::Unrelated;
	}
	if (candidate[other.size()] == L'\\' && PathPrefixEquals(candidate, other))
		return PathRelation::Descendant;
	return PathRelation::Unrelated;
}

struct DirectoryEntry
{
	std::wstring name;
	DWORD attributes;
	ULONGLONG size;
	FILETIME lastWrite;
};

// Lists one directory without touching library state, so callers can do the
// slow part (disk or network round trips) without holding the library lock.
// Returns the error that ended the listing: EnumerationComplete() tells
// whether it may justify pruning; ERROR_CANCELLED means isCancelled()
// stopped it.
template<class IsCancelled>
DWORD ListDirectory(const std::wstring& pattern, std::vector<DirectoryEntry>& entries, IsCancelled isCancelled)
{
	WIN32_FIND_DATAW find;
	const HANDLE hSearch = FindFirstFileW(pattern.c_str(), &find);
	if (hSearch == INVALID_HANDLE_VALUE)
		return GetLastError();

	DWORD error = ERROR_SUCCESS;
	for (;;)
	{
		const bool dots = find.cFileName[0] == L'.' &&
		                  (find.cFileName[1] == 0 || (find.cFileName[1] == L'.' && find.cFileName[2] == 0));
		if (!dots)
		{
			const ULONGLONG size = static_cast<ULONGLONG>(find.nFileSizeLow) |
			                       (static_cast<ULONGLONG>(find.nFileSizeHigh) << 32);
			entries.push_back(DirectoryEntry{ find.cFileName, find.dwFileAttributes, size, find.ftLastWriteTime });
		}
		if (isCancelled())
		{
			error = ERROR_CANCELLED;
			break;
		}
		if (!FindNextFileW(hSearch, &find))
		{
			error = GetLastError();
			break;
		}
	}
	FindClose(hSearch);
	return error;
}
}
