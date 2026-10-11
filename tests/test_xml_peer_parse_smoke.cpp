//
// test_xml_peer_parse_smoke.cpp
//
// Smoke tests for XmlParseValidate.h peer entry-point contracts.
// EnvyTests does not link MFC CXMLElement; these cases exercise the same
// AdmitPeerXmlBytes / AdmitPeerXmlChars / ChargeSharedPeerXmlChars helpers
// used by CXMLElement::FromPeerBytes / FromPeerString and G1/G2 callers.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#include "test_framework.h"
#include "../Envy/XmlParseValidate.h"
#include "../Envy/DownloadTransferHttpValidate.h"

static bool test_xml_peer_defaults()
{
	const XmlParseBudget oBudget = XmlParseBudget::PeerDefaults();
	return oBudget.m_nMaxDepth == XML_PEER_PARSE_DEPTH_MAX &&
	       oBudget.m_nMaxNodes == XML_PEER_PARSE_NODES_MAX &&
	       oBudget.m_nMaxChars == XML_PEER_PARSE_CHARS_MAX &&
	       XML_PEER_THEX_BODY_CAP == XML_PEER_PARSE_CHARS_MAX + XML_PEER_THEX_TREE_SLACK;
}

static bool test_xml_budget_chars_cap()
{
	XmlParseBudget oBudget = XmlParseBudget::PeerDefaults();
	return oBudget.ConsumeChars(XML_PEER_PARSE_CHARS_MAX) &&
	       !oBudget.ConsumeChars(1);
}

static bool test_xml_budget_depth_cap()
{
	XmlParseBudget oBudget(2, 100, 10000);
	const bool bFirst = oBudget.EnterElement();
	const bool bSecond = oBudget.EnterElement();
	const bool bThird = oBudget.EnterElement();
	return bFirst && bSecond && !bThird;
}

static bool test_xml_budget_nodes_cap()
{
	XmlParseBudget oBudget(32, 2, 10000);
	const bool bFirst = oBudget.AddNode();
	const bool bSecond = oBudget.AddNode();
	const bool bThird = oBudget.AddNode();
	return bFirst && bSecond && !bThird;
}

// Models FromPeerString: root EnterElement, then child EnterElement calls.
static bool test_xml_budget_depth_includes_root()
{
	XmlParseBudget oBudget(2, 100, 10000);
	if (!oBudget.AcceptsElementChain(2) || oBudget.AcceptsElementChain(3))
		return false;

	const bool bRoot = oBudget.EnterElement();
	const bool bChild = oBudget.EnterElement();
	const bool bTooDeep = oBudget.EnterElement();
	return bRoot && bChild && !bTooDeep;
}

// Direct FromPeerBytes admission contract (production AdmitPeerXmlBytes).
static bool test_from_peer_bytes_entry_gate()
{
	return !AdmitPeerXmlBytes(0) &&
	       AdmitPeerXmlBytes(1) &&
	       AdmitPeerXmlBytes(XML_PEER_PARSE_CHARS_MAX) &&
	       !AdmitPeerXmlBytes(XML_PEER_PARSE_CHARS_MAX + 1) &&
	       !AdmitPeerXmlBytes(MAXDWORD);
}

// Direct FromPeerString character gate when no shared budget is passed.
static bool test_from_peer_string_entry_gate()
{
	XmlParseBudget oExact = XmlParseBudget::PeerDefaults();
	if (!AdmitPeerXmlChars(oExact, XML_PEER_PARSE_CHARS_MAX) ||
	    AdmitPeerXmlChars(oExact, 1))
		return false;

	XmlParseBudget oEmpty = XmlParseBudget::PeerDefaults();
	if (!AdmitPeerXmlChars(oEmpty, 0)) // empty length is a no-op charge
		return false;

	XmlParseBudget oOver = XmlParseBudget::PeerDefaults();
	if (AdmitPeerXmlChars(oOver, XML_PEER_PARSE_CHARS_MAX + 1))
		return false;

	// Production FromPeerString rejects size_t lengths above the DWORD cap
	// before casting. On Win32, size_t is 32-bit so MAXDWORD+1 wraps; only
	// assert the above-DWORD case where size_t can represent it.
	const size_t nHuge = static_cast<size_t>(XML_PEER_PARSE_CHARS_MAX) + 1u;
	if (nHuge <= XML_PEER_PARSE_CHARS_MAX)
		return false;
	if (sizeof(size_t) > sizeof(DWORD))
	{
		const size_t nPastDword = static_cast<size_t>(MAXDWORD) + 1u;
		if (nPastDword <= XML_PEER_PARSE_CHARS_MAX)
			return false;
	}
	return true;
}

