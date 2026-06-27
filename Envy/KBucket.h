//
// KBucket.h
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

#pragma once

#ifdef ENVY_LEGACY_KADEMLIA

#include "Kademlia.h"
#include <list>
#include <time.h>

// Kademlia node information stored in routing table
struct KadNode {
    KadId id;                    // Node ID (128-bit)
    struct sockaddr_in addr;     // Node address
    time_t lastSeen;             // Last time we heard from this node
    time_t lastPinged;           // Last time we sent a ping
    bool isAlive;               // Whether node responded recently

    KadNode() : lastSeen(0), lastPinged(0), isAlive(false) {
        memset(&addr, 0, sizeof(addr));
    }
};

// Individual KBucket
class KBucket {
public:
    KBucket(int k = KAD_K);
    ~KBucket();

    // Add or update a node in this bucket
    bool AddNode(const unsigned char* nodeId, const struct sockaddr_in& addr);

    // Remove a node from this bucket
    bool RemoveNode(const unsigned char* nodeId);

    // Get closest nodes to a target ID
    std::list<KadNode*> GetClosestNodes(const unsigned char* targetId, int maxNodes);

    // Check if bucket needs to be refreshed
    bool NeedsRefresh(time_t now) const;

    // Mark bucket as refreshed
    void MarkRefreshed(time_t now);

    // Get bucket statistics
    size_t GetSize() const { return nodes.size(); }
    bool IsFull() const { return nodes.size() >= (size_t)maxSize; }

private:
    std::list<KadNode*> nodes;    // Nodes in this bucket (sorted by last seen)
    int maxSize;                  // Maximum nodes per bucket
    time_t lastRefresh;           // Last time bucket was refreshed

    // Sort nodes by distance to target (closer nodes first)
    static bool CompareByDistance(const KadNode* a, const KadNode* b, const unsigned char* target);
};

// Complete Kademlia routing table
class KadRoutingTable {
public:
    KadRoutingTable(const KadId& ownId);
    ~KadRoutingTable();

    // Add or update a node in the routing table
    bool AddNode(const unsigned char* nodeId, const struct sockaddr_in& addr);

    // Remove a node from the routing table
    bool RemoveNode(const unsigned char* nodeId);

    // Find closest nodes to a target
    std::list<KadNode*> FindClosestNodes(const unsigned char* targetId, int maxNodes);

    // Get nodes that need pinging
    std::list<KadNode*> GetNodesToPing(time_t now);

    // Get buckets that need refreshing
    std::list<int> GetBucketsToRefresh(time_t now);

    // Get routing table statistics
    void GetStats(int& totalNodes, int& totalBuckets);

private:
    KadId ownNodeId;                    // Our own node ID
    std::vector<KBucket*> buckets;      // KBuckets for each distance range

    // Calculate which bucket a node belongs to
    int GetBucketIndex(const unsigned char* nodeId) const;

    // Calculate XOR distance between two IDs
    static void XorDistance(KadId& result, const KadId& a, const KadId& b);
};

// Global routing table instance
extern KadRoutingTable* g_routingTable;

#endif // ENVY_LEGACY_KADEMLIA
