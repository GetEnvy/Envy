//
// FragmentBar.cpp
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
#include "FragmentBar.h"
#include "CoolInterface.h"
#include "Colors.h"
#include "Images.h"

#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferHTTP.h"
#include "DownloadTransferED2K.h"
#include "DownloadTransferBT.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"
#include "UploadTransferED2K.h"
#include "FragmentedFile.h"
#include "CtrlDownloads.h"
#include "CtrlUploads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug


//////////////////////////////////////////////////////////////////////
// CFragmentBar fragment

void CFragmentBar::DrawFragment(CDC* pDC, CRect* prcBar, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL b3D /*TRUE*/, BOOL bClip /*FALSE*/)
{
	if ( nTotal == 0 || nLength == 0 || nTotal == SIZE_UNKNOWN || nOffset == SIZE_UNKNOWN || nLength == SIZE_UNKNOWN || nLength > nTotal - nOffset )
		return;

	if ( Settings.General.LanguageRTL )
		nOffset = nTotal - nOffset - nLength;

	CRect rcArea;
	rcArea.top		= prcBar->top;
	rcArea.bottom	= prcBar->bottom;

	rcArea.left 	= prcBar->left + LONG( ( prcBar->Width() + 1 ) * nOffset / nTotal );
	rcArea.right	= prcBar->left + LONG( ( prcBar->Width() + 1 ) * ( nOffset + nLength ) / nTotal );

	rcArea.left 	= max( rcArea.left, prcBar->left );
	rcArea.right	= min( rcArea.right, prcBar->right );

	if ( rcArea.right <= rcArea.left )
		return;

	if ( crFill == Colors.m_crFragmentComplete && Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR ) )
	{
		// Done
	}
	else if ( b3D && rcArea.Width() > 2 )
	{
		pDC->Draw3dRect( &rcArea,
			CColors::CalculateColor( crFill, RGB(255,255,255), 75 ),
			CColors::CalculateColor( crFill, RGB(0,0,0), 75 ) );

		rcArea.DeflateRect( 1, 1 );
		pDC->FillSolidRect( &rcArea, crFill );
		rcArea.InflateRect( 1, 1 );
	}
	else
	{
		pDC->FillSolidRect( &rcArea, crFill );
	}

	if ( bClip )
		pDC->ExcludeClipRect( &rcArea );			// Potential high cpu here
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar state bar

