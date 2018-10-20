//
// DCClient.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2010 and PeerProject 2010-2012
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

#include "Transfer.h"

class CDCNeighbour;
class CDownloadTransferDC;
class CUploadTransferDC;

// Class for DC++ remote client connection.
// This class uses fictive CUploadTransferDC and CDownloadTransferDC classes
// to mimic download and upload connections like CEDClient.

class CDCClient : public CTransfer
{
public:
	CDCClient(const IN_ADDR* pHubAddress = NULL, WORD nHubPort = 0, LPCTSTR szNick = NULL, LPCTSTR szRemoteNick = NULL);
	virtual ~CDCClient();

public:
	Hashes::Guid	m_oGUID;							// GUID to identify callback connections

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	OnRun();

	CString			GetUserAgent();						// Detect remote user agent
	BOOL			Connect();							// Re-connect
	void			AttachDownload(CDownloadTransferDC* pTransfer); 	// Attach download transfer
	void			OnDownloadClose();					// When download transfer closed
	void			OnUploadClose();					// When upload transfer closed
	BOOL			Handshake();						// Second part of handshaking - Send [$MyNick, $Lock,] $Supports, $Direction and $Key commands
	BOOL			SendCommand(const CString& strSend);	// Send command
	void			Merge(CDCClient* pClient);			// Merge all useful data from old to current client and then destroy it
	void			Remove();							// Destroy this object
	BOOL			IsOnline() const;					// Check if client on-line
	BOOL			IsDownloading() const;				// Check if client busy downloading something (not in command mode)
	BOOL			IsUploading() const;				// Check if client busy uploading something (not in command mode)
	BOOL			IsIdle() const;						// Check if client does nothing (no upload, no download)
	BOOL			OnPush();							// Accept push connection

protected:
	CDownloadTransferDC* m_pDownloadTransfer;			// Download stream
	CUploadTransferDC*   m_pUploadTransfer;				// Upload stream
	CString			m_sNick;							// User nick
	std::string		m_strKey;							// Key calculated for remote client lock
	BOOL			m_bExtended;						// Using extended protocol
	CStringList		m_oFeatures;						// Remote client supported features
	TRISTATE		m_bDirection;						// Got $Direction command: TRI_TRUE - remote client want download, TRI_FALSE - upload.
	BOOL			m_bNumberSent;						// My $Direction number sent
	int				m_nNumber;							// My $Direction number (0...0x7fff)
	int				m_nRemoteNumber;					// Remote client $Direction number (0...0x7fff), -1 - unknown.
	BOOL			m_bLogin;							// Got $Lock command
	BOOL			m_bKey;								// Got $Key command

protected:
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();

	BOOL			ReadCommand(std::string& strLine);			// Read single command from input buffer
	BOOL			OnCommand(const std::string& strCommand, const std::string& strParams);	// Got DC++ command
	BOOL			OnChat(const std::string& strMessage);		// Got chat message
	BOOL			OnMyNick(const std::string& strParams);		// Got $MyNick command
	BOOL			OnLock(const std::string& strParams);		// Got $Lock command
	BOOL			OnSupports(const std::string& strParams);	// Got $Supports command
	BOOL			OnKey(const std::string& strParams);		// Got $Key command
	BOOL			OnDirection(const std::string& strParams);	// Got $Direction command
	BOOL			OnGet(const std::string& strParams);		// Got $Get command
	BOOL			OnSend(const std::string& strParams);		// Got $Send command
	BOOL			OnADCGet(const std::string& strParams); 	// Got $ADCGET command
	BOOL			OnADCSnd(const std::string& strParams); 	// Got $ADCSND command
	BOOL			OnMaxedOut(const std::string& strParams);	// Got $MaxedOut command
	BOOL			OnError(const std::string& strParams);		// Got $Error command
	std::string		GenerateLock() const;				// Generating challenge for this client
	int				GenerateNumber() const;				// Generating direction number
	BOOL			Greetings();						// First part of handshaking - Send $MyNick, $Lock
	BOOL			CanDownload() const;				// Can Envy start download?
	BOOL			CanUpload() const;					// Can Envy start upload?
	void			DetachDownload();					// Close download
	void			DetachUpload(); 					// Close upload
	BOOL			StartDownload();					// Start download
};
