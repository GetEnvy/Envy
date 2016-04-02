//
// Images.cpp
//
// This file is part of Envy (getenvy.com) © 2010-2016
// All work here is original and released as-is under Persistent Public Domain [PPD]
//

// Dynamic Watermarks  (Common volatile skinned buttons etc.)

#include "StdAfx.h"
#include "Settings.h"
#include "Images.h"
#include "Colors.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

CImages Images;


//////////////////////////////////////////////////////////////////////
// CImages construction

//CImages::CImages()
//{
//}

//CImages::~CImages()
//{
//}

//////////////////////////////////////////////////////////////////////
// CImages Bitmap Objects

void CImages::DeleteObjects()
{
	if ( m_brDialog.m_hObject ) m_brDialog.DeleteObject();
	if ( m_brDialogPanel.m_hObject ) m_brDialogPanel.DeleteObject();
	if ( m_brMediaControl.m_hObject ) m_brMediaControl.DeleteObject();

	if ( m_bmBanner.m_hObject ) m_bmBanner.DeleteObject();
	if ( m_bmBannerEdge.m_hObject ) m_bmBannerEdge.DeleteObject();
	if ( m_bmDialog.m_hObject ) m_bmDialog.DeleteObject();
	if ( m_bmDialogPanel.m_hObject ) m_bmDialogPanel.DeleteObject();
	if ( m_bmPanelMark.m_hObject ) m_bmPanelMark.DeleteObject();
	if ( m_bmMediaStatusBar.m_hObject ) m_bmMediaStatusBar.DeleteObject();

	if ( m_bmSelected.m_hObject ) m_bmSelected.DeleteObject();
	if ( m_bmSelectedGrey.m_hObject ) m_bmSelectedGrey.DeleteObject();
	if ( m_bmMenuSelected.m_hObject ) m_bmMenuSelected.DeleteObject();
	if ( m_bmMenuSelectedEdge.m_hObject ) m_bmMenuSelectedEdge.DeleteObject();
	if ( m_bmMenuDisabled.m_hObject ) m_bmMenuDisabled.DeleteObject();
	if ( m_bmMenuDisabledEdge.m_hObject ) m_bmMenuDisabledEdge.DeleteObject();
	if ( m_bmProgress.m_hObject ) m_bmProgress.DeleteObject();
	if ( m_bmProgressEdge.m_hObject ) m_bmProgressEdge.DeleteObject();
	if ( m_bmProgressNone.m_hObject ) m_bmProgressNone.DeleteObject();
	if ( m_bmProgressShaded.m_hObject ) m_bmProgressShaded.DeleteObject();
	if ( m_bmToolTip.m_hObject ) m_bmToolTip.DeleteObject();

	if ( m_bmIconButton.m_hObject ) m_bmIconButton.DeleteObject();
	if ( m_bmIconButtonHover.m_hObject ) m_bmIconButtonHover.DeleteObject();
	if ( m_bmIconButtonPress.m_hObject ) m_bmIconButtonPress.DeleteObject();
	if ( m_bmIconButtonActive.m_hObject ) m_bmIconButtonActive.DeleteObject();
	if ( m_bmIconButtonDisabled.m_hObject ) m_bmIconButtonDisabled.DeleteObject();

	if ( m_bmRichButton.m_hObject ) m_bmRichButton.DeleteObject();
	if ( m_bmRichButtonEdge.m_hObject ) m_bmRichButtonEdge.DeleteObject();
	if ( m_bmRichButtonHover.m_hObject ) m_bmRichButtonHover.DeleteObject();
	if ( m_bmRichButtonHoverEdge.m_hObject ) m_bmRichButtonHoverEdge.DeleteObject();
	if ( m_bmRichButtonPress.m_hObject ) m_bmRichButtonPress.DeleteObject();
	if ( m_bmRichButtonPressEdge.m_hObject ) m_bmRichButtonPressEdge.DeleteObject();
	if ( m_bmRichButtonActive.m_hObject ) m_bmRichButtonActive.DeleteObject();
	if ( m_bmRichButtonActiveEdge.m_hObject ) m_bmRichButtonActiveEdge.DeleteObject();
	if ( m_bmRichButtonDisabled.m_hObject ) m_bmRichButtonDisabled.DeleteObject();
	if ( m_bmRichButtonDisabledEdge.m_hObject ) m_bmRichButtonDisabledEdge.DeleteObject();

	if ( m_bmToolbarButton.m_hObject ) m_bmToolbarButton.DeleteObject();
	if ( m_bmToolbarButtonEdge.m_hObject ) m_bmToolbarButtonEdge.DeleteObject();
	if ( m_bmToolbarButtonHover.m_hObject ) m_bmToolbarButtonHover.DeleteObject();
	if ( m_bmToolbarButtonHoverEdge.m_hObject ) m_bmToolbarButtonHoverEdge.DeleteObject();
	if ( m_bmToolbarButtonPress.m_hObject ) m_bmToolbarButtonPress.DeleteObject();
	if ( m_bmToolbarButtonPressEdge.m_hObject ) m_bmToolbarButtonPressEdge.DeleteObject();
	if ( m_bmToolbarButtonActive.m_hObject ) m_bmToolbarButtonActive.DeleteObject();
	if ( m_bmToolbarButtonActiveEdge.m_hObject ) m_bmToolbarButtonActiveEdge.DeleteObject();
	if ( m_bmToolbarButtonDisabled.m_hObject ) m_bmToolbarButtonDisabled.DeleteObject();
	if ( m_bmToolbarButtonDisabledEdge.m_hObject ) m_bmToolbarButtonDisabledEdge.DeleteObject();
	if ( m_bmToolbarSeparator.m_hObject ) m_bmToolbarSeparator.DeleteObject();
	if ( m_bmToolbar.m_hObject ) m_bmToolbar.DeleteObject();

	if ( m_bmMenubarItem.m_hObject ) m_bmMenubarItem.DeleteObject();
	if ( m_bmMenubarItemEdge.m_hObject ) m_bmMenubarItemEdge.DeleteObject();
	if ( m_bmMenubarItemHover.m_hObject ) m_bmMenubarItemHover.DeleteObject();
	if ( m_bmMenubarItemHoverEdge.m_hObject ) m_bmMenubarItemHoverEdge.DeleteObject();
	if ( m_bmMenubarItemPress.m_hObject ) m_bmMenubarItemPress.DeleteObject();
	if ( m_bmMenubarItemPressEdge.m_hObject ) m_bmMenubarItemPressEdge.DeleteObject();
	if ( m_bmMenubarButton.m_hObject ) m_bmMenubarButton.DeleteObject();
	if ( m_bmMenubarButtonEdge.m_hObject ) m_bmMenubarButtonEdge.DeleteObject();
	if ( m_bmMenubarButtonHover.m_hObject ) m_bmMenubarButtonHover.DeleteObject();
	if ( m_bmMenubarButtonHoverEdge.m_hObject ) m_bmMenubarButtonHoverEdge.DeleteObject();
	if ( m_bmMenubarButtonPress.m_hObject ) m_bmMenubarButtonPress.DeleteObject();
	if ( m_bmMenubarButtonPressEdge.m_hObject ) m_bmMenubarButtonPressEdge.DeleteObject();
	if ( m_bmMenubarButtonActive.m_hObject ) m_bmMenubarButtonActive.DeleteObject();
	if ( m_bmMenubarButtonActiveEdge.m_hObject ) m_bmMenubarButtonActiveEdge.DeleteObject();

	if ( m_bmTaskbarButton.m_hObject ) m_bmTaskbarButton.DeleteObject();
	if ( m_bmTaskbarButtonEdge.m_hObject ) m_bmTaskbarButtonEdge.DeleteObject();
	if ( m_bmTaskbarButtonActive.m_hObject ) m_bmTaskbarButtonActive.DeleteObject();
	if ( m_bmTaskbarButtonActiveEdge.m_hObject ) m_bmTaskbarButtonActiveEdge.DeleteObject();
	if ( m_bmTaskbarButtonHover.m_hObject ) m_bmTaskbarButtonHover.DeleteObject();
	if ( m_bmTaskbarButtonHoverEdge.m_hObject ) m_bmTaskbarButtonHoverEdge.DeleteObject();
	if ( m_bmTaskbarButtonPress.m_hObject ) m_bmTaskbarButtonPress.DeleteObject();
	if ( m_bmTaskbarButtonPressEdge.m_hObject ) m_bmTaskbarButtonPressEdge.DeleteObject();

	if ( m_bmDownloadGroup.m_hObject ) m_bmDownloadGroup.DeleteObject();
	if ( m_bmDownloadGroupEdge.m_hObject ) m_bmDownloadGroupEdge.DeleteObject();
	if ( m_bmDownloadGroupActive.m_hObject ) m_bmDownloadGroupActive.DeleteObject();
	if ( m_bmDownloadGroupActiveEdge.m_hObject ) m_bmDownloadGroupActiveEdge.DeleteObject();
	if ( m_bmDownloadGroupHover.m_hObject ) m_bmDownloadGroupHover.DeleteObject();
	if ( m_bmDownloadGroupHoverEdge.m_hObject ) m_bmDownloadGroupHoverEdge.DeleteObject();
	if ( m_bmDownloadGroupPress.m_hObject ) m_bmDownloadGroupPress.DeleteObject();
	if ( m_bmDownloadGroupPressEdge.m_hObject ) m_bmDownloadGroupPressEdge.DeleteObject();

	if ( m_bmButtonMapIconbox.m_hObject ) m_bmButtonMapIconbox.DeleteObject();
	if ( m_bmButtonMapRichdoc.m_hObject ) m_bmButtonMapRichdoc.DeleteObject();
	if ( m_bmButtonMapToolbar.m_hObject ) m_bmButtonMapToolbar.DeleteObject();
	if ( m_bmButtonMapMenubar.m_hObject ) m_bmButtonMapMenubar.DeleteObject();
	if ( m_bmButtonMapMenutext.m_hObject ) m_bmButtonMapMenutext.DeleteObject();
	if ( m_bmButtonMapMenuselect.m_hObject ) m_bmButtonMapMenuselect.DeleteObject();
	if ( m_bmButtonMapSelect.m_hObject ) m_bmButtonMapSelect.DeleteObject();
	if ( m_bmButtonMapProgressbar.m_hObject ) m_bmButtonMapProgressbar.DeleteObject();
	if ( m_bmButtonMapDownloadgroup.m_hObject ) m_bmButtonMapDownloadgroup.DeleteObject();
	if ( m_bmButtonMapTaskbar.m_hObject ) m_bmButtonMapTaskbar.DeleteObject();
}

