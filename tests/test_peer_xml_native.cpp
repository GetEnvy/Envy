// SPDX-License-Identifier: AGPL-3.0-or-later
// Executes the production MFC XML parser and HTTP receive loop.

#include "../Envy/StdAfx.h"
#include "../Envy/XML.h"
#include "../Envy/XmlParseValidate.h"
#include "../Envy/DownloadTransferHttpValidate.h"
#include "test_framework.h"

static CString NestedXml(unsigned nDepth)
{
	CString sXml;
	for (unsigned n = 0; n < nDepth; ++n) sXml += L"<a>";
	for (unsigned n = 0; n < nDepth; ++n) sXml += L"</a>";
	return sXml;
}

static bool test_native_xml_valid_matches_local_parser()
{
	const CString sXml = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!--peer--><root name=\"a&amp;b\"><child>hello</child><![CDATA[world]]></root>";
	CAutoPtr<CXMLElement> pPeer(CXMLElement::FromPeerString(sXml, TRUE));
	CAutoPtr<CXMLElement> pLocal(CXMLElement::FromString(sXml, TRUE));
	return pPeer && pLocal && pPeer->Equals(pLocal);
}

static bool test_native_xml_depth_boundary()
{
	CAutoPtr<CXMLElement> pMax(CXMLElement::FromPeerString(NestedXml(XML_PEER_PARSE_DEPTH_MAX)));
	CAutoPtr<CXMLElement> pOver(CXMLElement::FromPeerString(NestedXml(XML_PEER_PARSE_DEPTH_MAX + 1)));
	CAutoPtr<CXMLElement> pAttack(CXMLElement::FromPeerString(NestedXml(4096)));
	CAutoPtr<CXMLElement> pLocal(CXMLElement::FromString(NestedXml(XML_PEER_PARSE_DEPTH_MAX + 1)));
	return pMax && !pOver && !pAttack && pLocal;
}

static bool test_native_xml_nodes_and_attributes()
{
	CString sXml = L"<root>";
	for (unsigned n = 1; n < XML_PEER_PARSE_NODES_MAX; ++n) sXml += L"<a/>";
	CAutoPtr<CXMLElement> pMax(CXMLElement::FromPeerString(sXml + L"</root>"));
	CAutoPtr<CXMLElement> pOver(CXMLElement::FromPeerString(sXml + L"<a/></root>"));
	XmlParseBudget oBudget(32, 3, 1024);
	const CString sAttrs = L"<root a=\"1\" b=\"2\"/>";
	if (!ChargeSharedPeerXmlChars(oBudget, sAttrs.GetLength())) return false;
	CAutoPtr<CXMLElement> pAttrs(CXMLElement::FromPeerString(sAttrs, FALSE, NULL, &oBudget));
	if (!ChargeSharedPeerXmlChars(oBudget, 4)) return false;
	CAutoPtr<CXMLElement> pNext(CXMLElement::FromPeerString(L"<a/>", FALSE, NULL, &oBudget));
	return pMax && !pOver && pAttrs && !pNext && oBudget.m_nNodes == 3;
}

static bool test_native_xml_character_boundary()
{
	CString sXml = L"<r>" + CString(L'x', XML_PEER_PARSE_CHARS_MAX - 7) + L"</r>";
	CAutoPtr<CXMLElement> pMax(CXMLElement::FromPeerString(sXml));
	CAutoPtr<CXMLElement> pOver(CXMLElement::FromPeerString(sXml + L" "));
	return pMax && !pOver;
}

static bool test_native_xml_shared_budget_and_depth_restore()
{
	XmlParseBudget oBudget(2, 8, 128);
	const CString sAttack = NestedXml(3);
	if (!ChargeSharedPeerXmlChars(oBudget, sAttack.GetLength())) return false;
	CAutoPtr<CXMLElement> pAttack(CXMLElement::FromPeerString(sAttack, FALSE, NULL, &oBudget));
	if (pAttack || oBudget.m_nDepth != 0 || !oBudget.m_bDepthCapped || oBudget.m_nNodes != 2) return false;
	if (!ChargeSharedPeerXmlChars(oBudget, 4)) return false;
	CAutoPtr<CXMLElement> pValid(CXMLElement::FromPeerString(L"<r/>", FALSE, NULL, &oBudget));
	return pValid && oBudget.m_nDepth == 0 && oBudget.m_nNodes == 3 &&
	       oBudget.m_nChars == sAttack.GetLength() + 4;
}

static bool test_native_xml_malformed_and_empty()
{
	const LPCTSTR pCases[] = { NULL, L"", L"<r>", L"<r><a/></x>", L"<?xml version=\"1.0\"", L"<!--unfinished", L"<r a=\"unfinished", L"<r><![CDATA[unfinished" };
	for (LPCTSTR pszXml : pCases)
	{
		CAutoPtr<CXMLElement> pXml(CXMLElement::FromPeerString(pszXml));
		if (pXml) return false;
	}
	return true;
}

