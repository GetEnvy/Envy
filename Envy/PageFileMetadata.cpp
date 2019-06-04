//
// PageFileMetadata.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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
#include "Envy.h"
#include "PageFileMetadata.h"
#include "SharedFile.h"
#include "Library.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

IMPLEMENT_DYNCREATE(CFileMetadataPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileMetadataPage, CFilePropertiesPage)
	ON_CBN_SELCHANGE(IDC_SCHEMAS, &CFileMetadataPage::OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, &CFileMetadataPage::OnCloseUpSchemas)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage property page

CFileMetadataPage::CFileMetadataPage()
	: CFilePropertiesPage	( CFileMetadataPage::IDD )
	, m_pXML				( NULL )
{
}

void CFileMetadataPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
}

/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage message handlers

BOOL CFileMetadataPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CRect rcClient, rcCombo;
	GetClientRect( &rcClient );
	m_wndSchemas.GetWindowRect( &rcCombo );

	ScreenToClient( &rcCombo );
	rcCombo.top = rcCombo.bottom + 8;
	rcCombo.bottom = rcClient.bottom - 8;

	m_wndData.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rcCombo, this, IDC_METADATA );
	m_wndSchemas.m_sNoSchemaText = LoadString( IDS_SEARCH_NO_METADATA );

	CQuickLock oLock( Library.m_pSection );

	CLibraryListPtr pFiles( GetList() );

	BOOL bCollection = FALSE;
	CSchemaPtr pSchema = NULL;

	for ( POSITION pos = pFiles->GetHeadPosition(); pos; )
	{
		if ( CLibraryFile* pFile = pFiles->GetNextFile( pos ) )
		{
			CSchemaPtr pThisSchema = pFile->m_pSchema;

			if ( pThisSchema != NULL && pThisSchema->m_nType == CSchema::typeFolder )
				bCollection = TRUE;

			if ( pSchema == NULL )
			{
				pSchema = pThisSchema;
			}
			else if ( pSchema != pThisSchema )
			{
				pSchema = NULL;
				break;
			}
		}
	}

	m_wndSchemas.Load( pSchema != NULL ? pSchema->GetURI() : L"", bCollection ? CSchema::typeAny : CSchema::typeFile );

	if ( pSchema != NULL )
	{
		m_pSchemaContainer.Attach( pSchema->Instantiate( TRUE ) );
		m_pXML = m_pSchemaContainer->AddElement( pSchema->m_sSingular );

		for ( POSITION pos1 = pFiles->GetHeadPosition(); pos1; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
			{
				if ( pFile->m_pMetadata != NULL && pSchema->Equals( pFile->m_pSchema ) )
				{
					for ( POSITION pos2 = pSchema->GetMemberIterator(); pos2; )
					{
						CSchemaMemberPtr pMember = pSchema->GetNextMember( pos2 );
						CString strOld = pMember->GetValueFrom( m_pXML, NO_VALUE, FALSE, TRUE );
						if ( strOld != MULTI_VALUE )
						{
							CString strNew = pMember->GetValueFrom( pFile->m_pMetadata, L"", FALSE );
							if ( strOld == NO_VALUE )
							{
								m_pXML->AddAttribute( pMember->m_sName, strNew );
							}
							else if ( strOld != strNew )
							{
								m_wndData.AddItem( pMember, pMember->GetValueFrom( m_pXML, L"", TRUE, TRUE ) );
								m_wndData.AddItem( pMember, pMember->GetValueFrom( pFile->m_pMetadata, L"", TRUE ) );
								m_pXML->AddAttribute( pMember->m_sName, MULTI_VALUE );
							}
						}
						else
						{
							m_wndData.AddItem( pMember, pMember->GetValueFrom( pFile->m_pMetadata, L"", TRUE ) );
						}
					}
				}
			}
		}
	}

	OnSelChangeSchemas();

	m_wndData.UpdateData( m_pXML, FALSE );

	return TRUE;
}

