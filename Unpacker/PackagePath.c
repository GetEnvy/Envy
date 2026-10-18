// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Unpacker.h"
#include <strsafe.h>
#include <winternl.h>

int PackageRelativePath(LPCTSTR path, int directoryEntry)
{
	const WCHAR* start = path;
	const WCHAR* p;
	size_t length;
	if (!path) return 0;
	length = wcslen(path);
	if (!length || length >= MAX_PATH) return 0;
	for (p = path;; ++p)
	{
		WCHAR c = *p;
		if (!c || c == L'/' || c == L'\\')
		{
			size_t count = p - start, base = 0;
			if (!count || count > 255 || start[count - 1] == L'.' || start[count - 1] == L' ') return 0;
			while (base < count && start[base] != L'.') ++base;
			while (base && start[base - 1] == L' ') --base;
			if ((base == 3 && (!_wcsnicmp(start, L"CON", 3) || !_wcsnicmp(start, L"PRN", 3) ||
			                   !_wcsnicmp(start, L"AUX", 3) || !_wcsnicmp(start, L"NUL", 3))) ||
			    (base == 6 && !_wcsnicmp(start, L"CONIN$", 6)) ||
			    (base == 7 && !_wcsnicmp(start, L"CONOUT$", 7))) return 0;
			if (base == 4 && (!_wcsnicmp(start, L"COM", 3) || !_wcsnicmp(start, L"LPT", 3)) &&
			    ((start[3] >= L'0' && start[3] <= L'9') || start[3] == 0xb9 || start[3] == 0xb2 || start[3] == 0xb3)) return 0;
			if (!c) return 1;
			if (!p[1]) return directoryEntry;
			start = p + 1;
		}
		else if (c < 32 || wcschr(L"<>:\"|?*", c)) return 0;
		else if (c >= 0xd800 && c <= 0xdbff)
		{
			if (p[1] < 0xdc00 || p[1] > 0xdfff) return 0;
			++p;
		}
		else if (c >= 0xdc00 && c <= 0xdfff) return 0;
	}
}

int PackageJoin(LPTSTR result, LPCTSTR root, LPCTSTR relative)
{
	size_t length;
	if (!root || !PackageRelativePath(relative, 0)) return 0;
	length = wcslen(root);
	if (!length || length >= MAX_PATH) return 0;
	return SUCCEEDED(StringCchPrintfW(result, MAX_PATH, L"%s%s%s", root,
	                                  root[length - 1] == L'\\' ? L"" : L"\\", relative));
}

void PackageUnlock(PACKAGE_DIRECTORY* directory)
{
	while (directory->count) CloseHandle(directory->handles[--directory->count]);
}

// Pin every ancestor without write/delete sharing before using its child name.
// An attribute check alone would leave a junction replacement race.
int PackageLockDirectory(LPCTSTR path, PACKAGE_DIRECTORY* directory, int create)
{
	WCHAR full[MAX_PATH], part[MAX_PATH];
	size_t start, i, length;
	DWORD n;
	directory->count = 0;
	if (!path || wcslen(path) >= MAX_PATH) return 0;
	n = GetFullPathNameW(path, MAX_PATH, full, NULL);
	if (!n || n >= MAX_PATH || _wcsicmp(path, full)) return 0;
	length = wcslen(full);
	if (length >= 3 && full[1] == L':' && full[2] == L'\\') start = 3;
	else if (length > 2 && full[0] == L'\\' && full[1] == L'\\' && full[2] != L'?' && full[2] != L'.')
	{
		WCHAR* share = wcschr(full + 2, L'\\');
		WCHAR* end = share ? wcschr(share + 1, L'\\') : NULL;
		if (!share || share == full + 2 || !share[1]) return 0;
		start = end ? (size_t)(end - full + 1) : length;
	}
	else return 0;
	if (start < length && !PackageRelativePath(full + start, 0)) return 0;
	for (i = start; i <= length; ++i)
	{
		HANDLE handle;
		BY_HANDLE_FILE_INFORMATION info;
		if (i != start && i != length && full[i] != L'\\') continue;
		memcpy(part, full, i * sizeof(WCHAR));
		part[i] = 0;
		handle = CreateFileW(part, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		                     FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		if (handle == INVALID_HANDLE_VALUE && create && i > start && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			if (!CreateDirectoryW(part, NULL)) goto fail;
			handle = CreateFileW(part, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			                     FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		}
		if (handle == INVALID_HANDLE_VALUE) goto fail;
		if (!GetFileInformationByHandle(handle, &info) || !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
		    (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			CloseHandle(handle);
			goto fail;
		}
		if (directory->count >= _countof(directory->handles))
		{
			CloseHandle(handle);
			goto fail;
		}
		directory->handles[directory->count++] = handle;
	}
	return 1;
fail:
	PackageUnlock(directory);
	return 0;
}

int PackageCanReplace(LPCTSTR path)
{
	BY_HANDLE_FILE_INFORMATION info;
	HANDLE handle = CreateFileW(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING,
	                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	int safe;
	if (handle == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND;
	safe = GetFileInformationByHandle(handle, &info) && info.nNumberOfLinks == 1 &&
	       !(info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_READONLY));
	CloseHandle(handle);
	return safe;
}

// Native FileRenameInformation (10) with a leaf and NULL root renames in the
// source directory. The Win32 wrapper resolves relative names against the
// process directory and reopens parents, conflicting with the write-denying
// directory pins. The kernel requires at least sizeof(FILE_RENAME_INFO) even for
// one-character names; FileNameLength still bounds the name that is read.
int PackageRenameLeaf(HANDLE file, LPCTSTR leaf)
{
	typedef NTSTATUS (NTAPI *RENAME_FILE)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
	RENAME_FILE renameFile = (RENAME_FILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtSetInformationFile");
	struct { FILE_RENAME_INFO info; WCHAR room[MAX_PATH]; } renameInfo;
	IO_STATUS_BLOCK status;
	size_t bytes, length;
	if (!renameFile || !PackageRelativePath(leaf, 0) || wcspbrk(leaf, L"/\\")) return 0;
	length = wcslen(leaf) * sizeof(WCHAR);
	memset(&renameInfo, 0, sizeof(renameInfo));
	renameInfo.info.ReplaceIfExists = TRUE;
	renameInfo.info.FileNameLength = (DWORD)length;
	memcpy(renameInfo.info.FileName, leaf, length);
	bytes = offsetof(FILE_RENAME_INFO, FileName) + length;
	if (bytes < sizeof(FILE_RENAME_INFO)) bytes = sizeof(FILE_RENAME_INFO);
	return renameFile(file, &status, &renameInfo, (ULONG)bytes, (FILE_INFORMATION_CLASS)10) >= 0;
}
