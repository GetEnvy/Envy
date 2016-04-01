//
// DownloadWithTransfers.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
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

#include "DownloadWithSources.h"

class CConnection;
class CEDClient;
class CDownloadTransfer;


class CDownloadWithTransfers : public CDownloadWithSources
{
protected:
	CDownloadWithTransfers();
	virtual ~CDownloadWithTransfers();

private:
	CDownloadTransfer*	m_pTransferFirst;
	CDownloadTransfer*	m_pTransferLast;
	int					m_nTransferCount;
	DWORD				m_tTransferStart;

public:
	bool		HasActiveTransfers() const;
	DWORD		GetTransferCount() const;
	DWORD		GetTransferCount(int nState, const IN_ADDR* pAddress = NULL) const;
	QWORD		GetAmountDownloadedFrom(const IN_ADDR* pAddress) const;
	DWORD		GetAverageSpeed() const;
	DWORD		GetMeasuredSpeed() const;
	BOOL		OnAcceptPush(const Hashes::Guid& oClientID, CConnection* pConnection);
	BOOL		OnDonkeyCallback(const CEDClient* pClient, CDownloadSource* pExcept = NULL);
	BOOL		StartNewTransfer(DWORD tNow = 0);
	BOOL		CanStartTransfers(DWORD tNow);
	void		CloseTransfers();
protected:
	BOOL		StartTransfersIfNeeded(DWORD tNow = 0);
private:
	void		AddTransfer(CDownloadTransfer* pTransfer);
	void		RemoveTransfer(CDownloadTransfer* pTransfer);

// Inlines
public:
	inline bool ValidTransfer(const IN_ADDR* pAddress, const CDownloadTransfer* pTransfer) const;
	inline CDownloadTransfer* GetFirstTransfer() const { return m_pTransferFirst; }

	friend class CDownloadTransfer;		// AddTransfer && RemoveTransfer
};
