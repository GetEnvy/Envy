//
 // KadFirewall.h
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

#define FW_OPEN		0
#define FW_UNKNOWN	1
#define FW_CLOSED	2

class CKadFirewall
{
public:
	CKadFirewall();
	virtual ~CKadFirewall();

	// Start firewall test
	void StartFirewallTest();

	// Get firewall status
	int GetFirewallStatus() const;

	// Check if we can accept buddy connections
	bool CanAcceptBuddy() const;

private:
	int m_nFirewallStatus;
	bool m_bBuddyAccepted;
	CCriticalSection m_pSection;
};
