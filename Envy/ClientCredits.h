//
 // ClientCredits.h
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

class CClientCredits
{
public:
	CClientCredits();
	CClientCredits(const Hashes::Guid& oGUID);
	virtual ~CClientCredits();

	// Get credit for a client
	CClientCredits* GetCredit(const Hashes::Guid& oGUID);

	// Load/save credits
	void LoadCredits();
	void SaveCredits();

	// Credit calculation
	float GetRatio(const Hashes::Guid& oGUID) const;
	void AddUpload(const Hashes::Guid& oGUID, QWORD nBytes);
	void AddDownload(const Hashes::Guid& oGUID, QWORD nBytes);

	// Credit-based ranking
	int GetRank(const Hashes::Guid& oGUID) const;

private:
	struct CreditEntry
	{
		Hashes::Guid oGUID;
		QWORD nUploaded;
		QWORD nDownloaded;
		float nRatio;
		int nRank;
	};

	CMap<Hashes::Guid, const Hashes::Guid&, CreditEntry*, CreditEntry*&> m_Credits;
	CCriticalSection m_pSection;
};
