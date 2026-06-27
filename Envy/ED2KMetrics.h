//
// ED2KMetrics.h
//
// Performance monitoring and metrics collection for ED2K protocol
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#pragma once

class CED2KMetrics
{
public:
    CED2KMetrics();
    virtual ~CED2KMetrics();

    // AICH Metrics
    void RecordAICHVerification(bool bSuccess, DWORD dwTimeMs);
    void RecordAICHHashing(DWORD dwBytesProcessed, DWORD dwTimeMs);

    // Kademlia Metrics
    void RecordKadRequest(KadRequestType type, bool bSuccess);
    void RecordKadResponseTime(DWORD dwTimeMs);
    void RecordKadContactAdded();
    void RecordKadContactRemoved();

    // MultiPacket Metrics
    void RecordMultiPacketSent(DWORD dwPacketsCombined);
    void RecordMultiPacketReceived(DWORD dwPacketsCombined);

    // Network Metrics
    void RecordIPv6Connection(bool bSuccess);
    void RecordUPnPAttempt(bool bSuccess);

    // Get metrics (for UI display)
    void GetMetrics(CString& strMetrics);

    // Reset metrics
    void Reset();

private:
    // AICH metrics
    struct AICHMetrics {
        DWORD dwVerificationsAttempted;
        DWORD dwVerificationsSuccessful;
        DWORD dwTotalVerificationTime;
        DWORD dwTotalBytesHashed;
        DWORD dwTotalHashingTime;
    } m_aichMetrics;

    // Kademlia metrics
    struct KadMetrics {
        DWORD dwRequestsSent;
        DWORD dwRequestsSuccessful;
        DWORD dwTotalResponseTime;
        DWORD dwContactsAdded;
        DWORD dwContactsRemoved;
        DWORD dwRequestsByType[3]; // Bootstrap, FindNode, FindValue
    } m_kadMetrics;

    // MultiPacket metrics
    struct MultiPacketMetrics {
        DWORD dwPacketsSent;
        DWORD dwPacketsReceived;
        DWORD dwTotalPacketsCombined;
    } m_multiPacketMetrics;

    // Network metrics
    struct NetworkMetrics {
        DWORD dwIPv6ConnectionsAttempted;
        DWORD dwIPv6ConnectionsSuccessful;
        DWORD dwUPnPAttempts;
        DWORD dwUPnPSuccesses;
    } m_networkMetrics;

    // Thread safety
    CCriticalSection m_csMetrics;
};