// Same production AdmitPeerXmlBytes gate used before ReadString on
// HostBrowser / COMMENT paths (alias coverage of the shared helper).
static bool test_xml_peer_readstring_prematerialize_gate()
{
	return test_from_peer_bytes_entry_gate();
}

// Shared-budget charge used by G2 METADATA/COMMENT and G1 ReadXML before FromPeer*.
static bool test_charge_shared_peer_xml_chars()
{
	XmlParseBudget oBudget(32, 100, 100);
	if (ChargeSharedPeerXmlChars(oBudget, 0))
		return false;
	if (!ChargeSharedPeerXmlChars(oBudget, 60) || oBudget.m_nChars != 60)
		return false;
	if (!ChargeSharedPeerXmlChars(oBudget, 40) || oBudget.m_nChars != 100)
		return false;
	// Already at cap / overflow / MAXDWORD-near must fail closed before materialize.
	if (ChargeSharedPeerXmlChars(oBudget, 1))
		return false;
	XmlParseBudget oNear = XmlParseBudget(32, 100, 10);
	if (!ChargeSharedPeerXmlChars(oNear, 9))
		return false;
	return !ChargeSharedPeerXmlChars(oNear, 2);
}

// Models multi-fragment G2/G1 metadata: one shared budget across sequential roots.
static bool test_xml_budget_shared_nodes_across_fragments()
{
	XmlParseBudget oBudget(32, 3, 10000);
	if (!ChargeSharedPeerXmlChars(oBudget, 100))
		return false;

	// Fragment A: root + one child = 2 nodes
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	oBudget.LeaveElement();
	oBudget.LeaveElement();

	// Fragment B: root would be node 3; a child must fail the shared node cap.
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	const bool bChildBlocked = !oBudget.AddNode();
	oBudget.LeaveElement();
	return bChildBlocked && oBudget.m_nNodes == 3;
}

// Models FromG2Packet sibling METADATA children sharing one budget.
static bool test_xml_budget_shared_across_sibling_metadata()
{
	XmlParseBudget oBudget(32, 5, 1000);

	// Sibling METADATA A (wrapper node + root + child)
	if (!ChargeSharedPeerXmlChars(oBudget, 50))
		return false;
	if (!oBudget.AddNode()) // Metadata wrapper
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false; // root
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false; // child
	oBudget.LeaveElement();
	oBudget.LeaveElement();

	// Sibling METADATA B — must not reset the node counter
	if (!ChargeSharedPeerXmlChars(oBudget, 50))
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false; // root = node 4
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false; // child = node 5
	oBudget.LeaveElement();
	oBudget.LeaveElement();

	// Sibling METADATA C — further nodes must fail under the shared cap
	if (!ChargeSharedPeerXmlChars(oBudget, 50))
		return false;
	const bool bBlocked = !oBudget.AddNode();
	return bBlocked && oBudget.m_nNodes == 5 && oBudget.m_nChars == 150;
}

