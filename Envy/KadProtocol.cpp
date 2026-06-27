//
// KadProtocol.cpp
//
// Kademlia protocol message handling and UDP networking
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

#include "KadProtocol.h"
#include "KBucket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Global protocol instance
KadProtocol* g_kadProtocol = NULL;

// Default ports
#define DEFAULT_KAD_TCP_PORT 4672
#define DEFAULT_KAD_UDP_PORT 4672

KadProtocol::KadProtocol() : m_socketFd(-1), m_initialized(false) {
}

KadProtocol::~KadProtocol() {
    m_socketFd = -1;
    m_initialized = false;
}

bool KadProtocol::Initialize(int socketFd) {
    if (m_initialized) {
        return false;
    }

    m_socketFd = socketFd;
    m_initialized = true;

    return true;
}

bool KadProtocol::SendBootstrapRequest(const struct sockaddr_in& target) {
    if (!m_initialized) return false;

    // Bootstrap request has no body, just header
    unsigned char buffer[sizeof(KadPacketHeader)];
    KadPacketHeader* header = (KadPacketHeader*)buffer;

    header->messageType = KAD_MSG_REQ;
    header->operationCode = KAD_OP_BOOTSTRAP;
    header->bodyLength = 0;

    return SendPacket(buffer, sizeof(KadPacketHeader), target);
}

bool KadProtocol::SendHelloRequest(const unsigned char* targetId, const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreateHelloRequest(buffer, sizeof(buffer), targetId,
                                       GetTcpPort(), GetUdpPort());

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

bool KadProtocol::SendHelloResponse(const unsigned char* targetId, const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreateHelloResponse(buffer, sizeof(buffer), targetId,
                                        GetTcpPort(), GetUdpPort());

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

bool KadProtocol::SendFindNodeRequest(const unsigned char* targetId, const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreateFindNodeRequest(buffer, sizeof(buffer), targetId);

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

bool KadProtocol::SendFindNodeResponse(const unsigned char* targetId, const std::list<KadNode*>& nodes,
                                      const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreateFindNodeResponse(buffer, sizeof(buffer), targetId, nodes);

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

bool KadProtocol::SendPublishRequest(const unsigned char* targetId, const char* keyword,
                                   const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreatePublishRequest(buffer, sizeof(buffer), targetId, keyword);

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

bool KadProtocol::SendPublishResponse(const unsigned char* targetId, bool success,
                                    const struct sockaddr_in& target) {
    unsigned char buffer[KAD_MAX_PACKET_SIZE];
    int packetSize = CreatePublishResponse(buffer, sizeof(buffer), targetId, success);

    if (packetSize <= 0) return false;

    return SendPacket(buffer, packetSize, target);
}

// Public interface for publishing
bool KadProtocol::PublishKeyword(const unsigned char* targetId, const char* keyword) {
    if (!m_initialized || !targetId || !keyword) return false;

    // Find closest nodes to the target ID
    if (!g_routingTable) return false;

    auto closestNodes = g_routingTable->FindClosestNodes(targetId, KAD_K);

    // Send publish requests to closest nodes
    bool success = false;
    for (auto node : closestNodes) {
        if (SendPublishRequest(targetId, keyword, node->addr)) {
            success = true;
            // In a real implementation, we'd wait for responses and handle retries
        }
    }

    return success;
}

bool KadProtocol::ReceivePacket() {
    if (!m_initialized) return false;

    KadPacket packet;
    packet.fromAddrLen = sizeof(packet.fromAddr);

    int received = recvfrom(m_socketFd, (char*)&packet.header,
                           sizeof(KadPacketHeader), MSG_PEEK,
                           (struct sockaddr*)&packet.fromAddr, &packet.fromAddrLen);

    if (received != sizeof(KadPacketHeader)) {
        return false;
    }

    // Get body length
    packet.bodyLength = packet.header.bodyLength;
    if (packet.bodyLength > sizeof(packet.body)) {
        return false; // Packet too large
    }

    // Receive the full packet
    int totalSize = sizeof(KadPacketHeader) + packet.bodyLength;
    received = recvfrom(m_socketFd, (char*)&packet.header, totalSize, 0,
                       (struct sockaddr*)&packet.fromAddr, &packet.fromAddrLen);

    if (received == totalSize) {
        ProcessPacket(packet);
        return true;
    }

    return false;
}

void KadProtocol::ProcessPacket(const KadPacket& packet) {
    switch (packet.header.operationCode) {
    case KAD_OP_HELLO:
        ProcessHelloRequest(packet);
        break;
    case KAD_OP_HELLO_RES:
        ProcessHelloResponse(packet);
        break;
    case KAD_OP_SEARCH_REQ:
        ProcessFindNodeRequest(packet);
        break;
    case KAD_OP_SEARCH_RES:
        ProcessFindNodeResponse(packet);
        break;
    case KAD_OP_PUBLISH_REQ:
        ProcessPublishRequest(packet);
        break;
    case KAD_OP_PUBLISH_RES:
        ProcessPublishResponse(packet);
        break;
    // Add other packet types as implemented
    default:
        // Unknown packet type, ignore
        break;
    }
}

void KadProtocol::ProcessHelloRequest(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadHelloRequest)) {
        return; // Invalid packet
    }

    const KadHelloRequest* request = (const KadHelloRequest*)packet.body;

    // Add the sender to our routing table
    kad_insert_node(request->targetId, (const struct sockaddr*)&packet.fromAddr, packet.fromAddrLen);

    // Send hello response
    SendHelloResponse(request->targetId, packet.fromAddr);
}

void KadProtocol::ProcessHelloResponse(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadHelloResponse)) {
        return; // Invalid packet
    }

    const KadHelloResponse* response = (const KadHelloResponse*)packet.body;

    // Add the responder to our routing table
    kad_insert_node(response->targetId, (const struct sockaddr*)&packet.fromAddr, packet.fromAddrLen);

    // Mark as successfully contacted
    // TODO: Update node status in routing table
}

void KadProtocol::ProcessFindNodeRequest(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadSearchRequest)) {
        return; // Invalid packet
    }

    const KadSearchRequest* request = (const KadSearchRequest*)packet.body;

    // Find closest nodes to the target
    if (g_routingTable) {
        auto closestNodes = g_routingTable->FindClosestNodes(request->targetId, KAD_K);

        // Send response with closest nodes
        SendFindNodeResponse(request->targetId, closestNodes, packet.fromAddr);
    }
}

