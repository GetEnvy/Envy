//
// DownloadWithFile.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

#include "DownloadWithTransfers.h"
#include "FragmentedFile.h"


class CDownloadWithFile : public CDownloadWithTransfers
{
protected:
	CDownloadWithFile();
	virtual ~CDownloadWithFile();

public:
	TRISTATE		m_bVerify;				// Verify status (TRI_TRUE verified, TRI_FALSE failed, TRI_UNKNOWN not yet)
	DWORD			m_tReceived;
private:
	auto_ptr< CFragmentedFile >	m_pFile;	// File(s)
	DWORD			m_nFileError;			// Last file/disk error
	CString			m_sFileError;			// More info about error

public:
	virtual float	GetProgress() const;
	QWORD			GetVolumeComplete() const;
	QWORD			GetVolumeRemaining() const;
	DWORD			GetTimeRemaining() const;
	CString			GetDisplayName() const;
	BOOL			IsValid() const;
	BOOL			IsFileOpen() const;
	BOOL			IsComplete() const;
	BOOL			IsRemaining() const;
	BOOL			IsPositionEmpty(QWORD nOffset);
//	BOOL			GetFragment(CDownloadTransfer* pTransfer);
//	BOOL			AreRangesUseful(const Fragments::List& oAvailable);
//	BOOL			IsRangeUseful(QWORD nOffset, QWORD nLength);
//	BOOL			IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength);
	BOOL			ClipUploadRange(QWORD nOffset, QWORD& nLength) const;
	BOOL			GetRandomRange(QWORD& nOffset, QWORD& nLength) const;
	bool			GetAvailableRanges( CString& strRanges ) const;
	QWORD			InvalidateFileRange(QWORD nOffset, QWORD nLength);
	QWORD			EraseRange(QWORD nOffset, QWORD nLength);
	BOOL			SetSize(QWORD nSize);
	BOOL			MakeComplete();
	Fragments::List	GetFullFragmentList() const;	// All fragments which must be downloaded
	Fragments::List	GetEmptyFragmentList() const;	// All empty fragments
//	Fragments::List	GetWantedFragmentList() const;
	CFragmentedFile* GetFile();
	BOOL			FindByPath(const CString& sPath) const;
	DWORD			GetFileCount() const;
	QWORD			GetOffset(DWORD nIndex) const;
	QWORD			GetLength(DWORD nIndex = 0) const;
	CString			GetPath(DWORD nIndex = 0) const;
	CString			GetName(DWORD nIndex = 0) const;
	QWORD			GetCompleted(DWORD nIndex) const;
	int				SelectFile(CSingleLock* pLock = NULL) const;
	DWORD			GetFileError() const;
	const CString&	GetFileErrorString() const;
	void			SetFileError(DWORD nFileError, LPCTSTR szFileError);
	void			ClearFileError();
	DWORD			MoveFile(LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine = NULL, CDownloadTask* pTask = NULL);
	virtual bool	Rename(const CString& strName);		// Set download new name safely
	virtual BOOL	SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength);

protected:
	BOOL			Open(const CEnvyFile* pFile);
	BOOL			Open(const CBTInfo& pBTInfo);
	BOOL			OpenFile();		// Legacy (Crash workaround)
	void			CloseFile();	// Close files of this download
	void			DeleteFile();
	BOOL			FlushFile();
	void			AttachFile(CFragmentedFile* pFile);
	BOOL			ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	BOOL			WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	void			SerializeFile(CArchive& ar, int nVersion);
	virtual BOOL	OnVerify(const CLibraryFile* pFile, TRISTATE bVerified);	// File was hashed and verified in the Library

	virtual void	Serialize(CArchive& ar, int nVersion);

	// Unsupported:
	//BOOL			AppendMetadata();
	//BOOL			AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML);
	//Fragments::List GetPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);
};
