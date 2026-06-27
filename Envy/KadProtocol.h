//
// KadProtocol.h
//
// Kademlia protocol message definitions and packet handling
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

// Forward declaration
struct KadNode;

// Kademlia message types (eDonkey2000 Kad protocol)
#define KAD_MSG_REQ             0x00  // Request
#define KAD_MSG_RES             0x01  // Response

// Kademlia operation codes
#define KAD_OP_BOOTSTRAP        0x00  // Bootstrap request
#define KAD_OP_HELLO            0x10  // Hello/Ping
#define KAD_OP_HELLO_RES        0x18  // Hello/Pong response
#define KAD_OP_REQ              0x20  // Generic request
#define KAD_OP_SEARCH_REQ       0x21  // Search request
#define KAD_OP_SEARCH_RES       0x22  // Search response
#define KAD_OP_PUBLISH_REQ      0x30  // Publish request
#define KAD_OP_PUBLISH_RES      0x31  // Publish response
#define KAD_OP_FW_CHECK_REQ     0x50  // Firewall check request
#define KAD_OP_FW_CHECK_RES     0x51  // Firewall check response

// Kademlia packet header
#pragma pack(push, 1)
struct KadPacketHeader {
    unsigned char messageType;     // Message type (REQ/RES)
    unsigned char operationCode;   // Operation code
    unsigned char bodyLength;      // Body length (can be extended)
};
#pragma pack(pop)

// Bootstrap request (no body needed)

// Hello request body
#pragma pack(push, 1)
struct KadHelloRequest {
    KadId targetId;               // Target node ID
    unsigned short tcpPort;       // TCP port (network byte order)
    unsigned short udpPort;       // UDP port (network byte order)
    unsigned short version;       // Client version
};
#pragma pack(pop)

// Hello response body
#pragma pack(push, 1)
struct KadHelloResponse {
    KadId targetId;               // Our node ID
    unsigned short tcpPort;       // TCP port (network byte order)
    unsigned short udpPort;       // UDP port (network byte order)
    unsigned short version;       // Client version
    unsigned char tagCount;       // Number of tags
    // Followed by tags...
};
#pragma pack(pop)

// Search request body
#pragma pack(push, 1)
struct KadSearchRequest {
    KadId targetId;               // Target ID to search for
    unsigned char searchType;     // Search type (NODE, FILE, etc.)
};
#pragma pack(pop)

// Search response body
#pragma pack(push, 1)
struct KadSearchResponse {
    unsigned char resultCount;    // Number of results
    // Followed by result entries...
};
#pragma pack(pop)

// Result entry in search response
#pragma pack(push, 1)
struct KadSearchResult {
    KadId nodeId;                 // Node ID
    unsigned int ipAddress;       // IP address (network byte order)
    unsigned short tcpPort;       // TCP port (network byte order)
    unsigned short udpPort;       // UDP port (network byte order)
};
#pragma pack(pop)

// Publish request body (extended version for protocol messages)
#pragma pack(push, 1)
struct KadProtocolPublishRequest {
    KadId targetId;               // Target ID for publishing
    unsigned char keywordCount;   // Number of keywords
    // Followed by keywords and file info...
};
#pragma pack(pop)

// Maximum packet size
#define KAD_MAX_PACKET_SIZE 4096

// Packet buffer for receiving/sending
struct KadPacket {
    struct sockaddr_in fromAddr;          // Source address
    int fromAddrLen;                      // Address length
    KadPacketHeader header;               // Packet header
    unsigned char body[KAD_MAX_PACKET_SIZE - sizeof(KadPacketHeader)]; // Body data
    int bodyLength;                       // Actual body length
};

// KadProtocol class for handling network communication
class KadProtocol {
public:
    KadProtocol();
    ~KadProtocol();

    // Initialize with socket
    bool Initialize(int socketFd);

    // Send packets
    bool SendBootstrapRequest(const struct sockaddr_in& target);
    bool SendHelloRequest(const unsigned char* targetId, const struct sockaddr_in& target);
    bool SendHelloResponse(const unsigned char* targetId, const struct sockaddr_in& target);
    bool SendFindNodeRequest(const unsigned char* targetId, const struct sockaddr_in& target);
    bool SendFindNodeResponse(const unsigned char* targetId, const std::list<KadNode*>& nodes,
                             const struct sockaddr_in& target);
    bool SendPublishRequest(const unsigned char* targetId, const char* keyword,
                           const struct sockaddr_in& target);
    bool SendPublishResponse(const unsigned char* targetId, bool success,
                            const struct sockaddr_in& target);
    bool PublishKeyword(const unsigned char* targetId, const char* keyword);

    // Receive and process packets
    bool ReceivePacket();
    void ProcessPacket(const KadPacket& packet);

    // Packet creation helpers
    static int CreateHelloRequest(unsigned char* buffer, int bufferSize,
                                 const unsigned char* targetId, unsigned short tcpPort, unsigned short udpPort);
    static int CreateHelloResponse(unsigned char* buffer, int bufferSize,
                                  const unsigned char* targetId, unsigned short tcpPort, unsigned short udpPort);
    static int CreateFindNodeRequest(unsigned char* buffer, int bufferSize, const unsigned char* targetId);
    static     int CreateFindNodeResponse(unsigned char* buffer, int bufferSize,
                              const unsigned char* targetId, const std::list<KadNode*>& nodes);
    int CreatePublishRequest(unsigned char* buffer, int bufferSize,
                           const unsigned char* targetId, const char* keyword);
    int CreatePublishResponse(unsigned char* buffer, int bufferSize,
                            const unsigned char* targetId, bool success);

private:
    int m_socketFd;               // UDP socket file descriptor
    bool m_initialized;           // Whether protocol is initialized

    // Packet processing methods
    void ProcessHelloRequest(const KadPacket& packet);
    void ProcessHelloResponse(const KadPacket& packet);
    void ProcessFindNodeRequest(const KadPacket& packet);
    void ProcessFindNodeResponse(const KadPacket& packet);
    void ProcessPublishRequest(const KadPacket& packet);
    void ProcessPublishResponse(const KadPacket& packet);

    // Utility methods
    bool SendPacket(const unsigned char* data, int dataLength, const struct sockaddr_in& target);
    unsigned short GetTcpPort() const;
    unsigned short GetUdpPort() const;
};

// Global protocol instance
extern KadProtocol* g_kadProtocol;

#endif // ENVY_LEGACY_KADEMLIA
