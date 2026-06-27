//
 // KadIndex.h
//
// This file is part of Envy (getenvy.com) � 2016-2020
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
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

struct SourceInfo
{
	Hashes::Guid oGUID;
	IN_ADDR pAddress;
	WORD nPort;
};

struct NoteInfo
{
	Hashes::Guid oGUID;
	CString sNote;
	DWORD nTime;
};

class CKadSourceEntry
{
public:
	CKadSourceEntry(const Hashes::Guid& oTarget);
	virtual ~CKadSourceEntry();

	CArray<SourceInfo> m_Sources;
};

class CKadNotesEntry
{
public:
	CKadNotesEntry(const Hashes::Guid& oTarget);
	virtual ~CKadNotesEntry();

	CArray<NoteInfo> m_Notes;
};

class CKadIndex
{
public:
	CKadIndex();
	virtual ~CKadIndex();

	// Add source to index
	void AddSource(const Hashes::Guid& oTarget, const SourceInfo& oSource);

	// Get sources for target
	bool GetSources(const Hashes::Guid& oTarget, CArray<SourceInfo>& oSources);

	// Add notes to index
	void AddNotes(const Hashes::Guid& oTarget, const NoteInfo& oNote);

	// Get notes for target
	bool GetNotes(const Hashes::Guid& oTarget, CArray<NoteInfo>& oNotes);

private:
	CMap<Hashes::Guid, Hashes::Guid&, CKadSourceEntry*, CKadSourceEntry*&> m_Sources;
	CMap<Hashes::Guid, Hashes::Guid&, CKadNotesEntry*, CKadNotesEntry*&> m_Notes;
	CCriticalSection m_pSection;
};
