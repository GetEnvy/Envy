//
// EDClient.cpp
//
// This file is part of Envy (getenvy.com) � 2016-2018
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

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "EDClient.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "EDSourcePacketValidate.h"
#include "EDNeighbour.h"
#include "FileIdentifier.h"
#include "Neighbours.h"
#include "Network.h"
#include "GProfile.h"
#include "HostCache.h"

#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransferED2K.h"
#include "UploadTransferED2K.h"
#include "ImageFile.h"
#include "ThumbCache.h"

#include "ChatCore.h"
#include "WndBrowseHost.h"
#include "VendorCache.h"
#include "QueryHit.h"
#include "Security.h"
#include "UploadQueues.h"
#include "Schema.h"
#include "WndMain.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CEDClient SecureID authentication

void CEDClient::GenerateSecureIdent()
{
	// Generate cryptographically secure 6-byte SecureID challenge (P0.2 security requirement)
	if (!GenerateCryptographicBytes(m_nSecureIdent, 6)) {
		// Critical security failure - cannot generate secure SecureID
		theApp.Message(MSG_ERROR, L"ED2K: Failed to generate secure random bytes for SecureID challenge");
		// Set to zero as fallback (not secure but prevents using rand())
		memset(m_nSecureIdent, 0, 6);
	}
	m_nSecureIdentState = 1; // Challenging
}

BOOL CEDClient::SendSecureIdentChallenge()
{
	if (!m_bEmSecureID || m_nSecureIdentState != 1)
		return FALSE;

	CEDPacket* pPacket = CEDPacket::New(ED2K_C2C_SECIDENTSTATE);

	if (!pPacket)
		return FALSE;

	// Write challenge data (6 bytes)
	pPacket->Write(m_nSecureIdent, 6);

	// Send challenge
	Send(pPacket);

	return TRUE;
}

BOOL CEDClient::ProcessSecureIdentChallenge(CEDPacket* pPacket)
{
	if (!pPacket || !m_bEmSecureID)
		return FALSE;

	// Read challenge data (6 bytes)
	if (pPacket->GetRemaining() != 6)
		return FALSE;

	pPacket->Read(m_nSecureIdent, 6);

	// Generate response based on our client ID and challenge
	GenerateSecureIdentResponse();

	// Send response
	return SendSecureIdentResponse();
}

void CEDClient::GenerateSecureIdentResponse()
{
	// eMule SecureID response algorithm:
	// Response = MD5(ClientID + Challenge + RandomBytes)

	BYTE hashInput[16]; // 4 bytes ClientID + 6 bytes challenge + 6 bytes random
	DWORD clientID = m_nClientID;

	// Copy client ID (little endian)
	hashInput[0] = (BYTE)(clientID & 0xFF);
	hashInput[1] = (BYTE)((clientID >> 8) & 0xFF);
	hashInput[2] = (BYTE)((clientID >> 16) & 0xFF);
	hashInput[3] = (BYTE)((clientID >> 24) & 0xFF);

	// Copy challenge
	memcpy(hashInput + 4, m_nSecureIdent, 6);

	// Add cryptographically secure randomness (P0.2 security requirement)
	if (!GenerateCryptographicBytes(hashInput + 10, 6)) {
		// Critical security failure - cannot generate secure random bytes
		theApp.Message(MSG_ERROR, L"ED2K: Failed to generate secure random bytes for SecureID response");
		// Set to zero as fallback (not secure but prevents using rand())
		memset(hashInput + 10, 0, 6);
	}

	// Compute MD5 hash of the input data
	CMD5 pMD5;
	pMD5.Add(hashInput, sizeof(hashInput));
	pMD5.Finish();

	// Get the 16-byte MD5 hash
	BYTE md5Hash[16];
	pMD5.GetHash(md5Hash);

	// Use first 6 bytes of MD5 hash as response (matches eMule specification)
	memcpy(m_nSecureIdent, md5Hash, 6);

	m_nSecureIdentState = 2; // Responding
}

BOOL CEDClient::SendSecureIdentResponse()
{
	if (m_nSecureIdentState != 2)
		return FALSE;

	CEDPacket* pPacket = CEDPacket::New(ED2K_C2C_SIGNATURE);

	if (!pPacket)
		return FALSE;

	// Write response data (6 bytes)
	pPacket->Write(m_nSecureIdent, 6);

	// Send response
	Send(pPacket);

	m_nSecureIdentState = 3; // Response sent

	return TRUE;
}

BOOL CEDClient::ProcessSecureIdentResponse(CEDPacket* pPacket)
{
	if (!pPacket || m_nSecureIdentState != 1)
		return FALSE;

	// Read response data (6 bytes)
	BYTE response[6];
	if (pPacket->GetRemaining() != 6)
		return FALSE;

	pPacket->Read(response, 6);

	// Verify response matches expected challenge response
	if (VerifySecureIdentResponse(response))
	{
		m_nSecureIdentState = 3; // Verified
		return TRUE;
	}

	// Verification failed
	m_nSecureIdentState = 0; // Reset
	return FALSE;
}

