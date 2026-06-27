//
// UPnPManager.h
//
// Universal Plug and Play (UPnP) port forwarding support for ED2K
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#pragma once

class CUPnPManager
{
public:
    CUPnPManager();
    virtual ~CUPnPManager();

    // Port mapping operations
    bool AddPortMapping(WORD wPort, LPCTSTR pszProtocol = L"TCP", LPCTSTR pszDescription = L"Envy ED2K");
    bool RemovePortMapping(WORD wPort, LPCTSTR pszProtocol = L"TCP");
    bool GetExternalIP(CString& strExternalIP);

    // Discovery and management
    bool DiscoverDevices();
    void CleanupMappings();

    // Status
    bool IsAvailable() const { return m_bAvailable; }
    bool HasMapping(WORD wPort) const;

private:
    bool m_bAvailable;
    CMap<WORD, WORD&, bool, bool> m_mMappings;
    CCriticalSection m_pSection;

    // UPnP device discovery and communication methods
    bool SendDiscoveryRequest();
    bool ParseDiscoveryResponse(const CString& strResponse);
    bool SendPortMappingRequest(WORD wPort, LPCTSTR pszProtocol, LPCTSTR pszDescription, bool bAdd);
};
