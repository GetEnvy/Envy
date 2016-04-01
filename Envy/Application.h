//
// Application.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2008
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


class CApplication : public CComObject
{
	DECLARE_DYNAMIC(CApplication)

public:
	CApplication();
	virtual ~CApplication();

public:
	static HRESULT GetApp(IApplication** ppIApplication) throw();
	static HRESULT GetUI(IUserInterface** ppIUserInterface) throw();
	static HRESULT GetSettings(ISettings** ppISettings) throw();

// IApplication
protected:
	BEGIN_INTERFACE_PART(Application, IApplication)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Version)(BSTR FAR* psVersion);
		STDMETHOD(CheckVersion)(BSTR sVersion);
		STDMETHOD(CreateXML)(ISXMLElement FAR* FAR* ppXML);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Settings)(ISettings FAR* FAR* ppSettings);
		STDMETHOD(get_ImageService)(IImageServicePlugin FAR* FAR* ppImageService);
		STDMETHOD(get_SmartAgent)(BSTR FAR* psSmartAgent);
		STDMETHOD(Message)(WORD nType, BSTR bsMessage);
	END_INTERFACE_PART(Application)

	BEGIN_INTERFACE_PART(UserInterface, IUserInterface)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(NewWindow)(BSTR bsName, IPluginWindowOwner FAR* pOwner, IPluginWindow FAR* FAR* ppWindow);
		STDMETHOD(get_MainWindowHwnd)(HWND FAR* phWnd);
		STDMETHOD(get_ActiveView)(IGenericView FAR* FAR* ppView);
		STDMETHOD(RegisterCommand)(BSTR bsName, HICON hIcon, UINT* pnCommandID);
		STDMETHOD(AddFromString)(BSTR sXML);
		STDMETHOD(AddFromResource)(HINSTANCE hInstance, UINT nID);
		STDMETHOD(AddFromXML)(ISXMLElement FAR* pXML);
		STDMETHOD(GetMenu)(BSTR bsName, VARIANT_BOOL bCreate, ISMenu FAR* FAR* ppMenu);
		STDMETHOD(GetToolbar)(BSTR bsName, VARIANT_BOOL bCreate, ISToolbar FAR* FAR* ppToolbar);
		STDMETHOD(NameToID)(BSTR bsName, UINT* pnCommandID);
		STDMETHOD(AddString)(UINT nStringID, BSTR sText);
		STDMETHOD(LoadString)(UINT nStringID, BSTR* psText);
	END_INTERFACE_PART(UserInterface)

	BEGIN_INTERFACE_PART(Settings, ISettings)
		DECLARE_DISPATCH()
		STDMETHOD(GetValue)(VARIANT* value);	// Pass as BSTR path (ex. Gnutella2.EnableAlways), get back the actual value
	END_INTERFACE_PART(Settings)

	DECLARE_OLECREATE(CApplication)
	DECLARE_INTERFACE_MAP()
//	DECLARE_MESSAGE_MAP()
};