void CImages::Load()
{
	DeleteObjects();

	SkinImage( &m_bmSelected, L"System.Highlight" ) ||
		SkinImage( &m_bmSelected, L"CTransfers.Selected" );

	SkinImage( &m_bmSelectedGrey, L"System.Highlight.Inactive" ) ||
		SkinImage( &m_bmSelectedGrey, L"CTransfers.Selected.Inactive" );

	if ( SkinImage( &m_bmMenuSelected, L"System.MenuSelected" ) )
		SkinImage( &m_bmMenuSelectedEdge, L"System.MenuSelectedEdge" );
	else if ( SkinImage( &m_bmMenuSelected, L"CCoolMenu.Hover" ) )
		SkinImage( &m_bmMenuSelectedEdge, L"CCoolMenu.Hover.Edge" );

	if ( SkinImage( &m_bmMenuDisabled, L"System.MenuDisabled" ) )
		SkinImage( &m_bmMenuDisabledEdge, L"System.MenuDisabledEdge" );
	else if ( SkinImage( &m_bmMenuDisabled, L"CCoolMenu.Disabled" ) )
		SkinImage( &m_bmMenuDisabledEdge, L"CCoolMenu.Disabled.Edge" );

	if ( SkinImage( &m_bmProgress, L"MenubarItem" ) )
		SkinImage( &m_bmProgressEdge, L"MenubarItemEdge" );
	else if ( SkinImage( &m_bmProgress, L"ProgressBar" ) )
		SkinImage( &m_bmProgressEdge, L"ProgressBar.Edge" );

	SkinImage( &m_bmProgressNone, L"System.Progress.None" ) ||
		SkinImage( &m_bmProgressNone, L"ProgressBar.None" );

	SkinImage( &m_bmProgressShaded, L"System.Progress.Shaded" ) ||
		SkinImage( &m_bmProgressShaded, L"ProgressBar.Shaded" );

	SkinImage( &m_bmToolTip, L"System.ToolTip" ) ||
		SkinImage( &m_bmToolTip, L"System.Tooltips" ) ||
		SkinImage( &m_bmToolTip, L"CToolTipCtrl" );

	if ( SkinImage( &m_bmDialog, L"System.Dialogs", FALSE ) ||
		 SkinImage( &m_bmDialog, L"CDialog", FALSE ) )
		m_brDialog.CreatePatternBrush( &m_bmDialog );
	else
		m_brDialog.CreateSolidBrush( Colors.m_crDialog );

	if ( SkinImage( &m_bmDialogPanel, L"System.DialogPanels", FALSE ) ||
		 SkinImage( &m_bmDialogPanel, L"CDialog.Panel", FALSE ) )
		m_brDialogPanel.CreatePatternBrush( &m_bmDialogPanel );
	else
		m_brDialogPanel.CreateSolidBrush( Colors.m_crDialogPanel );

	CBitmap bmBrush;
	// Attach( (HBRUSH)GetStockObject( NULL_BRUSH ) )  ?

	if ( SkinImage( &bmBrush, L"System.Dialogs.Control", FALSE ) ||
		 SkinImage( &bmBrush, L"CDialog.Control", FALSE ) )
		m_brDialog.CreatePatternBrush( &bmBrush );

	if ( SkinImage( &bmBrush, L"System.DialogPanels.Control", FALSE ) ||
		 SkinImage( &bmBrush, L"CDialog.Panel.Control", FALSE ) )
		m_brDialogPanel.CreatePatternBrush( &bmBrush );

	if ( SkinImage( &bmBrush, L"CMediaFrame.Slider", FALSE ) ||
		 SkinImage( &bmBrush, L"CCoolbar.Control", FALSE ) )
		m_brMediaControl.CreatePatternBrush( &bmBrush );
	else
		m_brMediaControl.CreateSolidBrush( Colors.m_crMidtone );

	// Note: PreBlend here breaks default IDB_BANNER (Why?)
	if ( SkinImage( &m_bmBanner, L"System.Header", FALSE ) )
		SkinImage( &m_bmBannerEdge, L"System.Header.Edge", FALSE );
	else if ( SkinImage( &m_bmBanner, L"Banner", FALSE ) )
		SkinImage( &m_bmBannerEdge, L"Banner.Edge", FALSE );

	Skin.m_nBanner = m_nBanner = m_bmBanner.m_hObject ? m_bmBanner.GetBitmapDimension().cy : 0;	// IDB_BANNER

	if ( ! SkinImage( &m_bmPanelMark, L"CPanelWnd.Caption" ) && Colors.m_crPanelBack == RGB_DEFAULT_CASE )
		m_bmPanelMark.LoadBitmap( IDB_PANEL_MARK );				// Special-case default resource handling

	SkinImage( &m_bmMediaStatusBar, L"CMediaFrame.StatusBar" );

	// Note "System.Toolbars" fallback handled at toolbar creation

	// Button states:

	// Dialog Single-Icon Buttons:

	SkinImage( &m_bmIconButton, L"IconButton" );
	SkinImage( &m_bmIconButtonHover, L"IconButton.Hover" );
	SkinImage( &m_bmIconButtonPress, L"IconButton.Press" );
	SkinImage( &m_bmIconButtonActive, L"IconButton.Active" );
	SkinImage( &m_bmIconButtonDisabled, L"IconButton.Disabled" );

	// RichDoc Buttons (Search):

	if ( SkinImage( &m_bmRichButton, L"RichButton" ) )
		SkinImage( &m_bmRichButtonEdge, L"RichButtonEdge" );

	if ( SkinImage( &m_bmRichButtonHover, L"RichButton.Hover" ) )
		SkinImage( &m_bmRichButtonHoverEdge, L"RichButtonEdge.Hover" );

	if ( SkinImage( &m_bmRichButtonPress, L"RichButton.Press" ) )
		SkinImage( &m_bmRichButtonPressEdge, L"RichButtonEdge.Press" );

	if ( SkinImage( &m_bmRichButtonActive, L"RichButton.Active" ) )
		SkinImage( &m_bmRichButtonActiveEdge, L"RichButtonEdge.Active" );

	if ( SkinImage( &m_bmRichButtonDisabled, L"RichButton.Disabled" ) )
		SkinImage( &m_bmRichButtonDisabledEdge, L"RichButtonEdge.Disabled" );

	// Command Toolbar Buttons:

	if ( SkinImage( &m_bmToolbarButton, L"ToolbarButton" ) )
		SkinImage( &m_bmToolbarButtonEdge, L"ToolbarButtonEdge" );
	else if ( SkinImage( &m_bmToolbarButton, L"CCoolbar.Up" ) )
		SkinImage( &m_bmToolbarButtonEdge, L"CCoolbar.Up.Edge" );

	if ( SkinImage( &m_bmToolbarButtonHover, L"ToolbarButton.Hover" ) )
		SkinImage( &m_bmToolbarButtonHoverEdge, L"ToolbarButtonEdge.Hover" );
	else if ( SkinImage( &m_bmToolbarButtonHover, L"CCoolbar.Hover" ) )
		SkinImage( &m_bmToolbarButtonHoverEdge, L"CCoolbar.Hover.Edge" );

	if ( SkinImage( &m_bmToolbarButtonPress, L"ToolbarButton.Press" ) )
		SkinImage( &m_bmToolbarButtonPressEdge, L"ToolbarButtonEdge.Press" );
	else if ( SkinImage( &m_bmToolbarButtonPress, L"CCoolbar.Down" ) )
		SkinImage( &m_bmToolbarButtonPressEdge, L"CCoolbar.Down.Edge" );

	if ( SkinImage( &m_bmToolbarButtonActive, L"ToolbarButton.Active" ) )
		SkinImage( &m_bmToolbarButtonActiveEdge, L"ToolbarButtonEdge.Active" );
	else if ( SkinImage( &m_bmToolbarButtonActive, L"CCoolbar.Checked" ) )
		SkinImage( &m_bmToolbarButtonActiveEdge, L"CCoolbar.Checked.Edge" );

	if ( SkinImage( &m_bmToolbarButtonDisabled, L"ToolbarButton.Disabled" ) )
		SkinImage( &m_bmToolbarButtonDisabledEdge, L"ToolbarButtonEdge.Disabled" );
	else if ( SkinImage( &m_bmToolbarButtonDisabled, L"CCoolbar.Disabled" ) )
		SkinImage( &m_bmToolbarButtonDisabledEdge, L"CCoolbar.Disabled.Edge" );

	SkinImage( &m_bmToolbarSeparator, L"System.Toolbars.Separator" ) ||
		SkinImage( &m_bmToolbarSeparator, L"CCoolbar.Separator" ) ||
		SkinImage( &m_bmToolbarSeparator, L"ToolbarSeparator" );

	SkinImage( &m_bmToolbar, L"System.Toolbars" ) ||
		SkinImage( &m_bmToolbar, L"CCoolbar" ) ||
		SkinImage( &m_bmToolbar, L"Toolbar" );

	// "System.Toolbars.CClass" override in Skin

	// Main Menubar (Text) Buttons:

	if ( SkinImage( &m_bmMenubarItem, L"MenubarItem" ) )
		SkinImage( &m_bmMenubarItemEdge, L"MenubarItemEdge" );
	else if ( SkinImage( &m_bmMenubarItem, L"CCoolMenuItem.Up" ) )
		SkinImage( &m_bmMenubarItemEdge, L"CCoolMenuItem.Up.Edge" );

	if ( SkinImage( &m_bmMenubarItemHover, L"MenubarItem.Hover" ) )
		SkinImage( &m_bmMenubarItemHoverEdge, L"MenubarItemEdge.Hover" );
	else if ( SkinImage( &m_bmMenubarItemHover, L"CCoolMenuItem.Hover" ) )
		SkinImage( &m_bmMenubarItemHoverEdge, L"CCoolMenuItem.Hover.Edge" );

	if ( SkinImage( &m_bmMenubarItemPress, L"MenubarItem.Press" ) )
		SkinImage( &m_bmMenubarItemPressEdge, L"MenubarItemEdge.Press" );
	else if ( SkinImage( &m_bmMenubarItemPress, L"CCoolMenuItem.Down" ) )
		SkinImage( &m_bmMenubarItemPressEdge, L"CCoolMenuItem.Down.Edge" );

	if ( SkinImage( &m_bmMenubarButton, L"MenubarButton" ) )
		SkinImage( &m_bmMenubarButtonEdge, L"MenubarButtonEdge" );
	else if ( SkinImage( &m_bmMenubarButton, L"CCoolMenuBar.Up" ) )
		SkinImage( &m_bmMenubarButtonEdge, L"CCoolMenuBar.Up.Edge" );

	if ( SkinImage( &m_bmMenubarButtonHover, L"MenubarButton.Hover" ) )
		SkinImage( &m_bmMenubarButtonHoverEdge, L"MenubarButtonEdge.Hover" );
	else if ( SkinImage( &m_bmMenubarButtonHover, L"CCoolMenuBar.Hover" ) )
		SkinImage( &m_bmMenubarButtonHoverEdge, L"CCoolMenuBar.Hover.Edge" );

	if ( SkinImage( &m_bmMenubarButtonPress, L"MenubarButton.Press" ) )
		SkinImage( &m_bmMenubarButtonPressEdge, L"MenubarButtonEdge.Press" );
	else if ( SkinImage( &m_bmMenubarButtonPress, L"CCoolMenuBar.Down" ) )
		SkinImage( &m_bmMenubarButtonPressEdge, L"CCoolMenuBar.Down.Edge" );

	if ( SkinImage( &m_bmMenubarButtonActive, L"MenubarButton.Active" ) )
		SkinImage( &m_bmMenubarButtonActiveEdge, L"MenubarButtonEdge.Active" );
	else if ( SkinImage( &m_bmMenubarButtonActive, L"CCoolMenuBar.Checked" ) )
		SkinImage( &m_bmMenubarButtonActiveEdge, L"CCoolMenuBar.Checked.Edge" );

	// Taskbar Tabs:

	if ( SkinImage( &m_bmTaskbarButton, L"TaskbarButton" ) )
		SkinImage( &m_bmTaskbarButtonEdge, L"TaskbarButtonEdge" );
	else if ( SkinImage( &m_bmTaskbarButton, L"CWndTabBar.Tab" ) )
		SkinImage( &m_bmTaskbarButtonEdge, L"CWndTabBar.Edge" );

	if ( SkinImage( &m_bmTaskbarButtonActive, L"TaskbarButton.Active" ) )
		SkinImage( &m_bmTaskbarButtonActiveEdge, L"TaskbarButtonEdge.Active" );
	else if ( SkinImage( &m_bmTaskbarButtonActive, L"CWndTabBar.Active" ) )
		SkinImage( &m_bmTaskbarButtonActiveEdge, L"CWndTabBar.Active.Edge" );

	if ( SkinImage( &m_bmTaskbarButtonHover, L"TaskbarButton.Hover" ) )
		SkinImage( &m_bmTaskbarButtonHoverEdge, L"TaskbarButtonEdge.Hover" );
	else if ( SkinImage( &m_bmTaskbarButtonHover, L"CWndTabBar.Hover" ) )
		SkinImage( &m_bmTaskbarButtonHoverEdge, L"CWndTabBar.Hover.Edge" );

	if ( SkinImage( &m_bmTaskbarButtonPress, L"TaskbarButton.Press" ) )
		SkinImage( &m_bmTaskbarButtonPressEdge, L"TaskbarButtonEdge.Press" );
	else if ( SkinImage( &m_bmTaskbarButtonPress, L"CWndTabBar.Active.Hover" ) )
		SkinImage( &m_bmTaskbarButtonPressEdge, L"CWndTabBar.Active.Hover.Edge" );

	// Download Groups Bar Tabs:

	if ( SkinImage( &m_bmDownloadGroup, L"DownloadGroup" ) )
		SkinImage( &m_bmDownloadGroupEdge, L"DownloadGroupEdge" );
	else if ( SkinImage( &m_bmDownloadGroup, L"CDownloadTabBar.Up" ) )
		SkinImage( &m_bmDownloadGroupEdge, L"CDownloadTabBar.Up.Edge" );

	if ( SkinImage( &m_bmDownloadGroupActive, L"DownloadGroup.Active" ) )
		SkinImage( &m_bmDownloadGroupActiveEdge, L"DownloadGroupEdge.Active" );
	else if ( SkinImage( &m_bmDownloadGroupActive, L"CDownloadTabBar.Active" ) )
		SkinImage( &m_bmDownloadGroupActiveEdge, L"CDownloadTabBar.Active.Edge" );

	if ( SkinImage( &m_bmDownloadGroupHover, L"DownloadGroup.Hover" ) )
		SkinImage( &m_bmDownloadGroupHoverEdge, L"DownloadGroupEdge.Hover" );
	else if ( SkinImage( &m_bmDownloadGroupHover, L"CDownloadTabBar.Hover" ) )
		SkinImage( &m_bmDownloadGroupHoverEdge, L"CDownloadTabBar.Hover.Edge" );

	if ( SkinImage( &m_bmDownloadGroupPress, L"DownloadGroup.Press" ) )
		SkinImage( &m_bmDownloadGroupPressEdge, L"DownloadGroupEdge.Press" );
	else if ( SkinImage( &m_bmDownloadGroupPress, L"CDownloadTabBar.Active.Hover" ) )
		SkinImage( &m_bmDownloadGroupPressEdge, L"CDownloadTabBar.Active.Hover.Edge" );

	if ( SkinImage( &m_bmDownloadGroupDisabled, L"DownloadGroup.Disabled" ) )
		SkinImage( &m_bmDownloadGroupDisabledEdge, L"DownloadGroupEdge.Disabled" );
	else if ( SkinImage( &m_bmDownloadGroupDisabled, L"CDownloadTabBar.Disabled" ) )
		SkinImage( &m_bmDownloadGroupDisabledEdge, L"CDownloadTabBar.Disabled.Edge" );

	// Button Maps:

	SkinImage( &m_bmButtonMapIconbox, L"ButtonMap.Iconbox", TRUE, STATE_COUNT );
	SkinImage( &m_bmButtonMapRichdoc, L"ButtonMap.Richdoc", TRUE, STATE_COUNT );
	SkinImage( &m_bmButtonMapToolbar, L"ButtonMap.Toolbar", TRUE, STATE_COUNT );
	SkinImage( &m_bmButtonMapMenubar, L"ButtonMap.Menubar", TRUE, STATE_COUNT );
	SkinImage( &m_bmButtonMapMenutext, L"ButtonMap.MenuText", TRUE, 3 );
	SkinImage( &m_bmButtonMapMenuselect, L"ButtonMap.MenuSelect", TRUE, 3 );
	SkinImage( &m_bmButtonMapSelect, L"ButtonMap.Select", TRUE, 3 ) ||
		SkinImage( &m_bmButtonMapSelect, L"ButtonMap.Highlight", TRUE, 3 );
	SkinImage( &m_bmButtonMapProgressbar, L"ButtonMap.ProgressBar", TRUE, 3 );
	SkinImage( &m_bmButtonMapDownloadgroup, L"ButtonMap.DownloadGroup", TRUE, STATE_COUNT );
	SkinImage( &m_bmButtonMapTaskbar, L"ButtonMap.Taskbar", TRUE, STATE_COUNT );
}