void CFragmentBar::DrawStateBar(CDC* pDC, CRect* prcBar, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL bTop)
{
	if ( nTotal == 0 || nLength == 0 || nTotal == SIZE_UNKNOWN || nOffset == SIZE_UNKNOWN || nLength == SIZE_UNKNOWN )
		return;

	if ( nLength == nOffset )					// ToDo: Why does this happen? (torrents)
		return;
	ASSERT( nLength <= nTotal - nOffset );		// ToDo: Why does this fail?

	if ( Settings.General.LanguageRTL )
		nOffset = nTotal - nOffset - nLength;

	CRect rcArea;
	rcArea.left 	= prcBar->left + LONG( ( prcBar->Width() + 1 ) * nOffset / nTotal );
	rcArea.right	= prcBar->left + LONG( ( prcBar->Width() + 1 ) * ( nOffset + nLength ) / nTotal );
//	rcArea.left 	= max( rcArea.left, prcBar->left );
	rcArea.right	= min( rcArea.right, prcBar->right );

	if ( bTop )
	{
		rcArea.top		= prcBar->top;
		rcArea.bottom	= min( ( prcBar->top + prcBar->bottom ) / 2, prcBar->top + 2 ) - 1;
	}
	else
	{
		rcArea.top		= max( ( prcBar->top + prcBar->bottom ) / 2, prcBar->bottom - 3 ) + 1;
		rcArea.bottom	= prcBar->bottom;
	}

	if ( rcArea.right <= rcArea.left )
		return;

	if ( rcArea.Width() > 2 )
	{
		rcArea.DeflateRect( 1, 0 );
		pDC->FillSolidRect( &rcArea, crFill );
		rcArea.InflateRect( 1, 0 );

		pDC->FillSolidRect( rcArea.left, rcArea.top, 1, rcArea.Height(),
			CColors::CalculateColor( crFill, RGB(255,255,255), 100 ) );
		pDC->FillSolidRect( rcArea.right - 1, rcArea.top, 1, rcArea.Height(),
			CColors::CalculateColor( crFill, RGB(0,0,0), 75 ) );
	}
	else
	{
		pDC->FillSolidRect( &rcArea, crFill );
	}

	if ( bTop )
	{
		pDC->FillSolidRect( rcArea.left, rcArea.bottom, rcArea.Width(), 1,
			CColors::CalculateColor( crFill, RGB(0,0,0), 100 ) );
		rcArea.bottom ++;
	}
	else
	{
		rcArea.top --;
		pDC->FillSolidRect( rcArea.left, rcArea.top, rcArea.Width(), 1,
			CColors::CalculateColor( crFill, RGB(255,255,255), 100 ) );
	}

	pDC->ExcludeClipRect( &rcArea );
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar download

// New abstracted method:
void CFragmentBar::DrawDownload(CDC* pDC, CRect* prcBar, const CDownloadDisplayData* pDownloadData, COLORREF crNatural)
{
	if ( Settings.Downloads.SimpleBar )
	{
		pDC->FillSolidRect( prcBar, crNatural );
		DrawFragment( pDC, prcBar, pDownloadData->m_nSize, 0, pDownloadData->m_nVolumeComplete, Colors.m_crFragmentComplete, FALSE );
		return;
	}

	if ( Settings.Downloads.ShowPercent )
		DrawStateBar( pDC, prcBar, pDownloadData->m_nSize, 0, pDownloadData->m_nVolumeComplete, Colors.m_crFragmentComplete, TRUE );

	// pDownload->GetNextVerifyRange( nvOffset, nvLength, bvSuccess )
	for ( INT_PTR nRange = 0; nRange < pDownloadData->m_pVerifyRanges.GetCount(); nRange++ )
	{
		DrawStateBar( pDC, prcBar, pDownloadData->m_nSize,
			pDownloadData->m_pVerifyRanges.GetAt( nRange ).nOffset, pDownloadData->m_pVerifyRanges.GetAt( nRange ).nLength,
			pDownloadData->m_pVerifyRanges.GetAt( nRange ).bSuccess ? Colors.m_crFragmentPass : Colors.m_crFragmentFail );
	}

	// Draw background first, to avoid clipping
	if ( pDownloadData->m_nVolumeComplete || pDownloadData->m_bSeeding )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR ) )
			pDC->FillSolidRect( prcBar, Colors.m_crFragmentComplete );
	}
	else
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, crNatural );
	}

	Fragments::List oList( pDownloadData->m_oEmptyFragments );
	Fragments::List::const_iterator pItr = oList.begin();
	const Fragments::List::const_iterator pEnd = oList.end();
	for ( ; pItr != pEnd; ++pItr )
	{
		DrawFragment( pDC, prcBar, pDownloadData->m_nSize, pItr->begin(), pItr->size(), crNatural, FALSE );
	}

	for ( UINT nSource = 0; nSource < pDownloadData->m_nSourceCount; nSource++ )
	{
		// Note: Was pDownload->GetNext( posSource )->Draw( pDC, prcBar );
		if ( ! ( pDownloadData->m_bCompleted || pDownloadData->m_bSeeding ) || ! pDownloadData->m_pSourcesData[ nSource ].m_oPastFragments.empty() )
			DrawSource( pDC, prcBar, &pDownloadData->m_pSourcesData.GetAt( nSource ), crNatural, FALSE );
	}
}

// Legacy locking method:
void CFragmentBar::DrawDownload(CDC* pDC, CRect* prcBar, const CDownload* pDownload, COLORREF crNatural)
{
	if ( Settings.Downloads.SimpleBar )
	{
		pDC->FillSolidRect( prcBar, crNatural );
		DrawFragment( pDC, prcBar, pDownload->m_nSize, 0, pDownload->GetVolumeComplete(), Colors.m_crFragmentComplete, FALSE );
		return;
	}

	QWORD nvOffset, nvLength;
	BOOL bvSuccess;

	if ( Settings.Downloads.ShowPercent )
		DrawStateBar( pDC, prcBar, pDownload->m_nSize, 0, pDownload->GetVolumeComplete(), RGB( 0, 255, 0 ), TRUE );

	for ( nvOffset = 0; pDownload->GetNextVerifyRange( nvOffset, nvLength, bvSuccess ); )
	{
		DrawStateBar( pDC, prcBar, pDownload->m_nSize, nvOffset, nvLength, bvSuccess ? Colors.m_crFragmentPass : Colors.m_crFragmentFail );
		nvOffset += nvLength;
	}

	Fragments::List oList( pDownload->GetEmptyFragmentList() );
	Fragments::List::const_iterator pItr = oList.begin();
	const Fragments::List::const_iterator pEnd = oList.end();
	for ( ; pItr != pEnd; ++pItr )
	{
		DrawFragment( pDC, prcBar, pDownload->m_nSize, pItr->begin(), pItr->size(), crNatural, FALSE );
	}

	for ( POSITION posSource = pDownload->GetIterator(); posSource; )
	{
		// CDownloadSource* pSource
		pDownload->GetNext( posSource )->Draw( pDC, prcBar );
	}

	if ( pDownload->IsStarted() )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR ) )
			pDC->FillSolidRect( prcBar, Colors.m_crFragmentComplete );
	}
	else
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, crNatural );
	}
}

