//
// SHA256.cpp
//
// SHA-256 hash implementation for BitTorrent v2 (BEP-52)
//
// This file is part of Envy (getenvy.com) © 2016-2026
//
// Based on FIPS 180-4 Secure Hash Standard (SHS)
//
//

#include "StdAfx.h"
#include "SHA256.h"
#include <algorithm>

// SHA-256 constants (first 32 bits of fractional parts of cube roots of first 64 primes)
const uint32 CSHA256::K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// SHA-256 initial hash values (first 32 bits of fractional parts of square roots of first 8 primes)
static const uint32 SHA256_INITIAL_STATE[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

CSHA256::CSHA256()
{
	Reset();
}

void CSHA256::Reset()
{
	m_State.m_nCount = 0;
	// Initialize hash state with SHA-256 initial values
	std::copy(SHA256_INITIAL_STATE, SHA256_INITIAL_STATE + 8, m_State.m_nState);
	std::fill_n(m_State.m_oBuffer, SHA256State::blockSize, static_cast<uint8>(0));
}

void CSHA256::Add(const void* pData, size_t nLength)
{
	const uint8* data = static_cast<const uint8*>(pData);
	size_t bytesProcessed = 0;

	// Buffer position = bytes already in buffer (before this call)
	size_t bufferPos = static_cast<size_t>((m_State.m_nCount / 8) % SHA256State::blockSize);

	// Update bit count
	m_State.m_nCount += nLength * 8; // Convert to bits

	while (bytesProcessed < nLength) {
		size_t bytesToCopy = (std::min)(nLength - bytesProcessed,
			static_cast<size_t>(SHA256State::blockSize - bufferPos));

		std::copy(data + bytesProcessed, data + bytesProcessed + bytesToCopy,
			m_State.m_oBuffer + bufferPos);

		bytesProcessed += bytesToCopy;
		bufferPos += bytesToCopy;

		if (bufferPos == SHA256State::blockSize) {
			// Buffer is full, process it
			uint32 w[64];

			// Prepare message schedule
			for (int i = 0; i < 16; ++i) {
				w[i] = (static_cast<uint32>(m_State.m_oBuffer[i * 4]) << 24) |
					   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 1]) << 16) |
					   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 2]) << 8) |
					   static_cast<uint32>(m_State.m_oBuffer[i * 4 + 3]);
			}

			for (int i = 16; i < 64; ++i) {
				w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
			}

			Transform(w);
			bufferPos = 0;
		}
	}
}

void CSHA256::Finish()
{
	// Calculate padding length
	uint64 bitLength = m_State.m_nCount;
	size_t bufferPos = (bitLength / 8) % SHA256State::blockSize;

	// Empty message: ensure buffer is fully defined (no reliance on prior state)
	if (bitLength == 0) {
		std::fill_n(m_State.m_oBuffer, SHA256State::blockSize, static_cast<uint8>(0));
		bufferPos = 0;
	}

	// Append padding
	m_State.m_oBuffer[bufferPos++] = 0x80; // Append '1' bit

	// If not enough space in this block for the 64-bit length, pad and process this block first
	if (bufferPos > SHA256State::blockSize - 8) {
		std::fill(m_State.m_oBuffer + bufferPos, m_State.m_oBuffer + SHA256State::blockSize, static_cast<uint8>(0));
		uint32 w[64];
		for (int i = 0; i < 16; ++i) {
			w[i] = (static_cast<uint32>(m_State.m_oBuffer[i * 4]) << 24) |
				   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 1]) << 16) |
				   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 2]) << 8) |
				   static_cast<uint32>(m_State.m_oBuffer[i * 4 + 3]);
		}
		for (int i = 16; i < 64; ++i) {
			w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
		}
		Transform(w);
		std::fill_n(m_State.m_oBuffer, SHA256State::blockSize - 8, static_cast<uint8>(0));
	} else {
		std::fill(m_State.m_oBuffer + bufferPos, m_State.m_oBuffer + SHA256State::blockSize - 8, static_cast<uint8>(0));
	}

	// Append 64-bit message length in bits, big-endian (MSB first in buffer)
	uint64 lengthBE = bitLength;
	for (int i = 0; i < 8; ++i) {
		m_State.m_oBuffer[SHA256State::blockSize - 8 + i] = static_cast<uint8>((lengthBE >> (56 - i * 8)) & 0xFF);
	}

	// Final transform
	uint32 w[64];
	for (int i = 0; i < 16; ++i) {
		w[i] = (static_cast<uint32>(m_State.m_oBuffer[i * 4]) << 24) |
			   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 1]) << 16) |
			   (static_cast<uint32>(m_State.m_oBuffer[i * 4 + 2]) << 8) |
			   static_cast<uint32>(m_State.m_oBuffer[i * 4 + 3]);
	}
	for (int i = 16; i < 64; ++i) {
		w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
	}
	Transform(w);
}

