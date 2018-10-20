//
// KeywordTest.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2011 and PeerProject 2012
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

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "Resource.h"

class CKeywordTestApp : public CWinApp
{
public:
	CKeywordTestApp();

protected:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CKeywordTestApp theApp;