BOOL CImages::SkinImage(CBitmap* bmImage, LPCTSTR pszName, BOOL bAllowAlpha /*1*/, UINT nStates /*1*/)
{
	if ( HBITMAP hImage = Skin.GetWatermark( pszName ) )
	{
		BITMAP pInfo;
		if ( bAllowAlpha )
			PreBlend( hImage );
		bmImage->Attach( hImage );
		bmImage->GetObject( sizeof( BITMAP ), &pInfo );
		if ( nStates > 1 )		// Special ButtonMap handling
			bmImage->SetBitmapDimension( ( Settings.Skin.ButtonEdge < pInfo.bmWidth ) ? Settings.Skin.ButtonEdge : pInfo.bmWidth, pInfo.bmHeight / nStates );
		else
			bmImage->SetBitmapDimension( pInfo.bmWidth, pInfo.bmHeight );
		return TRUE;
	}
	return FALSE;
}

void CImages::BlendAlpha(CBitmap* bmImage, COLORREF crBlend /*RGB(255,255,255)*/)
{
	BITMAP pInfo;
	bmImage->GetBitmap( &pInfo );
	if ( pInfo.bmBitsPixel != 32 )
		return;

	HBITMAP hImage = (HBITMAP)bmImage->Detach();

	const BYTE nRValue = GetBValue( crBlend );	// Flip
	const BYTE nGValue = GetGValue( crBlend );
	const BYTE nBValue = GetRValue( crBlend );	// Flip

	const int bufferSize = pInfo.bmWidthBytes * pInfo.bmHeight;
	BYTE* buffer = (BYTE*)malloc(bufferSize);
	GetBitmapBits( hImage, bufferSize, buffer );	// Get/SetBitmapBits() deprecated by MS, but useful here

	for ( int i = 0, j = 0 ; i < bufferSize ; i += 4, j += 3 )
	{
		DWORD nAlpha = (DWORD)buffer[i + 3];

		if ( nAlpha == 255 )
		{
			buffer[j + 0] = buffer[i + 0];
			buffer[j + 1] = buffer[i + 1];
			buffer[j + 2] = buffer[i + 2];
		}
		else if ( nAlpha == 0 )
		{
			buffer[j + 0] = nRValue;
			buffer[j + 1] = nGValue;
			buffer[j + 2] = nBValue;
		}
		else
		{
			DWORD dwInR = (DWORD)(buffer[i + 0]);
			DWORD dwInG = (DWORD)(buffer[i + 1]);
			DWORD dwInB = (DWORD)(buffer[i + 2]);

			if ( ! dwInR )
				buffer[j + 0] = (BYTE)( nAlpha > nRValue ? 0 : nRValue - nAlpha );
			else if ( dwInR == 255 )
				buffer[j + 0] = (BYTE)( nAlpha > ( 255 - nRValue ) ? 255 : nRValue + nAlpha );
			else
				buffer[j + 0] = (BYTE)( ( dwInR * nAlpha + (DWORD)nRValue * ( 255 - nAlpha ) ) / 255 );

			if ( ! dwInG )
				buffer[j + 1] = (BYTE)( nAlpha > nGValue ? 0 : nGValue - nAlpha );
			else if ( dwInG == 255 )
				buffer[j + 1] = (BYTE)( nAlpha > ( 255 - nGValue ) ? 255 : nGValue + nAlpha );
			else
				buffer[j + 1] = (BYTE)( ( dwInG * nAlpha + (DWORD)nGValue * ( 255 - nAlpha ) ) / 255 );

			if ( ! dwInB )
				buffer[j + 2] = (BYTE)( nAlpha > nBValue ? 0 : nBValue - nAlpha );
			else if ( dwInB == 255 )
				buffer[j + 2] = (BYTE)( nAlpha > ( 255 - nBValue ) ? 255 : nBValue + nAlpha );
			else
				buffer[j + 2] = (BYTE)( ( dwInB * nAlpha + (DWORD)nBValue * ( 255 - nAlpha ) ) / 255 );

		//	buffer[j + 0] = (BYTE)( ( (DWORD)(buffer[i + 0]) * nAlpha + (DWORD)nRValue * ( 255 - nAlpha ) ) / 255 );
		//	buffer[j + 1] = (BYTE)( ( (DWORD)(buffer[i + 1]) * nAlpha + (DWORD)nGValue * ( 255 - nAlpha ) ) / 255 );
		//	buffer[j + 2] = (BYTE)( ( (DWORD)(buffer[i + 2]) * nAlpha + (DWORD)nBValue * ( 255 - nAlpha ) ) / 255 );
		}
	}

	BITMAPV5HEADER pV5Header = {};
	pV5Header.bV5Size			= sizeof(BITMAPV5HEADER);
	pV5Header.bV5Width			= (LONG)pInfo.bmWidth;
	pV5Header.bV5Height			= (LONG)pInfo.bmHeight;
	pV5Header.bV5Planes			= 1;
	pV5Header.bV5BitCount		= 24;
	pV5Header.bV5Compression	= BI_RGB;
	pV5Header.bV5SizeImage		= pInfo.bmWidth * pInfo.bmHeight * 3;

	__try
	{
		void* pBits = NULL;
		hImage = CreateDIBSection( GetDC( 0 ), (BITMAPINFO*)&pV5Header, DIB_RGB_COLORS, (void**)&pBits, NULL, 0ul );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		hImage = NULL;
	}

	SetBitmapBits( hImage, bufferSize * 3 / 4, buffer );
	bmImage->Attach( hImage );

	delete buffer;
}