static bool test_native_xml_byte_encodings()
{
	BYTE pUtf8[] = { 0xef, 0xbb, 0xbf, '<', 'r', '/', '>' };
	BYTE pLe[] = { 0xff, 0xfe, '<', 0, 'r', 0, '/', 0, '>', 0 };
	BYTE pBe[] = { 0xfe, 0xff, 0, '<', 0, 'r', 0, '/', 0, '>' };
	CAutoPtr<CXMLElement> pA(CXMLElement::FromPeerBytes(pUtf8, sizeof(pUtf8)));
	CAutoPtr<CXMLElement> pB(CXMLElement::FromPeerBytes(pLe, sizeof(pLe)));
	CAutoPtr<CXMLElement> pC(CXMLElement::FromPeerBytes(pBe, sizeof(pBe)));
	return pA && pB && pC && pA->Equals(pB) && pA->Equals(pC);
}

struct TestSocket
{
	SOCKET m_hSocket = INVALID_SOCKET;
	~TestSocket()
	{
		if (m_hSocket != INVALID_SOCKET) closesocket(m_hSocket);
	}
};

static bool test_native_http_close_drains_past_header_cap()
{
	TestSocket oListener, oSender, oReceiver;
	oListener.m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	oSender.m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in oAddress = {};
	oAddress.sin_family = AF_INET;
	oAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(oListener.m_hSocket, reinterpret_cast<sockaddr*>(&oAddress), sizeof(oAddress)) ||
	    listen(oListener.m_hSocket, 1)) return false;
	int nAddress = sizeof(oAddress);
	if (getsockname(oListener.m_hSocket, reinterpret_cast<sockaddr*>(&oAddress), &nAddress) ||
	    connect(oSender.m_hSocket, reinterpret_cast<sockaddr*>(&oAddress), nAddress)) return false;
	oReceiver.m_hSocket = accept(oListener.m_hSocket, NULL, NULL);
	if (oReceiver.m_hSocket == INVALID_SOCKET) return false;
	const int nBuffer = 512 * 1024;
	setsockopt(oReceiver.m_hSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&nBuffer), sizeof(nBuffer));
	const DWORD nTimeout = 5000;
	setsockopt(oSender.m_hSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&nTimeout), sizeof(nTimeout));
	WSAEVENT hClose = WSACreateEvent();
	if (hClose == WSA_INVALID_EVENT) return false;
	bool bResult = false;
	if (WSAEventSelect(oReceiver.m_hSocket, hClose, FD_CLOSE) == 0)
	{
		const CStringA sBody('x', 65536);
		int nSent = 0;
		while (nSent < sBody.GetLength())
		{
			int nWrite = send(oSender.m_hSocket, sBody.GetString() + nSent, sBody.GetLength() - nSent, 0);
			if (nWrite <= 0) break;
			nSent += nWrite;
		}
		shutdown(oSender.m_hSocket, SD_SEND);
		if (nSent == sBody.GetLength() && WSAWaitForMultipleEvents(1, &hClose, TRUE, nTimeout, FALSE) == WSA_WAIT_EVENT_0)
		{
			DWORD nRead = 0;
			unsigned nReads = 0;
			char pBytes[65536];
			bResult = ReadHttpInputUntilBlocked(
			              [&]()
			              {
				              const int nCap = ++nReads == 1 ? 16384 : sizeof(pBytes);
				              const int nReceived = recv(oReceiver.m_hSocket, pBytes, nCap, 0);
				              if (nReceived <= 0) return false;
				              nRead += nReceived;
				              return true;
			              },
			              [&]()
			              {
				              u_long nPending = 0;
				              return ioctlsocket(oReceiver.m_hSocket, FIONREAD, &nPending) == 0 ? nPending : 0ul;
			              }) &&
			          nRead == 65536 && nReads >= 2;
		}
	}
	WSACloseEvent(hClose);
	return bResult;
}

void RegisterThexProductionTests(TestSuite& suite);

int main()
{
	if (!AfxWinInit(GetModuleHandle(NULL), NULL, GetCommandLine(), 0)) return 1;
	WSADATA oData = {};
	if (WSAStartup(MAKEWORD(2, 2), &oData)) return 1;
	TestSuite suite;
	RegisterThexProductionTests(suite);
	suite.add_test("native_xml_valid_matches_local_parser", test_native_xml_valid_matches_local_parser);
	suite.add_test("native_xml_depth_boundary", test_native_xml_depth_boundary);
	suite.add_test("native_xml_nodes_and_attributes", test_native_xml_nodes_and_attributes);
	suite.add_test("native_xml_character_boundary", test_native_xml_character_boundary);
	suite.add_test("native_xml_shared_budget_and_depth_restore", test_native_xml_shared_budget_and_depth_restore);
	suite.add_test("native_xml_malformed_and_empty", test_native_xml_malformed_and_empty);
	suite.add_test("native_xml_byte_encodings", test_native_xml_byte_encodings);
	suite.add_test("native_http_close_drains_past_header_cap", test_native_http_close_drains_past_header_cap);
	const int nResult = suite.run_all_tests() == 0 ? 0 : 1;
	WSACleanup();
	return nResult;
}