void CSHA256::GetHash(__in_bcount(32) uint8* pHash) const
{
	// Output each 32-bit state word in big-endian byte order (MSB first)
	for (int i = 0; i < 8; ++i) {
		uint32 word = m_State.m_nState[i];
		pHash[i * 4 + 0] = static_cast<uint8>((word >> 24) & 0xFF);
		pHash[i * 4 + 1] = static_cast<uint8>((word >> 16) & 0xFF);
		pHash[i * 4 + 2] = static_cast<uint8>((word >> 8) & 0xFF);
		pHash[i * 4 + 3] = static_cast<uint8>(word & 0xFF);
	}
}

bool CSHA256::operator==(const CSHA256& other) const
{
	return std::equal(m_State.m_nState, m_State.m_nState + 8, other.m_State.m_nState) &&
		   m_State.m_nCount == other.m_State.m_nCount &&
		   std::equal(m_State.m_oBuffer, m_State.m_oBuffer + SHA256State::blockSize, other.m_State.m_oBuffer);
}

bool CSHA256::operator!=(const CSHA256& other) const
{
	return !(*this == other);
}

void CSHA256::Transform(const uint32* w)
{
	uint32 a = m_State.m_nState[0];
	uint32 b = m_State.m_nState[1];
	uint32 c = m_State.m_nState[2];
	uint32 d = m_State.m_nState[3];
	uint32 e = m_State.m_nState[4];
	uint32 f = m_State.m_nState[5];
	uint32 g = m_State.m_nState[6];
	uint32 h = m_State.m_nState[7];

	for (int i = 0; i < 64; ++i) {
		uint32 t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + w[i];
		uint32 t2 = Sigma0(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	m_State.m_nState[0] += a;
	m_State.m_nState[1] += b;
	m_State.m_nState[2] += c;
	m_State.m_nState[3] += d;
	m_State.m_nState[4] += e;
	m_State.m_nState[5] += f;
	m_State.m_nState[6] += g;
	m_State.m_nState[7] += h;
}

// SHA-256 helper functions
uint32 CSHA256::RotateRight(uint32 value, int amount) const
{
	return (value >> amount) | (value << (32 - amount));
}

uint32 CSHA256::Ch(uint32 x, uint32 y, uint32 z) const
{
	return (x & y) ^ (~x & z);
}

uint32 CSHA256::Maj(uint32 x, uint32 y, uint32 z) const
{
	return (x & y) ^ (x & z) ^ (y & z);
}

uint32 CSHA256::Sigma0(uint32 x) const
{
	return RotateRight(x, 2) ^ RotateRight(x, 13) ^ RotateRight(x, 22);
}

uint32 CSHA256::Sigma1(uint32 x) const
{
	return RotateRight(x, 6) ^ RotateRight(x, 11) ^ RotateRight(x, 25);
}

uint32 CSHA256::sigma0(uint32 x) const
{
	return RotateRight(x, 7) ^ RotateRight(x, 18) ^ (x >> 3);
}

uint32 CSHA256::sigma1(uint32 x) const
{
	return RotateRight(x, 17) ^ RotateRight(x, 19) ^ (x >> 10);
}
