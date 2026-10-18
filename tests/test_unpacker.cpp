// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test_framework.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
extern "C"
{
#include "../Unpacker/Unpacker.h"

	int skinType;
	TCHAR *szName, *szPath, *szVersion, *szAuthor, *szUpdates, *szXML;
	TCHAR skins_dir[MAX_PATH];
	INT_PTR CALLBACK ExtractProc(HWND, UINT, WPARAM, LPARAM) { return 0; }
}

#include <winioctl.h>
#include <winternl.h>

namespace
{
struct Entry
{
	std::string name, data;
	unsigned attributes = 0, flags = 0x800, sizeExtra = 0;
	bool badCrc = false;
};

void Put(std::vector<unsigned char>& bytes, unsigned value, unsigned width)
{
	for (unsigned i = 0; i < width; ++i) bytes.push_back(static_cast<unsigned char>(value >> (8 * i)));
}

std::vector<unsigned char> Zip(const std::vector<Entry>& entries)
{
	std::vector<unsigned char> bytes, central;
	for (const auto& entry : entries)
	{
		unsigned offset = static_cast<unsigned>(bytes.size());
		unsigned size = static_cast<unsigned>(entry.data.size()) + entry.sizeExtra;
		unsigned crc = crc32(0, reinterpret_cast<const Bytef*>(entry.data.data()), static_cast<uInt>(entry.data.size()));
		if (entry.badCrc) ++crc;
		Put(bytes, 0x04034b50, 4);
		Put(bytes, 20, 2);
		Put(bytes, entry.flags, 2);
		Put(bytes, 0, 2);
		Put(bytes, 0, 4);
		Put(bytes, crc, 4);
		Put(bytes, size, 4);
		Put(bytes, size, 4);
		Put(bytes, static_cast<unsigned>(entry.name.size()), 2);
		Put(bytes, 0, 2);
		bytes.insert(bytes.end(), entry.name.begin(), entry.name.end());
		bytes.insert(bytes.end(), entry.data.begin(), entry.data.end());
		Put(central, 0x02014b50, 4);
		Put(central, 0x314, 2);
		Put(central, 20, 2);
		Put(central, entry.flags, 2);
		Put(central, 0, 2);
		Put(central, 0, 4);
		Put(central, crc, 4);
		Put(central, size, 4);
		Put(central, size, 4);
		Put(central, static_cast<unsigned>(entry.name.size()), 2);
		Put(central, 0, 2);
		Put(central, 0, 2);
		Put(central, 0, 2);
		Put(central, 0, 2);
		Put(central, entry.attributes, 4);
		Put(central, offset, 4);
		central.insert(central.end(), entry.name.begin(), entry.name.end());
	}
	unsigned offset = static_cast<unsigned>(bytes.size());
	bytes.insert(bytes.end(), central.begin(), central.end());
	Put(bytes, 0x06054b50, 4);
	Put(bytes, 0, 4);
	Put(bytes, static_cast<unsigned>(entries.size()), 2);
	Put(bytes, static_cast<unsigned>(entries.size()), 2);
	Put(bytes, static_cast<unsigned>(central.size()), 4);
	Put(bytes, offset, 4);
	Put(bytes, 0, 2);
	return bytes;
}

void Reset()
{
	free(szName);
	free(szPath);
	free(szVersion);
	free(szAuthor);
	free(szUpdates);
	free(szXML);
	szName = szPath = szVersion = szAuthor = szUpdates = szXML = nullptr;
	skinType = typeSkin;
}

struct Fixture
{
	std::filesystem::path root, archive, destination;
	Fixture()
	{
		WCHAR temp[MAX_PATH], name[MAX_PATH];
		if (!GetTempPathW(MAX_PATH, temp) || !GetTempFileNameW(temp, L"env", 0, name)) throw std::runtime_error("temporary fixture unavailable");
		DeleteFileW(name);
		if (!CreateDirectoryW(name, nullptr)) throw std::runtime_error("fixture directory unavailable");
		root = name;
		archive = root / L"package.zip";
		destination = root / L"install";
		std::filesystem::create_directory(destination);
		StringCchCopyW(skins_dir, MAX_PATH, destination.c_str());
		Reset();
	}
	~Fixture()
	{
		Reset();
		std::error_code error;
		std::filesystem::remove_all(root, error);
	}
	void save(const std::vector<Entry>& entries)
	{
		auto bytes = Zip(entries);
		std::ofstream file(archive, std::ios::binary);
		file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}
	bool validate() { return ValidateSkin(const_cast<WCHAR*>(archive.c_str()), nullptr) != 0; }
	bool install() { return ExtractSkin(const_cast<WCHAR*>(archive.c_str()), nullptr) != 0; }
};

Entry Manifest(const std::string& extra = "")
{
	return { "skin.xml", "<skin><manifest name=\"Example\" " + extra + "/></skin>" };
}

std::string Contents(const std::filesystem::path& file)
{
	std::ifstream input(file, std::ios::binary);
	return std::string(std::istreambuf_iterator<char>(input), {});
}

bool ValidTypes()
{
	const char* types[] = { "skin", "language", "schemas", "plugin", "data", "security" };
	for (const char* type : types)
	{
		Fixture f;
		f.save({ Manifest(std::string("type=\"") + type + "\""), { "folder/", "", FILE_ATTRIBUTE_DIRECTORY }, { "folder\\payload.txt", "new" }, { "a", "one" } });
		if (!f.validate() || !f.install()) return false;
		auto target = f.destination;
		if (skinType == typeSkin) target /= L"Example";
		if (skinType == typeLang) target /= L"Languages";
		if (Contents(target / L"a") != "one" || Contents(target / L"payload.txt") != "new" || !std::filesystem::exists(target / L"skin.xml")) return false;
	}
	return true;
}

bool UnsafeNames()
{
	const char* names[] = { "../outside", "..\\outside", "a/../outside", "a\\..//outside", "/rooted", "\\rooted", "C:relative", "C:/absolute", "\\\\server\\share", "//server/share", "\\\\?\\C:\\x", "x:stream", "CON", "nul.txt", "COM1.txt", "LPT9", "x.", "x ", "a//b", "a/./b", "a?b", "a*b", "a|b", "a\x01" };
	for (const char* name : names)
	{
		Fixture f;
		f.save({ Manifest(std::string("path=\"") + name + "\""), { "payload.txt", "bad" } });
		if (f.validate() || f.install()) return false;
		f.save({ Manifest(), { name, "bad" } });
		if (f.validate() || f.install()) return false;
		if (!std::filesystem::is_empty(f.destination)) return false;
	}
	return true;
}

bool ManifestOwnership()
{
	Fixture f;
	for (unsigned i = 0; i < 50; ++i)
	{
		std::string text = "<manifest name=\"Example\"/>";
		if (!LoadManifestInfo(text.data()) || szName == szPath || wcscmp(szName, szPath)) return false;
		Reset();
	}
	// Shipped skins pair a display name containing a separator with an explicit path.
	{
		std::string text = "<manifest name=\"Windows8/Alt\" path=\"Windows\"/>";
		if (!LoadManifestInfo(text.data()) || wcscmp(szName, L"Windows8/Alt") || wcscmp(szPath, L"Windows")) return false;
		Reset();
	}
	for (const char* text : { "<manifest name=\"a/b\"/>", "<manifest name=\"../bad\"/>", "<manifest name=\"ok\" path=\"../bad\"/>", "<manifest name=\"x\" path=\"\"/>", "<manifest name=\"x\" name=\"y\"/>", "<manifest name=\"x", "<manifest name=", "<manifest ", "<manifest name=\"x\" type=\"unknown\"/>" })
	{
		std::string copy(text);
		if (LoadManifestInfo(copy.data())) return false;
	}
	return true;
}

bool UnicodeAndLengths()
{
	Fixture f;
	std::string name;
	for (int i = 0; i < 90; ++i) name += "\xe6\x97\xa5";
	f.save({ Manifest(), { name + ".txt", "unicode" } });
	if (!f.validate() || !f.install()) return false;
	for (const auto& bad : { std::string(260, 'x'), std::string(1040, 'x'), std::string("bad\xc0\xaf"), std::string("bad\xed\xa0\x80"), std::string("embedded\0name", 13) })
	{
		f.save({ Manifest(), { bad, "bad" } });
		if (f.validate() || f.install()) return false;
	}
	std::wstring root(MAX_PATH - 8, L'a');
	WCHAR result[MAX_PATH];
	if (PackageJoin(result, root.c_str(), L"12345678")) return false;
	std::string path(245, 'x');
	f.save({ Manifest("path=\"" + path + "\"") });
	return f.validate() && !f.install();
}

bool InvalidMetadata()
{
	for (unsigned variant = 0; variant < 8; ++variant)
	{
		Fixture f;
		Entry bad = { "payload.txt", "payload" };
		if (variant == 0) bad.sizeExtra = 100;
		if (variant == 1) bad.badCrc = true;
		if (variant == 2) bad.attributes = 0120000U << 16;
		if (variant == 3) bad.flags |= 1;
		if (variant == 4) bad.attributes = FILE_ATTRIBUTE_DIRECTORY;
		if (variant == 5) bad.attributes = FILE_ATTRIBUTE_REPARSE_POINT;
		if (variant == 6) bad.name = "directory/";
		f.save({ Manifest(), bad });
		if (variant == 7) std::filesystem::resize_file(f.archive, std::filesystem::file_size(f.archive) - 12);
		if (f.validate() || f.install() || !std::filesystem::is_empty(f.destination)) return false;
	}
	return true;
}

bool Overwrite()
{
	Fixture f;
	f.save({ Manifest(), { "payload.txt", "first" } });
	if (!f.validate() || !f.install()) return false;
	auto target = f.destination / L"Example" / L"payload.txt";
	f.save({ Manifest(), { "payload.txt", "replacement" } });
	if (!f.validate() || !f.install() || Contents(target) != "replacement") return false;
	Entry bad = { "payload.txt", "bad" };
	bad.badCrc = true;
	f.save({ Manifest(), bad });
	if (f.install() || Contents(target) != "replacement") return false;
	f.save({ Manifest(), { "payload.txt", "bad" } });
	if (!SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_READONLY)) return false;
	bool rejected = !f.install() && Contents(target) == "replacement";
	SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_NORMAL);
	return rejected;
}

