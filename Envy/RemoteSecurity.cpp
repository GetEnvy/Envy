//
 // RemoteSecurity.cpp
 //
 // Security utilities for Envy remote web interface
 // Part of Envy (getenvy.com) © 2016-2026
 //

#include "StdAfx.h"
#include "RemoteSecurity.h"
#include "Settings.h"
#include <vector>
#include <sstream>
#include <iphlpapi.h>  // For GetAdaptersInfo
#include <ws2tcpip.h>  // For inet_pton

#pragma comment(lib, "bcrypt.lib")  // Link bcrypt library
#pragma comment(lib, "iphlpapi.lib") // Link IP helper API library

// Session storage
std::map<std::string, RemoteSession> CRemoteSecurity::m_sessions;
CCriticalSection CRemoteSecurity::m_sessionLock;

// Brute force protection
std::map<uint32_t, CRemoteSecurity::FailedLoginAttempt> CRemoteSecurity::m_failedLogins;
CCriticalSection CRemoteSecurity::m_failedLoginLock;

/////////////////////////////////////////////////////////////////////////////
// IP Access Control

bool CRemoteSecurity::IsRemoteAccessAllowed(const IN_ADDR& clientIP)
{
	// Always allow localhost (127.0.0.1)
	if (clientIP.s_addr == htonl(INADDR_LOOPBACK)) {
		return true;
	}

	// Check deprecated AllowExternal setting (backward compatibility)
	if (Settings.Remote.AllowExternal) {
		return true;
	}

	// Check WAN access (with warning in UI)
	if (Settings.Remote.AllowWAN) {
		// WAN access explicitly enabled
		return true;
	}

	// Check LAN access
	if (Settings.Remote.AllowLAN) {
		if (IsPrivateIP(clientIP) || IsLocalSubnet(clientIP)) {
			return true;
		}
	}

	// Check explicit CIDR whitelist
	if (!Settings.Remote.AllowedCIDRs.empty()) {
		if (IsInCIDRList(clientIP, Settings.Remote.AllowedCIDRs)) {
			return true;
		}
	}

	// Default: deny access
	return false;
}

bool CRemoteSecurity::IsPrivateIP(const IN_ADDR& ip)
{
	// RFC1918 private address ranges
	uint32_t addr = ntohl(ip.s_addr);

	// 10.0.0.0/8
	if ((addr & 0xFF000000) == 0x0A000000) return true;

	// 172.16.0.0/12
	if ((addr & 0xFFF00000) == 0xAC100000) return true;

	// 192.168.0.0/16
	if ((addr & 0xFFFF0000) == 0xC0A80000) return true;

	return false;
}

bool CRemoteSecurity::IsLocalSubnet(const IN_ADDR& ip)
{
	// For Windows, we can use GetAdaptersInfo to get local network interfaces
	// and check if the client IP is on the same subnet as any local interface

	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get the necessary size
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			return false; // Memory allocation failed
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			// Check each IP address of this adapter
			PIP_ADDR_STRING pAddrString = &pAdapter->IpAddressList;
			PIP_ADDR_STRING pMaskString = &pAdapter->IpAddressList;

			while (pAddrString) {
				if (strcmp(pAddrString->IpAddress.String, "0.0.0.0") != 0) {
					// Parse adapter IP and subnet mask
					IN_ADDR adapterIP, adapterMask;
					if (inet_pton(AF_INET, pAddrString->IpAddress.String, &adapterIP) == 1 &&
						inet_pton(AF_INET, pAddrString->IpMask.String, &adapterMask) == 1) {

						// Check if client IP is on the same subnet
						if ((ip.s_addr & adapterMask.s_addr) == (adapterIP.s_addr & adapterMask.s_addr)) {
							free(pAdapterInfo);
							return true;
						}
					}
				}
				pAddrString = pAddrString->Next;
			}
			pAdapter = pAdapter->Next;
		}
	}

	if (pAdapterInfo) {
		free(pAdapterInfo);
	}

	return false; // Not on local subnet
}

bool CRemoteSecurity::IsInCIDRList(const IN_ADDR& ip, const std::set<CString>& cidrs)
{
	// Parse CIDR ranges and check if IP is within any range
	// Format: "192.168.1.0/24"
	for (std::set<CString>::const_iterator it = cidrs.begin(); it != cidrs.end(); ++it) {
		const CString& cidr = *it;
		if (IsIPInCIDR(ip, cidr)) {
			return true;
		}
	}
	return false;
}

