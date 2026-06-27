//
 // RemoteSecurity.h
 //
 // Security utilities for Envy remote web interface
 // Part of Envy (getenvy.com) © 2016-2026
 //

#pragma once

#include "StdAfx.h"
#include <vector>

struct RemoteSession
{
	std::string sessionId;    // 128-bit cryptographically secure session ID
	uint64_t created;         // Session creation timestamp
	uint64_t lastSeen;        // Last activity timestamp
	std::string csrfToken;    // CSRF token for this session
	IN_ADDR clientIP;         // Client IP address for security tracking
};

class CRemoteSecurity
{
public:
	// IP Access Control
	static bool IsRemoteAccessAllowed(const IN_ADDR& clientIP);

	// Session Management
	static bool CreateSession(const IN_ADDR& clientIP, RemoteSession& session);
	static bool ValidateSession(const std::string& sessionId, RemoteSession& session);
	static bool UpdateSessionActivity(const std::string& sessionId);
	static bool DestroySession(const std::string& sessionId);

	// Brute Force Protection
	static bool CheckLoginThrottle(const IN_ADDR& clientIP);
	static void RecordFailedLogin(const IN_ADDR& clientIP);

	// CSRF Protection
	static bool ValidateCSRFToken(const std::string& sessionId, const std::string& token);

	// Security Headers
	static void AddSecurityHeaders(CString& headers, bool isSecureConnection = false);

	// Password Hashing
	static bool HashPassword(const std::string& password, std::string& hashOutput);
	static bool VerifyPassword(const std::string& password, const std::string& hashString);

private:
	// Session storage
	static std::map<std::string, RemoteSession> m_sessions;
	static CCriticalSection m_sessionLock;

	// Brute force protection
	struct FailedLoginAttempt
	{
		uint32_t count;
		uint64_t firstAttempt;
		uint64_t lastAttempt;
	};
	static std::map<uint32_t, FailedLoginAttempt> m_failedLogins; // Key: IP address as uint32_t
	static CCriticalSection m_failedLoginLock;

	// Helper functions - IP checking
	static bool IsPrivateIP(const IN_ADDR& ip);
	static bool IsLocalSubnet(const IN_ADDR& ip);
	static bool IsInCIDRList(const IN_ADDR& ip, const std::set<CString>& cidrs);
	static bool IsIPInCIDR(const IN_ADDR& ip, const CString& cidr);

	// Helper functions - Session/Token generation
	static std::string GenerateSecureId(size_t length = 32);
	static std::string GenerateCSRFToken();

	// Helper functions - Password hashing
	static bool FallbackHashPassword(const std::string& password, std::string& hashOutput);
	static bool FallbackVerifyPassword(const std::string& password, const std::string& hashString);
	static bool VerifyLegacySHA1(const std::string& password, const std::string& hashString);

	// Helper functions - Encoding/Utilities
	static std::string Base64Encode(const BYTE* data, size_t length);
	static std::vector<BYTE> Base64Decode(const std::string& encoded);
	static std::vector<std::string> SplitString(const std::string& str, char delimiter);
	static bool ConstantTimeCompare(const void* a, const void* b, size_t length);

	// Session cleanup
	static void CleanupExpiredSessions();
	static void CleanupExpiredFailedLogins();
};
