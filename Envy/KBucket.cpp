//
// KBucket.cpp
//
// Kademlia KBucket routing table implementation
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

#include "StdAfx.h"

#ifdef ENVY_LEGACY_KADEMLIA

#include "KBucket.h"
#include <algorithm>
#include <queue>
#include <set>
#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Global routing table instance
KadRoutingTable* g_routingTable = NULL;

// Global own node ID
KadId g_own_id;

// ============================================================================
// KBucket Implementation
// ============================================================================

KBucket::KBucket(int k) : maxSize(k), lastRefresh(0) {
}

KBucket::~KBucket() {
    // Clean up all nodes
    for (auto node : nodes) {
        delete node;
    }
    nodes.clear();
}

bool KBucket::AddNode(const unsigned char* nodeId, const struct sockaddr_in& addr) {
    // Check if node already exists
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        KadNode* node = *it;
        if (kad_id_compare(node->id, nodeId) == 0) {
            // Update existing node
            node->addr = addr;
            node->lastSeen = time(NULL);
            node->isAlive = true;
            return true;
        }
    }

    // Node doesn't exist, add it if bucket isn't full
    if ((int)nodes.size() >= maxSize) {
        // Bucket is full - in a real implementation, we'd ping the oldest node
        // For now, just don't add
        return false;
    }

    // Create new node
    KadNode* newNode = new KadNode();
    memcpy(newNode->id, nodeId, KAD_ID_SIZE);
    newNode->addr = addr;
    newNode->lastSeen = time(NULL);
    newNode->isAlive = true;

    // Add to end of list (most recently seen)
    nodes.push_back(newNode);

    return true;
}

bool KBucket::RemoveNode(const unsigned char* nodeId) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        KadNode* node = *it;
        if (kad_id_compare(node->id, nodeId) == 0) {
            delete node;
            nodes.erase(it);
            return true;
        }
    }
    return false;
}

std::list<KadNode*> KBucket::GetClosestNodes(const unsigned char* targetId, int maxNodes) {
    std::list<KadNode*> result;

    // Create a copy of nodes sorted by distance to target
    std::vector<KadNode*> sortedNodes;
    for (auto node : nodes) {
        sortedNodes.push_back(node);
    }

    // Sort by XOR distance to target
    std::sort(sortedNodes.begin(), sortedNodes.end(),
              [&targetId](const KadNode* a, const KadNode* b) {
                  return CompareByDistance(a, b, targetId);
              });

    // Return up to maxNodes closest nodes
    int count = 0;
    for (auto node : sortedNodes) {
        if (count >= maxNodes) break;
        result.push_back(node);
        count++;
    }

    return result;
}

bool KBucket::CompareByDistance(const KadNode* a, const KadNode* b, const unsigned char* target) {
    unsigned char distA[KAD_ID_SIZE];
    unsigned char distB[KAD_ID_SIZE];

    kad_xor_distance(distA, a->id, target);
    kad_xor_distance(distB, b->id, target);

    return memcmp(distA, distB, KAD_ID_SIZE) < 0;
}

bool KBucket::NeedsRefresh(time_t now) const {
    return (now - lastRefresh) > KAD_REFRESH_INTERVAL;
}

void KBucket::MarkRefreshed(time_t now) {
    lastRefresh = now;
}

// ============================================================================
// KadRoutingTable Implementation
// ============================================================================

KadRoutingTable::KadRoutingTable(const KadId& ownId) {
    memcpy(ownNodeId, ownId, KAD_ID_SIZE);

    // Create buckets for each possible leading zero count
    // For 128-bit IDs, we can have 0-127 leading zeros, plus one bucket for identical IDs
    // Total: 128 buckets (0-127) for standard Kademlia
    // However, for efficiency, we can use fewer buckets by grouping
    // Using KAD_ID_SIZE * 8 = 128 buckets for full Kademlia compliance
    const int numBuckets = KAD_ID_SIZE * 8;
    buckets.resize(numBuckets);
    for (int i = 0; i < numBuckets; i++) {
        buckets[i] = new KBucket(KAD_K);
    }
}

KadRoutingTable::~KadRoutingTable() {
    for (auto bucket : buckets) {
        delete bucket;
    }
    buckets.clear();
}

