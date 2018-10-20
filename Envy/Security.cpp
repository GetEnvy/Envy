//
// Security.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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
#include "Security.h"
#include "SecureRule.h"
#include "WndSecurity.h"	// Column enum
#include "EnvyFile.h"
#include "QuerySearch.h"
#include "LiveList.h"
//#include "Network.h"
#include "RegExp.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CSecurity Security;
CAdultFilter AdultFilter;
CMessageFilter MessageFilter;
CListLoader ListLoader;


//////////////////////////////////////////////////////////////////////
// CSecurity construction

CSecurity::CSecurity()
	: m_HashMap ()
	, m_bDenyPolicy ( FALSE )
{
}

CSecurity::~CSecurity()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule access

POSITION CSecurity::GetIterator() const
{
	return m_pRules.GetHeadPosition();
}

CSecureRule* CSecurity::GetNext(POSITION& pos) const
{
	return m_pRules.GetNext( pos );
}

INT_PTR CSecurity::GetCount() const
{
	return m_pRules.GetCount();
}

BOOL CSecurity::Check(CSecureRule* pRule) const
{
	CQuickLock oLock( m_pSection );

	return pRule != NULL && GetGUID( pRule->m_pGUID ) != NULL;
}

CSecureRule* CSecurity::GetGUID(const GUID& pGUID) const
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pRules.GetHeadPosition(); pos; )
	{
		CSecureRule* pRule = m_pRules.GetNext( pos );
		if ( pRule->m_pGUID == pGUID ) return pRule;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule modification

void CSecurity::Add(CSecureRule* pRule)
{
	{
		CQuickLock oLock( m_pSection );

		if ( pRule->m_nAction == CSecureRule::srDeny )
		{
			if ( pRule->m_nType == CSecureRule::srAddress )
			{
				pRule->MaskFix();
				if ( *(DWORD*)pRule->m_nMask == 0xffffffff )
					m_Cache.erase( *(DWORD*)pRule->m_nIP );
				else
					m_Cache.clear();

				if ( *(DWORD*)pRule->m_nMask == 0xffffffff && pRule->m_nExpire == CSecureRule::srIndefinite )
					SetAddressMap( *(DWORD*)pRule->m_nIP, SetRuleIndex( pRule ) );
			}
			else if ( pRule->m_nType == CSecureRule::srContentHash )
			{
				if ( pRule->m_nExpire == CSecureRule::srIndefinite )
					SetHashMap( pRule->GetContentWords(), SetRuleIndex( pRule ) );
			}
			else if ( pRule->m_nType == CSecureRule::srExternal )
			{
				ListLoader.AddList( pRule );
			}
		}

		CSecureRule* pExistingRule = GetGUID( pRule->m_pGUID );
		if ( pExistingRule == NULL )
		{
			m_pRules.AddHead( pRule );
		}
		else if ( pExistingRule != pRule )
		{
			*pExistingRule = *pRule;
			delete pRule;
		}
	}

	// Check all lists for newly denied hosts
	PostMainWndMessage( WM_SANITY_CHECK );
}

void CSecurity::Remove(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

	//if ( pRule->m_nType == CSecureRule::srExternal )
	//	ListLoader.Cancel( pRule );

	if ( POSITION pos = m_pRules.Find( pRule ) )
	{
		m_pRules.RemoveAt( pos );
	}

	for ( BYTE nIndex = (BYTE)m_pRuleIndexMap.size(); nIndex; nIndex-- )
	{
		if ( GetRuleByIndex( nIndex ) == pRule )
		{
			m_pRuleIndexMap[ nIndex ] = NULL;
			break;
		}
	}

	// This also accounts for double entries.
	//if ( pRule->m_nType == CSecureRule::srAddress )
	//{
	//	pRule->MaskFix();
	//
	//	if ( pRule->m_nType == CSecureRule::srAddress &&
	//		*(DWORD*)pRule->m_nMask == 0xffffffff )
	//	{
	//		CAddressRuleMap::const_iterator i = m_pIPRules.find( *(DWORD*)pRule->m_nIP );
	//		if ( i != m_pIPRules.end() )
	//			m_pIPRules.erase( i );
	//	}
	//}

	delete pRule;
}

void CSecurity::MoveUp(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

	POSITION posMe = m_pRules.Find( pRule );
	if ( posMe == NULL ) return;

	POSITION posOther = posMe;
	m_pRules.GetPrev( posOther );

	if ( posOther )
	{
		m_pRules.InsertBefore( posOther, pRule );
		m_pRules.RemoveAt( posMe );
	}
}

void CSecurity::MoveDown(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

	POSITION posMe = m_pRules.Find( pRule );
	if ( posMe == NULL ) return;

	POSITION posOther = posMe;
	m_pRules.GetNext( posOther );

	if ( posOther )
	{
		m_pRules.InsertAfter( posOther, pRule );
		m_pRules.RemoveAt( posMe );
	}
}

void CSecurity::Clear()
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_Complains.GetStartPosition(); pos; )
	{
		DWORD pAddress;
		CComplain* pComplain;
		m_Complains.GetNextAssoc( pos, pAddress, pComplain );
		delete pComplain;
	}
	m_Complains.RemoveAll();

	for ( POSITION pos = GetIterator(); pos; )
	{
		delete GetNext( pos );
	}
	m_pRules.RemoveAll();

	m_Cache.clear();
	m_AddressMap.clear();
	m_pRuleIndexMap.clear();
	for ( BYTE nType = 0; nType < urnLast; nType++ )
		m_HashMap[ nType ].clear();
}

BYTE CSecurity::SetRuleIndex(CSecureRule* pRule)
{
	const size_t nSize = m_pRuleIndexMap.size() + 1;

	for ( BYTE nTest = 1; nTest < (BYTE)nSize; nTest++ )
	{
		if ( m_pRuleIndexMap[ nTest ] == pRule )
			return nTest;
	}

	const BYTE nIndex = nSize <= 255 ? (BYTE)nSize : 0;		// Special case limit
	if ( nIndex && ( nIndex < 250 || pRule->m_nType == CSecureRule::srExternal ) )
		m_pRuleIndexMap[ nIndex ] = pRule;
	return nIndex;
}