void KadProtocol::ProcessFindNodeResponse(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadSearchResponse)) {
        return; // Invalid packet
    }

    const KadSearchResponse* response = (const KadSearchResponse*)packet.body;

    // Process the search results
    const unsigned char* resultData = packet.body + sizeof(KadSearchResponse);
    int resultsProcessed = 0;

    for (int i = 0; i < response->resultCount && resultsProcessed < response->resultCount; i++) {
        if (packet.bodyLength < (int)(sizeof(KadSearchResponse) + (i + 1) * sizeof(KadSearchResult))) {
            break; // Not enough data
        }

        const KadSearchResult* result = (const KadSearchResult*)(resultData + i * sizeof(KadSearchResult));

        // Convert to sockaddr_in
        struct sockaddr_in nodeAddr;
        memset(&nodeAddr, 0, sizeof(nodeAddr));
        nodeAddr.sin_family = AF_INET;
        nodeAddr.sin_addr.s_addr = result->ipAddress;
        nodeAddr.sin_port = result->udpPort;

        // Add to routing table
        kad_insert_node(result->nodeId, (const struct sockaddr*)&nodeAddr, sizeof(nodeAddr));
        resultsProcessed++;
    }
}

void KadProtocol::ProcessPublishRequest(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadPublishRequest)) {
        return; // Invalid packet
    }

    const KadPublishRequest* request = (const KadPublishRequest*)packet.body;

    // Extract keyword from packet data
    const char* keyword = (const char*)(packet.body + sizeof(KadPublishRequest));
    size_t keywordLen = packet.bodyLength - sizeof(KadPublishRequest);

    if (keywordLen == 0 || keywordLen > 255) {
        return; // Invalid keyword length
    }

    // Store the keyword/file association in our local DHT
    // In a real implementation, this would be stored persistently
    bool success = kad_store(request->targetId, keyword);

    // Send response
    SendPublishResponse(request->targetId, success, packet.fromAddr);
}

