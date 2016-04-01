//
// UploadTransferDC.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2011-2012 and Shareaza 2010
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

class CDCClient;
class CLibraryFolder;
class CUploadTransfer;


class CUploadTransferDC : public CUploadTransfer
{
public:
	CUploadTransferDC(CDCClient* pClient);
	virtual ~CUploadTransferDC();

public:
	CDCClient*		m_pClient;			// Upload owner

	virtual void	Close(UINT nError = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual void	OnDropped();
	virtual BOOL	OnRun();
	virtual BOOL	OnWrite();

	// Got $ADCGET command
	BOOL			OnUpload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions);
	BOOL			IsIdle() const; 	// Check if transfer idle

protected:
	DWORD			m_tRankingCheck;	// Time the queue position was last checked
	BOOL			m_bGet;				// Client uses $Get
	CBuffer			m_pXML;				// Cached library file list

	// Check the client's Q rank. Start upload or send notification if required
	BOOL			CheckRanking();
	void			Cleanup(BOOL bDequeue = TRUE);
	BOOL			RequestFileList(BOOL bFile, BOOL bZip, const std::string& strFilename, QWORD nOffset, QWORD nLength);
	BOOL			RequestTigerTree(CLibraryFile* pFile, QWORD nOffset, QWORD nLength);
	BOOL			RequestFile(CLibraryFile* pFile, QWORD nOffset, QWORD nLength);
	BOOL			SendFile();
};
