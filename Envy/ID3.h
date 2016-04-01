//
// ID3.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2010 and Shareaza 2002-2007
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

#pragma pack(1)

typedef struct
{
	CHAR	szTag[3];
	CHAR	szSongname[30];
	CHAR	szArtist[30];
	CHAR	szAlbum[30];
	CHAR	szYear[4];
	CHAR	szComment[30];
	BYTE	nGenre;
} ID3V1;

#define ID3V1_TAG	"TAG"

typedef struct
{
	CHAR	szTag[3];
	BYTE	nMajorVersion;
	BYTE	nMinorVersion;
	BYTE	nFlags;
	DWORD	nSize;
} ID3V2_HEADER;

#define ID3V2_TAG	"ID3"

#define ID3V2_UNSYNCHRONISED	0x80
#define ID3V2_EXTENDEDHEADER	0x40
#define ID3V2_EXPERIMENTAL		0x20
#define ID3V2_FOOTERPRESENT		0x10
#define ID3V2_KNOWNMASK			0xF0

typedef struct
{
	DWORD	nSize;
	BYTE	nFlags1;
	BYTE	nFlags2;
	DWORD	nPadding;
} ID3V2_EXTENDED_HEADER_1;

typedef struct
{
	DWORD	nSize;
	BYTE	nFlagBytes;
} ID3V2_EXTENDED_HEADER_2;

typedef struct
{
	CHAR	szID[4];
	DWORD	nSize;
	BYTE	nFlags1;
	BYTE	nFlags2;
} ID3V2_FRAME;

typedef struct
{
	CHAR	szID[3];
	BYTE	nSize[3];
} ID3V2_FRAME_2;

#define ID3V2_KNOWNFRAME	0x4F

#define ID3_DESYNC_SIZE(x)	{	\
	x = ( ( x & 0xFF000000 ) >> 3 ) + ( ( x & 0x00FF0000 ) >> 2 ) + \
	    ( ( x & 0x0000FF00 ) >> 1 ) + ( x & 0x000000FF ); }

typedef struct
{
	char cID[4];							// Should equal 'MAC '
	unsigned __int16 nVersion;				// Version number * 1000 (3.81 = 3810)
	unsigned __int16 nCompressionLevel;		// Compression level
	unsigned __int16 nFormatFlags;			// Any format flags (for future use)
	unsigned __int16 nChannels;				// Number of channels (1 or 2)
	unsigned __int32 nSampleRate;			// Sample rate (typically 44100)
	unsigned __int32 nHeaderBytes;			// Bytes after the MAC header that compose the WAV header
	unsigned __int32 nTerminatingBytes;		// Bytes after that raw data (for extended info)
	unsigned __int32 nTotalFrames;			// Number of frames in the file
	unsigned __int32 nFinalFrameBlocks;		// Number of samples in the final frame

	// That data are optional their availability is stored in the nFormatFlags
	// Anyway, we suppose that at least 2 __int32's are required for the plain audio :)
	unsigned __int32 nPeakLevel;
	unsigned __int32 nSeekElements;
} APE_HEADER;

#define APE2_VERSION 3980

typedef struct
{
	char cID[4];                             // Should equal 'MAC '
	unsigned __int16 nVersion;               // Version number * 1000 (3.81 = 3810)
	unsigned __int16 nPadding;               // Padding/reserved (always empty)
	unsigned __int32 nDescriptorBytes;       // Number of descriptor bytes (allows later expansion of this header)
	unsigned __int32 nHeaderBytes;           // Number of header APE_HEADER bytes
	unsigned __int32 nSeekTableBytes;        // Number of bytes of the seek table
	unsigned __int32 nHeaderDataBytes;       // Number of header data bytes (from original file)
	unsigned __int32 nAPEFrameDataBytes;     // Number of bytes of APE frame data
	unsigned __int32 nAPEFrameDataBytesHigh; // High order number of APE frame data bytes
	unsigned __int32 nTerminatingDataBytes;  // Terminating data of the file (not including tag data)
	char cFileMD5[16];                       // MD5 hash of the file
	unsigned __int16 nCompressionLevel;      // Compression level
	unsigned __int16 nFormatFlags;           // Any format flags (for future use)
	unsigned __int32 nBlocksPerFrame;        // Number of audio blocks in one frame
	unsigned __int32 nFinalFrameBlocks;      // Number of audio blocks in the final frame
	unsigned __int32 nTotalFrames;           // Number of frames in the file
	unsigned __int16 nChannels;              // Number of channels (1 or 2)
	unsigned __int16 nBitsPerSample;         // Bits per sample (typically 16)
	unsigned __int32 nSampleRate;            // Sample rate (typically 44100)
} APE_HEADER_NEW;

typedef struct
{
	char	cID[8]; 				// Should equal 'APETAGEX'
	int 	nVersion;				// Equals CURRENT_APE_TAG_VERSION
	int 	nSize;					// The complete size of the tag, including this footer
	int 	nFields;				// The number of fields in the tag
	int 	nFlags;					// The tag flags (none currently defined)
	char	cReserved[8];			// Reserved for later use
} APE_TAG_FOOTER;

