//
// SHA256.h
//
// SHA-256 hash implementation for BitTorrent v2 (BEP-52)
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)
//

#pragma once

#include "Utility.hpp"

#ifndef HASHLIB_API
#ifdef HASHLIB_EXPORTS
#define HASHLIB_API __declspec(dllexport)
#else
#define HASHLIB_API __declspec(dllimport)
#endif
#endif

class HASHLIB_API CSHA256
{
public:
	CSHA256();
	~CSHA256() {}

	// Default copy and move operations (Rule of Five)
	// Hash state is POD, so default copy/move is safe
	CSHA256(const CSHA256&) = default;
	CSHA256& operator=(const CSHA256&) = default;
	CSHA256(CSHA256&&) = default;
	CSHA256& operator=(CSHA256&&) = default;

public:
	void Reset();
	void Finish();
	void Add(const void* pData, size_t nLength);

	struct Digest // 256 bit (32 bytes)
	{
		uint32& operator[](size_t i) { return data[i]; }
		const uint32& operator[](size_t i) const { return data[i]; }
		uint32 data[8]; // 8 * 32-bit = 256-bit hash
	};

	void GetHash(__in_bcount(32) uint8* pHash) const;

	// Comparison operators
	bool operator==(const CSHA256& other) const;
	bool operator!=(const CSHA256& other) const;

private:
	struct SHA256State
	{
		static const size_t blockSize = 64; // 512 bits
		uint64 m_nCount;                    // Message length in bits
		uint32 m_nState[8];                 // Hash state (H0-H7)
		uint8 m_oBuffer[blockSize];         // Input buffer
	};

	SHA256State m_State;

	// SHA-256 constants
	static const uint32 K[64];

	// Helper functions
	void Transform(const uint32* data);
	uint32 RotateRight(uint32 value, int amount) const;
	uint32 Ch(uint32 x, uint32 y, uint32 z) const;
	uint32 Maj(uint32 x, uint32 y, uint32 z) const;
	uint32 Sigma0(uint32 x) const;
	uint32 Sigma1(uint32 x) const;
	uint32 sigma0(uint32 x) const;
	uint32 sigma1(uint32 x) const;
};
