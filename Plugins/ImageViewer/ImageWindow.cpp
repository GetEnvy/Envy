//
// ImageWindow.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012
//
// Original author Michael Stokes released portions into the public domain.
// You are free to redistribute and modify this page without any restrictions.
//

// The CImageWindow class contained in this file represents a single image window
// in the Envy GUI.  Several windows can be open at once, viewing different images.
//
// This class extends the ATL CWindow class to provide wrappers for the Win32
// window functions, and basic message map functionality.  Note that we are not
// using CWindowImpl, because we do not actually create the window here or
// indeed provide a window procedure, etc.
//
// Whenever plugins create a window in the Envy GUI, two interfaces are involved.
// The IPluginWindow interface is implemented by Envy, and allows us to
// interact with the window we have created.  The IPluginWindowOwner interface
// is implemented by us (on this class), and allows Envy to pass event
// notifications for the window to us.  Envy will release the IPluginWindowOwner
// interface when the window is closed (the IPluginWindow interface will be
// internally released at this time also).
//

#include "StdAfx.h"
#include "ImageViewer.h"	// Generated
#include "ImageViewerPlugin.h"
#include "ImageWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Some constants
#define REFRESH_DELAY		5000
#define BACKGROUND_COLOR	RGB( 254, 254, 254 )
//#define TOOLBAR_HEIGHT	28		// m_nToolbarHeight ~= Skin.m_nToolbarHeight


//////////////////////////////////////////////////////////////////////
// CImageWindow construction

CImageWindow::CImageWindow()
	: m_pPlugin		( NULL )		// Reference to the plugin that owns us
	, m_pNext		( NULL )		// Next CImageWindow in the linked list of open windows
	, m_hBitmap		( NULL )		// A HBITMAP bitmap image handle
	, m_hIcon		( NULL )		// A HICON icon for the window
	, m_bFullSize	( FALSE )		// Are we in acutal-size mode, or fit-to-window mode?
	, m_bDrag		( FALSE )		// Are we dragging?
	, m_ptOffset	( ) 			// Offset coordinates when dragged
	, m_nZoomIndex	( 0 )			// Zoom index (control)
	, m_nZoomFactor	( 1.0f )		// Zoom factor (scale)
{
}

//////////////////////////////////////////////////////////////////////
// CImageWindow destruction

CImageWindow::~CImageWindow()
{
	// Remove this CImageWindow object from the plugin's linked list of open windows
	if ( m_pPlugin != NULL ) m_pPlugin->RemoveWindow( this );

	// Destroy the various objects we may have allocated
	if ( m_hIcon != NULL ) DestroyIcon( m_hIcon );
	if ( m_hBitmap != NULL ) DeleteObject( m_hBitmap );
}

//////////////////////////////////////////////////////////////////////
// CImageWindow create the window

