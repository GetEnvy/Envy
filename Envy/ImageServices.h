//
// ImageServices.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2010
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CImageFile;


class CImageServices : public CComObject
{
	DECLARE_DYNCREATE(CImageServices)

public:
	static BOOL	IsFileViewable(LPCTSTR pszPath);
	static BOOL	LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	static BOOL	LoadFromFile(CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	static BOOL	SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
	static BOOL	SaveToFile(CImageFile* pFile, LPCTSTR szFilename, int nQuality, DWORD* pnLength = NULL);

protected:
	typedef std::list< std::pair< CLSID, CAdapt< CComPtr< IImageServicePlugin > > > > service_list;

	static BOOL	LookupUniversalPlugins(service_list& oList);	// Get universal plugins
	static BOOL	LoadFromFileHelper(IImageServicePlugin* pService, CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly, BOOL bPartialOk);
	static SAFEARRAY* ImageToArray(CImageFile* pFile);

	// IImageServicePlugin
	BEGIN_INTERFACE_PART(ImageService, IImageServicePlugin)
		STDMETHOD(LoadFromFile)( __in BSTR sFile, __inout IMAGESERVICEDATA* pParams, __out SAFEARRAY** ppImage );
		STDMETHOD(LoadFromMemory)( __in BSTR sType, __in SAFEARRAY* pMemory, __inout IMAGESERVICEDATA* pParams, __out SAFEARRAY** ppImage );
		STDMETHOD(SaveToFile)( __in BSTR sFile, __inout IMAGESERVICEDATA* pParams, __in SAFEARRAY* pImage);
		STDMETHOD(SaveToMemory)( __in BSTR sType, __out SAFEARRAY** ppMemory, __inout IMAGESERVICEDATA* pParams, __in SAFEARRAY* pImage);
	END_INTERFACE_PART(ImageService)

	DECLARE_INTERFACE_MAP()
};

extern CImageServices ImageServices;
