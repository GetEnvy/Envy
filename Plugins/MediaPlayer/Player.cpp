//
// Player.cpp : Implementation of CPlayer
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2009 and PeerProject 2008-2012
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
#include "Player.h"

#define SAFE_RELEASE(comPtr)  \
			if ((comPtr)) { (comPtr).Release(); }

// CPlayer

CPlayer::CPlayer()
	: m_hwndOwner	( NULL )
	, m_rcWindow	()
	, m_dAspect 	( 0.0 )
	, m_nZoom		( smaDefault )
	, m_bAudioOnly	( FALSE )
{
}

HRESULT CPlayer::FinalConstruct()
{
	return S_OK;
}

void CPlayer::FinalRelease()
{
	Destroy();
}

STDMETHODIMP CPlayer::Create(/*[in]*/ HWND hWnd)
{
	if ( ! hWnd )
		return E_INVALIDARG;

	m_hwndOwner = hWnd;

	if ( m_pGraph )
		return E_INVALIDARG;

	HRESULT hr = m_pGraph.CoCreateInstance( CLSID_FilterGraph );
	if ( SUCCEEDED( hr ) )
	{
		m_pControl = m_pGraph;
		if ( ! m_pControl )
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pEvent = m_pGraph;
		if ( ! m_pEvent )
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pVideo = m_pGraph;
		if ( ! m_pVideo )
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pWindow = m_pGraph;
		if ( ! m_pWindow )
			hr = E_NOINTERFACE;
	}

	if ( FAILED( hr ) )
	{
		SAFE_RELEASE(m_pControl);
		SAFE_RELEASE(m_pEvent);
		SAFE_RELEASE(m_pVideo);
		SAFE_RELEASE(m_pWindow);
		SAFE_RELEASE(m_pGraph);
	}

	return hr;
}

STDMETHODIMP CPlayer::Destroy(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	Close();

	m_pWindow->put_Owner( NULL );

	m_pWindow.Release();
	m_pVideo.Release();
	m_pEvent.Release();
	m_pControl.Release();
	m_pGraph.Release();

	return S_OK;
}

STDMETHODIMP CPlayer::Reposition(/*[in]*/ RECT *prcWnd)
{
	if ( ! prcWnd )
		return E_POINTER;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	if ( m_bAudioOnly )
		return S_OK;

	m_rcWindow = *prcWnd;

	HRESULT hr = m_pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
		m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );

	if ( FAILED( hr ) )
		return hr;

	return AdjustVideoZoom();
}

STDMETHODIMP CPlayer::SetLogoBitmap(/*[in]*/ HBITMAP /*hLogo*/)
{
	return E_NOTIMPL;
}

// ToDo: Better scale conversion?
// -10,000 to 0 decibal scale, silent below ~6000
#define AUDIO_FLOOR 6200.

STDMETHODIMP CPlayer::GetVolume(/*[out]*/ DOUBLE *pnVolume)
{
	if ( ! pnVolume )
		return E_POINTER;

	*pnVolume = 1.;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	// ToDo: Use IAudioClient under Vista+

	CComQIPtr< IBasicAudio > pAudio( m_pGraph );
	if ( ! pAudio )
		return E_NOINTERFACE;

	long lVolume = 0;
	HRESULT hr = pAudio->get_Volume( &lVolume );
	if ( FAILED( hr ) )
		return hr;

	// Convert -10,000-0 to 0.0-1.0
	*pnVolume =
		( lVolume > -10. ) ? 1 :
		( lVolume <= -AUDIO_FLOOR ) ? 0 :
		( lVolume + AUDIO_FLOOR ) / AUDIO_FLOOR;

	return S_OK;
}

STDMETHODIMP CPlayer::SetVolume(/*[in]*/ DOUBLE nVolume)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	// ToDo: Use IAudioClient under Windows Vista+

	CComQIPtr< IBasicAudio > pAudio( m_pGraph );
	if ( ! pAudio )
		return E_NOINTERFACE;

	// Convert 0.0-1.0 to -10,000-0
	long lVolume = (long)(
		( nVolume > 0.99 ) ? 0 :
		( nVolume < 0.01 ) ? -AUDIO_FLOOR :
		( nVolume * AUDIO_FLOOR - AUDIO_FLOOR ) );

	return pAudio->put_Volume( lVolume );
}