BOOL CImages::PreBlend(HBITMAP hButton)
{
	BITMAP pInfo;
	CBitmap::FromHandle( hButton )->GetObject( sizeof( BITMAP ), &pInfo );		// CBitmap*
	if ( pInfo.bmBitsPixel != 32 )
		return FALSE;

	// Pre-multiply for AlphaBlend transparency support (rgba from PNG)

	const int bufferSize = pInfo.bmWidthBytes * pInfo.bmHeight;
	BYTE* buffer = (BYTE*)malloc( bufferSize );
	GetBitmapBits( hButton, bufferSize, buffer );	// Get/SetBitmapBits() deprecated by MS, but useful here

	for ( int i = 0 ; i < bufferSize ; i += 4 )
	{
		if ( buffer[i + 3] == 255 ) continue;

		buffer[i + 0] = buffer[i + 0] * buffer[i + 3] / 255;
		buffer[i + 1] = buffer[i + 1] * buffer[i + 3] / 255;
		buffer[i + 2] = buffer[i + 2] * buffer[i + 3] / 255;
	}

	SetBitmapBits( hButton, bufferSize, buffer );
	delete buffer;

	return TRUE;
}

BOOL CImages::DrawImage(CDC* pDC, const CRect* prc, CBitmap* bmImage, BOOL bRepeat /*=TRUE*/)
{
	if ( ! bmImage->m_hObject || pDC == NULL || prc == NULL )
		return FALSE;

	CDC dcMark;
	CBitmap* pOldMark;
	BITMAP bmWatermark;

	dcMark.CreateCompatibleDC( pDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dcMark.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );

	pOldMark = (CBitmap*)dcMark.SelectObject( bmImage );
	bmImage->GetBitmap( &bmWatermark );

	if ( bRepeat )
	{
		for ( int nY = prc->top ; nY < prc->bottom ; nY += bmWatermark.bmHeight )
		{
			for ( int nX = prc->left ; nX < prc->right ; nX += bmWatermark.bmWidth )
			{
				pDC->BitBlt( nX, nY,
					min( bmWatermark.bmWidth, prc->right - nX ),
					min( bmWatermark.bmHeight, prc->bottom - nY ),
					&dcMark, 0, 0, SRCCOPY );
			}
		}
	}
	else
	{
		pDC->BitBlt( 0, 0,
			min( bmWatermark.bmWidth, prc->right ),
			min( bmWatermark.bmHeight, prc->bottom ),
			&dcMark, 0, 0, SRCCOPY );
	}

	dcMark.SelectObject( pOldMark );
	dcMark.DeleteDC();

	return TRUE;
}

