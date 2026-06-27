//
// UPnPManager.cpp
//
// Universal Plug and Play (UPnP) implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "StdAfx.h"
#include "UPnPManager.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CUPnPManager::CUPnPManager()
    : m_bAvailable(false)
{
}

CUPnPManager::~CUPnPManager()
{
    CleanupMappings();
}

bool CUPnPManager::AddPortMapping(WORD wPort, LPCTSTR pszProtocol, LPCTSTR pszDescription)
{
    CSingleLock pLock(&m_pSection, TRUE);

    if (!m_bAvailable) {
        if (!DiscoverDevices()) {
            return false;
        }
    }

    // Check if mapping already exists
    if (HasMapping(wPort)) {
        return true; // Already mapped
    }

    if (SendPortMappingRequest(wPort, pszProtocol, pszDescription, true)) {
        m_mMappings.SetAt(wPort, true);
        theApp.Message(MSG_DEBUG, L"UPnP: Successfully mapped port %d (%s)", wPort, pszProtocol);
        return true;
    }

    return false;
}

bool CUPnPManager::RemovePortMapping(WORD wPort, LPCTSTR pszProtocol)
{
    CSingleLock pLock(&m_pSection, TRUE);

    if (!m_bAvailable || !HasMapping(wPort)) {
        return true; // Nothing to remove or not available
    }

    if (SendPortMappingRequest(wPort, pszProtocol, L"", false)) {
        m_mMappings.RemoveKey(wPort);
        theApp.Message(MSG_DEBUG, L"UPnP: Successfully removed port mapping for %d", wPort);
        return true;
    }

    return false;
}

bool CUPnPManager::GetExternalIP(CString& strExternalIP)
{
    // Basic implementation - would need actual UPnP communication
    // For now, return empty string to indicate not implemented
    strExternalIP.Empty();
    return false;
}

bool CUPnPManager::DiscoverDevices()
{
    // Send UPnP discovery request
    if (SendDiscoveryRequest()) {
        m_bAvailable = true;
        theApp.Message(MSG_DEBUG, L"UPnP: Device discovery successful");
        return true;
    }

    m_bAvailable = false;
    theApp.Message(MSG_DEBUG, L"UPnP: No UPnP devices found");
    return false;
}

void CUPnPManager::CleanupMappings()
{
    CSingleLock pLock(&m_pSection, TRUE);

    // Remove all active mappings
    for (POSITION pos = m_mMappings.GetStartPosition(); pos != NULL;) {
        WORD wPort;
        bool bMapped;
        m_mMappings.GetNextAssoc(pos, wPort, bMapped);
        if (bMapped) {
            RemovePortMapping(wPort, L"TCP");
            RemovePortMapping(wPort, L"UDP");
        }
    }

    m_mMappings.RemoveAll();
}

bool CUPnPManager::HasMapping(WORD wPort) const
{
    bool bMapped = false;
    m_mMappings.Lookup(wPort, bMapped);
    return bMapped;
}

bool CUPnPManager::SendDiscoveryRequest()
{
    // This is a placeholder for actual UPnP M-SEARCH request
    // In a full implementation, this would send SSDP discovery packets
    // and parse responses to find UPnP-enabled routers

    // For now, return false to indicate UPnP is not implemented
    return false;
}

bool CUPnPManager::ParseDiscoveryResponse(const CString& strResponse)
{
    // Parse UPnP discovery response
    // Would extract device information and control URLs
    return false;
}

bool CUPnPManager::SendPortMappingRequest(WORD wPort, LPCTSTR pszProtocol, LPCTSTR pszDescription, bool bAdd)
{
    // This is a placeholder for actual UPnP AddPortMapping or DeletePortMapping request
    // In a full implementation, this would use SOAP over HTTP to communicate with the UPnP device

    // For now, return false to indicate UPnP is not fully implemented
    return false;
}