BOOL CImageWindow::Create(CImageViewerPlugin* pPlugin, LPCTSTR pszFile)
{
	// Store a reference to the plugin, the application and the filename.
	// The application is AddRef'ed because it's a CComPtr<>.
	// We don't AddRef the plugin.
	m_pPlugin		= pPlugin;
	m_pApplication	= pPlugin->m_pApplication;
	m_sFile			= pszFile;

	// Get an IUserInterface pointer from the IApplication
	CComPtr<IUserInterface> pUI;
	m_pApplication->get_UserInterface( &pUI );
	if ( ! pUI ) return FALSE;

	// Tell the GUI manager to create a new window called "ImageViewerWindow".
	// We pass "this" as the IPluginWindowOwner, and store the IPluginWindow in m_pWindow.
	pUI->NewWindow( L"ImageViewerWindow", this, &m_pWindow );
	if ( ! m_pWindow ) return FALSE;

	// Request notifications for a number of messages.
	m_pWindow->ListenForSingleMessage( WM_DESTROY );
	m_pWindow->ListenForSingleMessage( WM_SIZE );
	m_pWindow->ListenForSingleMessage( WM_PAINT );
	m_pWindow->ListenForSingleMessage( WM_KEYDOWN );
	m_pWindow->ListenForSingleMessage( WM_LBUTTONDBLCLK );
	m_pWindow->ListenForSingleMessage( WM_LBUTTONDOWN );
	m_pWindow->ListenForSingleMessage( WM_LBUTTONUP );
	m_pWindow->ListenForSingleMessage( WM_MOUSEMOVE );
	m_pWindow->ListenForSingleMessage( WM_MOUSEWHEEL );
	m_pWindow->ListenForSingleMessage( WM_CONTEXTMENU );
	m_pWindow->ListenForSingleMessage( WM_SETCURSOR );
	m_pWindow->ListenForSingleMessage( WM_TIMER );

	// Physically create the window using create method one, passing a caption and icon directly.
	// Set panel/tabbed mode to false because we want a "normal" window.
	m_pWindow->Create1( CComBSTR( L"Image Viewer" ), NULL, VARIANT_FALSE, VARIANT_FALSE );

	// Get the HWND and assign it to the CWindow parent class' m_hWnd member,
	// so we can call the Win32 API wrappers.
	m_pWindow->GetHwnd( &m_hWnd );

	// Add a toolbar to the window.  This toolbar was defined in
	// the XML in our resources, using the commands we registered in the plugin.
	// Pass NULL for the HWND and IToolbar outputs, as we don't want them.
	// The default position 0 for the toolbar is "on the bottom".
	HWND hWndToolbar;
	m_pWindow->AddToolbar( CComBSTR( L"ImageViewerWnd" ), 0, &hWndToolbar, NULL );

	// Possible improvement: Accept the HWND of the toolbar above and get its height manually,
	// instead of assuming toolbars are always 28 pixels high (could change in the future/skins).
	// In reality we'd have to do this whenever the skin changes, too.
	RECT rc;
	::GetWindowRect( hWndToolbar, &rc );
	m_nToolbarHeight = rc.bottom - rc.top;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow refresh the image

BOOL CImageWindow::Refresh()
{
	// Go into wait cursor mode
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	// Scrap the timer if it's running
	KillTimer( 2 );

	// Try to load the image, and convert it to RGB color if not already

	if ( ! m_pImage.Load( m_sFile ) ||
		 ! m_pImage.EnsureRGB( BACKGROUND_COLOR ) )
	{
		// Didn't work?  Clear the image, in case the load worked but the EnsureRGB() didn't.
		// Otherwise we could get left with a non-RGB image.
		m_pImage.Clear();

		// Restore old cursor
		SetCursor( hCursor );

		return FALSE;
	}

	if ( m_hBitmap )
	{
		DeleteObject( m_hBitmap );
		m_hBitmap = NULL;
	}

	// Ask the Windows Shell for the appropriate small icon for this filename
	SHFILEINFO pInfo = {};
	SHGetFileInfo( m_sFile, 0, &pInfo, sizeof(pInfo), SHGFI_ICON|SHGFI_SMALLICON );

	if ( m_hIcon )
		DestroyIcon( m_hIcon );

	if ( pInfo.hIcon )
		m_hIcon = pInfo.hIcon;
	else // No icon, load a default from our resources
		m_hIcon = (HICON)LoadImage( _AtlBaseModule.GetResourceInstance(),
			MAKEINTRESOURCE(IDI_IMAGE), IMAGE_ICON, 16, 16, 0 );

	SetIcon( m_hIcon, FALSE );

	// Create a caption showing only the filename part of the path, plus " : Image Viewer" (Obsolete)
	//LPCTSTR pszName = _tcsrchr( m_sFile, '\\' );
	//CComBSTR bsFile( pszName ? pszName + 1 : m_sFile );
	//bsFile.Append( L" : Image Viewer" );
	SetWindowText( (CString)PathFindFileName( m_sFile ) + L" Viewer" );

	// If this is the first load, resize the window
	ResizeWindow();

	// If we need to create a scaled HBITMAP, do so
	RescaleImage();

	// If the image is partially loaded, set a timer to refresh it later.
	// We use timer ID 2, because Envy sends a global heartbeat on timer ID 1.
	if ( m_pImage.m_bPartial )
		SetTimer( 2, REFRESH_DELAY, NULL );

	// Restore old cursor
	SetCursor( hCursor );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow resize the window to handle the image

BOOL CImageWindow::ResizeWindow()
{
	HWND hMDI = GetParent();
	RECT rcWnd, rcMDI;

	// Must have an image
	if ( m_pImage.m_nHeight == 0 ) return FALSE;

	// Get the client rectangle of the MDI client area
	::GetClientRect( hMDI, &rcMDI );

	// Create an ideal window size (client coordinates)
	rcWnd.left		= rcWnd.top = 0;
	rcWnd.right		= m_pImage.m_nWidth;
	rcWnd.bottom	= m_pImage.m_nHeight + m_nToolbarHeight;

	// Get the IPluginWindow to translate client size to window size (we use IPluginWindow
	// rather than CalcWindowRect because skins could have custom window graphics & metrics)
	m_pWindow->AdjustWindowRect( &rcWnd, VARIANT_TRUE );

	// Clip our new window size to the MDI size
	if ( rcWnd.right - rcWnd.left > rcMDI.right - rcMDI.left )
	{
		rcWnd.left = rcMDI.left;
		rcWnd.right = rcMDI.right;
	}

	if ( rcWnd.bottom - rcWnd.top > rcMDI.bottom - rcMDI.top )
	{
		rcWnd.top = rcMDI.top;
		rcWnd.bottom = rcMDI.bottom;
	}

	// Center in the MDI client area
	OffsetRect( &rcWnd,
		rcMDI.right / 2 - ( rcWnd.right - rcWnd.left ) / 2 - rcWnd.left,
		rcMDI.bottom / 2 - ( rcWnd.bottom - rcWnd.top ) / 2 - rcWnd.top );

	// Move
	MoveWindow( &rcWnd );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow rescale the image to fit in the window

BOOL CImageWindow::RescaleImage()
{
	RECT rc;
	SIZE sz;

	// Must have an image
	if ( m_pImage.m_nHeight == 0 ) return FALSE;

	if ( m_bFullSize )
	{
		// Full size mode is always full size / zoomed
		sz.cx = (int)( m_nZoomFactor * m_pImage.m_nWidth );
		sz.cy = (int)( m_nZoomFactor * m_pImage.m_nHeight );
	}
	else
	{
		// Fit to window mode, get the window size
		GetClientRect( &rc );
		rc.bottom -= m_nToolbarHeight;

		sz.cx = rc.right - rc.left;
		sz.cy = rc.bottom - rc.top;

		// Compute the most appropriate size, preserving aspect ratio
		int nSize = sz.cy * m_pImage.m_nWidth / m_pImage.m_nHeight;

		if ( nSize > sz.cx )
			sz.cy = sz.cx * m_pImage.m_nHeight / m_pImage.m_nWidth;
		else
			sz.cx = nSize;

		// Don't "grow" the image, only shrink it
		if ( sz.cx > m_pImage.m_nWidth || sz.cy > m_pImage.m_nHeight )
		{
			sz.cx = m_pImage.m_nWidth;
			sz.cy = m_pImage.m_nHeight;
		}
	}

	// Get rid of the old bitmap if there is one
	if ( m_hBitmap != NULL ) DeleteObject( m_hBitmap );

	// Create a new bitmap, resampled to the correct size
	m_hBitmap = m_pImage.Resample( sz.cx, sz.cy );

	// Repaint
	Invalidate();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow IPluginWindowOwner message dispatch

HRESULT STDMETHODCALLTYPE CImageWindow::OnTranslate(MSG __RPC_FAR* /*pMessage*/)
{
	// This IPluginWindowOwner method is called when translating a message while window is active.
	// It gives the window the opportunity to translate a keyboard message into a command.

	// Return S_OK if the message was translated into a command (and handled),
	// S_FALSE if the message was NOT translated, or E_NOTIMPL if the window
	// does not implement this method. In this case it will not be called again.

	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CImageWindow::OnMessage(UINT nMessage, WPARAM wParam, LPARAM lParam, LRESULT __RPC_FAR *plResult)
{
	// This IPluginWindowOwner method is called whenever the window receives a message that
	// we requested a notification for.  Return S_OK if processed, S_FALSE to pass the message
	// on to the default handler.  The result is stored in *plResult.

	// NOTE: If we want to call the default message handler BEFORE returning, we can invoke
	// IPluginWindow::HandleMessage( plResult ), which returns the result from the default handler.
	// We then return S_OK to avoid calling the default handler again.

	// Here we forward the message to ATL CWindow's message processor, so it gets fed into our message map.

	return ProcessWindowMessage( m_hWnd, nMessage, wParam, lParam, *plResult ) ? S_OK : S_FALSE;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow message handlers

LRESULT CImageWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	KillTimer( 2 );
	bHandled = FALSE;
	return 0;
}

LRESULT CImageWindow::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Create a POINT structure with the screen coordinates of the mouse click
	POINT pt = { LOWORD( lParam ), HIWORD( lParam ) };

	if ( pt.x > 65000 )		// Keyboard fix (-1,-1)
	{
		pt.x = pt.y = 1;
		ClientToScreen( &pt );
	}

	// Get the IPluginWindow to throw our named menu at the location
	m_pWindow->ThrowMenu( L"ImageViewer_Context", 0, &pt );
	return 0;
}

LRESULT CImageWindow::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Rescale the image to fit the window.  Making sure we have the hWnd first,
	// as this can get called after the Create1() call but before the GetHwnd() call.

	if ( m_hWnd != NULL && ! m_bFullSize )
		RescaleImage();
	bHandled = FALSE;
	return 0;
}

LRESULT CImageWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rcClient, rcImage;
	PAINTSTRUCT ps;
	BITMAP pInfo;

	// Get the client rectangle, and begin painting
	GetClientRect( &rcClient );
	HDC hDC = BeginPaint( &ps );

	// Don't touch the toolbar
	ExcludeClipRect( hDC, 0, rcClient.bottom - m_nToolbarHeight, rcClient.right, rcClient.bottom );
	rcClient.bottom -= m_nToolbarHeight;

	// Setup
	SetBkMode( hDC, OPAQUE );
	SetBkColor( hDC, BACKGROUND_COLOR );

	if ( m_hBitmap != NULL )
	{
		// Get the size of the HBITMAP
		GetObject( m_hBitmap, sizeof(BITMAP), &pInfo );

		// Center it
		rcImage.left	= ( rcClient.left + rcClient.right ) / 2 - pInfo.bmWidth / 2;
		rcImage.top		= ( rcClient.top + rcClient.bottom ) / 2 - pInfo.bmHeight / 2;
		rcImage.right	= rcImage.left + pInfo.bmWidth;
		rcImage.bottom	= rcImage.top + pInfo.bmHeight;

		if ( m_bFullSize )
		{
			m_nPaddingWidth = (short)rcImage.left;
			m_nPaddingHeight = (short)rcImage.top;

			// In full size mode, the image can be dragged around
			OffsetRect( &rcImage, m_ptOffset.x, m_ptOffset.y );
		}

		// Create a memory DC and select the bitmap
		HDC hMemDC = CreateCompatibleDC( hDC );
		HBITMAP hOldBmp = (HBITMAP)SelectObject( hMemDC, m_hBitmap );

		// Display the bitmap
		BitBlt( hDC, rcImage.left, rcImage.top, pInfo.bmWidth, pInfo.bmHeight,
			hMemDC, 0, 0, SRCCOPY );

		// Cleanup memory DC
		SelectObject( hMemDC, hOldBmp );
		DeleteDC( hMemDC );

		// Don't paint over that image
		ExcludeClipRect( hDC, rcImage.left, rcImage.top, rcImage.right, rcImage.bottom );

		// Draw a rectangle around the image
		HPEN hPen=(HPEN)CreatePen( PS_SOLID, 1, RGB(230,230,230) );
		SelectObject( hDC, hPen );
		InflateRect( &rcImage, 1, 1 );
		Rectangle( hDC, rcImage.left, rcImage.top, rcImage.right, rcImage.bottom );
		DeleteObject(hPen);

		// Don't paint over that, either
		ExcludeClipRect( hDC, rcImage.left, rcImage.top, rcImage.right, rcImage.bottom );

		// Fill the rest of the window in with the background color
		ExtTextOut( hDC, 0, 0, ETO_OPAQUE, &rcClient, NULL, 0, NULL );
	}
	else
	{
		// If there is no image, we couldn't load it.  Display an error message instead.

		CString strText = L"Unable to load: " + m_sFile;;
		ExtTextOut( hDC, 6, 6, ETO_OPAQUE, &rcClient, strText, strText.GetLength(), NULL );
		rcClient.top += 24;
		ExtTextOut( hDC, 6, 30, ETO_OPAQUE, &rcClient,
			L"Hold Shift key to open in default application.", 50, NULL );
	}

	// Finish painting
	EndPaint( &ps );

	return 0;
}

LRESULT CImageWindow::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	switch ( wParam )
	{
	// Close if ESCAPE is hit
	case VK_ESCAPE:
		PostMessage( WM_CLOSE );
		break;

	// View next file
	case VK_DOWN:
	case VK_RIGHT:
	case VK_NEXT:
		OnNext();
		break;

	// View previous file
	case VK_UP:
	case VK_LEFT:
	case VK_PRIOR:
		OnPrevious();
		break;

	// View first file
	case VK_HOME:
		OnFirst();
		break;

	// View last file
	case VK_END:
		OnLast();
		break;

	// Delete file
	case VK_DELETE:
		Delete();
		break;
	}

	return 0;
}

LRESULT CImageWindow::OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Switch between full size and best fit modes
	if ( m_bFullSize )
		OnBestFit();
	else
		OnActualSize();

	return 0;
}

