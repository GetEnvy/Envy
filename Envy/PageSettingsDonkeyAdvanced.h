//
// PageSettingsDonkeyAdvanced.h
//
// Advanced ED2K Protocol Settings Page
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#pragma once

#include "WndSettingsPage.h"

class CDonkeyAdvancedSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CDonkeyAdvancedSettingsPage)

public:
	CDonkeyAdvancedSettingsPage();
	virtual ~CDonkeyAdvancedSettingsPage();

	enum { IDD = IDD_SETTINGS_DONKEY_ADVANCED };

public:
	// AICH Settings
	BOOL m_bEnableAICH;
	BOOL m_bAICHTrustEveryHash;
	int m_nAICHHashSetTimeout;
	BOOL m_bAICHRecover;

	// Kademlia Settings
	BOOL m_bEnableKadHello;
	BOOL m_bKadFindValue;
	int m_nKadHelloTimeout;
	int m_nKadFindValueTimeout;

	// Protocol Extensions
	BOOL m_bEnableMultiPacketExt2;
	BOOL m_bEnableHashSetRequest2;

	// Network Settings
	BOOL m_bPreferIPv6;
	BOOL m_bEnableDualStack;
	int m_nIPv6ConnectTimeout;

public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();

private:
	bool ValidateSettings();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};