// Note: CFragmentBar::DrawDownloadSimple() moved to DrawDownload()

// New abstracted method:
void CFragmentBar::DrawSource(CDC* pDC, CRect* prcBar, const CSourceDisplayData* pSourceData, COLORREF crNatural, BOOL bDrawEmpty /*TRUE*/)
{
	if ( ! pSourceData )
		return;		// Crash fix?

	if ( ! pSourceData->m_bIdle )
	{
		// Note moved from CDownloadTransfer::DrawStateBar
		CFragmentBar::DrawStateBar( pDC, prcBar, pSourceData->m_nSize, pSourceData->m_nTransferOffset, pSourceData->m_nTransferLength, Colors.m_crFragmentRequest, TRUE );

	// ToDo:
	//	if ( pSourceData->m_nProtocol == PROTOCOL_BT || pSourceData->m_nProtocol == PROTOCOL_ED2K )
	//	{
	//		for ( Fragments::Queue::const_iterator pItr = m_oRequested.begin(); pItr != m_oRequested.end(); ++pItr )
	//		{
	//			CFragmentBar::DrawStateBar( pDC, prcBar, pSourceData->m_nSize, pItr->begin(), pItr->size(), Colors.m_crFragmentRequest, TRUE );
	//		}
	//	}
	}

	static COLORREF crFill[] =
	{
		Colors.m_crFragmentSource1, Colors.m_crFragmentSource2,
		Colors.m_crFragmentSource3, Colors.m_crFragmentSource4,
		Colors.m_crFragmentSource5, Colors.m_crFragmentSource6,
		Colors.m_crFragmentSource7, Colors.m_crFragmentSource8
	};

	COLORREF crTransfer = pSourceData->m_bReadContent ? crFill[ pSourceData->m_nColor ] : Colors.m_crFragmentComplete;
	crTransfer = CColors::CalculateColor( crTransfer, Colors.m_crHighlight, 90 );

	if ( ! pSourceData->m_bIdle && pSourceData->m_nState == dtsDownloading )	//&& m_pTransfer->m_nOffset < SIZE_UNKNOWN
	{
		if ( pSourceData->m_bTransferBackwards )
		{
			CFragmentBar::DrawFragment( pDC, prcBar, pSourceData->m_nSize,
				pSourceData->m_nTransferOffset + pSourceData->m_nTransferLength - pSourceData->m_nTransferPosition,
				pSourceData->m_nTransferPosition, crTransfer, TRUE, TRUE );
		}
		else
		{
			CFragmentBar::DrawFragment( pDC, prcBar, pSourceData->m_nSize,
				pSourceData->m_nTransferOffset,
				pSourceData->m_nTransferPosition, crTransfer, TRUE, TRUE );
		}
	}

	Fragments::List oList( pSourceData->m_oPastFragments );
	Fragments::List::const_iterator pItr = oList.begin();
	const Fragments::List::const_iterator pEnd = oList.end();
	for ( ; pItr != pEnd; ++pItr )
	{
		DrawFragment( pDC, prcBar, pSourceData->m_nSize, pItr->begin(), pItr->size(), crTransfer, TRUE, TRUE );
	}

	if ( ! bDrawEmpty )
		return;

	// Draw empty bar areas
	if ( ! pSourceData->m_oAvailable.empty() )
	{
		for ( Fragments::List::const_iterator pItr = pSourceData->m_oAvailable.begin(); pItr != pSourceData->m_oAvailable.end(); ++pItr )
		{
			CFragmentBar::DrawFragment( pDC, prcBar, pSourceData->m_nSize, pItr->begin(), pItr->size(), crNatural, FALSE, TRUE );		// ToDo: Crash here?
		}

		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, Colors.m_crWindow );
	}
	else if ( pSourceData->m_bHasFragments )	// IsOnline() && HasUsefulRanges() || ! pSourceData->m_oPastFragments.empty()
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_SHADED ) )
			pDC->FillSolidRect( prcBar, crNatural );
	}
	else
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, Colors.m_crWindow );
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar original moved to DownloadSource

