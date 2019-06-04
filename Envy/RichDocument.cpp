//
// RichDocument.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2015
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
#include "RichDocument.h"
#include "RichElement.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CRichDocument construction

CRichDocument::CRichDocument()
	: m_nCookie		( 0 )
	, m_szMargin	( 8, 8 )
	, m_crBackground( Colors.m_crRichdocBack )
	, m_crText		( Colors.m_crRichdocText )
	, m_crLink		( Colors.m_crTextLink )
	, m_crHover		( Colors.m_crTextLinkHot )
	, m_crHeading	( Colors.m_crRichdocHeading )
{
	CreateFonts();
}

CRichDocument::~CRichDocument()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CRichDocument element access

POSITION CRichDocument::GetIterator() const
{
	return m_pElements.GetHeadPosition();
}

CRichElement* CRichDocument::GetNext(POSITION& pos) const
{
	return m_pElements.GetNext( pos );
}

CRichElement* CRichDocument::GetPrev(POSITION& pos) const
{
	return m_pElements.GetPrev( pos );
}

INT_PTR CRichDocument::GetCount() const
{
	return m_pElements.GetCount();
}

POSITION CRichDocument::Find(CRichElement* pElement) const
{
	return m_pElements.Find( pElement );
}

//////////////////////////////////////////////////////////////////////
// CRichDocument element modification

CRichElement* CRichDocument::Add(CRichElement* pElement, POSITION posBefore)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( posBefore )
		m_pElements.InsertBefore( posBefore, pElement );
	else
		m_pElements.AddTail( pElement );

	pElement->m_pDocument = this;
	m_nCookie++;

	return pElement;
}

CRichElement* CRichDocument::Add(int nType, LPCTSTR pszText, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore)
{
	return Add( new CRichElement( nType, pszText, pszLink, nFlags, nGroup ), posBefore );
}

CRichElement* CRichDocument::Add(HBITMAP hBitmap, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore )
{
	return Add( new CRichElement( hBitmap, pszLink, nFlags, nGroup ), posBefore );
}

CRichElement* CRichDocument::Add(HICON hIcon, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore )
{
	return Add( new CRichElement( hIcon, pszLink, nFlags, nGroup ), posBefore );
}

void CRichDocument::Remove(CRichElement* pElement)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( POSITION pos = m_pElements.Find( pElement ) )
	{
		m_pElements.RemoveAt( pos );
		pElement->m_pDocument = NULL;
		m_nCookie++;
	}
}

void CRichDocument::ShowGroup(int nGroup, BOOL bShow)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator(); pos; )
	{
		CRichElement* pElement = GetNext( pos );
		if ( pElement->m_nGroup == nGroup )
			pElement->Show( bShow );
	}
}

void CRichDocument::ShowGroupRange(int nMin, int nMax, BOOL bShow)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator(); pos; )
	{
		CRichElement* pElement = GetNext( pos );
		if ( pElement->m_nGroup >= nMin && pElement->m_nGroup <= nMax )
			pElement->Show( bShow );
	}
}

void CRichDocument::SetModified()
{
	m_nCookie++;
}

void CRichDocument::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator(); pos; )
	{
		delete GetNext( pos );
	}

	m_pElements.RemoveAll();
	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CRichDocument font construction

void CRichDocument::CreateFonts(const LOGFONT* lpDefault, const LOGFONT* lpHeading)
{
	CSingleLock pLock( &m_pSection, TRUE );

	LOGFONT lfDefault = {};
	if ( lpDefault )
		CopyMemory( &lfDefault, lpDefault, sizeof( lfDefault ) );
	else
		CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );

	lfDefault.lfWeight = FW_NORMAL;
	lfDefault.lfItalic = FALSE;
	lfDefault.lfUnderline = FALSE;
	if ( m_fntNormal.m_hObject ) m_fntNormal.DeleteObject();
	m_fntNormal.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_BOLD;
	if ( m_fntBold.m_hObject ) m_fntBold.DeleteObject();
	m_fntBold.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_NORMAL;
	lfDefault.lfItalic = TRUE;
	if ( m_fntItalic.m_hObject ) m_fntItalic.DeleteObject();
	m_fntItalic.CreateFontIndirect( &lfDefault );

	lfDefault.lfItalic = FALSE;
	lfDefault.lfUnderline = TRUE;
	if ( m_fntUnder.m_hObject ) m_fntUnder.DeleteObject();
	m_fntUnder.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_BOLD;
	if ( m_fntBoldUnder.m_hObject ) m_fntBoldUnder.DeleteObject();
	m_fntBoldUnder.CreateFontIndirect( &lfDefault );

	LOGFONT lfHeading = {};
	if ( lpHeading )
		CopyMemory( &lfHeading, lpHeading, sizeof( lfHeading ) );
	else
		CoolInterface.m_fntRichHeading.GetLogFont( &lfHeading );

	if ( m_fntHeading.m_hObject ) m_fntHeading.DeleteObject();
	m_fntHeading.CreateFontIndirect( &lfHeading );
}

