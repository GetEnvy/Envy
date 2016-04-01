//
// AutocompleteEdit.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2012 and Shareaza 2008
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CRegEnum : public CComObject
{
public:
	CRegEnum();
	virtual ~CRegEnum();

public:
	// Create IAutoComplete object and attach HWND to it
	//	szSection	: HKCU\SOFTWARE\{CompanyKey}\{ApplicationKey}\{szSection}
	//	szRoot		: Must contain inside a "%i" format specifier (n = 1,2..200)
	BOOL AttachTo(HWND hWnd, LPCTSTR szSection, LPCTSTR szRoot);

	// Add new string to autocomplete list
	void AddString(const CString& rString) const;

protected:
	CString						m_sect;
	CString						m_root;
	int							m_iter;
	CComPtr< IAutoComplete >	m_pIAutoComplete;

protected:
	BEGIN_INTERFACE_PART(EnumString, IEnumString)
		STDMETHOD(Next)(
			/*[in]*/ ULONG celt,
			/*[length_is][size_is][out]*/ LPOLESTR* rgelt,
			/*[out]*/ ULONG *pceltFetched);
		STDMETHOD(Skip)(
			/*[in]*/ ULONG celt);
		STDMETHOD(Reset)(void);
		STDMETHOD(Clone)(
			/*[out]*/ IEnumString** ppenum);
	END_INTERFACE_PART(EnumString)

	DECLARE_DYNCREATE(CRegEnum)
	DECLARE_OLECREATE(CRegEnum)
	DECLARE_INTERFACE_MAP()
};

class CAutocompleteEdit : public CEdit
{
	DECLARE_DYNCREATE(CAutocompleteEdit)

public:
	CAutocompleteEdit();
	virtual ~CAutocompleteEdit();

public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPCTSTR szSection, LPCTSTR szRoot);

	virtual int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
	virtual void GetWindowText(CString& rString) const;

protected:
	CRegEnum*	m_pData;

	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
