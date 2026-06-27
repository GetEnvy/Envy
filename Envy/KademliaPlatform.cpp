//
// KademliaPlatform.cpp
//
// Platform-specific functions for Kademlia DHT
// Windows/Visual Studio implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//

#include "StdAfx.h"
#include "Kademlia.h"
#include "Security.h"
#include "Network.h"
#include "../HashLib/HashLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Check if an IP address is blacklisted
int kad_blacklisted(const struct sockaddr *addr, int addr_len)
{
    if (!addr || addr->sa_family != AF_INET) {
        return 1; // Blacklist unknown address families
    }

    const struct sockaddr_in *addr_in = (const struct sockaddr_in *)addr;
    const IN_ADDR *ip_addr = &addr_in->sin_addr;

    // Use Envy's security system to check if IP is blocked
    if (Security.IsDenied(ip_addr)) {
        return 1; // Blacklisted
    }

    // Check if it's a firewalled address
    if (Network.IsFirewalledAddress(ip_addr, FALSE)) {
        return 1; // Don't communicate with firewalled addresses
    }

    // Check if it's a reserved/private address that we shouldn't contact
    if (Network.IsReserved(ip_addr)) {
        return 1; // Reserved addresses
    }

    return 0; // Not blacklisted
}

// Hash function - uses Envy's existing hash functions
void kad_hash(void *hash_return, int hash_size,
             const void *v1, int len1,
             const void *v2, int len2,
             const void *v3, int len3)
{
    // For Kademlia, we typically need SHA-1 or similar
    // Use Envy's SHA class if available, otherwise fallback to simple hash

    // Simple implementation - concatenate inputs and hash
    std::vector<unsigned char> data;

    if (v1 && len1 > 0) {
        data.insert(data.end(), (unsigned char*)v1, (unsigned char*)v1 + len1);
    }
    if (v2 && len2 > 0) {
        data.insert(data.end(), (unsigned char*)v2, (unsigned char*)v2 + len2);
    }
    if (v3 && len3 > 0) {
        data.insert(data.end(), (unsigned char*)v3, (unsigned char*)v3 + len3);
    }

    // Use proper SHA-1 cryptographic hash from Envy's hash library
    CSHA sha1;
    sha1.Add(data.data(), data.size());
    sha1.Finish();
    sha1.GetHash(reinterpret_cast<unsigned char*>(hash_return));
}

// Generate random bytes
int kad_random_bytes(void *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    // Use Windows CryptGenRandom for cryptographic randomness
    HCRYPTPROV hProvider = 0;

    if (!CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        // Use GenerateCryptographicBytes for secure random generation (P0.2 security requirement)
        if (!GenerateCryptographicBytes((BYTE *)buf, size)) {
            // Critical security failure - cannot generate secure random bytes
            return -1; // Fail rather than use insecure rand()
        }
        return 0;
    }

    BOOL success = CryptGenRandom(hProvider, (DWORD)size, (BYTE *)buf);
    CryptReleaseContext(hProvider, 0);

    return success ? 0 : -1;
}

// Send UDP packet
int kad_sendto(int socket_fd, const void *buf, int len, int flags,
              const struct sockaddr *to, int tolen)
{
    if (socket_fd < 0 || !buf || !to) {
        return -1;
    }

    // Use Windows sendto
    int result = sendto(socket_fd, (const char *)buf, len, flags, to, tolen);

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        TRACE("kad_sendto failed: %d\n", error);
        return -1;
    }

    return result;
}