BOOL CImages::DrawIconButton(CDC* pDC, const CRect* rc, CBitmap* bmButton)
{
	if ( ! bmButton->m_hObject || pDC == NULL || rc == NULL )
		return FALSE;

	CDC dcMark;
	dcMark.CreateCompatibleDC( pDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dcMark.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );

	CBitmap* pOld = (CBitmap*)dcMark.SelectObject( bmButton );

	CSize szButton( bmButton->GetBitmapDimension() );
	if ( szButton.cx > 16 && szButton.cy > 16 && szButton.cx < rc->Width() + 2 && szButton.cy < rc->Height() + 2 )
	{
		if ( ! m_bmDialog.m_hObject )
			pDC->FillSolidRect( rc, pDC->GetBkColor() );

		// Set button rect to centered image size
		CRect rcIcon( rc );
		rcIcon.top += ( rcIcon.Height() - szButton.cy ) / 2;
		rcIcon.left += ( rcIcon.Width() - szButton.cx ) / 2;
		rcIcon.right = rcIcon.left + szButton.cx;
		rcIcon.bottom = rcIcon.top + szButton.cy;

		pDC->BitBlt( rcIcon.left, rcIcon.top,
			szButton.cx, szButton.cy,
			&dcMark, 0, 0, SRCCOPY );
	}
	else
	{
		for ( int nY = rc->top ; nY < rc->bottom ; nY += szButton.cy )
		{
			for ( int nX = rc->left ; nX < rc->right ; nX += szButton.cx )
			{
				pDC->BitBlt( nX, nY,
					min( szButton.cx, rc->right - nX ),
					min( szButton.cy, rc->bottom - nY ),
					&dcMark, 0, 0, SRCCOPY );
			}
		}
	}

	dcMark.SelectObject( pOld );
	dcMark.DeleteDC();

	return TRUE;
}