LRESULT CImageWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Start dragging in full size mode
	if ( m_bFullSize && m_hBitmap != NULL )
	{
		SetCapture();
		m_ptDrag = MAKEPOINTS( lParam );
		m_bDrag = TRUE;
		SetCursor( m_pPlugin->m_hcMove );
	}

	return 0;
}

LRESULT CImageWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Finish dragging
	if ( m_bDrag )
	{
		ReleaseCapture();
		m_bDrag = FALSE;
	}

	return 0;
}

LRESULT CImageWindow::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Update the drag and repaint if necessary
	if ( m_bDrag )
	{
		RECT rc;
		GetClientRect( &rc );
		rc.bottom -= m_nToolbarHeight;
		POINTS pt = MAKEPOINTS( lParam );

		m_ptOffset.x += ( pt.x - m_ptDrag.x );
		m_ptOffset.y += ( pt.y - m_ptDrag.y );
		m_ptDrag = pt;

		// Edge Detection (Limit Excessive Movement)

		if ( m_pImage.m_nWidth * m_nZoomFactor < rc.right )
		{
			if ( m_ptOffset.x < -m_nPaddingWidth )
				m_ptOffset.x = -m_nPaddingWidth;
			else if ( m_ptOffset.x > m_nPaddingWidth )
				m_ptOffset.x = m_nPaddingWidth;
		}
		else // Oversized
		{
			if ( m_ptOffset.x < m_nPaddingWidth )
				m_ptOffset.x = m_nPaddingWidth;
			else if ( m_ptOffset.x > -m_nPaddingWidth )
				m_ptOffset.x = -m_nPaddingWidth;
		}

		if ( m_pImage.m_nHeight * m_nZoomFactor < rc.bottom )
		{
			if ( m_ptOffset.y < -m_nPaddingHeight )
				m_ptOffset.y = -m_nPaddingHeight;
			else if ( m_ptOffset.y > m_nPaddingHeight )
				m_ptOffset.y = m_nPaddingHeight;
		}
		else // Oversized
		{
			if ( m_ptOffset.y < m_nPaddingHeight )
				m_ptOffset.y = m_nPaddingHeight;
			else if ( m_ptOffset.y > -m_nPaddingHeight )
				m_ptOffset.y = -m_nPaddingHeight;
		}

		Invalidate();
	}

	return 0;
}

