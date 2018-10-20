//
// WndPlugin.h
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

#pragma once

#include "WndPanel.h"


class CPluginWnd : public CPanelWnd
{
	DECLARE_DYNCREATE(CPluginWnd)

public:
	CPluginWnd(LPCTSTR pszName = L"", IPluginWindowOwner* pOwner = NULL);
	virtual ~CPluginWnd();

public:
	CComPtr< IPluginWindowOwner > m_pOwner;

protected:
	CString			m_sName;
	UINT*			m_pHandled;
	DWORD			m_nHandled;
	CCoolBarCtrl*	m_pToolbar;
	int				m_nToolbar;
	BOOL			m_bAccel;

	virtual void	OnSkinChange();
	virtual HRESULT	GetGenericView(IGenericView** ppView);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

// IPluginWindow
	BEGIN_INTERFACE_PART(PluginWindow, IPluginWindow)
		STDMETHOD(ListenForSingleMessage)(UINT nMessage);
		STDMETHOD(ListenForMultipleMessages)(SAFEARRAY FAR* pMessages);
		STDMETHOD(Create1)(BSTR bsCaption, HICON hIcon, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed);
		STDMETHOD(Create2)(UINT nCommandID, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed);
		STDMETHOD(GetHwnd)(HWND FAR* phWnd);
		STDMETHOD(HandleMessage)(LRESULT* plResult);
		STDMETHOD(LoadState)(VARIANT_BOOL bMaximise);
		STDMETHOD(SaveState)();
		STDMETHOD(ThrowMenu)(BSTR sName, LONG nDefaultID, POINT FAR* pPoint);
		STDMETHOD(AddToolbar)(BSTR sName, LONG nPosition, HWND FAR* phWnd, ISToolbar FAR* FAR* ppToolbar);
		STDMETHOD(AdjustWindowRect)(RECT FAR* pRect, VARIANT_BOOL bClientToWindow);
	END_INTERFACE_PART(PluginWindow)

	DECLARE_INTERFACE_MAP()

	DECLARE_MESSAGE_MAP()
};
