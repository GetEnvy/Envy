//
// ImageFile.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once


class CImageFile
{
public:
	CImageFile();
	virtual ~CImageFile();

public:
	BOOL	m_bScanned;
	int		m_nWidth;
	int		m_nHeight;
	DWORD	m_nComponents;
	BOOL	m_bLoaded;
	LPBYTE	m_pImage;
	WORD	m_nFlags;

public:
	BOOL	LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromURL(LPCTSTR pszURL);
	BOOL	LoadFromBitmap(HBITMAP hBitmap, BOOL bScanOnly = FALSE);	// Get image copy from HBITMAP (24/32-bit only)
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
	BOOL	SaveToFile(LPCTSTR pszFile, int nQuality, DWORD* pnLength = NULL);
	DWORD	GetSerialSize() const;
	void	Serialize(CArchive& ar);
	HBITMAP	CreateBitmap(HDC hUseDC = 0);
	BOOL	FitTo(int nNewWidth, int nNewHeight = 0);
	BOOL	Resample(int nNewWidth, int nNewHeight = 0);
//	BOOL	FastResample(int nNewWidth, int nNewHeight);
	BOOL	EnsureRGB(COLORREF crBack = 0xFFFFFFFF);
	BOOL	SwapRGB();

	static HBITMAP LoadBitmapFromFile(LPCTSTR pszFile, BOOL bRGB = FALSE);
	static HBITMAP LoadBitmapFromResource(UINT nResourceID, HINSTANCE hInstance = AfxGetResourceHandle());

	enum ImageFlags
	{
		idRemote = 0x1
	};

protected:
	void	Clear();
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crBack);

private:
	CImageFile(const CImageFile&);
	CImageFile& operator=(CImageFile&);
};