LRESULT CImageWindow::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( m_hBitmap != NULL )
	{
		HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

		if ( ! m_bFullSize ) OnActualSize();

		short nDelta = (short)HIWORD(wParam) / WHEEL_DELTA;

		m_ptOffset.x = (short)( m_ptOffset.x / m_nZoomFactor );
		m_ptOffset.y = (short)( m_ptOffset.y / m_nZoomFactor );

		m_nZoomIndex += ( nDelta > 0 ? 1 : -1 );
		m_nZoomFactor = 1.0f;

		if ( m_nZoomIndex > 0 )
		{
			for ( int nRun = m_nZoomIndex ; nRun ; nRun-- )
			{
				m_nZoomFactor *= 1.5f;
			}
		}
		else if ( m_nZoomIndex < 0 )
		{
			for ( int nRun = -m_nZoomIndex ; nRun ; nRun-- )
			{
				m_nZoomFactor /= 1.5f;
			}
		}

		m_ptOffset.x = (short)( m_ptOffset.x * m_nZoomFactor );
		m_ptOffset.y = (short)( m_ptOffset.y * m_nZoomFactor );

		RescaleImage();
		SetCursor( hCursor );
	}

	return 0;
}

LRESULT CImageWindow::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	if ( m_bDrag && m_bFullSize && m_pPlugin != NULL && LOWORD(lParam) == HTCLIENT )
	{
		// If we're in full size mode, show a hand cursor over the client area (for drag)
		SetCursor( m_pPlugin->m_hcMove );
		ShowCursor( TRUE );
	}
	else
	{
		// Otherwise, don't handle the message
		bHandled = FALSE;
	}
	return 0;
}