// LAME version 3.98
typedef struct
{
	DWORD	VbrTag;
	DWORD	HeaderFlags;
	DWORD	FrameCount;
	DWORD	StreamSize; 			// Include Xing/LAME Header
	BYTE	BitrateTOC[100];
	DWORD	EncQuality;
	CHAR	ClassID[9]; 			// Lame code uses 9 bytes, in comments 20 bytes (?)
	BYTE	VbrMethodRevision;
	BYTE	LowPass;
	DWORD	PeakSignalAmplitude;
	USHORT	RadioReplayGain;
	USHORT	AudiophileReplayGain;
	BYTE	Flags;
	BYTE	AbrBitrate;
	BYTE	EncDelayPadding[3];
	BYTE	Misc;
	BYTE	Unused;
	USHORT	Preset;
	DWORD	MusicLength;
	USHORT	MusicCRC;
	USHORT	FrameCRC;
} LAME_FRAME;

typedef struct
{
	DWORD		dwMicroSecPerFrame;	// Frame display rate (or 0L)
	DWORD		dwMaxBytesPerSec;	// Max. transfer rate
	DWORD		dwPaddingGranularity;	// Pad to multiples of this size; normally 2K.
	DWORD		dwFlags;			// The ever-present flags
	DWORD		dwTotalFrames;		// # frames in file
	DWORD		dwInitialFrames;
	DWORD		dwStreams;
	DWORD		dwSuggestedBufferSize;

	DWORD		dwWidth;
	DWORD		dwHeight;

	DWORD		dwReserved[4];
} AVI_HEADER;

#ifdef _ID3_DEFINE_GENRES
LPCTSTR CLibraryBuilderInternals::pszID3Genre[] =
{
	L"Blues", L"Classic Rock", L"Country", L"Dance", L"Disco",
	L"Funk", L"Grunge", L"Hip-Hop", L"Jazz", L"Metal", L"New Age",
	L"Oldies", L"Other", L"Pop", L"R&B", L"Rap", L"Reggae",
	L"Rock", L"Techno", L"Industrial", L"Alternative", L"Ska",
	L"Death Metal", L"Pranks", L"Soundtrack", L"Euro-Techno", L"Ambient",
	L"Trip-Hop", L"Vocal", L"Jazz+Funk", L"Fusion", L"Trance",
	L"Classical", L"Instrumental", L"Acid", L"House", L"Game",
	L"Sound Clip", L"Gospel", L"Noise", L"AlternRock", L"Bass",
	L"Soul", L"Punk", L"Space", L"Meditative", L"Instrumental Pop",
	L"Instrumental Rock", L"Ethnic", L"Gothic", L"Darkwave",
	L"Techno-Industrial", L"Electronic", L"Pop-Folk", L"Eurodance",
	L"Dream", L"Southern Rock", L"Comedy", L"Cult", L"Gangsta", L"Top 40",
	L"Christian Rap", L"Pop/Funk", L"Jungle", L"Native American", L"Cabaret",
	L"New Wave", L"Psychadelic", L"Rave", L"Showtunes", L"Trailer",
	L"Lo-Fi", L"Tribal", L"Acid Punk", L"Acid Jazz", L"Polka", L"Retro",
	L"Musical", L"Rock & Roll", L"Hard Rock", L"Folk", L"Folk/Rock",
	L"National Folk", L"Swing", L"Easy Listening", L"Bebob", L"Latin", L"Revival",
	L"Celtic", L"Bluegrass", L"Avantgarde", L"Gothic Rock",
	L"Progressive Rock", L"Psychedelic Rock", L"Symphonic Rock",
	L"Slow Rock", L"Big Band", L"Chorus", L"Easy Listening",
	L"Acoustic", L"Humour", L"Speech", L"Chanson", L"Opera",
	L"Chamber Music", L"Sonata", L"Symphony", L"Booty Bass",
	L"Primus", L"Porn Groove", L"Satire", L"Slow Jam", L"Club",
	L"Tango", L"Samba", L"Folklore", L"Ballad", L"Power Ballad",
	L"Rhythmic Soul", L"Freestyle", L"Duet", L"Punk Rock", L"Drum Solo",
	L"A capella", L"Euro-House", L"Dance Hall", L"Goa", L"Drum & Bass",
	L"Club-House", L"Hardcore", L"Terror", L"Indie", L"BritPop", L"Negerpunk",
	L"Polsk Punk", L"Beat", L"Christian Gangsta", L"Heavy Metal", L"Black Metal",
	L"Crossover", L"Contemporary C", L"Christian Rock", L"Merengue",
	L"Salsa", L"Thrash Metal", L"Anime", L"JPop", L"SynthPop", L"DubStep"
};
#endif

#define ID3_GENRES	148

#pragma pack()
