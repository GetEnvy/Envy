//
// Kademlia.cpp
//
// Kad2 (Kademlia2) DHT implementation for eDonkey2000 network
// eMule-compatible Kademlia protocol implementation
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
#include "EDPacket.h"
#include "Datagrams.h"
#include "HostCache.h"
#include "Envy.h"
#include "GProfile.h"
#include "Settings.h"
#include <array>
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

// Global Kademlia instance
CKademlia Kademlia;

// Helper function to check if KadID is all zeros
static bool IsZeroId(const KadId& id) {
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        if (id[i] != 0) return false;
    }
    return true;
}

// KadBucket implementation
bool KadBucket::AddContact(const KadContact& contact) {
    // Use binary search for better performance (assuming contacts are sorted by ID)
    auto it = std::lower_bound(contacts.begin(), contacts.end(), contact,
        [](const KadContact& a, const KadContact& b) {
            return memcmp(a.id, b.id, KAD_ID_SIZE) < 0;
        });

    if (it != contacts.end() && memcmp(it->id, contact.id, KAD_ID_SIZE) == 0) {
        // Update existing contact
        *it = contact;
        it->lastSeen = GetTickCount();
        return true;
    }

    // Add new contact if bucket not full
    if (contacts.size() < (size_t)KAD_K) {
        contacts.insert(it, contact);
        return true;
    }

    return false; // Bucket full
}

bool KadBucket::RemoveContact(const KadId& id) {
    // Binary search for removal
    auto it = std::lower_bound(contacts.begin(), contacts.end(), id,
        [](const KadContact& contact, const KadId& targetId) {
            return memcmp(contact.id, targetId, KAD_ID_SIZE) < 0;
        });

    if (it != contacts.end() && memcmp(it->id, id, KAD_ID_SIZE) == 0) {
        contacts.erase(it);
        return true;
    }
    return false;
}

const KadContact* KadBucket::FindContact(const KadId& id) const {
    // Binary search for better performance
    auto it = std::lower_bound(contacts.begin(), contacts.end(), id,
        [](const KadContact& contact, const KadId& targetId) {
            return memcmp(contact.id, targetId, KAD_ID_SIZE) < 0;
        });

    if (it != contacts.end() && memcmp(it->id, id, KAD_ID_SIZE) == 0) {
        return &(*it);
    }
    return nullptr;
}

// Kad2RoutingTable implementation
Kad2RoutingTable::Kad2RoutingTable() {
    memset(ownId, 0, KAD_ID_SIZE);
}

Kad2RoutingTable::~Kad2RoutingTable() {
    // Nothing to clean up
}

bool Kad2RoutingTable::Initialize(const KadId& nodeId) {
    memcpy(ownId, nodeId, KAD_ID_SIZE);
    return true;
}

bool Kad2RoutingTable::AddContact(const KadContact& contact) {
    // Don't add ourselves
    if (memcmp(contact.id, ownId, KAD_ID_SIZE) == 0) {
        return false;
    }

    // Calculate bucket index based on XOR distance
    unsigned char distance[KAD_ID_SIZE];
    for (int i = 0; i < KAD_ID_SIZE; i++) {
        distance[i] = ownId[i] ^ contact.id[i];
    }

    // Find the bucket (highest bit set in distance)
    int bucketIndex = 0;
    for (int i = 0; i < KAD_ID_BITS; i++) {
        if (distance[i / 8] & (0x80 >> (i % 8))) {
            bucketIndex = KAD_ID_BITS - 1 - i;
            break;
        }
    }

    if (bucketIndex >= KAD_BUCKET_COUNT) {
        bucketIndex = KAD_BUCKET_COUNT - 1;
    }

    return buckets[bucketIndex].AddContact(contact);
}

bool Kad2RoutingTable::RemoveContact(const KadId& id) {
    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        if (buckets[i].RemoveContact(id)) {
            return true;
        }
    }
    return false;
}

const KadContact* Kad2RoutingTable::FindContact(const KadId& id) const {
    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        const KadContact* contact = buckets[i].FindContact(id);
        if (contact) {
            return contact;
        }
    }
    return nullptr;
}

void Kad2RoutingTable::MarkContactVerified(const KadId& id) {
    // Find the contact and mark it as verified
    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        for (auto& contact : buckets[i].contacts) {
            if (memcmp(contact.id, id, KAD_ID_SIZE) == 0) {
                contact.verified = TRUE;
                contact.lastSeen = GetTickCount();
                return;
            }
        }
    }
}

void Kad2RoutingTable::FindClosestContacts(const KadId& targetId, std::vector<KadContact>& results, int maxCount) {
    // Simple implementation: collect contacts from all buckets and sort by XOR distance
    std::vector<std::pair<std::array<unsigned char, KAD_ID_SIZE>, const KadContact*>> candidates;

    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        for (const auto& contact : buckets[i].contacts) {
            std::array<unsigned char, KAD_ID_SIZE> distance;
            for (int j = 0; j < KAD_ID_SIZE; j++) {
                distance[j] = targetId[j] ^ contact.id[j];
            }
            candidates.push_back(std::make_pair(distance, &contact));
        }
    }

    // Sort by XOR distance (lexicographical comparison)
    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<std::array<unsigned char, KAD_ID_SIZE>, const KadContact*>& a,
           const std::pair<std::array<unsigned char, KAD_ID_SIZE>, const KadContact*>& b) {
            return memcmp(a.first.data(), b.first.data(), KAD_ID_SIZE) < 0;
        });

    // Take the closest contacts
    for (size_t i = 0; i < candidates.size() && results.size() < (size_t)maxCount; i++) {
        results.push_back(*candidates[i].second);
    }
}

size_t Kad2RoutingTable::GetTotalContacts() const {
    size_t total = 0;
    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        total += buckets[i].GetContactCount();
    }
    return total;
}

void Kad2RoutingTable::GetContactsForBootstrap(std::vector<KadContact>& results, int maxCount) {
    for (int i = 0; i < KAD_BUCKET_COUNT; i++) {
        for (const auto& contact : buckets[i].contacts) {
            if (results.size() >= (size_t)maxCount) return;
            results.push_back(contact);
        }
    }
}

