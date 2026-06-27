//
// test_hashlib.cpp
//
// Unit tests for HashLib algorithms (MD4, MD5, SHA-1, SHA-256, ED2K)
// Uses NIST / RFC test vectors for correctness verification.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// License: GNU Affero General Public License v3.0 (AGPLv3)
//

#include "test_framework.h"

#ifdef WIN64
#define NTDDI_VERSION 0x06000000
#define _WIN32_WINNT  0x0600
#else
#define NTDDI_VERSION 0x05010200
#define _WIN32_WINNT  0x0501
#endif
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../HashLib/HashLib.h"
#include <cstring>
#include <cstdio>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool hash_eq(const void* a, const void* b, size_t len) {
	return std::memcmp(a, b, len) == 0;
}

static void print_hex(const char* label, const uint8* buf, size_t len) {
	std::printf("    %s: ", label);
	for (size_t i = 0; i < len; ++i)
		std::printf("%02x", buf[i]);
	std::printf("\n");
}

// ---------------------------------------------------------------------------
// MD4 (RFC 1320)
// ---------------------------------------------------------------------------

static bool test_md4_vectors() {
	struct { const char* input; uint8 expected[16]; } vectors[] = {
		// RFC 1320 test vectors
		{ "",
		  {0x31,0xd6,0xcf,0xe0,0xd1,0x6a,0xe9,0x31,0xb7,0x3c,0x59,0xd7,0xe0,0xc0,0x89,0xc0} },
		{ "a",
		  {0xbd,0xe5,0x2c,0xb3,0x1d,0xe3,0x3e,0x46,0x24,0x5e,0x05,0xfb,0xdb,0xd6,0xfb,0x24} },
		{ "abc",
		  {0xa4,0x48,0x01,0x7a,0xaf,0x21,0xd8,0x52,0x5f,0xc1,0x0a,0xe8,0x7a,0xa6,0x72,0x9d} },
		{ "message digest",
		  {0xd9,0x13,0x0a,0x81,0x64,0x54,0x9f,0xe8,0x18,0x87,0x48,0x06,0xe1,0xc7,0x01,0x4b} },
		{ "abcdefghijklmnopqrstuvwxyz",
		  {0xd7,0x9e,0x1c,0x30,0x8a,0xa5,0xbb,0xcd,0xee,0xa8,0xed,0x63,0xdf,0x41,0x2d,0xa9} },
	};

	for (const auto& v : vectors) {
		CMD4 md4;
		md4.Reset();
		md4.Add(v.input, std::strlen(v.input));
		md4.Finish();
		uint8 hash[16];
		md4.GetHash(hash);
		if (!hash_eq(hash, v.expected, 16)) {
			std::printf("MD4 mismatch for \"%s\"\n", v.input);
			print_hex("got     ", hash, 16);
			print_hex("expected", v.expected, 16);
			return false;
		}
	}
	return true;
}

