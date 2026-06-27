//
// PageSettingsDonkeyAdvanced.cpp
//
// Advanced ED2K Protocol Settings Page Implementation
//
// This file is part of Envy (getenvy.com) © 2016-2026
//

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "WndSettingsSheet.h"
#include "PageSettingsDonkeyAdvanced.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDonkeyAdvancedSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CDonkeyAdvancedSettingsPage, CSettingsPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDonkeyAdvancedSettingsPage property page

CDonkeyAdvancedSettingsPage::CDonkeyAdvancedSettingsPage()
	: CSettingsPage(CDonkeyAdvancedSettingsPage::IDD)
	, m_bEnableAICH(FALSE)
	, m_bAICHTrustEveryHash(FALSE)
	, m_nAICHHashSetTimeout(0)
	, m_bAICHRecover(FALSE)
	, m_bEnableKadHello(FALSE)
	, m_bKadFindValue(FALSE)
	, m_nKadHelloTimeout(0)
	, m_nKadFindValueTimeout(0)
	, m_bEnableMultiPacketExt2(FALSE)
	, m_bEnableHashSetRequest2(FALSE)
	, m_bPreferIPv6(FALSE)
	, m_bEnableDualStack(FALSE)
	, m_nIPv6ConnectTimeout(0)
{
}

CDonkeyAdvancedSettingsPage::~CDonkeyAdvancedSettingsPage()
{
}

void CDonkeyAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	// AICH Settings
	DDX_Check(pDX, IDC_ED2K_ENABLE_AICH, m_bEnableAICH);
	DDX_Check(pDX, IDC_ED2K_AICH_TRUST_EVERY_HASH, m_bAICHTrustEveryHash);
	DDX_Text(pDX, IDC_ED2K_AICH_HASHSET_TIMEOUT, m_nAICHHashSetTimeout);
	DDX_Check(pDX, IDC_ED2K_AICH_RECOVER, m_bAICHRecover);

	// Kademlia Settings
	DDX_Check(pDX, IDC_ED2K_ENABLE_KAD_HELLO, m_bEnableKadHello);
	DDX_Check(pDX, IDC_ED2K_KAD_FIND_VALUE, m_bKadFindValue);
	DDX_Text(pDX, IDC_ED2K_KAD_HELLO_TIMEOUT, m_nKadHelloTimeout);
	DDX_Text(pDX, IDC_ED2K_KAD_FIND_VALUE_TIMEOUT, m_nKadFindValueTimeout);

	// Protocol Extensions
	DDX_Check(pDX, IDC_ED2K_ENABLE_MULTIPACKET_EXT2, m_bEnableMultiPacketExt2);
	DDX_Check(pDX, IDC_ED2K_ENABLE_HASHSET_REQUEST2, m_bEnableHashSetRequest2);

	// Network Settings
	DDX_Check(pDX, IDC_ED2K_PREFER_IPV6, m_bPreferIPv6);
	DDX_Check(pDX, IDC_ED2K_ENABLE_DUAL_STACK, m_bEnableDualStack);
	DDX_Text(pDX, IDC_ED2K_IPV6_CONNECT_TIMEOUT, m_nIPv6ConnectTimeout);
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeyAdvancedSettingsPage message handlers

BOOL CDonkeyAdvancedSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	// Load current settings
	m_bEnableAICH = Settings.eDonkey.EnableAICH;
	m_bAICHTrustEveryHash = Settings.eDonkey.AICHTrustEveryHash;
	m_nAICHHashSetTimeout = Settings.eDonkey.AICHHashSetTimeout;
	m_bAICHRecover = Settings.eDonkey.AICHRecover;

	m_bEnableKadHello = Settings.eDonkey.EnableKadHello;
	m_bKadFindValue = Settings.eDonkey.KadFindValue;
	m_nKadHelloTimeout = Settings.eDonkey.KadHelloTimeout;
	m_nKadFindValueTimeout = Settings.eDonkey.KadFindValueTimeout;

	m_bEnableMultiPacketExt2 = Settings.eDonkey.EnableMultiPacketExt2;
	m_bEnableHashSetRequest2 = Settings.eDonkey.EnableHashSetRequest2;

	m_bPreferIPv6 = Settings.eDonkey.PreferIPv6;
	m_bEnableDualStack = Settings.eDonkey.EnableDualStack;
	m_nIPv6ConnectTimeout = Settings.eDonkey.IPv6ConnectTimeout;

	UpdateData(FALSE);

	return TRUE;
}

