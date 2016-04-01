//
// Image.h
//
// This file is part of Envy (getenvy.com) © 2008
// Original author Michael Stokes released portions into the public domain.
// You are free to redistribute and modify this page without any restrictions.
//

#pragma once

class CImage
{
// Construction
public:
	CImage();
	virtual ~CImage();

// Attributes
public:
	BYTE*	m_pImage;		// Pointer to image data
	int		m_nWidth;
	int		m_nHeight;
	int		m_nComponents;	// (1=mono, 3=RGB, 4=RGBA)
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