//void CFragmentBar::DrawSource(CDC* pDC, CRect* prcBar, CDownloadSource* pSource, COLORREF crNatural)
//{
//	if ( pSource->m_pTransfer != NULL )
//	{
//		if ( pSource->m_pTransfer->m_nLength < SIZE_UNKNOWN )
//		{
//			DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
//				pSource->m_pTransfer->m_nOffset, pSource->m_pTransfer->m_nLength,
//				Colors.m_crFragmentRequest, TRUE );
//		}
//
//		switch ( pSource->m_pTransfer->m_nProtocol )
//		{
//		case PROTOCOL_BT:
//			for ( Fragments::Queue::const_iterator pRequested =
//				static_cast< CDownloadTransferBT* >( pSource->m_pTransfer )->m_oRequested.begin();
//				pRequested !=
//				static_cast< CDownloadTransferBT* >( pSource->m_pTransfer )->m_oRequested.end();
//				++pRequested )
//			{
//				DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
//					pRequested->begin(), pRequested->size(), Colors.m_crFragmentRequest, TRUE );
//			}
//			break;
//		case PROTOCOL_ED2K:
//			for ( Fragments::Queue::const_iterator pRequested
//				= static_cast< CDownloadTransferED2K* >( pSource->m_pTransfer )->m_oRequested.begin();
//				pRequested
//				!= static_cast< CDownloadTransferED2K* >( pSource->m_pTransfer )->m_oRequested.end();
//				++pRequested )
//			{
//				DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
//					pRequested->begin(), pRequested->size(), Colors.m_crFragmentRequest, TRUE );
//			}
//			break;
//		case PROTOCOL_G1:
//		case PROTOCOL_G2:
//		case PROTOCOL_HTTP:
//		case PROTOCOL_FTP:
//		case PROTOCOL_NULL:
//		case PROTOCOL_ANY:
//		default:
//			;	// Do nothing more
//		}
//	}
//
//	DrawSourceImpl( pDC, prcBar, pSource );
//
//	if ( ! pSource->m_oAvailable.empty() )
//	{
//		for ( Fragments::List::const_iterator pFragment = pSource->m_oAvailable.begin();
//			pFragment != pSource->m_oAvailable.end(); ++pFragment )
//		{
//			DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
//				pFragment->begin(), pFragment->size(), crNatural, FALSE );
//		}
//
//		pDC->FillSolidRect( prcBar, Colors.m_crWindow );
//	}
//	else if ( pSource->IsOnline() && pSource->HasUsefulRanges() || ! pSource->m_oPastFragments.empty() )
//	{
//		pDC->FillSolidRect( prcBar, crNatural );
//	}
//	else
//	{
//		pDC->FillSolidRect( prcBar, Colors.m_crWindow );
//	}
//}

//void CFragmentBar::DrawSourceImpl(CDC* pDC, CRect* prcBar, CDownloadSource* pSource)
//{
//	static COLORREF crFill[] =
//	{
//		Colors.m_crFragmentSource1, Colors.m_crFragmentSource2, Colors.m_crFragmentSource3,
//		Colors.m_crFragmentSource4, Colors.m_crFragmentSource5, Colors.m_crFragmentSource6,
//		Colors.m_crFragmentSource7, Colors.m_crFragmentSource8
//	};
//
//	COLORREF crTransfer = ? pSource->m_bReadContent : crFill[ pSource->GetColor() ] : Colors.m_crFragmentComplete;
//	crTransfer = CColors::CalculateColor( crTransfer, Colors.m_crHighlight, 90 );
//
//	if ( pSource->m_pTransfer != NULL &&
//		 pSource->m_pTransfer->m_nState == dtsDownloading &&
//		 pSource->m_pTransfer->m_nOffset < SIZE_UNKNOWN )
//	{
//		if ( pSource->m_pTransfer->m_bRecvBackwards )
//		{
//			DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
//				pSource->m_pTransfer->m_nOffset + pSource->m_pTransfer->m_nLength - pSource->m_pTransfer->m_nPosition,
//				pSource->m_pTransfer->m_nPosition, crTransfer, TRUE );
//		}
//		else
//		{
//			DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
//				pSource->m_pTransfer->m_nOffset,
//				pSource->m_pTransfer->m_nPosition, crTransfer, TRUE );
//		}
//	}
//
//	for ( Fragments::List::const_iterator pFragment = pSource->m_oPastFragments.begin();
//		pFragment != pSource->m_oPastFragments.end(); ++pFragment )
//	{
//		DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
//			pFragment->begin(), pFragment->size(), crTransfer, TRUE );
//	}
//}