LRESULT CImageWindow::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Do an automatic refresh on timer ID 2.  Envy sends a global heartbeat on timer ID1,
	// so we set bHandled to false and pass the message back.

	if ( wParam == 2 ) Refresh();
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CImageWindow IPluginWindowOwner command implementation

HRESULT STDMETHODCALLTYPE CImageWindow::OnUpdate(UINT nCommandID, TRISTATE __RPC_FAR* /*pbVisible*/, TRISTATE __RPC_FAR* /*pbEnabled*/, TRISTATE __RPC_FAR* pbChecked)
{
	// The OnUpdate() method is invoked when Envy needs to update the state of a command
	// in its user interface.  This provides an opportunity to show or hide, enable or disable,
	// and check or uncheck user interface commands.  Because of the unified command architecture,
	// it does not matter if the command is in a menu or a toolbar or something else entirely.

	// The nCommandID argument is the ID of the command being updated.  You should check this against
	// a list of command IDs your plugin has registered.  If you don't get a match, return S_FALSE.
	// Unless you have a really good reason, you don't want to mess with commands that you didn't register
	// (for one thing, you probably won't know what their ID number is).
	// The S_FALSE code tells Envy to keep looking.

	// If you do find a match, you should modify pbVisible, pbEnabled and pbChecked.  Each is a "tri-state"
	// enumeration, defaulting to TSUNKNOWN.  Set TSTRUE to activate, or TSFALSE to deactivate.  Then,
	// return S_OK to indicate that you are responsible for this command, and have updated it.

	// You must check whether pbVisible, pbEnabled and pbChecked are NULL before reading or writing to them,
	// as one or more of them may be NULL if it is not required.

	// Note that IPluginWindowOwner::OnUpdate() is different to ICommandPlugin::OnUpdate() because it is
	// only called if the plugin window in question is active.  If you return S_OK here, no further
	// action will be taken.  If you return S_FALSE, the generic course of action will proceed.

	// We check our nCommandID against the command IDs stored in the plugin, from our earlier registration

	if ( nCommandID == m_pPlugin->m_nCmdBestFit )
	{
		// Check this option if full size mode is off (smart sized)
		if ( pbChecked ) *pbChecked = ( m_bFullSize == FALSE ) ? TRI_TRUE : TRI_FALSE;
		return S_OK;
	}
	if ( nCommandID == m_pPlugin->m_nCmdActualSize )
	{
		// Check this option if full size mode is on (actual sized)
		if ( pbChecked ) *pbChecked = ( m_bFullSize == TRUE ) ? TRI_TRUE : TRI_FALSE;
		return S_OK;
	}
	if ( nCommandID == m_pPlugin->m_nCmdRefresh ||
		 nCommandID == m_pPlugin->m_nCmdClose ||
		 nCommandID == m_pPlugin->m_nCmdDelete ||
		 nCommandID == m_pPlugin->m_nCmdNext ||
		 nCommandID == m_pPlugin->m_nCmdPrior ||
		 nCommandID == m_pPlugin->m_nCmdFirst ||
		 nCommandID == m_pPlugin->m_nCmdLast )
	{
		// These commands are ours, so we return S_OK, even though we haven't touched them.
		// Commands are enabled by default.
		return S_OK;
	}

	// No match, so return S_FALSE to let something else handle it.
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CImageWindow::OnCommand(UINT nCommandID)
{
	// The OnCommand() method is invoked whenever the user invokes a command.
	// This applies to ANY command in the unified architecture, which could be a built-in command,
	// a command you registered, or a command registered by another plugin.

	// Return S_OK if you are handling the command, or S_FALSE if Envy should keep looking.
	// Failure codes (E_*) will also cause Envy to stop looking for a handler.

	// Typically you would check the nCommandID argument against a list of command IDs
	// that you have registered, and only return S_OK if you get a match.
	// If the command is not currently available, you'd return E_UNEXPECTED.

	// However, for a good cause, you could also check for internal commands from the
	// base Envy UI (those which did not come from plugins).
	// These have fixed command IDs, so they can be safely detected.
	// If you return S_OK for one of these, Envy won't take its default action.

	// Note that IPluginWindowOwner::OnCommand() is different to ICommandPlugin::OnCommand()
	// because it is only called if the plugin window in question is active.
	// If you return S_OK here, no further action will be taken.
	// If you return S_FALSE, the generic course of action will proceed.

	// We check our nCommandID against the command IDs stored in the plugin, from our earlier registration

	if ( nCommandID == m_pPlugin->m_nCmdBestFit )
		OnBestFit();
	else if ( nCommandID == m_pPlugin->m_nCmdActualSize )
		OnActualSize();
	else if ( nCommandID == m_pPlugin->m_nCmdRefresh )
		Refresh();
	else if ( nCommandID == m_pPlugin->m_nCmdDelete )
		Delete();
	else if ( nCommandID == m_pPlugin->m_nCmdClose )
		PostMessage( WM_CLOSE );
	else if ( nCommandID == m_pPlugin->m_nCmdNext )
		OnNext();
	else if ( nCommandID == m_pPlugin->m_nCmdPrior )
		OnPrevious();
	else if ( nCommandID == m_pPlugin->m_nCmdFirst )
		OnFirst();
	else if ( nCommandID == m_pPlugin->m_nCmdLast )
		OnLast();
	else	// No match, return S_FALSE to let something else handle it.
		return S_FALSE;

	return S_OK;
}

void CImageWindow::OnBestFit()
{
	// Select best fit mode
	if ( m_bFullSize == FALSE ) return;

	m_bFullSize		= FALSE;
	m_ptOffset.x	= 0;
	m_ptOffset.y	= 0;
	m_nZoomIndex	= 0;
	m_nZoomFactor	= 1.0f;

	RescaleImage();
}

void CImageWindow::OnActualSize()
{
	// Select actual size mode (full size)
	if ( m_bFullSize == TRUE ) return;

	m_bFullSize		= TRUE;
	m_ptOffset.x	= 0;
	m_ptOffset.y	= 0;
	m_nZoomIndex	= 0;
	m_nZoomFactor	= 1.0f;

	RescaleImage();
}

// Navigation

void CImageWindow::OnNext()
{
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	BOOL bFound = FALSE;

	LPCTSTR szFile = m_sFile;
	LPCTSTR szFileName = PathFindFileName( szFile );
	CString sSearchDir = m_sFile.Left( (int)( szFileName - szFile ) );

	WIN32_FIND_DATA wfd = {};
	HANDLE hFind = FindFirstFile( sSearchDir + L"*.*", &wfd );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ( wfd.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) == 0 )
			{
				if ( bFound )
				{
					m_sFile = sSearchDir + wfd.cFileName;
					if ( Refresh() )
						break;	// Next file successfully loaded
				}

				if ( _tcsicmp( szFileName, wfd.cFileName ) == 0 )
					bFound = TRUE;
			}
		}
		while ( FindNextFile( hFind, &wfd ) );

		FindClose( hFind );
	}

	SetCursor( hCursor );
}

