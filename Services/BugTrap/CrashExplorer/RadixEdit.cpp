/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Radix aware editor.
 * Author: Maksim Pyatkovskiy
 */

#include "StdAfx.h"
#include "Resource.h"
#include "RadixEdit.h"

void CRadixEdit::Attach(HWND hwndEdit, HWND hwndRadixMenu)
{
	m_txtNumEdit.SetRadixMenu(hwndRadixMenu);
	m_btnRadixMenu.SetEditWindow(hwndEdit);
	m_txtNumEdit.SubclassWindow(hwndEdit);
	m_btnRadixMenu.SubclassWindow(hwndRadixMenu);
}