//////////////////////////////////////////////////////////////////////
// CRichDocument XML Load

BOOL CRichDocument::LoadXML(CXMLElement* pBase, CElementMap* pMap, int nGroup)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( pBase == NULL ) return FALSE;

	CString strTemp;

	if ( pBase->IsNamed( L"document" ) )
	{
		strTemp = pBase->GetAttributeValue( L"fontFace" );
		if ( ! strTemp.IsEmpty() )
		{
			// Change font face
			LOGFONT lfDefault = {};
			CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );
			_tcsncpy( lfDefault.lfFaceName, strTemp, LF_FACESIZE );

			LOGFONT lfHeading = {};
			CoolInterface.m_fntRichHeading.GetLogFont( &lfHeading );
			_tcsncpy( lfHeading.lfFaceName, strTemp, LF_FACESIZE );

			CreateFonts( &lfDefault, &lfHeading );
		}

		m_crBackground	= Colors.m_crRichdocBack;
		m_crText		= Colors.m_crRichdocText;
		m_crLink		= Colors.m_crTextLink;
		m_crHover		= Colors.m_crTextLinkHot;
		m_crHeading		= Colors.m_crRichdocHeading;

		Skin.LoadColor( pBase, L"crBackground", &m_crBackground );
		Skin.LoadColor( pBase, L"crText", &m_crText );
		Skin.LoadColor( pBase, L"crLink", &m_crLink );
		Skin.LoadColor( pBase, L"crHover", &m_crHover );
		Skin.LoadColor( pBase, L"crHeading", &m_crHeading );

		strTemp = pBase->GetAttributeValue( L"leftMargin" );
		if ( ! strTemp.IsEmpty() ) _stscanf( strTemp, L"%li", &m_szMargin.cx );
		strTemp = pBase->GetAttributeValue( L"topMargin" );
		if ( ! strTemp.IsEmpty() ) _stscanf( strTemp, L"%li", &m_szMargin.cy );
	}

	for ( POSITION pos = pBase->GetElementIterator(); pos; )
	{
		CXMLElement* pXML		= pBase->GetNextElement( pos );
		CRichElement* pElement	= NULL;

		if ( pXML->IsNamed( L"text" ) )
		{
			pElement = new CRichElement( retText );
		}
		else if ( pXML->IsNamed( L"link" ) )
		{
			pElement = new CRichElement( retLink );
		}
		else if ( pXML->IsNamed( L"heading" ) )
		{
			pElement = new CRichElement( retHeading );
		}
		else if ( pXML->IsNamed( L"newline" ) )
		{
			pElement = new CRichElement( retNewline );

			strTemp = pXML->GetAttributeValue( L"gap" );

			if ( ! strTemp.IsEmpty() )
			{
				pElement->m_sText = strTemp;
				strTemp = pXML->GetAttributeValue( L"indent" );
				if ( ! strTemp.IsEmpty() ) pElement->m_sText += '.' + strTemp;
			}
			else
			{
				strTemp = pXML->GetAttributeValue( L"indent" );
				if ( ! strTemp.IsEmpty() ) pElement->m_sText = L"0." + strTemp;
			}
		}
		else if ( pXML->IsNamed( L"gap" ) )
		{
			pElement = new CRichElement( retGap );

			strTemp = pXML->GetAttributeValue( L"size" );
			if ( strTemp ) pElement->m_sText = strTemp;
		}
		else if ( pXML->IsNamed( L"bitmap" ) )
		{
			pElement = new CRichElement( retBitmap );
		}
		else if ( pXML->IsNamed( L"icon" ) )
		{
			pElement = new CRichElement( retIcon );
		}
		else if ( pXML->IsNamed( L"anchor" ) )
		{
			pElement = new CRichElement( retAnchor );
		}
		else if ( pXML->IsNamed( L"para" ) )
		{
			pElement = new CRichElement( retAlign, pXML->GetAttributeValue( L"align" ) );
			Add( pElement );

			if ( pXML->GetElementCount() )
			{
				if ( ! LoadXML( pXML, pMap, nGroup ) ) return FALSE;

				if ( pElement->m_sText.CompareNoCase( L"left" ) )
					Add( new CRichElement( retAlign, L"left" ) );
			}

			continue;
		}
		else if ( pXML->IsNamed( L"group" ) )
		{
			int nSubGroup = 0;
			if ( _stscanf( pXML->GetAttributeValue( L"id" ), L"%i", &nSubGroup ) != 1 )
				return FALSE;
			if ( ! LoadXML( pXML, pMap, nSubGroup ) ) return FALSE;
			continue;
		}
		else if ( pXML->IsNamed( L"styles" ) )
		{
			if ( ! LoadXMLStyles( pXML ) ) return FALSE;
		}
		else // Log unknown?
		{
			return FALSE;
		}

		if ( pElement == NULL ) continue;

		strTemp = pXML->GetValue();
		if ( ! strTemp.IsEmpty() ) pElement->m_sText = strTemp;

		pElement->m_nGroup = nGroup;
		strTemp = pXML->GetAttributeValue( L"group" );
		if ( ! strTemp.IsEmpty() ) _stscanf( strTemp, L"%i", &pElement->m_nGroup );

		strTemp = pXML->GetAttributeValue( L"format" );
		ToLower( strTemp );

		if ( strTemp.FindOneOf( L"bB" ) >= 0 )	pElement->m_nFlags |= retfBold;
		if ( strTemp.FindOneOf( L"iI" ) >= 0 )	pElement->m_nFlags |= retfItalic;
		if ( strTemp.FindOneOf( L"uU" ) >= 0 )	pElement->m_nFlags |= retfUnderline;

		strTemp = pXML->GetAttributeValue( L"align" );

		if ( strTemp.CompareNoCase( L"middle" ) == 0 )
			pElement->m_nFlags |= retfMiddle;

		if ( Skin.LoadColor( pXML, L"color",  &pElement->m_cColor ) ||
			 Skin.LoadColor( pXML, L"colour", &pElement->m_cColor ) )
			pElement->m_nFlags |= retfColor;

		if ( pElement->m_nType == retIcon )
		{
			strTemp = pXML->GetAttributeValue( L"command" );
			if ( ! strTemp.IsEmpty() )
			{
				pElement->m_nType = retCmdIcon;
				pElement->m_sText = strTemp;
			}
		}

		if ( pElement->m_nType == retIcon || pElement->m_nType == retBitmap || pElement->m_nType == retAnchor )
		{
			strTemp = pXML->GetAttributeValue( L"res" );
			if ( ! strTemp.IsEmpty() ) pElement->m_sText = strTemp;
			strTemp = pXML->GetAttributeValue( L"path" );
			if ( ! strTemp.IsEmpty() ) pElement->m_sText = strTemp;

			strTemp = pXML->GetAttributeValue( L"width" );
			if ( ! strTemp.IsEmpty() )
			{
				if ( ! pElement->m_sText.IsEmpty() )
					pElement->m_sText += '.';
				pElement->m_sText += strTemp;
				strTemp = pXML->GetAttributeValue( L"height" );
				if ( ! strTemp.IsEmpty() )
					pElement->m_sText += '.' + strTemp;
			}
		}

		pElement->m_sLink = pXML->GetAttributeValue( L"target" );

		if ( pMap )
		{
			strTemp = pXML->GetAttributeValue( L"id" );
			if ( ! strTemp.IsEmpty() )
				pMap->SetAt( strTemp, pElement );
		}

		Add( pElement );
	}

	return TRUE;
}

