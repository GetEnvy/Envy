//
// GProfile.h
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

#pragma once

class CXMLElement;
class CG2Packet;


class CGProfile : public CComObject
{
public:
	CGProfile();
//	virtual ~CGProfile();

public:
	CGuarded< Hashes::Guid >	oGUID;					// Gnutella GUID (128 bit)
	CGuarded< Hashes::BtGuid >	oGUIDBT;				// BitTorrent GUID (160 bit), first 128 bits same as oGUID

	void			Create();							// Create default local profile
	BOOL			Load();								// Get local profile from file at Envy start up
	BOOL			Save();								// Save local profile to file
	BOOL			FromXML(const CXMLElement* pXML);	// Create remote profile from XML
	void			Serialize(CArchive& ar, int nVersion = 0);	// Load/Save browsed host profile  (BROWSER_SER_VERSION)
	BOOL			IsValid() const;

	CXMLElement*	GetXML(LPCTSTR pszElement = NULL, BOOL bCreate = FALSE);
	CXMLElement*	GetPublicXML(CString strClient = NULL, BOOL bChallenge = FALSE);
	CString			GetNick() const;
	CString			GetContact(LPCTSTR pszType) const;
	CString			GetLocation() const;
	DWORD			GetPackedGPS() const;

	CG2Packet*		CreateAvatar() const;

protected:
	static LPCTSTR	xmlns;
	static LPCTSTR	xmlnsLegacy;			// Shareaza compatibility
//	CXMLElement*	m_pXML;					// Insecure legacy method
	CAutoPtr< CXMLElement > m_pXML;			// Local profile (file)
	CAutoPtr< CXMLElement > m_pXMLExport;	// Profile for public export  -Recreated from m_pXML by GetXML()

	void			CreateBT();				// Create BitTorrent GUID from Gnutella GUID

	DECLARE_INTERFACE_MAP()
};

extern CGProfile MyProfile;
