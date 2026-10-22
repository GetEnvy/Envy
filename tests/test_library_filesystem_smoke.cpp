// Regression coverage for Windows path preparation used by library scanning.

#include "test_framework.h"
#include "LibraryFileSystem.h"

static bool test_local_long_path()
{
	const std::wstring input = L"C:\\TestShare\\Media\\" + std::wstring(240, L'x');
	const std::wstring output = EnvyFileSystem::ApiPath(input);
	return output.compare(0, 4, L"\\\\?\\") == 0 && output.substr(4) == input;
}

static bool test_unc_long_path()
{
	const std::wstring input = L"\\\\test-server\\test-share\\Media\\" + std::wstring(240, L'x');
	const std::wstring output = EnvyFileSystem::ApiPath(input);
	return output.compare(0, 8, L"\\\\?\\UNC\\") == 0 && output.substr(8) == input.substr(2);
}

static bool test_mapped_drive_long_path()
{
	const std::wstring input = L"X:\\TestShare\\Media\\" + std::wstring(240, L'x');
	const std::wstring output = EnvyFileSystem::ApiPath(input);
	return output.compare(0, 4, L"\\\\?\\") == 0 && output.substr(4) == input;
}

static bool test_existing_extended_path()
{
	const std::wstring input = L"\\\\?\\UNC\\test-server\\test-share\\Media";
	return EnvyFileSystem::ApiPath(input) == input;
}

static bool test_enumeration_errors()
{
	return EnvyFileSystem::EnumerationComplete(ERROR_NO_MORE_FILES) &&
	       EnvyFileSystem::EnumerationComplete(ERROR_FILE_NOT_FOUND) &&
	       !EnvyFileSystem::EnumerationComplete(ERROR_ACCESS_DENIED) &&
	       !EnvyFileSystem::EnumerationComplete(ERROR_OPERATION_ABORTED);
}

static bool test_reparse_point_directories_are_not_traversed()
{
	return EnvyFileSystem::IsDirectoryReparsePoint(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT) &&
	       !EnvyFileSystem::IsDirectoryReparsePoint(FILE_ATTRIBUTE_DIRECTORY) &&
	       !EnvyFileSystem::IsDirectoryReparsePoint(FILE_ATTRIBUTE_NORMAL) &&
	       !EnvyFileSystem::IsDirectoryReparsePoint(FILE_ATTRIBUTE_REPARSE_POINT);
}

// Enumerate a real directory through ApiPath the way CLibraryFolder::ThreadScan
// does and require a complete listing, then a failed one for a missing path.
static bool test_real_enumeration_completeness()
{
	wchar_t temp[MAX_PATH];
	const DWORD nTempLen = GetTempPathW(MAX_PATH, temp);
	if (nTempLen == 0 || nTempLen >= MAX_PATH)
		return false;

	// Own a fresh directory: never reuse or delete anything that already exists.
	std::wstring root;
	for (int attempt = 0; attempt < 100; ++attempt)
	{
		root = std::wstring(temp) + L"EnvyScanTest" + std::to_wstring(GetCurrentProcessId()) + L"_" +
		       std::to_wstring(GetTickCount64()) + L"_" + std::to_wstring(attempt);
		if (CreateDirectoryW(root.c_str(), NULL))
			break;
		root.clear();
	}
	if (root.empty())
		return false;
	const std::wstring sub = root + L"\\nested";
	if (!CreateDirectoryW(sub.c_str(), NULL))
	{
		RemoveDirectoryW(root.c_str());
		return false;
	}
	const std::wstring file = sub + L"\\item.bin";
	HANDLE hFile = CreateFileW(file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		RemoveDirectoryW(sub.c_str());
		RemoveDirectoryW(root.c_str());
		return false;
	}
	CloseHandle(hFile);

	int files = 0;
	DWORD error = ERROR_SUCCESS;
	WIN32_FIND_DATAW find;
	HANDLE hSearch = FindFirstFileW(EnvyFileSystem::ApiPath(sub + L"\\*.*").c_str(), &find);
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				++files;
		} while (FindNextFileW(hSearch, &find));
		error = GetLastError();
		FindClose(hSearch);
	}

	WIN32_FIND_DATAW missing;
	HANDLE hMissing = FindFirstFileW(EnvyFileSystem::ApiPath(root + L"\\absent\\*.*").c_str(), &missing);
	const bool missingFails = hMissing == INVALID_HANDLE_VALUE && !EnvyFileSystem::EnumerationComplete(GetLastError());
	if (hMissing != INVALID_HANDLE_VALUE)
		FindClose(hMissing);

	DeleteFileW(file.c_str());
	RemoveDirectoryW(sub.c_str());
	RemoveDirectoryW(root.c_str());

	return files == 1 && EnvyFileSystem::EnumerationComplete(error) && missingFails;
}

// Equal-length spellings that differ only by an ignorable soft hyphen are
// different NTFS names, so they must not be treated as the same folder.
static bool test_path_relation_is_ordinal()
{
	const std::wstring a = L"C:\\Share\\ab\u00AD";
	const std::wstring b = L"C:\\Share\\a\u00ADb";
	return EnvyFileSystem::RelatePaths(a, b) == EnvyFileSystem::PathRelation::Unrelated &&
	       EnvyFileSystem::RelatePaths(L"C:\\Share\\Music", L"c:\\share\\music") == EnvyFileSystem::PathRelation::Same &&
	       EnvyFileSystem::RelatePaths(L"C:\\Share", L"C:\\Share\\Music") == EnvyFileSystem::PathRelation::Ancestor &&
	       EnvyFileSystem::RelatePaths(L"C:\\Share\\Music", L"C:\\Share") == EnvyFileSystem::PathRelation::Descendant &&
	       EnvyFileSystem::RelatePaths(L"C:\\Share", L"C:\\ShareX") == EnvyFileSystem::PathRelation::Unrelated;
}

// A path shorter than the prefix must be rejected before any comparison reads past its end.
static bool test_path_prefix_rejects_shorter_path()
{
	return !EnvyFileSystem::PathPrefixEquals(L"C:", L"C:\\Share") &&
	       EnvyFileSystem::PathPrefixEquals(L"C:\\Share\\Music", L"c:\\share");
}

void register_library_filesystem_smoke_tests(TestSuite& suite)
{
	suite.add_test("library_filesystem_local_long_path", test_local_long_path);
	suite.add_test("library_filesystem_unc_long_path", test_unc_long_path);
	suite.add_test("library_filesystem_mapped_drive_long_path", test_mapped_drive_long_path);
	suite.add_test("library_filesystem_existing_extended_path", test_existing_extended_path);
	suite.add_test("library_filesystem_enumeration_errors", test_enumeration_errors);
	suite.add_test("library_filesystem_reparse_point_directories", test_reparse_point_directories_are_not_traversed);
	suite.add_test("library_filesystem_real_enumeration", test_real_enumeration_completeness);
	suite.add_test("library_filesystem_path_relation_is_ordinal", test_path_relation_is_ordinal);
	suite.add_test("library_filesystem_path_prefix_rejects_shorter_path", test_path_prefix_rejects_shorter_path);
}
