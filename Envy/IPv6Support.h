//
// IPv6Support.h
//
// IPv6 support utilities for ED2K protocol
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#pragma once

#include <ws2tcpip.h>

// IPv6 address structure
struct IPv6Address
{
    union {
        BYTE bytes[16];
        WORD words[8];
        DWORD dwords[4];
    } addr;

    IPv6Address();
    IPv6Address(const struct in6_addr& ipv6);

    // Conversion methods
    bool FromString(LPCTSTR pszAddress);
    CString ToString() const;
    bool IsValid() const;

    // Comparison
    bool operator==(const IPv6Address& other) const;
    bool operator!=(const IPv6Address& other) const;

    // Special addresses
    static IPv6Address AnyAddress();      // ::
    static IPv6Address Loopback();        // ::1
    static IPv6Address V4Mapped();        // ::ffff:0:0/96

    // IPv4 compatibility
    bool IsIPv4Mapped() const;
    DWORD GetIPv4Mapped() const;
};

class CIPv6Manager
{
public:
    CIPv6Manager();
    virtual ~CIPv6Manager();

    // IPv6 support detection
    static bool IsIPv6Supported();
    static bool HasIPv6Connectivity();

    // Address resolution
    static bool ResolveAddress(LPCTSTR pszHost, WORD wPort, IPv6Address& address, bool bPreferIPv6 = true);
    static bool GetLocalAddresses(CArray<IPv6Address>& addresses);

    // Socket operations
    static bool CreateDualStackSocket(SOCKET& hSocket, WORD wPort);
    static bool BindSocketIPv6(SOCKET hSocket, const IPv6Address& address, WORD wPort);

private:
    static bool m_bIPv6Supported;
    static bool m_bInitialized;
};