BOOL CImages::DrawButton(CDC* pDC, const CRect* rc, CBitmap* bmButton, CBitmap* bmButtonEdge, BOOL bRTL)
{
	if ( ! bmButton->m_hObject || pDC == NULL || rc == NULL )
		return FALSE;

	static CDC dcMark;
	if ( ! dcMark.m_hDC )
		dcMark.CreateCompatibleDC( pDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dcMark.m_hDC, bRTL ? LAYOUT_RTL : LAYOUT_BITMAPORIENTATIONPRESERVED );

	CBitmap* pOld = (CBitmap*)dcMark.SelectObject( bmButton );

	BLENDFUNCTION bf;				// Set now if needed
	bf.BlendFlags = 0;				// Must be zero
	bf.BlendOp = AC_SRC_OVER;		// Must be defined
	bf.AlphaFormat = AC_SRC_ALPHA;	// Use bitmap alpha
	bf.SourceConstantAlpha = 0xFF;	// Opaque (Bitmap alpha only)

	const int nEdge = ( bmButtonEdge && bmButtonEdge->m_hObject ) ? bmButtonEdge->GetBitmapDimension().cx : 0;

	if ( rc->Width() > nEdge )
	{
		BITMAP pInfo;
		bmButton->GetBitmap( &pInfo );

		for ( int nY = rc->top ; nY < rc->bottom ; nY += pInfo.bmHeight )
		{
			for ( int nX = rc->left ; nX < rc->right - nEdge ; nX += pInfo.bmWidth )
			{
				const int nWidth  = min( pInfo.bmWidth, rc->right - nX - nEdge );
				const int nHeight = min( pInfo.bmHeight, rc->bottom - nY ); 	// No repeat (+ 1 to allow 1px overdraw?)

				if ( pInfo.bmBitsPixel == 32 )		// (Pre-multiplied for AlphaBlend Transparency)
					pDC->AlphaBlend( nX, nY, nWidth, nHeight, &dcMark, 0, 0, nWidth, nHeight, bf );
				else
					pDC->BitBlt( nX, nY, nWidth, nHeight, &dcMark, 0, 0, SRCCOPY );
			}
		}
	}

	if ( nEdge > 0 )
	{
		BITMAP pInfo;
		bmButtonEdge->GetBitmap( &pInfo );
		dcMark.SelectObject( bmButtonEdge );

		const int nHeight = min( pInfo.bmHeight, rc->Height() ); 	// No repeat (+ 1 to allow 1px overdraw?)

		if ( pInfo.bmBitsPixel == 32 )
			pDC->AlphaBlend( rc->right - nEdge, rc->top, nEdge, nHeight, &dcMark, 0, 0, nEdge, nHeight, bf );
		else
			pDC->BitBlt( rc->right - nEdge, rc->top, nEdge, nHeight, &dcMark, 0, 0, SRCCOPY );
	}

	dcMark.SelectObject( pOld );
//	dcMark.DeleteDC();

	return TRUE;
}