// CKademlia implementation
CKademlia::CKademlia() :
    m_bInitialized(false),
    m_lastBootstrapTime(0),
    m_lastTimerCall(0),
    m_lastRateLimitCleanup(0),
    m_lastStoreCleanup(0)
{
    memset(m_ownId, 0, KAD_ID_SIZE);
}

CKademlia::~CKademlia() {
    Stop();
}

bool CKademlia::Init() {
    if (m_bInitialized) return true;

    // Generate our own Kad ID (use MyProfile GUID as base for now)
    GenerateOwnKadId();

    // Initialize routing table
    if (!m_routingTable.Initialize(m_ownId)) {
        return false;
    }

    m_bInitialized = true;
    m_lastBootstrapTime = 0;
    m_lastTimerCall = GetTickCount();

    theApp.Message(MSG_NOTICE, L"Kad2 initialized with ID: %02x%02x%02x%02x...",
        m_ownId[0], m_ownId[1], m_ownId[2], m_ownId[3]);

    // Bootstrap immediately
    Bootstrap();

    return true;
}

void CKademlia::Stop() {
    if (!m_bInitialized) return;

    m_bInitialized = false;
    memset(m_ownId, 0, KAD_ID_SIZE);

    theApp.Message(MSG_NOTICE, L"Kad2 stopped");
}

void CKademlia::GenerateOwnKadId() {
    // Use MyProfile GUID as base for Kad ID
    Hashes::Guid oGUID = MyProfile.oGUID;
    memcpy(m_ownId, &oGUID[0], std::min(oGUID.byteCount, size_t(KAD_ID_SIZE)));

    // If GUID is shorter than 16 bytes, pad with cryptographically secure random data
    if (oGUID.byteCount < KAD_ID_SIZE) {
        // Fill remaining bytes with secure random data (P0.2 security requirement)
        size_t remainingBytes = KAD_ID_SIZE - oGUID.byteCount;
        if (!GenerateCryptographicBytes(&m_ownId[oGUID.byteCount], remainingBytes)) {
            // Critical security failure - cannot generate secure Kad ID
            theApp.Message(MSG_ERROR, L"Kademlia: Failed to generate secure random bytes for Kad ID");
            // Set remaining bytes to zero as fallback (not secure but better than rand())
            memset(&m_ownId[oGUID.byteCount], 0, remainingBytes);
        }
    }
}