bool HardLink()
{
	Fixture f;
	std::filesystem::create_directory(f.destination / L"Example");
	auto outside = f.root / L"sentinel.txt", target = f.destination / L"Example" / L"payload.txt";
	{
		std::ofstream file(outside);
		file << "sentinel";
	}
	if (!CreateHardLinkW(target.c_str(), outside.c_str(), nullptr)) return false;
	f.save({ Manifest(), { "payload.txt", "bad" } });
	return f.validate() && !f.install() && Contents(outside) == "sentinel";
}

// Mount-point junctions do not require the symbolic-link privilege.
bool Junction(const std::filesystem::path& link, const std::filesystem::path& target)
{
	if (!CreateDirectoryW(link.c_str(), nullptr)) return false;
	HANDLE handle = CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
	                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (handle == INVALID_HANDLE_VALUE) return false;
	std::wstring substitute = L"\\??\\" + target.wstring();
	struct Buffer
	{
		DWORD tag;
		WORD size, reserved, subOffset, subLength, printOffset, printLength;
		WCHAR path[2 * MAX_PATH];
	} buffer = {};
	buffer.tag = IO_REPARSE_TAG_MOUNT_POINT;
	buffer.subLength = static_cast<WORD>(substitute.size() * sizeof(WCHAR));
	buffer.printOffset = buffer.subLength + sizeof(WCHAR);
	buffer.printLength = static_cast<WORD>(target.wstring().size() * sizeof(WCHAR));
	memcpy(buffer.path, substitute.c_str(), buffer.subLength);
	memcpy(reinterpret_cast<BYTE*>(buffer.path) + buffer.printOffset, target.c_str(), buffer.printLength);
	buffer.size = 8 + buffer.printOffset + buffer.printLength + sizeof(WCHAR);
	DWORD returned;
	bool ok = DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT, &buffer, buffer.size + 8, nullptr, 0, &returned, nullptr) != 0;
	CloseHandle(handle);
	return ok;
}

