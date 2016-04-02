//
// Images.h
//
// This file is part of Envy (getenvy.com) © 2010-2015
// All work here is original and released as-is under Persistent Public Domain [PPD]
//

#pragma once

#include <shlobj.h>


class CImages
{
//public:
//	CImages();
//	~CImages();

public:
	int 		m_nBanner;	// Height
	CBitmap		m_bmBanner;
	CBitmap		m_bmBannerEdge;

	CBrush		m_brDialog;
	CBrush		m_brDialogPanel;
	CBrush		m_brMediaControl;	// m_brMediaSlider

	CBitmap		m_bmDialog;
	CBitmap		m_bmDialogPanel;
	CBitmap		m_bmPanelMark;
	CBitmap		m_bmToolTip;
	CBitmap		m_bmMediaStatusBar;
//	CBitmap		m_bmSystemWindow;
//	CBitmap		m_bmDialogList;

	CBitmap		m_bmSelected;
	CBitmap		m_bmSelectedGrey;
	CBitmap		m_bmMenuSelected;
	CBitmap		m_bmMenuSelectedEdge;
	CBitmap		m_bmMenuDisabled;
	CBitmap		m_bmMenuDisabledEdge;
	CBitmap		m_bmProgress;
	CBitmap		m_bmProgressEdge;
	CBitmap		m_bmProgressNone;
	CBitmap		m_bmProgressShaded;

	CBitmap		m_bmIconButton;
	CBitmap		m_bmIconButtonHover;
	CBitmap		m_bmIconButtonPress;
	CBitmap		m_bmIconButtonActive;
	CBitmap		m_bmIconButtonDisabled;

	CBitmap		m_bmRichButton;
	CBitmap		m_bmRichButtonEdge;
	CBitmap		m_bmRichButtonHover;
	CBitmap		m_bmRichButtonHoverEdge;
	CBitmap		m_bmRichButtonPress;
	CBitmap		m_bmRichButtonPressEdge;
	CBitmap		m_bmRichButtonActive;
	CBitmap		m_bmRichButtonActiveEdge;
	CBitmap		m_bmRichButtonDisabled;
	CBitmap		m_bmRichButtonDisabledEdge;

	CBitmap		m_bmToolbar;
	CBitmap		m_bmToolbarButton;
	CBitmap		m_bmToolbarButtonEdge;
	CBitmap		m_bmToolbarButtonHover;
	CBitmap		m_bmToolbarButtonHoverEdge;
	CBitmap		m_bmToolbarButtonPress;
	CBitmap		m_bmToolbarButtonPressEdge;
	CBitmap		m_bmToolbarButtonActive;
	CBitmap		m_bmToolbarButtonActiveEdge;
	CBitmap		m_bmToolbarButtonDisabled;
	CBitmap		m_bmToolbarButtonDisabledEdge;
	CBitmap		m_bmToolbarSeparator;

	CBitmap		m_bmMenubarItem;
	CBitmap		m_bmMenubarItemEdge;
	CBitmap		m_bmMenubarItemHover;
	CBitmap		m_bmMenubarItemHoverEdge;
	CBitmap		m_bmMenubarItemPress;
	CBitmap		m_bmMenubarItemPressEdge;
	CBitmap		m_bmMenubarButton;
	CBitmap		m_bmMenubarButtonEdge;
	CBitmap		m_bmMenubarButtonHover;
	CBitmap		m_bmMenubarButtonHoverEdge;
	CBitmap		m_bmMenubarButtonPress;
	CBitmap		m_bmMenubarButtonPressEdge;
	CBitmap		m_bmMenubarButtonActive;
	CBitmap		m_bmMenubarButtonActiveEdge;

	CBitmap		m_bmTaskbarButton;
	CBitmap		m_bmTaskbarButtonEdge;
	CBitmap		m_bmTaskbarButtonActive;
	CBitmap		m_bmTaskbarButtonActiveEdge;
	CBitmap		m_bmTaskbarButtonHover;
	CBitmap		m_bmTaskbarButtonHoverEdge;
	CBitmap		m_bmTaskbarButtonPress;
	CBitmap		m_bmTaskbarButtonPressEdge;

	CBitmap		m_bmDownloadGroup;
	CBitmap		m_bmDownloadGroupEdge;
	CBitmap		m_bmDownloadGroupActive;
	CBitmap		m_bmDownloadGroupActiveEdge;
	CBitmap		m_bmDownloadGroupHover;
	CBitmap		m_bmDownloadGroupHoverEdge;
	CBitmap		m_bmDownloadGroupPress;
	CBitmap		m_bmDownloadGroupPressEdge;
	CBitmap		m_bmDownloadGroupDisabled;
	CBitmap		m_bmDownloadGroupDisabledEdge;

