//
// BootstrapCatalog.h
//
// Line parsers for the shipped bootstrap catalogues:
//   Data/DefaultServices.dat  - discovery URLs (GWC, UHC, server.met, hublist)
//   Data/DefaultServers.dat   - direct host:port seeds (BT DHT routers, optional
//                               ED2K/Kad/G1/G2/DC hosts)
//
// These files are DATA, not discovered runtime peers. HostCache.dat /
// Discovery.dat hold learned hosts. Do not compile bootstrap IPs into C++.
//
// Pure wchar helpers for CDiscoveryServices::AddDefaults,
// CHostCache::LoadDefaultServers, and EnvyTests. No MFC, no network.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#pragma once

#include <cstddef>
#include <cwchar>

// EnoughServices() minima in DiscoveryServices.cpp. Keep the shipped
// DefaultServices.dat at or above these counts so a wiped Discovery.dat
// cold-starts without re-injecting a C++ list (AddDefaults C++ fallback
// is commented out).
constexpr int BootstrapMinWebCaches = 1;
constexpr int BootstrapMinG2Services = 3;
constexpr int BootstrapMinG1Services = 2;
constexpr int BootstrapMinG1UdpServices = 1;
constexpr int BootstrapMinEd2kMet = 2;
constexpr int BootstrapMinDcHublists = 2;
constexpr int BootstrapMinKadNodesDat = 1;

// Cap HostCache BT hosts copied for DHT cold-start (not a protocol limit).
constexpr int BootstrapDhtRouterPingCap = 8;
// Cap blocking gethostbyname calls on the network thread during Connect.
constexpr int BootstrapDhtBlockingResolveCap = 3;

// Directory part of a module path, without the trailing separator.
// theApp.m_strBinaryPath names Envy.exe itself, so packaged Data lives
// beside it, not under it. Returns 0 when no directory part exists.
inline size_t BootstrapModuleDirectoryLength(const wchar_t* pszModulePath)
{
	const wchar_t* pszSlash = pszModulePath ? std::wcsrchr(pszModulePath, L'\\') : nullptr;
	return pszSlash ? static_cast<size_t>(pszSlash - pszModulePath) : 0;
}

// Prefer mutable per-user catalogues when present, but never make them the
// only copy of immutable bootstrap defaults.
inline const wchar_t* BootstrapSelectCataloguePath(
    const wchar_t* pszWritablePath,
    bool bWritableExists,
    const wchar_t* pszPackagedPath,
    bool bPackagedExists)
{
	if (bWritableExists)
		return pszWritablePath;
	if (bPackagedExists)
		return pszPackagedPath;
	return nullptr;
}

inline bool BootstrapDiscoveryMinimumsSatisfied(
    int nWebCaches,
    int nG2Services,
    int nG1Services,
    int nG1UdpServices,
    int nEd2kMet,
    int nDcHublists,
    int nKadNodesDat,
    bool bG1Enabled,
    bool bEd2kEnabled,
    bool bDcEnabled,
    bool bKadEnabled)
{
	return nWebCaches >= BootstrapMinWebCaches &&
	       nG2Services >= BootstrapMinG2Services &&
	       ((nG1Services >= BootstrapMinG1Services && nG1UdpServices >= BootstrapMinG1UdpServices) || !bG1Enabled) &&
	       (nEd2kMet >= BootstrapMinEd2kMet || !bEd2kEnabled) &&
	       (nDcHublists >= BootstrapMinDcHublists || !bDcEnabled) &&
	       (nKadNodesDat >= BootstrapMinKadNodesDat || !bKadEnabled);
}

enum class BootstrapParseStatus
{
	Skip, // blank or comment
	Ok,
	Invalid
};

enum class BootstrapServiceClass
{
	Unrecognized,
	MultiGwc,    // M
	G2Gwc,       // 2
	G1Gwc,       // 1
	Ed2kMet,     // D
	DcHublist,   // H or C
	GnutellaUdp, // U (uhc / ukhl / host)
	KadNodesDat, // K (HTTPS nodes.dat bootstrap)
	Blocked,     // X
	Comment
};

enum class BootstrapServerClass
{
	Unrecognized,
	Gnutella1,  // 1 or L
	Gnutella2,  // 2 or G
	Ed2k,       // E or legacy leading space
	Dc,         // D
	BitTorrent, // B
	KadNode,    // K
	Blocked     // X
};

