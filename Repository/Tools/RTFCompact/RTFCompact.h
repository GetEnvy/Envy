//
// RTFCompact.h : Main header file for the RTFCompact application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"		// Main symbols


// CRTFCompactApp:
// See RTFCompact.cpp for the implementation of this class
//

class CRTFCompactApp : public CWinApp
{
public:
	CRTFCompactApp();

// Overrides
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CRTFCompactApp theApp;