STDMETHODIMP CPlayer::GetZoom(/*[out]*/ MediaZoom *pnZoom)
{
	if ( ! pnZoom )
		return E_POINTER;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	*pnZoom = m_nZoom;

	return S_OK;
}

STDMETHODIMP CPlayer::SetZoom(/*[in]*/ MediaZoom nZoom)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_nZoom = nZoom;

	if ( m_bAudioOnly )
		return S_OK;

	return AdjustVideoZoom();
}

STDMETHODIMP CPlayer::GetAspect(/*[out]*/ DOUBLE *pdAspect)
{
	if ( ! pdAspect )
		return E_POINTER;

	*pdAspect = m_dAspect;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CPlayer::SetAspect(/*[in]*/ DOUBLE dAspect)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_dAspect = dAspect;

	if ( m_bAudioOnly )
		return S_OK;

	return AdjustVideoZoom();
}

//HRESULT FindPin(IBaseFilter* pFilter, int count, PIN_DIRECTION dir, IPin** ppPin)
//{
//	*ppPin = NULL;
//
//	CComPtr< IEnumPins > pPins;
//	HRESULT hr = pFilter->EnumPins( &pPins );
//	if ( FAILED( hr ) ) return hr;
//
//	pPins->Reset();
//
//	for (;;)
//	{
//		CComPtr< IPin > pPin;
//		hr = pPins->Next( 1, &pPin, NULL );
//		if ( hr != S_OK ) break;
//
//		PIN_INFO PinInfo = {};
//		hr = pPin->QueryPinInfo( &PinInfo );
//		if ( FAILED( hr ) ) break;
//
//		if ( PinInfo.dir == dir )
//		{
//			if ( count-- == 0 )
//			{
//				*ppPin = pPin.Detach();
//				return S_OK;
//			}
//		}
//	}
//
//	return E_FAIL;
//}

HRESULT SafeRenderFile(IGraphBuilder* pGraph, BSTR sFilename) throw()
{
	__try
	{
		return pGraph->RenderFile( sFilename, NULL );
	}
	_except( EXCEPTION_EXECUTE_HANDLER )
	{
		return E_FAIL;
	}
}

STDMETHODIMP CPlayer::Open(/*[in]*/ BSTR sFilename)
{
	HRESULT hr;
	long lVisible;

	if ( ! sFilename )
		return E_POINTER;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	hr = SafeRenderFile( m_pGraph, sFilename );
	if ( FAILED( hr ) )
		return hr;

	if ( ! m_pVideo || ! m_pWindow )
	{
		m_bAudioOnly = TRUE;
	}
	else
	{
		hr = m_pWindow->get_Visible(&lVisible);
		if ( hr == E_NOINTERFACE )
		{
			m_bAudioOnly = TRUE;
		}
		else
		{
			m_bAudioOnly = FALSE;
			hr = m_pWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE );
			m_pWindow->put_Owner( (OAHWND)m_hwndOwner );
		}
	}

	return Play();
}

STDMETHODIMP CPlayer::Close(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	Stop();

	return S_OK;
}

STDMETHODIMP CPlayer::Play(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	if ( m_bAudioOnly )
	{
		HRESULT hr = m_pControl->Run();
		if ( FAILED( hr ) )
			return hr;

		return S_OK;
	}

	m_pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
		m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );

	HRESULT hr = m_pControl->Run();
	if ( FAILED( hr ) )
		return hr;

	// Handle pending zoom and aspect changes
	return AdjustVideoZoom();
}

STDMETHODIMP CPlayer::Pause(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	return m_pControl->Pause();
}

STDMETHODIMP CPlayer::Stop(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	if ( ! m_bAudioOnly )
		m_pWindow->put_Visible( OAFALSE );

	return m_pControl->Stop();
}

STDMETHODIMP CPlayer::GetState(/*[out]*/ MediaState *pnState)
{
	if ( ! pnState )
		return E_POINTER;

	*pnState = smsNull;

	if ( ! m_pGraph )
		return S_OK;

	OAFilterState st = State_Stopped;
	if ( SUCCEEDED( m_pControl->GetState( 500, &st ) ) )
	{
		switch ( st )
		{
		case State_Stopped:
			*pnState = smsOpen;
			break;
		case State_Paused:
			*pnState = smsPaused;
			break;
		case State_Running:
			*pnState = smsPlaying;
			break;
		}
	}

	return S_OK;
}