// Models FromG1Packet: per-hit ReadXML extensions + trailer ReadXML share one budget.
static bool test_xml_budget_shared_across_g1_hits_and_trailer()
{
	XmlParseBudget oBudget(32, 5, 500);

	// Hit 1 extension
	if (!ChargeSharedPeerXmlChars(oBudget, 40))
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	oBudget.LeaveElement();
	oBudget.LeaveElement();

	// Hit 2 extension — must not reset counters
	if (!ChargeSharedPeerXmlChars(oBudget, 40))
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	oBudget.LeaveElement();
	oBudget.LeaveElement();

	// Trailer metadata — one more root ok (node 5), further nodes blocked
	if (!ChargeSharedPeerXmlChars(oBudget, 40))
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	const bool bChildBlocked = !oBudget.AddNode();
	oBudget.LeaveElement();

	// Additional trailer chars beyond the shared 500 must also fail closed
	const bool bCharsBlocked = !ChargeSharedPeerXmlChars(oBudget, 381); // 120 used + 381 > 500
	return bChildBlocked && bCharsBlocked &&
	       oBudget.m_nNodes == 5 && oBudget.m_nChars == 120;
}

// Partially consumed shared budget + new fragment (HIT_DESCRIPTOR after METADATA).
static bool test_shared_budget_partially_consumed_blocks_next_fragment()
{
	XmlParseBudget oBudget(32, 10, 100);
	if (!ChargeSharedPeerXmlChars(oBudget, 90))
		return false;
	if (!oBudget.EnterElement() || !oBudget.AddNode())
		return false;
	oBudget.LeaveElement();

	// Next fragment of 20 chars must fail before materialization.
	if (ChargeSharedPeerXmlChars(oBudget, 20))
		return false;
	// Exact remaining 10 still admitted.
	return ChargeSharedPeerXmlChars(oBudget, 10) &&
	       !ChargeSharedPeerXmlChars(oBudget, 1) &&
	       oBudget.m_nChars == 100 &&
	       oBudget.m_nNodes == 1;
}

static bool test_xml_budget_default_depth_chain()
{
	const XmlParseBudget oBudget = XmlParseBudget::PeerDefaults();
	return oBudget.AcceptsElementChain(XML_PEER_PARSE_DEPTH_MAX) &&
	       !oBudget.AcceptsElementChain(XML_PEER_PARSE_DEPTH_MAX + 1) &&
	       !oBudget.AcceptsElementChain(0);
}

// Depth limit and limit+1 via EnterElement (FromPeerString root accounting).
static bool test_from_peer_depth_limit_and_overflow()
{
	XmlParseBudget oOk = XmlParseBudget(XML_PEER_PARSE_DEPTH_MAX, 10000, 10000);
	for (DWORD i = 0; i < XML_PEER_PARSE_DEPTH_MAX; ++i)
	{
		if (!oOk.EnterElement() || !oOk.AddNode())
			return false;
	}
	if (oOk.EnterElement() || !oOk.m_bDepthCapped)
		return false;

	XmlParseBudget oTiny(1, 100, 10000);
	return oTiny.EnterElement() && !oTiny.EnterElement() && oTiny.m_bDepthCapped;
}

// Node limit and limit+1.
static bool test_from_peer_nodes_limit_and_overflow()
{
	XmlParseBudget oBudget(32, XML_PEER_PARSE_NODES_MAX, XML_PEER_PARSE_CHARS_MAX);
	for (DWORD i = 0; i < XML_PEER_PARSE_NODES_MAX; ++i)
	{
		if (!oBudget.AddNode())
			return false;
	}
	return !oBudget.AddNode() && oBudget.m_nNodes == XML_PEER_PARSE_NODES_MAX;
}

// Production AdmitThexBodyLength (OnHeadersComplete THEX known-length gate).
static bool test_thex_body_cap_gate()
{
	return !AdmitThexBodyLength(0) &&
	       AdmitThexBodyLength(1) &&
	       AdmitThexBodyLength(XML_PEER_THEX_BODY_CAP) &&
	       !AdmitThexBodyLength((ULONGLONG)XML_PEER_THEX_BODY_CAP + 1) &&
	       !AdmitThexBodyLength(~(ULONGLONG)0);
}

