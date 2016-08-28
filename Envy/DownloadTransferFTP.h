//
// DownloadTransferFTP.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007

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

#include "DownloadTransfer.h"
#include "DownloadSource.h"
#include "Download.h"
#include "Downloads.h"
#include "Handshakes.h"

// Note: FTP active mode code commented out

#define FTP_RETRY_DELAY		30	// Seconds

class CDownloadTransferFTP : public CDownloadTransfer
{
public:
	CDownloadTransferFTP(CDownloadSource* pSource);
	virtual ~CDownloadTransferFTP() {}

public:
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource = TRI_TRUE, DWORD nRetryAfter = FTP_RETRY_DELAY);
	virtual void	Boost();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);

protected:
	// FTP "LIST" helper class
	class CFTPLIST : public CTransfer
	{
	public:
		CFTPLIST() {}
		virtual ~CFTPLIST() {}

		virtual BOOL ConnectTo(const SOCKADDR_IN* pHost)
		{
			m_sData.Empty ();
			return CConnection::ConnectTo( pHost );
		}

		//virtual void AttachTo(CConnection* pConnection)
		//{
		//	m_sData.Empty ();
		//	CTransfer::AttachTo( pConnection );
		//}

		//virtual void Close()
		//{
		//	Handshakes.Remove( this );
		//	CTransfer::Close();
		//}

		virtual BOOL OnRead()
		{
			if ( CTransfer::OnRead() )
			{
				CLockedBuffer pInput( GetInput() );
				if ( pInput->m_nLength > 0 )
				{
					m_sData.Append( CString( (char*) pInput->m_pBuffer ), pInput->m_nLength );
					pInput->Clear();
				}
				return TRUE;
			}
			Close();
			return FALSE;
		}

		virtual QWORD ExtractFileSize() const
		{
			TRACE( L"Extracting file size from:\n%ls\n", m_sData );
			CString in( m_sData ), out;
			for ( int n = 0 ; Split( in, L' ', out ) ; ++n )
			{
				int i = 0;
				for ( ; i < out.GetLength() ; ++i )
					if ( ! isdigit( out [i] ) )
						break;
				if ( i == out.GetLength() && out [0] != L'0' && n != 2 )
				{
					QWORD size = _tstoi64( out );
					TRACE( L"File size: %ld bytes\n", size );
					return size;
				}
			}
			TRACE( L"Unknown file size.\n" );
			return SIZE_UNKNOWN;
		}

	protected:
		CString m_sData;	// Received file listing data

		inline bool Split(CString& in, TCHAR token, CString& out) const
		{
			in = in.Trim( L" \t\r\n" );
			if ( in.IsEmpty() )
			{
				out.Empty();
				return false;
			}
			int p = in.ReverseFind( token );
			if ( p != -1)
			{
				out = in.Mid( p + 1 );
				in = in.Mid( 0, p );
			}
			else
			{
				out = in;
				in.Empty();
			}
			return true;
		}
	};

	// FTP "RETR" helper class
	class CFTPRETR : public CTransfer
	{
	public:
		CFTPRETR() : m_pOwner( NULL ), m_tContent( 0 ), m_nTotal( 0 ) {}
		virtual ~CFTPRETR() {}

		inline void SetOwner(CDownloadTransferFTP* pOwner)
		{
			m_pOwner = pOwner;
		}

		virtual BOOL ConnectTo(const SOCKADDR_IN* pHost)
		{
			m_tContent = GetTickCount();
			m_nTotal = 0;
			return CConnection::ConnectTo( pHost );
		}

		//virtual void AttachTo(CConnection* pConnection)
		//{
		//	m_tContent = GetTickCount();
		//	m_nTotal = 0;
		//	CTransfer::AttachTo( pConnection );
		//}

		//virtual void Close()
		//{
		//	Handshakes.Remove( this );
		//	CTransfer::Close();
		//}

		virtual BOOL OnRead()
		{
			if ( CTransfer::OnRead() )
			{
				CLockedBuffer pInput( GetInput() );

				if ( m_pOwner && pInput->m_nLength > 0 )
				{
					QWORD nLength = min( (QWORD)pInput->m_nLength, m_pOwner->m_nLength - m_pOwner->m_nPosition );
					m_pOwner->m_pDownload->SubmitData( m_pOwner->m_nOffset + m_pOwner->m_nPosition, pInput->m_pBuffer, nLength );
					m_pOwner->m_nPosition += nLength;
					m_pOwner->m_nDownloaded += nLength;
					// Measuring speed
					DWORD nCurrent = GetTickCount();
					if ( nCurrent - m_tContent != 0 )
					{
						m_pOwner->GetSource()->m_nSpeed =
							(DWORD) ( ( ( pInput->m_nLength + m_nTotal ) /
							( nCurrent - m_tContent ) ) * 1000 );
						m_tContent = nCurrent;
						m_nTotal = 0;
					}
					else
						m_nTotal += pInput->m_nLength;
					pInput->Clear();
					if ( m_pOwner->m_nPosition >= m_pOwner->m_nLength )
					{
						m_pOwner->GetSource()->AddFragment( m_pOwner->m_nOffset, m_pOwner->m_nLength );
						Close();
					}
				}
				return TRUE;
			}
			Close();
			return FALSE;
		}

	protected:
		CDownloadTransferFTP* m_pOwner;	// Owner object
		DWORD		m_tContent;			// Last Receive time
		QWORD		m_nTotal;			// Received bytes by m_tContent time
	};

	enum FTP_STATES {
		ftpConnecting,									// Initial state
		ftpUSER, ftpPASS,								// Authenticating
		ftpSIZE_TYPE, ftpSIZE,							// File size getting via SIZE: TYPE I -> SIZE
		ftpLIST_TYPE, ftpLIST_PASVPORT, ftpLIST,		// File size getting via LIST: TYPE A -> PASV or PORT -> LIST
		ftpDownloading,									// Intermediate state
		ftpRETR_TYPE, ftpRETR_PASVPORT, ftpRETR_REST, ftpRETR,		// Downloading: TYPE I -> PASV or PORT -> REST -> RETR
		ftpABOR											// Aborting after each transfer (for some strange FTP servers)
	};

	FTP_STATES		m_FtpState;			// FTP Control chanell state
	CFTPLIST		m_LIST;				// FTP "LIST" helper object
	CFTPRETR		m_RETR;				// FTP "RETR" helper object
	BOOL			m_bPassive;			// Passive or Active FTP mode
	BOOL			m_bSizeChecked; 	// File size flag
	BOOL			m_bMultiline;		// Processing multiline reply
	CString			m_sMultiNumber; 	// Multiline number
	CString			m_sMultiReply;		// Multiline reply

	BOOL			StartNextFragment(); // Connecting or file size getting or download starting
	BOOL			SendCommand(LPCTSTR args = NULL);	// Sending command to FTP server
};
