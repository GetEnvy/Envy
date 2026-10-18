//
// Utils.c
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

static const WCHAR* ManifestKeys[] = { L"type", L"name", L"path", L"version", L"author", L"updatedby" };

static void FreeManifest(WCHAR** values)
{
	unsigned i;
	for (i = 0; i < 6; ++i) { free(values[i]); values[i] = NULL; }
}

static int ManifestSpace(WCHAR c)
{
	return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

// Parse only a complete manifest start tag, with one owner per retained value.
static int ParseManifest(const WCHAR* text, WCHAR** values)
{
	const WCHAR* p = text;
	if (!p || wcsncmp(p, L"<manifest", 9) || !ManifestSpace(p[9])) return 0;
	p += 9;
	for (;;)
	{
		const WCHAR *key, *value;
		size_t keyLength, valueLength;
		WCHAR quote;
		unsigned i;
		while (ManifestSpace(*p)) ++p;
		if (*p == L'>' || (*p == L'/' && p[1] == L'>')) return 1;
		key = p;
		while (*p && !ManifestSpace(*p) && *p != L'=' && *p != L'>' && *p != L'/') ++p;
		keyLength = p - key;
		while (ManifestSpace(*p)) ++p;
		if (!keyLength || *p != L'=') return 0;
		++p;
		while (ManifestSpace(*p)) ++p;
		quote = *p++;
		if (quote != L'\'' && quote != L'"') return 0;
		value = p;
		while (*p && *p != quote && *p != L'<') ++p;
		if (*p != quote) return 0;
		valueLength = p++ - value;
		for (i = 0; i < 6; ++i)
		{
			if (wcslen(ManifestKeys[i]) == keyLength && !_wcsnicmp(key, ManifestKeys[i], keyLength))
			{
				if (values[i]) return 0;
				values[i] = (WCHAR*)malloc((valueLength + 1) * sizeof(WCHAR));
				if (!values[i]) return 0;
				memcpy(values[i], value, valueLength * sizeof(WCHAR));
				values[i][valueLength] = 0;
			}
		}
		if (!ManifestSpace(*p) && *p != L'/' && *p != L'>') return 0;
	}
}

int LoadManifestInfo(char* buffer)
{
	WCHAR* text = (WCHAR*)GetUnicodeString(buffer);
	WCHAR* values[6] = { 0 };
	WCHAR* path = NULL;
	int type = typeSkin, valid = 0;
	if (!text || !ParseManifest(text, values)) goto done;
	if (values[0])
	{
		if (!_wcsicmp(values[0], L"skin")) type = typeSkin;
		else if (!_wcsicmp(values[0], L"language")) type = typeLang;
		else if (!_wcsicmp(values[0], L"schemas")) type = typeSchemas;
		else if (!_wcsicmp(values[0], L"plugin")) type = typePlugin;
		else if (!_wcsicmp(values[0], L"data") || !_wcsicmp(values[0], L"security")) type = typeData;
		else goto done;
	}
	// A name is only a folder when no path is given; with a path it is display text,
	// and shipped skins use names such as "Windows8/Alt" next to path="Windows".
	if (!values[1] || !PackageRelativePath(values[1], 0) || (!values[2] && wcspbrk(values[1], L"/\\"))) goto done;
	if (values[2] && !PackageRelativePath(values[2], 0)) goto done;
	path = _wcsdup(values[2] ? values[2] : values[1]);
	if (!path) goto done;
	{
		WCHAR* separator;
		for (separator = path; *separator; ++separator) if (*separator == L'/') *separator = L'\\';
	}
	free(szName); szName = values[1]; values[1] = NULL;
	free(szPath); szPath = path; path = NULL;
	free(szVersion); szVersion = values[3]; values[3] = NULL;
	free(szAuthor); szAuthor = values[4]; values[4] = NULL;
	free(szUpdates); szUpdates = values[5]; values[5] = NULL;
	skinType = type;
	valid = 1;
done:
	free(path);
	free(text);
	FreeManifest(values);
	return valid;
}

static int CheckManifestForSkin(LPCTSTR relative)
{
	WCHAR file[MAX_PATH];
	FILE* stream;
	long size;
	char* bytes = NULL;
	WCHAR *text = NULL, *manifest;
	WCHAR* values[6] = { 0 };
	int skin = 0;
	if (!PackageJoin(file, skins_dir, relative)) return 0;
	stream = _wfopen(file, L"rb");
	if (!stream) return 0;
	if (fseek(stream, 0, SEEK_END)) goto done;
	size = ftell(stream);
	if (size <= 0 || size > 4 * 1024 * 1024) goto done;
	if (fseek(stream, 0, SEEK_SET)) goto done;
	bytes = (char*)malloc((size_t)size + 1);
	if (!bytes || fread(bytes, 1, size, stream) != (size_t)size || memchr(bytes, 0, size)) goto done;
	bytes[size] = 0;
	text = (WCHAR*)GetUnicodeString(bytes);
	if (!text) goto done;
	manifest = wcsstr(text, L"<manifest");
	if (!manifest) skin = 1;
	else if (ParseManifest(manifest, values)) skin = !values[0] || !_wcsicmp(values[0], L"skin");
done:
	FreeManifest(values);
	free(text);
	free(bytes);
	fclose(stream);
	return skin;
}

int SetSkinAsDefault()
{
	HKEY key;
	WCHAR xml[MAX_PATH];
	DWORD index, enabled;
	if (skinType != typeSkin || !szXML || !PackageJoin(xml, szPath, szXML)) return 0;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Envy\\Envy\\Skins", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &key) != ERROR_SUCCESS)
		return 0;
	for (index = 0; ; ++index)
	{
		WCHAR name[MAX_PATH];
		DWORD length = MAX_PATH;
		LSTATUS status = RegEnumValueW(key, index, name, &length, NULL, NULL, NULL, NULL);
		if (status == ERROR_NO_MORE_ITEMS) break;
		if (status != ERROR_SUCCESS) continue;
		if (_wcsicmp(name, xml) && CheckManifestForSkin(name))
		{
			enabled = 0;
			RegSetValueExW(key, name, 0, REG_DWORD, (BYTE*)&enabled, sizeof(enabled));
		}
	}
	enabled = 1;
	{
		LSTATUS status = RegSetValueExW(key, xml, 0, REG_DWORD, (BYTE*)&enabled, sizeof(enabled));
		RegCloseKey(key);
		return status == ERROR_SUCCESS;
	}
}