BOOL CDonkeyAdvancedSettingsPage::OnSetActive()
{
	CSettingsPage::OnSetActive();

	// Update controls based on settings
	UpdateData(FALSE);

	return TRUE;
}

void CDonkeyAdvancedSettingsPage::OnOK()
{
	UpdateData();

	// Validate settings before saving
	if (!ValidateSettings()) {
		// Validation failed, don't save
		return;
	}

	// Save settings
	Settings.eDonkey.EnableAICH = m_bEnableAICH;
	Settings.eDonkey.AICHTrustEveryHash = m_bAICHTrustEveryHash;
	Settings.eDonkey.AICHHashSetTimeout = m_nAICHHashSetTimeout;
	Settings.eDonkey.AICHRecover = m_bAICHRecover;

	Settings.eDonkey.EnableKadHello = m_bEnableKadHello;
	Settings.eDonkey.KadFindValue = m_bKadFindValue;
	Settings.eDonkey.KadHelloTimeout = m_nKadHelloTimeout;
	Settings.eDonkey.KadFindValueTimeout = m_nKadFindValueTimeout;

	Settings.eDonkey.EnableMultiPacketExt2 = m_bEnableMultiPacketExt2;
	Settings.eDonkey.EnableHashSetRequest2 = m_bEnableHashSetRequest2;

	Settings.eDonkey.PreferIPv6 = m_bPreferIPv6;
	Settings.eDonkey.EnableDualStack = m_bEnableDualStack;
	Settings.eDonkey.IPv6ConnectTimeout = m_nIPv6ConnectTimeout;

	CSettingsPage::OnOK();
}

bool CDonkeyAdvancedSettingsPage::ValidateSettings()
{
	CString strError;

	// Validate AICH timeout
	if (m_nAICHHashSetTimeout < 5 || m_nAICHHashSetTimeout > 300) {
		strError = L"AICH Hash Set Timeout must be between 5 and 300 seconds.";
		AfxMessageBox(strError, MB_ICONEXCLAMATION);
		return false;
	}

	// Validate Kad timeouts
	if (m_nKadHelloTimeout < 1 || m_nKadHelloTimeout > 60) {
		strError = L"Kad Hello Timeout must be between 1 and 60 seconds.";
		AfxMessageBox(strError, MB_ICONEXCLAMATION);
		return false;
	}

	if (m_nKadFindValueTimeout < 1 || m_nKadFindValueTimeout > 120) {
		strError = L"Kad Find Value Timeout must be between 1 and 120 seconds.";
		AfxMessageBox(strError, MB_ICONEXCLAMATION);
		return false;
	}

	// Validate IPv6 timeout
	if (m_nIPv6ConnectTimeout < 5 || m_nIPv6ConnectTimeout > 300) {
		strError = L"IPv6 Connect Timeout must be between 5 and 300 seconds.";
		AfxMessageBox(strError, MB_ICONEXCLAMATION);
		return false;
	}

	// Warn about potentially unsafe combinations
	if (m_bAICHTrustEveryHash && m_bEnableAICH) {
		CString strWarning = L"Warning: Trusting every AICH hash reduces security. Only enable this if you trust all sources.";
		if (AfxMessageBox(strWarning, MB_ICONWARNING | MB_OKCANCEL) != IDOK) {
			return false; // User cancelled
		}
	}

	// Validate logical dependencies
	if (m_bKadFindValue && !m_bEnableKadHello) {
		CString strWarning = L"Warning: Kad Find Value requires Kad Hello to be enabled for proper operation.";
		if (AfxMessageBox(strWarning, MB_ICONWARNING | MB_OKCANCEL) != IDOK) {
			return false; // User cancelled
		}
	}

	return true;
}
