//
// Player.h : Declaration of the CPlayer
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2009 and Shareaza 2009
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "MediaPlayer_h.h"


// CPlayer

class ATL_NO_VTABLE CPlayer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlayer, &CLSID_MediaPlayer>,
	public IMediaPlayer
{
public:
	CPlayer();

DECLARE_REGISTRY_RESOURCEID(IDR_PLAYER)

BEGIN_COM_MAP(CPlayer)
	COM_INTERFACE_ENTRY(IMediaPlayer)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

protected:
	HRESULT AdjustVideoZoom(void);

	HWND						m_hwndOwner;
	RECT						m_rcWindow;
	CComPtr< IGraphBuilder >	m_pGraph;
	CComQIPtr< IMediaControl >	m_pControl;
	CComQIPtr< IMediaEvent >	m_pEvent;
	CComQIPtr< IBasicVideo >	m_pVideo;
	CComQIPtr< IVideoWindow >	m_pWindow;
	MediaZoom					m_nZoom;
	DOUBLE						m_dAspect;
	BOOLEAN						m_bAudioOnly;

// IMediaPlayer
public:
	STDMETHOD(Create)(
		/* [in] */ HWND hWnd);
	STDMETHOD(Destroy)(void);
	STDMETHOD(Reposition)(
		/* [in] */ RECT *prcWnd);
	STDMETHOD(SetLogoBitmap)(
		/* [in] */ HBITMAP hLogo);
	STDMETHOD(GetVolume)(
		/* [out] */ DOUBLE *pnVolume);
	STDMETHOD(SetVolume)(
		/* [in] */ DOUBLE nVolume);
	STDMETHOD(GetZoom)(
		/* [out] */ MediaZoom *pnZoom);
	STDMETHOD(SetZoom)(
		/* [in] */ MediaZoom nZoom);
	STDMETHOD(GetAspect)(
		/* [out] */ DOUBLE *pdAspect);
	STDMETHOD(SetAspect)(
		/* [in] */ DOUBLE dAspect) ;
	STDMETHOD(Open)(
		/* [in] */ BSTR sFilename);
	STDMETHOD(Close)(void);
	STDMETHOD(Play)(void);
	STDMETHOD(Pause)(void);
	STDMETHOD(Stop)(void);
	STDMETHOD(GetState)(
		/* [out] */ MediaState *pnState);
	STDMETHOD(GetLength)(
		/* [out] */ LONGLONG *pnLength);
	STDMETHOD(GetPosition)(
		/* [out] */ LONGLONG *pnPosition);
	STDMETHOD(SetPosition)(
		/* [in] */ LONGLONG nPosition);
	STDMETHOD(GetSpeed)(
		/* [out] */ DOUBLE *pnSpeed);
	STDMETHOD(SetSpeed)(
		/* [in] */ DOUBLE nSpeed);
	STDMETHOD(GetPlugin)(
		/* [out] */ IAudioVisPlugin **ppPlugin);
	STDMETHOD(SetPlugin)(
		/* [in] */ IAudioVisPlugin *pPlugin);
	STDMETHOD(GetPluginSize)(
		/* [out] */ LONG *pnSize);
	STDMETHOD(SetPluginSize)(
		/* [in] */ LONG nSize);
};

OBJECT_ENTRY_AUTO(__uuidof(MediaPlayer), CPlayer)
