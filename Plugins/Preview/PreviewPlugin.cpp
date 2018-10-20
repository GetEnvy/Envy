//
// PreviewPlugin.cpp : Implementation of CPreviewPlugin
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright PeerProject 2009 and Nikolay Raspopov 2009
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#include "StdAfx.h"
#include "PreviewPlugin.h"
#include "ConfigDlg.h"
#include "dllmain.h"

// CPreviewPlugin

CPreviewPlugin::CPreviewPlugin()
	: m_bCancel( false )
{
}

HRESULT CPreviewPlugin::FinalConstruct()
{
	return CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
}

void CPreviewPlugin::FinalRelease()
{
	m_pSite.Release();
	m_pUnkMarshaler.Release();
}

bool CPreviewPlugin::Execute(LPCTSTR szCommand)
{
	TCHAR szFile[ MAX_PATH ] = {};
	lstrcpyn( szFile, szCommand, _countof( szFile ) );
	PathRemoveArgs( szFile );
	TCHAR szParameters[ MAX_PATH * 2 ] = {};
	lstrcpyn( szParameters, szCommand + lstrlen( szFile ), _countof( szParameters ) );
	StrTrim( szParameters, L" \t" );
	PathUnquoteSpaces( szFile );

	// Launch new process
	SHELLEXECUTEINFO sei =
	{
		sizeof( SHELLEXECUTEINFO ),
		SEE_MASK_NOCLOSEPROCESS,
		GetDesktopWindow(),
		NULL,
		szFile,
		szParameters,
		NULL,
		SW_SHOWNORMAL
	};
	if ( ! ShellExecuteEx( &sei ) )
		return false;

	// Wait for process exit
	m_hProcess.Attach( sei.hProcess );
	WaitForSingleObject( sei.hProcess, INFINITE );
	m_hProcess.Close();

	return true;
}

// IGeneralPlugin

STDMETHODIMP CPreviewPlugin::SetApplication(
	/* [in] */ IApplication* /* pApplication */)
{
	return S_OK;
}

STDMETHODIMP CPreviewPlugin::QueryCapabilities(
	/* [out] */ DWORD* /* pnCaps */)
{
	return S_OK;
}

STDMETHODIMP CPreviewPlugin::Configure(void)
{
	CConfigDlg dlg;
	dlg.DoModal();

	return S_OK;
}

STDMETHODIMP CPreviewPlugin::OnSkinChanged(void)
{
	return S_OK;
}

// IDownloadPreviewPlugin

STDMETHODIMP CPreviewPlugin::SetSite(
	/* [in] */ IDownloadPreviewSite* pSite)
{
	m_pSite = pSite;

	return S_OK;
}

STDMETHODIMP CPreviewPlugin::Preview(
	/* [in] */ HANDLE /* hFile */,
	/* [in] */ BSTR /* sTarget */)
{
	// ToDo: Use GetFileInformationByHandleEx (Vista+)

	return E_NOTIMPL;
}

STDMETHODIMP CPreviewPlugin::Cancel()
{
	m_bCancel = true;

	if ( ! m_hProcess )
		return E_FAIL;

	TerminateProcess( m_hProcess, 0 );

	return S_OK;
}

// IDownloadPreviewPlugin2

STDMETHODIMP CPreviewPlugin::Preview2(
	/* [in] */ BSTR sSource,
	/* [in] */ BSTR sTarget)
{
	m_bCancel = false;

	if ( ! sSource || ! sTarget )
		return E_POINTER;

	if ( ! m_pSite )
		return E_UNEXPECTED;

	// Get extension
	LPCTSTR szTargetExt = PathFindExtension( sTarget );
	if ( ! *szTargetExt )
		return E_INVALIDARG;

	// Load data
	CAtlMap< CString, CString > oData;
	LoadData( oData );

	// Find command line for extension
	for ( POSITION pos = oData.GetStartPosition(); pos; )
	{
		CString strExt, strCommand;
		oData.GetNextAssoc( pos, strExt, strCommand );

		TCHAR szKey[ MAX_PATH ] = { L"." };
		for ( LPCTSTR str = strExt.GetBuffer(); ; ++str )
		{
			LPTSTR space = StrChr( str, L' ' );
			if ( space )
				*space = L'\0';
			lstrcpyn( szKey + 1, str, _countof( szKey ) - 1 );
			StrTrim( szKey + 1, L". /t" );

			if ( lstrcmpi( szKey, szTargetExt ) == 0 )
			{
				// Found
				strExt.ReleaseBuffer();

				if ( sCommand.Replace( L"%1", sSource ) == 0 ||
					 sCommand.Replace( L"%2", sTarget ) == 0 )
					return E_INVALIDARG;	 // Invalid command line

				m_pSite->SetProgressMessage( CComBSTR( sCommand ) );
				m_pSite->SetProgressRange( 100 );

				if ( ! Execute( sCommand ) )
					return E_FAIL;			// Can't launch external program

				m_pSite->SetProgressPosition( 100 );

				// Check for resulting file
				if ( GetFileAttributes( sTarget ) == INVALID_FILE_ATTRIBUTES )
					return S_FALSE;			// No file (user cancelled it)

				// Run it
				m_pSite->QueueDeleteFile( sTarget );
				if ( ! m_bCancel )
					m_pSite->ExecuteFile( sTarget );

				return S_OK;
			}

			if ( ! space )
			{
				strExt.ReleaseBuffer();
				break;
			}
			str = space;
		}
	}

	return E_INVALIDARG;
}
