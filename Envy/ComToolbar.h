//
// ComToolbar.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

class CCoolBarCtrl;
class CCoolBarItem;


class CComToolbar : public CComObject
{
public:
	CComToolbar(CCoolBarCtrl* pBar, CCoolBarItem* pItem);
	virtual ~CComToolbar();

public:
	CCoolBarCtrl* m_pBar;
	CCoolBarItem* m_pItem;

public:
	static ISToolbar*		Wrap(CCoolBarCtrl* pBar);
	static ISToolbarItem*	Wrap(CCoolBarCtrl* pBar, CCoolBarItem* pItem);

// ISToolbar
protected:
	BEGIN_INTERFACE_PART(SToolbar, ISToolbar)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISToolbarItem FAR* FAR* ppItem);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(InsertSeparator)(LONG nPosition);
		STDMETHOD(InsertButton)(LONG nPosition, LONG nCommandID, BSTR sText, ISToolbarItem FAR* FAR* ppItem);
	END_INTERFACE_PART(SToolbar)

	BEGIN_INTERFACE_PART(SToolbarItem, ISToolbarItem)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get_Toolbar)(ISToolbar FAR* FAR* ppToolbar);
		STDMETHOD(get_ItemType)(SToolbarType FAR* pnType);
		STDMETHOD(get_CommandID)(LONG FAR* pnCommandID);
		STDMETHOD(put_CommandID)(LONG nCommandID);
		STDMETHOD(get_Text)(BSTR FAR* psText);
		STDMETHOD(put_Text)(BSTR sText);
		STDMETHOD(Remove)();
	END_INTERFACE_PART(SToolbarItem)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		UINT	m_nIndex;
	END_INTERFACE_PART(EnumVARIANT)

	DECLARE_INTERFACE_MAP()

	//DECLARE_MESSAGE_MAP()
};