// Ordinary file content rejects CL=0; control / MetaFetch / THEX must not.
static bool test_http_zero_content_length_policy()
{
	if (!RejectExplicitZeroContentLength(false, false, false, false, false))
		return false;
	if (RejectExplicitZeroContentLength(true, false, false, false, false))
		return false; // MetaFetch
	if (RejectExplicitZeroContentLength(false, true, false, false, false))
		return false; // THEX
	if (RejectExplicitZeroContentLength(false, false, true, false, false))
		return false; // 503 / busy
	if (RejectExplicitZeroContentLength(false, false, false, true, false))
		return false; // 416
	if (RejectExplicitZeroContentLength(false, false, false, false, true))
		return false; // redirect
	return TigerKnownLengthBodyFullyConsumed(0) &&
	       !TigerKnownLengthBodyFullyConsumed(1);
}

// Models the G1 AutodetectAudio funding contract (root + two attributes) on
// the shared XmlParseBudget helpers. EnvyTests do not link CG1Packet / MFC,
// so this asserts the budget arithmetic the production loop charges, not a
// call into AutoDetectAudio itself.
static bool test_xml_budget_autodetect_fallback_funds_objects()
{
	XmlParseBudget oBudget(32, 3, 10000);
	if (!oBudget.AddNode() || !oBudget.AddNode() || !oBudget.AddNode())
		return false;
	// Node cap exhausted: Autodetect must not retain an under-funded tree.
	return !oBudget.AddNode() && oBudget.m_nNodes == 3;
}

// XmlParseDepthRestore must reset nested EnterElement after a simulated
// parse failure so a shared packet budget is not left deeper than before.
static bool test_xml_depth_restore_after_nested_enter()
{
	XmlParseBudget oBudget(8, 100, 10000);
	if (!oBudget.EnterElement())
		return false;
	const DWORD nOuter = oBudget.m_nDepth;
	{
		XmlParseDepthRestore oRestore(oBudget);
		if (!oBudget.EnterElement() || !oBudget.EnterElement())
			return false;
		if (oBudget.m_nDepth != nOuter + 2)
			return false;
	}
	if (oBudget.m_nDepth != nOuter)
		return false;

	{
		XmlEnterElement oChild;
		if (!oChild.Enter(&oBudget) || oBudget.m_nDepth != nOuter + 1)
			return false;
	}
	return oBudget.m_nDepth == nOuter;
}

// ConsumeChars must fail closed on DWORD wrap rather than admit a huge add.
static bool test_xml_consume_chars_dword_overflow()
{
	XmlParseBudget oBudget(32, 100, MAXDWORD);
	if (!oBudget.ConsumeChars(MAXDWORD - 1u))
		return false;
	if (oBudget.ConsumeChars(2))
		return false;
	XmlParseBudget oZeroAdd = XmlParseBudget::PeerDefaults();
	return oZeroAdd.ConsumeChars(0) && oZeroAdd.m_nChars == 0 &&
	       !oBudget.ConsumeChars(MAXDWORD);
}


static bool test_http_capped_header_then_body_drain()
{
	DWORD nPending = 65536;
	unsigned nReads = 0;
	const bool bResult = ReadHttpInputUntilBlocked(
	    [&]()
	    {
		    ++nReads;
		    nPending -= nReads == 1 ? 16384 : nPending;
		    return true;
	    },
	    [&]()
	    { return nPending; });
	return bResult && nPending == 0 && nReads == 2;
}

static bool test_http_drain_stops_without_progress()
{
	unsigned nReads = 0;
	return ReadHttpInputUntilBlocked(
	           [&]()
	           { ++nReads; return true; },
	           []()
	           { return 42ul; }) &&
	       nReads == 1;
}

static bool test_http_drain_stops_after_close()
{
	unsigned nReads = 0;
	unsigned nProbes = 0;
	return !ReadHttpInputUntilBlocked(
	           [&]()
	           { ++nReads; return false; },
	           [&]()
	           { ++nProbes; return 42ul; }) &&
	       nReads == 1 && nProbes == 1;
}

static bool test_http_drain_stops_on_growing_queue()
{
	unsigned nReads = 0;
	DWORD nPending = 42;
	return ReadHttpInputUntilBlocked(
	           [&]()
	           { ++nReads; ++nPending; return true; },
	           [&]()
	           { return nPending; }) &&
	       nReads == 1;
}