void CKademlia::Bootstrap() {
    if (!m_bInitialized) return;

    DWORD now = GetTickCount();
    if (now - m_lastBootstrapTime < 30000) { // Don't bootstrap more than once every 30 seconds
        return;
    }

    m_lastBootstrapTime = now;

    // Get bootstrap contacts from host cache
    std::vector<KadContact> bootstrapContacts;
    int contactsFound = 0;

    // Import from host cache
    for (CHostCacheIterator it = HostCache.Kademlia.Begin(); it != HostCache.Kademlia.End() && contactsFound < 20; ++it) {
        CHostCacheHostPtr pHost = *it;
        if (pHost && pHost->m_pAddress.s_addr != INADDR_ANY) {
            KadContact contact;
            memcpy(contact.id, &pHost->m_oGUID, KAD_ID_SIZE);
            contact.ip = ntohl(pHost->m_pAddress.s_addr);  // Convert network order to host order
            contact.udpPort = pHost->m_nUDPPort ? pHost->m_nUDPPort : pHost->m_nPort;
            contact.tcpPort = pHost->m_nPort;
            contact.verified = FALSE;
            contact.version = pHost->m_nKADVersion;

            bootstrapContacts.push_back(contact);
            contactsFound++;
        }
    }

    if (bootstrapContacts.empty()) {
        theApp.Message(MSG_DEBUG, L"Kad2: No bootstrap contacts found in host cache");
        return;
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Bootstrapping with %d contacts", bootstrapContacts.size());

    // Send bootstrap requests to first few contacts
    int bootstrapRequestsSent = 0;
    int findNodeRequestsSent = 0;

    for (const auto& contact : bootstrapContacts) {
        if (bootstrapRequestsSent < 5) { // Bootstrap with up to 5 nodes initially
            SendBootstrapRequest(contact);
            bootstrapRequestsSent++;
        }

        // Also send some find node requests to build routing table
        if (findNodeRequestsSent < 3) {
            SendFindNodeRequest(contact);
            findNodeRequestsSent++;
        }
    }
}

void CKademlia::SendBootstrapRequest(const KadContact& contact) {
    if (!m_bInitialized) return;

    // Create bootstrap request packet - empty body as per eMule spec
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_BOOTSTRAP_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    // BOOTSTRAP_REQ has empty body according to eMule spec

    // Send packet and track the request
    sockaddr_in addr;
    contact.GetSockAddr(addr);
    DWORD requestId = AddOutstandingRequest(KAD_REQUEST_BOOTSTRAP, addr);

    theApp.Message(MSG_DEBUG, L"Kad2: Sent bootstrap request to %s (ID: %u)",
        (LPCTSTR)CString(inet_ntoa(addr.sin_addr)), requestId);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}

void CKademlia::SendFindNodeRequest(const KadContact& contact) {
    if (!m_bInitialized) return;

    // Create find node request: <Type(1)><TargetID(16)><ReceiverID(16)>
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    // Add search type (1 byte) - KADEMLIA_FIND_NODE for node search
    pPacket->WriteByte(KADEMLIA_FIND_NODE);

    // Add target ID (16 bytes) - use cryptographically secure random ID for node discovery (P0.2 security requirement)
    KadId targetId;
    if (!GenerateCryptographicBytes(targetId, KAD_ID_SIZE)) {
        // Critical security failure - cannot generate secure target ID
        theApp.Message(MSG_ERROR, L"Kademlia: Failed to generate secure random bytes for target ID");
        // Set to zero as fallback (not secure but prevents using rand())
        memset(targetId, 0, KAD_ID_SIZE);
    }
    pPacket->Write(targetId, KAD_ID_SIZE);

    // Add receiver ID (16 bytes) - target node's ID
    pPacket->Write(contact.id, KAD_ID_SIZE);

    // Send packet and track the request
    sockaddr_in addr;
    contact.GetSockAddr(addr);
    DWORD requestId = AddOutstandingRequest(KAD_REQUEST_FIND_NODE, addr);

    theApp.Message(MSG_DEBUG, L"Kad2: Sent find node request to %s (ID: %u)",
        (LPCTSTR)CString(inet_ntoa(addr.sin_addr)), requestId);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}

void CKademlia::OnTimer() {
    if (!m_bInitialized) return;

    DWORD now = GetTickCount();
    if (now - m_lastTimerCall < 5000) return; // Call at most every 5 seconds

    m_lastTimerCall = now;

    // Clean up expired requests
    CleanupExpiredRequests();

    // Periodic maintenance
    LogKadStatus();

    // Clean up expired DHT entries every 5 minutes
    if (now - m_lastStoreCleanup > 5 * 60 * 1000) {
        CleanupExpiredEntries();
        m_lastStoreCleanup = now;
    }

    // Re-bootstrap if we have very few contacts
    if (m_routingTable.GetTotalContacts() < 5) {
        Bootstrap();
    }
}

BOOL CKademlia::OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    if (!m_bInitialized || !pHost || !pPacket) {
        return FALSE;
    }

    // Route packet based on opcode
    switch (pPacket->m_nType) {
    case KADEMLIA2_BOOTSTRAP_REQ:
        OnBootstrapRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_BOOTSTRAP_RES:
        OnBootstrapResponse(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_PING:
        OnPing(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_PONG:
        OnPong(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_REQ:
        OnFindNodeRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_RES:
        OnFindNodeResponse(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_HELLO_REQ:
        OnHelloRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_HELLO_RES:
        OnHelloResponse(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_SEARCH_KEY_REQ:
        OnSearchKeyRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_SEARCH_SOURCE_REQ:
        OnSearchSourceRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_SEARCH_RES:
        OnSearchResponse(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_PUBLISH_KEY_REQ:
        OnPublishKeyRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_PUBLISH_SOURCE_REQ:
        OnPublishSourceRequest(pHost, pPacket);
        return TRUE;

    case KADEMLIA2_PUBLISH_RES:
        OnPublishResponse(pHost, pPacket);
        return TRUE;

    default:
        theApp.Message(MSG_DEBUG, L"Kad2: Unknown opcode 0x%02x from %s",
            pPacket->m_nType, (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return FALSE;
    }
}

void CKademlia::OnBootstrapRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // BOOTSTRAP_REQ should have empty body, but we'll accept it anyway
    // and extract the sender info from the packet source

    theApp.Message(MSG_DEBUG, L"Kad2: Bootstrap request from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // Create bootstrap response: <MyKadID(16)><TCPPort(2)><KadVersion(1)><Count(2)><contacts...>
    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_BOOTSTRAP_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    // Add our node ID (16 bytes)
    pResponse->Write(m_ownId, KAD_ID_SIZE);

    // Add our TCP port (2 bytes) - use UDP port as TCP port for now
    WORD tcpPort = Settings.Connection.InPort;
    if (tcpPort == 0) tcpPort = 4672; // Default Kad port
    pResponse->WriteShortLE(tcpPort);

    // Add Kad version (1 byte)
    BYTE kadVersion = 8; // eMule Kad version
    pResponse->WriteByte(kadVersion);

    // Get closest contacts (up to 10)
    KadId zeroId = {0}; // Use zero ID to get any contacts for bootstrap
    std::vector<KadContact> closestContacts;
    m_routingTable.FindClosestContacts(zeroId, closestContacts, 10);

    // Add contact count (2 bytes)
    WORD contactCount = (WORD)closestContacts.size();
    pResponse->WriteShortLE(contactCount);

    // Add contacts: each <ID(16)><IP(4)><UDP(2)><TCP(2)><Ver(1)>
    for (const auto& contact : closestContacts) {
        pResponse->Write(contact.id, KAD_ID_SIZE);        // Node ID (16)
        pResponse->WriteLongLE(contact.ip);  // Write IP in host order LE as per eMule format
        pResponse->WriteShortLE(contact.udpPort);         // UDP Port (2)
        pResponse->WriteShortLE(contact.tcpPort);         // TCP Port (2)
        pResponse->WriteByte(contact.version);            // Version (1)
    }

    // Send response
    SendPacket(pHost, pResponse);
    pResponse->Release();

    // Don't add requester to routing table - we don't have their ID from empty request
}

void CKademlia::OnBootstrapResponse(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // Check if this response matches an outstanding request
    if (!IsRequestOutstanding(0, KAD_REQUEST_BOOTSTRAP, *pHost)) {
        theApp.Message(MSG_DEBUG, L"Kad2: Ignoring unsolicited bootstrap response from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return;
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Bootstrap response from %s (accepted)",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // Minimum size check: MyKadID(16) + TCPPort(2) + KadVersion(1) + Count(2)
    if (pPacket->GetRemaining() < (KAD_ID_SIZE + 2 + 1 + 2)) {
        theApp.Message(MSG_DEBUG, L"Kad2: Bootstrap response too small");
        return;
    }

    // Read responder's node ID (16 bytes)
    if (pPacket->GetRemaining() < KAD_ID_SIZE) return;
    KadId responderId;
    pPacket->Read(responderId, KAD_ID_SIZE);

    // Read responder's TCP port (2 bytes)
    WORD responderTcpPort = pPacket->ReadShortLE();

    // Read Kad version (1 byte)
    BYTE responderKadVersion = pPacket->ReadByte();

    // Read contact count (2 bytes)
    WORD contactCount = pPacket->ReadShortLE();

    // Sanity check on contact count
    if (contactCount > 100) {
        theApp.Message(MSG_DEBUG, L"Kad2: Bootstrap response has too many contacts (%d)", contactCount);
        return;
    }

    // Read contacts: each <ID(16)><IP(4)><UDP(2)><TCP(2)><Ver(1)>
    int contactsAdded = 0;
    for (WORD i = 0; i < contactCount; i++) {
        if (pPacket->GetRemaining() < (KAD_ID_SIZE + 4 + 2 + 2 + 1)) break;

        KadContact contact;
        if (pPacket->GetRemaining() < KAD_ID_SIZE) break;
        pPacket->Read(contact.id, KAD_ID_SIZE);
        contact.ip = pPacket->ReadLongLE();  // eMule stores IP in host order LE in payload
        contact.udpPort = pPacket->ReadShortLE();
        contact.tcpPort = pPacket->ReadShortLE();
        contact.version = pPacket->ReadByte();
        contact.verified = FALSE;

        // Add to routing table
        if (UpdateContact(contact)) {
            contactsAdded++;
        }
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Bootstrap response added %d contacts", contactsAdded);

    // Add responder to routing table
    KadContact responderContact(responderId, ntohl(pHost->sin_addr.s_addr),  // Convert to host order
                               ntohs(pHost->sin_port), responderTcpPort);
    responderContact.version = responderKadVersion;
    UpdateContact(responderContact);
}

void CKademlia::OnPing(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    theApp.Message(MSG_DEBUG, L"Kad2: Ping from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // Send pong response - eMule format: 2 bytes (UDP port observed)
    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_PONG, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    // Add the observed UDP port (2 bytes) - eMule PONG contains the port the peer sees us on
    WORD observedPort = ntohs(pHost->sin_port);
    pResponse->WriteShortLE(observedPort);

    SendPacket(pHost, pResponse);
    pResponse->Release();
}

void CKademlia::OnPong(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // eMule PONG format: 2 bytes (observed UDP port) + optional tags
    if (pPacket->GetRemaining() < 2) {
        theApp.Message(MSG_DEBUG, L"Kad2: Pong too small from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return;
    }

    // Read observed UDP port (2 bytes) - the port the responder thinks we have
    WORD observedPort = pPacket->ReadShortLE();

    theApp.Message(MSG_DEBUG, L"Kad2: Pong from %s (observed port: %d)",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)), observedPort);

    // Don't add contact - we don't have the responder's ID in PONG
}

void CKademlia::OnFindNodeRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    if (!pHost || !pPacket) {
        theApp.Message(MSG_ERROR, L"Kad2: Invalid parameters in find node request");
        return;
    }

    // KADEMLIA2_REQ format: <Type(1)><TargetID(16)><ReceiverID(16)>
    // Minimum size check
    if (pPacket->GetRemaining() < (1 + KAD_ID_SIZE + KAD_ID_SIZE)) {
        theApp.Message(MSG_DEBUG, L"Kad2: Find node request too small from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return;
    }

    try {
        // Read search type (1 byte)
        BYTE searchType = pPacket->ReadByte();
        BYTE type = (searchType & 0x1F);

        // Security: Only allow known search types to prevent protocol abuse
        if (type == 0 || (type != KADEMLIA_FIND_NODE && type != KADEMLIA_FIND_VALUE)) {
            theApp.Message(MSG_WARNING, L"Kad2: Rejected find node request with unknown search type %d from %s",
                searchType, (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
            // Don't respond to prevent amplification attacks
            return;
        }

        // Security: Rate limiting check
        if (!CheckRateLimit(pHost, KAD_REQUEST_FIND_NODE)) {
            theApp.Message(MSG_WARNING, L"Kad2: Rate limit exceeded for find node request from %s",
                (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
            return;
        }

    // Read target ID (16 bytes) - this is the ID we're looking for
    if (pPacket->GetRemaining() < KAD_ID_SIZE) return;
    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);

    // Read receiver ID (16 bytes) - this should be our own ID or a broadcast
    if (pPacket->GetRemaining() < KAD_ID_SIZE) return;
    KadId receiverId;
    pPacket->Read(receiverId, KAD_ID_SIZE);

    // eMule expects the receiver ID to match our own KadID
    if (memcmp(receiverId, m_ownId, KAD_ID_SIZE) != 0) {
        theApp.Message(MSG_DEBUG, L"Kad2: Find node request not for us (receiver mismatch) from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return;
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Find node request for target from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // Create response: KADEMLIA2_RES format: <TargetID(16)><Count(1)><contacts...>
    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    // Add target ID (16 bytes)
    pResponse->Write(targetId, KAD_ID_SIZE);

    // Get closest contacts to target
    std::vector<KadContact> closestContacts;
    m_routingTable.FindClosestContacts(targetId, closestContacts, KAD_K);

    // Add contact count (1 byte)
    BYTE contactCount = (BYTE)min(closestContacts.size(), (size_t)255);
    pResponse->WriteByte(contactCount);

    // Add contacts: each <ID(16)><IP(4)><UDP(2)><TCP(2)><Ver(1)>
    for (size_t i = 0; i < contactCount; i++) {
        const auto& contact = closestContacts[i];
        pResponse->Write(contact.id, KAD_ID_SIZE);        // Node ID (16)
        pResponse->WriteLongLE(contact.ip);  // Write IP in host order LE as per eMule format
        pResponse->WriteShortLE(contact.udpPort);         // UDP Port (2)
        pResponse->WriteShortLE(contact.tcpPort);         // TCP Port (2)
        pResponse->WriteByte(contact.version);            // Version (1)
    }

    // Send response
    SendPacket(pHost, pResponse);
    pResponse->Release();

    // Don't add requester to routing table - we don't know their ID from this packet format
    }
    catch (...) {
        // Handle any exceptions during packet processing
        theApp.Message(MSG_WARNING, L"Kad2: Exception during find node request processing from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
    }
}

void CKademlia::OnFindNodeResponse(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // Check if this response matches an outstanding request
    if (!IsRequestOutstanding(0, KAD_REQUEST_FIND_NODE, *pHost)) {
        theApp.Message(MSG_DEBUG, L"Kad2: Ignoring unsolicited find node response from %s",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));
        return;
    }

    // KADEMLIA2_RES format: <TargetID(16)><Count(1)><contacts...>
    // Minimum size check: TargetID(16) + Count(1)
    if (pPacket->GetRemaining() < (KAD_ID_SIZE + 1)) {
        theApp.Message(MSG_DEBUG, L"Kad2: Find node response too small");
        return;
    }

    // Read target ID (16 bytes)
    if (pPacket->GetRemaining() < KAD_ID_SIZE) return;
    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);

    // Read contact count (1 byte)
    BYTE contactCount = pPacket->ReadByte();

    theApp.Message(MSG_DEBUG, L"Kad2: Find node response from %s with %d contacts (accepted)",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)), contactCount);

    // Read contacts: each <ID(16)><IP(4)><UDP(2)><TCP(2)><Ver(1)>
    int contactsAdded = 0;
    for (BYTE i = 0; i < contactCount; i++) {
        if (pPacket->GetRemaining() < (KAD_ID_SIZE + 4 + 2 + 2 + 1)) break;

        KadContact contact;
        if (pPacket->GetRemaining() < KAD_ID_SIZE) break;
        pPacket->Read(contact.id, KAD_ID_SIZE);
        contact.ip = pPacket->ReadLongLE();  // eMule stores IP in host order LE in payload
        contact.udpPort = pPacket->ReadShortLE();
        contact.tcpPort = pPacket->ReadShortLE();
        contact.version = pPacket->ReadByte();
        contact.verified = FALSE;

        // Add to routing table
        if (UpdateContact(contact)) {
            contactsAdded++;
        }
    }

    if (contactsAdded > 0) {
        theApp.Message(MSG_DEBUG, L"Kad2: Find node response added %d contacts", contactsAdded);
    }

    // Don't add responder to routing table - we don't know their ID from this packet format
}

void CKademlia::SendPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    if (!pHost || !pPacket) return;

    Datagrams.Send(pHost, pPacket, FALSE);
}

bool CKademlia::UpdateContact(const KadContact& contact) {
    // Don't add ourselves
    if (memcmp(contact.id, m_ownId, KAD_ID_SIZE) == 0) {
        return false;
    }

    // Don't add unknown/invalid node IDs (all-zero)
    if (IsZeroId(contact.id)) {
        return false;
    }

    // Don't add invalid IPs
    if (contact.ip == 0 || contact.ip == INADDR_NONE || contact.ip == INADDR_ANY) {
        return false;
    }

    // Don't add loopback/private IPs for Kad (unless in LAN mode)
    if (!Settings.Experimental.LAN_Mode) {
        if ((contact.ip & 0xFF000000) == 0x7F000000 ||  // 127.x.x.x
            (contact.ip & 0xFF000000) == 0x0A000000 ||  // 10.x.x.x
            (contact.ip & 0xFFF00000) == 0xAC100000 ||  // 172.16.x.x - 172.31.x.x
            (contact.ip & 0xFFFF0000) == 0xC0A80000) {  // 192.168.x.x
            return false;
        }
    }

    return m_routingTable.AddContact(contact);
}

void CKademlia::MarkContactVerified(const KadId& id) {
    // Mark a contact as verified in the routing table
    m_routingTable.MarkContactVerified(id);
}

void CKademlia::LogKadStatus() {
    size_t contactCount = m_routingTable.GetTotalContacts();
    theApp.Message(MSG_DEBUG, L"Kad2: Routing table has %d contacts", contactCount);
}

void CKademlia::OnHelloRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    theApp.Message(MSG_DEBUG, L"Kad2: Hello request from %s:%d",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)), ntohs(pHost->sin_port));

    // HELLO_REQ format: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)>
    // We need to read the TargetID and respond with our info if we're the target
    // or forward to the appropriate node

    if (pPacket->GetRemaining() < KAD_ID_SIZE + 2 + 1 + 2) {
        theApp.Message(MSG_WARNING, L"Kad2: Hello request packet too small from %s - expected %d bytes, got %d",
            (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)),
            KAD_ID_SIZE + 2 + 1 + 2, pPacket->GetRemaining());
        return;
    }

    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);
    WORD tcpPort = pPacket->ReadShortLE();
    BYTE version = pPacket->ReadByte();
    WORD udpPort = pPacket->ReadShortLE();

    // Check if this is for us
    if (memcmp(targetId, m_ownId, KAD_ID_SIZE) == 0) {
        // This is for us - send HELLO_RES
        SendHelloResponse(pHost);
    } else {
        // Forward to appropriate node (simplified - just drop for now)
        theApp.Message(MSG_DEBUG, L"Kad2: Hello request not for us - dropping");
    }
}

void CKademlia::OnHelloResponse(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    theApp.Message(MSG_DEBUG, L"Kad2: Hello response from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // HELLO_RES format: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)><TagCount(1)><Tags...>
    // Extract and store contact information

    if (pPacket->GetRemaining() < KAD_ID_SIZE + 2 + 1 + 2 + 1) {
        theApp.Message(MSG_DEBUG, L"Kad2: Hello response packet too small");
        return;
    }

    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);
    WORD tcpPort = pPacket->ReadShortLE();
    BYTE version = pPacket->ReadByte();
    WORD udpPort = pPacket->ReadShortLE();
    BYTE tagCount = pPacket->ReadByte();

    // Store/update contact information
    KadContact contact;
    memcpy(contact.id, targetId, KAD_ID_SIZE);
    contact.ip = pHost->sin_addr.s_addr;
    contact.tcpPort = tcpPort;
    contact.udpPort = udpPort;
    contact.lastSeen = GetTickCount();

    // Add to our routing table and mark as verified (successful hello response)
    if (UpdateContact(contact)) {
        // Mark contact as verified since we received a valid hello response
        MarkContactVerified(contact.id);
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Added/updated contact from hello response");
}

void CKademlia::SendHelloRequest(const SOCKADDR_IN* pTarget) {
    theApp.Message(MSG_DEBUG, L"Kad2: Sending hello request to %s",
        (LPCTSTR)CString(inet_ntoa(pTarget->sin_addr)));

    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_HELLO_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    // HELLO_REQ format: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)>
    // TargetID should be the ID of the node we're contacting
    // For now, use a zero ID to request general contact
    KadId zeroId = {0};
    pPacket->Write(zeroId, KAD_ID_SIZE);

    // Add our TCP port
    pPacket->WriteShortLE(4662); // Default eMule TCP port

    // Add Kad version
    pPacket->WriteByte(KADEMLIA_VERSION);

    // Add our UDP port
    pPacket->WriteShortLE(4672); // Default eMule UDP port

    SendPacket(pTarget, pPacket);
}

void CKademlia::SendHelloResponse(const SOCKADDR_IN* pTarget) {
    theApp.Message(MSG_DEBUG, L"Kad2: Sending hello response to %s",
        (LPCTSTR)CString(inet_ntoa(pTarget->sin_addr)));

    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_HELLO_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    // HELLO_RES format: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)><TagCount(1)><Tags...>
    pResponse->Write(m_ownId, KAD_ID_SIZE);
    pResponse->WriteShortLE(4662); // Our TCP port
    pResponse->WriteByte(KADEMLIA_VERSION); // Kad version
    pResponse->WriteShortLE(4672); // Our UDP port

    // Tag count (0 for now - no additional tags)
    pResponse->WriteByte(0);

    SendPacket(pTarget, pResponse);
}

// Security and rate limiting implementation
bool CKademlia::CheckRateLimit(const SOCKADDR_IN* pHost, KadRequestType type) {
    if (!pHost) return false;

    DWORD currentTime = GetTickCount();
    DWORD clientIP = pHost->sin_addr.s_addr;

    // Clean up old entries periodically (every 5 minutes)
    if (currentTime - m_lastRateLimitCleanup > 5 * 60 * 1000) {
        CleanupRateLimitMap();
        m_lastRateLimitCleanup = currentTime;
    }

    auto it = m_rateLimitMap.find(clientIP);
    if (it != m_rateLimitMap.end()) {
        DWORD lastRequestTime = it->second;
        // Allow max 10 requests per minute per IP
        if (currentTime - lastRequestTime < 6000) { // 6 seconds = 10 requests per minute
            return false;
        }
    }

    // Update last request time
    m_rateLimitMap[clientIP] = currentTime;
    return true;
}

void CKademlia::CleanupRateLimitMap() {
    DWORD currentTime = GetTickCount();
    // Remove entries older than 10 minutes
    for (auto it = m_rateLimitMap.begin(); it != m_rateLimitMap.end(); ) {
        if (currentTime - it->second > 10 * 60 * 1000) {
            it = m_rateLimitMap.erase(it);
        } else {
            ++it;
        }
    }
}

// Request tracking implementation
DWORD CKademlia::AddOutstandingRequest(KadRequestType type, const SOCKADDR_IN& targetAddr) {
    // Simple request ID generation - use a counter for now
    static DWORD nextRequestId = 1;
    DWORD requestId = nextRequestId++;

    // Clean up expired requests first
    CleanupExpiredRequests();

    // Don't allow too many outstanding requests
    if (m_outstandingRequests.size() >= KAD2_MAX_OUTSTANDING_REQUESTS) {
        // Remove oldest request
        auto oldest = m_outstandingRequests.begin();
        for (auto it = m_outstandingRequests.begin(); it != m_outstandingRequests.end(); ++it) {
            if (it->second.sentTime < oldest->second.sentTime) {
                oldest = it;
            }
        }
        m_outstandingRequests.erase(oldest);
    }

    m_outstandingRequests[requestId] = KadOutstandingRequest(type, targetAddr);
    return requestId;
}

bool CKademlia::IsRequestOutstanding(DWORD requestId, KadRequestType expectedType, const SOCKADDR_IN& fromAddr) {
    // For now, we accept any response from the same IP as valid
    // In a full implementation, we'd track specific request IDs

    auto it = m_outstandingRequests.begin();
    while (it != m_outstandingRequests.end()) {
        if (it->second.targetAddr.sin_addr.s_addr == fromAddr.sin_addr.s_addr &&
            it->second.targetAddr.sin_port == fromAddr.sin_port &&
            it->second.type == expectedType) {
            // Found matching request, remove it
            m_outstandingRequests.erase(it);
            return true;
        }
        ++it;
    }

    return false;
}

void CKademlia::RemoveOutstandingRequest(DWORD requestId) {
    m_outstandingRequests.erase(requestId);
}

void CKademlia::CleanupExpiredRequests() {
    DWORD now = GetTickCount();
    auto it = m_outstandingRequests.begin();

    while (it != m_outstandingRequests.end()) {
        if (now - it->second.sentTime > KAD2_REQUEST_TIMEOUT) {
            it = m_outstandingRequests.erase(it);
        } else {
            ++it;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// DHT Storage

static KadIdKey ToKey(const KadId& id) {
    KadIdKey key;
    memcpy(key.data(), id, KAD_ID_SIZE);
    return key;
}

bool CKademlia::StoreEntry(const KadIdKey& key, const KadStoredEntry& entry) {
    // Enforce global limit
    size_t total = 0;
    for (const auto& bucket : m_keywordStore)
        total += bucket.second.size();
    for (const auto& bucket : m_sourceStore)
        total += bucket.second.size();
    if (total >= KAD_STORE_MAX_TOTAL)
        return false;

    return true;
}

void CKademlia::CleanupExpiredEntries() {
    DWORD now = GetTickCount();
    auto cleanup = [now](KadStore& store) {
        for (auto it = store.begin(); it != store.end(); ) {
            auto& entries = it->second;
            entries.erase(
                std::remove_if(entries.begin(), entries.end(),
                    [now](const KadStoredEntry& e) { return now > e.lifetime; }),
                entries.end());
            if (entries.empty())
                it = store.erase(it);
            else
                ++it;
        }
    };
    cleanup(m_keywordStore);
    cleanup(m_sourceStore);
}

size_t CKademlia::GetStoredEntryCount() const {
    size_t total = 0;
    for (const auto& bucket : m_keywordStore)
        total += bucket.second.size();
    for (const auto& bucket : m_sourceStore)
        total += bucket.second.size();
    return total;
}

void CKademlia::WriteEntryTags(CEDPacket* pPacket, const KadStoredEntry& entry) {
    pPacket->Write(entry.sourceId, KAD_ID_SIZE);
    pPacket->WriteLongLE(entry.ip);
    pPacket->WriteShortLE(entry.udpPort);
    pPacket->WriteShortLE(entry.tcpPort);
    // Write tag count + tags
    pPacket->WriteByte((BYTE)entry.tags.size());
    for (const auto& tag : entry.tags) {
        pPacket->WriteByte(tag.first);
        pPacket->WriteShortLE((WORD)tag.second.size());
        if (!tag.second.empty())
            pPacket->Write(tag.second.data(), tag.second.size());
    }
}

bool CKademlia::ReadEntryTags(CEDPacket* pPacket, KadStoredEntry& entry) {
    if (pPacket->GetRemaining() < KAD_ID_SIZE + 4 + 2 + 2 + 1)
        return false;

    pPacket->Read(entry.sourceId, KAD_ID_SIZE);
    entry.ip = pPacket->ReadLongLE();
    entry.udpPort = pPacket->ReadShortLE();
    entry.tcpPort = pPacket->ReadShortLE();
    entry.lifetime = GetTickCount() + KAD_STORE_ENTRY_LIFETIME;

    BYTE tagCount = pPacket->ReadByte();
    if (tagCount > 32) return false;

    for (BYTE t = 0; t < tagCount; t++) {
        if (pPacket->GetRemaining() < 3) return false;
        BYTE tagId = pPacket->ReadByte();
        WORD tagLen = pPacket->ReadShortLE();
        if (pPacket->GetRemaining() < tagLen) return false;

        std::vector<BYTE> tagData(tagLen);
        if (tagLen > 0)
            pPacket->Read(tagData.data(), tagLen);
        entry.tags.push_back(std::make_pair(tagId, std::move(tagData)));
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Search handlers

void CKademlia::OnSearchKeyRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_SEARCH_KEY_REQ: <TargetID 16><StartPos 1 or 2>
    if (pPacket->GetRemaining() < KAD_ID_SIZE)
        return;

    if (!CheckRateLimit(pHost, KAD_REQUEST_SEARCH_KEY))
        return;

    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);

    theApp.Message(MSG_DEBUG, L"Kad2: Search key request from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    // Look up in keyword store
    KadIdKey key = ToKey(targetId);
    auto it = m_keywordStore.find(key);

    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_SEARCH_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    pResponse->Write(targetId, KAD_ID_SIZE);

    BYTE count = 0;
    if (it != m_keywordStore.end()) {
        count = (BYTE)min(it->second.size(), (size_t)255);
    }
    pResponse->WriteByte(count);

    if (it != m_keywordStore.end()) {
        for (size_t i = 0; i < count; i++)
            WriteEntryTags(pResponse, it->second[i]);
    }

    SendPacket(pHost, pResponse);
    pResponse->Release();
}

void CKademlia::OnSearchSourceRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_SEARCH_SOURCE_REQ: <FileHash 16><FileSize 8>
    if (pPacket->GetRemaining() < KAD_ID_SIZE)
        return;

    if (!CheckRateLimit(pHost, KAD_REQUEST_SEARCH_SOURCE))
        return;

    KadId fileHash;
    pPacket->Read(fileHash, KAD_ID_SIZE);

    theApp.Message(MSG_DEBUG, L"Kad2: Search source request from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    KadIdKey key = ToKey(fileHash);
    auto it = m_sourceStore.find(key);

    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_SEARCH_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    pResponse->Write(fileHash, KAD_ID_SIZE);

    BYTE count = 0;
    if (it != m_sourceStore.end()) {
        count = (BYTE)min(it->second.size(), (size_t)255);
    }
    pResponse->WriteByte(count);

    if (it != m_sourceStore.end()) {
        for (size_t i = 0; i < count; i++)
            WriteEntryTags(pResponse, it->second[i]);
    }

    SendPacket(pHost, pResponse);
    pResponse->Release();
}

void CKademlia::OnSearchResponse(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_SEARCH_RES: <TargetID 16><Count 1>[entries...]
    if (pPacket->GetRemaining() < KAD_ID_SIZE + 1)
        return;

    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);
    BYTE count = pPacket->ReadByte();

    theApp.Message(MSG_DEBUG, L"Kad2: Search response from %s with %d results",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)), count);

    for (BYTE i = 0; i < count; i++) {
        KadStoredEntry entry;
        if (!ReadEntryTags(pPacket, entry))
            break;

        // TODO: Deliver results to download/search system via callback
        theApp.Message(MSG_DEBUG, L"Kad2: Search result - source IP %u.%u.%u.%u:%d",
            (entry.ip >> 24) & 0xFF, (entry.ip >> 16) & 0xFF,
            (entry.ip >> 8) & 0xFF, entry.ip & 0xFF, entry.udpPort);
    }
}

//////////////////////////////////////////////////////////////////////
// Publish handlers

void CKademlia::OnPublishKeyRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_PUBLISH_KEY_REQ: <KeywordHash 16><PublisherID 16><TagList>
    if (pPacket->GetRemaining() < KAD_ID_SIZE)
        return;

    if (!CheckRateLimit(pHost, KAD_REQUEST_PUBLISH_KEY))
        return;

    KadId keywordHash;
    pPacket->Read(keywordHash, KAD_ID_SIZE);

    KadStoredEntry entry;
    if (!ReadEntryTags(pPacket, entry))
        return;

    theApp.Message(MSG_DEBUG, L"Kad2: Publish key request from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    KadIdKey key = ToKey(keywordHash);
    auto& entries = m_keywordStore[key];

    BYTE load = 0;  // 0 = success

    if (entries.size() < KAD_STORE_MAX_ENTRIES_PER_KEY && GetStoredEntryCount() < KAD_STORE_MAX_TOTAL) {
        // Check for duplicate by sourceId
        bool found = false;
        for (auto& existing : entries) {
            if (memcmp(existing.sourceId, entry.sourceId, KAD_ID_SIZE) == 0) {
                existing = entry;
                found = true;
                break;
            }
        }
        if (!found)
            entries.push_back(entry);
    } else {
        load = 100;  // Overloaded
    }

    // Send PUBLISH_RES
    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_PUBLISH_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    pResponse->Write(keywordHash, KAD_ID_SIZE);
    pResponse->WriteByte(load);

    SendPacket(pHost, pResponse);
    pResponse->Release();
}

void CKademlia::OnPublishSourceRequest(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_PUBLISH_SOURCE_REQ: <FileHash 16><PublisherID 16><TagList>
    if (pPacket->GetRemaining() < KAD_ID_SIZE)
        return;

    if (!CheckRateLimit(pHost, KAD_REQUEST_PUBLISH_SOURCE))
        return;

    KadId fileHash;
    pPacket->Read(fileHash, KAD_ID_SIZE);

    KadStoredEntry entry;
    if (!ReadEntryTags(pPacket, entry))
        return;

    theApp.Message(MSG_DEBUG, L"Kad2: Publish source request from %s",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)));

    KadIdKey key = ToKey(fileHash);
    auto& entries = m_sourceStore[key];

    BYTE load = 0;

    if (entries.size() < KAD_STORE_MAX_ENTRIES_PER_KEY && GetStoredEntryCount() < KAD_STORE_MAX_TOTAL) {
        bool found = false;
        for (auto& existing : entries) {
            if (memcmp(existing.sourceId, entry.sourceId, KAD_ID_SIZE) == 0) {
                existing = entry;
                found = true;
                break;
            }
        }
        if (!found)
            entries.push_back(entry);
    } else {
        load = 100;
    }

    CEDPacket* pResponse = CEDPacket::New(KADEMLIA2_PUBLISH_RES, ED2K_PROTOCOL_KAD);
    if (!pResponse) return;

    pResponse->Write(fileHash, KAD_ID_SIZE);
    pResponse->WriteByte(load);

    SendPacket(pHost, pResponse);
    pResponse->Release();
}

void CKademlia::OnPublishResponse(const SOCKADDR_IN* pHost, CEDPacket* pPacket) {
    // KADEMLIA2_PUBLISH_RES: <TargetID 16><Load 1>
    if (pPacket->GetRemaining() < KAD_ID_SIZE + 1)
        return;

    KadId targetId;
    pPacket->Read(targetId, KAD_ID_SIZE);
    BYTE load = pPacket->ReadByte();

    theApp.Message(MSG_DEBUG, L"Kad2: Publish response from %s (load: %d)",
        (LPCTSTR)CString(inet_ntoa(pHost->sin_addr)), load);
}

//////////////////////////////////////////////////////////////////////
// Search/Publish initiation (send to closest contacts)

void CKademlia::SearchKeyword(const KadId& keywordHash) {
    if (!m_bInitialized) return;

    std::vector<KadContact> closest;
    m_routingTable.FindClosestContacts(keywordHash, closest, KAD_K);

    if (closest.empty()) {
        theApp.Message(MSG_DEBUG, L"Kad2: No contacts for keyword search");
        return;
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Starting keyword search, querying %d contacts", closest.size());

    for (const auto& contact : closest)
        SendSearchKeyRequest(contact, keywordHash);
}

void CKademlia::SearchSource(const KadId& fileHash) {
    if (!m_bInitialized) return;

    std::vector<KadContact> closest;
    m_routingTable.FindClosestContacts(fileHash, closest, KAD_K);

    if (closest.empty()) {
        theApp.Message(MSG_DEBUG, L"Kad2: No contacts for source search");
        return;
    }

    theApp.Message(MSG_DEBUG, L"Kad2: Starting source search, querying %d contacts", closest.size());

    for (const auto& contact : closest)
        SendSearchSourceRequest(contact, fileHash);
}

void CKademlia::PublishKeyword(const KadId& keywordHash, const KadStoredEntry& entry) {
    if (!m_bInitialized) return;

    std::vector<KadContact> closest;
    m_routingTable.FindClosestContacts(keywordHash, closest, KAD_K);

    theApp.Message(MSG_DEBUG, L"Kad2: Publishing keyword to %d contacts", closest.size());

    for (const auto& contact : closest)
        SendPublishKeyRequest(contact, keywordHash, entry);
}

void CKademlia::PublishSource(const KadId& fileHash, const KadStoredEntry& entry) {
    if (!m_bInitialized) return;

    std::vector<KadContact> closest;
    m_routingTable.FindClosestContacts(fileHash, closest, KAD_K);

    theApp.Message(MSG_DEBUG, L"Kad2: Publishing source to %d contacts", closest.size());

    for (const auto& contact : closest)
        SendPublishSourceRequest(contact, fileHash, entry);
}

//////////////////////////////////////////////////////////////////////
// Send search/publish packets

void CKademlia::SendSearchKeyRequest(const KadContact& contact, const KadId& targetId) {
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_SEARCH_KEY_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    pPacket->Write(targetId, KAD_ID_SIZE);

    sockaddr_in addr;
    contact.GetSockAddr(addr);
    AddOutstandingRequest(KAD_REQUEST_SEARCH_KEY, addr);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}

void CKademlia::SendSearchSourceRequest(const KadContact& contact, const KadId& targetId) {
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_SEARCH_SOURCE_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    pPacket->Write(targetId, KAD_ID_SIZE);

    sockaddr_in addr;
    contact.GetSockAddr(addr);
    AddOutstandingRequest(KAD_REQUEST_SEARCH_SOURCE, addr);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}

void CKademlia::SendPublishKeyRequest(const KadContact& contact, const KadId& targetId, const KadStoredEntry& entry) {
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_PUBLISH_KEY_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    pPacket->Write(targetId, KAD_ID_SIZE);
    WriteEntryTags(pPacket, entry);

    sockaddr_in addr;
    contact.GetSockAddr(addr);
    AddOutstandingRequest(KAD_REQUEST_PUBLISH_KEY, addr);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}

void CKademlia::SendPublishSourceRequest(const KadContact& contact, const KadId& targetId, const KadStoredEntry& entry) {
    CEDPacket* pPacket = CEDPacket::New(KADEMLIA2_PUBLISH_SOURCE_REQ, ED2K_PROTOCOL_KAD);
    if (!pPacket) return;

    pPacket->Write(targetId, KAD_ID_SIZE);
    WriteEntryTags(pPacket, entry);

    sockaddr_in addr;
    contact.GetSockAddr(addr);
    AddOutstandingRequest(KAD_REQUEST_PUBLISH_SOURCE, addr);

    SendPacket(&addr, pPacket);
    pPacket->Release();
}
