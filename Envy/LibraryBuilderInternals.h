//
// LibraryBuilderInternals.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2014
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

class CXMLElement;


class CLibraryBuilderInternals
{
//public:
//	CLibraryBuilderInternals();
//	virtual ~CLibraryBuilderInternals()=0;

private:
	static LPCTSTR	pszID3Genre[];

public:
	int			LookupID3v1Genre(const CString& strGenre) const;
	bool		ExtractMetadata(DWORD nIndex, const CString& strPath, HANDLE hFile);
	bool		ExtractProperties(DWORD nIndex, const CString& strPath);	// Windows properties

private:
	// ID3v1 and ID3v2 and MP3
	bool		ReadID3v1(DWORD nIndex, HANDLE hFile);
	bool		CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, CString strValue);
	bool		ReadID3v2(DWORD nIndex, HANDLE hFile);
	bool		CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, bool bSkipLanguage = false);
	bool		ReadMP3Frames(DWORD nIndex, HANDLE hFile);
	bool		ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore);

	// Module Version
	bool		ReadVersion(DWORD nIndex, LPCTSTR pszPath);
	bool		CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId, bool bCommaToDot = false);
	CString		GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId);
	bool		GetLanguageId(LPVOID pBuffer, UINT nSize, WORD nLangId, DWORD &nId, bool bOnlyPrimary = false);
	DWORD		GetBestLanguageId(LPVOID pBuffer);

	// Module Manifest Validation
	bool		ValidateManifest(LPCTSTR pszPath);

	// Windows Installer
	bool		ReadMSI(DWORD nIndex, LPCTSTR pszPath);
	CString		GetSummaryField(MSIHANDLE hSummaryInfo, UINT nProperty);

	// Image Files (Prefer GFLLibraryBuilder Plugin)
	bool		ReadJPEG(DWORD nIndex, HANDLE hFile);
	bool		ReadGIF(DWORD nIndex, HANDLE hFile);
	bool		ReadPNG(DWORD nIndex, HANDLE hFile);
	bool		ReadBMP(DWORD nIndex, HANDLE hFile);

	// General Media
	bool		ReadFLV(DWORD nIndex, HANDLE hFile);
	bool		ReadFLVVariable(HANDLE hFile, DWORD& nRemaning, VARIANT& varData, CXMLElement* pXML = NULL);
	bool		ReadFLVDouble(HANDLE hFile, DWORD& nRemaning, double& dValue);
	bool		ReadFLVBool(HANDLE hFile, DWORD& nRemaning, bool& bValue);
	bool		ReadFLVString(HANDLE hFile, DWORD& nRemaning, BOOL bLong, CStringA& strValue);
	bool		ReadFLVEMCA(HANDLE hFile, DWORD& nRemaning, CXMLElement* pXML = NULL);

	bool		ReadASF(DWORD nIndex, HANDLE hFile);
	bool		ReadAVI(DWORD nIndex, HANDLE hFile);
	bool		ReadMPEG(DWORD nIndex, HANDLE hFile);
//	bool		ReadMKV(DWORD nIndex, HANDLE hFile);
	bool		ReadOGG(DWORD nIndex, HANDLE hFile);
	BYTE*		ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize = 0);
	bool		ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str);
	bool		ReadAPE(DWORD nIndex, HANDLE hFile, bool bPreferFooter = false);

	bool		ReadDJVU(DWORD nIndex, HANDLE hFile);
	bool		ReadCHM(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	bool		ReadPDF(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	CString		ReadPDFLine(HANDLE hFile, bool bReverse, bool bComplex = false, bool bSplitter = true);
	CString		DecodePDFText(CString strInput);

	bool		ReadTorrent(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	bool		ReadCollection(DWORD nIndex, LPCTSTR pszPath);
	bool		ReadSkin(DWORD nIndex);
	bool		ReadBook(DWORD nIndex, CString strPath);
	bool		ReadText(DWORD nIndex, CString strPath);
};
