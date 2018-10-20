//
// UPnPFinder.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
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

#include "UPnP.h"


class CUPnPFinder : public CUPnP
{
public:
	CUPnPFinder();
	virtual ~CUPnPFinder();

public:
	virtual void DeletePorts();
	virtual void StartDiscovery();
	virtual void StopAsyncFind();
	virtual bool IsAsyncFindRunning();

	typedef CComPtr< IUPnPDeviceFinder > FinderPointer;
	typedef CComPtr< IUPnPDevice > DevicePointer;
	typedef CComPtr< IUPnPService > ServicePointer;

	void StartDiscovery(bool bSecondTry);
	void AddDevice(DevicePointer pDevice, bool bAddChilds, int nLevel = 0);
	void RemoveDevice(CComBSTR bsUDN);
	bool OnSearchComplete();

	static CString translateUPnPResult(HRESULT hr);
	static HRESULT UPnPMessage(HRESULT hr);

private:
	bool Init();

	static FinderPointer CreateFinderInstance() throw();

	struct FindDevice : public std::unary_function< DevicePointer, bool >
	{
		FindDevice(const CComBSTR& udn) : m_udn( udn ) {}
		result_type operator()(argument_type device) const
		{
			CComBSTR deviceName;
			HRESULT hr = device->get_UniqueDeviceName( &deviceName );

			if ( FAILED( hr ) )
				return UPnPMessage( hr ), false;

			return wcscmp( deviceName.m_str, m_udn ) == 0;
		}
		CComBSTR m_udn;
	};

	void	ProcessAsyncFind(BSTR bsSearchType) throw();
	HRESULT	GetDeviceServices(DevicePointer pDevice);
	void	StartPortMapping();
	HRESULT	MapPort(const ServicePointer& service);
	void	DeleteExistingPortMappings(ServicePointer pService);
	void	CreatePortMappings(ServicePointer pService);
	HRESULT SaveServices(CComPtr< IEnumUnknown >, const LONG nTotalItems);

	HRESULT InvokeAction(ServicePointer pService, CComBSTR action, LPCTSTR pszInArgString, CString& strResult);

	// Utility functions
	HRESULT CreateSafeArray(const VARTYPE vt, const ULONG nArgs, SAFEARRAY** ppsa);
	INT_PTR CreateVarFromString(const CString& strArgs, VARIANT*** pppVars);
	INT_PTR	GetStringFromOutArgs(const VARIANT* pvaOutArgs, CString& strArgs);
	void	DestroyVars(const INT_PTR nCount, VARIANT*** pppVars);
	HRESULT GetSafeArrayBounds(SAFEARRAY* psa, LONG* pLBound, LONG* pUBound);
	HRESULT GetVariantElement(SAFEARRAY* psa, LONG pos, VARIANT* pvar);
	CString	GetLocalRoutableIP(ServicePointer pService);

// Public members
public:
	DWORD	m_tLastEvent;	// When the last event was received?

// Private members
private:
	std::vector< CAdapt< DevicePointer > >	m_pDevices;
	std::vector< CAdapt< ServicePointer> >	m_pServices;
	CComPtr< IUPnPServiceCallback > 		m_pServiceCallback;
	CComPtr< IUPnPDeviceFinderCallback >	m_pDeviceFinderCallback;
	FinderPointer							m_pDeviceFinder;

	LONG	m_nAsyncFindHandle;
	bool	m_bAsyncFindRunning;
	bool	m_bPortIsFree;
	CString m_sLocalIP;
	CString m_sExternalIP;
	bool	m_bADSL;			// Is the device ADSL?
	bool	m_bADSLFailed;		// Did port mapping failed for the ADSL device?
	bool	m_bInited;
	bool	m_bSecondTry;
	bool	m_bDisableWANIPSetup;
	bool	m_bDisableWANPPPSetup;
	ServicePointer m_pWANIPService;
};


// IUnknownImplementation defined in Augment

// DeviceFinder Callback
class CDeviceFinderCallback : public IUnknownImplementation< IUPnPDeviceFinderCallback >
{
public:
	CDeviceFinderCallback(CUPnPFinder& instance)
		: m_instance( instance )
	{}

private:
	CUPnPFinder& m_instance;

	HRESULT __stdcall DeviceAdded(LONG nFindData, IUPnPDevice* pDevice);
	HRESULT __stdcall DeviceRemoved(LONG nFindData, BSTR bsUDN);
	HRESULT __stdcall SearchComplete(LONG nFindData);
};


// Service Callback
class CServiceCallback : public IUnknownImplementation< IUPnPServiceCallback >
{
public:
	CServiceCallback(CUPnPFinder& instance)
		: m_instance( instance )
	{}

private:
	CUPnPFinder& m_instance;

	HRESULT __stdcall StateVariableChanged(IUPnPService* pService, LPCWSTR pszStateVarName, VARIANT varValue);
	HRESULT __stdcall ServiceInstanceDied(IUPnPService* pService);
};