static bool test_md4_incremental() {
	CMD4 full, inc;
	full.Reset();
	full.Add("Hello World", 11);
	full.Finish();

	inc.Reset();
	inc.Add("Hello", 5);
	inc.Add(" ", 1);
	inc.Add("World", 5);
	inc.Finish();

	uint8 h1[16], h2[16];
	full.GetHash(h1);
	inc.GetHash(h2);
	if (!hash_eq(h1, h2, 16)) {
		std::printf("MD4 incremental hashing mismatch\n");
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// MD5 (RFC 1321)
// ---------------------------------------------------------------------------

static bool test_md5_vectors() {
	struct { const char* input; uint8 expected[16]; } vectors[] = {
		{ "",
		  {0xd4,0x1d,0x8c,0xd9,0x8f,0x00,0xb2,0x04,0xe9,0x80,0x09,0x98,0xec,0xf8,0x42,0x7e} },
		{ "a",
		  {0x0c,0xc1,0x75,0xb9,0xc0,0xf1,0xb6,0xa8,0x31,0xc3,0x99,0xe2,0x69,0x77,0x26,0x61} },
		{ "abc",
		  {0x90,0x01,0x50,0x98,0x3c,0xd2,0x4f,0xb0,0xd6,0x96,0x3f,0x7d,0x28,0xe1,0x7f,0x72} },
		{ "message digest",
		  {0xf9,0x6b,0x69,0x7d,0x7c,0xb7,0x93,0x8d,0x52,0x5a,0x2f,0x31,0xaa,0xf1,0x61,0xd0} },
		{ "abcdefghijklmnopqrstuvwxyz",
		  {0xc3,0xfc,0xd3,0xd7,0x61,0x92,0xe4,0x00,0x7d,0xfb,0x49,0x6c,0xca,0x67,0xe1,0x3b} },
	};

	for (const auto& v : vectors) {
		CMD5 md5;
		md5.Reset();
		md5.Add(v.input, std::strlen(v.input));
		md5.Finish();
		uint8 hash[16];
		md5.GetHash(hash);
		if (!hash_eq(hash, v.expected, 16)) {
			std::printf("MD5 mismatch for \"%s\"\n", v.input);
			print_hex("got     ", hash, 16);
			print_hex("expected", v.expected, 16);
			return false;
		}
	}
	return true;
}

static bool test_md5_incremental() {
	CMD5 full, inc;
	full.Reset();
	full.Add("Hello World", 11);
	full.Finish();

	inc.Reset();
	inc.Add("Hello", 5);
	inc.Add(" ", 1);
	inc.Add("World", 5);
	inc.Finish();

	uint8 h1[16], h2[16];
	full.GetHash(h1);
	inc.GetHash(h2);
	if (!hash_eq(h1, h2, 16)) {
		std::printf("MD5 incremental hashing mismatch\n");
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// SHA-1 (FIPS 180-4 / RFC 3174)
// ---------------------------------------------------------------------------

static bool test_sha1_vectors() {
	struct { const char* input; uint8 expected[20]; } vectors[] = {
		// NIST FIPS 180-4 examples
		{ "abc",
		  {0xa9,0x99,0x3e,0x36,0x47,0x06,0x81,0x6a,0xba,0x3e,
		   0x25,0x71,0x78,0x50,0xc2,0x6c,0x9c,0xd0,0xd8,0x9d} },
		{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		  {0x84,0x98,0x3e,0x44,0x1c,0x3b,0xd2,0x6e,0xba,0xae,
		   0x4a,0xa1,0xf9,0x51,0x29,0xe5,0xe5,0x46,0x70,0xf1} },
	};

	for (const auto& v : vectors) {
		CSHA sha;
		sha.Reset();
		sha.Add(v.input, std::strlen(v.input));
		sha.Finish();
		uint8 hash[20];
		sha.GetHash(hash);
		if (!hash_eq(hash, v.expected, 20)) {
			std::printf("SHA-1 mismatch for \"%s\"\n", v.input);
			print_hex("got     ", hash, 20);
			print_hex("expected", v.expected, 20);
			return false;
		}
	}
	return true;
}

static bool test_sha1_incremental() {
	CSHA full, inc;
	full.Reset();
	full.Add("Hello World", 11);
	full.Finish();

	inc.Reset();
	inc.Add("Hello", 5);
	inc.Add(" ", 1);
	inc.Add("World", 5);
	inc.Finish();

	uint8 h1[20], h2[20];
	full.GetHash(h1);
	inc.GetHash(h2);
	if (!hash_eq(h1, h2, 20)) {
		std::printf("SHA-1 incremental hashing mismatch\n");
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// SHA-256 (FIPS 180-4)
// ---------------------------------------------------------------------------

static bool test_sha256_vectors() {
	struct { const char* input; uint8 expected[32]; } vectors[] = {
		{ "",
		  {0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,
		   0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55} },
		{ "a",
		  {0xca,0x97,0x81,0x12,0xca,0x1b,0xbd,0xca,0xfa,0xc2,0x31,0xb3,0x9a,0x23,0xdc,0x4d,
		   0xa7,0x86,0xef,0xf8,0x14,0x7c,0x4e,0x72,0xb9,0x80,0x77,0x85,0xaf,0xee,0x48,0xbb} },
		{ "abc",
		  {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
		   0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad} },
		{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		  {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
		   0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1} },
	};

	for (const auto& v : vectors) {
		CSHA256 sha;
		sha.Reset();
		sha.Add(v.input, std::strlen(v.input));
		sha.Finish();
		uint8 hash[32];
		sha.GetHash(hash);
		if (!hash_eq(hash, v.expected, 32)) {
			std::printf("SHA-256 mismatch for \"%s\"\n", v.input);
			print_hex("got     ", hash, 32);
			print_hex("expected", v.expected, 32);
			return false;
		}
	}
	return true;
}

static bool test_sha256_incremental() {
	CSHA256 full, inc;
	full.Reset();
	full.Add("Hello World", 11);
	full.Finish();

	inc.Reset();
	inc.Add("Hello", 5);
	inc.Add(" ", 1);
	inc.Add("World", 5);
	inc.Finish();

	uint8 h1[32], h2[32];
	full.GetHash(h1);
	inc.GetHash(h2);
	if (!hash_eq(h1, h2, 32)) {
		std::printf("SHA-256 incremental hashing mismatch\n");
		return false;
	}
	return true;
}

static bool test_sha256_copy() {
	CSHA256 sha1;
	sha1.Reset();
	sha1.Add("test", 4);

	CSHA256 sha2(sha1);
	if (sha1 != sha2) {
		std::printf("SHA-256 copy constructor broken\n");
		return false;
	}

	CSHA256 sha3;
	sha3 = sha1;
	if (sha1 != sha3) {
		std::printf("SHA-256 assignment operator broken\n");
		return false;
	}
	return true;
}

static bool test_sha256_block_boundary() {
	// Hash exactly one 64-byte block, then the same 64 bytes in two 32-byte chunks.
	// Ensures block boundary handling and buffer reset are correct.
	uint8 block[64];
	for (int i = 0; i < 64; ++i)
		block[i] = static_cast<uint8>('a' + (i % 26));

	CSHA256 full, chunks;
	full.Reset();
	full.Add(block, 64);
	full.Finish();

	chunks.Reset();
	chunks.Add(block, 32);
	chunks.Add(block + 32, 32);
	chunks.Finish();

	uint8 h1[32], h2[32];
	full.GetHash(h1);
	chunks.GetHash(h2);
	if (!hash_eq(h1, h2, 32)) {
		std::printf("SHA-256 block boundary: full block vs two chunks mismatch\n");
		print_hex("full  ", h1, 32);
		print_hex("chunks", h2, 32);
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// ED2K hash (MD4-based, 9500 KiB parts)
// ---------------------------------------------------------------------------

static bool test_ed2k_small_file() {
	// A file smaller than one ED2K part (9500 KiB) should produce
	// the same hash as a plain MD4 of that data.
	const char* data = "The quick brown fox jumps over the lazy dog";
	size_t len = std::strlen(data);

	CMD4 md4;
	md4.Reset();
	md4.Add(data, len);
	md4.Finish();
	uint8 md4_hash[16];
	md4.GetHash(md4_hash);

	CED2K ed2k;
	ed2k.BeginFile(static_cast<uint64>(len));
	ed2k.AddToFile(data, static_cast<uint32>(len));
	if (!ed2k.FinishFile()) {
		std::printf("ED2K FinishFile failed\n");
		return false;
	}
	uint8 ed2k_hash[16];
	ed2k.GetRoot(ed2k_hash);

	if (!hash_eq(md4_hash, ed2k_hash, 16)) {
		std::printf("ED2K single-part file hash != MD4\n");
		print_hex("MD4  ", md4_hash, 16);
		print_hex("ED2K ", ed2k_hash, 16);
		return false;
	}
	return true;
}

static bool test_ed2k_consistency() {
	// Hashing the same data twice must yield the same root hash.
	std::vector<uint8> data(1024 * 100, 0xAB);

	CED2K e1, e2;
	e1.BeginFile(data.size());
	e1.AddToFile(data.data(), static_cast<uint32>(data.size()));
	e1.FinishFile();

	e2.BeginFile(data.size());
	e2.AddToFile(data.data(), static_cast<uint32>(data.size()));
	e2.FinishFile();

	uint8 h1[16], h2[16];
	e1.GetRoot(h1);
	e2.GetRoot(h2);

	if (!hash_eq(h1, h2, 16)) {
		std::printf("ED2K consistency failure\n");
		return false;
	}
	return true;
}

static bool test_ed2k_incremental() {
	// Feeding data in chunks must produce the same result as all at once.
	const size_t total = 50000;
	std::vector<uint8> data(total, 0x42);

	CED2K full;
	full.BeginFile(total);
	full.AddToFile(data.data(), static_cast<uint32>(total));
	full.FinishFile();

	CED2K chunked;
	chunked.BeginFile(total);
	size_t off = 0;
	while (off < total) {
		uint32 chunk = static_cast<uint32>(std::min<size_t>(4096, total - off));
		chunked.AddToFile(data.data() + off, chunk);
		off += chunk;
	}
	chunked.FinishFile();

	uint8 h1[16], h2[16];
	full.GetRoot(h1);
	chunked.GetRoot(h2);

	if (!hash_eq(h1, h2, 16)) {
		std::printf("ED2K incremental mismatch\n");
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

void register_hashlib_tests(TestSuite& suite) {
	// MD4
	suite.add_test("MD4 - RFC 1320 test vectors",   test_md4_vectors);
	suite.add_test("MD4 - incremental hashing",      test_md4_incremental);

	// MD5
	suite.add_test("MD5 - RFC 1321 test vectors",   test_md5_vectors);
	suite.add_test("MD5 - incremental hashing",      test_md5_incremental);

	// SHA-1
	suite.add_test("SHA-1 - FIPS 180-4 vectors",    test_sha1_vectors);
	suite.add_test("SHA-1 - incremental hashing",    test_sha1_incremental);

	// SHA-256
	suite.add_test("SHA-256 - FIPS 180-4 vectors",  test_sha256_vectors);
	suite.add_test("SHA-256 - incremental hashing",  test_sha256_incremental);
	suite.add_test("SHA-256 - copy/assign",          test_sha256_copy);
	suite.add_test("SHA-256 - block boundary",       test_sha256_block_boundary);

	// ED2K
	suite.add_test("ED2K - small file == MD4",       test_ed2k_small_file);
	suite.add_test("ED2K - consistency",             test_ed2k_consistency);
	suite.add_test("ED2K - incremental chunks",      test_ed2k_incremental);
}