void CImageWindow::OnPrevious()
{
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	CAtlList< CString > sPrevFiles;

	LPCTSTR szFile = m_sFile;
	LPCTSTR szFileName = PathFindFileName( szFile );
	CString sSearchDir = m_sFile.Left( (int)( szFileName - szFile ) );

	WIN32_FIND_DATA wfd = {};
	HANDLE hFind = FindFirstFile( sSearchDir + L"*.*", &wfd );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ( wfd.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) == 0 )
			{
				if ( _tcsicmp( szFileName, wfd.cFileName ) == 0 )
				{
					for ( POSITION pos = sPrevFiles.GetHeadPosition(); pos ; )
					{
						m_sFile = sPrevFiles.GetNext( pos );
						if ( Refresh() )
							break;	// Previous file successfully loaded
					}

					break;
				}

				sPrevFiles.AddHead( sSearchDir + wfd.cFileName );
			}
		}
		while ( FindNextFile( hFind, &wfd ) );

		FindClose( hFind );
	}

	SetCursor( hCursor );
}

void CImageWindow::OnFirst()
{
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	LPCTSTR szFile = m_sFile;
	LPCTSTR szFileName = PathFindFileName( szFile );
	CString sSearchDir = m_sFile.Left( (int)( szFileName - szFile ) );

	WIN32_FIND_DATA wfd = {};
	HANDLE hFind = FindFirstFile( sSearchDir + L"*.*", &wfd );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ( wfd.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) == 0 )
			{
				m_sFile = sSearchDir + wfd.cFileName;
				if ( Refresh() )
					break;	// First file successfully loaded
			}
		}
		while ( FindNextFile( hFind, &wfd ) );

		FindClose( hFind );
	}

	SetCursor( hCursor );
}

