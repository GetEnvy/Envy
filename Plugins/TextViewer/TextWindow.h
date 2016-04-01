//
// TextViewerWindow.h
//
// This file is part of Envy (getenvy.com) © 2010
// TextViewer plugin is released under the Persistent Public Domain license.
//
// This code may be treated as Public Domain, provided:
// the work in all its forms and attendant uses shall remain available as
// persistently "Public Domain" until it naturally enters the public domain.
// History remains immutable:  Authors do not disclaim copyright, but do disclaim
// all rights beyond asserting the reach and duration and spirit of this license.

#ifndef _TextViewerWINDOW_H_
#define _TextViewerWINDOW_H_

#include "Resource.h"
#include "Text.h"

class CTextViewerPlugin;


class CTextWindow :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CWindow, public IPluginWindowOwner
{
// Construction
public:
	CTextWindow();
	virtual ~CTextWindow();

// Attributes
public:
	CTextViewerPlugin*		m_pPlugin;
	CTextWindow*			m_pNext;
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IPluginWindow>	m_pWindow;
public:
	LPTSTR				m_pszFile;
	CImage				m_pImage;
	HBITMAP				m_hBitmap;
	HICON				m_hIcon;
	BOOL				m_bFullSize;
	BOOL				m_bDrag;
	POINTS				m_ptDrag;
	POINTS				m_ptOffset;
	float				m_nZoomFactor;
	int					m_nZoomIndex;
	int					m_nPaddingWidth;
	int					m_nPaddingHeight;

// Operations
public:
	BOOL	Create(CTextViewerPlugin* pPlugin, LPCTSTR pszFile);
	BOOL	Refresh();
protected:
	BOOL	ResizeWindow();
	BOOL	RescaleImage();
protected:
	void	OnBestFit();
	void	OnActualSize();

// Interfaces
public:
	BEGIN_COM_MAP(CTextWindow)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()

// IPluginWindowOwner
protected:
    virtual HRESULT STDMETHODCALLTYPE OnTranslate(MSG __RPC_FAR *pMessage);
	virtual HRESULT STDMETHODCALLTYPE OnMessage(UINT nMessage, WPARAM wParam, LPARAM lParam, LRESULT __RPC_FAR *plResult);
    virtual HRESULT STDMETHODCALLTYPE OnUpdate(UINT nCommandID, TRISTATE __RPC_FAR *pbVisible, TRISTATE __RPC_FAR *pbEnabled, TRISTATE __RPC_FAR *pbChecked);
	virtual HRESULT STDMETHODCALLTYPE OnCommand(UINT nCommandID);

// Message Map
public:
	BEGIN_MSG_MAP(CTextViewerWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

#endif