BOOL CRichDocument::LoadXMLStyles(CXMLElement* pParent)
{
	for ( POSITION pos = pParent->GetElementIterator(); pos; )
	{
		CXMLElement* pXML = pParent->GetNextElement( pos );
		if ( ! pXML->IsNamed( L"style" ) )
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, L"Unknown element in [styles] element", (LPCTSTR)pXML->ToString() );
			continue;
		}

		CString strName = pXML->GetAttributeValue( L"name" );
		strName.MakeLower();
		bool bDefault = ( strName == L"default" || strName.IsEmpty() );
		bool bHeading = ( strName == L"heading" );

		LOGFONT lf = {};
		if ( bDefault )
			CoolInterface.m_fntRichDefault.GetLogFont( &lf );
		else if ( bHeading )
			CoolInterface.m_fntRichHeading.GetLogFont( &lf );

		if ( CXMLElement* pFont = pXML->GetElementByName( L"font" ) )
		{
			CString strFontFace = pFont->GetAttributeValue( L"face" );
			if ( ! strFontFace.IsEmpty() )
				_tcsncpy( lf.lfFaceName, strFontFace, LF_FACESIZE );

			CString strSize = pFont->GetAttributeValue( L"size" );
			if ( ! strSize.IsEmpty() )
			{
				if ( _stscanf( strSize, L"%li", &lf.lfHeight ) == 1 )
					lf.lfHeight = - lf.lfHeight;
			}

			CString strWeight = pFont->GetAttributeValue( L"weight" );
			if ( ! strWeight.IsEmpty() )
				_stscanf( strWeight, L"%li", &lf.lfWeight );
		}

		CXMLElement* pColors = pXML->GetElementByName( L"colors" );
		if ( pColors == NULL ) pColors = pXML->GetElementByName( L"colours" );
		if ( pColors == NULL ) pColors = pXML;

		if ( strName == L"default" || strName.IsEmpty() )
		{
			Skin.LoadColor( pColors, L"back", &m_crBackground );
			Skin.LoadColor( pColors, L"text", &m_crText );
			Skin.LoadColor( pColors, L"link", &m_crLink );
			Skin.LoadColor( pColors, L"hover", &m_crHover );

			// Create specified fonts (using default font as heading font)
			CreateFonts( &lf, NULL );
		}
		else if ( bHeading )
		{
			Skin.LoadColor( pColors, L"text", &m_crHeading );

			// Create heading font
			if ( m_fntHeading.m_hObject ) m_fntHeading.DeleteObject();
			m_fntHeading.CreateFontIndirect( &lf );
		}
	}

	return TRUE;
}