bool ReparseEscape()
{
	Fixture f;
	auto outside = f.root / L"outside";
	std::filesystem::create_directory(outside);
	{
		std::ofstream file(outside / L"payload.txt");
		file << "sentinel";
	}
	auto link = f.destination / L"Example";
	if (!Junction(link, outside)) return false;
	f.save({ Manifest(), { "payload.txt", "bad" } });
	bool rejected = f.validate() && !f.install() && Contents(outside / L"payload.txt") == "sentinel";
	RemoveDirectoryW(link.c_str());
	return rejected;
}

bool PinnedDirectory()
{
	Fixture f;
	PACKAGE_DIRECTORY pinned = {};
	if (!PackageLockDirectory(f.destination.c_str(), &pinned, 0)) return false;
	bool blocked = !MoveFileW(f.destination.c_str(), (f.root / L"moved").c_str());
	PackageUnlock(&pinned);
	return blocked;
}

bool MixedPathAndDuplicateBasenames()
{
	Fixture f;
	f.save({ Manifest("path=\"Nested/Mixed\\Skin\""), { "a/payload.txt", "first" }, { "b\\payload.txt", "last" } });
	return f.validate() && !wcscmp(szPath, L"Nested\\Mixed\\Skin") && f.install() && Contents(f.destination / L"Nested" / L"Mixed" / L"Skin" / L"payload.txt") == "last";
}

