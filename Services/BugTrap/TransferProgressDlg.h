/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Transfer Progress dialog.
 * Author: Maksim Pyatkovskiy.
 */

#pragma once

#define UM_CONNECTINGTOSERVER         (WM_APP + 1)
#define UM_SENDINGREPORT              (WM_APP + 2)
#define UM_CHECKINGERRORSTATUS        (WM_APP + 3)
#define UM_TRANSFERCOMPLETE           (WM_APP + 4)

INT_PTR CALLBACK TransferProgressDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