STDMETHODIMP CPlayer::GetLength(/*[out]*/ LONGLONG *pnLength)
{
	if ( ! pnLength )
		return E_POINTER;

	*pnLength = 0;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetDuration( pnLength );
}

STDMETHODIMP CPlayer::GetPosition(/*[out]*/ LONGLONG *pnPosition)
{
	if ( ! pnPosition )
		return E_POINTER;

	*pnPosition = 0;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetCurrentPosition( pnPosition );
}

STDMETHODIMP CPlayer::SetPosition(/*[in]*/ LONGLONG nPosition)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->SetPositions( &nPosition, AM_SEEKING_AbsolutePositioning,
		NULL, AM_SEEKING_NoPositioning );
}

STDMETHODIMP CPlayer::GetSpeed(/*[out]*/ DOUBLE *pnSpeed)
{
	if ( ! pnSpeed )
		return E_POINTER;

	*pnSpeed = 1.;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetRate( pnSpeed );
}

STDMETHODIMP CPlayer::SetSpeed(/*[in]*/ DOUBLE nSpeed)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->SetRate( nSpeed );
}

STDMETHODIMP CPlayer::GetPlugin(/*[out]*/ IAudioVisPlugin **ppPlugin)
{
	if ( ! ppPlugin )
		return E_POINTER;

	*ppPlugin = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetPlugin(/*[in]*/ IAudioVisPlugin *pPlugin)
{
	if ( ! pPlugin )
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::GetPluginSize(/*[out]*/ LONG *pnSize)
{
	if ( ! pnSize )
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetPluginSize(/*[in]*/ LONG /*nSize*/)
{
	return E_NOTIMPL;
}

// Adjusts video position and zoom according to aspect ratio, zoom level and zoom type
HRESULT CPlayer::AdjustVideoZoom(void)
{
	long VideoWidth, VideoHeight;
	long WindowWidth, WindowHeight;

	HRESULT hr = m_pVideo->GetVideoSize( &VideoWidth, &VideoHeight );
	if ( FAILED( hr ) )
		return hr;

	WindowWidth = this->m_rcWindow.right - this->m_rcWindow.left;
	WindowHeight = this->m_rcWindow.bottom - this->m_rcWindow.top;

	if ( m_dAspect > 1 )
		VideoHeight = (long)(VideoWidth / m_dAspect);

	if ( m_nZoom == smzHalf )
	{
		VideoWidth >>= 1;
		VideoHeight >>= 1;
	}
	else if ( m_nZoom == smzDouble )
	{
		VideoWidth <<= 1;
		VideoHeight <<= 1;
	}
	else if ( m_nZoom == smzFill )
	{
		double VideoAspect = m_dAspect > 1 ? m_dAspect : ( (double)VideoWidth / VideoHeight );

		VideoWidth = WindowWidth;
		VideoHeight = (long)(WindowWidth / VideoAspect);

		if ( VideoHeight > WindowHeight )
		{
			VideoHeight = WindowHeight;
			VideoWidth = (long)(WindowHeight * VideoAspect);
		}
	}
	else if ( m_nZoom > 0 && m_nZoom != 1 )
	{
		VideoWidth *= m_nZoom;
		VideoHeight *= m_nZoom;
	}

	CRect tr;

	if ( m_nZoom == smzDistort )
	{
		tr.left = 0;
		tr.top = 0;
		tr.right = WindowWidth;
		tr.bottom = WindowHeight;
	}
	else
	{
		tr.top  = this->m_rcWindow.top + (WindowHeight >> 1) - (VideoHeight >> 1);
		tr.left = this->m_rcWindow.left + (WindowWidth >> 1) - (VideoWidth >> 1);
		tr.bottom = tr.top + VideoHeight;
		tr.right = tr.left + VideoWidth;
	}

	hr = m_pVideo->SetDestinationPosition( tr.left, tr.top, tr.Width(), tr.Height() );
	if ( FAILED( hr ) )
		return hr;

	return S_OK;
}