inline bool BootstrapIsSpace(wchar_t c)
{
	return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

// ASCII A-Z only. Catalogue URLs are ASCII; non-ASCII wchar_t values compare as-is.
inline wchar_t BootstrapFoldAscii(wchar_t c)
{
	if (c >= L'A' && c <= L'Z')
		return static_cast<wchar_t>(c - L'A' + L'a');
	return c;
}

inline bool BootstrapTrimEdges(const wchar_t* pszLine, size_t nLen, size_t* pnBegin, size_t* pnEnd)
{
	if (pszLine == nullptr || nLen == 0 || pnBegin == nullptr || pnEnd == nullptr)
		return false;
	size_t nBegin = 0;
	while (nBegin < nLen && BootstrapIsSpace(pszLine[nBegin]))
		++nBegin;
	size_t nEnd = nLen;
	while (nEnd > nBegin && BootstrapIsSpace(pszLine[nEnd - 1]))
		--nEnd;
	if (nBegin >= nEnd)
		return false;
	*pnBegin = nBegin;
	*pnEnd = nEnd;
	return true;
}

// Allowlist for catalogue URLs. Live GWC still speaks cleartext; HTTPS is
// accepted when the cache actually serves it. Scheme match is ASCII
// case-insensitive (same as CDiscoveryServices::CheckWebCacheValid).
inline bool BootstrapIsWebUrl(const wchar_t* psz, size_t nLen)
{
	if (psz == nullptr || nLen < 7)
		return false;
	wchar_t szFold[5] = {};
	const size_t nFold = nLen >= 5 ? 5 : nLen;
	for (size_t i = 0; i < nFold; ++i)
		szFold[i] = BootstrapFoldAscii(psz[i]);
	size_t nScheme = 0;
	if (nLen >= 5 && wcsncmp(szFold, L"https", 5) == 0)
		nScheme = 5;
	else if (nLen >= 4 && wcsncmp(szFold, L"http", 4) == 0)
		nScheme = 4;
	else
		return false;
	return nLen >= nScheme + 3 && psz[nScheme] == L':' && psz[nScheme + 1] == L'/' && psz[nScheme + 2] == L'/';
}

inline bool BootstrapIsHttpsUrl(const wchar_t* psz, size_t nLen)
{
	if (psz == nullptr || nLen < 8)
		return false;
	return BootstrapFoldAscii(psz[0]) == L'h' &&
	       BootstrapFoldAscii(psz[1]) == L't' &&
	       BootstrapFoldAscii(psz[2]) == L't' &&
	       BootstrapFoldAscii(psz[3]) == L'p' &&
	       BootstrapFoldAscii(psz[4]) == L's' &&
	       psz[5] == L':' && psz[6] == L'/' && psz[7] == L'/';
}

inline bool BootstrapLooksLikeHostPort(const wchar_t* psz, size_t nLen)
{
	if (psz == nullptr || nLen < 3)
		return false;
	const wchar_t* pColon = nullptr;
	for (size_t i = 0; i < nLen; ++i)
	{
		if (psz[i] == L':')
		{
			if (pColon != nullptr)
				return false; // IPv4/DNS host:port only; extra colon is malformed
			pColon = psz + i;
		}
	}
	if (pColon == nullptr || pColon == psz || pColon + 1 >= psz + nLen)
		return false;
	unsigned int nPort = 0;
	for (const wchar_t* p = pColon + 1; p < psz + nLen; ++p)
	{
		if (*p < L'0' || *p > L'9')
			return false;
		nPort = nPort * 10u + static_cast<unsigned int>(*p - L'0');
		if (nPort > 65535u)
			return false;
	}
	return nPort >= 1u && nPort <= 65535u;
}

inline bool BootstrapHasAsciiPrefix(const wchar_t* psz, size_t nLen, const wchar_t* pszPrefix, size_t nPrefix)
{
	if (psz == nullptr || pszPrefix == nullptr || nLen < nPrefix)
		return false;
	for (size_t i = 0; i < nPrefix; ++i)
	{
		if (BootstrapFoldAscii(psz[i]) != BootstrapFoldAscii(pszPrefix[i]))
			return false;
	}
	return true;
}

// U / UHC / UKHL catalogue endpoints. Same prefixes as CheckWebCacheValid;
// host is IPv4/DNS with an optional decimal port in 1..65535.
inline bool BootstrapLooksLikeUdpDiscovery(const wchar_t* psz, size_t nLen)
{
	if (psz == nullptr || nLen < 5)
		return false;
	size_t nPrefix = 0;
	if (BootstrapHasAsciiPrefix(psz, nLen, L"uhc:", 4))
		nPrefix = 4;
	else if (BootstrapHasAsciiPrefix(psz, nLen, L"ukhl:", 5))
		nPrefix = 5;
	else if (BootstrapHasAsciiPrefix(psz, nLen, L"gnutella1:host:", 15))
		nPrefix = 15;
	else if (BootstrapHasAsciiPrefix(psz, nLen, L"gnutella2:host:", 15))
		nPrefix = 15;
	else if (BootstrapHasAsciiPrefix(psz, nLen, L"gnutella:host:", 14))
		nPrefix = 14;
	else if (BootstrapHasAsciiPrefix(psz, nLen, L"g2:host:", 8))
		nPrefix = 8;
	else
		return false;

	const wchar_t* pszHost = psz + nPrefix;
	const size_t nHost = nLen - nPrefix;
	if (nHost == 0)
		return false;
	bool bHasColon = false;
	for (size_t i = 0; i < nHost; ++i)
	{
		if (pszHost[i] == L':')
		{
			bHasColon = true;
			break;
		}
		if (pszHost[i] == L' ' || pszHost[i] == L'\t' || pszHost[i] == L'/')
			return false;
	}
	if (bHasColon)
		return BootstrapLooksLikeHostPort(pszHost, nHost);
	if (nHost < 3)
		return false;
	bool bDot = false;
	for (size_t i = 0; i < nHost; ++i)
	{
		if (pszHost[i] == L'.')
			bDot = true;
	}
	return bDot;
}

inline BootstrapServiceClass BootstrapClassifyServiceType(wchar_t cType)
{
	switch (cType)
	{
	case L'M': return BootstrapServiceClass::MultiGwc;
	case L'2': return BootstrapServiceClass::G2Gwc;
	case L'1': return BootstrapServiceClass::G1Gwc;
	case L'D': return BootstrapServiceClass::Ed2kMet;
	case L'C':
	case L'H': return BootstrapServiceClass::DcHublist;
	case L'U': return BootstrapServiceClass::GnutellaUdp;
	case L'K': return BootstrapServiceClass::KadNodesDat;
	case L'X': return BootstrapServiceClass::Blocked;
	case L'#': return BootstrapServiceClass::Comment;
	default: return BootstrapServiceClass::Unrecognized;
	}
}

inline BootstrapServerClass BootstrapClassifyServerType(wchar_t cType)
{
	switch (cType)
	{
	case L'1':
	case L'L': return BootstrapServerClass::Gnutella1;
	case L'2':
	case L'G': return BootstrapServerClass::Gnutella2;
	case L'E': return BootstrapServerClass::Ed2k;
	case L'D': return BootstrapServerClass::Dc;
	case L'B': return BootstrapServerClass::BitTorrent;
	case L'K': return BootstrapServerClass::KadNode;
	case L'X': return BootstrapServerClass::Blocked;
	default: return BootstrapServerClass::Unrecognized;
	}
}

inline bool BootstrapServiceTypeNeedsUrl(BootstrapServiceClass nClass)
{
	switch (nClass)
	{
	case BootstrapServiceClass::MultiGwc:
	case BootstrapServiceClass::G2Gwc:
	case BootstrapServiceClass::G1Gwc:
	case BootstrapServiceClass::Ed2kMet:
	case BootstrapServiceClass::DcHublist:
	case BootstrapServiceClass::KadNodesDat:
	case BootstrapServiceClass::Blocked:
		return true;
	default:
		return false;
	}
}

// DefaultServices.dat: "<type> <endpoint>"
inline BootstrapParseStatus BootstrapParseServiceLine(
    const wchar_t* pszLine,
    size_t nLen,
    wchar_t* pcType,
    const wchar_t** ppszEndpoint,
    size_t* pnEndpoint)
{
	if (pcType != nullptr)
		*pcType = 0;
	if (ppszEndpoint != nullptr)
		*ppszEndpoint = nullptr;
	if (pnEndpoint != nullptr)
		*pnEndpoint = 0;

	size_t nBegin = 0;
	size_t nEnd = 0;
	if (!BootstrapTrimEdges(pszLine, nLen, &nBegin, &nEnd))
		return BootstrapParseStatus::Skip;

	const wchar_t cType = pszLine[nBegin];
	if (cType == L'#')
		return BootstrapParseStatus::Skip;

	// Match CDiscoveryServices::AddDefaults: ignore impossibly short rows.
	if (nEnd - nBegin < 7)
		return BootstrapParseStatus::Skip;

	if (nBegin + 2 > nEnd || (pszLine[nBegin + 1] != L' ' && pszLine[nBegin + 1] != L'\t'))
		return BootstrapParseStatus::Invalid;

	const BootstrapServiceClass nClass = BootstrapClassifyServiceType(cType);
	if (nClass == BootstrapServiceClass::Unrecognized)
		return BootstrapParseStatus::Invalid;

	const wchar_t* pszEndpoint = pszLine + nBegin + 2;
	size_t nEndpoint = nEnd - (nBegin + 2);
	while (nEndpoint > 0 && (*pszEndpoint == L' ' || *pszEndpoint == L'\t'))
	{
		++pszEndpoint;
		--nEndpoint;
	}
	if (nEndpoint == 0)
		return BootstrapParseStatus::Invalid;

	if (BootstrapServiceTypeNeedsUrl(nClass) && !BootstrapIsWebUrl(pszEndpoint, nEndpoint))
		return BootstrapParseStatus::Invalid;
	if (nClass == BootstrapServiceClass::KadNodesDat && !BootstrapIsHttpsUrl(pszEndpoint, nEndpoint))
		return BootstrapParseStatus::Invalid;
	if (nClass == BootstrapServiceClass::GnutellaUdp && !BootstrapLooksLikeUdpDiscovery(pszEndpoint, nEndpoint))
		return BootstrapParseStatus::Invalid;

	if (pcType != nullptr)
		*pcType = cType;
	if (ppszEndpoint != nullptr)
		*ppszEndpoint = pszEndpoint;
	if (pnEndpoint != nullptr)
		*pnEndpoint = nEndpoint;
	return BootstrapParseStatus::Ok;
}

// DefaultServers.dat: "[P|*][type][P|*] host:port"
inline BootstrapParseStatus BootstrapParseServerLine(
    const wchar_t* pszLine,
    size_t nLen,
    wchar_t* pcType,
    bool* pbPriority,
    const wchar_t** ppszHost,
    size_t* pnHost)
{
	if (pcType != nullptr)
		*pcType = 0;
	if (pbPriority != nullptr)
		*pbPriority = false;
	if (ppszHost != nullptr)
		*ppszHost = nullptr;
	if (pnHost != nullptr)
		*pnHost = 0;

	size_t nBegin = 0;
	size_t nEnd = 0;
	if (!BootstrapTrimEdges(pszLine, nLen, &nBegin, &nEnd))
		return BootstrapParseStatus::Skip;

	if (pszLine[nBegin] == L'#')
		return BootstrapParseStatus::Skip;

	bool bPriority = false;
	size_t nPos = nBegin;
	if (pszLine[nPos] == L'P' || pszLine[nPos] == L'*')
	{
		bPriority = true;
		++nPos;
	}
	if (nPos >= nEnd)
		return BootstrapParseStatus::Invalid;

	const wchar_t cType = pszLine[nPos];
	const BootstrapServerClass nClass = BootstrapClassifyServerType(cType);
	if (nClass == BootstrapServerClass::Unrecognized)
		return BootstrapParseStatus::Invalid;
	++nPos;

	if (nPos < nEnd && (pszLine[nPos] == L'P' || pszLine[nPos] == L'*'))
	{
		bPriority = true;
		++nPos;
	}
	while (nPos < nEnd && (pszLine[nPos] == L' ' || pszLine[nPos] == L'\t'))
		++nPos;
	if (nPos >= nEnd)
		return BootstrapParseStatus::Invalid;

	const size_t nHostBegin = nPos;
	while (nPos < nEnd && pszLine[nPos] != L' ' && pszLine[nPos] != L'\t')
		++nPos;
	const wchar_t* pszHost = pszLine + nHostBegin;
	const size_t nHost = nPos - nHostBegin;
	if (!BootstrapLooksLikeHostPort(pszHost, nHost))
		return BootstrapParseStatus::Invalid;

	if (pcType != nullptr)
		*pcType = cType;
	if (pbPriority != nullptr)
		*pbPriority = bPriority;
	if (ppszHost != nullptr)
		*ppszHost = pszHost;
	if (pnHost != nullptr)
		*pnHost = nHost;
	return BootstrapParseStatus::Ok;
}

inline bool BootstrapWideEqualsNoCase(const wchar_t* a, size_t na, const wchar_t* b, size_t nb)
{
	if (a == nullptr || b == nullptr || na != nb)
		return false;
	for (size_t i = 0; i < na; ++i)
	{
		if (BootstrapFoldAscii(a[i]) != BootstrapFoldAscii(b[i]))
			return false;
	}
	return true;
}

struct BootstrapDhtBootSlot
{
	unsigned long nAddr;
	unsigned short nPort;
	bool bHasName;
};

inline bool BootstrapDhtAppendBootSlot(
    BootstrapDhtBootSlot* pSlots,
    int nCap,
    int* pnBoot,
    unsigned long nAddr,
    unsigned short nPort,
    bool bHasName)
{
	if (pSlots == nullptr || pnBoot == nullptr || nCap <= 0)
		return false;
	if (*pnBoot < 0 || *pnBoot >= nCap)
		return false;
	pSlots[*pnBoot].nAddr = nAddr;
	pSlots[*pnBoot].nPort = nPort;
	pSlots[*pnBoot].bHasName = bHasName;
	++(*pnBoot);
	return true;
}

inline bool BootstrapDhtUseStoredAddr(unsigned long nAddr)
{
	return nAddr != 0ul;
}

inline bool BootstrapDhtTryBlockingResolve(unsigned long nAddr, bool bHasName, int nDnsUsed)
{
	return nAddr == 0ul && bHasName && nDnsUsed >= 0 && nDnsUsed < BootstrapDhtBlockingResolveCap;
}
