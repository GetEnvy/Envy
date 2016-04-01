//
// Text.h
//
// This file is part of Envy (getenvy.com) © 2010
// TextViewer plugin is released under the Persistent Public Domain license.
//
// This code may be treated as Public Domain, provided:
// the work in all its forms and attendant uses shall remain available as
// persistently "Public Domain" until it naturally enters the public domain.
// History remains immutable:  Authors do not disclaim copyright, but do disclaim
// all rights beyond asserting the reach and duration and spirit of this license.

#pragma once

class CText
{
// Construction
public:
	CText();
	virtual ~CImage();

// Attributes
public:
	BYTE*	m_pImage;		// Pointer to image data
	int		m_nWidth;		// Width
	int		m_nHeight;		// Height
	int		m_nComponents;	// Components (1=mono, 3=RGB, 4=RGBA)
	BOOL	m_bPartial;		// Is it partially loaded?

// Operations
public:
	BOOL	Load(LPCTSTR pszFile);
	void	Clear();
	BOOL	EnsureRGB(COLORREF crFill = 0xFFFFFFFF);
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crFill);
	HBITMAP	Resample(int nWidth, int nHeight);

// Internal Helpers
protected:
	IImageServicePlugin*	LoadService(LPCTSTR pszFile);
};
