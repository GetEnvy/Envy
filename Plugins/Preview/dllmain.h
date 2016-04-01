//
// dllmain.h : Declaration of module class.
//

#pragma once

#include "Resource.h"
#include "Preview.h"

#define REGISTRY_PATH	L"Software\\Envy\\Envy Preview Plugin"
#define PLUGIN_PATH		L"Software\\Envy\\Envy\\Plugins\\DownloadPreview"


class CPreviewModule : public CAtlDllModuleT< CPreviewModule >
{
public :
	DECLARE_LIBID(LIBID_PreviewLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PREVIEW, "{78537474-C03F-4ADE-A817-BD7CC0A4D2B4}")
};

extern class CPreviewModule _AtlModule;

void LoadData(CAtlMap< CString, CString >& oData);
void SaveData(const CAtlMap< CString, CString >& oData);
