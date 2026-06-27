//
// IPv6Support.cpp
//
// IPv6 support implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "StdAfx.h"
#include "IPv6Support.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// Static member initialization
bool CIPv6Manager::m_bIPv6Supported = false;
bool CIPv6Manager::m_bInitialized = false;

////////////////////////////////////////////////////////////////////////////////
// IPv6Address Implementation
////////////////////////////////////////////////////////////////////////////////

IPv6Address::IPv6Address()
{
    memset(&addr, 0, sizeof(addr));
}

IPv6Address::IPv6Address(const struct in6_addr& ipv6)
{
    memcpy(&addr, &ipv6, sizeof(addr));
}

bool IPv6Address::FromString(LPCTSTR pszAddress)
{
    if (!pszAddress || !*pszAddress)
        return false;

    struct sockaddr_in6 sa;
    int result = InetPtonW(AF_INET6, pszAddress, &sa.sin6_addr);

    if (result == 1) {
        memcpy(&addr, &sa.sin6_addr, sizeof(addr));
        return true;
    }

    // Try IPv4 mapped address
    struct sockaddr_in sa4;
    result = InetPtonW(AF_INET, pszAddress, &sa4.sin_addr);
    if (result == 1) {
        // Create IPv4-mapped IPv6 address
        memset(&addr, 0, sizeof(addr));
        addr.words[5] = 0xFFFF;
        memcpy(&addr.dwords[3], &sa4.sin_addr, 4);
        return true;
    }

    return false;
}

CString IPv6Address::ToString() const
{
    WCHAR buffer[INET6_ADDRSTRLEN];
    if (InetNtopW(AF_INET6, &addr, buffer, INET6_ADDRSTRLEN)) {
        return buffer;
    }
    return L"";
}

bool IPv6Address::IsValid() const
{
    // Check if not all zeros (though :: is technically valid)
    for (int i = 0; i < 16; ++i) {
        if (addr.bytes[i] != 0)
            return true;
    }
    return false;
}

bool IPv6Address::operator==(const IPv6Address& other) const
{
    return memcmp(&addr, &other.addr, sizeof(addr)) == 0;
}

bool IPv6Address::operator!=(const IPv6Address& other) const
{
    return !(*this == other);
}

IPv6Address IPv6Address::AnyAddress()
{
    return IPv6Address(); // All zeros
}

IPv6Address IPv6Address::Loopback()
{
    IPv6Address addr;
    addr.addr.bytes[15] = 1; // ::1
    return addr;
}

IPv6Address IPv6Address::V4Mapped()
{
    IPv6Address addr;
    addr.addr.words[5] = 0xFFFF; // ::ffff:0:0
    return addr;
}

bool IPv6Address::IsIPv4Mapped() const
{
    return (addr.words[0] == 0 && addr.words[1] == 0 &&
            addr.words[2] == 0 && addr.words[3] == 0 &&
            addr.words[4] == 0 && addr.words[5] == 0xFFFF);
}

DWORD IPv6Address::GetIPv4Mapped() const
{
    if (!IsIPv4Mapped())
        return 0;

    return addr.dwords[3];
}

////////////////////////////////////////////////////////////////////////////////
// CIPv6Manager Implementation
////////////////////////////////////////////////////////////////////////////////

CIPv6Manager::CIPv6Manager()
{
    if (!m_bInitialized) {
        m_bIPv6Supported = IsIPv6Supported();
        m_bInitialized = true;
    }
}

CIPv6Manager::~CIPv6Manager()
{
}

bool CIPv6Manager::IsIPv6Supported()
{
    // Check if IPv6 is supported on this system
    SOCKET testSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (testSocket != INVALID_SOCKET) {
        closesocket(testSocket);
        return true;
    }
    return false;
}

bool CIPv6Manager::HasIPv6Connectivity()
{
    if (!m_bIPv6Supported)
        return false;

    // Try to resolve an IPv6-only hostname to test connectivity
    // This is a simplified check
    addrinfo hints = {0};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    int error = getaddrinfo("ipv6.google.com", "80", &hints, &result);
    if (error == 0 && result != nullptr) {
        freeaddrinfo(result);
        return true;
    }

    return false;
}

bool CIPv6Manager::ResolveAddress(LPCTSTR pszHost, WORD wPort, IPv6Address& address, bool bPreferIPv6)
{
    if (!pszHost || !*pszHost)
        return false;

    // Use getaddrinfo for dual-stack resolution
    addrinfo hints = {0};
    hints.ai_family = bPreferIPv6 ? AF_INET6 : AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;

    char portStr[16];
    sprintf_s(portStr, sizeof(portStr), "%d", wPort);

    addrinfo* result = nullptr;
    int error = getaddrinfo(CT2A(pszHost), portStr, &hints, &result);
    if (error != 0 || !result)
        return false;

    // Take the first result
    if (result->ai_family == AF_INET6) {
        struct sockaddr_in6* sa6 = (struct sockaddr_in6*)result->ai_addr;
        address = IPv6Address(sa6->sin6_addr);
    } else if (result->ai_family == AF_INET) {
        // Create IPv4-mapped IPv6 address
        struct sockaddr_in* sa4 = (struct sockaddr_in*)result->ai_addr;
        address = IPv6Address::V4Mapped();
        memcpy(&address.addr.dwords[3], &sa4->sin_addr, 4);
    }

    freeaddrinfo(result);
    return true;
}

bool CIPv6Manager::GetLocalAddresses(CArray<IPv6Address>& addresses)
{
    addresses.RemoveAll();

    if (!m_bIPv6Supported)
        return false;

    // Get local IPv6 addresses
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
        return false;

    addrinfo hints = {0};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    if (getaddrinfo(hostname, NULL, &hints, &result) != 0)
        return false;

    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET6) {
            struct sockaddr_in6* sa6 = (struct sockaddr_in6*)ptr->ai_addr;
            IPv6Address addr(sa6->sin6_addr);
            addresses.Add(addr);
        }
    }

    freeaddrinfo(result);
    return addresses.GetSize() > 0;
}

bool CIPv6Manager::CreateDualStackSocket(SOCKET& hSocket, WORD wPort)
{
    if (!m_bIPv6Supported)
        return false;

    // Create IPv6 socket that can handle both IPv4 and IPv6
    hSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (hSocket == INVALID_SOCKET)
        return false;

    // Set IPv6-only option to false to allow dual stack
    int no = 0;
    if (setsockopt(hSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&no, sizeof(no)) != 0) {
        closesocket(hSocket);
        hSocket = INVALID_SOCKET;
        return false;
    }

    return true;
}

bool CIPv6Manager::BindSocketIPv6(SOCKET hSocket, const IPv6Address& address, WORD wPort)
{
    if (hSocket == INVALID_SOCKET || !address.IsValid())
        return false;

    struct sockaddr_in6 sa6 = {0};
    sa6.sin6_family = AF_INET6;
    sa6.sin6_port = htons(wPort);
    memcpy(&sa6.sin6_addr, &address.addr, sizeof(address.addr));

    return bind(hSocket, (struct sockaddr*)&sa6, sizeof(sa6)) == 0;
}
