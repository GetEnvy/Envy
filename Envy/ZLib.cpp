//
// ZLib.cpp
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2014
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

// CZLib makes it easier to use the zlib compression library
// http://shareaza.sourceforge.net/mediawiki/index.php/Developers.Code.CZLib
// http://getenvy.com/archives/shareazawiki/Developers.Code.CZLib.html

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

//////////////////////////////////////////////////////////////////////
// CZLib compression

// Takes a pointer to memory and how many bytes are there
// Compresses the memory into a new buffer this function allocates
// Returns a pointer to the new buffer, and writes its size under pnOutput

auto_array< BYTE > CZLib::Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	if ( ! nInput )
	{
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	// Use nSuggest as the output buffer size if given,
	// Otherwise call compressBound to set it just using math to guess, it doesn't look at the data
	*pnOutput = nSuggest ? nSuggest : compressBound( nInput );

	// Allocate a new buffer of pnOutput bytes
	auto_array< BYTE > pBuffer( new BYTE[ *pnOutput ] );
	if ( ! pBuffer.get() )
	{
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	int nRes = compress2( pBuffer.get(), pnOutput, (const BYTE *)pInput, nInput, Settings.Connection.ZLibCompressionLevel );

	if ( nRes != Z_OK )
	{
		// The compress function reported error
		ASSERT( Z_BUF_ERROR != nRes );	// ToDo: Error
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	// The pBuffer buffer is too big, make a new one exactly the right size, copy the data, delete the first, and return the second
	//auto_array< BYTE > pOutput( new BYTE[ *pnOutput ] );	// Allocate a new buffer exact size to hold the bytes compress wrote
	//memcpy( pOutput.get(), pBuffer.get(), *pnOutput );	// Copy the compressed bytes from the old buffer to the new one
	//return pOutput;										// Return new buffer

	return pBuffer;
}

BYTE* CZLib::Compress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	if ( ! nInput )
	{
		*pnOutput = 0;
		return NULL;
	}

	*pnOutput = nSuggest ? nSuggest : compressBound( nInput );

	// Allocate a new buffer of pnOutput bytes
	BYTE* pBuffer = (BYTE*)malloc( *pnOutput );
	if ( ! pBuffer )
	{
		*pnOutput = 0;
		return NULL;
	}

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	int nRes = compress2( pBuffer, pnOutput, (const BYTE *)pInput, nInput, Settings.Connection.ZLibCompressionLevel );
	if ( nRes != Z_OK )
	{
		// The compress function reported error
		ASSERT( Z_BUF_ERROR != nRes );	// ToDo: Error
		free( pBuffer );
		*pnOutput = 0;
		return NULL;
	}

	return pBuffer;
}


//////////////////////////////////////////////////////////////////////
// CZLib decompression

// Takes a pointer to compressed input bytes, and how many are there
// Decompresses the memory into a new buffer this function allocates
// Returns a pointer to the new buffer, and writes its size under pnOutput

auto_array< BYTE > CZLib::Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput)
{
	// Guess how big the data will be decompressed, use nSuggest, or just guess it will be 4 times as big
	for ( DWORD nSuggest = nInput * 4; ; nSuggest *= 2 )
	{
		*pnOutput = nSuggest;

		auto_array< BYTE > pBuffer( new BYTE[ *pnOutput ] );
		if ( ! pBuffer.get() )
		{
			// Out of memory
			*pnOutput = 0;
			return auto_array< BYTE >();
		}

		// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
		int nRes = uncompress( pBuffer.get(), pnOutput, (const BYTE *)pInput, nInput );

		if ( Z_OK == nRes )
			return pBuffer;

		if ( Z_BUF_ERROR != nRes )
		{
			// Decompression error
			*pnOutput = 0;
			return auto_array< BYTE >();
		}
	}

	// The pBuffer buffer is bigger than necessary, move its bytes into one perfectly sized, and return it
	//auto_array< BYTE > pOutput( new BYTE[ *pnOutput ] );	// Make a new buffer exactly the right size
	//memcpy( pOutput.get(), pBuffer, *pnOutput );			// Copy the data from the one that's too big
	//delete [] pBuffer;
	//return pOutput;										// Return a pointer to the perfectly sized one
}

BYTE* CZLib::Decompress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput)
{
	BYTE* pBuffer = NULL;

	// Guess how big the data will be decompressed, use nSuggest, or just guess it will be 4 times as big
	for ( DWORD nSuggest = nInput * 4; ; nSuggest *= 2 )
	{
		*pnOutput = nSuggest;

		BYTE* pNewBuffer = (BYTE*)realloc( pBuffer, *pnOutput );
		if ( ! pNewBuffer )
		{
			// Out of memory
			free( pBuffer );
			*pnOutput = 0;
			return NULL;
		}
		pBuffer = pNewBuffer;

		// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
		int nRes = uncompress( pBuffer, pnOutput, (const BYTE *)pInput, nInput );
		if ( Z_OK == nRes )
			return pBuffer;

		if ( Z_BUF_ERROR != nRes )
		{
			// Decompression error
			free( pBuffer );
			*pnOutput = 0;
			return NULL;
		}
	}
}