bool ManifestBuffers()
{
	for (const auto& data : { std::string(), std::string("<manifest name=\"bad\xc0\xaf\"/>"), (std::string("<manifest name=\"ok\"/>") + std::string("\0bad", 4)), std::string(4 * 1024 * 1024 + 1, 'x') })
	{
		Fixture f;
		f.save({ { "skin.xml", data } });
		if (f.validate() || f.install()) return false;
	}
	Fixture f;
	std::string boundary = "<manifest name=\"Example\"/>";
	boundary.resize(4 * 1024 * 1024, ' ');
	f.save({ { "skin.xml", boundary } });
	return f.validate() && f.install();
}

struct RegistryFixture
{
	HKEY key = nullptr;
	std::wstring name;
	RegistryFixture()
	{
		name = L"Software\\EnvyUnpackerTests-" + std::to_wstring(GetCurrentProcessId());
		if (RegCreateKeyExW(HKEY_CURRENT_USER, name.c_str(), 0, nullptr, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS ||
		    RegOverridePredefKey(HKEY_CURRENT_USER, key) != ERROR_SUCCESS) throw std::runtime_error("isolated registry unavailable");
	}
	~RegistryFixture()
	{
		RegOverridePredefKey(HKEY_CURRENT_USER, nullptr);
		if (key) RegCloseKey(key);
		RegDeleteTreeW(HKEY_CURRENT_USER, name.c_str());
	}
};

bool RootSelectionAndRegistryFailure()
{
	Fixture f;
	RegistryFixture registry;
	HKEY settings;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Envy\\Envy", 0, nullptr, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, nullptr, &settings, nullptr) != ERROR_SUCCESS) return false;
	std::wstring base = f.destination.wstring();
	LSTATUS status = RegSetValueExW(settings, L"Path", 0, REG_SZ, reinterpret_cast<const BYTE*>(base.c_str()), static_cast<DWORD>((base.size() + 1) * sizeof(WCHAR)));
	const WCHAR* suffixes[] = { L"Skins", L"Skins", L"Schemas", L"Plugins", L"Data" };
	bool valid = status == ERROR_SUCCESS;
	for (int type = typeSkin; type <= typeData; ++type)
	{
		skinType = type;
		valid = valid && GetInstallDirectory() && (f.destination / suffixes[type]).wstring() == skins_dir;
	}
	std::wstring data = (f.root / L"custom").wstring();
	status = RegSetValueExW(settings, L"DataPath", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), static_cast<DWORD>((data.size() + 1) * sizeof(WCHAR)));
	valid = valid && status == ERROR_SUCCESS && GetInstallDirectory() && data == skins_dir;
	RegCloseKey(settings);
	skinType = typeSkin;
	szPath = _wcsdup(L"Example");
	szXML = _wcsdup(L"skin.xml");
	return valid && !SetSkinAsDefault();
}

bool PreviewCannotAuthorizeDifferentType()
{
	Fixture f;
	f.save({ Manifest() });
	if (!f.validate()) return false;
	f.save({ Manifest("type=\"plugin\"") });
	return !f.install() && std::filesystem::is_empty(f.destination);
}

bool StagingNameCollision()
{
	Fixture f;
	std::string name = ".envy-" + std::to_string(GetCurrentProcessId()) + "-0.tmp";
	f.save({ Manifest(), { name, "valid" } });
	return f.validate() && f.install() && Contents(f.destination / L"Example" / name) == "valid";
}