void KadProtocol::ProcessPublishResponse(const KadPacket& packet) {
    if (packet.bodyLength < sizeof(KadPublishResponse)) {
        return; // Invalid packet
    }

    const KadPublishResponse* response = (const KadPublishResponse*)packet.body;

    // Handle publish response - could update statistics or retry logic
    // For now, just acknowledge receipt
    (void)response; // Suppress unused parameter warning
}

bool KadProtocol::SendPacket(const unsigned char* data, int dataLength, const struct sockaddr_in& target) {
    if (!m_initialized || !data) {
        return false;
    }

    return (kad_sendto(m_socketFd, data, dataLength, 0,
                      (const struct sockaddr*)&target, sizeof(target)) >= 0);
}

unsigned short KadProtocol::GetTcpPort() const {
    // Get actual TCP port from network settings
    return htons(Network.GetPort());
}

unsigned short KadProtocol::GetUdpPort() const {
    // Get actual UDP port from network settings (Kad uses same port as TCP)
    return htons(Network.GetPort());
}

// Static packet creation methods

int KadProtocol::CreateHelloRequest(unsigned char* buffer, int bufferSize,
                                   const unsigned char* targetId, unsigned short tcpPort, unsigned short udpPort) {
    if (bufferSize < (int)sizeof(KadPacketHeader) + (int)sizeof(KadHelloRequest)) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadHelloRequest* request = (KadHelloRequest*)(buffer + sizeof(KadPacketHeader));

    // Fill header
    header->messageType = KAD_MSG_REQ;
    header->operationCode = KAD_OP_HELLO;
    header->bodyLength = sizeof(KadHelloRequest);

    // Fill request body
    memcpy(request->targetId, targetId, KAD_ID_SIZE);
    request->tcpPort = tcpPort;
    request->udpPort = udpPort;
    request->version = 0x0100; // Version 1.0

    return sizeof(KadPacketHeader) + sizeof(KadHelloRequest);
}

int KadProtocol::CreateHelloResponse(unsigned char* buffer, int bufferSize,
                                    const unsigned char* targetId, unsigned short tcpPort, unsigned short udpPort) {
    if (bufferSize < (int)sizeof(KadPacketHeader) + (int)sizeof(KadHelloResponse)) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadHelloResponse* response = (KadHelloResponse*)(buffer + sizeof(KadPacketHeader));

    // Fill header
    header->messageType = KAD_MSG_RES;
    header->operationCode = KAD_OP_HELLO_RES;
    header->bodyLength = sizeof(KadHelloResponse);

    // Fill response body
    memcpy(response->targetId, targetId, KAD_ID_SIZE);
    response->tcpPort = tcpPort;
    response->udpPort = udpPort;
    response->version = 0x0100; // Version 1.0
    response->tagCount = 0;     // No tags for basic implementation

    return sizeof(KadPacketHeader) + sizeof(KadHelloResponse);
}

int KadProtocol::CreateFindNodeRequest(unsigned char* buffer, int bufferSize, const unsigned char* targetId) {
    if (bufferSize < (int)sizeof(KadPacketHeader) + (int)sizeof(KadSearchRequest)) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadSearchRequest* request = (KadSearchRequest*)(buffer + sizeof(KadPacketHeader));

    // Fill header
    header->messageType = KAD_MSG_REQ;
    header->operationCode = KAD_OP_SEARCH_REQ;
    header->bodyLength = sizeof(KadSearchRequest);

    // Fill request body
    memcpy(request->targetId, targetId, KAD_ID_SIZE);
    request->searchType = 0; // Node search

    return sizeof(KadPacketHeader) + sizeof(KadSearchRequest);
}