void CImageWindow::OnLast()
{
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	CAtlList< CString > sPrevFiles;

	LPCTSTR szFile = m_sFile;
	LPCTSTR szFileName = PathFindFileName( szFile );
	CString sSearchDir = m_sFile.Left( (int)( szFileName - szFile ) );

	WIN32_FIND_DATA wfd = {};
	HANDLE hFind = FindFirstFile( sSearchDir + L"*.*", &wfd );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ( wfd.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) == 0 )
				sPrevFiles.AddHead( sSearchDir + wfd.cFileName );
		}
		while ( FindNextFile( hFind, &wfd ) );

		FindClose( hFind );
	}

	for ( POSITION pos = sPrevFiles.GetHeadPosition(); pos ; )
	{
		m_sFile = sPrevFiles.GetNext( pos );
		if ( Refresh() )
			break;	// Last file successfully loaded
	}

	SetCursor( hCursor );
}

void CImageWindow::Delete()
{
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	LPCTSTR szFile = m_sFile;
	LPCTSTR szFileName = PathFindFileName( szFile );
	CString sSearchDir = m_sFile.Left( (int)( szFileName - szFile ) );

	DWORD dwAttr = GetFileAttributes( m_sFile );
	if (  dwAttr != INVALID_FILE_ATTRIBUTES &&			// Filename exists
		( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) == 0 )	// Not a folder
	{
		CString strPath = m_sFile + '\0';	// Double null terminated
		OnNext();

		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0 )
		{
			DeleteFile( strPath );
		}
		else // Use Recycle Bin by default
		{
			SHFILEOPSTRUCT sfo = {};
			sfo.hwnd = GetDesktopWindow();
			sfo.pFrom = strPath;
			sfo.wFunc = FO_DELETE;
			sfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NORECURSION;	// FOF_NO_UI
			SHFileOperation( &sfo );
		}
	}

	SetCursor( hCursor );
}