// Force a late alias after both path checks, exercising the exact native rename
// helper used by InstallEntry rather than relying on a scheduler race.
bool LateHardLinkReplacement()
{
	Fixture f;
	auto target = f.destination / L"payload.txt", staging = f.destination / L"staging.tmp";
	auto outside = f.root / L"sentinel.txt";
	{ std::ofstream file(target); file << "old"; }
	{ std::ofstream file(outside); file << "sentinel"; }
	HANDLE file = CreateFileW(staging.c_str(), GENERIC_WRITE | DELETE, 0, nullptr, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
	bool valid = file != INVALID_HANDLE_VALUE;
	DWORD written = 0;
	valid = valid && WriteFile(file, "new", 3, &written, nullptr) && written == 3 &&
		FlushFileBuffers(file) && PackageCanReplace(target.c_str());
	valid = valid && DeleteFileW(target.c_str()) && CreateHardLinkW(target.c_str(), outside.c_str(), nullptr);
	valid = valid && PackageRenameLeaf(file, L"payload.txt");
	if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
	return valid && Contents(target) == "new" && Contents(outside) == "sentinel";
}

bool ExistingManifestBuffers()
{
	Fixture f;
	RegistryFixture registry;
	HKEY key;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Envy\\Envy\\Skins", 0, nullptr, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS) return false;
	szPath = _wcsdup(L"Example");
	szXML = _wcsdup(L"skin.xml");
	const std::string inputs[] = { "<manifest type=\"skin\"/>", "", "<manifest type=\"\xc0\xaf\"/>", std::string(4 * 1024 * 1024 + 1, 'x') };
	bool valid = true;
	for (size_t i = 0; i < 4; ++i)
	{
		{
			std::ofstream file(f.destination / L"old.xml", std::ios::binary);
			file << inputs[i];
		}
		DWORD enabled = 1, bytes = sizeof(enabled);
		valid = valid && RegSetValueExW(key, L"old.xml", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&enabled), bytes) == ERROR_SUCCESS;
		valid = valid && SetSkinAsDefault();
		valid = valid && RegQueryValueExW(key, L"old.xml", nullptr, nullptr, reinterpret_cast<BYTE*>(&enabled), &bytes) == ERROR_SUCCESS;
		valid = valid && enabled == (i == 0 ? 0U : 1U);
	}
	RegCloseKey(key);
	return valid;
}
}

void register_unpacker_tests(TestSuite& suite)
{
	suite.add_test("Unpacker.ValidPackageTypesAndFlattening", ValidTypes);
	suite.add_test("Unpacker.ManifestAndZipUnsafeNames", UnsafeNames);
	suite.add_test("Unpacker.ManifestOwnershipAndMalformedInput", ManifestOwnership);
	suite.add_test("Unpacker.UnicodeAndLengthBoundaries", UnicodeAndLengths);
	suite.add_test("Unpacker.TruncationCrcAndMetadata", InvalidMetadata);
	suite.add_test("Unpacker.RegularFileReplacement", Overwrite);
	suite.add_test("Unpacker.HardLinkEscape", HardLink);
	suite.add_test("Unpacker.JunctionEscape", ReparseEscape);
	suite.add_test("Unpacker.PinnedDirectoryCannotMove", PinnedDirectory);
	suite.add_test("Unpacker.MixedPathAndDuplicateBasenames", MixedPathAndDuplicateBasenames);
	suite.add_test("Unpacker.ManifestBufferBoundaries", ManifestBuffers);
	suite.add_test("Unpacker.RootSelectionAndRegistryFailure", RootSelectionAndRegistryFailure);
	suite.add_test("Unpacker.PreviewTypeChangeRejected", PreviewCannotAuthorizeDifferentType);
	suite.add_test("Unpacker.StagingNameCollision", StagingNameCollision);
	suite.add_test("Unpacker.ExistingManifestBufferSafety", ExistingManifestBuffers);
	suite.add_test("Unpacker.LateHardLinkReplacement", LateHardLinkReplacement);
}

#ifdef UNPACKER_TEST_MAIN
int main()
{
	TestSuite suite;
	register_unpacker_tests(suite);
	return suite.run_all_tests();
}
#endif