//////////////////////////////////////////////////////////////////////
// CFragmentBar upload

// New abstracted method:
void CFragmentBar::DrawUpload(CDC* pDC, CRect* prcBar, const CUploadDisplayData* pUploadData, COLORREF crNatural)
{
	// Empty Bar first
	if ( pUploadData->m_bBaseFile )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_SHADED ) )
			pDC->FillSolidRect( prcBar, Colors.m_crFragmentShaded );
	}
	else // if ( pFile != pUpload->m_pBaseFile )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, crNatural );
	}

#if (_MSC_VER > 1700)	// VS2012 for C++11
	for ( const auto &frag : pUploadData->m_oFragments )
	{
		DrawFragment( pDC, prcBar, pUploadData->m_nSize, frag.begin(), frag.size(), Colors.m_crFragmentComplete, TRUE );
	}
#else
	for ( Fragments::List::const_iterator pItr = pUploadData->m_oFragments.begin(); pItr != pUploadData->m_oFragments.end(); pItr++ )
	{
		DrawFragment( pDC, prcBar, pUploadData->m_nSize, pItr->begin(), pItr->size(), Colors.m_crFragmentComplete, TRUE );
	}
#endif

	if ( pUploadData->m_bBaseFile && pUploadData->m_nLength != SIZE_UNKNOWN )
	{
		if ( pUploadData->m_nProtocol == PROTOCOL_HTTP && pUploadData->m_bBackwards )
		{
			DrawFragment( pDC, prcBar, pUploadData->m_nSize,
				pUploadData->m_nOffset + pUploadData->m_nLength - pUploadData->m_nPosition,
				pUploadData->m_nPosition, Colors.m_crFragmentComplete, TRUE );

			DrawFragment( pDC, prcBar, pUploadData->m_nSize,
				pUploadData->m_nOffset,
				pUploadData->m_nLength - pUploadData->m_nPosition, crNatural, FALSE );
		}
		else
		{
			DrawFragment( pDC, prcBar, pUploadData->m_nSize,
				pUploadData->m_nOffset, pUploadData->m_nPosition,
				Colors.m_crFragmentComplete, TRUE );

			DrawFragment( pDC, prcBar, pUploadData->m_nSize,
				pUploadData->m_nOffset + pUploadData->m_nPosition,
				pUploadData->m_nLength - pUploadData->m_nPosition, crNatural, FALSE );
		}
	}
}

// Legacy locking method:  (CtrlUploadTip)
void CFragmentBar::DrawUpload(CDC* pDC, CRect* prcBar, CUploadFile* pFile, COLORREF crNatural)
{
	CUploadTransfer* pUpload = pFile->GetActive();
	if ( ! pUpload ) return;

	// Empty Bar first
	if ( pFile == pUpload->m_pBaseFile )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_SHADED ) )
			pDC->FillSolidRect( prcBar, Colors.m_crFragmentShaded );
	}
	else // if ( pFile != pUpload->m_pBaseFile )
	{
		if ( ! Images.DrawButtonState( pDC, prcBar, IMAGE_PROGRESSBAR_NONE ) )
			pDC->FillSolidRect( prcBar, crNatural );
	}

	Fragments::List::const_iterator pItr = pFile->m_oFragments.begin();
	const Fragments::List::const_iterator pEnd = pFile->m_oFragments.end();
	for ( ; pItr != pEnd; ++pItr )
	{
		DrawFragment( pDC, prcBar, pFile->m_nSize, pItr->begin(), pItr->size(), Colors.m_crFragmentComplete, TRUE );
	}

	if ( pFile == pUpload->m_pBaseFile && pUpload->m_nLength != SIZE_UNKNOWN )
	{
		if ( pUpload->m_nProtocol == PROTOCOL_HTTP && ((CUploadTransferHTTP*)pUpload)->IsBackwards() )
		{
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset + pUpload->m_nLength - pUpload->m_nPosition,
				pUpload->m_nPosition, Colors.m_crFragmentComplete, TRUE );

			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset,
				pUpload->m_nLength - pUpload->m_nPosition, crNatural, FALSE );
		}
		else
		{
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset, pUpload->m_nPosition,
				Colors.m_crFragmentComplete, TRUE );

			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset + pUpload->m_nPosition,
				pUpload->m_nLength - pUpload->m_nPosition, crNatural, FALSE );
		}
	}
}