// Helper function to check if an IP is within a CIDR range
bool CRemoteSecurity::IsIPInCIDR(const IN_ADDR& ip, const CString& cidr)
{
	// Parse CIDR notation: "192.168.1.0/24"
	int slashPos = cidr.Find('/');
	if (slashPos == -1) {
		return false; // Invalid CIDR format
	}

	CString networkStr = cidr.Left(slashPos);
	CString prefixStr = cidr.Mid(slashPos + 1);

	// Parse prefix length
	int prefixLen = _ttoi(prefixStr);
	if (prefixLen < 0 || prefixLen > 32) {
		return false; // Invalid prefix length
	}

	// Parse network address
	IN_ADDR networkAddr;
	if (inet_pton(AF_INET, CT2A(networkStr), &networkAddr) != 1) {
		return false; // Invalid IP address
	}

	// Create subnet mask from prefix length
	uint32_t mask = (prefixLen == 0) ? 0 : (~0U << (32 - prefixLen));

	// Convert to host byte order for comparison
	uint32_t ipHost = ntohl(ip.s_addr);
	uint32_t networkHost = ntohl(networkAddr.s_addr);

	// Check if IP is in the subnet
	return (ipHost & mask) == (networkHost & mask);
}


/////////////////////////////////////////////////////////////////////////////
// Session Management

bool CRemoteSecurity::CreateSession(const IN_ADDR& clientIP, RemoteSession& session)
{
	CQuickLock lock(m_sessionLock);

	// Generate cryptographically secure session ID (128 bits = 32 hex chars)
	session.sessionId = GenerateSecureId(32);
	session.created = GetTickCount64();
	session.lastSeen = session.created;
	session.clientIP = clientIP;
	session.csrfToken = GenerateCSRFToken();

	// Store session
	m_sessions[session.sessionId] = session;

	// Cleanup expired sessions periodically
	if (m_sessions.size() % 10 == 0) { // Every 10 sessions
		CleanupExpiredSessions();
	}

	return true;
}

bool CRemoteSecurity::ValidateSession(const std::string& sessionId, RemoteSession& session)
{
	CQuickLock lock(m_sessionLock);

	auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return false;
	}

	session = it->second;

	// Check session expiry (30 minutes)
	const uint64_t SESSION_TIMEOUT = 30 * 60 * 1000; // 30 minutes
	if (GetTickCount64() - session.created > SESSION_TIMEOUT) {
		m_sessions.erase(it);
		return false;
	}

	// Update last seen
	session.lastSeen = GetTickCount64();
	it->second.lastSeen = session.lastSeen;

	return true;
}

bool CRemoteSecurity::UpdateSessionActivity(const std::string& sessionId)
{
	CQuickLock lock(m_sessionLock);

	auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return false;
	}

	it->second.lastSeen = GetTickCount64();
	return true;
}

bool CRemoteSecurity::DestroySession(const std::string& sessionId)
{
	CQuickLock lock(m_sessionLock);

	auto it = m_sessions.find(sessionId);
	if (it != m_sessions.end()) {
		m_sessions.erase(it);
		return true;
	}

	return false;
}

