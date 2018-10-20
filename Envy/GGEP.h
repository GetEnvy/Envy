//
// GGEP.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2007 and PeerProject 2008-2012
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

#pragma once

#define GGEP_MAGIC			0xC3	// GGEP extension prefix

#define GGEP_HDR_LAST		0x80	// Last extension in GGEP block
#define GGEP_HDR_COBS		0x40	// Whether COBS was used on payload
#define GGEP_HDR_DEFLATE	0x20	// Whether payload was deflated
#define GGEP_HDR_RESERVE	0x10	// Reserved. Must be set to 0.
#define GGEP_HDR_IDLEN		0x0F	// Where ID length is stored

#define GGEP_LEN_MORE		0x80	// Continuation present
#define GGEP_LEN_LAST		0x40	// Last byte
#define GGEP_LEN_MASK		0x3F	// Value

// Other defines in G1Packet.h

class CGGEPBlock;
class CGGEPItem;
class CPacket;


class CGGEPBlock
{
public:
	CGGEPBlock();
	virtual ~CGGEPBlock();

public:
	CGGEPItem*	Add(LPCTSTR pszID);
	CGGEPItem*	Find(LPCTSTR pszID, DWORD nMinLength = 0) const;
	BOOL		ReadFromPacket(CPacket* pPacket);
	void		Write(CPacket* pPacket);

	inline BOOL	IsEmpty() const
	{
		return ( m_nItemCount == 0 );
	}

	inline DWORD GetCount() const
	{
		return m_nItemCount;
	}

	inline CGGEPItem* GetFirst() const
	{
		return m_pFirst;
	}

protected:
	CGGEPItem*	m_pFirst;
	CGGEPItem*	m_pLast;
	BYTE		m_nItemCount;
	const BYTE*	m_pInput;		// Current read position
	DWORD		m_nInput;		// Remaining size available for reading

	void		Clear();
	BOOL		ReadInternal();
	BYTE		ReadByte();
	CGGEPItem*	ReadItem(BYTE nFlags);
};


class CGGEPItem
{
public:
	CGGEPItem(LPCTSTR pszID = NULL);
	virtual ~CGGEPItem();

public:
	CGGEPItem*	m_pNext;
	CString		m_sID;
	BYTE*		m_pBuffer;
	DWORD		m_nLength;
	DWORD		m_nPosition;

	inline BOOL	IsNamed(LPCTSTR pszID) const
	{
		return ( m_sID == pszID );
	}

	void		Read(LPVOID pData, int nLength);
	BYTE		ReadByte();
	void		Write(LPCVOID pData, int nLength);
	void		WriteByte(BYTE nValue);
	void		WriteShort(WORD nValue);
	void		WriteLong(DWORD nValue);
	void		WriteInt64(QWORD nValue);
	void		WriteUTF8(const CString& strText);
	void		WriteVary(QWORD nValue);	// Write variable length (1-8 bytes) according to parameter value
	CString		ToString() const;

protected:
	void		WriteTo(CPacket* pPacket);
	BOOL		Encode();
	BOOL		Decode();
	BOOL		Deflate();
	BOOL		Inflate();

friend class CGGEPBlock;
};