BOOL CImages::DrawButtonMap(CDC* pDC, const CRect* rc, CBitmap* bmButtonMap, const int nState, const int nOption /*0*/)
{
	if ( ! bmButtonMap->m_hObject || pDC == NULL || rc == NULL )
		return FALSE;

	BITMAP pInfo;
	bmButtonMap->GetBitmap( &pInfo );

	const int nSourceHeight	= bmButtonMap->GetBitmapDimension().cy;
	const int nPosition 	= nSourceHeight * nState;
	const int nEdge			= bmButtonMap->GetBitmapDimension().cx;
	const int nSourceWidth	= pInfo.bmWidth - nEdge;

	if ( pInfo.bmBitsPixel == 32 && nState == STATE_DEFAULT )	// Test for empty inactive button
	{
		// ToDo: Faster/smarter way to check if available button state should be drawn now?
		BOOL bEmpty = TRUE;
		const int bufferSize = pInfo.bmWidthBytes * pInfo.bmHeight;
		BYTE* buffer = (BYTE*)malloc(bufferSize);
		GetBitmapBits( (HBITMAP)bmButtonMap->m_hObject, bufferSize, buffer );	// Deprecated function but useful

		for ( int i = 3 ; i < ( pInfo.bmWidthBytes * ( nSourceHeight - 1 ) ) ; i += 4 )
		{
			if ( buffer[i] > 1 )	// Non-zero alpha
			{
				bEmpty = FALSE;
				break;
			}
		}

		delete buffer;
		if ( bEmpty ) return FALSE;
	}

	CDC dcMark;
	dcMark.CreateCompatibleDC( pDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dcMark.m_hDC, nOption == OPTION_RTL ? LAYOUT_RTL : LAYOUT_BITMAPORIENTATIONPRESERVED );

	CBitmap* pOld;
	pOld = (CBitmap*)dcMark.SelectObject( bmButtonMap );

	BLENDFUNCTION bf;				// Set now if needed
	bf.BlendOp = AC_SRC_OVER;		// Must be defined
	bf.BlendFlags = 0;				// Must be zero
	bf.SourceConstantAlpha = 0xFF;	// Opaque (Bitmap alpha only)
	bf.AlphaFormat = AC_SRC_ALPHA;	// Use bitmap alpha

	if ( nOption == OPTION_NOREPEAT )	// Centered (Icon)
	{
		int nWidth  = rc->Width();	// min( pInfo.bmWidth, rc->Width() );
		int nHeight = rc->Height();	//min( nSourceHeight, rc->Height() );
		int nLeft = rc->left;
		int nTop  = rc->top;
		if ( pInfo.bmWidth < nWidth )
		{
			nLeft += ( nWidth - pInfo.bmWidth ) / 2;
			nWidth = pInfo.bmWidth;
		}
		if ( nSourceHeight < nHeight )
		{
			nTop  += ( nHeight - nSourceHeight ) / 2;
			nHeight = nSourceHeight;
		}

		if ( pInfo.bmBitsPixel == 32 )		// (Pre-multiplied for AlphaBlend Transparency)
			pDC->AlphaBlend( nLeft, nTop, nWidth, nHeight, &dcMark, 0, nPosition, nWidth, nHeight, bf );
		else
			pDC->BitBlt( nLeft, nTop, nWidth, nHeight, &dcMark, 0, nPosition, SRCCOPY );
	}
	else	// Default
	{
		if ( rc->Width() > nEdge )
		{
			for ( int nY = rc->top ; nY < rc->bottom ; nY += nSourceHeight )
			{
				for ( int nX = rc->left ; nX < rc->right - nEdge ; nX += nSourceWidth )
				{
					const int nWidth  = min( nSourceWidth, rc->right - nX - nEdge );
					const int nHeight = min( nSourceHeight, rc->bottom - nY ); 	// No repeat (+ 1 to allow 1px overdraw?)

					if ( pInfo.bmBitsPixel == 32 )		// (Pre-multiplied for AlphaBlend Transparency)
						pDC->AlphaBlend( nX, nY, nWidth, nHeight, &dcMark, 0, nPosition, nWidth, nHeight, bf );
					else
						pDC->BitBlt( nX, nY, nWidth, nHeight, &dcMark, 0, nPosition, SRCCOPY );
				}
			}
		}

		if ( nEdge > 0 )
		{
			const int nHeight = min( nSourceHeight, rc->Height() ); 	// No repeat (+ 1 to allow 1px overdraw?)

			if ( pInfo.bmBitsPixel == 32 )
				pDC->AlphaBlend( rc->right - nEdge, rc->top, nEdge, nHeight, &dcMark, nSourceWidth, nPosition, nEdge, nHeight, bf );
			else
				pDC->BitBlt( rc->right - nEdge, rc->top, nEdge, nHeight, &dcMark, nSourceWidth, nPosition, SRCCOPY );
		}
	}

	dcMark.SelectObject( pOld );
	dcMark.DeleteDC();

	return TRUE;
}