CSecureRule* CSecurity::GetRuleByIndex(BYTE nIndex)
{
	return nIndex < 255 ? m_pRuleIndexMap[ nIndex ] : NULL;
}

void CSecurity::SetHashMap(CString sURN, BYTE nIndex)
{
	if ( ! nIndex || sURN.GetLength() < 36 )
		return;

	const CString strHash = sURN.Mid( sURN.ReverseFind( L':' ) + 1 );

	if ( StartsWith( sURN, _P( L"urn:sha1:" ) ) )
		m_HashMap[urnSHA][ strHash ] = nIndex;
	else if ( StartsWith( sURN, _P( L"urn:tree:" ) ) )
		m_HashMap[urnTiger][ strHash ] = nIndex;
	else if ( StartsWith( sURN, _P( L"urn:ed2k:" ) ) )
		m_HashMap[urnED2K][ strHash ] = nIndex;
	else if ( StartsWith( sURN, _P( L"urn:bth:" ) ) )
		m_HashMap[urnBTH][ strHash ] = nIndex;
	else if ( StartsWith( sURN, _P( L"urn:md5:" ) ) )
		m_HashMap[urnMD5][ strHash ] = nIndex;
}

BYTE CSecurity::GetHashMap(CString sURN)
{
	if ( sURN.GetLength() < 36 || ( sURN[0] != L'u' && sURN[0] != L'U' ) )
		return 0;

	const CString strHash = sURN.Mid( sURN.ReverseFind( L':' ) + 1 );

	if ( StartsWith( sURN, _P( L"urn:sha1:" ) ) )
		return m_HashMap[urnSHA].count( strHash ) ? m_HashMap[urnSHA][ strHash ] : 0;
	if ( StartsWith( sURN, _P( L"urn:tree:" ) ) )
		return m_HashMap[urnTiger].count( strHash ) ? m_HashMap[urnTiger][ strHash ] : 0;
	if ( StartsWith( sURN, _P( L"urn:ed2k:" ) ) )
		return m_HashMap[urnED2K].count( strHash ) ? m_HashMap[urnED2K][strHash ] : 0;
	if ( StartsWith( sURN, _P( L"urn:bth:" ) ) )
		return m_HashMap[urnBTH].count( strHash ) ? m_HashMap[urnBTH][ strHash ] : 0;
	if ( StartsWith( sURN, _P( L"urn:md5:" ) ) )
		return m_HashMap[urnMD5].count( strHash ) ? m_HashMap[urnMD5][ strHash ] : 0;

	return 0;
}


//////////////////////////////////////////////////////////////////////
// CSecurity ban

void CSecurity::Ban(const CEnvyFile* pFile, int nBanLength, BOOL bMessage)
{
	CQuickLock oLock( m_pSection );

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posCurrent = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posCurrent );
			delete pRule;
			continue;
		}

		if ( pRule->Match( pFile ) )			// Non-regexp name, hash, or size:ext:0000
		{
			if ( pRule->m_nAction == CSecureRule::srDeny )
			{
				if ( nBanLength == banWeek && ( pRule->m_nExpire < tNow + 604000 ) )
					pRule->m_nExpire = tNow + 604800;
				else if ( nBanLength == banCustom && ( pRule->m_nExpire < tNow + Settings.Security.DefaultBan + 3600 ) )
					pRule->m_nExpire = tNow + Settings.Security.DefaultBan + 3600;
				else if ( nBanLength == banForever && ( pRule->m_nExpire != CSecureRule::srIndefinite ) )
					pRule->m_nExpire = CSecureRule::srIndefinite;
				return;
			}
		}
	}

	CSecureRule* pRule = NewBanRule( nBanLength );

	if ( pFile->m_oSHA1 || pFile->m_oTiger || pFile->m_oED2K || pFile->m_oBTH || pFile->m_oMD5 )
	{
		pRule->m_nType = CSecureRule::srContentHash;
		pRule->SetContentWords(
			( pFile->m_oSHA1  ? pFile->m_oSHA1.toUrn()  + L" " : CString() ) +
			( pFile->m_oTiger ? pFile->m_oTiger.toUrn() + L" " : CString() ) +
			( pFile->m_oED2K  ? pFile->m_oED2K.toUrn()  + L" " : CString() ) +
			( pFile->m_oMD5   ? pFile->m_oMD5.toUrn()   + L" " : CString() ) +
			( pFile->m_oBTH   ? pFile->m_oBTH.toUrn()          : CString() ) );
	}

	Add( pRule );

	if ( bMessage && pFile )
		theApp.Message( MSG_NOTICE, IDS_SECURITY_ADDED, (LPCTSTR)pFile->m_sName );
}