void CFileMetadataPage::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	CString strSelectedURI = m_wndData.GetSchemaURI();

	if ( strSelectedURI.IsEmpty() || ! pSchema || pSchema->CheckURI( strSelectedURI ) )
	{
		m_wndData.SetSchema( pSchema );
		return;
	}

	// SchemaMappings.xml
	CString strBody( ::LoadHTML( GetModuleHandle( NULL ), IDR_XML_SCHEMA_MAPS ) );

	if ( CXMLElement* pXML = CXMLElement::FromString( strBody, TRUE ) )
	{
		if ( pXML->IsNamed( L"schemaMappings" ) )
		{
			for ( POSITION pos = pXML->GetElementIterator(); pos; )
			{
				if ( CXMLElement* pMapping = pXML->GetNextElement( pos ) )
				{
					// Add attributes which correspond to other schema
					// Don't need to delete old ones- after submitting new data, they will be ignored.
					// Also allows to save the old ones if we switch schema back.

					if ( pMapping->IsNamed( L"map" ) || pMapping->IsNamed( L"source" ) )
					{
						CXMLAttribute* pSourceURI = pMapping->GetAttribute( L"uri" );
						if ( pSourceURI && pSourceURI->GetValue() == m_wndData.GetSchemaURI() )
						{
							AddCrossAttributes( pMapping, pSchema->GetURI() );
							break;
						}
					}
					else if ( pMapping->IsNamed( L"schemaMapping" ) )	// Legacy
					{
						CXMLAttribute* pSourceURI = pMapping->GetAttribute( L"sourceURI" );
						if ( pSourceURI && pSourceURI->GetValue() == m_wndData.GetSchemaURI() )
						{
							AddCrossAttributes( pMapping, pSchema->GetURI() );
							break;
						}
					}
					delete pMapping;
				}
			}
		}
		delete pXML;
	}

	m_wndData.SetSchema( pSchema );
	if ( m_pXML )
	{
		// Change schema of data
		m_pXML->SetName( pSchema->m_sSingular );
		m_wndData.UpdateData( m_pXML, FALSE );
	}
}

void CFileMetadataPage::AddCrossAttributes(CXMLElement* pXML, LPCTSTR pszTargetURI)
{
	if ( pXML == NULL ) return;
	CXMLElement* pTargetURI = NULL;

	for ( POSITION pos = pXML->GetElementIterator(); pos; )
	{
		pTargetURI = pXML->GetNextElement( pos );

		if ( pTargetURI && pTargetURI->IsNamed( L"target" ) )
		{
			CXMLAttribute* pURI = pTargetURI->GetAttribute( L"uri" );
			if ( pURI && _tcscmp( pURI->GetValue(), pszTargetURI ) == 0 )
				break;
		}

		pTargetURI = NULL;
	}

	if ( pTargetURI == NULL ) return;

	for ( POSITION pos = pTargetURI->GetElementIterator(); pos; )
	{
		if ( CXMLElement* pAttribute = pTargetURI->GetNextElement( pos ) )
		{
			if ( pAttribute->IsNamed( L"attribute" ) )
			{
				if ( CXMLAttribute* pFrom = pAttribute->GetAttribute( L"from" ) )
				{
					if ( CXMLAttribute* pTo = pAttribute->GetAttribute( L"to" ) )
					{
						CString strFrom = pFrom->GetValue();
						CString strTo = pTo->GetValue();
						if ( strFrom.IsEmpty() || strTo.IsEmpty() ) continue;

						CString strValue = m_pXML->GetAttributeValue( strFrom );
						if ( strValue.GetLength() && strValue != MULTI_VALUE )
							m_pXML->AddAttribute( strTo, strValue );
					}
				}
			}
		}
	}
}

void CFileMetadataPage::OnCloseUpSchemas()
{
	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		PostMessage( WM_KEYDOWN, VK_TAB );
}

void CFileMetadataPage::OnOK()
{
	UpdateData();

	DWORD nModified = UpdateFileData( FALSE );

	if ( nModified >= 10 )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_LIBRARY_METADATA_MANY ), nModified );
		if ( MsgBox( strMessage, MB_YESNO|MB_ICONQUESTION ) != IDYES ) return;
	}

	CProgressDialog dlgProgress( LoadString( ID_LIBRARY_REFRESH_METADATA ) + L"..." );

	UpdateFileData();

	CFilePropertiesPage::OnOK();
}

DWORD CFileMetadataPage::UpdateFileData(BOOL bRealSave /*True*/)
{
	CQuickLock oLock( Library.m_pSection );

	DWORD nModified = 0;
	CLibraryListPtr pFiles( GetList() );
	if ( pFiles )
	{
		if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		{
			for ( POSITION pos1 = pFiles->GetHeadPosition() ; pos1 ; )
			{
				if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
				{
					if ( CXMLElement* pContainer = pSchema->Instantiate( TRUE ) )
					{
						CXMLElement* pXML = NULL;

						if ( pFile->m_pMetadata != NULL )
						{
							pXML = pContainer->AddElement( pFile->m_pMetadata->Clone() );
							// Change schema
							pXML->SetName( pSchema->m_sSingular );
						}
						else
						{
							pXML = pContainer->AddElement( pSchema->m_sSingular );
						}

						// Save changed data to pXML
						nModified += m_wndData.UpdateData( pXML, TRUE, bRealSave );

						if ( bRealSave )
							pFile->SetMetadata( pContainer );

						delete pContainer;
					}
				}
			}
		}
		else
		{
			for ( POSITION pos1 = pFiles->GetHeadPosition(); pos1; )
			{
				if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
				{
					if ( pFile->m_pMetadata )
					{
						++nModified;

						if ( bRealSave )
							pFile->ClearMetadata();
					}
				}
			}
		}

		if ( bRealSave && nModified )
			Library.Update();
	}
	return nModified;
}
