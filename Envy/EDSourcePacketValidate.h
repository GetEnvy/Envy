//
// EDSourcePacketValidate.h
//
// Pure ED2K Source Exchange packet-boundary checks (no network / MFC).
// Shared by EDClient handlers and EnvyTests smoke tests.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#pragma once

#include <windows.h>

// Declared source count vs bytes remaining (SourceEx v1/v2 answer bodies).
inline BOOL Ed2kValidateSourcePacketBody(DWORD nRemaining, DWORD nCount, DWORD nSourceSize)
{
	if ( nSourceSize == 0 )
		return FALSE;

	if ( nCount > ( nRemaining / nSourceSize ) )
		return FALSE;

	return nRemaining >= nCount * nSourceSize;
}

// <HASH 16><FileSizeLow 4><Options 2> minimum before optional legacy high32.
inline constexpr DWORD Ed2kSourceEx2RequestMinBytes()
{
	return 16u + 4u + 2u;
}

// <HASH 16><Count 2> minimum before source records.
inline constexpr DWORD Ed2kSourceEx2AnswerMinBytes()
{
	return 16u + 2u;
}

// Per-source record size in SourceEx2 answers (IPv4 + GUID).
inline constexpr DWORD Ed2kSourceEx2SourceRecordBytes()
{
	return 4u + 2u + 4u + 2u + 16u;
}

// Legacy 64-bit size encoding: optional high32 dword when Low32==0 and room remains.
inline DWORD Ed2kSourceEx2LegacyHigh32Bytes(DWORD nFileSizeLow, DWORD nRemainingAfterLow)
{
	// eMule-inherited ambiguity: Low32==0 may introduce a legacy high32 before Options.
	if ( nFileSizeLow == 0 && nRemainingAfterLow >= 6 )
		return 4;

	return 0;
}

// Backward-compatible wrapper for existing call sites.
inline DWORD Ed2kSourceEx2ConsumeLegacyHigh32(DWORD nFileSizeLow, DWORD nRemainingAfterLow)
{
	return Ed2kSourceEx2LegacyHigh32Bytes( nFileSizeLow, nRemainingAfterLow );
}

// Options ushort must still be present after size fields.
inline BOOL Ed2kSourceEx2HasOptionsBytes(DWORD nRemainingAfterSizeFields)
{
	return nRemainingAfterSizeFields >= 2;
}