void CSecurity::Ban(const IN_ADDR* pAddress, int nBanLength, BOOL bMessage, LPCTSTR szComment)
{
	CQuickLock oLock( m_pSection );

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posCurrent = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posCurrent );
			delete pRule;
			continue;
		}

		if ( pRule->Match( pAddress ) && pRule->m_nAction == CSecureRule::srDeny )
		{
			if ( nBanLength == banWeek && ( pRule->m_nExpire < tNow + 604000 ) )
				pRule->m_nExpire = tNow + 604800;
			else if ( nBanLength == banCustom && ( pRule->m_nExpire < tNow + Settings.Security.DefaultBan + 3600 ) )
				pRule->m_nExpire = tNow + Settings.Security.DefaultBan + 3600;
			else if ( nBanLength == banForever && ( pRule->m_nExpire != CSecureRule::srIndefinite ) )
				pRule->m_nExpire = CSecureRule::srIndefinite;
			else if ( bMessage && pAddress )
				theApp.Message( MSG_NOTICE, IDS_SECURITY_EXISTS, (LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
			return;
		}
	}

	CSecureRule* pRule = NewBanRule( nBanLength, szComment );
	pRule->m_nType = CSecureRule::srAddress;

	CopyMemory( pRule->m_nIP, pAddress, sizeof pRule->m_nIP );

	Add( pRule );

	if ( bMessage )
		theApp.Message( MSG_NOTICE, IDS_SECURITY_ADDED, (LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
}

CSecureRule* CSecurity::NewBanRule(int nBanLength, CString sComment) const
{
	CSecureRule* pRule	= new CSecureRule();
	pRule->m_nAction	= CSecureRule::srDeny;

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	switch ( nBanLength )
	{
	case banSession:
		pRule->m_nExpire	= CSecureRule::srSession;
		pRule->m_sComment	= L"Quick Ban";
		break;
	case ban5Mins:
		pRule->m_nExpire	= tNow + 300;
		pRule->m_sComment	= L"Temp Ignore";
		break;
	case ban30Mins:
		pRule->m_nExpire	= tNow + 1800;
		pRule->m_sComment	= L"Temp Ignore";
		break;
	case ban2Hours:
		pRule->m_nExpire	= tNow + 7200;
		pRule->m_sComment	= L"Temp Ignore";
		break;
	case banWeek:
		pRule->m_nExpire	= tNow + 604800;
		pRule->m_sComment	= L"Client Block";
		break;
	case banCustom:
		pRule->m_nExpire	= tNow + Settings.Security.DefaultBan + 3600;
		pRule->m_sComment	= L"Ban";
		break;
	case banForever:
	default:
		pRule->m_nExpire	= CSecureRule::srIndefinite;
		pRule->m_sComment	= L"Ban";
		break;
	}

	if ( ! sComment.IsEmpty() )
		pRule->m_sComment = sComment;

	return pRule;
}


//////////////////////////////////////////////////////////////////////
// CSecurity complain

bool CSecurity::Complain(const IN_ADDR* pAddress, int nBanLength, int nExpire, int nCount)
{
	CQuickLock oLock( m_pSection );

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CComplain* pComplain = NULL;
	if ( m_Complains.Lookup( pAddress->s_addr, pComplain ) )
	{
		if ( pComplain->m_nExpire < tNow )
		{
			pComplain->m_nScore = 1;
		}
		else
		{
			pComplain->m_nScore ++;
			if ( pComplain->m_nScore > nCount )
			{
				m_Complains.RemoveKey( pAddress->s_addr );
				delete pComplain;
				Ban( pAddress, nBanLength );
				return true;
			}
		}
	}
	else
	{
		pComplain = new CComplain;
		pComplain->m_nScore = 1;
		m_Complains.SetAt( pAddress->s_addr, pComplain );
	}

	pComplain->m_nExpire = tNow + nExpire;

	return false;
}

//////////////////////////////////////////////////////////////////////
// CSecurity access checks

BOOL CSecurity::IsDenied(const IN_ADDR* pAddress)
{
	{
		CQuickLock oLock( m_pSection );

		if ( m_Cache.count( *(DWORD*)pAddress ) )		// Rare crash if unlocked
			return m_bDenyPolicy;
			//theApp.Message( MSG_DEBUG, L"Skipped Repeat IP Security Check  (%i Cached)", m_Cache.size() );
	}

	if ( BYTE nIndex = GetAddressMap( *(DWORD*)pAddress ) )
	{
		if ( CSecureRule* pRule = m_pRuleIndexMap[ nIndex ] )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;
			if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
		}
	}

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
			continue;
		}

		if ( pRule->Match( pAddress ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			// Add 5 min penalty for early access
			if ( pRule->m_nExpire > CSecureRule::srSession &&
				 pRule->m_nExpire < tNow + 300 )
				pRule->m_nExpire = tNow + 300;

			if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
		}
	}

	m_Cache.insert( *(DWORD*)pAddress );	// Skip future lookups

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(LPCTSTR pszContent)
{
	if ( CString(pszContent).GetLength() > 30 && StartsWith( (CString)pszContent, (LPCTSTR)_P( L"urn:" ) ) )
	{
		if ( BYTE nIndex = GetHashMap( pszContent ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

		return m_bDenyPolicy;
	}

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pszContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			// Add 5 min penalty for early access
			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < tNow + 300 )
				pRule->m_nExpire = tNow + 300;

			if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(const CEnvyFile* pFile)
{
	if ( pFile->m_oSHA1 && ! m_HashMap[urnSHA].empty() )
		if ( BYTE nIndex = GetHashMap( pFile->m_oSHA1.toUrn() ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

	if ( pFile->m_oTiger && ! m_HashMap[urnTiger].empty() )
		if ( BYTE nIndex = GetHashMap( pFile->m_oTiger.toUrn() ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

	if ( pFile->m_oED2K && ! m_HashMap[urnED2K].empty() )
		if ( BYTE nIndex = GetHashMap( pFile->m_oED2K.toUrn() ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

	if ( pFile->m_oBTH && ! m_HashMap[urnBTH].empty() )
		if ( BYTE nIndex = GetHashMap( pFile->m_oBTH.toUrn() ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

	if ( pFile->m_oMD5 && ! m_HashMap[urnMD5].empty() )
		if ( BYTE nIndex = GetHashMap( pFile->m_oMD5.toUrn() ) )
		{
			if ( CSecureRule* pRule = GetRuleByIndex( nIndex ) )
			{
				pRule->m_nToday ++;
				pRule->m_nEver ++;
				if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
				if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			}
		}

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pFile ) )	// Non-regexp name, hash, or size:ext:0000
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			// Add 5 min penalty for early access
			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < tNow + 300 )
				pRule->m_nExpire = tNow + 300;

			if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(const CQuerySearch* pQuery, const CString& strContent)
{
	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pQuery, strContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nAction == CSecureRule::srDeny )   return TRUE;
			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsFlood(const IN_ADDR* pAddress, const LPCTSTR pszVendor /*NULL*/, PROTOCOLID nProtocol /*PROTOCOL_NULL*/)
{
	if ( nProtocol == PROTOCOL_G2 )
	{
		CString strCode = theApp.GetCountryCode( *pAddress );
		if ( StartsWith( strCode, _P( L"TW" ) ) ||
			 StartsWith( strCode, _P( L"HK" ) ) ||
			 StartsWith( strCode, _P( L"CN" ) ) )
		{
			if ( ! pszVendor || ! *pszVendor ||
				 _tcsicmp( pszVendor, L"FOXY" ) == 0 ||
				 _tcsicmp( pszVendor, L"RAZB" ) == 0 )
				return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSecurity expire

void CSecurity::Expire()
{
	CQuickLock oLock( m_pSection );

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = m_Complains.GetStartPosition(); pos; )
	{
		DWORD pAddress;
		CComplain* pComplain;
		m_Complains.GetNextAssoc( pos, pAddress, pComplain );
		if ( pComplain->m_nExpire < tNow )
		{
			m_Complains.RemoveKey( pAddress );
			delete pComplain;
		}
	}

	for ( POSITION pos = GetIterator(); pos; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity load and save

BOOL CSecurity::Load()
{
	const CString strFile = Settings.General.DataPath + L"Security.dat";

	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load, 131072 );	// 128 KB buffer
			try
			{
				CQuickLock oLock( m_pSection );

				Serialize( ar );

				ar.Close();

				pFile.Close();

				return TRUE;	// Success
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
			}
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
		}

		pFile.Close();
	}

	theApp.Message( MSG_ERROR, L"Failed to load security rules: %s", (LPCTSTR)strFile );
	return FALSE;
}

BOOL CSecurity::Save()
{
	const CString strFile = Settings.General.DataPath + L"Security.dat";
	const CString strTemp = Settings.General.DataPath + L"Security.tmp";

	CFile pFile;
	if ( pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::store, 131072 );	// 128 KB buffer
			try
			{
				{
					CQuickLock oLock( m_pSection );

					Serialize( ar );
					ar.Close();
				}

				pFile.Close();

				if ( MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
					return TRUE;	// Success
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
			}
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
		}

		pFile.Close();
		DeleteFile( strTemp );
	}

	theApp.Message( MSG_ERROR, L"Failed to save security rules: %s", (LPCTSTR)strFile );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSecurity serialize

// Set at INTERNAL_VERSION on change:
#define SECURITY_SER_VERSION 1

// nVersion History:
// 5 - Extended security rule type (ryo-oh-ki)
// 1000 - Added banCustom
// 1 - (Envy 1.0)

void CSecurity::Serialize(CArchive& ar)
{
	int nVersion = SECURITY_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar << m_bDenyPolicy;

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator(); pos; )
		{
			GetNext( pos )->Serialize( ar, nVersion );
		}

		// Unimplemented
		//for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i )
		//{
		//	(*i).second->Serialize( ar, nVersion );
		//}
	}
	else // Loading
	{
		Clear();

		ar >> nVersion;
		ar >> m_bDenyPolicy;

		const DWORD tNow = static_cast< DWORD >( time( NULL ) );

		for ( DWORD_PTR nCount = ar.ReadCount(); nCount > 0; nCount-- )
		{
			CSecureRule* pRule = new CSecureRule( FALSE );
			pRule->Serialize( ar, nVersion );

			if ( pRule->IsExpired( tNow, TRUE ) )
			{
				delete pRule;
				continue;
			}

			// Special handling for single-IP security rules
			if ( pRule->m_nType == CSecureRule::srAddress &&
				 pRule->m_nAction == CSecureRule::srDeny &&
				*(DWORD*)pRule->m_nMask == 0xffffffff )
			{
				SetAddressMap( *(DWORD*)pRule->m_nIP, SetRuleIndex( pRule ) );
				continue;
			}

			if ( pRule->m_nType == CSecureRule::srContentHash &&
				 pRule->m_nAction == CSecureRule::srDeny )
			{
				SetHashMap( pRule->GetContentWords(), SetRuleIndex( pRule ) );
				continue;
			}

			if ( pRule->m_nType == CSecureRule::srExternal )
				ListLoader.AddList( pRule );

			m_pRules.AddTail( pRule );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity XML

LPCTSTR CSecurity::xmlns = L"http://schemas.getenvy.com/Security.xsd";

CXMLElement* CSecurity::ToXML(BOOL bRules)
{
	CXMLElement* pXML = new CXMLElement( NULL, L"security" );
	pXML->AddAttribute( L"xmlns", CSecurity::xmlns );

	if ( bRules )
	{
		for ( POSITION pos = GetIterator(); pos; )
		{
			pXML->AddElement( GetNext( pos )->ToXML() );
		}
	}

	return pXML;
}

BOOL CSecurity::FromXML(const CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( L"security" ) ) return FALSE;

	int nCount = 0;

	for ( POSITION pos = pXML->GetElementIterator(); pos; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );

		if ( pElement->IsNamed( L"rule" ) )
		{
			CQuickLock oLock( m_pSection );
			CSecureRule* pRule	= NULL;
			CString strGUID		= pElement->GetAttributeValue( L"guid" );
			BOOL bExisting		= FALSE;
			GUID pGUID;

			if ( Hashes::fromGuid( strGUID, &pGUID ) )
			{
				if ( ( pRule = GetGUID( pGUID ) ) != NULL )
					bExisting = TRUE;

				if ( pRule == NULL )
				{
					pRule = new CSecureRule( FALSE );
					pRule->m_pGUID = pGUID;
				}
			}
			else
			{
				pRule = new CSecureRule();
			}

			if ( pRule->FromXML( pElement ) )
			{
				if ( ! bExisting )
					m_pRules.AddTail( pRule );

				nCount++;
			}
			else
			{
				if ( ! bExisting )
					delete pRule;
			}
		}
	}

	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CSecurity import

BOOL CSecurity::Import(LPCTSTR pszFile)
{
	CBuffer pBuffer;
	CFile pFile;

	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength, TRUE );
	BOOL bResult = FALSE;

	if ( pXML != NULL )
	{
		bResult = FromXML( pXML );
		delete pXML;
	}
	else
	{
		CString strLine;

		while ( pBuffer.ReadLine( strLine ) )
		{
			strLine.Trim();
			if ( strLine.IsEmpty() ) continue;
			if ( strLine.GetAt( 0 ) == L';' ) continue;

			CSecureRule* pRule = new CSecureRule();

			if ( pRule->FromGnucleusString( strLine ) )
			{
				CQuickLock oLock( m_pSection );
				m_pRules.AddTail( pRule );
				bResult = TRUE;
			}
			else
			{
				delete pRule;
			}
		}
	}

	// Check all lists for newly denied hosts
	PostMainWndMessage( WM_SANITY_CHECK );

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CSecurity.IsClientBad/IsClientBanned
//
// Check user agent for troublemakers

BOOL CSecurity::IsClientBad(const CString& sUserAgent) const
{
	// No user agent: Assume bad. (Allowed connection but no searches performed)
	if ( sUserAgent.IsEmpty() )									return TRUE;

	// Envy Fakes
	if ( StartsWith( sUserAgent, _P( L"Envy" ) ) )
	{
		LPCTSTR szVersion = sUserAgent;
		szVersion += 11;
		if ( _tcsistr( sUserAgent, L" 1." ) )					return FALSE;
		if ( _tcsistr( sUserAgent, L" 2.0" ) )					return FALSE;

		return TRUE;
	}

	// Shareaza Fakes
	if ( StartsWith( sUserAgent, _P( L"Shareaza" ) ) )
	{
		LPCTSTR szVersion = sUserAgent;
		szVersion += 8;
		if ( _tcsistr( sUserAgent, L" 2.0" ) )					return TRUE;
		if ( _tcsistr( sUserAgent, L" 2." ) )					return FALSE;
		if ( _tcsistr( sUserAgent, L"Plus" ) )					return FALSE;

		return TRUE;
	}

	// Dianlei: Shareaza rip-off
	// Based on Alpha code, need verification for current 1.x status
	if ( StartsWith( sUserAgent, _P( L"Dianlei" ) ) )
	{
		LPCTSTR szVersion = sUserAgent;
		szVersion += 7;
		if ( _tcsistr( szVersion, L" 0." ) )					return TRUE;

		return FALSE;
	}

	// BearShare Selectivity
	if ( StartsWith( sUserAgent, _P( L"BearShare" ) ) )
	{
		LPCTSTR szVersion = sUserAgent;
		szVersion += 9;
		if ( _tcsistr( szVersion, L" 4." ) )					return FALSE;
		if ( _tcsistr( szVersion, L" 5." ) )					return FALSE;

		return TRUE;
	}

	// Any iMesh
	if ( StartsWith( sUserAgent, _P( L"iMesh" ) ) )				return TRUE;

	// Other Miscellaneous
	if ( StartsWith( sUserAgent, _P( L"Trilix" ) ) )			return TRUE;
	if ( StartsWith( sUserAgent, _P( L"Gnutella Turbo" ) ) )	return TRUE;
	if ( StartsWith( sUserAgent, _P( L"Mastermax" ) ) )			return TRUE;
	if ( StartsWith( sUserAgent, _P( L"Fildelarprogram" ) ) )	return TRUE;
	if ( StartsWith( sUserAgent, _P( L"Fastload.TV" ) ) )		return TRUE;

	// Other GPL Violaters, Etc.
	if ( StartsWith( sUserAgent, _P( L"K-Lite" ) ) )			return TRUE;
	if ( StartsWith( sUserAgent, _P( L"SlingerX" ) ) )			return TRUE;
	if ( StartsWith( sUserAgent, _P( L"C -3.0.1" ) ) )			return TRUE;
	if ( StartsWith( sUserAgent, _P( L"vagaa" ) ) )				return TRUE;
	if ( StartsWith( sUserAgent, _P( L"mxie" ) ) )				return TRUE;
	if ( StartsWith( sUserAgent, _P( L"WinMX" ) ) )				return TRUE;
	if ( StartsWith( sUserAgent, _P( L"eTomi" ) ) )				return TRUE;

	// Unknown: Assume OK
	return FALSE;
}

// Checks the user agent to see if it's a leecher or banned client
BOOL CSecurity::IsClientBanned(const CString& sUserAgent)
{
	// No user agent- assume OK
	if ( sUserAgent.IsEmpty() )
		return FALSE;

	// Foxy (Private G2)
	if ( _tcsistr( sUserAgent, L"Foxy" ) )						return TRUE;

	// i2hub leecher client. (Tested, does not upload)
	if ( _tcsistr( sUserAgent, L"i2hub" ) )						return TRUE;

	// Check by content filter
	// ToDo: Implement user agent filter type
	return IsDenied( sUserAgent );
}

BOOL CSecurity::IsAgentBlocked(const CString& sUserAgent) const
{
	// The remote computer didn't send a "User-Agent", or it sent whitespace
	if ( sUserAgent.IsEmpty() ||
		CString( sUserAgent ).Trim().IsEmpty() )				return TRUE;

	// Loop through the user-defined list of programs to block
	for ( string_set::const_iterator i = Settings.Uploads.BlockAgents.begin();
		i != Settings.Uploads.BlockAgents.end(); i++ )
	{
		if ( _tcsistr( sUserAgent, *i ) )						return TRUE;
	}

	// Allow it
	return FALSE;
}

BOOL CSecurity::IsVendorBlocked(const CString& sVendor) const
{
	// Foxy (Private G2)
	if ( _tcsistr( sVendor, L"foxy" ) )							return TRUE;

	// Allow it
	return FALSE;
}

CLiveList* CSecurity::GetList() const
{
	CQuickLock oLock( m_pSection );
	CLiveList* pLiveList = new CLiveList( COL_SECURITY_LAST, GetCount() + GetCount() / 4u );

	if ( CLiveItem* pDefault = pLiveList->Add( (LPVOID)0 ) )
	{
		pDefault->Set( COL_SECURITY_NUM, L" - " );									// Need leading space for proper sort priority (until sorting is fixed)
		pDefault->Set( COL_SECURITY_CONTENT, LoadString( IDS_SECURITY_DEFAULT ) );	// "Default Policy"
		pDefault->Set( COL_SECURITY_ACTION,  LoadString( Security.m_bDenyPolicy ? IDS_SECURITY_DENY : IDS_SECURITY_ACCEPT ) );
		pDefault->SetImage( Security.m_bDenyPolicy ? Settings.General.LanguageRTL ? 0 : 2 : 1 );
	}

	const DWORD tNow = static_cast< DWORD >( time( NULL ) );

	int nCount = 1;
	for ( POSITION pos = GetIterator(); pos; ++nCount )
	{
		GetNext( pos )->ToList( pLiveList, nCount, tNow );
	}

	// Unimplemented separate IP address rules
	//for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i, ++nCount )
	//{
	//	(*i).second->ToList( pLiveList, nCount, tNow );
	//}

	return pLiveList;
}


//////////////////////////////////////////////////////////////////////
// CAdultFilter construction

CAdultFilter::CAdultFilter()
	: m_pszBlockedWords	( NULL )
	, m_pszDubiousWords	( NULL )
	, m_pszChildWords	( NULL )
{
}

CAdultFilter::~CAdultFilter()
{
	if ( m_pszBlockedWords ) delete [] m_pszBlockedWords;
	m_pszBlockedWords = NULL;

	if ( m_pszDubiousWords ) delete [] m_pszDubiousWords;
	m_pszDubiousWords = NULL;

	if ( m_pszChildWords ) delete [] m_pszChildWords;
	m_pszChildWords = NULL;
}

void CAdultFilter::Load()
{
	CFile pFile;
	CString strBlockedWords, strDubiousWords, strChildWords;
	const CString strFile = Settings.General.DataPath + L"AdultFilter.dat";

	// Delete current adult filters (if present)
	if ( m_pszBlockedWords ) delete [] m_pszBlockedWords;
	m_pszBlockedWords = NULL;

	if ( m_pszDubiousWords ) delete [] m_pszDubiousWords;
	m_pszDubiousWords = NULL;

	if ( m_pszChildWords ) delete [] m_pszChildWords;
	m_pszChildWords = NULL;

	// Load the adult filter from disk
	if ( pFile.Open( strFile, CFile::modeRead ) )
	{
		try
		{
			CBuffer pBuffer;
			const DWORD nLen = (DWORD)pFile.GetLength();
			if ( ! pBuffer.EnsureBuffer( nLen ) )
				AfxThrowUserException();

			pBuffer.m_nLength = nLen;
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			pBuffer.ReadLine( strBlockedWords );	// Line 1: words that are blocked
			if ( ! strBlockedWords.IsEmpty() && strBlockedWords.GetAt( 0 ) == '#' )
				strBlockedWords.Empty();
			pBuffer.ReadLine( strDubiousWords );	// Line 2: words that may be okay
			if ( ! strDubiousWords.IsEmpty() && strDubiousWords.GetAt( 0 ) == '#' )
				strDubiousWords.Empty();
			pBuffer.ReadLine( strChildWords );		// Line 3: words for child pornography
			if ( ! strChildWords.IsEmpty() && strChildWords.GetAt( 0 ) == '#' )
				strChildWords.Empty();
		}
		catch ( CException* pException )
		{
			if ( pFile.m_hFile != CFile::hFileNull )
				pFile.Close();	// File is still open so close it
			pException->Delete();
		}
	}

	// Insert some defaults if the load failed
	if ( strBlockedWords.IsEmpty() )
		strBlockedWords = L"xxx porn fuck cock cunt vagina pussy nude naked boobs breast hentai "
						  L"lesbian whore shit rape preteen hardcore lolita playboy penthouse "
						  L"topless r-rated x-rated dildo pr0n erotic sexy orgasm nipple fetish "
						  L"upskirt beastiality bestiality pedofil necrofil tits lolicon shemale fisting";
	if ( strDubiousWords.IsEmpty() )
		strDubiousWords = L"ass sex anal gay teen thong babe bikini viagra dick cum sluts";

	if ( strChildWords.IsEmpty() )
		strChildWords = L"child preteen";

	// Load the blocked words into the Adult Filter
	if ( strBlockedWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strBlockedWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strBlockedWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strBlockedWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszBlockedWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszBlockedWords;

		for ( POSITION pos = pWords.GetHeadPosition(); pos; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load the possibly blocked words into the Adult Filter
	if ( strDubiousWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strDubiousWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strDubiousWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strDubiousWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszDubiousWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszDubiousWords;

		for ( POSITION pos = pWords.GetHeadPosition(); pos; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load child pornography words into the Adult Filter
	if ( strChildWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strChildWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strChildWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strChildWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszChildWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszChildWords;

		for ( POSITION pos = pWords.GetHeadPosition(); pos; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}
}

BOOL CAdultFilter::IsHitAdult(LPCTSTR pszText) const
{
	if ( pszText )
		return IsFiltered( pszText );

	return FALSE;
}

BOOL CAdultFilter::IsSearchFiltered(LPCTSTR pszText) const
{
	if ( Settings.Search.AdultFilter && pszText )
		return IsFiltered( pszText );

	return FALSE;
}

BOOL CAdultFilter::IsChatFiltered(LPCTSTR pszText) const
{
	if ( Settings.Community.ChatCensor && pszText )
		return IsFiltered( pszText );

	return FALSE;
}

BOOL CAdultFilter::Censor(CString& sText) const
{
	if ( sText.GetLength() < 3 )
		return FALSE;

	BOOL bModified = FALSE;

	// Check and replace blocked words
	for ( LPCTSTR pszWord = m_pszBlockedWords; pszWord && *pszWord; )
	{
		int nWordLen = (int)_tcslen( pszWord );
		if ( ReplaceNoCase( sText, pszWord, CString( L'*', nWordLen ) ) )
			bModified = TRUE;
		pszWord += nWordLen + 1;
	}

	return bModified;
}

BOOL CAdultFilter::IsChildPornography(LPCTSTR pszText) const
{
	if ( ! pszText )
		return FALSE;

	for ( LPCTSTR pszWord = m_pszChildWords; *pszWord; )
	{
		if ( _tcsistr( pszText, pszWord ) != NULL )
			return ( IsFiltered( pszText ) );
		pszWord += _tcslen( pszWord ) + 1;
	}

	return FALSE;
}

BOOL CAdultFilter::IsFiltered(LPCTSTR pszText) const
{
	if ( ! pszText )
		return FALSE;

	// Check blocked words
	if ( m_pszBlockedWords )
	{
		for ( LPCTSTR pszWord = m_pszBlockedWords; *pszWord; )
		{
			if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
			pszWord += _tcslen( pszWord ) + 1;
		}
	}

	// Check dubious words
	if ( m_pszDubiousWords )
	{
		size_t nDubiousWords = 0, nWordsPermitted = min( (DWORD)(_tcslen( pszText ) / 8), 4 );

		for ( LPCTSTR pszWord = m_pszDubiousWords; *pszWord; )
		{
			if ( _tcsistr( pszText, pszWord ) != NULL ) nDubiousWords++;
			if ( nDubiousWords > nWordsPermitted ) return TRUE;
			pszWord += _tcslen( pszWord ) + 1;
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CMessageFilter construction

CMessageFilter::CMessageFilter()
{
	m_pszED2KSpam = NULL;
	m_pszFilteredPhrases = NULL;
}

CMessageFilter::~CMessageFilter()
{
	if ( m_pszED2KSpam ) delete [] m_pszED2KSpam;
	m_pszED2KSpam = NULL;

	if ( m_pszFilteredPhrases ) delete [] m_pszFilteredPhrases;
	m_pszFilteredPhrases = NULL;
}

void CMessageFilter::Load()
{
	CFile pFile;
	CString strFilteredPhrases, strED2KSpamPhrases;
	const CString strFile = Settings.General.DataPath + L"MessageFilter.dat";

	// Delete current filter (if present)
	if ( m_pszFilteredPhrases ) delete [] m_pszFilteredPhrases;
	m_pszFilteredPhrases = NULL;

	// Load the message filter from disk
	if ( pFile.Open( strFile, CFile::modeRead ) )
	{
		try
		{
			CBuffer pBuffer;
			DWORD nLen = (DWORD)pFile.GetLength();
			if ( ! pBuffer.EnsureBuffer( nLen ) )
				AfxThrowUserException();

			pBuffer.m_nLength = nLen;
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			pBuffer.ReadLine( strED2KSpamPhrases );
			pBuffer.ReadLine( strFilteredPhrases );
		}
		catch ( CException* pException )
		{
			if ( pFile.m_hFile != CFile::hFileNull )
				pFile.Close();		// If file is still open close it
			pException->Delete();
		}
	}

	// Insert some defaults if there was a read error

	if ( strED2KSpamPhrases.IsEmpty() )
		strED2KSpamPhrases = L"Your client is connecting too fast|Join the L33cher Team|PeerFactor|Your client is making too many connections|ZamBoR 2|AUTOMATED MESSAGE:|eMule FX the BEST eMule ever|DI-Emule";

	//if ( strFilteredPhrases.IsEmpty() )
	//	strFilteredPhrases = L"";

	// Load the ED2K spam into the filter
	if ( strED2KSpamPhrases.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strED2KSpamPhrases;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr; nPos++, pszPtr++ )
		{
			if ( *pszPtr == '|' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strED2KSpamPhrases.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strED2KSpamPhrases.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszED2KSpam = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszED2KSpam;

		for ( POSITION pos = pWords.GetHeadPosition(); pos; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load the blocked strings into the filter
	if ( strFilteredPhrases.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strFilteredPhrases;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr; nPos++, pszPtr++ )
		{
			if ( *pszPtr == '|' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strFilteredPhrases.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strFilteredPhrases.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszFilteredPhrases = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszFilteredPhrases;

		for ( POSITION pos = pWords.GetHeadPosition(); pos; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof( TCHAR ) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}
}

BOOL CMessageFilter::IsED2KSpam( LPCTSTR pszText )
{
	if ( ! Settings.Community.ChatFilterED2K || ! m_pszED2KSpam || ! pszText )
		return FALSE;

	// Check for Ed2K spam phrases
	for ( LPCTSTR pszWord = m_pszED2KSpam; *pszWord; )
	{
		if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
		pszWord += _tcslen( pszWord ) + 1;
	}

	return FALSE;
}

BOOL CMessageFilter::IsFiltered(LPCTSTR pszText)
{
	if ( ! Settings.Community.ChatFilter|| ! m_pszFilteredPhrases || ! pszText )
		return FALSE;

	// Check for filtered (spam) phrases
	for ( LPCTSTR pszWord = m_pszFilteredPhrases; *pszWord; )
	{
		if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
		pszWord += _tcslen( pszWord ) + 1;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CListLoader construction

CListLoader::CListLoader()
{
}

CListLoader::~CListLoader()
{
	if ( IsThreadAlive() )
		CloseThread();
}

//void CListLoader::Cancel(CSecureRule* pRule)
//{
//	if ( ! IsThreadEnabled() )
//		return;
//
//	if ( pRule == m_pQueue.GetHead() )
//	{
//		StopThread();
//		m_pQueue.RemoveHead();
//		Sleep( 10 );
//		if ( m_pQueue.GetCount() )
//			BeginThread( "ListLoader" );
//		return;
//	}
//
//	if ( POSITION pos = m_pQueue.Find( pRule ) )
//		m_pQueue.RemoveAt( pos );
//}

void CListLoader::AddList(CSecureRule* pRule)
{
	ASSERT( pRule->m_nType == CSecureRule::srExternal );
	if ( ! pRule || pRule->m_nType != CSecureRule::srExternal || ! pRule->m_pContent )
		return;

	if ( ! m_pQueue.Find( pRule ) )
		m_pQueue.AddTail( pRule );

	if ( ! IsThreadAlive() )
		BeginThread( "ListLoader" );
}

void CListLoader::OnRun()
{
	BOOL bUpdate = FALSE;
	while ( IsThreadEnabled() && m_pQueue.GetCount() )
	{
		CSecureRule* pRule = m_pQueue.GetHead();

		if ( ! pRule || ! pRule->m_pContent || pRule->m_nType != CSecureRule::srExternal )
		{
			m_pQueue.RemoveHead();
			continue;
		}

		CString strPath = pRule->GetContentWords();
		if ( strPath.GetLength() < 6 )
		{
			m_pQueue.RemoveHead();
			continue;
		}

		CString strCommentBase = pRule->m_sComment;
		if ( strCommentBase.IsEmpty() )
			strCommentBase = L"• %u";
		else if ( strCommentBase.ReverseFind( L'•' ) >= 0 )
			strCommentBase = strCommentBase.Left( strCommentBase.ReverseFind( L'•' ) + 1 ) + L" %u";
		else
			strCommentBase += L"  • %u";

		if ( strPath[1] != L':' )
			strPath = Settings.General.DataPath + strPath;

		CFile pFile;
		if ( ! pFile.Open( (LPCTSTR)strPath, CFile::modeRead ) )	// .GetBuffer() ?
		{
			m_pQueue.RemoveHead();
			continue;
		}

		const BYTE nIndex = Security.SetRuleIndex( pRule );

		try
		{
			CBuffer pBuffer;
			const DWORD nLength = pFile.GetLength();
			pBuffer.EnsureBuffer( nLength );
			pBuffer.m_nLength = nLength;
			pFile.Read( pBuffer.m_pBuffer, nLength );
			pFile.Close();

			// Format: Delineated Lists

			CString strLine, strURN;
			DWORD nCount = 0;
			int nPos;

//TIMER_START
			while ( pBuffer.ReadLine( strLine ) && IsThreadEnabled() && pRule )
			{
				strLine.TrimRight();

				if ( strLine.GetLength() < 7 )
					continue;										// Blank/Invalid line

				if ( strLine[ 0 ] == L'#' )
				{
					if ( strLine[ strLine.GetLength() - 1 ] == L':' && strLine.Find( L"urn:" ) > 0 )
						strURN = strLine.Mid( strLine.Find( L"urn:" ) );		// Default "# urn:type:"
					continue;										// Comment line
				}

				if ( strLine[ 0 ] < L'0' || strLine[ 0 ] > L'z' )	// Whitespace/Chars
					continue;										// Invalid line

				// Limit CPU:
				if ( ++nCount % 10 == 0 )
				{
					if ( pRule->m_sComment.IsEmpty() )
						strCommentBase = L"• %u";
					else if ( pRule->m_sComment.ReverseFind( L'•' ) < 0 )
						strCommentBase = pRule->m_sComment + L"  • %u";

					pRule->m_sComment.Format( strCommentBase, nCount );
					Sleep( 1 );
				}

				// Hashes:

				if ( ( ! strURN.IsEmpty() && strLine.Find( L'.', 5 ) < 0 ) || StartsWith( strLine, _P( L"urn:" ) ) )
				{
					nPos = strLine.FindOneOf( L" \t" );
					if ( nPos > 0 )
						strLine.Truncate( nPos );					// Trim at whitespace (remove any trailing comments)
					if ( ! strURN.IsEmpty() && ! StartsWith( strLine, _P( L"urn:" ) ) )
						strLine = strURN + strLine;					// Default "urn:type:" prepended
					if ( strLine.GetLength() > 35 )
						Security.SetHashMap( strLine, nIndex );
					else
						nCount--;
					continue;
				}

				// IPs:

				nPos = strLine.ReverseFind( L':' );
				if ( nPos > 0 )
					strLine = strLine.Mid( nPos + 1 );				// Remove leading comment for some formats

				nPos = strLine.FindOneOf( L"\t," );
				if ( nPos > 0 )
					strLine.Truncate( nPos );						// Trim at whitespace (remove any trailing comments)

				if ( strLine.GetLength() < 7 || strLine[0] > L'9' || strLine.Find( L'.' ) < 1 )
				{
					nCount--;
					continue;
				}

				nPos = strLine.Find( L'-' );						// Possible Range
				if ( nPos < 0 )										// Single IP
				{
					Security.SetAddressMap( IPStringToDWORD( strLine, TRUE ), nIndex );
					continue;
				}

				CString strFirst = strLine.Left( nPos ).Trim();
				CString strLast  = strLine.Mid( nPos + 1 ).Trim();

				if ( strFirst == strLast )
				{
					Security.SetAddressMap( IPStringToDWORD( strLine, TRUE ), nIndex );
					continue;
				}

				// inet_addr( CT2CA( (LPCTSTR)strLast )
				DWORD nFirst = IPStringToDWORD( strFirst, FALSE );
				DWORD nLast  = IPStringToDWORD( strLast, FALSE );

				if ( nFirst < 10 || nFirst >= 0xE0000000 )			// 0 or "0.0." or "224-255"
					continue;		// Redundant/Invalid

				//if ( Network.IsReserved( (IN_ADDR*)nFirst ) )		// Crash
				//if ( StartsWith( strFirst, _P( L"0.0" ) ) ||
				//	 StartsWith( strFirst, _P( L"6.0" ) ) ||
				//	 StartsWith( strFirst, _P( L"7.0" ) ) ||
				//	 StartsWith( strFirst, _P( L"11.0" ) ) ||
				//	 StartsWith( strFirst, _P( L"55.0" ) ) ||
				//	 StartsWith( strFirst, _P( L"127.0" ) ) )
				//	continue;		// Redundant

				for ( DWORD nRange = Settings.Security.ListRangeLimit; nFirst <= nLast && nRange; nFirst++, nRange-- )
				{
					Security.SetAddressMap( htonl( nFirst ), nIndex );	// Reverse host-byte order
				}
			}

			if ( pRule )
				pRule->m_sComment.Format( strCommentBase, nCount );		// Final update
			if ( nCount )
				bUpdate = TRUE;
//TIMER_STOP
		}
		catch ( CException* pException )
		{
			if ( pFile.m_hFile != CFile::hFileNull )
				pFile.Close();	// File is still open so close it
			pException->Delete();
		}

		m_pQueue.RemoveHead();	// Done
	}

	if ( ! IsThreadEnabled() )
		return;

	if ( bUpdate )
	{
		CQuickLock oLock( Security.m_pSection );
		Security.m_Cache.clear();
		PostMainWndMessage( WM_SANITY_CHECK );
	}

	CloseThread();
}
