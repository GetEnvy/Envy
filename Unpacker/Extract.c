//
// Extract.c
//
// This file is part of Envy (getenvy.com) © 2016-2018
//
// Portions of this page have been previously released into the public domain.
// You are free to redistribute and modify it without any restrictions
// with the exception of the following notice:
//
// The Zlib  library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
// The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.

#include "Unpacker.h"
#include <share.h>
#include <winternl.h>

#define PACKAGE_MANIFEST_LIMIT (4UL * 1024 * 1024)
#define PACKAGE_ENTRY_LIMIT 65534UL

// The archive remains open with write sharing denied throughout validation and use.
static voidpf ZCALLBACK OpenArchive(voidpf opaque, const char* name, int mode)
{
	(void)name;
	(void)mode;
	return _wfsopen((LPCTSTR)opaque, L"rb", _SH_DENYWR);
}
static uLong ZCALLBACK ReadArchive(voidpf opaque, voidpf stream, void* buffer, uLong size)
{
	(void)opaque;
	return (uLong)fread(buffer, 1, size, (FILE*)stream);
}
static uLong ZCALLBACK WriteArchive(voidpf opaque, voidpf stream, const void* buffer, uLong size)
{
	(void)opaque; (void)stream; (void)buffer; (void)size;
	return 0;
}
static long ZCALLBACK TellArchive(voidpf opaque, voidpf stream)
{
	(void)opaque;
	return ftell((FILE*)stream);
}
static long ZCALLBACK SeekArchive(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	(void)opaque;
	if (offset > LONG_MAX) return -1;
	return fseek((FILE*)stream, (long)offset, origin);
}
static int ZCALLBACK CloseArchive(voidpf opaque, voidpf stream)
{
	(void)opaque;
	return fclose((FILE*)stream);
}
static int ZCALLBACK ErrorArchive(voidpf opaque, voidpf stream)
{
	(void)opaque;
	return ferror((FILE*)stream);
}
static unzFile OpenPackage(LPCTSTR path)
{
	zlib_filefunc_def callbacks = { OpenArchive, ReadArchive, WriteArchive, TellArchive,
		SeekArchive, CloseArchive, ErrorArchive, (voidpf)path };
	if (!path || !*path || wcslen(path) >= MAX_PATH) return NULL;
	return unzOpen2("package", &callbacks);
}

void ExtractSkinFile(LPCTSTR szFile)
{
	TCHAR* allocation = _wcsdup(szFile);
	TCHAR* file = allocation;
	TCHAR* end;
	if (!file) return;
	if (*file == L'"') ++file;
	end = wcschr(file, L'"');
	if (end) *end = 0;
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, ExtractProc, (LPARAM)file);
	free(allocation);
}

static int ReadInstallPath(LPCTSTR value, LPTSTR path)
{
	DWORD bytes = MAX_PATH * sizeof(WCHAR), type = 0;
	if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Envy\\Envy", value, &type, path, &bytes) != ERROR_SUCCESS)
		return 0;
	return type == REG_SZ && bytes >= sizeof(WCHAR) && bytes <= MAX_PATH * sizeof(WCHAR) &&
		bytes % sizeof(WCHAR) == 0 && path[bytes / sizeof(WCHAR) - 1] == 0 && *path &&
		wcslen(path) + 1 == bytes / sizeof(WCHAR);
}

int GetInstallDirectory()
{
	WCHAR base[MAX_PATH], full[MAX_PATH];
	DWORD length;
	int custom = skinType == typeData && ReadInstallPath(L"DataPath", base);
	skins_dir[0] = 0;
	if (!custom && !ReadInstallPath(L"Path", base))
	{
		WCHAR* end;
		length = GetModuleFileNameW(NULL, base, MAX_PATH);
		if (!length || length >= MAX_PATH) return 0;
		end = wcsrchr(base, L'\\');
		if (!end) return 0;
		*end = 0;
	}
	length = GetFullPathNameW(base, MAX_PATH, full, NULL);
	if (!length || length >= MAX_PATH || PathIsRelativeW(base)) return 0;
	while (length > 3 && full[length - 1] == L'\\') full[--length] = 0;
	if (custom) return SUCCEEDED(StringCchCopyW(skins_dir, MAX_PATH, full));
	return PackageJoin(skins_dir, full, skinType == typeSchemas ? L"Schemas" :
		skinType == typePlugin ? L"Plugins" : skinType == typeData ? L"Data" : L"Skins");
}