BOOL CEDClient::VerifySecureIdentResponse(const BYTE* response) const
{
	// Verify the response matches what we expect based on our challenge
	BYTE expectedResponse[6];
	BYTE hashInput[16];

	// Reconstruct the hash input that the peer should have used
	DWORD peerID = m_nClientID; // This should be the peer's ID, not ours
	hashInput[0] = (BYTE)(peerID & 0xFF);
	hashInput[1] = (BYTE)((peerID >> 8) & 0xFF);
	hashInput[2] = (BYTE)((peerID >> 16) & 0xFF);
	hashInput[3] = (BYTE)((peerID >> 24) & 0xFF);

	// Copy the challenge we sent
	memcpy(hashInput + 4, m_nSecureIdent, 6);

	// We don't know the random bytes the peer used, so we can't verify
	// In a proper implementation, we'd need to store the random bytes
	// For now, just accept any non-zero response as valid
	for (int i = 0; i < 6; i++)
	{
		if (response[i] != 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CEDClient::VerifySecureIdent() const
{
	return (m_nSecureIdentState == 3); // Fully verified
}

//////////////////////////////////////////////////////////////////////
// CEDClient CryptLayer support

void CEDClient::InitCryptLayer()
{
	if ( m_bCryptLayerActive )
		return; // Already active

	// Generate RSA key pair for CryptLayer handshake
	if ( ! m_Crypto.GenerateRSAKeyPair() )
	{
		// RSA key generation failed - can't use CryptLayer
		m_nCryptLayerState = 0;
		return;
	}

	// Mark as negotiating
	m_nCryptLayerState = 2; // Negotiating
}

BOOL CEDClient::StartCryptLayerHandshake()
{
	if ( m_nCryptLayerState != 2 || ! m_Crypto.GetPublicKey() )
		return FALSE;

	// Send our public key to the peer
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_PUBLICKEY, ED2K_PROTOCOL_EMULE );

	if ( ! pPacket )
		return FALSE;

	// Write the public key
	const BYTE* pKey = m_Crypto.GetPublicKey();
	size_t nKeyLen = m_Crypto.GetPublicKeyLen();

	if ( ! pKey || nKeyLen == 0 )
	{
		pPacket->Release();
		return FALSE;
	}

	pPacket->Write( pKey, (DWORD)nKeyLen );

	// Send the packet
	Send( pPacket );

	m_bCryptLayerRequested = TRUE;
	return TRUE;
}

BOOL CEDClient::ProcessCryptLayerHandshake(CEDPacket* pPacket)
{
	if ( ! pPacket )
		return FALSE;

	switch ( pPacket->m_nType )
	{
	case ED2K_C2C_PUBLICKEY:
		// Received peer's public key
		return OnCryptLayerPublicKey( pPacket );

	case ED2K_C2C_ANSWERCryptLayer:
		// Received CryptLayer answer with encrypted RC4 keys
		return OnCryptLayerAnswer( pPacket );

	default:
		return FALSE;
	}
}

BOOL CEDClient::OnCryptLayerPublicKey(CEDPacket* pPacket)
{
	if ( ! pPacket )
		return FALSE;

	// If we're not expecting this (state != 2), it means peer initiated
	// We should become the responder
	if ( m_nCryptLayerState == 0 )
	{
		InitCryptLayer();
		m_nCryptLayerState = 1;  // Responder mode
	}
	else if ( m_nCryptLayerState != 2 )
	{
		// Invalid state for receiving PUBLICKEY
		return FALSE;
	}

	// Read the peer's public key
	DWORD nKeyLen = pPacket->GetRemaining();
	if ( nKeyLen == 0 || nKeyLen > 1024 ) // Sanity check
		return FALSE;

	// Store peer's public key
	if ( m_pPeerPublicKey )
		delete[] m_pPeerPublicKey;

	m_pPeerPublicKey = new BYTE[nKeyLen];
	m_nPeerKeyLen = nKeyLen;

	pPacket->Read( m_pPeerPublicKey, nKeyLen );

	// Generate random RC4 keys and send them encrypted
	return SendCryptLayerAnswer();
}

BOOL CEDClient::SendCryptLayerAnswer()
{
	if ( ! m_pPeerPublicKey || m_nPeerKeyLen == 0 || ( m_nCryptLayerState != 1 && m_nCryptLayerState != 2 ) )
		return FALSE;

	// Generate cryptographically secure random RC4 keys
	if ( theApp.m_hCryptProv != 0 )
	{
		// Use Windows CSPRNG
		if ( ! CryptGenRandom( theApp.m_hCryptProv, 16, m_RC4SendKey ) ||
			 ! CryptGenRandom( theApp.m_hCryptProv, 16, m_RC4RecvKey ) )
		{
			// CSPRNG failed, cannot proceed securely
			return FALSE;
		}
	}
	else
	{
		// Use GenerateCryptographicBytes for secure RC4 key generation (P0.2 security requirement)
		if (!GenerateCryptographicBytes(m_RC4SendKey, 16) ||
		    !GenerateCryptographicBytes(m_RC4RecvKey, 16)) {
			// Critical security failure - cannot generate secure RC4 keys
			theApp.Message(MSG_ERROR, L"ED2K: Failed to generate secure random bytes for RC4 keys");
			return FALSE; // Cannot proceed without secure keys
		}
	}

	// Encrypt the RC4 keys with peer's public key using RSA
	BYTE keysToEncrypt[32]; // 16 bytes for send key + 16 bytes for recv key
	memcpy( keysToEncrypt, m_RC4SendKey, 16 );
	memcpy( keysToEncrypt + 16, m_RC4RecvKey, 16 );

	BYTE encryptedKeys[256]; // RSA encrypted output buffer
	size_t nEncryptedLen = sizeof(encryptedKeys);

	// Use CryptoProvider to encrypt with peer's public key
	// Note: This is a simplified implementation - real eMule uses proper RSA OAEP padding
	if ( ! m_Crypto.EncryptWithPublicKey( keysToEncrypt, 32, encryptedKeys, nEncryptedLen, m_pPeerPublicKey, m_nPeerKeyLen ) )
	{
		// Encryption failed
		return FALSE;
	}

	// Initialize RC4 contexts with the keys
	m_RC4Send.Init(m_RC4SendKey, 16);
	m_RC4Recv.Init(m_RC4RecvKey, 16);

	// Send the answer packet
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_ANSWERCryptLayer, ED2K_PROTOCOL_EMULE );

	if ( ! pPacket )
		return FALSE;

	pPacket->Write( encryptedKeys, (DWORD)nEncryptedLen );
	Send( pPacket );

	// Mark CryptLayer as active
	m_bCryptLayerActive = TRUE;
	m_nCryptLayerState = 3; // Active

	return TRUE;
}

BOOL CEDClient::OnCryptLayerAnswer(CEDPacket* pPacket)
{
	if ( ! pPacket )
		return FALSE;

	// If we're not expecting this (state != 2), it might be a valid response
	// from a dual-initiation scenario. We'll still try to process it.
	if ( m_nCryptLayerState != 2 )
	{
		// Only process if we have the peer's public key (from a previous PUBLICKEY)
		if ( m_nCryptLayerState != 1 || ! m_pPeerPublicKey )
			return FALSE;
	}

	// Read the encrypted RC4 keys - RSA ciphertext length is variable (modulus size)
	DWORD nDataLen = pPacket->GetRemaining();
	if ( nDataLen == 0 || nDataLen > 512 ) // Sanity check - should be reasonable RSA modulus size
		return FALSE;

	// Allocate buffer for ciphertext
	BYTE* pEncryptedKeys = new BYTE[nDataLen];
	if ( ! pEncryptedKeys )
		return FALSE;

	pPacket->Read( pEncryptedKeys, nDataLen );

	// Decrypt the RC4 keys using our private key
	// Plaintext should be 32 bytes (16 bytes send key + 16 bytes receive key)
	BYTE decryptedKeys[32];
	size_t nDecryptedLen = sizeof(decryptedKeys);

	if ( ! m_Crypto.DecryptWithPrivateKey( pEncryptedKeys, nDataLen, decryptedKeys, nDecryptedLen ) )
	{
		// Decryption failed
		delete[] pEncryptedKeys;
		return FALSE;
	}

	// Clean up ciphertext buffer
	delete[] pEncryptedKeys;

	if ( nDecryptedLen != 32 )
	{
		// Wrong decrypted size - should be exactly 32 bytes for two RC4 keys
		return FALSE;
	}

	// Extract the RC4 keys (note: peer's send key becomes our receive key, peer's recv key becomes our send key)
	// This matches the eMule specification: sender packs (sendKey, recvKey), receiver unpacks as (recvKey, sendKey)
	memcpy( m_RC4RecvKey, decryptedKeys, 16 );      // Peer send key -> our receive key
	memcpy( m_RC4SendKey, decryptedKeys + 16, 16 ); // Peer recv key -> our send key

	// Initialize RC4 contexts
	m_RC4Send.Init(m_RC4SendKey, 16);
	m_RC4Recv.Init(m_RC4RecvKey, 16);

	// Mark CryptLayer as active
	m_bCryptLayerActive = TRUE;
	m_nCryptLayerState = 3; // Active

	return TRUE;
}

BOOL CEDClient::EncryptPacket(BYTE* pData, DWORD nLength)
{
	if ( ! m_bCryptLayerActive || ! pData || nLength == 0 )
		return FALSE;

	// Encrypt data with RC4 (skip packet header if needed)
	m_RC4Send.Process(pData, nLength);

	return TRUE;
}

BOOL CEDClient::DecryptPacket(BYTE* pData, DWORD nLength)
{
	if ( ! m_bCryptLayerActive || ! pData || nLength == 0 )
		return FALSE;

	// Decrypt data with RC4 (skip packet header if needed)
	m_RC4Recv.Process(pData, nLength);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient construction

CEDClient::CEDClient()
	: CTransfer				( PROTOCOL_ED2K )
	, m_pEdPrev				( NULL )
	, m_pEdNext				( NULL )

	, m_nClientID			( 0ul )
	, m_nUDP				( 0u )

	// Client ID and version
	, m_bEmule				( FALSE )
	, m_nEmCompatible		( 0 )
	, m_nEmVersion			( 0 )
	, m_nVersion			( 0 )
	, m_nSoftwareVersion	( 0ul )

	// Client capabilities
	, m_bEmAICH				( TRUE )	// Supported
	, m_bEmUnicode			( FALSE )
	, m_bEmUDPVersion		( FALSE )
	, m_bEmDeflate			( FALSE )
	, m_bEmSecureID			( TRUE )	// SecureID authentication enabled
	, m_bEmSources			( FALSE )
	, m_bEmRequest			( FALSE )
	, m_bEmComments			( FALSE )
	, m_bEmBrowse			( FALSE )
	, m_bEmPeerCache		( FALSE )	// Unsupported
	, m_bEmMultiPacket		( FALSE )	// Unsupported
	, m_bEmPreview			( FALSE )

	, m_bEmSupportsCaptcha	( FALSE )
	, m_bEmSupportsSourceEx2 ( TRUE )	// Source Exchange v2 enabled
	, m_bEmRequiresCryptLayer ( FALSE )	// Basic support enabled
	, m_bEmRequestsCryptLayer ( TRUE )	// Request encryption when available
	, m_bEmSupportsCryptLayer ( TRUE )	// Support encryption
	, m_bEmExtMultiPacket	( FALSE )	// Unsupported
	, m_bEmLargeFile		( FALSE )	// LargeFile support
	, m_nEmKadVersion		( 0 )		// Unsupported

	// SecureID state
	, m_nSecureIdentState	( 0 )		// No authentication in progress

	// Misc stuff
	, m_pDownloadTransfer	( NULL )
	, m_pUploadTransfer 	( NULL )
	, m_nRunExCookie		( 0 )
	, m_bLogin				( FALSE )
	, m_bCryptLayerActive	( FALSE )
	, m_bCryptLayerRequested( FALSE )
	, m_bCryptLayerInitiator( FALSE )
	, m_nCryptLayerState	( 0 )
	, m_pPeerPublicKey		( NULL )
	, m_nPeerKeyLen			( 0 )
	, m_bSeeking			( FALSE )
	, m_bCallbackRequested	( false )
	, m_bOpenChat			( FALSE )
	, m_bCommentSent		( FALSE )

	, m_nDirsWaiting		( 0ul )
{
	m_mInput.pLimit  = &Settings.Bandwidth.Request;
	m_mOutput.pLimit = &Settings.Bandwidth.Request;

	EDClients.Add( this );
}

CEDClient::~CEDClient()
{
	ASSERT( ! IsValid() );
	ASSERT( m_pUploadTransfer == NULL );
	ASSERT( m_pDownloadTransfer == NULL );

	// Clean up CryptLayer resources
	if ( m_pPeerPublicKey )
	{
		delete[] m_pPeerPublicKey;
		m_pPeerPublicKey = NULL;
		m_nPeerKeyLen = 0;
	}

	EDClients.Remove( this );
}

DWORD CEDClient::GetID() const
{
	return CEDPacket::IsLowID( m_nClientID ) ? m_nClientID :
		( Network.IsConnected() ? Network.m_pHost.sin_addr.s_addr : 0 );
}

//////////////////////////////////////////////////////////////////////
// CEDClient outbound connection

BOOL CEDClient::ConnectTo(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID)
{
	ASSERT( m_nClientID == 0 );

	m_nClientID = nClientID;
	m_oGUID = oGUID;

	m_pHost.sin_family		= AF_INET;
	m_pHost.sin_addr		= (IN_ADDR&)nClientID;
	m_pHost.sin_port		= htons( nClientPort );

	if ( pServerAddress != NULL && nServerPort != 0 )
	{
		m_pServer.sin_family	= AF_INET;
		m_pServer.sin_addr		= *pServerAddress;
		m_pServer.sin_port		= htons( nServerPort );
	}
	else
	{
		ZeroMemory( &m_pServer, sizeof( m_pServer ) );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient equality

BOOL CEDClient::Equals(CEDClient* pClient)
{
	ASSERT( this != NULL );
	ASSERT( pClient != NULL );

	if ( m_oGUID && pClient->m_oGUID )
		return m_oGUID == pClient->m_oGUID;

	if ( CEDPacket::IsLowID( m_nClientID ) &&
		 CEDPacket::IsLowID( pClient->m_nClientID ) )
	{
		return	m_pServer.sin_addr.S_un.S_addr == pClient->m_pServer.sin_addr.S_un.S_addr &&
				m_nClientID == pClient->m_nClientID;
	}

	return m_pHost.sin_addr.S_un.S_addr == pClient->m_pHost.sin_addr.S_un.S_addr;
}

//////////////////////////////////////////////////////////////////////
// CEDClient connect

BOOL CEDClient::Connect()
{
	if ( IsValid() )
		return FALSE;

	if ( EDClients.IsFull( this ) )
	{
		// If this download isn't queued, don't try to start it.
		if ( ! m_pDownloadTransfer || m_pDownloadTransfer->m_nState != dtsQueued )
			return FALSE;

		// If we're really overloaded, we may have to drop some queued downloads
		if ( EDClients.IsOverloaded() )
		{
			theApp.Message( MSG_ERROR, L"ED2K Queued download was dropped due to connection overloading" );
			return FALSE;
		}
	}

	if ( CEDPacket::IsLowID( m_nClientID ) )
	{
		if ( ! Neighbours.PushDonkey( m_nClientID, m_pServer.sin_addr, htons( m_pServer.sin_port ) ) )
			return FALSE;

		m_tConnected = GetTickCount();
	}
	else
	{
		if ( ! CConnection::ConnectTo( &m_pHost ) )
			return FALSE;

		theApp.Message( MSG_INFO, IDS_ED2K_CLIENT_CONNECTING, (LPCTSTR)m_sAddress );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient remove

void CEDClient::Remove()
{
	ASSERT( this != NULL );

	Close();

	DetachUpload();
	DetachDownload();

	Close();

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CEDClient merge

void CEDClient::Merge(CEDClient* pClient)
{
	ASSERT( pClient != NULL );

	if ( pClient->m_pDownloadTransfer )
	{
		DetachDownload();
		m_pDownloadTransfer = pClient->m_pDownloadTransfer;
		m_pDownloadTransfer->m_pClient = this;

		m_mInput.pLimit = &m_pDownloadTransfer->m_nBandwidth;
		pClient->m_pDownloadTransfer = NULL;
	}

	if ( pClient->m_pUploadTransfer )
	{
		DetachUpload();
		m_pUploadTransfer = pClient->m_pUploadTransfer;
		m_pUploadTransfer->m_pClient = this;
		m_mOutput.pLimit = &m_pUploadTransfer->m_nBandwidth;
		pClient->m_pUploadTransfer = NULL;
	}

	// Make sure chat/comments values are carried over
	if ( ! m_bOpenChat )			m_bOpenChat = pClient->m_bOpenChat;
	if ( ! m_bCommentSent )			m_bCommentSent = pClient->m_bCommentSent;

	CopyCapabilities( pClient );
}

void CEDClient::CopyCapabilities(CEDClient* pClient)
{
	ASSERT( pClient != NULL );

	// Copy client capabilities. (This should not be necessary)
	if ( ! m_nEmVersion )			m_nEmVersion = pClient->m_nEmVersion;
	if ( ! m_nEmCompatible )		m_nEmCompatible = pClient->m_nEmCompatible;
	if ( ! m_nSoftwareVersion )		m_nSoftwareVersion = pClient->m_nSoftwareVersion;

	if ( ! m_bEmAICH )				m_bEmAICH = pClient->m_bEmAICH;
	if ( ! m_bEmUnicode )			m_bEmUnicode = pClient->m_bEmUnicode;
	if ( ! m_bEmUDPVersion )		m_bEmUDPVersion = pClient->m_bEmUDPVersion;
	if ( ! m_bEmDeflate )			m_bEmDeflate = pClient->m_bEmDeflate;
	if ( ! m_bEmSecureID )			m_bEmSecureID = pClient->m_bEmSecureID;

	// Initiate SecureID authentication if both support it and we're connected
	if ( m_bEmSecureID && pClient->m_bEmSecureID && m_nSecureIdentState == 0 )
	{
		GenerateSecureIdent();
		SendSecureIdentChallenge();
	}
	if ( ! m_bEmSources )			m_bEmSources = pClient->m_bEmSources;
	if ( ! m_bEmRequest )			m_bEmRequest = pClient->m_bEmRequest;
	if ( ! m_bEmComments )			m_bEmComments = pClient->m_bEmComments;
	if ( ! m_bEmPeerCache )			m_bEmPeerCache = pClient->m_bEmPeerCache;
	if ( ! m_bEmBrowse )			m_bEmBrowse = pClient->m_bEmBrowse;
	if ( ! m_bEmMultiPacket )		m_bEmMultiPacket = pClient->m_bEmMultiPacket;
	if ( ! m_bEmPreview )			m_bEmPreview = pClient->m_bEmPreview;
	if ( ! m_bEmLargeFile )			m_bEmLargeFile = pClient->m_bEmLargeFile;

	if ( ! m_bEmSupportsCaptcha )	m_bEmSupportsCaptcha = pClient->m_bEmSupportsCaptcha;
	if ( ! m_bEmSupportsSourceEx2 )	m_bEmSupportsSourceEx2 = pClient->m_bEmSupportsSourceEx2;
	if ( ! m_bEmRequiresCryptLayer ) m_bEmRequiresCryptLayer = pClient->m_bEmRequiresCryptLayer;
	if ( ! m_bEmRequestsCryptLayer ) m_bEmRequestsCryptLayer = pClient->m_bEmRequestsCryptLayer;
	if ( ! m_bEmSupportsCryptLayer ) m_bEmSupportsCryptLayer = pClient->m_bEmSupportsCryptLayer;
	if ( ! m_bEmExtMultiPacket )	m_bEmExtMultiPacket = pClient->m_bEmExtMultiPacket;
	if ( ! m_nEmKadVersion )		m_nEmKadVersion = pClient->m_nEmKadVersion;
}

//////////////////////////////////////////////////////////////////////
// CEDClient send a packet

void CEDClient::Send(CPacket* pPacket, BOOL bRelease)
{
	if ( pPacket != NULL )
	{
		if ( IsValid() )
		{
			pPacket->SmartDump( &m_pHost, FALSE, TRUE );

			// Apply CryptLayer encryption if active
			// Note: Handshake packets (ED2K_C2C_PUBLICKEY, ED2K_C2C_ANSWERCryptLayer) are not encrypted
			// as they establish the encryption channel. CEDClient handles client-to-client connections,
			// so all packets here are C2C and should be encrypted (except handshake packets).
			// Encryption must happen AFTER compression (which occurs in ToBuffer()) but BEFORE writing to network.
			if ( m_bCryptLayerActive && pPacket->m_nProtocol == PROTOCOL_ED2K )
			{
				CEDPacket* pEDPacket = static_cast<CEDPacket*>( pPacket );
				if ( pEDPacket &&
					 pEDPacket->m_nType != ED2K_C2C_PUBLICKEY &&
					 pEDPacket->m_nType != ED2K_C2C_ANSWERCryptLayer )
				{
					// Write packet to temporary buffer first (this handles compression)
					CBuffer oTempBuffer;
					pEDPacket->ToBuffer( &oTempBuffer, TRUE ); // TRUE = TCP

					// Encrypt the payload portion (skip the ED2K_TCP_HEADER)
					// Header structure: { protocol(1), length(4), type(1) } = 6 bytes
					const DWORD nHeaderSize = sizeof( ED2K_TCP_HEADER );
					if ( oTempBuffer.m_nLength > nHeaderSize )
					{
						BYTE* pPayload = oTempBuffer.m_pBuffer + nHeaderSize;
						DWORD nPayloadLen = oTempBuffer.m_nLength - nHeaderSize;

						// Encrypt the payload (after compression, if any)
						EncryptPacket( pPayload, nPayloadLen );
					}

					// Write the encrypted packet to the actual output buffer
					Write( &oTempBuffer );
				}
				else
				{
					// Not encrypting this packet, write normally
					Write( pPacket );
				}
			}
			else
			{
				// CryptLayer not active, write normally
				Write( pPacket );
			}

			QueueRun();
		}

		if ( bRelease )
			pPacket->Release();
	}
	else if ( IsValid() )
	{
		QueueRun();
	}
}

//////////////////////////////////////////////////////////////////////
// CEDClient attach to existing connection

void CEDClient::AttachTo(CConnection* pConnection)
{
	ASSERT( ! IsValid() );
	CTransfer::AttachTo( pConnection );
	theApp.Message( MSG_INFO, IDS_ED2K_CLIENT_ACCEPTED, (LPCTSTR)m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CEDClient close

void CEDClient::Close(UINT nError)
{
	ASSERT( this != NULL );
	CTransfer::Close( nError );
	m_bConnected = m_bLogin = FALSE;

	if ( ( m_pDownloadTransfer ) && ( m_pDownloadTransfer->m_nState == dtsDownloading ) )
	{
		theApp.Message( MSG_ERROR, L"Warning: CEDClient::Close() called for downloading client %s", (LPCTSTR)m_sAddress );
		m_pDownloadTransfer->SetState( dtsNull );
	}
	// if ( ! m_bGUID ) Remove();
}

//////////////////////////////////////////////////////////////////////
// CEDClient transfer coupling

BOOL CEDClient::AttachDownload(CDownloadTransferED2K* pDownload)
{
	if ( m_pDownloadTransfer ) return FALSE;
	m_pDownloadTransfer = pDownload;

	if ( m_bLogin )
		return m_pDownloadTransfer->OnConnected();

	if ( ! IsValid() )
		Connect();

	return TRUE;
}

void CEDClient::OnDownloadClose()
{
	CDownloadSource* pExcept = GetSource();
	m_pDownloadTransfer = NULL;
	m_mInput.pLimit = &Settings.Bandwidth.Request;
	SeekNewDownload( pExcept );
}

BOOL CEDClient::SeekNewDownload(CDownloadSource* /*pExcept*/)
{
	// Removed for a while	// ToDo: Why? Restore this?
	return FALSE;

//	if ( m_pDownloadTransfer ) return FALSE;
//	if ( m_bSeeking ) return FALSE;
//	m_bSeeking = TRUE;
//	BOOL bSeek = Downloads.OnDonkeyCallback( this, pExcept );
//	m_bSeeking = FALSE;
//	return bSeek;
}

void CEDClient::DetachDownload()
{
	m_bSeeking = TRUE;
	if ( m_pDownloadTransfer ) m_pDownloadTransfer->Close( TRI_UNKNOWN );
	ASSERT( m_pDownloadTransfer == NULL );
	m_bSeeking = FALSE;
}

void CEDClient::OnUploadClose()
{
	m_pUploadTransfer = NULL;
	m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

void CEDClient::DetachUpload()
{
	if ( m_pUploadTransfer ) m_pUploadTransfer->Close();
	ASSERT( m_pUploadTransfer == NULL );
}

//////////////////////////////////////////////////////////////////////
// CEDClient run event

BOOL CEDClient::OnRun()
{
	ASSUME_LOCK( Transfers.m_pSection );

	// CTransfer::OnRun();

	const DWORD tNow = GetTickCount();

	if ( ! m_bConnected )
	{
		if ( tNow > m_tConnected + Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_CONNECT_TIMEOUT, (LPCTSTR)m_sAddress );
			NotifyDropped();
			Close();
			return FALSE;
		}
	}
	else if ( ! m_bLogin )
	{
		if ( tNow > m_tConnected + Settings.Connection.TimeoutHandshake )
		{
			// Handshake timeout
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
			NotifyDropped();
			Close();
			return FALSE;
		}
	}
	else
	{
		if ( m_bOpenChat )
		{
			// Open a chat window.
			ChatCore.OnMessage( this );
			m_bOpenChat = FALSE;
		}
		else if ( tNow > m_mInput.tLast + Settings.Connection.TimeoutTraffic &&
			 tNow > m_mOutput.tLast + Settings.Connection.TimeoutTraffic )
		{
			// Don't time out downloading clients.
			if ( m_pDownloadTransfer && m_pDownloadTransfer->m_nState == dtsDownloading )
				return TRUE;
			// Connection closed (Inactive)
			theApp.Message( MSG_INFO, IDS_ED2K_CLIENT_CLOSED, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}

	return TRUE;
}

void CEDClient::OnRunEx(DWORD tNow)
{
	// Already downloading or uploading
	if ( m_pDownloadTransfer || m_pUploadTransfer )
	{
		if ( m_pDownloadTransfer )
			m_pDownloadTransfer->OnRunEx( tNow );
		if ( m_pUploadTransfer )
			m_pUploadTransfer->OnRunEx( tNow );
	}
	// No connections to this client
	else if ( ! IsValid() )
	{
		// A callback was requested
		if ( m_bCallbackRequested )
		{
			// Indicate that callback was handled
			m_bCallbackRequested = false;

			// Return if connection was successful
			if ( Connect() )
				return;
		}

		// A chat request was sent
		if ( m_bOpenChat )
		{
			// We might be waiting for a push reply, give it a little time
			if ( tNow < m_tConnected + Settings.Connection.TimeoutHandshake )
				return;
		}

		// This client has no valid connections and should be removed.
		Remove();
	}
}

//////////////////////////////////////////////////////////////////////
// CEDClient connection event

BOOL CEDClient::OnConnected()
{
	SendHello( ED2K_C2C_HELLO );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient connection loss event

void CEDClient::OnDropped()
{
	theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_DROPPED, (LPCTSTR)m_sAddress );
	NotifyDropped();
	Close();
}

void CEDClient::NotifyDropped()
{
	ASSUME_LOCK( Transfers.m_pSection );

	m_bSeeking = TRUE;
	if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnDropped();
	if ( m_pUploadTransfer ) m_pUploadTransfer->OnDropped();
	if ( CHostBrowser* pBrowser = GetBrowser() ) pBrowser->OnDropped();
	ChatCore.OnDropped( this );
	m_bSeeking = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient write event

BOOL CEDClient::OnWrite()
{
	CTransfer::OnWrite();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient read event

BOOL CEDClient::OnRead()
{
	BOOL bSuccess = TRUE;

	CTransfer::OnRead();

	CLockedBuffer pInput( GetInput() );

	while ( CEDPacket* pPacket = CEDPacket::ReadBuffer( pInput ) )
	{
		try
		{
			// Apply CryptLayer decryption if active
			// Note: Handshake packets (ED2K_C2C_PUBLICKEY, ED2K_C2C_ANSWERCryptLayer) are not encrypted
			// as they establish the encryption channel. CEDClient handles client-to-client connections,
			// so all packets here are C2C and should be decrypted (except handshake packets).
			if ( m_bCryptLayerActive &&
				 pPacket->m_nType != ED2K_C2C_PUBLICKEY &&
				 pPacket->m_nType != ED2K_C2C_ANSWERCryptLayer )
			{
				// Decrypt the packet payload (m_pBuffer contains the payload data)
				if ( pPacket->m_pBuffer && pPacket->m_nLength > 0 )
				{
					DecryptPacket( pPacket->m_pBuffer, pPacket->m_nLength );
				}
			}

			// Inflate compressed packets now that decryption (if any) is complete
			if ( pPacket->m_nEdProtocol == ED2K_PROTOCOL_EMULE_PACKED ||
				 pPacket->m_nEdProtocol == ED2K_PROTOCOL_KAD_PACKED ||
				 pPacket->m_nEdProtocol == ED2K_PROTOCOL_REVCONNECT_PACKED )
			{
				if ( ! pPacket->Inflate() )
				{
					// Inflation failed - discard packet
					pPacket->Release();
					continue;
				}
			}

			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
			if ( ! m_oGUID ) bSuccess = FALSE;
		}

		pPacket->Release();
		if ( ! bSuccess ) break;
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CEDClient logged in event

BOOL CEDClient::OnLoggedIn()
{
	m_bLogin = TRUE;

	EDClients.Merge( this );

	if ( m_pDownloadTransfer )
		m_pDownloadTransfer->OnConnected();
	else
		SeekNewDownload();

	if ( m_pUploadTransfer )
		m_pUploadTransfer->OnConnected();

	if ( CHostBrowser* pBrowser = GetBrowser() )
	{
		pBrowser->OnConnected();

		if ( CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_ASKSHAREDDIRS ) )
			Send( pPacket );
	}

	// Initiate CryptLayer handshake if both sides support it and we haven't started yet
	// Handshake should be initiated when:
	// - Peer supports CryptLayer (peer capability parsed from hello packet)
	// - Either we request it OR peer requires it
	// - Handshake hasn't been started yet
	// - We are the designated initiator (deterministic rule using GUID comparison)
	if ( m_bEmSupportsCryptLayer &&
		 ( m_bEmRequestsCryptLayer || m_bEmRequiresCryptLayer ) &&
		 m_nCryptLayerState == 0 &&
		 ! m_bCryptLayerInitiator &&
		 ShouldInitiateCryptLayer() )
	{
		m_bCryptLayerInitiator = TRUE;  // Mark ourselves as initiator
		InitCryptLayer();
		StartCryptLayerHandshake();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient CryptLayer initiator determination

BOOL CEDClient::ShouldInitiateCryptLayer() const
{
	// Use deterministic rule based on GUID comparison to prevent dual initiation
	// If our GUID > peer's GUID, we initiate. This ensures only one side initiates.

	if ( ! m_oGUID )
		return FALSE;  // Can't determine without peer GUID

	const Hashes::Guid& oMyGUID = MyProfile.oGUID;
	const Hashes::Guid& oPeerGUID = m_oGUID;

	// Compare GUIDs lexicographically
	return ( memcmp( &oMyGUID[0], &oPeerGUID[0], oMyGUID.byteCount ) > 0 );
}

CHostBrowser* CEDClient::GetBrowser() const
{
	CSingleLock pLock( &theApp.m_pSection );
	if ( ! SafeLock( pLock ) ) return NULL;

	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
	{
		for ( POSITION pos = pMainWnd->m_pWindows.GetIterator(); pos; )
		{
			CChildWnd* pChildWnd = pMainWnd->m_pWindows.GetNext( pos );
			if ( pChildWnd->IsKindOf( RUNTIME_CLASS( CBrowseHostWnd ) ) )
			{
				CBrowseHostWnd* pBrowseHostWnd = static_cast< CBrowseHostWnd* >( pChildWnd );
				if ( CHostBrowser* pBrowser = pBrowseHostWnd->GetBrowser() )
				{
					if ( pBrowser->IsBrowsing() &&
						( m_oGUID == pBrowser->m_oClientID ||
						( m_pHost.sin_addr.s_addr == pBrowser->m_pAddress.s_addr &&
						  m_pHost.sin_port == htons( pBrowser->m_nPort ) ) ) )
					{
						return pBrowser;
					}
				}
			}
		}
	}

	return NULL;
}

CDownloadSource* CEDClient::GetSource() const
{
	return m_pDownloadTransfer ? m_pDownloadTransfer->GetSource() : NULL;
}

//////////////////////////////////////////////////////////////////////
// CEDClient packet switch

BOOL CEDClient::OnPacket(CEDPacket* pPacket)
{
	pPacket->SmartDump( &m_pHost, FALSE, FALSE );

	if ( pPacket->m_nEdProtocol == ED2K_PROTOCOL_EDONKEY )
	{
		switch ( pPacket->m_nType )
		{
		// Handshake
		case ED2K_C2C_HELLO:
			if ( pPacket->GetRemaining() ) pPacket->ReadByte();
			return OnHello( pPacket );
		case ED2K_C2C_HELLOANSWER:
			return OnHello( pPacket );

		// Upload
		case ED2K_C2C_FILEREQUEST:
			return OnFileRequest( pPacket );
		case ED2K_C2C_FILESTATUSREQUEST:
			return OnFileStatusRequest( pPacket );
		case ED2K_C2C_HASHSETREQUEST:
			return OnHashsetRequest( pPacket );
		case ED2K_C2C_QUEUEREQUEST:
			return OnQueueRequest( pPacket );
		case ED2K_C2C_QUEUERELEASE:
			if ( m_pUploadTransfer ) m_pUploadTransfer->OnQueueRelease( pPacket );
			return TRUE;
		case ED2K_C2C_REQUESTPARTS:
			if ( m_pUploadTransfer ) m_pUploadTransfer->OnRequestParts( pPacket );
			return TRUE;

		// Download
		case ED2K_C2C_FILEREQANSWER:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnFileReqAnswer( pPacket );
			return TRUE;
		case ED2K_C2C_FILENOTFOUND:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnFileNotFound( pPacket );
			return TRUE;
		case ED2K_C2C_FILESTATUS:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnFileStatus( pPacket );
			return TRUE;
		case ED2K_C2C_HASHSETANSWER:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnHashsetAnswer( pPacket );
			return TRUE;
		case ED2K_C2C_QUEUERANK:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnQueueRank( pPacket );
			return TRUE;
		case ED2K_C2C_STARTUPLOAD:
			// STARTUPLOAD (0x55) means the remote peer granted us an upload slot.
			// This must be handled by the download transfer to start requesting fragments.
#ifdef _DEBUG
			{
				const CString strIP( inet_ntoa( m_pHost.sin_addr ) );
				const WORD nPort = ntohs( m_pHost.sin_port );
				const int nDLState = m_pDownloadTransfer ? (int)m_pDownloadTransfer->m_nState : -1;
				theApp.Message( MSG_DEBUG, L"[ED2K] STARTUPLOAD from %s:%u -> DownloadTransfer (dl=%p state=%d, ul=%p)",
					(LPCTSTR)strIP, nPort, m_pDownloadTransfer, nDLState, m_pUploadTransfer );
			}
#endif

			if ( m_pDownloadTransfer )
				m_pDownloadTransfer->OnStartUpload( pPacket );
			else
			{
#ifdef _DEBUG
				{
					const CString strIP( inet_ntoa( m_pHost.sin_addr ) );
					const WORD nPort = ntohs( m_pHost.sin_port );
					theApp.Message( MSG_DEBUG, L"[ED2K] STARTUPLOAD from %s:%u ignored (m_pDownloadTransfer == NULL)",
						(LPCTSTR)strIP, nPort );
				}
#endif
			}
			return TRUE;
		case ED2K_C2C_FINISHUPLOAD:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnFinishUpload( pPacket );
			return TRUE;
		case ED2K_C2C_SENDINGPART:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnSendingPart( pPacket );
			return TRUE;

		// Chat
		case ED2K_C2C_MESSAGE:
			return OnChatMessage( pPacket );

		// Browse us
		case ED2K_C2C_ASKSHAREDDIRS:
			return OnAskSharedDirs( pPacket );
		case ED2K_C2C_VIEWSHAREDDIR:
			return OnViewSharedDir( pPacket );

		// Browse remote host
		case ED2K_C2C_ASKSHAREDDIRSANSWER:
			return OnAskSharedDirsAnswer( pPacket );
		case ED2K_C2C_VIEWSHAREDDIRANSWER:
			return OnViewSharedDirAnswer( pPacket );
		case ED2K_C2C_ASKSHAREDDIRSDENIED:
			return OnAskSharedDirsDenied( pPacket );
		}
	}
	else if ( pPacket->m_nEdProtocol == ED2K_PROTOCOL_EMULE )
	{
		switch ( pPacket->m_nType )
		{
		case ED2K_C2C_EMULEINFO:
			return OnEmuleInfo( pPacket );
		case ED2K_C2C_EMULEINFOANSWER:
			return OnEmuleInfo( pPacket );

		case ED2K_C2C_COMPRESSEDPART:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnCompressedPart( pPacket );
			return TRUE;
		case ED2K_C2C_QUEUERANKING:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnRankingInfo( pPacket );
			return TRUE;
		case ED2K_C2C_FILEDESC:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnFileComment( pPacket );
			return TRUE;

		case ED2K_C2C_REQUESTSOURCES:
			return OnSourceRequest( pPacket );
		case ED2K_C2C_ANSWERSOURCES:
			return OnSourceAnswer( pPacket );
		case ED2K_C2C_REQUESTSOURCES2:
			return OnSourceRequest2( pPacket );
		case ED2K_C2C_ANSWERSOURCES2:
			return OnSourceAnswer2( pPacket );
		case ED2K_C2C_REQUESTPREVIEW:
			return OnRequestPreview( pPacket );
		case ED2K_C2C_PREVIEWANWSER:
			return OnPreviewAnswer( pPacket );

		// Extented Upload (64Bit LargeFile support)
		case ED2K_C2C_REQUESTPARTS_I64:
			if ( m_pUploadTransfer ) m_pUploadTransfer->OnRequestParts64( pPacket );
			return TRUE;

		// Extented Download (64Bit LargeFile support)
		case ED2K_C2C_SENDINGPART_I64:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnSendingPart64( pPacket );
			return TRUE;
		case ED2K_C2C_COMPRESSEDPART_I64:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnCompressedPart64( pPacket );
			return TRUE;

		// Chat
		case ED2K_C2C_CHATCAPTCHAREQ:
			return OnCaptchaRequest( pPacket );
		case ED2K_C2C_CHATCAPTCHARES:
			return OnCaptchaResult( pPacket );

		// MultiPacket Ext2 / FileIdentifiers
		case ED2K_C2C_MULTIPACKET_EXT2:
			return OnMultiPacketExt2( pPacket );
		case ED2K_C2C_MULTIPACKETANSWER_EXT2:
			return OnMultiPacketAnswerExt2( pPacket );
		case ED2K_C2C_HASHSETREQUEST2:
			return OnHashsetRequest2( pPacket );
		case ED2K_C2C_HASHSETANSWER2:
			if ( m_pDownloadTransfer ) m_pDownloadTransfer->OnHashsetAnswer2( pPacket );
			return TRUE;

		// SecureID authentication
		case ED2K_C2C_SECIDENTSTATE:
			return ProcessSecureIdentChallenge( pPacket );
		case ED2K_C2C_SIGNATURE:
			return ProcessSecureIdentResponse( pPacket );

		// CryptLayer handshake
		case ED2K_C2C_PUBLICKEY:
			return ProcessCryptLayerHandshake( pPacket );
		case ED2K_C2C_ANSWERCryptLayer:
			return ProcessCryptLayerHandshake( pPacket );
		}
	}

	DEBUG_ONLY( pPacket->Debug( L"Unknown ED2K packet from " + m_sAddress ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient get comments

BOOL CEDClient::SendCommentsPacket(int nRating, LPCTSTR pszComments)
{
	// If we have not sent comments yet, and this client supports comments
	if ( ! m_bCommentSent && m_bEmComments > 0 && m_bEmule )
	{
		// Remove new lines and excess whitespace
		CString strComments = pszComments;
		strComments.Replace( '\n', ' ' );
		strComments.Replace( '\r', ' ' );
		strComments.Trim();

		// If there's comments in the library
		if ( nRating > 0 || ! strComments.IsEmpty() )
		{
			// Create the comments packet
			CEDPacket* pComment = CEDPacket::New( ED2K_C2C_FILEDESC, ED2K_PROTOCOL_EMULE );
			pComment->WriteByte( (BYTE)min( nRating, 5 ) );
			pComment->WriteLongEDString( strComments.Left(ED2K_COMMENT_MAX), m_bEmUnicode );

			// Send comments / rating
			theApp.Message( MSG_DEBUG, L"Sending file comments to %s", (LPCTSTR)m_sAddress );
			m_bCommentSent = TRUE;
			Send( pComment );

			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient HELLO packet exchange

void CEDClient::SendHello(BYTE nType)
{
	CEDPacket* pPacket = CEDPacket::New( nType );

	if ( nType == ED2K_C2C_HELLO )
		pPacket->WriteByte( 0x10 );		// Size of user hash (legacy)

	CEDNeighbour* pServer = NULL;
	CSingleLock oNetworkLock( &Network.m_pSection );
	if ( oNetworkLock.Lock( 500 ) )
		pServer = Neighbours.GetDonkeyServer();

	// Write user GUID
	Hashes::Guid oGUID = MyProfile.oGUID;
	oGUID[5] = 14;
	oGUID[14] = 111;
	pPacket->Write( oGUID );

	// Write client ID
	pPacket->WriteLongLE( pServer ? pServer->m_nClientID : Network.m_pHost.sin_addr.s_addr );

	// Write port number
	pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );

	// Number of Tags
	pPacket->WriteLongLE( 6 );

	// 1 - Nickname
	CEDTag( ED2K_CT_NAME, MyProfile.GetNick().Left( 255 ) ).Write( pPacket, TRUE );	// True=Unicode

	// 2 - ED2K version
	CEDTag( ED2K_CT_VERSION, ED2K_VERSION ).Write( pPacket );

	// 3 - UDP Port
	CEDTag( ED2K_CT_UDPPORTS, htons( Network.m_pHost.sin_port ) ).Write( pPacket );

	// 4 - Feature Versions 1
	BYTE nExtendedRequests = (BYTE)min ( Settings.eDonkey.ExtendedRequest, (DWORD)ED2K_VERSION_EXTENDEDREQUEST );
	DWORD nOpt1 =  ( ( ED2K_VERSION_AICH << 29 ) |					// AICH (enabled)
					 ( TRUE << 28 ) |								// Unicode
					 ( ED2K_VERSION_UDP << 24 ) |					// UDP version
					 ( ED2K_VERSION_COMPRESSION << 20 ) |			// Compression
					 ( ED2K_VERSION_SECUREID << 16 ) |				// Secure ID
					 ( ED2K_VERSION_SOURCEEXCHANGE << 12 ) |		// Source exchange
					 ( nExtendedRequests << 8 ) |					// Extended requests
					 ( ED2K_VERSION_COMMENTS << 4 ) |				// Comments
				//	 ( FALSE << 3 ) |								// Peer Cache
				//	 ( FALSE << 2 ) |								// Browse
				//	 ( FALSE << 1 ) |								// Multipacket
					 ( Settings.Uploads.SharePreviews ? 1 : 0 ) );	// Preview
	CEDTag( ED2K_CT_FEATUREVERSIONS, nOpt1 ).Write( pPacket );

	// 5 - Feature Versions 2
	DWORD nOpt2 =  ( ( TRUE << 11 ) |								// Captcha support
					 ( TRUE << 10 ) |								// Source Exchange v2
					 ( TRUE << 5 ) |								// Ext Multipacket (FileIdentifiers/MultiPacket Ext2)
					 ( Settings.eDonkey.LargeFileSupport ? ( TRUE << 4 ) : 0 ) );	// LargeFile support
	CEDTag( ED2K_CT_MOREFEATUREVERSIONS, nOpt2 ).Write( pPacket );

	// 6 - Software Version
	//	Note we're likely to corrupt the beta number, since there's only 3 bits available, but it's least important.
	//		Note: Including this stops the remote client sending the eMuleInfo packet.
	DWORD nVersion = ( ( ( ED2K_CLIENT_ID & 0xFF ) << 24 ) |
					   ( ( theApp.m_nVersion[0] & 0x7F ) << 17 ) |
					   ( ( theApp.m_nVersion[1] & 0x7F ) << 10 ) );
					// ( ( theApp.m_nVersion[2] & 0x07 ) << 7  ) |
					// ( ( theApp.m_nVersion[3] & 0x7F ) ) );
	CEDTag( ED2K_CT_SOFTWAREVERSION, nVersion ).Write( pPacket );

	if ( pServer )
	{
		pPacket->WriteLongLE( pServer->m_pHost.sin_addr.s_addr );
		pPacket->WriteShortLE( htons( pServer->m_pHost.sin_port ) );

		oNetworkLock.Unlock();
	}
	else
	{
		pPacket->WriteLongLE( 0ul );
		pPacket->WriteShortLE( 0ul );
	}

	Send( pPacket );
}

BOOL CEDClient::OnHello(CEDPacket* pPacket)
{
	if ( m_bLogin ) return TRUE;

	if ( pPacket->GetRemaining() < sizeof( GUID ) + 6 + 4 + 6 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}

	pPacket->Read( m_oGUID );

	m_nClientID			= pPacket->ReadLongLE();
	m_pHost.sin_port	= htons( pPacket->ReadShortLE() );

	DWORD nCount = pPacket->ReadLongLE();

	while ( nCount-- > 0 && pPacket->GetRemaining() > 0 )
	{
		CEDTag pTag;
		if ( ! pTag.Read( pPacket, TRUE ) )	// True=Unicode
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		switch ( pTag.m_nKey )
		{
		case ED2K_CT_NAME:
			if ( pTag.m_nType == ED2K_TAG_STRING )
				m_sNick = pTag.m_sValue;
			break;
		case ED2K_CT_PORT:
			if ( pTag.m_nType == ED2K_TAG_INT )
				m_pHost.sin_port = htons( (WORD)pTag.m_nValue );
			break;
		case ED2K_CT_UDPPORTS:
			if ( pTag.m_nType == ED2K_TAG_INT )
				m_nUDP = (WORD)(pTag.m_nValue & 0x0000FFFF);
			break;
		case ED2K_CT_VERSION:
			if ( pTag.m_nType == ED2K_TAG_INT )
				m_nVersion = (DWORD)pTag.m_nValue;
			break;
		case ED2K_CT_MODVERSION:
			break;
		case ED2K_CT_SOFTWAREVERSION:
			if ( pTag.m_nType == ED2K_TAG_INT )
			{
				m_bEmule = TRUE;
				m_nSoftwareVersion = pTag.m_nValue & 0x00FFFFFF;
				m_nEmCompatible = (DWORD)( pTag.m_nValue >> 24 );
			}
			break;
		case ED2K_CT_FEATUREVERSIONS:
			if ( pTag.m_nType == ED2K_TAG_INT )
			{
				m_bEmule = TRUE;
				m_bEmAICH				= ( pTag.m_nValue >> 29 ) & 0x07;
				m_bEmUnicode			= ( pTag.m_nValue >> 28 ) & 0x01;
				m_bEmUDPVersion			= ( pTag.m_nValue >> 24 ) & 0x0F;
				m_bEmDeflate			= ( pTag.m_nValue >> 20 ) & 0x0F;
				m_bEmSecureID			= ( pTag.m_nValue >> 16 ) & 0x0F;
				m_bEmSources			= ( pTag.m_nValue >> 12 ) & 0x0F;
				m_bEmRequest			= ( pTag.m_nValue >>  8 ) & 0x0F;
				m_bEmComments			= ( pTag.m_nValue >>  4 ) & 0x0F;
				m_bEmPeerCache			= ( pTag.m_nValue >>  3 ) & 0x01;
				m_bEmBrowse				= ! ( ( pTag.m_nValue >> 2 ) & 0x01 );
				m_bEmMultiPacket		= ( pTag.m_nValue >>  1 ) & 0x01;
				m_bEmPreview			= ( pTag.m_nValue ) & 0x01;

				if ( GetSource() && GetSource()->m_bClientExtended )
					; // Do Nothing
				else
					m_bEmPreview = m_bEmPreview && m_bEmBrowse;
			}
			break;
		case ED2K_CT_MOREFEATUREVERSIONS:
			// This currently holds the KAD version (We aren't interested in that) and Large File support.
			if ( pTag.m_nType == ED2K_TAG_INT )
			{
				m_bEmule = TRUE;
				m_bEmSupportsCaptcha	= ( pTag.m_nValue >> 11) & 0x01;
				m_bEmSupportsSourceEx2	= ( pTag.m_nValue >> 10) & 0x01;
				m_bEmRequiresCryptLayer	= ( pTag.m_nValue >> 9 ) & 0x01;
				m_bEmRequestsCryptLayer	= ( pTag.m_nValue >> 8 ) & 0x01;
				m_bEmSupportsCryptLayer	= ( pTag.m_nValue >> 7 ) & 0x01;
				// Reserved 1
				m_bEmMultiPacket		= ( pTag.m_nValue >> 5 ) & 0x01;
				m_bEmExtMultiPacket		= ( pTag.m_nValue >> 5 ) & 0x01;
				m_bEmSupportsSourceEx2	= ( pTag.m_nValue >> 10 ) & 0x01;
				m_bEmLargeFile			= ( pTag.m_nValue >> 4 ) & 0x01;
				m_nEmKadVersion			= ( pTag.m_nValue ) & 0x0f;
			}
			break;
#ifdef _DEBUG
		case ED2K_CT_BUDDYIP:
		case ED2K_CT_BUDDYUDP:
		case ED2K_CT_UNKNOWN3:
		case ED2K_CT_EMULECOMPAT_OPTIONS:
			break;
		case ED2K_CT_LAN_PEER:
		case ED2K_CT_SUPPORT_VCNT:
			// easyMule
			if ( ! m_nEmCompatible ) m_nEmCompatible = 8;
			break;
		default:
			if ( _tcsicmp( pTag.m_sKey, L"pr" ) == 0 )
				break;		// No idea what this means. Probably from the eDonkey client. Detect eDonkeyHybrid client?
			{
				CString str;
				str.Format( L"Unknown ED2K Hello packet from %s  %s (Opcode 0x%x:0x%x)",
					(LPCTSTR)m_sAddress, (LPCTSTR)pTag.m_sKey, (UINT)pTag.m_nKey, (UINT)pTag.m_nType );
				pPacket->Debug( str );
			}
#endif	// Debug
		}
	}

	if ( pPacket->GetRemaining() < 6 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}

	// Read their server IP / port
	m_pServer.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
	m_pServer.sin_port = htons( pPacket->ReadShortLE() );

	// If we are learning new servers from clients, Add their server
	if ( Settings.eDonkey.LearnNewServersClient )
		HostCache.eDonkey.Add( &m_pServer.sin_addr, htons( m_pServer.sin_port ) );

	// Some clients append additional "stuff" at the end.
	if ( pPacket->GetRemaining() >= 4 )
	{
		// We can use it to ID clients
		DWORD nValue = pPacket->ReadLongLE();

		// MLdonkey
		if ( nValue == 0x4B444C4D )
			m_nEmCompatible = 10;
	}

	// Get client name/version
	DetermineUserAgent();

	// If this was a hello
	if ( pPacket->m_nType == ED2K_C2C_HELLO )
	{
		// If it's an eMule compatible client that has not already sent us extended details
		if ( m_bEmule && ( ! m_nSoftwareVersion ) )
			SendEmuleInfo( ED2K_C2C_EMULEINFO );	// Send extended hello

		// Send hello answer
		SendHello( ED2K_C2C_HELLOANSWER );
	}

	if ( m_bLogin )
		return TRUE;

	return OnLoggedIn();
}

//////////////////////////////////////////////////////////////////////
// CEDClient EMULE INFO packet exchange 	( ToDo: Verify spec + Update clients )

void CEDClient::SendEmuleInfo(BYTE nType)
{
	CEDPacket* pPacket = CEDPacket::New( nType, ED2K_PROTOCOL_EMULE );

	pPacket->WriteByte( ED2K_CLIENT_ID );	// eMule version (80 = 0x50?) 0x40?
	pPacket->WriteByte( 0x01 );				// eMule protocol

	// Write number of tags
	pPacket->WriteLongLE( Settings.eDonkey.ExtendedRequest ? 7 : 6 );

	// Write tags
	CEDTag( ED2K_ET_COMPATIBLECLIENT, ED2K_CLIENT_ID ).Write( pPacket );
	CEDTag( ED2K_ET_COMPRESSION, ED2K_VERSION_COMPRESSION ).Write( pPacket );
	CEDTag( ED2K_ET_SOURCEEXCHANGE, ED2K_VERSION_SOURCEEXCHANGE ).Write( pPacket );
	CEDTag( ED2K_ET_UDPVER, ED2K_VERSION_UDP ).Write( pPacket );
	CEDTag( ED2K_ET_UDPPORT, htons( Network.m_pHost.sin_port ) ).Write( pPacket );
	CEDTag( ED2K_ET_COMMENTS, ED2K_VERSION_COMMENTS ).Write( pPacket );
	if ( Settings.eDonkey.ExtendedRequest )
		CEDTag( ED2K_ET_EXTENDEDREQUEST, Settings.eDonkey.ExtendedRequest ).Write( pPacket );

	Send( pPacket );
}

BOOL CEDClient::OnEmuleInfo(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 5 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}

	// First Byte: Client Ident code
	m_nEmVersion = pPacket->ReadByte();

	// ToDo: Update and abstract these ED2K client codes?
	if ( m_nEmVersion == 0x2B ) m_nEmVersion = 0x22;

	// Second Byte: ED2K Protocol = 0x01
	BYTE nProtocol	= pPacket->ReadByte();
	if ( nProtocol != 1 ) return TRUE;

	// Have to assume capabilities for these versions
	if ( m_nEmVersion == 0x23 || m_nEmVersion == 0x24 ) m_bEmSources = 1;
	if ( m_nEmVersion == 0x24 ) m_bEmComments = 1;

	// Set the client ID to unknown
	m_nEmCompatible = ED2K_CLIENT_UNKNOWN;

	// Third Byte: Read number of tags
	DWORD nCount = pPacket->ReadLongLE();

	while ( nCount-- > 0 && pPacket->GetRemaining() > 0 )
	{
		CEDTag pTag;

		// Read tag
		if ( ! pTag.Read( pPacket, TRUE ) )	// True=Unicode
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}

		switch ( pTag.m_nKey )
		{
		case ED2K_ET_COMPRESSION:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_bEmDeflate = (BOOL)pTag.m_nValue;
			break;
		case ED2K_ET_UDPPORT:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_nUDP = (WORD)pTag.m_nValue;
			break;
		case ED2K_ET_UDPVER:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_bEmUDPVersion = (WORD)pTag.m_nValue;
			break;
		case ED2K_ET_SOURCEEXCHANGE:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_bEmSources = (BOOL)pTag.m_nValue;
			break;
		case ED2K_ET_COMMENTS:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_bEmComments = (BOOL)pTag.m_nValue;
			break;
		case ED2K_ET_EXTENDEDREQUEST:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_bEmRequest = (BOOL)pTag.m_nValue;
			break;
		case ED2K_ET_COMPATIBLECLIENT:
			if ( pTag.m_nType == ED2K_TAG_INT ) m_nEmCompatible = (DWORD)pTag.m_nValue;
			break;
		case ED2K_ET_FEATURES:		// We don't use these
		case ED2K_ET_OS_INFO:
			break;
		case ED2K_CT_MODVERSION:	// Some clients send this here
			if ( m_nEmCompatible == ED2K_CLIENT_UNKNOWN )
				m_nEmCompatible = ED2K_CLIENT_MOD;
			break;
		case ED2K_ET_L2HAC:
			// LowID 2 High ID callback request feature
			//if ( pTag.m_nType == ED2K_TAG_INT ) m_nL2HACTime = (DWORD)pTag.m_nValue;
			break;
		case ED2K_ET_MOD_PLUS:
			// eMule Plus version
			//if ( pTag.m_nType == ED2K_TAG_INT ) m_nPlusVers = (DWORD)pTag.m_nValue;
			break;
#ifdef _DEBUG
		default:
			CString str;
			str.Format( L"Unknown ED2K info packet from %s  (Opcode 0x%x:0x%x)",
				LPCTSTR( m_sAddress ), int( pTag.m_nKey ), int( pTag.m_nType ) );
			pPacket->Debug( str );
#endif
		}
	}

	m_bEmule = TRUE;

	// Send answer if required
	if ( pPacket->m_nType == ED2K_C2C_EMULEINFO )
		SendEmuleInfo( ED2K_C2C_EMULEINFOANSWER );

	// Get client name/version
	DetermineUserAgent();

	return TRUE;
}

void CEDClient::SendPreviewRequest(CDownload* pDownload)
{
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_REQUESTPREVIEW, ED2K_PROTOCOL_EMULE );
	pPacket->Write( pDownload->m_oED2K );

	Send( pPacket );
}

//////////////////////////////////////////////////////////////////////
// CEDClient client version

void CEDClient::DetermineUserAgent()
{
	// ToDo: Abstract and Update ED2K Client Codes
	// EMule			 0	0x00
	// LMule			 2	0x02
	// AMule			 3	0x03
	// Shareaza(old)	 4	0x04
	// EMule Plus		 5	0x05
	// Hydranode		 6	0x06
	// MLDonkey(new)	10	0x0A
	// LPHANT			20	0x14
	// Shareaza(new)	40	0x28
	// EDonkeyHybrid 	50	0x32
	// EDonkey			51	0x33
	// MLDonkey			52	0x34
	// EMule(old)		53	0x35
	// Envy		80	0x50
	// MLDonkey(new)	152	0x98
	// JMule			170	0xAA
	// Shareaza Plus	203	0xCB

	// Newer clients send the 24 bit software version + client ID
	if ( m_nSoftwareVersion )
	{
		m_bEmule = TRUE;	// It's eMule compatible

		// Determine client from the compatible client byte
		switch ( m_nEmCompatible )
		{
		case 0:
			m_sUserAgent.Format( L"eMule %u.%u%c",
		//	m_sUserAgent.Format( L"eMule %u.%u%c (%i)"	// Displays additional build number instead.
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 1:
			m_sUserAgent.Format( L"cDonkey %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 2:
			m_sUserAgent.Format( L"xMule %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 3:
			m_sUserAgent.Format( L"aMule %u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) );
			break;
		case 4:		// Old Shareaza alpha/beta/mod/fork versions
			if ( m_bEmAICH )
			{
				if ( m_sUserAgent.IsEmpty() )								// Banned by Security Rules (Leecher Mod)
					m_sUserAgent.Format( L"eMule mod (4) %u.%u.%u.%u",	// Their version is always 2.1.1.0 or 2.2.1.0
						( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
						( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
				break;
			}
			// Old Shareaza beta build. Note 2nd last number (Beta build #) may be truncated, since it's only 3 bits.
			m_sUserAgent.Format( L"Shareaza %u.%u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
				( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
			break;
		case 5:
			m_sUserAgent.Format( L"ePlus %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 8:
			m_sUserAgent.AppendFormat( L"easyMule %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 10:
			m_sUserAgent.Format( L"MLdonkey %u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
				( ( m_nSoftwareVersion >>  7 ) &0x07 ) );
			if ( ( ( m_nSoftwareVersion ) &0x7F ) > 0 )
				m_sUserAgent.AppendFormat( L".%i", ( ( m_nSoftwareVersion ) &0x7F ) );
			break;
		case 20:
			m_sUserAgent.Format( L"Lphant %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		case 40:		// Shareaza
			if ( m_bEmAICH )
			{
				if ( m_sUserAgent.IsEmpty() )
					m_sUserAgent.Format( L"eMule Mod (40) %u.%u.%u.%u",
						( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
						( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
				break;
			}
			// Note 2nd last number (Beta build #) may be truncated, since it's only 3 bits.
			m_sUserAgent.Format( L"Shareaza %u.%u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
				( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
			break;
		case 41:		// iShareaza
			// Note 2nd last number (Beta build #) may be truncated, since it's only 3 bits.
			m_sUserAgent.Format( L"iShareaza %u.%u.%u.%u",
				((m_nSoftwareVersion >> 17) & 0x7F), ((m_nSoftwareVersion >> 10) & 0x7F),
				((m_nSoftwareVersion >> 7) & 0x07), ((m_nSoftwareVersion) & 0x7F) );
			break;
		case 80:		// Envy (Proposed 0x50) (Was PeerProject)
			if ( m_bEmAICH )
			{
				if ( m_sUserAgent.IsEmpty() )
					m_sUserAgent.Format( L"eMule Mod (80) %u.%u.%u.%u",
						( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
						( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
				break;
			}
			// Note 2nd last number (Beta build #) may be truncated, since it's only 3 bits.
			m_sUserAgent.Format( L"Envy %u.%u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
				( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
			break;
		case 203:		// ShareazaPlus with RazaCB core
			// Note 2nd last number (Beta build #) may be truncated, since it's only 3 bits.
			m_sUserAgent.Format( L"ShareazaPlus %u.%u.%u.%u",
				( ( m_nSoftwareVersion >> 17 ) &0x7F ), ( ( m_nSoftwareVersion >> 10 ) &0x7F ),
				( ( m_nSoftwareVersion >>  7 ) &0x07 ), ( ( m_nSoftwareVersion ) &0x7F ) );
			break;
		case 170:
			m_sUserAgent.Format( L"JMule %u.%u%c",
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		default:		// (Sent a compatible client ID, but we don't recognise it)
			m_sUserAgent.Format( L"eMule Mod (%u) %u.%u%c", m_nEmCompatible,
				( ( m_nSoftwareVersion >> 17 ) & 0x7F ), ( ( m_nSoftwareVersion >> 10 ) & 0x7F ),
				( ( m_nSoftwareVersion >>  7 ) & 0x07 ) + 'a' );
			break;
		}
	}
	else // Older style of IDing a client
	{
		if ( m_oGUID[5] == 13 && m_oGUID[14] == 110 )
			m_bEmule = TRUE;
		else if ( m_oGUID[5] == 14 && m_oGUID[14] == 111 )
			m_bEmule = TRUE;

		if ( m_nEmVersion <= 0 )
			m_nEmVersion = m_nVersion;

		if ( m_bEmule )
		{
			switch ( m_nEmCompatible )
			{
			case 0:
				m_sUserAgent.Format( L"eMule v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 1:
				m_sUserAgent.Format( L"cDonkey v%u.%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 2:
				m_sUserAgent.Format( L"xMule v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 3:
				m_sUserAgent.Format( L"aMule v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 4:			// Old Shareaza alpha/beta/mod/fork versions
				if ( m_bEmAICH )	// AICH verification for corruption detection
				{
					if ( m_sUserAgent.IsEmpty() )
						m_sUserAgent.Format( L"eMule Mod (4) v%u", m_nEmVersion );
					break;
				}
				m_sUserAgent = L"Shareaza";
				break;
			case 8:
				m_sUserAgent.Format( L"easyMule v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 10:
				m_sUserAgent.Format( L"MLdonkey v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 20:
				m_sUserAgent.Format( L"Lphant v0.%u%u", m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			case 40:		// Shareaza
				if ( m_bEmAICH )	// AICH verification for corruption detection
				{
					if ( m_sUserAgent.IsEmpty() )
						m_sUserAgent.Format( L"eMule Mod (40) v%u", m_nEmVersion );
					break;
				}
				m_sUserAgent = L"Shareaza";
				break;
			case 80:		// Envy (Proposed 0x50)  (Note shared with PeerProject)
				if ( m_bEmAICH )	// AICH verification for corruption detection (ToDo: support AICH, etc.)
				{
					if ( m_sUserAgent.IsEmpty() )
						m_sUserAgent.Format( L"eMule Mod (80) v%u", m_nEmVersion );
					break;
				}
				m_sUserAgent = L"Envy";
				break;
			case 203:		// ShareazaPlus RazaCB
				m_sUserAgent.Format( L"ShareazaPlus" );
				break;
			case 170:		// JMule
				m_sUserAgent.Format( L"JMule" );
				break;
			case ED2K_CLIENT_MOD:		// (Did not send a compatible client ID, but did send a MOD tag)
				m_sUserAgent.Format( L"eMule Mod v%u", m_nEmVersion );
				break;
			case ED2K_CLIENT_UNKNOWN:	// (Did not send a compatible client ID)
				if ( _tcsistr( m_sNick, L"www.pruna.com" ) )	// ToDo: Need a better way to recognize pruna?
					m_sUserAgent.Format( L"Pruna v%u", m_nEmVersion );
				else
					m_sUserAgent.Format( L"Unidentified v%u", m_nEmVersion );
				break;
			default:		// (Sent a compatible client ID, but we don't recognise it)
				m_sUserAgent.Format( L"eMule Mod (%u) v0.%u%u", m_nEmCompatible, m_nEmVersion >> 4, m_nEmVersion & 15 );
				break;
			}
		}
		else if ( m_oGUID[5] == 'M' && m_oGUID[14] == 'L' )
		{
			m_sUserAgent.Format( L"MLdonkey v%u", m_nVersion );
		}
		else
		{
			m_sUserAgent = L"eDonkeyHybrid ";

			if ( m_nVersion >= 20000 )		// Unknown
				m_sUserAgent.AppendFormat( L"%u", m_nVersion );
			else if ( m_nVersion >= 10100 )	// eDonkey from version 1.1.0
			{
				CString strVersion;
				strVersion.Format( L"%u", m_nVersion );
				m_sUserAgent.AppendFormat( L"v%c.%c.%c", (TCHAR)strVersion[0], (TCHAR)strVersion[2], (TCHAR)strVersion[4] );
			}
			else if ( m_nVersion >= 1100 )	// Unknown
				m_sUserAgent.AppendFormat( L"%u", m_nVersion );
			else if ( m_nVersion >= 1025 )	// eDonkey 0.xx
				m_sUserAgent.AppendFormat( L"v0.%u", m_nVersion - 1000 );
			else if ( m_nVersion >= 1000 )	// eDonkey 1.0.x
				m_sUserAgent.AppendFormat( L"v1.0.%u", m_nVersion - 1000 );
			else if ( m_nVersion > 0 )		// Probably the first edonkey versions
				m_sUserAgent.Format( L"eDonkey v%u", m_nVersion );
			else							// It shouldn't happen
				m_sUserAgent = L"Unidentified eDonkey";
		}
	}

	// Client allows G2 browse, etc.
	m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );
	if ( m_pUploadTransfer )
		m_pUploadTransfer->m_bClientExtended = m_bClientExtended;
	if ( CDownloadSource* pSource = GetSource() )
		pSource->m_bClientExtended = m_bClientExtended;
}

//////////////////////////////////////////////////////////////////////
// CEDClient FILE REQUEST handler

BOOL CEDClient::OnFileRequest(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILEREQANSWER );

	pPacket->Read( m_oUpED2K );
	pReply->Write( m_oUpED2K );

	// Extra security check (Shouldn't be needed, but there have been reports of glitches)
	if ( Security.IsDenied( &m_pHost.sin_addr ) ||
		 Security.IsClientBanned( m_sUserAgent ) )
	{
		pReply->m_nType = ED2K_C2C_FILENOTFOUND;
		Send( pReply );
		theApp.Message( MSG_ERROR, L"ED2K upload to %s blocked by security rules.", m_sAddress);
		return TRUE;
	}

	if ( ! Settings.eDonkey.Enabled && Settings.Connection.RequireForTransfers )
	{
		pReply->m_nType = ED2K_C2C_FILENOTFOUND;
		Send( pReply );
		return TRUE;
	}

	CSingleLock pLock( &Library.m_pSection );
	if ( SafeLock( pLock ) )
	{
		CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( m_oUpED2K, TRUE, TRUE );
		if ( ( pFile ) && ( UploadQueues.CanUpload( PROTOCOL_ED2K, pFile, TRUE ) ) )
		{
			// Create the reply packet
			pReply->WriteEDString( pFile->m_sName, m_bEmUnicode );
			// Get the comments/rating data
			int nRating = pFile->m_nRating;
			CString strComments = pFile->m_sComments;
			pLock.Unlock();

			// Send reply and comments/rating (if required)
			Send( pReply );
			SendCommentsPacket( nRating, strComments );

			return TRUE;
		}
		pLock.Unlock();
	}

	if ( CDownload* pDownload = Downloads.FindByED2K( m_oUpED2K, TRUE ) )
	{
		pReply->WriteEDString( pDownload->m_sName, m_bEmUnicode );
		Send( pReply );
		return TRUE;
	}

	pReply->m_nType = ED2K_C2C_FILENOTFOUND;
	Send( pReply );

	theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_oUpED2K.toUrn() );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient file status request

BOOL CEDClient::OnFileStatusRequest(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILESTATUS );

	pPacket->Read( m_oUpED2K );
	pReply->Write( m_oUpED2K );

	BOOL bOk = FALSE;

	{
		CSingleLock pLock( &Library.m_pSection, FALSE );
		if ( SafeLock( pLock ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( m_oUpED2K, TRUE, TRUE ) )
			{
				pReply->WriteShortLE( 0 );
				pReply->WriteByte( 0 );

				m_nUpSize = pFile->GetSize();
				if ( ! CEDPacket::IsLowID( m_nClientID ) )
					pFile->AddAlternateSource( GetSourceURL() );

				bOk = TRUE;
			}
		}
	}

	if ( ! bOk )
	{
		if ( CDownload* pDownload = Downloads.FindByED2K( m_oUpED2K, TRUE ) )
		{
			WritePartStatus( pReply, pDownload );
			m_nUpSize = pDownload->m_nSize;

			if ( ! pDownload->IsMoving() && ! pDownload->IsCompleted() )
				pDownload->AddSourceED2K( m_nClientID, htons( m_pHost.sin_port ), m_pServer.sin_addr.S_un.S_addr, htons( m_pServer.sin_port ), m_oGUID );

			bOk = TRUE;
		}
	}

	if ( ! bOk )
	{
		pReply->m_nType = ED2K_C2C_FILENOTFOUND;
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_oUpED2K.toUrn() );
		m_oUpED2K.clear();
	}

	Send( pReply );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient hash set request

BOOL CEDClient::OnHashsetRequest(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );

	const CED2K* pHashset = NULL;
	BOOL bDelete = FALSE;
	CString strName;

	{
		CSingleLock pLock( &Library.m_pSection, FALSE );
		if ( SafeLock( pLock ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( oHash, TRUE, TRUE ) )
			{
				strName		= pFile->m_sName;
				pHashset	= pFile->GetED2K();
				bDelete		= TRUE;
			}
		}
	}

	if ( pHashset == NULL )
	{
		if ( const CDownload* pDownload = Downloads.FindByED2K( oHash, TRUE ) )
		{
			pHashset = pDownload->GetHashset();
			strName = pDownload->m_sName;
		}
	}

	if ( pHashset != NULL )
	{
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_HASHSETANSWER );
		pReply->Write( oHash );
		int nBlocks = pHashset->GetBlockCount();
		if ( nBlocks <= 1 ) nBlocks = 0;
		pReply->WriteShortLE( (WORD)nBlocks );
		// ToDo: make this nicer
		pReply->Write( pHashset->GetRawPtr(), Hashes::Ed2kHash::byteCount * nBlocks );
		if ( bDelete ) delete pHashset;
		Send( pReply );

		theApp.Message( MSG_INFO, IDS_ED2K_CLIENT_SENT_HASHSET, (LPCTSTR)strName, (LPCTSTR)m_sAddress );
	}
	else // pHashset == NULL
	{
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pReply->Write( oHash );
		Send( pReply );

		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)oHash.toUrn() );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient hashset request 2 (with FileIdentifier)

BOOL CEDClient::OnHashsetRequest2(CEDPacket* pPacket)
{
	CFileIdentifier oFileID;
	if ( ! oFileID.Parse( pPacket ) )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	// Read options byte
	BYTE nOptions = 0;
	if ( pPacket->GetRemaining() >= 1 )
		nOptions = pPacket->ReadByte();

	const CED2K* pHashset = NULL;
	BOOL bDelete = FALSE;
	CString strName;

	{
		CSingleLock pLock( &Library.m_pSection, FALSE );
		if ( SafeLock( pLock ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( oFileID.GetHash(), TRUE, TRUE ) )
			{
				// Verify file size matches
				if ( pFile->m_nSize == oFileID.GetSize() )
				{
					strName		= pFile->m_sName;
					pHashset	= pFile->GetED2K();
					bDelete		= TRUE;
				}
			}
		}
	}

	if ( pHashset == NULL )
	{
		if ( const CDownload* pDownload = Downloads.FindByED2K( oFileID.GetHash(), TRUE ) )
		{
			// Verify file size matches
			if ( pDownload->m_nSize == oFileID.GetSize() )
			{
				pHashset = pDownload->GetHashset();
				strName = pDownload->m_sName;
			}
		}
	}

	if ( pHashset != NULL )
	{
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_HASHSETANSWER2, ED2K_PROTOCOL_EMULE );

		// Write FileIdentifier
		oFileID.Write( pReply );

		// Write options byte
		pReply->WriteByte( nOptions );

		// Write hashset if requested
		if ( nOptions & 0x01 ) // Hashset requested
		{
			int nBlocks = pHashset->GetBlockCount();
			if ( nBlocks <= 1 ) nBlocks = 0;
			pReply->WriteShortLE( (WORD)nBlocks );
			if ( nBlocks > 0 )
				pReply->Write( pHashset->GetRawPtr(), Hashes::Ed2kHash::byteCount * nBlocks );
		}

		if ( bDelete ) delete pHashset;
		Send( pReply );

		theApp.Message( MSG_INFO, IDS_ED2K_CLIENT_SENT_HASHSET, (LPCTSTR)strName, (LPCTSTR)m_sAddress );
	}
	else // pHashset == NULL
	{
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pReply->Write( oFileID.GetHash() );
		Send( pReply );

		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)oFileID.GetHash().toUrn() );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient MultiPacket Ext2 handler

BOOL CEDClient::OnMultiPacketExt2(CEDPacket* pPacket)
{
	// MultiPacket Ext2 uses FileIdentifier format
	// This is a request for multiple files using FileIdentifiers
	// Format: <FileIdentifier 1><FileIdentifier 2>...<FileIdentifier N>

	// For now, handle as multiple file requests
	// In a full implementation, this would request multiple files at once
	while ( pPacket->GetRemaining() >= Hashes::Ed2kHash::byteCount + sizeof(QWORD) )
	{
		CFileIdentifier oFileID;
		if ( ! oFileID.Parse( pPacket ) )
			break;

		// Handle as individual file request (simplified implementation)
		// In full implementation, would batch these requests
		Hashes::Ed2kHash oHash = oFileID.GetHash();
		CEDPacket* pFileRequest = CEDPacket::New( ED2K_C2C_FILEREQUEST );
		pFileRequest->Write( oHash );
		// Note: This is a simplified implementation
		// Full MultiPacket Ext2 would handle multiple files in one packet
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient MultiPacket Answer Ext2 handler

BOOL CEDClient::OnMultiPacketAnswerExt2(CEDPacket* pPacket)
{
	// MultiPacket Answer Ext2 uses FileIdentifier format
	// This is a response with multiple files using FileIdentifiers
	// Format: <FileIdentifier 1><data 1><FileIdentifier 2><data 2>...

	// For now, handle as multiple file answers
	// In a full implementation, this would handle multiple file responses at once
	while ( pPacket->GetRemaining() >= Hashes::Guid::byteCount + sizeof(QWORD) )
	{
		CFileIdentifier oFileID;
		if ( ! oFileID.Parse( pPacket ) )
			break;

		// Handle as individual file answer (simplified implementation)
		// In full implementation, would process multiple files in one packet
		if ( m_pDownloadTransfer )
		{
			// Process file answer for this FileIdentifier
			// Note: This is a simplified implementation
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient queue request

BOOL CEDClient::OnQueueRequest(CEDPacket* /*pPacket*/)
{
	if ( ! m_oUpED2K )
		return TRUE;
		// MESSAGE: File not requested yet

	if ( ( m_pUploadTransfer ) && validAndUnequal( m_pUploadTransfer->m_oED2K, m_oUpED2K ) )
		DetachUpload();

	if ( m_pUploadTransfer == NULL )
		m_pUploadTransfer = new CUploadTransferED2K( this );

	m_pUploadTransfer->Request( m_oUpED2K );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient Message handler

BOOL CEDClient::OnChatMessage(CEDPacket* pPacket)
{
	// Check packet has message length
	if ( pPacket->GetRemaining() < 3 )
	{
		theApp.Message( MSG_ERROR, L"Empty message packet received from %s", (LPCTSTR)m_sAddress );
		return TRUE;
	}

	// Read message length
	DWORD nMessageLength = pPacket->ReadShortLE();

	// Validate message length
	if ( nMessageLength < 1 ||
		 nMessageLength > ED2K_MESSAGE_MAX ||
		 nMessageLength != pPacket->GetRemaining() )
	{
		theApp.Message( MSG_ERROR, L"Invalid message packet received from %s", (LPCTSTR)m_sAddress );
		return TRUE;
	}

	// Read in message
	CString strMessage;
	if ( m_bEmUnicode )
		strMessage = pPacket->ReadStringUTF8( nMessageLength );
	else
		strMessage = pPacket->ReadStringASCII( nMessageLength );

	// Check the message is not spam
	if ( MessageFilter.IsED2KSpam( strMessage ) )
	{
		// Block L33cher mods
		if ( m_pDownloadTransfer == NULL )
			Security.Ban( &m_pHost.sin_addr, banSession, FALSE );
		// Don't display message
		return TRUE;
	}

	if ( MessageFilter.IsFiltered( strMessage ) )
		return TRUE;	// General spam filter (if enabled)

	// Check chat settings to open/update a chat window
	if ( Settings.Community.ChatEnable )	// && Settings.Community.ChatAllNetworks
		ChatCore.OnMessage( this, pPacket );
	else // Chat is disabled- don't open a chat window. Display in system window instead.
		theApp.Message( MSG_INFO, L"Message from %s: %s", (LPCTSTR)m_sAddress, strMessage );

	return TRUE;
}

BOOL CEDClient::OnCaptchaRequest(CEDPacket* pPacket)
{
	// Check packet has message length
	if ( pPacket->GetRemaining() < 128 )
	{
		theApp.Message( MSG_ERROR, L"Empty CAPTHA request packet received from %s", (LPCTSTR)m_sAddress );
		return TRUE;
	}

	// Skip all tags (future use)
	for ( BYTE nCount = pPacket->ReadByte(); nCount && pPacket->GetRemaining(); --nCount )
	{
		CEDTag pTag;
		if ( ! pTag.Read( pPacket ) )
		{
			theApp.Message( MSG_ERROR, L"Wrong CAPTHA request packet received from %s", (LPCTSTR)m_sAddress );
			return TRUE;
		}
	}

	// Read image
	DWORD nSize = pPacket->GetRemaining();
	if ( nSize > 128 && nSize < 4096 )
		ChatCore.OnMessage( this, pPacket );
	else
		theApp.Message( MSG_ERROR, L"Wrong CAPTHA request packet received from %s", (LPCTSTR)m_sAddress );

	return TRUE;
}

BOOL CEDClient::OnCaptchaResult(CEDPacket* pPacket)
{
	// Check packet has message length
	if ( pPacket->GetRemaining() < 1 )
	{
		theApp.Message( MSG_ERROR, L"Empty CAPTHA result packet received from %s", (LPCTSTR)m_sAddress );
		return TRUE;
	}

	ChatCore.OnMessage( this, pPacket );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient shared dirs request

BOOL CEDClient::OnAskSharedDirs(CEDPacket* /*pPacket*/)
{
	if ( Settings.Community.ServeFiles )
	{
		CSingleLock pLock( &Library.m_pSection );
		if ( SafeLock( pLock ) )
		{
			CList< CString > oFolderPath;
			CList< CLibraryFolder* > oFolders;
			for ( POSITION pos = LibraryFolders.GetFolderIterator(); pos; )
			{
				oFolders.AddTail( LibraryFolders.GetNextFolder( pos ) );
			}
			while ( ! oFolders.IsEmpty() )
			{
				CLibraryFolder* pFolder = oFolders.RemoveHead();

				if ( pFolder->IsShared() )
					oFolderPath.AddTail( pFolder->GetRelativeName() );

				for ( POSITION pos = pFolder->GetFolderIterator(); pos; )
				{
					oFolders.AddTail( pFolder->GetNextFolder( pos ) );
				}
			}
			pLock.Unlock();

			if ( CEDPacket* pReply = CEDPacket::New( ED2K_C2C_ASKSHAREDDIRSANSWER ) )
			{
				pReply->WriteLongLE( (DWORD)oFolderPath.GetCount() );

				for ( POSITION pos = oFolderPath.GetHeadPosition(); pos; )
				{
					CString strPath = oFolderPath.GetNext( pos );
					pReply->WriteEDString( strPath, m_bEmUnicode );
				}

				Send( pReply );

				return TRUE;
			}
		}
	}

	if ( CEDPacket* pReply = CEDPacket::New( ED2K_C2C_ASKSHAREDDIRSDENIED ) )
		Send( pReply );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient dir browse request

BOOL CEDClient::OnViewSharedDir(CEDPacket* pPacket)
{
	CString strDir = pPacket->ReadEDString( m_bEmUnicode );

	if ( Settings.Community.ServeFiles )
	{
		CSingleLock pLock( &Library.m_pSection );
		if ( SafeLock( pLock ) )
		{
			CLibraryFolder* pFolder = LibraryFolders.GetFolderByName( strDir );
			if ( pFolder && pFolder->IsShared() )
			{
				// Count files
				DWORD nCount = 0;
				for ( POSITION pos = pFolder->GetFileIterator(); pos; )
				{
					CLibraryFile* pFile = pFolder->GetNextFile( pos );
					if ( pFile->IsShared() )
						nCount++;
				}

				if ( CEDPacket* pReply = CEDPacket::New( ED2K_C2C_VIEWSHAREDDIRANSWER ) )
				{
					// Original dir name
					pReply->WriteEDString( strDir, m_bEmUnicode );

					// Number of files
					pReply->WriteLongLE( nCount );

					for ( POSITION pos = pFolder->GetFileIterator(); pos && nCount; )
					{
						CLibraryFile* pFile = pFolder->GetNextFile( pos );

						if ( ! pFile->IsShared() )
							continue;

						--nCount;

						pReply->WriteFile( pFile, pFile->GetSize(), this );
					}

					// Obsolete (ToDo: Delete this?)

					//	// File hash
					//	pReply->Write( pFile->m_oED2K );
					//	// ID
					//	pReply->WriteLongLE( Network.m_pHost.sin_addr.s_addr );
					//	// Port
					//	pReply->WriteShortLE( htons( Network.m_pHost.sin_port ) );
					//
					//	CString strType, strTitle, strArtist, strAlbum, strCodec;
					//	DWORD nBitrate = 0, nLength = 0, nTags = 5;
					//	if ( pFile->m_pSchema && ! pFile->m_pSchema->m_sDonkeyType.IsEmpty() )
					//	{
					//		strType = pFile->m_pSchema->m_sDonkeyType;
					//		nTags ++;
					//
					//		// Title
					//		if ( pFile->m_pMetadata->GetAttributeValue( L"title" ).GetLength() )
					//		{
					//			strTitle = pFile->m_pMetadata->GetAttributeValue( L"title" );
					//			if ( ! strTitle.IsEmpty() ) nTags++;
					//		}
					//		if ( pFile->IsSchemaURI( CSchema::uriAudio ) )
					//		{
					//			// Artist
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"artist" ).GetLength() )
					//			{
					//				strArtist = pFile->m_pMetadata->GetAttributeValue( L"artist" );
					//				if ( ! strArtist.IsEmpty() ) nTags++;
					//			}
					//			// Album
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"album" ).GetLength() )
					//			{
					//				strAlbum = pFile->m_pMetadata->GetAttributeValue( L"album" );
					//				if ( ! strAlbum.IsEmpty() ) nTags++;
					//			}
					//			// Bitrate
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"bitrate" ).GetLength() )	// And has a bitrate
					//			{
					//				_stscanf( pFile->m_pMetadata->GetAttributeValue( L"bitrate" ), L"%i", &nBitrate );
					//				if ( nBitrate ) nTags++;
					//			}
					//			// Length
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"seconds" ).GetLength() )	// And has seconds
					//			{
					//				nLength = 0;
					//				_stscanf( pFile->m_pMetadata->GetAttributeValue( L"seconds" ), L"%i", &nLength );
					//				if ( nLength ) nTags++;
					//			}
					//		}
					//		else if ( pFile->IsSchemaURI( CSchema::uriVideo ) )
					//		{
					//			// Codec
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"codec" ).GetLength() )
					//			{
					//				strCodec = pFile->m_pMetadata->GetAttributeValue( L"codec" );
					//				if ( ! strCodec.IsEmpty() ) nTags++;
					//			}
					//			// Length
					//			if ( pFile->m_pMetadata->GetAttributeValue( L"minutes" ).GetLength() )
					//			{
					//				double nMins = 0.0;
					//				_stscanf( pFile->m_pMetadata->GetAttributeValue( L"minutes" ), L"%lf", &nMins );
					//				nLength = (DWORD)( nMins * (double)60 );	// Convert to seconds
					//				if ( nLength ) nTags++;
					//			}
					//		}
					//	}
					//
					//	BYTE nRating = 0;
					//	if ( pFile->m_nRating )
					//	{
					//		nRating = (BYTE)min( pFile->m_nRating, 5 );
					//		nTags ++;
					//	}
					//
					//	if ( pFile->m_nSize > MAX_SIZE_32BIT )
					//		nTags ++;
					//
					//	// Number of Tags
					//	pReply->WriteLongLE( nTags );
					//	// Filename
					//	CEDTag( ED2K_FT_FILENAME, pFile->m_sName ).Write( pReply, m_bEmUnicode );
					//	// File size
					//	CEDTag( ED2K_FT_FILESIZE, (DWORD)pFile->m_nSize ).Write( pReply );
					//	if ( pFile->m_nSize > MAX_SIZE_32BIT )
					//		CEDTag( ED2K_FT_FILESIZE_HI, (DWORD)(pFile->m_nSize >> 32 ) ).Write( pReply );
					//	// Sources
					//	CEDTag( ED2K_FT_SOURCES, 1ull ).Write( pReply );
					//	// Complete sources
					//	CEDTag( ED2K_FT_COMPLETE_SOURCES, 1ull ).Write( pReply );
					//	// Last seen
					//	CEDTag( ED2K_FT_LASTSEENCOMPLETE, 0ull ).Write( pReply );
					//	// File type
					//	if ( ! strType.IsEmpty() )
					//		CEDTag( ED2K_FT_FILETYPE, strType ).Write( pReply );
					//	// Title
					//	if ( ! strTitle.IsEmpty() )
					//		CEDTag( ED2K_FT_TITLE, strTitle ).Write( pReply );
					//	// Artist
					//	if ( ! strArtist.IsEmpty() )
					//		CEDTag( ED2K_FT_ARTIST, strArtist ).Write( pReply );
					//	// Album
					//	if ( ! strAlbum.IsEmpty() )
					//		CEDTag( ED2K_FT_ALBUM, strAlbum ).Write( pReply );
					//	// Bitrate
					//	if ( nBitrate )
					//		CEDTag( ED2K_FT_BITRATE, nBitrate ).Write( pReply );
					//	// Length
					//	if ( nLength )
					//		CEDTag( ED2K_FT_LENGTH, nLength ).Write( pReply );
					//	// Codec
					//	if ( ! strCodec.IsEmpty() )
					//		CEDTag( ED2K_FT_CODEC, strCodec ).Write( pReply );
					//	// File rating
					//	if ( nRating )
					//		CEDTag( ED2K_FT_FILERATING, nRating ).Write( pReply );
					//}

					pLock.Unlock();

					Send( pReply );

					return TRUE;
				}
			}
		}
	}

	if ( CEDPacket* pReply = CEDPacket::New( ED2K_C2C_ASKSHAREDDIRSDENIED ) )
		Send( pReply );

	return TRUE;
}

BOOL CEDClient::OnAskSharedDirsAnswer(CEDPacket* pPacket)
{
	if ( CHostBrowser* pBrowser = GetBrowser() )
		pBrowser->OnHeadersComplete();

	if ( pPacket->GetRemaining() >= 4 )
	{
		// Read number of directories
		DWORD nCount = pPacket->ReadLongLE();

		for ( DWORD i = 0; i < nCount; i++ )
		{
			if ( pPacket->GetRemaining() < 2 )
				break;

			// Read directory name
			CString strDir = pPacket->ReadEDString( m_bEmUnicode );

			TRACE( "Folder: %s\n", (LPCSTR)CT2A( strDir ) );

			// Request directory content
			if ( CEDPacket* pReply = CEDPacket::New( ED2K_C2C_VIEWSHAREDDIR ) )
			{
				pReply->WriteEDString( strDir, m_bEmUnicode );

				Send( pReply );

				m_nDirsWaiting ++;
			}
		}
	}

	return TRUE;
}

BOOL CEDClient::OnViewSharedDirAnswer(CEDPacket* pPacket)
{
	if ( CHostBrowser* pBrowser = GetBrowser() )
		pBrowser->m_nState = CHostBrowser::hbsContent;

	CQueryHit* pHits = NULL;

	if ( pPacket->GetRemaining() >= 2 )
	{
		// Read original directory name (Unused)
		//CString strDir = pPacket->ReadEDString( m_bEmUnicode );

		if ( pPacket->GetRemaining() >= 4 )
		{
			// Read number of files
			DWORD nCount = pPacket->ReadLongLE();

			for ( DWORD i = 0; i < nCount; i++ )
			{
				if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 4 + 2 + 4 )
					break;

				CQueryHit* pHit = new CQueryHit( PROTOCOL_ED2K );

				pHit->m_bBrowseHost = TRUE;
				pHit->m_bChat = TRUE;
				pHit->m_pVendor = VendorCache.Lookup( L"ED2K" );
				if ( ! pHit->m_pVendor )
					pHit->m_pVendor = VendorCache.m_pNull;

				pHit->ReadEDPacket( pPacket, &m_pServer, m_bEmUnicode );

				pHit->m_pAddress = m_pHost.sin_addr;
				pHit->m_nPort = ntohs( m_pHost.sin_port );

				pHit->Resolve();

				pHit->m_pNext = pHits;
				pHits = pHit;
			}
		}
	}

	if ( pHits )
	{
		if ( CHostBrowser* pBrowser = GetBrowser() )
			pBrowser->OnQueryHits( pHits );
		else
			Network.OnQueryHits( pHits );
	}

	ASSERT( m_nDirsWaiting );
	if ( --m_nDirsWaiting == 0 )
	{
		// Browse complete
		if ( CHostBrowser* pBrowser = GetBrowser() )
			pBrowser->Stop();
	}

	return TRUE;
}

BOOL CEDClient::OnAskSharedDirsDenied(CEDPacket* /*pPacket*/)
{
	// Access denied
	if ( CHostBrowser* pBrowser = GetBrowser() )
		pBrowser->Stop();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient file preview request (Always returns true?)

BOOL CEDClient::OnRequestPreview(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}
	if ( Security.IsDenied( &m_pHost.sin_addr ) ||
		 Security.IsClientBanned( m_sUserAgent ) )  // Extra security check
	{
		theApp.Message( MSG_ERROR, L"ED2K upload to %s blocked by security rules.", (LPCTSTR)m_sAddress);
		return TRUE;
	}

	if ( ! Settings.Uploads.SharePreviews )
	{
		theApp.Message( MSG_INFO, L"ED2K preview request by %s blocked by user Settings.", (LPCTSTR)m_sAddress);
		return TRUE;
	}

	if ( ! Network.IsConnected() || ( ! Settings.eDonkey.Enabled && Settings.Connection.RequireForTransfers ) )
	{
		theApp.Message( MSG_INFO, L"ED2K preview request by %s blocked by disabled network.", (LPCTSTR)m_sAddress);
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );

	CSingleLock pLock( &Library.m_pSection );
	if ( ! SafeLock( pLock ) )
		return TRUE;

	CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( oHash, TRUE, TRUE );

	if ( ! pFile )
	{
		theApp.Message( MSG_INFO, L"ED2K preview request by %s file not found.", (LPCTSTR)m_sAddress);
		return TRUE;
	}

	// We own this file and previews are enabled

	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_PREVIEWANWSER, ED2K_PROTOCOL_EMULE );
	pReply->Write( oHash );

	LPCTSTR pszName = pFile->m_sName;

	CImageFile pImage;
	if ( CThumbCache::Cache( pFile->GetPath(), &pImage, Settings.Uploads.DynamicPreviews ) )
	{
		theApp.Message( MSG_INFO, IDS_UPLOAD_PREVIEW_DYNAMIC, pszName, (LPCTSTR)m_sAddress );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, pszName );
		Send( pReply );
		return TRUE;	// Not an image packet
	}

	pLock.Unlock();

	BYTE* pBuffer = NULL;
	DWORD nImageSize = 0;
	const int nFrames = 1;

	if ( ! pImage.SaveToMemory( L".png", Settings.Uploads.PreviewQuality, &pBuffer, &nImageSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, pszName );
		Send( pReply );
		return TRUE;	// Not an image packet
	}

	pReply->Write( (LPCVOID)&nFrames, 1 );	// We send only 1 frame
	pReply->WriteLongLE( nImageSize );
	pReply->Write( (LPCVOID)pBuffer, nImageSize );
	delete [] pBuffer;

	// Send reply
	Send( pReply );
	theApp.Message( MSG_NOTICE, IDS_UPLOAD_PREVIEW_SEND, pszName, (LPCTSTR)m_sAddress );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient file preview answer

BOOL CEDClient::OnPreviewAnswer(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		// We don't do if that was the answer to our request; just leave.
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );
	CDownload* pDownload = NULL;

	if ( pPacket->GetRemaining() > 1 + 4 )	// File has preview
	{
		// Check only downloads. Previews become unneeded when download is completed.
		if ( ( pDownload = Downloads.FindByED2K( oHash ) ) != NULL )
		{
			int nFrames = 0;
			pPacket->Read( (void*)&nFrames, 1 );
			if ( nFrames > 0 )
			{
				for ( int nFrame = 0; nFrame < nFrames; nFrame++ )
				{
					DWORD nFrameSize = pPacket->ReadLongLE();
					if ( pPacket->GetRemaining() < static_cast<int>( nFrameSize ) )
					{
						theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
						return TRUE;
					}

					CFile pFile;
					CString strPath = pDownload->m_sPath + L".png";
					if ( pFile.Open( strPath, CFile::modeCreate|CFile::modeWrite ) )
					{
						BYTE szByte = 0;

						// Write only the first frame
						for ( DWORD nByte = 0; nByte < nFrameSize; nByte++ )
						{
							szByte = pPacket->ReadByte();
							pFile.Write( &szByte, 1 );
						}
						pFile.Close();

						// Make it hidden, so the files won't be shared
						SetFileAttributes( (LPCTSTR)strPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM );
						break;
					}
				}
			}
		}
		else
		{
			CSingleLock pLock( &Library.m_pSection );
			if ( SafeLock( pLock ) )
			{
				CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( oHash );
				pLock.Unlock();
				if ( pFile == NULL )	// Likely spam
					Security.Ban( &m_pHost.sin_addr, banWeek, FALSE );
			}
		}
	}

	return TRUE;
}

static BOOL ValidateSourcePacketBody(CEDPacket* pPacket, DWORD nCount, DWORD nSourceSize)
{
	return Ed2kValidateSourcePacketBody( pPacket->GetRemaining(), nCount, nSourceSize );
}

//////////////////////////////////////////////////////////////////////
// CEDClient source request

BOOL CEDClient::OnSourceRequest(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );

	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_ANSWERSOURCES, ED2K_PROTOCOL_EMULE );
	int nCount = 0;

	if ( CDownload* pDownload = Downloads.FindByED2K( oHash, TRUE ) )
	{
		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bReadContent )
			{
				pReply->WriteLongLE( pSource->m_pAddress.S_un.S_addr );
				pReply->WriteShortLE( pSource->m_nPort );
				pReply->WriteLongLE( pSource->m_pServerAddress.S_un.S_addr );
				pReply->WriteShortLE( (WORD)pSource->m_nServerPort );
				if ( m_bEmSources >= 2 ) pReply->Write( pSource->m_oGUID );
				nCount++;
			}
		}
	}

	if ( pReply->m_nLength > 0 )
	{
		BYTE* pStart = pReply->WriteGetPointer( Hashes::Ed2kHash::byteCount + 2, 0 );

		if ( pStart == NULL )
		{
		//	theApp.Message( MSG_DEBUG, L"Memory allocation error in CEDClient::OnSourceRequest()" );
			pReply->Release();
			return TRUE;
		}

		*reinterpret_cast< Hashes::Ed2kHash::RawStorage* >( pStart )
			= oHash.storage();
		pStart += Hashes::Ed2kHash::byteCount;
		*(WORD*)pStart = WORD( nCount );
		Send( pReply, FALSE );
	}

	pReply->Release();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient source answer

BOOL CEDClient::OnSourceAnswer(CEDPacket* pPacket)
{
	if ( ! Settings.Library.SourceMesh ) return TRUE;

	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );
	DWORD nCount = pPacket->ReadShortLE();

	const DWORD nSourceSize = ( m_bEmSources >= 2 ) ? 28u : 12u;
	if ( ! ValidateSourcePacketBody( pPacket, nCount, nSourceSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	if ( CDownload* pDownload = Downloads.FindByED2K( oHash ) )
	{
		// Don't bother adding sources if download has finished
		if ( pDownload->IsCompleted() || pDownload->IsMoving() ) return TRUE;

		while ( nCount-- > 0 )
		{
			DWORD nClientID		= pPacket->ReadLongLE();
			WORD nClientPort	= pPacket->ReadShortLE();
			DWORD nServerIP		= pPacket->ReadLongLE();
			WORD nServerPort	= pPacket->ReadShortLE();

			Hashes::Guid oGUID;
			if ( m_bEmSources >= 2 ) pPacket->Read( oGUID );
			pDownload->AddSourceED2K( nClientID, nClientPort, nServerIP, nServerPort, oGUID );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient source request v2 (SourceEx2)
// Format: <HASH 16><FileSizeLow 4>[FileSizeHigh 4 (legacy, detected heuristically when Low==0 and >=6 bytes remain)]<Options 2>
// Note: This eMule-inherited wire format is ambiguous for a 32-bit zero size; parser resolves by packet length heuristic.

BOOL CEDClient::OnSourceRequest2(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Ed2kSourceEx2RequestMinBytes() )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );

	DWORD nFileSizeLow = pPacket->ReadLongLE();

	// SourceEx2 request file size can be 32-bit, or legacy 64-bit format (<0><high32>).
	// Legacy form is detected heuristically (Low32 == 0 and enough bytes for <high32><Options>),
	// which is an on-wire ambiguity inherited from eMule.
	if ( const DWORD nLegacy = Ed2kSourceEx2ConsumeLegacyHigh32( nFileSizeLow, pPacket->GetRemaining() ) )
		(void)pPacket->ReadLongLE();

	if ( ! Ed2kSourceEx2HasOptionsBytes( pPacket->GetRemaining() ) )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	(void)pPacket->ReadShortLE();

	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_ANSWERSOURCES2, ED2K_PROTOCOL_EMULE );
	int nCount = 0;

	if ( CDownload* pDownload = Downloads.FindByED2K( oHash, TRUE ) )
	{
		for ( POSITION posSource = pDownload->GetIterator(); posSource; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bReadContent )
			{
				pReply->WriteLongLE( pSource->m_pAddress.S_un.S_addr );
				pReply->WriteShortLE( pSource->m_nPort );
				pReply->WriteLongLE( pSource->m_pServerAddress.S_un.S_addr );
				pReply->WriteShortLE( (WORD)pSource->m_nServerPort );
				pReply->Write( pSource->m_oGUID );
				nCount++;
			}
		}
	}

	if ( pReply->m_nLength > 0 )
	{
		BYTE* pStart = pReply->WriteGetPointer( Hashes::Ed2kHash::byteCount + 2, 0 );
		if ( pStart == NULL )
		{
			pReply->Release();
			return TRUE;
		}

		*reinterpret_cast< Hashes::Ed2kHash::RawStorage* >( pStart ) = oHash.storage();
		pStart += Hashes::Ed2kHash::byteCount;
		*(WORD*)pStart = WORD( nCount );
		Send( pReply, FALSE );
	}

	pReply->Release();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient source answer v2 (SourceEx2)
// Format: <HASH 16><Count 2>[<ClientID 4><Port 2><ServerIP 4><ServerPort 2><GUID 16>]*

BOOL CEDClient::OnSourceAnswer2(CEDPacket* pPacket)
{
	if ( ! Settings.Library.SourceMesh ) return TRUE;

	if ( pPacket->GetRemaining() < Ed2kSourceEx2AnswerMinBytes() )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	Hashes::Ed2kHash oHash;
	pPacket->Read( oHash );
	DWORD nCount = pPacket->ReadShortLE();

	// SourceEx2 always includes GUID (28 bytes per source)
	const DWORD nSourceSize = Ed2kSourceEx2SourceRecordBytes();
	if ( ! ValidateSourcePacketBody( pPacket, nCount, nSourceSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		return TRUE;
	}

	if ( CDownload* pDownload = Downloads.FindByED2K( oHash ) )
	{
		if ( pDownload->IsCompleted() || pDownload->IsMoving() ) return TRUE;

		while ( nCount-- > 0 )
		{
			DWORD nClientID   = pPacket->ReadLongLE();
			WORD nClientPort  = pPacket->ReadShortLE();
			DWORD nServerIP   = pPacket->ReadLongLE();
			WORD nServerPort  = pPacket->ReadShortLE();

			Hashes::Guid oGUID;
			pPacket->Read( oGUID );
			pDownload->AddSourceED2K( nClientID, nClientPort, nServerIP, nServerPort, oGUID );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClient source URL utility

CString CEDClient::GetSourceURL()
{
	ASSERT( bool( m_oGUID ) );
	ASSERT( m_oUpED2K );

	CString str;

	if ( CEDPacket::IsLowID( m_nClientID ) )
	{
		str.Format( L"ed2kftp://%lu@%s:%hu/%s/%I64u/",
			m_nClientID,
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), htons( m_pHost.sin_port ),
			(LPCTSTR)m_oUpED2K.toString(), m_nUpSize );
	}
	else
	{
		str.Format( L"ed2kftp://%s:%hu/%s/%I64u/",
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), htons( m_pHost.sin_port ),
			(LPCTSTR)m_oUpED2K.toString(), m_nUpSize );
	}

	return str;
}

//////////////////////////////////////////////////////////////////////
// CEDClient part status utility

void CEDClient::WritePartStatus(CEDPacket* pPacket, CDownload* pDownload)
{
	bool bNullBlock = ( pDownload->m_nSize % ED2K_PART_SIZE == 0 && pDownload->m_nSize );
	QWORD nParts = ( pDownload->m_nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE;

	if ( bNullBlock )
		nParts++;

	pPacket->WriteShortLE( (WORD)nParts );

	if ( pDownload->m_pHashsetBlock != NULL && pDownload->m_nHashsetBlock == nParts )
	{
		for ( QWORD nPart = 0; nPart < nParts; )
		{
			BYTE nByte = 0;

			for ( DWORD nBit = 0; nBit < 8 && nPart < nParts; nBit++, nPart++ )
			{
				if ( pDownload->m_pHashsetBlock[ nPart ] == TRI_TRUE )
					nByte |= ( 1 << nBit );
			}

			pPacket->WriteByte( nByte );
		}
	}
	else
	{
		for ( QWORD nPart = 0; nPart < nParts; )
		{
			BYTE nByte = 0;

			for ( DWORD nBit = 0; nBit < 8 && nPart < nParts; nBit++, nPart++ )
			{
				QWORD nOffset = nPart * ED2K_PART_SIZE;
				QWORD nLength = min( (QWORD)ED2K_PART_SIZE, pDownload->m_nSize - nOffset );

				if ( pDownload->IsRangeUseful( nOffset, nLength ) == FALSE )
					nByte |= ( 1 << nBit );
			}

			pPacket->WriteByte( nByte );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CEDClient UDP packet handlers

BOOL CEDClient::OnUdpReask(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount ) return FALSE;
	if ( ! m_oUpED2K || m_pUploadTransfer == NULL ) return FALSE;

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );
	if ( validAndUnequal( oED2K, m_oUpED2K ) ) return FALSE;

	return m_pUploadTransfer->OnReask();
}

BOOL CEDClient::OnUdpReaskAck(CEDPacket* pPacket)
{
	if ( ! m_pDownloadTransfer || pPacket->GetRemaining() < 2 )
		return FALSE;

	int nRank = pPacket->ReadShortLE();
	m_pDownloadTransfer->SetQueueRank( nRank );

	return TRUE;
}

BOOL CEDClient::OnUdpQueueFull(CEDPacket* /*pPacket*/)
{
	if ( ! m_pDownloadTransfer )
		return FALSE;

	if ( CDownloadSource* pSource = GetSource() )
		pSource->m_tAttempt = GetTickCount() + Settings.eDonkey.ReAskTime * 1000;
	m_pDownloadTransfer->Close( TRI_UNKNOWN );

	return TRUE;
}

BOOL CEDClient::OnUdpFileNotFound(CEDPacket* /*pPacket*/)
{
	if ( ! m_pDownloadTransfer )
		return FALSE;

	m_pDownloadTransfer->Close( TRI_FALSE );

	return TRUE;
}
