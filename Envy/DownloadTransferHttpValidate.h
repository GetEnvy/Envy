//
// DownloadTransferHttpValidate.h
//
// Shared policy helpers for bounded HTTP download framing.
// Suitable for EnvyTests without linking the full MFC transfer stack.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#pragma once

#include <windows.h>

// Peer numeric fields must not accept a sign, a numeric prefix followed by
// junk, or an overflowing decimal value as scanf would.
inline bool ParsePeerUnsignedDecimal(const wchar_t* pszValue, ULONGLONG& nValue) noexcept
{
	nValue = 0;
	if (!pszValue || !*pszValue) return false;
	for (; *pszValue; ++pszValue)
	{
		if (*pszValue < L'0' || *pszValue > L'9') return false;
		const unsigned nDigit = *pszValue - L'0';
		if (nValue > (~0ull - nDigit) / 10) return false;
		nValue = nValue * 10 + nDigit;
	}
	return true;
}

// FromBytes checks hashes but can ignore a trailing partial hash or extra
// nodes. Admit only complete breadth-first rows for 1024-byte Tiger leaves.
// Shareaza counts the root as level 1; LimeWire/gtk-gnutella use depth 0.
// MAXDWORD means a bare tree with no XML depth to compare.
inline bool ThexTreeMatchesDepth(ULONGLONG nSize, DWORD nBytes, DWORD nDepth = MAXDWORD) noexcept
{
	if (nSize == 0 || nSize == ~0ull || nBytes == 0 || nBytes % 24 != 0) return false;
	const ULONGLONG nLeaves = nSize / 1024 + (nSize % 1024 != 0);
	ULONGLONG nDivisor = 1;
	while (nDivisor < nLeaves) nDivisor *= 2;
	ULONGLONG nNodes = 0;
	for (DWORD nLevel = 1; nDivisor; ++nLevel, nDivisor /= 2)
	{
		nNodes += nLeaves / nDivisor + (nLeaves % nDivisor != 0);
		if (nNodes == nBytes / 24 &&
		    (nDepth == MAXDWORD || nDepth == nLevel || nDepth == nLevel - 1)) return true;
	}
	return false;
}

// Explicit Content-Length: 0 must not be rejected while parsing headers for
// control responses (busy/queue, range fault, redirect) or MetaFetch/THEX,
// which may legitimately advertise an empty body. Ordinary file content
// rejection is applied only after those paths have been classified.
inline bool RejectExplicitZeroContentLength(
    bool bMetaFetch,
    bool bTigerFetch,
    bool bBusyFault,
    bool bRangeFault,
    bool bRedirect) noexcept
{
	return !bMetaFetch && !bTigerFetch && !bBusyFault && !bRangeFault && !bRedirect;
}

// After a known-length THEX body is fully consumed, the remainder tracked on
// m_nLength must be zero so OnDropped does not treat a completed fetch as
// truncated. A positive remainder means bytes are still outstanding.
inline bool TigerKnownLengthBodyFullyConsumed(ULONGLONG nRemainingLength) noexcept
{
	return nRemainingLength == 0;
}

// One socket event can span headers and body. Each read applies the current
// phase's cap; repeat only while the socket queue shrinks, so bandwidth limits
// or a full partial-frame buffer cannot create a busy loop.
template<typename ReadInput, typename PendingInput>
inline bool ReadHttpInputUntilBlocked(ReadInput readInput, PendingInput pendingInput)
{
	DWORD nPending = pendingInput();
	for (;;)
	{
		if (!readInput())
			return false;
		const DWORD nRemaining = pendingInput();
		if (nRemaining == 0 || nRemaining >= nPending)
			return true;
		nPending = nRemaining;
	}
}
