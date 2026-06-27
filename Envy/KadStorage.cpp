//
// KadStorage.cpp
//
// Kademlia key-value storage implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//

#include "StdAfx.h"

#ifdef ENVY_LEGACY_KADEMLIA

#include "KadStorage.h"
#include "KBucket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Global storage instance (defined in Kademlia.cpp)

// Maximum values per key
#define KAD_MAX_VALUES_PER_KEY 10

// Default expiration time (24 hours)
#define KAD_DEFAULT_EXPIRATION (24 * 60 * 60)

KadStorage::KadStorage() {
}

KadStorage::~KadStorage() {
    m_storage.clear();
}

bool KadStorage::Store(const unsigned char* key, const std::string& value, time_t expiration) {
    if (expiration == 0) {
        expiration = time(NULL) + KAD_DEFAULT_EXPIRATION;
    }

    std::string keyStr = KadIdToString(key);
    KadValue newValue("", value, expiration);

    // Check if key already exists
    auto it = m_storage.find(keyStr);
    if (it != m_storage.end()) {
        // Limit number of values per key
        if (it->second.size() >= KAD_MAX_VALUES_PER_KEY) {
            // Remove oldest value
            it->second.pop_front();
        }

        // Add new value
        it->second.push_back(newValue);
    } else {
        // Create new key entry
        std::list<KadValue> values;
        values.push_back(newValue);
        m_storage[keyStr] = values;
    }

    return true;
}

std::list<std::string> KadStorage::Find(const unsigned char* key) {
    std::list<std::string> results;

    std::string keyStr = KadIdToString(key);
    auto it = m_storage.find(keyStr);

    if (it != m_storage.end()) {
        time_t now = time(NULL);

        // Return non-expired values
        for (const auto& value : it->second) {
            if (value.expiration > now) {
                results.push_back(value.value);
            }
        }

        // Clean expired values
        it->second.remove_if([now](const KadValue& v) {
            return v.expiration <= now;
        });

        // Remove key if no values left
        if (it->second.empty()) {
            m_storage.erase(it);
        }
    }

    return results;
}

bool KadStorage::IsResponsible(const unsigned char* key) {
    if (!g_routingTable) {
        return false;
    }

    // We are responsible if we are among the K closest nodes to the key
    auto closestNodes = g_routingTable->FindClosestNodes(key, KAD_K);

    // Check if our node ID is in the closest nodes
    for (auto node : closestNodes) {
        if (kad_id_compare(node->id, g_own_id) == 0) {
            return true;
        }
    }

    return false;
}

std::list<std::string> KadStorage::GetKeysToRepublish(time_t now) {
    std::list<std::string> keysToRepublish;

    for (const auto& entry : m_storage) {
        const auto& values = entry.second;

        // Check if any value needs republishing
        for (const auto& value : values) {
            if (now - value.lastRepublished > KAD_REPUBLISH_INTERVAL) {
                keysToRepublish.push_back(entry.first);
                break; // Only add key once
            }
        }
    }

    return keysToRepublish;
}

void KadStorage::CleanExpired(time_t now) {
    for (auto it = m_storage.begin(); it != m_storage.end(); ) {
        // Remove expired values
        it->second.remove_if([now](const KadValue& v) {
            return v.expiration <= now;
        });

        // Remove empty keys
        if (it->second.empty()) {
            it = m_storage.erase(it);
        } else {
            ++it;
        }
    }
}

size_t KadStorage::GetSize() const {
    size_t totalValues = 0;
    for (const auto& entry : m_storage) {
        totalValues += entry.second.size();
    }
    return totalValues;
}

std::string KadStorage::KadIdToString(const unsigned char* id) {
    char buffer[KAD_ID_SIZE * 2 + 1];
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        ASSERT( static_cast<size_t>(i * 2) < sizeof(buffer) );
        const size_t remaining = sizeof(buffer) - static_cast<size_t>(i * 2);
        sprintf_s(buffer + i * 2, remaining, "%02x", static_cast<unsigned int>(id[i]));
    }
    buffer[KAD_ID_SIZE * 2] = '\0';
    return std::string(buffer);
}

void KadStorage::StringToKadId(const std::string& str, KadId& id) {
    memset(&id[0], 0, KAD_ID_SIZE);
    if (str.length() < KAD_ID_SIZE * 2) return;
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        // Validate hex characters before parsing
        const char c1 = str[i * 2];
        const char c2 = str[i * 2 + 1];
        if (!isxdigit(static_cast<unsigned char>(c1)) || !isxdigit(static_cast<unsigned char>(c2))) {
            TRACE("KadStorage::StringToKadId: Invalid hex character at position %d\n", i * 2);
            return;
        }
        unsigned int byte = 0;
        sscanf_s(str.c_str() + i * 2, "%02x", &byte);
        id[i] = static_cast<unsigned char>(byte);
    }
}

#endif // ENVY_LEGACY_KADEMLIA
