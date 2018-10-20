//
// DlgSplash.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2015
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


class CSplashDlg : public CDialog
{
	//DECLARE_DYNAMIC(CSplashDlg)

public:
	CSplashDlg(int nMax, bool bClosing);
	//virtual ~CSplashDlg();

	enum { IDD = IDD_SPLASH };

public:
	void		Step(LPCTSTR pszText);
	void		Update(LPCTSTR pszText = NULL);
	void		Hide(BOOL bAbort = FALSE);
	void		Topmost();

protected:
	int			m_nPos;
	int			m_nMax;
	int			m_nWidth;
	int			m_nHeight;
	bool		m_bClosing;
	CString		m_sState;
	static CBitmap m_bmSplash;

	void		DoPaint(CDC* pDC);
//	BOOL		(WINAPI *m_pfnAnimateWindow)(HWND, DWORD, DWORD);	// Legacy transition effects

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnPaint();
	afx_msg BOOL OnQueryEndSession();

	DECLARE_MESSAGE_MAP()
};