static bool test_peer_unsigned_decimal_boundaries()
{
	ULONGLONG value = 0;
	if (!ParsePeerUnsignedDecimal(L"0", value) || value != 0 ||
	    !ParsePeerUnsignedDecimal(L"18446744073709551615", value) || value != ~0ull) return false;
	for (const wchar_t* input : { L"", L"-1", L"+1", L"1x", L" 1", L"1 ", L"18446744073709551616" })
		if (ParsePeerUnsignedDecimal(input, value)) return false;
	return true;
}

static bool test_thex_non_power_of_two_rows_and_large_file()
{
	// Three leaves serialize as 1+2+3 nodes, never a padded fourth leaf.
	return ThexTreeMatchesDepth(2050, 144, 3) && ThexTreeMatchesDepth(2050, 144, 2) &&
	       !ThexTreeMatchesDepth(2050, 168, 3) && !ThexTreeMatchesDepth(2050, 120, 3) &&
	       !ThexTreeMatchesDepth(2050, 145, 3) && !ThexTreeMatchesDepth(2050, 144, 1) &&
	       ThexTreeMatchesDepth(~0ull - 1, 24, 0) && !ThexTreeMatchesDepth(~0ull, 24, 0);
}

void register_xml_peer_parse_smoke_tests(TestSuite& suite)
{
	suite.add_test("peer_unsigned_decimal_boundaries", test_peer_unsigned_decimal_boundaries);
	suite.add_test("thex_non_power_of_two_rows_and_large_file", test_thex_non_power_of_two_rows_and_large_file);
	suite.add_test("http_capped_header_then_body_drain", test_http_capped_header_then_body_drain);
	suite.add_test("http_drain_stops_without_progress", test_http_drain_stops_without_progress);
	suite.add_test("http_drain_stops_after_close", test_http_drain_stops_after_close);
	suite.add_test("http_drain_stops_on_growing_queue", test_http_drain_stops_on_growing_queue);
	suite.add_test("xml_peer_defaults", test_xml_peer_defaults);
	suite.add_test("xml_budget_chars_cap", test_xml_budget_chars_cap);
	suite.add_test("xml_budget_depth_cap", test_xml_budget_depth_cap);
	suite.add_test("xml_budget_nodes_cap", test_xml_budget_nodes_cap);
	suite.add_test("xml_budget_depth_includes_root", test_xml_budget_depth_includes_root);
	suite.add_test("xml_budget_default_depth_chain", test_xml_budget_default_depth_chain);
	suite.add_test("from_peer_bytes_entry_gate", test_from_peer_bytes_entry_gate);
	suite.add_test("from_peer_string_entry_gate", test_from_peer_string_entry_gate);
	suite.add_test("xml_peer_readstring_prematerialize_gate", test_xml_peer_readstring_prematerialize_gate);
	suite.add_test("charge_shared_peer_xml_chars", test_charge_shared_peer_xml_chars);
	suite.add_test("xml_budget_shared_nodes_across_fragments", test_xml_budget_shared_nodes_across_fragments);
	suite.add_test("xml_budget_shared_across_sibling_metadata", test_xml_budget_shared_across_sibling_metadata);
	suite.add_test("xml_budget_shared_across_g1_hits_and_trailer", test_xml_budget_shared_across_g1_hits_and_trailer);
	suite.add_test("shared_budget_partially_consumed_blocks_next_fragment", test_shared_budget_partially_consumed_blocks_next_fragment);
	suite.add_test("from_peer_depth_limit_and_overflow", test_from_peer_depth_limit_and_overflow);
	suite.add_test("from_peer_nodes_limit_and_overflow", test_from_peer_nodes_limit_and_overflow);
	suite.add_test("thex_body_cap_gate", test_thex_body_cap_gate);
	suite.add_test("http_zero_content_length_policy", test_http_zero_content_length_policy);
	suite.add_test("xml_budget_autodetect_fallback_funds_objects", test_xml_budget_autodetect_fallback_funds_objects);
	suite.add_test("xml_depth_restore_after_nested_enter", test_xml_depth_restore_after_nested_enter);
	suite.add_test("xml_consume_chars_dword_overflow", test_xml_consume_chars_dword_overflow);
}
