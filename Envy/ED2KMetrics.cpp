//
// ED2KMetrics.cpp
//
// Performance monitoring and metrics collection implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "StdAfx.h"
#include "ED2KMetrics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CED2KMetrics::CED2KMetrics()
{
    Reset();
}

CED2KMetrics::~CED2KMetrics()
{
}

void CED2KMetrics::RecordAICHVerification(bool bSuccess, DWORD dwTimeMs)
{
    CSingleLock lock(&m_csMetrics, TRUE);

    m_aichMetrics.dwVerificationsAttempted++;
    if (bSuccess) {
        m_aichMetrics.dwVerificationsSuccessful++;
    }
    m_aichMetrics.dwTotalVerificationTime += dwTimeMs;
}

void CED2KMetrics::RecordAICHHashing(DWORD dwBytesProcessed, DWORD dwTimeMs)
{
    CSingleLock lock(&m_csMetrics, TRUE);

    m_aichMetrics.dwTotalBytesHashed += dwBytesProcessed;
    m_aichMetrics.dwTotalHashingTime += dwTimeMs;
}

void CED2KMetrics::RecordKadRequest(KadRequestType type, bool bSuccess)
{
    CSingleLock lock(&m_csMetrics, TRUE);

    if (type < _countof(m_kadMetrics.dwRequestsByType)) {
        m_kadMetrics.dwRequestsByType[type]++;
    }

    m_kadMetrics.dwRequestsSent++;
    if (bSuccess) {
        m_kadMetrics.dwRequestsSuccessful++;
    }
}

void CED2KMetrics::RecordKadResponseTime(DWORD dwTimeMs)
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_kadMetrics.dwTotalResponseTime += dwTimeMs;
}

void CED2KMetrics::RecordKadContactAdded()
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_kadMetrics.dwContactsAdded++;
}

void CED2KMetrics::RecordKadContactRemoved()
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_kadMetrics.dwContactsRemoved++;
}

void CED2KMetrics::RecordMultiPacketSent(DWORD dwPacketsCombined)
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_multiPacketMetrics.dwPacketsSent++;
    m_multiPacketMetrics.dwTotalPacketsCombined += dwPacketsCombined;
}

void CED2KMetrics::RecordMultiPacketReceived(DWORD dwPacketsCombined)
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_multiPacketMetrics.dwPacketsReceived++;
    m_multiPacketMetrics.dwTotalPacketsCombined += dwPacketsCombined;
}

void CED2KMetrics::RecordIPv6Connection(bool bSuccess)
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_networkMetrics.dwIPv6ConnectionsAttempted++;
    if (bSuccess) {
        m_networkMetrics.dwIPv6ConnectionsSuccessful++;
    }
}

void CED2KMetrics::RecordUPnPAttempt(bool bSuccess)
{
    CSingleLock lock(&m_csMetrics, TRUE);
    m_networkMetrics.dwUPnPAttempts++;
    if (bSuccess) {
        m_networkMetrics.dwUPnPSuccesses++;
    }
}

void CED2KMetrics::GetMetrics(CString& strMetrics)
{
    CSingleLock lock(&m_csMetrics, TRUE);

    strMetrics.Empty();

    // AICH Metrics
    strMetrics += L"AICH Metrics:\r\n";
    strMetrics.AppendFormat(L"  Verifications: %d/%d successful\r\n",
        m_aichMetrics.dwVerificationsSuccessful, m_aichMetrics.dwVerificationsAttempted);
    if (m_aichMetrics.dwVerificationsAttempted > 0) {
        DWORD dwAvgTime = m_aichMetrics.dwTotalVerificationTime / m_aichMetrics.dwVerificationsAttempted;
        strMetrics.AppendFormat(L"  Avg verification time: %d ms\r\n", dwAvgTime);
    }
    if (m_aichMetrics.dwTotalHashingTime > 0) {
        float fMBps = (float)m_aichMetrics.dwTotalBytesHashed / (1024.0f * 1024.0f) /
                     ((float)m_aichMetrics.dwTotalHashingTime / 1000.0f);
        strMetrics.AppendFormat(L"  Hashing speed: %.2f MB/s\r\n", fMBps);
    }

    // Kademlia Metrics
    strMetrics += L"\r\nKademlia Metrics:\r\n";
    strMetrics.AppendFormat(L"  Requests: %d/%d successful\r\n",
        m_kadMetrics.dwRequestsSuccessful, m_kadMetrics.dwRequestsSent);
    if (m_kadMetrics.dwRequestsSuccessful > 0) {
        DWORD dwAvgResponseTime = m_kadMetrics.dwTotalResponseTime / m_kadMetrics.dwRequestsSuccessful;
        strMetrics.AppendFormat(L"  Avg response time: %d ms\r\n", dwAvgResponseTime);
    }
    strMetrics.AppendFormat(L"  Contacts: +%d -%d\r\n",
        m_kadMetrics.dwContactsAdded, m_kadMetrics.dwContactsRemoved);
    strMetrics.AppendFormat(L"  By type: Bootstrap=%d, FindNode=%d, FindValue=%d\r\n",
        m_kadMetrics.dwRequestsByType[0], m_kadMetrics.dwRequestsByType[1], m_kadMetrics.dwRequestsByType[2]);

    // MultiPacket Metrics
    strMetrics += L"\r\nMultiPacket Metrics:\r\n";
    strMetrics.AppendFormat(L"  Sent: %d packets (%d combined)\r\n",
        m_multiPacketMetrics.dwPacketsSent, m_multiPacketMetrics.dwTotalPacketsCombined);
    strMetrics.AppendFormat(L"  Received: %d packets\r\n", m_multiPacketMetrics.dwPacketsReceived);

    // Network Metrics
    strMetrics += L"\r\nNetwork Metrics:\r\n";
    strMetrics.AppendFormat(L"  IPv6: %d/%d successful\r\n",
        m_networkMetrics.dwIPv6ConnectionsSuccessful, m_networkMetrics.dwIPv6ConnectionsAttempted);
    strMetrics.AppendFormat(L"  UPnP: %d/%d successful\r\n",
        m_networkMetrics.dwUPnPSuccesses, m_networkMetrics.dwUPnPAttempts);
}

void CED2KMetrics::Reset()
{
    CSingleLock lock(&m_csMetrics, TRUE);

    memset(&m_aichMetrics, 0, sizeof(m_aichMetrics));
    memset(&m_kadMetrics, 0, sizeof(m_kadMetrics));
    memset(&m_multiPacketMetrics, 0, sizeof(m_multiPacketMetrics));
    memset(&m_networkMetrics, 0, sizeof(m_networkMetrics));
}