	CBitmap		m_bmButtonMapIconbox;
	CBitmap		m_bmButtonMapRichdoc;
	CBitmap		m_bmButtonMapToolbar;
	CBitmap		m_bmButtonMapMenubar;
	CBitmap		m_bmButtonMapMenutext;
	CBitmap		m_bmButtonMapMenuselect;
	CBitmap		m_bmButtonMapSelect;
	CBitmap		m_bmButtonMapProgressbar;
	CBitmap		m_bmButtonMapDownloadgroup;
	CBitmap		m_bmButtonMapTaskbar;

	void		Load();
	void		DeleteObjects();
	void		BlendAlpha(CBitmap* bmImage, COLORREF crBlend = RGB(255,255,255));
	BOOL		PreBlend(HBITMAP hButton);
	BOOL		SkinImage(CBitmap* bmImage, LPCTSTR pszName, BOOL bAllowAlpha = TRUE, UINT nStates = 1);
	BOOL		DrawButtonState(CDC* pDC, const CRect* rc, const int nResource);
	BOOL		DrawButtonMap(CDC* pDC, const CRect* rc, CBitmap* bmButton, const int nState = 0, const int nOption = 0);
	BOOL		DrawButton(CDC* pDC, const CRect* rc, CBitmap* bmButton, CBitmap* bmButtonEdge = NULL, BOOL bRTL = FALSE);
	BOOL		DrawIconButton(CDC* pDC, const CRect* rc, CBitmap* bmButton);
	BOOL		DrawImage(CDC* pDC, const CRect* prc, CBitmap* bmImage, BOOL bRepeat = TRUE);
};

extern CImages Images;


// Map Order:
#define STATE_DEFAULT			0
#define STATE_HOVER				1
#define STATE_PRESS				2
#define STATE_ACTIVE			3
#define STATE_DISABLED			4
#define STATE_COUNT 			5

// Map Options:
#define OPTION_NONE				0
#define OPTION_NOREPEAT			1
#define OPTION_CENTERED			1
#define OPTION_RTL				2

// External References
#define IMAGE_BANNER			0
#define IMAGE_DIALOG			1
#define IMAGE_DIALOGPANEL		2
#define IMAGE_PANELMARK			3
#define IMAGE_TOOLTIP			4

#define IMAGE_HIGHLIGHT			10
#define IMAGE_SELECTED			10
#define IMAGE_SELECTEDGREY		11
#define IMAGE_MENUSELECTED		12
#define IMAGE_MENUDISABLED		14
#define IMAGE_PROGRESSBAR		16
#define IMAGE_PROGRESSBAR_NONE	17
#define IMAGE_PROGRESSBAR_SHADED 18

#define ICONBUTTON_DEFAULT		20
#define ICONBUTTON_HOVER		21
#define ICONBUTTON_PRESS		22
#define ICONBUTTON_ACTIVE		23
#define ICONBUTTON_DISABLED 	24

#define RICHBUTTON_DEFAULT		25
#define RICHBUTTON_HOVER		26
#define RICHBUTTON_PRESS		27
#define RICHBUTTON_ACTIVE		28
#define RICHBUTTON_DISABLED		29

#define TOOLBARBUTTON_DEFAULT	30
#define TOOLBARBUTTON_HOVER		31
#define TOOLBARBUTTON_PRESS		32
#define TOOLBARBUTTON_ACTIVE	33
#define TOOLBARBUTTON_DISABLED	34
#define TOOLBAR_SEPARATOR		35

#define MENUBARITEM_DEFAULT 	40
#define MENUBARITEM_HOVER		41
#define MENUBARITEM_PRESS		42
#define MENUBARBUTTON_DEFAULT	43
#define MENUBARBUTTON_HOVER		44
#define MENUBARBUTTON_PRESS		45
#define MENUBARBUTTON_ACTIVE	46

#define TASKBARBUTTON_DEFAULT	50
#define TASKBARBUTTON_ACTIVE	51
#define TASKBARBUTTON_HOVER		52
#define TASKBARBUTTON_PRESS		53

#define DOWNLOADGROUP_DEFAULT	54
#define DOWNLOADGROUP_ACTIVE	55
#define DOWNLOADGROUP_HOVER 	56
#define DOWNLOADGROUP_PRESS		57
#define DOWNLOADGROUP_DISABLED	58

#define IMAGE_LAST				60