void CRemoteSecurity::CleanupExpiredSessions()
{
	const uint64_t SESSION_TIMEOUT = 30 * 60 * 1000; // 30 minutes
	uint64_t now = GetTickCount64();

	for (auto it = m_sessions.begin(); it != m_sessions.end(); ) {
		if (now - it->second.created > SESSION_TIMEOUT) {
			it = m_sessions.erase(it);
		} else {
			++it;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Brute Force Protection

bool CRemoteSecurity::CheckLoginThrottle(const IN_ADDR& clientIP)
{
	CQuickLock lock(m_failedLoginLock);

	uint32_t ipKey = clientIP.s_addr;
	auto it = m_failedLogins.find(ipKey);

	if (it == m_failedLogins.end()) {
		return true; // No failed attempts
	}

	FailedLoginAttempt& attempt = it->second;
	uint64_t now = GetTickCount64();

	// Reset counter if window has passed (5 minutes)
	const uint64_t THROTTLE_WINDOW = 5 * 60 * 1000;
	if (now - attempt.firstAttempt > THROTTLE_WINDOW) {
		m_failedLogins.erase(it);
		return true;
	}

	// Check if too many attempts
	const uint32_t MAX_ATTEMPTS = 5;
	if (attempt.count >= MAX_ATTEMPTS) {
		// Temporary ban for remaining window time
		return false;
	}

	return true;
}

void CRemoteSecurity::RecordFailedLogin(const IN_ADDR& clientIP)
{
	CQuickLock lock(m_failedLoginLock);

	uint32_t ipKey = clientIP.s_addr;
	uint64_t now = GetTickCount64();

	auto it = m_failedLogins.find(ipKey);
	if (it == m_failedLogins.end()) {
		FailedLoginAttempt attempt = {1, now, now};
		m_failedLogins[ipKey] = attempt;
	} else {
		it->second.count++;
		it->second.lastAttempt = now;
	}

	// Cleanup old entries periodically
	if (m_failedLogins.size() % 50 == 0) {
		CleanupExpiredFailedLogins();
	}
}

void CRemoteSecurity::CleanupExpiredFailedLogins()
{
	const uint64_t CLEANUP_WINDOW = 10 * 60 * 1000; // 10 minutes
	uint64_t now = GetTickCount64();

	for (auto it = m_failedLogins.begin(); it != m_failedLogins.end(); ) {
		if (now - it->second.lastAttempt > CLEANUP_WINDOW) {
			it = m_failedLogins.erase(it);
		} else {
			++it;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSRF Protection

bool CRemoteSecurity::ValidateCSRFToken(const std::string& sessionId, const std::string& token)
{
	CQuickLock lock(m_sessionLock);

	auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return false;
	}

	return (it->second.csrfToken == token);
}

/////////////////////////////////////////////////////////////////////////////
// Security Headers

void CRemoteSecurity::AddSecurityHeaders(CString& headers, bool isSecureConnection)
{
	headers += L"Cache-Control: no-store\r\n";
	headers += L"X-Content-Type-Options: nosniff\r\n";
	headers += L"X-Frame-Options: DENY\r\n";
	headers += L"Referrer-Policy: no-referrer\r\n";
	headers += L"Content-Security-Policy: default-src 'self'\r\n";

	// Add HSTS only for HTTPS connections
	if (isSecureConnection) {
		headers += L"Strict-Transport-Security: max-age=31536000; includeSubDomains\r\n";
	}
}

/////////////////////////////////////////////////////////////////////////////
// Helper Functions

std::string CRemoteSecurity::GenerateSecureId(size_t length)
{
	// Generate cryptographically secure random bytes
	std::vector<BYTE> bytes(length / 2 + 1); // +1 for rounding

	if (theApp.m_hCryptProv != 0) {
		if (!CryptGenRandom(theApp.m_hCryptProv, static_cast<DWORD>(bytes.size()), bytes.data())) {
			// Fallback to less secure method
			for (size_t i = 0; i < bytes.size(); ++i) {
				bytes[i] = static_cast<BYTE>(rand() % 256);
			}
		}
	} else {
		// No crypto provider available, use rand()
		for (size_t i = 0; i < bytes.size(); ++i) {
			bytes[i] = static_cast<BYTE>(rand() % 256);
		}
	}

	// Convert to hex string
	std::string result;
	for (size_t i = 0; i < length / 2; ++i) {
		char hex[3];
		sprintf_s(hex, "%02x", bytes[i]);
		result += hex;
	}

	return result;
}

std::string CRemoteSecurity::GenerateCSRFToken()
{
	return GenerateSecureId(32); // 128-bit token
}

/////////////////////////////////////////////////////////////////////////////
// Password Hashing with PBKDF2

bool CRemoteSecurity::HashPassword(const std::string& password, std::string& hashOutput)
{
	// Use a simple salted SHA256 hash for now
	// PBKDF2 would be better but requires more complex bcrypt setup
	return FallbackHashPassword(password, hashOutput);
}

bool CRemoteSecurity::VerifyPassword(const std::string& password, const std::string& hashString)
{
	// Parse hash string: algorithm:salt:hash
	std::vector<std::string> parts = SplitString(hashString, ':');

	if (parts.size() == 3 && parts[0] == "sha256-salted") {
		return FallbackVerifyPassword(password, hashString);
	}

	// Check if it's a legacy SHA1 hash for backward compatibility
	return VerifyLegacySHA1(password, hashString);
}

bool CRemoteSecurity::FallbackHashPassword(const std::string& password, std::string& hashOutput)
{
	// Fallback to SHA256 with salt (better than SHA1 but not as secure as PBKDF2)
	const size_t SALT_LENGTH = 16;

	// Generate random salt as hex string then convert to bytes
	std::string saltHex = GenerateSecureId(SALT_LENGTH * 2);  // Each byte = 2 hex chars
	std::vector<BYTE> salt(SALT_LENGTH);
	for (size_t i = 0; i < SALT_LENGTH && i * 2 + 1 < saltHex.length(); ++i) {
		char hex[3] = { saltHex[i * 2], saltHex[i * 2 + 1], 0 };
		salt[i] = static_cast<BYTE>(strtoul(hex, nullptr, 16));
	}

	// Simple hash: SHA256(salt + password)
	std::string combined = std::string(reinterpret_cast<char*>(salt.data()), SALT_LENGTH) + password;

	CSHA256 sha256;
	sha256.Add(reinterpret_cast<const BYTE*>(combined.c_str()), combined.length());
	Hashes::Sha256Hash hash;
	sha256.GetHash(&hash[0]);

	std::string saltB64 = Base64Encode(salt.data(), SALT_LENGTH);
	std::string hashB64 = Base64Encode(&hash[0], 32);

	char buffer[256];
	sprintf_s(buffer, sizeof(buffer), "sha256-salted:%s:%s", saltB64.c_str(), hashB64.c_str());
	hashOutput = buffer;

	return true;
}

bool CRemoteSecurity::FallbackVerifyPassword(const std::string& password, const std::string& hashString)
{
	std::vector<std::string> parts = SplitString(hashString, ':');
	if (parts.size() != 3 || parts[0] != "sha256-salted") {
		return false;
	}

	std::vector<BYTE> salt = Base64Decode(parts[1]);
	std::vector<BYTE> expectedHash = Base64Decode(parts[2]);

	if (salt.empty() || expectedHash.empty()) {
		return false;
	}

	std::string combined = std::string(reinterpret_cast<char*>(salt.data()), salt.size()) + password;

	CSHA256 sha256;
	sha256.Add(reinterpret_cast<const BYTE*>(combined.c_str()), combined.length());
	Hashes::Sha256Hash computedHash;
	sha256.GetHash(&computedHash[0]);

	return ConstantTimeCompare(&computedHash[0], expectedHash.data(), 32);
}

bool CRemoteSecurity::VerifyLegacySHA1(const std::string& password, const std::string& hashString)
{
	// Support legacy SHA1 hashes for backward compatibility
	CSHA sha1;
	sha1.Add(reinterpret_cast<const BYTE*>(password.c_str()), password.length());
	Hashes::Sha1Hash computedHash;
	sha1.GetHash(&computedHash[0]);

	CString computedCStr = computedHash.toString();
	CT2A computedStr(computedCStr);  // Convert CString to narrow string
	return ConstantTimeCompare(static_cast<const char*>(computedStr), hashString.c_str(), hashString.length());
}

/////////////////////////////////////////////////////////////////////////////
// Helper Functions

std::vector<std::string> CRemoteSecurity::SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

std::string CRemoteSecurity::Base64Encode(const BYTE* data, size_t length)
{
	// Simple base64 encoding (you might want to use a proper base64 library)
	static const char* base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string encoded;
	const BYTE* bytes = data;

	for (size_t i = 0; i < length; i += 3) {
		uint32_t octet_a = i < length ? bytes[i] : 0;
		uint32_t octet_b = i + 1 < length ? bytes[i + 1] : 0;
		uint32_t octet_c = i + 2 < length ? bytes[i + 2] : 0;

		uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

		encoded += base64Chars[(triple >> 18) & 0x3F];
		encoded += base64Chars[(triple >> 12) & 0x3F];
		encoded += base64Chars[(triple >> 6) & 0x3F];
		encoded += base64Chars[triple & 0x3F];
	}

	// Add padding
	size_t padding = (3 - (length % 3)) % 3;
	for (size_t i = 0; i < padding; ++i) {
		encoded[encoded.size() - 1 - i] = '=';
	}

	return encoded;
}

std::vector<BYTE> CRemoteSecurity::Base64Decode(const std::string& encoded)
{
	// Simple base64 decoding (you might want to use a proper base64 library)
	static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::vector<BYTE> decoded;

	uint32_t val = 0;
	int valb = -8;
	for (size_t i = 0; i < encoded.length(); ++i) {
		char c = encoded[i];
		if (c == '=') break;
		size_t pos = base64Chars.find(c);
		if (pos == std::string::npos) continue;

		val = (val << 6) + static_cast<uint32_t>(pos);
		valb += 6;
		if (valb >= 0) {
			decoded.push_back(static_cast<BYTE>((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	return decoded;
}

bool CRemoteSecurity::ConstantTimeCompare(const void* a, const void* b, size_t length)
{
	const BYTE* pa = static_cast<const BYTE*>(a);
	const BYTE* pb = static_cast<const BYTE*>(b);

	BYTE result = 0;
	for (size_t i = 0; i < length; ++i) {
		result |= pa[i] ^ pb[i];
	}

	return result == 0;
}
