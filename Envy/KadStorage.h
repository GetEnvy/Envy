//
// KadStorage.h
//
// Kademlia key-value storage for file indexing
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// NOTE: This is legacy code. To enable, define ENVY_LEGACY_KADEMLIA.
// The current implementation uses Kademlia.h / CKademlia class instead.

#pragma once

#ifdef ENVY_LEGACY_KADEMLIA

#include "Kademlia.h"
#include <map>
#include <list>
#include <time.h>

// Stored value in DHT
struct KadValue {
    std::string key;              // Keyword/file hash
    std::string value;            // File information (JSON or binary)
    time_t expiration;            // When this value expires
    time_t lastRepublished;       // Last time this was republished

    KadValue() : expiration(0), lastRepublished(0) {}
    KadValue(const std::string& k, const std::string& v, time_t exp)
        : key(k), value(v), expiration(exp), lastRepublished(time(NULL)) {}
};

// Key-value storage for local DHT data
class KadStorage {
public:
    KadStorage();
    ~KadStorage();

    // Store a key-value pair
    bool Store(const unsigned char* key, const std::string& value, time_t expiration = 0);

    // Retrieve values for a key
    std::list<std::string> Find(const unsigned char* key);

    // Check if we are responsible for a key (closest node)
    bool IsResponsible(const unsigned char* key);

    // Get all keys we should republish
    std::list<std::string> GetKeysToRepublish(time_t now);

    // Clean expired values
    void CleanExpired(time_t now);

    // Get storage statistics
    size_t GetSize() const;

private:
    std::map<std::string, std::list<KadValue>> m_storage; // Key -> list of values

    // Convert KadId to string key for map
    static std::string KadIdToString(const unsigned char* id);
    static void StringToKadId(const std::string& str, KadId& id);
};

// Global storage instance
extern KadStorage* g_kadStorage;

#endif // ENVY_LEGACY_KADEMLIA