int KadRoutingTable::GetBucketIndex(const unsigned char* nodeId) const {
    // Calculate XOR distance from our own ID
    unsigned char distance[KAD_ID_SIZE];
    kad_xor_distance(distance, ownNodeId, nodeId);

    // Kademlia bucket index is based on the number of leading zero bits in XOR distance
    // Bucket 0: distance MSB = 1 (0 leading zeros)
    // Bucket 1: distance has 1 leading zero
    // Bucket 2: distance has 2 leading zeros
    // etc.

    // Count leading zero bits from most significant byte
    int leadingZeros = 0;
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        unsigned char byte = distance[i];

        if (byte == 0) {
            // Entire byte is zero, count all 8 bits
            leadingZeros += 8;
        } else {
            // Find first set bit in this byte
            // Count leading zeros in this byte
            int bitPos = 7;
            while (bitPos >= 0 && !(byte & (1 << bitPos))) {
                leadingZeros++;
                bitPos--;
            }
            // Found first set bit, break
            break;
        }
    }

    // If all bits are zero, node ID is the same as ours
    // Put in the last bucket (bucket 127 for 128-bit IDs)
    if (leadingZeros == KAD_ID_SIZE * 8) {
        return (KAD_ID_SIZE * 8) - 1;
    }

    // Bucket index is the number of leading zeros
    // This correctly implements Kademlia bucket assignment:
    // - Nodes with XOR distance having 0 leading zeros go to bucket 0
    // - Nodes with XOR distance having 1 leading zero go to bucket 1
    // - etc.
    // Valid range: 0 to (KAD_ID_SIZE * 8 - 1)
    int bucketIndex = leadingZeros;

    // Safety check: clamp to valid bucket range
    if (bucketIndex < 0) {
        bucketIndex = 0;
    } else if (bucketIndex >= (int)buckets.size()) {
        bucketIndex = (int)buckets.size() - 1;
    }

    return bucketIndex;
}

void KadRoutingTable::XorDistance(KadId& result, const KadId& a, const KadId& b) {
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        result[i] = a[i] ^ b[i];
    }
}

bool KadRoutingTable::AddNode(const unsigned char* nodeId, const struct sockaddr_in& addr) {
    // Don't add our own ID
    if (kad_id_compare(ownNodeId, nodeId) == 0) {
        return false;
    }

    int bucketIndex = GetBucketIndex(nodeId);
    if (bucketIndex < 0 || bucketIndex >= (int)buckets.size()) {
        return false;
    }

    return buckets[bucketIndex]->AddNode(nodeId, addr);
}

bool KadRoutingTable::RemoveNode(const unsigned char* nodeId) {
    int bucketIndex = GetBucketIndex(nodeId);
    if (bucketIndex < 0 || bucketIndex >= (int)buckets.size()) {
        return false;
    }

    return buckets[bucketIndex]->RemoveNode(nodeId);
}

std::list<KadNode*> KadRoutingTable::FindClosestNodes(const unsigned char* targetId, int maxNodes) {
    std::list<KadNode*> result;

    // Start with the bucket that would contain the target
    int targetBucket = GetBucketIndex(targetId);

    // Collect nodes from buckets, starting with target bucket and expanding outward
    std::set<int> searchedBuckets;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> bucketQueue;

    // Add target bucket first
    bucketQueue.push(std::make_pair(0, targetBucket));
    searchedBuckets.insert(targetBucket);

    while (!bucketQueue.empty() && (int)result.size() < maxNodes) {
        auto current = bucketQueue.top();
        bucketQueue.pop();

        int distance = current.first;
        int bucketIndex = current.second;

        // Get nodes from this bucket
        if (bucketIndex >= 0 && bucketIndex < (int)buckets.size()) {
            auto bucketNodes = buckets[bucketIndex]->GetClosestNodes(targetId, maxNodes - result.size());
            result.insert(result.end(), bucketNodes.begin(), bucketNodes.end());
        }

        // Add adjacent buckets if we still need more nodes
        if ((int)result.size() < maxNodes) {
            // Add buckets at increasing distance
            for (int d = 1; d <= distance + 1 && (int)result.size() < maxNodes; d++) {
                int lowerBucket = targetBucket - d;
                int upperBucket = targetBucket + d;

                if (lowerBucket >= 0 && searchedBuckets.find(lowerBucket) == searchedBuckets.end()) {
                    bucketQueue.push(std::make_pair(d, lowerBucket));
                    searchedBuckets.insert(lowerBucket);
                }

                if (upperBucket < (int)buckets.size() && searchedBuckets.find(upperBucket) == searchedBuckets.end()) {
                    bucketQueue.push(std::make_pair(d, upperBucket));
                    searchedBuckets.insert(upperBucket);
                }
            }
        }
    }

    return result;
}

std::list<KadNode*> KadRoutingTable::GetNodesToPing(time_t now) {
    std::list<KadNode*> result;

    // Check all buckets for nodes that haven't been seen recently
    for (auto bucket : buckets) {
        // In a real implementation, we'd check each node individually
        // For now, just return empty list
    }

    return result;
}

std::list<int> KadRoutingTable::GetBucketsToRefresh(time_t now) {
    std::list<int> result;

    for (size_t i = 0; i < buckets.size(); i++) {
        if (buckets[i]->NeedsRefresh(now)) {
            result.push_back((int)i);
        }
    }

    return result;
}

void KadRoutingTable::GetStats(int& totalNodes, int& totalBuckets) {
    totalNodes = 0;
    totalBuckets = (int)buckets.size();

    for (auto bucket : buckets) {
        totalNodes += (int)bucket->GetSize();
    }
}

#endif // ENVY_LEGACY_KADEMLIA