BOOL CImages::DrawButtonState(CDC* pDC, const CRect* rc, const int nResource)
{
	if ( pDC == NULL || rc == NULL )
		return FALSE;

	// Abstracted pass-through for convenience/consistency elsewhere:

	switch ( nResource )
	{
	case IMAGE_BANNER:
		if ( m_nBanner < 2 ) return FALSE;
		return DrawButton( pDC, rc, &m_bmBanner, &m_bmBannerEdge );
	case IMAGE_DIALOG:
		return DrawButton( pDC, rc, &m_bmDialog );		// ToDo: m_bmDialogEdge?
	case IMAGE_DIALOGPANEL:
		return DrawButton( pDC, rc, &m_bmDialogPanel );	// ToDo: m_bmDialogPanelEdge?
	case IMAGE_PANELMARK:
		return DrawButton( pDC, rc, &m_bmPanelMark );
	case IMAGE_TOOLTIP:
		return DrawButton( pDC, rc, &m_bmToolTip );		// ToDo: &m_bmToolTipEdge?

	case IMAGE_SELECTED:	// + IMAGE_HIGHLIGHT
		return DrawButton( pDC, rc, &m_bmSelected ) ||	// ToDo: &m_bmSelectedEdge?
			DrawButtonMap( pDC, rc, &m_bmButtonMapSelect, STATE_DEFAULT, OPTION_RTL );
	case IMAGE_SELECTEDGREY:
		return DrawButton( pDC, rc, &m_bmSelectedGrey ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapSelect, STATE_DEFAULT+1, OPTION_RTL ) ||
			DrawButton( pDC, rc, &m_bmSelected );
	case IMAGE_MENUSELECTED:
		return DrawButton( pDC, rc, &m_bmMenuSelected, &m_bmMenuSelectedEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenuselect, STATE_HOVER, OPTION_RTL );
	case IMAGE_MENUDISABLED:
		return DrawButton( pDC, rc, &m_bmMenuDisabled, &m_bmMenuDisabledEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenuselect, STATE_PRESS, OPTION_RTL );
	case IMAGE_PROGRESSBAR:
		return DrawButton( pDC, rc, &m_bmProgress, &m_bmProgressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapProgressbar, STATE_HOVER );
	case IMAGE_PROGRESSBAR_NONE:
		return DrawButton( pDC, rc, &m_bmProgressNone ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapProgressbar, STATE_DEFAULT );
	case IMAGE_PROGRESSBAR_SHADED:
		return DrawButton( pDC, rc, &m_bmProgressShaded ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapProgressbar, STATE_PRESS );

	case ICONBUTTON_DEFAULT:
		return DrawIconButton( pDC, rc, &m_bmIconButton ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapIconbox, STATE_DEFAULT, OPTION_CENTERED );
	case ICONBUTTON_HOVER:
		return DrawIconButton( pDC, rc, &m_bmIconButtonHover ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapIconbox, STATE_HOVER, OPTION_CENTERED );
	case ICONBUTTON_PRESS:
		return DrawIconButton( pDC, rc, &m_bmIconButtonPress ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapIconbox, STATE_PRESS, OPTION_CENTERED );
	case ICONBUTTON_ACTIVE:
		return DrawIconButton( pDC, rc, &m_bmIconButtonActive ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapIconbox, STATE_ACTIVE, OPTION_CENTERED );
	case ICONBUTTON_DISABLED:
		return DrawIconButton( pDC, rc, &m_bmIconButtonDisabled ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapIconbox, STATE_DISABLED, OPTION_CENTERED );

	case RICHBUTTON_DEFAULT:
		return DrawButton( pDC, rc, &m_bmRichButton, &m_bmRichButtonEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapRichdoc, STATE_DEFAULT );
	case RICHBUTTON_HOVER:
		return DrawButton( pDC, rc, &m_bmRichButtonHover, &m_bmRichButtonHoverEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapRichdoc, STATE_HOVER );
	case RICHBUTTON_PRESS:
		return DrawButton( pDC, rc, &m_bmRichButtonPress, &m_bmRichButtonPressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapRichdoc, STATE_PRESS );
	case RICHBUTTON_ACTIVE:
		return DrawButton( pDC, rc, &m_bmRichButtonActive, &m_bmRichButtonActiveEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapRichdoc, STATE_ACTIVE );
	case RICHBUTTON_DISABLED:
		return DrawButton( pDC, rc, &m_bmRichButtonDisabled, &m_bmRichButtonDisabledEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapRichdoc, STATE_DISABLED );

	case TOOLBARBUTTON_DEFAULT:
		return DrawButton( pDC, rc, &m_bmToolbarButton, &m_bmToolbarButtonEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapToolbar, STATE_DEFAULT, OPTION_RTL );
	case TOOLBARBUTTON_HOVER:
		return DrawButton( pDC, rc, &m_bmToolbarButtonHover, &m_bmToolbarButtonHoverEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapToolbar, STATE_HOVER, OPTION_RTL );
	case TOOLBARBUTTON_PRESS:
		return DrawButton( pDC, rc, &m_bmToolbarButtonPress, &m_bmToolbarButtonPressEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapToolbar, STATE_PRESS, OPTION_RTL );
	case TOOLBARBUTTON_ACTIVE:
		return DrawButton( pDC, rc, &m_bmToolbarButtonActive, &m_bmToolbarButtonActiveEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapToolbar, STATE_ACTIVE, OPTION_RTL );
	case TOOLBARBUTTON_DISABLED:
		return DrawButton( pDC, rc, &m_bmToolbarButtonDisabled, &m_bmToolbarButtonDisabledEdge, TRUE ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapToolbar, STATE_DISABLED, OPTION_RTL );
	case TOOLBAR_SEPARATOR:
		return DrawButton( pDC, rc, &m_bmToolbarSeparator );

	case MENUBARITEM_DEFAULT:
		return DrawButton( pDC, rc, &m_bmMenubarItem, &m_bmMenubarItemEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenutext, STATE_DEFAULT );
	case MENUBARITEM_HOVER:
		return DrawButton( pDC, rc, &m_bmMenubarItemHover, &m_bmMenubarItemHoverEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenutext, STATE_HOVER );
	case MENUBARITEM_PRESS:
		return DrawButton( pDC, rc, &m_bmMenubarItemPress, &m_bmMenubarItemPressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenutext, STATE_PRESS );
	case MENUBARBUTTON_DEFAULT:
		return DrawButton( pDC, rc, &m_bmMenubarButton, &m_bmMenubarButtonEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenubar, STATE_DEFAULT );
	case MENUBARBUTTON_HOVER:
		return DrawButton( pDC, rc, &m_bmMenubarButtonHover, &m_bmMenubarButtonHoverEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenubar, STATE_HOVER );
	case MENUBARBUTTON_PRESS:
		return DrawButton( pDC, rc, &m_bmMenubarButtonPress, &m_bmMenubarButtonPressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenubar, STATE_PRESS );
	case MENUBARBUTTON_ACTIVE:
		return DrawButton( pDC, rc, &m_bmMenubarButtonActive, &m_bmMenubarButtonActiveEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapMenubar, STATE_ACTIVE );

	case TASKBARBUTTON_DEFAULT:
		return DrawButton( pDC, rc, &m_bmTaskbarButton, &m_bmTaskbarButtonEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapTaskbar, STATE_DEFAULT );
	case TASKBARBUTTON_HOVER:
		return DrawButton( pDC, rc, &m_bmTaskbarButtonHover, &m_bmTaskbarButtonHoverEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapTaskbar, STATE_HOVER );
	case TASKBARBUTTON_PRESS:
		return DrawButton( pDC, rc, &m_bmTaskbarButtonPress, &m_bmTaskbarButtonPressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapTaskbar, STATE_PRESS );
	case TASKBARBUTTON_ACTIVE:
		return DrawButton( pDC, rc, &m_bmTaskbarButtonActive, &m_bmTaskbarButtonActiveEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapTaskbar, STATE_ACTIVE );

	case DOWNLOADGROUP_DEFAULT:
		return DrawButton( pDC, rc, &m_bmDownloadGroup, &m_bmDownloadGroupEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapDownloadgroup, STATE_DEFAULT );
	case DOWNLOADGROUP_HOVER:
		return DrawButton( pDC, rc, &m_bmDownloadGroupHover, &m_bmDownloadGroupHoverEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapDownloadgroup, STATE_HOVER );
	case DOWNLOADGROUP_PRESS:
		return DrawButton( pDC, rc, &m_bmDownloadGroupPress, &m_bmDownloadGroupPressEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapDownloadgroup, STATE_PRESS );
	case DOWNLOADGROUP_ACTIVE:
		return DrawButton( pDC, rc, &m_bmDownloadGroupActive, &m_bmDownloadGroupActiveEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapDownloadgroup, STATE_ACTIVE );
	case DOWNLOADGROUP_DISABLED:
		return DrawButton( pDC, rc, &m_bmDownloadGroupDisabled, &m_bmDownloadGroupDisabledEdge ) ||
			DrawButtonMap( pDC, rc, &m_bmButtonMapDownloadgroup, STATE_DISABLED );
	}

	ASSERT( FALSE );
	return FALSE;
}