int GetSkinFileCount(LPTSTR pszFile)
{
	unz_global_info info;
	unzFile archive = OpenPackage(pszFile);
	int count = 0;
	if (!archive) return 0;
	if (unzGetGlobalInfo(archive, &info) == UNZ_OK && info.number_entry <= PACKAGE_ENTRY_LIMIT)
		count = (int)info.number_entry;
	unzClose(archive);
	return count;
}

LPCTSTR GetUnicodeString(char* text)
{
	int count;
	WCHAR* result;
	if (!text) return NULL;
	count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
	if (!count || (size_t)count > (size_t)-1 / sizeof(WCHAR)) return NULL;
	result = (WCHAR*)malloc((size_t)count * sizeof(WCHAR));
	if (!result) return NULL;
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, result, count) != count)
	{
		free(result);
		return NULL;
	}
	return result;
}

static int EntryInfo(unzFile archive, unz_file_info* info, WCHAR name[MAX_PATH], int* directory)
{
	char raw[MAX_PATH * 4];
	WCHAR* wide;
	size_t length;
	unsigned mode;
	if (unzGetCurrentFileInfo(archive, info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK ||
		!info->size_filename || info->size_filename >= sizeof(raw) || info->disk_num_start ||
		(info->flag & ~0x080eUL) || (info->compression_method != 0 && info->compression_method != Z_DEFLATED) ||
		info->compressed_size == MAXDWORD || info->uncompressed_size == MAXDWORD) return 0;
	memset(raw, 0, sizeof(raw));
	if (unzGetCurrentFileInfo(archive, info, raw, sizeof(raw), NULL, 0, NULL, 0) != UNZ_OK ||
		memchr(raw, 0, info->size_filename) || raw[info->size_filename] != 0) return 0;
	wide = (WCHAR*)GetUnicodeString(raw);
	if (!wide) return 0;
	length = wcslen(wide);
	*directory = length && (wide[length - 1] == L'/' || wide[length - 1] == L'\\');
	mode = (unsigned)(info->external_fa >> 16) & 0170000;
	if (!PackageRelativePath(wide, *directory) ||
		((info->version >> 8) == 3 && mode && mode != (*directory ? 0040000U : 0100000U)) ||
		(info->external_fa & FILE_ATTRIBUTE_REPARSE_POINT) ||
		((info->external_fa & FILE_ATTRIBUTE_DIRECTORY) && !*directory) ||
		(*directory && info->uncompressed_size))
	{
		free(wide);
		return 0;
	}
	StringCchCopyW(name, MAX_PATH, wide);
	free(wide);
	return 1;
}

static WCHAR* EntryBase(WCHAR* name)
{
	WCHAR* result = name;
	for (; *name; ++name) if (*name == L'/' || *name == L'\\') result = name + 1;
	return result;
}

// Read the declared size exactly, then check EOF and CRC, including empty entries.
static int ReadEntry(unzFile archive, const unz_file_info* info, HANDLE output, char* manifest)
{
	char buffer[16384];
	uLong total = 0;
	int count, valid = 1;
	if (unzOpenCurrentFile(archive) != UNZ_OK) return 0;
	while ((count = unzReadCurrentFile(archive, buffer, sizeof(buffer))) > 0)
	{
		DWORD written;
		if (total > info->uncompressed_size ||
			(uLong)count > info->uncompressed_size - total) { valid = 0; break; }
		if (manifest) memcpy(manifest + total, buffer, count);
		if (output != INVALID_HANDLE_VALUE &&
			(!WriteFile(output, buffer, count, &written, NULL) || written != (DWORD)count)) { valid = 0; break; }
		total += count;
	}
	if (count < 0 || total != info->uncompressed_size) valid = 0;
	if (unzCloseCurrentFile(archive) != UNZ_OK) valid = 0;
	return valid;
}

static int ValidatePackage(unzFile archive)
{
	unz_global_info global;
	uLong i;
	int manifestSeen = 0;
	if (unzGetGlobalInfo(archive, &global) != UNZ_OK || !global.number_entry ||
		global.number_entry > PACKAGE_ENTRY_LIMIT || unzGoToFirstFile(archive) != UNZ_OK) return 0;
	for (i = 0; i < global.number_entry; ++i)
	{
		unz_file_info info;
		WCHAR name[MAX_PATH];
		int directory, valid;
		char* data = NULL;
		if (!EntryInfo(archive, &info, name, &directory)) return 0;
		if (!directory && !manifestSeen && !_wcsicmp(PathFindExtensionW(name), L".xml"))
		{
			if (!info.uncompressed_size || info.uncompressed_size > PACKAGE_MANIFEST_LIMIT) return 0;
			data = (char*)malloc((size_t)info.uncompressed_size + 1);
			if (!data) return 0;
		}
		valid = ReadEntry(archive, &info, INVALID_HANDLE_VALUE, data);
		if (valid && data)
		{
			char* manifest = NULL;
			WCHAR* decoded;
			data[info.uncompressed_size] = 0;
			decoded = (WCHAR*)GetUnicodeString(data);
			if (decoded && !memchr(data, 0, info.uncompressed_size)) manifest = strstr(data, "<manifest");
			valid = manifest != NULL;
			free(decoded);
			if (valid) valid = LoadManifestInfo(manifest);
			if (valid)
			{
				WCHAR* xml = _wcsdup(EntryBase(name));
				if (!xml) valid = 0;
				else { free(szXML); szXML = xml; manifestSeen = 1; }
			}
		}
		free(data);
		if (!valid) return 0;
		if (i + 1 < global.number_entry && unzGoToNextFile(archive) != UNZ_OK) return 0;
	}
	return manifestSeen && unzGoToNextFile(archive) == UNZ_END_OF_LIST_OF_FILE;
}

int ValidateSkin(LPTSTR pszFile, HWND hwndDlg)
{
	unzFile archive = OpenPackage(pszFile);
	int valid;
	(void)hwndDlg;
	if (!archive) return 0;
	valid = ValidatePackage(archive);
	unzClose(archive);
	return valid;
}

static int InstallEntry(unzFile archive, const unz_file_info* info, LPCTSTR directory, LPCTSTR destination)
{
	WCHAR temporary[MAX_PATH];
	HANDLE file = INVALID_HANDLE_VALUE;
	unsigned attempt;
	int valid = 0;
	FILE_DISPOSITION_INFO disposition = { TRUE };
	if (!PackageCanReplace(destination)) return 0;
	for (attempt = 0; attempt < 100; ++attempt)
	{
		WCHAR leaf[64];
		StringCchPrintfW(leaf, 64, L".envy-%lu-%u.tmp", GetCurrentProcessId(), attempt);
		if (!PackageJoin(temporary, directory, leaf)) return 0;
		if (!_wcsicmp(temporary, destination)) continue;
		file = CreateFileW(temporary, GENERIC_WRITE | DELETE, 0, NULL, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		if (file != INVALID_HANDLE_VALUE) break;
		if (GetLastError() != ERROR_FILE_EXISTS && GetLastError() != ERROR_ALREADY_EXISTS) return 0;
	}
	if (file == INVALID_HANDLE_VALUE) return 0;
	if (ReadEntry(archive, info, file, NULL) && FlushFileBuffers(file) && PackageCanReplace(destination))
	{
		valid = PackageRenameLeaf(file, PathFindFileNameW(destination));
	}
	if (!valid) SetFileInformationByHandle(file, FileDispositionInfo, &disposition, sizeof(disposition));
	CloseHandle(file);
	return valid;
}

int ExtractSkin(LPTSTR pszFile, HWND hwndDlg)
{
	unzFile archive = OpenPackage(pszFile);
	unz_global_info global;
	WCHAR prefix[MAX_PATH], destination[MAX_PATH];
	PACKAGE_DIRECTORY pinned;
	uLong i;
	int valid = 0, previewType = skinType;
	pinned.count = 0;
	if (!archive) return 0;
	// Preview is not authorization: re-read the manifest from this locked archive.
	if (!ValidatePackage(archive) || skinType != previewType || !*skins_dir) goto done;
	if (skinType == typeSkin || skinType == typeLang)
	{
		if (!PackageJoin(prefix, skins_dir, skinType == typeLang ? L"Languages" : szPath)) goto done;
	}
	else if (FAILED(StringCchCopyW(prefix, MAX_PATH, skins_dir))) goto done;
	for (i = 0; prefix[i]; ++i) if (prefix[i] == L'/') prefix[i] = L'\\';
	if (!PackageLockDirectory(prefix, &pinned, 1)) goto done;
	if (unzGetGlobalInfo(archive, &global) != UNZ_OK || unzGoToFirstFile(archive) != UNZ_OK) goto done;
	for (i = 0; i < global.number_entry; ++i)
	{
		unz_file_info info;
		WCHAR name[MAX_PATH];
		int directory;
		if (!EntryInfo(archive, &info, name, &directory)) goto done;
		if (!directory)
		{
			if (!PackageJoin(destination, prefix, EntryBase(name)) ||
				!InstallEntry(archive, &info, prefix, destination)) goto done;
		}
		if (hwndDlg) SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_STEPIT, 0, 0);
		if (i + 1 < global.number_entry && unzGoToNextFile(archive) != UNZ_OK) goto done;
	}
	valid = 1;
done:
	PackageUnlock(&pinned);
	unzClose(archive);
	return valid;
}