int KadProtocol::CreateFindNodeResponse(unsigned char* buffer, int bufferSize,
                                       const unsigned char* targetId, const std::list<KadNode*>& nodes) {
    int resultCount = (int)nodes.size();
    int bodySize = sizeof(KadSearchResponse) + resultCount * sizeof(KadSearchResult);

    if (bufferSize < (int)sizeof(KadPacketHeader) + bodySize) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadSearchResponse* response = (KadSearchResponse*)(buffer + sizeof(KadPacketHeader));
    KadSearchResult* results = (KadSearchResult*)(buffer + sizeof(KadPacketHeader) + sizeof(KadSearchResponse));

    // Fill header
    header->messageType = KAD_MSG_RES;
    header->operationCode = KAD_OP_SEARCH_RES;
    header->bodyLength = (unsigned char)bodySize;

    // Fill response body
    response->resultCount = (unsigned char)resultCount;

    // Fill result entries
    int i = 0;
    for (auto node : nodes) {
        if (i >= resultCount) break;

        memcpy(results[i].nodeId, node->id, KAD_ID_SIZE);
        results[i].ipAddress = node->addr.sin_addr.s_addr;
        results[i].tcpPort = node->addr.sin_port;
        results[i].udpPort = node->addr.sin_port; // Use same port for now

        i++;
    }

    return sizeof(KadPacketHeader) + bodySize;
}

int KadProtocol::CreatePublishRequest(unsigned char* buffer, int bufferSize,
                                     const unsigned char* targetId, const char* keyword) {
    if (!buffer || !targetId || !keyword) return -1;

    const size_t maxKeywordLen = 255;
    size_t keywordLen = 0;
    while (keywordLen < maxKeywordLen && keyword[keywordLen] != '\0') {
        keywordLen++;
    }
    if (keywordLen == maxKeywordLen) return -1; // Keyword too long or not null-terminated in range

    int bodySize = sizeof(KadPublishRequest) + (int)keywordLen + 1; // +1 for null terminator
    if (bodySize > 0xFF) return -1; // bodyLength is one byte in KadPacketHeader
    int packetSize = (int)sizeof(KadPacketHeader) + bodySize;

    if (bufferSize < packetSize) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadPublishRequest* request = (KadPublishRequest*)(buffer + sizeof(KadPacketHeader));
    char* keywordData = (char*)(buffer + sizeof(KadPacketHeader) + sizeof(KadPublishRequest));

    // Fill header
    header->messageType = KAD_MSG_REQ;
    header->operationCode = KAD_OP_PUBLISH_REQ;
    header->bodyLength = (unsigned char)bodySize;

    // Fill request body
    memcpy(request->targetId, targetId, KAD_ID_SIZE);
    request->load = 0; // Not used in basic implementation

    // Copy keyword
    memcpy(keywordData, keyword, keywordLen);
    keywordData[keywordLen] = '\0';

    return packetSize;
}

int KadProtocol::CreatePublishResponse(unsigned char* buffer, int bufferSize,
                                      const unsigned char* targetId, bool success) {
    int bodySize = sizeof(KadPublishResponse);

    if (bufferSize < (int)sizeof(KadPacketHeader) + bodySize) {
        return -1; // Buffer too small
    }

    KadPacketHeader* header = (KadPacketHeader*)buffer;
    KadPublishResponse* response = (KadPublishResponse*)(buffer + sizeof(KadPacketHeader));

    // Fill header
    header->messageType = KAD_MSG_RES;
    header->operationCode = KAD_OP_PUBLISH_RES;
    header->bodyLength = (unsigned char)bodySize;

    // Fill response body
    memcpy(response->targetId, targetId, KAD_ID_SIZE);
    response->load = success ? 0 : 1; // 0 = success, 1 = failed

    return sizeof(KadPacketHeader) + bodySize;
}

#endif // ENVY_LEGACY_KADEMLIA
