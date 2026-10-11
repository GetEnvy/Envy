// SPDX-License-Identifier: AGPL-3.0-or-later
// Production THEX/DIME methods with download, connection and logging doubles.

#include "../Envy/StdAfx.h"
#include "../Envy/Buffer.h"
#include "../Envy/XML.h"
#include "../Envy/XmlParseValidate.h"
#include "../Envy/DownloadTransferHttpValidate.h"
#include "test_framework.h"
#include <string_view>

constexpr char TREE_TYPE[] = "http://open-content.net/spec/thex/breadthfirst";
// The production installation method and HashLib validate the actual tree.
class CDownloadWithTiger
{
public:
	QWORD m_nSize = SIZE_UNKNOWN;
	Hashes::TigerHash m_oTiger;
	CTigerTree m_pTigerTree;
	CCriticalSection m_pTigerSection;
	DWORD m_nTigerSize = 0, m_nTigerBlock = 0;
	BYTE* m_pTigerBlock = NULL;
	std::vector<BYTE> tree;
	unsigned installs = 0;
	~CDownloadWithTiger() { delete[] m_pTigerBlock; }
	BOOL SetTigerTree(BYTE*, DWORD, BOOL bLevel1 = FALSE);
	void SetModified()
	{
		++installs;
		BYTE* p = NULL;
		DWORD n = 0;
		if (m_pTigerTree.ToBytes(&p, &n)) tree.assign(p, p + n);
		GlobalFree(p);
	}
	void SetHashset(const BYTE*, DWORD) {}
	bool NeedTigerTree() const { return false; }
	bool GetFragment(void*) const { return true; }
	CString GetDisplayName() const { return CString(); }
};
using TestDownload = CDownloadWithTiger;
struct TestSource
{
	BOOL m_bMetaIgnore = FALSE;
	void SetAvailableRanges(void*) {}
};
struct TestSettings
{
	struct
	{
		bool VerifyTiger = false, VerifyED2K = false;
		DWORD ChunkSize = 0;
	} Downloads;
	struct
	{
		unsigned TigerHeight = 9;
	} Library;
	CString SmartVolume(DWORD) const { return CString(); }
} Settings;
struct TestLog
{
	template<typename... Args>
	void Message(Args...) {}
} theApp;
using CLockedBuffer = CBuffer*;

// Socket scheduling and profile ownership remain application/live tests.
class CDownloadTransferHTTP
{
public:
	TestDownload download;
	TestDownload* m_pDownload = &download;
	TestSource* m_pSource = NULL;
	CBuffer input;
	CString m_sContentType = L"application/dime", m_sTigerUUID;
	CString m_sAddress, m_sTigerTree, m_sMetadata;
	QWORD m_nLength = 0, m_nContentLength = 0, m_nOffset = 0, m_nPosition = 0;
	BOOL m_bKeepAlive = TRUE, m_bTigerAbandon = FALSE;
	BOOL m_bTigerDescriptor = FALSE, m_bTigerTreeSeen = FALSE;
	BOOL m_bTigerDimeStarted = FALSE, m_bTigerDimeEnded = FALSE;
	DWORD m_nTigerDepth = 0;
	BOOL m_bWantBackwards = FALSE, m_bRecvBackwards = FALSE;
	BOOL m_bTigerFetch = FALSE, m_bMetaFetch = FALSE, m_bTigerIgnore = FALSE;
	DWORD m_tContent = 0;
	struct
	{
		DWORD tLast = 0;
	} m_mInput;
	unsigned m_nRequests = 0, closes = 0, requests = 0;
	CBuffer* GetInput() { return &input; }
	DWORD GetInputLength() const { return input.m_nLength; }
	bool IsInputExist() const { return true; }
	bool IsOutputExist() const { return true; }
	void Close(int) { ++closes; }
	BOOL SendRequest()
	{
		++requests;
		return TRUE;
	}
	void ChunkifyRequest(QWORD*, QWORD*, DWORD, BOOL) {}
	BOOL StartNextFragment();
	BOOL ReadTiger(bool bDropped = false);
	void Begin(QWORD nLength)
	{
		m_nLength = m_nContentLength = nLength;
		download.m_nSize = 1024;
	}
};

// Only locking is replaced: all test calls run on one thread.
#undef ASSUME_LOCK
#define ASSUME_LOCK(section) ((void)0)
#define BLOCK_SIZE 1024
#define BLOCK_MASK 0xFFFFFC00
#include "thex_test_methods.inc"
#undef BLOCK_MASK
#undef BLOCK_SIZE

static void AddDescriptor(CBuffer& o, const char* id = "uuid:first", bool valid = true)
{
	const std::string xml = std::string("<hashtree><file size=\"") + (valid ? "1024" : "999") +
	                        "\" segmentsize=\"1024\"/><digest algorithm=\"http://open-content.net/spec/digest/tiger\" outputsize=\"24\"/>"
	                        "<serializedtree type=\"" +
	                        TREE_TYPE + "\" depth=\"1\" uri=\"" + id + "\"/></hashtree>";
	o.WriteDIME(1, "", 0, "text/xml", 8, xml.data(), xml.size());
}

static void AddTree(CBuffer& o, std::string_view id = "uuid:first")
{
	// Published THEX 1024-byte 'A' vector; not a parser-only placeholder.
	Hashes::TigerHash root;
	root.fromString(L"L66Q4YVNAFWVS23X2HJIRA5ZJ7WXR3F26RSASFA");
	const BYTE* tree = &root[0];
	o.WriteDIME(2, id.data(), id.size(), TREE_TYPE, sizeof(TREE_TYPE) - 1, tree, 24);
}

static bool test_thex_production_all_two_read_splits()
{
	CBuffer wire;
	AddDescriptor(wire);
	const DWORD descriptor = wire.m_nLength;
	AddTree(wire);
	CDownloadTransferHTTP whole;
	whole.Begin(wire.m_nLength);
	whole.input.Add(wire.m_pBuffer, wire.m_nLength);
	if (!whole.ReadTiger() || whole.closes || whole.download.installs != 1 || whole.m_nLength != 0) return false;
	// Includes split XML, the exact descriptor boundary, partial tree header,
	// ID/type, binary payload and padding. Observe intermediate state too.
	for (DWORD cut = 1; cut < wire.m_nLength; ++cut)
	{
		CDownloadTransferHTTP split;
		split.Begin(wire.m_nLength);
		split.input.Add(wire.m_pBuffer, cut);
		if (!split.ReadTiger() || split.closes || split.download.installs) return false;
		if (cut >= descriptor && split.m_sTigerUUID != L"uuid:first") return false;
		split.input.Add(wire.m_pBuffer + cut, wire.m_nLength - cut);
		if (!split.ReadTiger() || split.closes || split.m_nLength != 0 || split.input.m_nLength ||
		    split.download.installs != 1 || split.download.tree != whole.download.tree) return false;
	}
	return true;
}

static bool test_thex_production_identity_reset_and_mismatch()
{
	CBuffer descriptor, wrong;
	AddDescriptor(descriptor);
	AddTree(wrong, "uuid:other");
	CDownloadTransferHTTP transfer;
	transfer.Begin(descriptor.m_nLength + wrong.m_nLength);
	transfer.input.Add(descriptor.m_pBuffer, descriptor.m_nLength);
	if (!transfer.ReadTiger() || transfer.m_sTigerUUID != L"uuid:first") return false;
	transfer.input.Add(wrong.m_pBuffer, wrong.m_nLength);
	if (!transfer.ReadTiger() || transfer.download.installs || !transfer.m_sTigerUUID.IsEmpty()) return false;
	// StartNextFragment above executed the real reset. A second response must
	// accept its own nonempty identity across the same split boundary.
	CBuffer second;
	AddDescriptor(second, "uuid:second");
	const DWORD cut = second.m_nLength;
	AddTree(second, "uuid:second");
	transfer.Begin(second.m_nLength);
	transfer.input.Add(second.m_pBuffer, cut);
	if (!transfer.ReadTiger() || transfer.m_sTigerUUID != L"uuid:second") return false;
	transfer.input.Add(second.m_pBuffer + cut, second.m_nLength - cut);
	return transfer.ReadTiger() && !transfer.closes && transfer.download.installs == 1 && transfer.m_nLength == 0;
}

static bool test_thex_production_truncation_and_malformed()
{
	CBuffer wire;
	AddDescriptor(wire);
	AddTree(wire);
	for (DWORD cut = 0; cut < wire.m_nLength; ++cut)
	{
		CDownloadTransferHTTP transfer;
		transfer.Begin(wire.m_nLength);
		transfer.input.Add(wire.m_pBuffer, cut);
		if (!transfer.ReadTiger() || transfer.ReadTiger(true) || transfer.closes || transfer.download.installs) return false;
	}
	const BYTE malformed[12] = {};
	for (bool dropped : { false, true })
	{
		CDownloadTransferHTTP transfer;
		transfer.Begin(sizeof(malformed));
		transfer.input.Add(malformed, sizeof(malformed));
		if (transfer.ReadTiger(dropped) || transfer.closes != (dropped ? 0u : 1u)) return false;
	}
	return true;
}

static bool test_thex_production_rejects_oversize_header_before_body()
{
	const DWORD lengths[] = { XML_PEER_PARSE_CHARS_MAX + 1u, MAXDWORD };
	for (DWORD length : lengths)
	{
		CBuffer wire;
		wire.WriteDIME(1, "", 0, "text/xml", 8, NULL, length);
		CDownloadTransferHTTP transfer;
		transfer.Begin(SIZE_UNKNOWN);
		transfer.input.Add(wire.m_pBuffer, wire.m_nLength);
		if (transfer.ReadTiger() || transfer.closes != 1 || transfer.download.installs) return false;
	}
	// Binary record padding overflow must also fail before waiting for body.
	CBuffer wire;
	wire.WriteDIME(2, "", 0, TREE_TYPE, sizeof(TREE_TYPE) - 1, NULL, MAXDWORD - 1u);
	CDownloadTransferHTTP transfer;
	transfer.Begin(SIZE_UNKNOWN);
	transfer.input.Add(wire.m_pBuffer, wire.m_nLength);
	return !transfer.ReadTiger() && transfer.closes == 1 && !transfer.download.installs;
}

static bool test_thex_production_invalid_descriptor_drains_remaining_response()
{
	CBuffer wire;
	AddDescriptor(wire, "uuid:first", false);
	const DWORD cut = wire.m_nLength;
	AddTree(wire);
	CDownloadTransferHTTP transfer;
	transfer.Begin(wire.m_nLength);
	transfer.input.Add(wire.m_pBuffer, cut);
	if (!transfer.ReadTiger() || !transfer.m_bTigerAbandon || transfer.input.m_nLength) return false;
	transfer.input.Add(wire.m_pBuffer + cut, wire.m_nLength - cut);
	return transfer.ReadTiger() && !transfer.closes && !transfer.download.installs &&
	       !transfer.m_bTigerAbandon && !transfer.input.m_nLength;
}

static const wchar_t* TEST_ROOT = L"L66Q4YVNAFWVS23X2HJIRA5ZJ7WXR3F26RSASFA";

static std::string Descriptor(const char* uri = " uri=\"uuid:first\"", const char* file = "size=\"1024\" segmentsize=\"1024\"", const char* depth = "1")
{
	return std::string("<hashtree><file ") + file + "/><digest algorithm=\"http://open-content.net/spec/digest/tiger\" outputsize=\"24\"/><serializedtree type=\"" + TREE_TYPE + "\" depth=\"" + depth + "\"" + uri + "/></hashtree>";
}
static void AddXml(CBuffer& wire, const std::string& xml, DWORD flags = 1)
{
	wire.WriteDIME(flags, "", 0, "text/xml", 8, xml.data(), xml.size());
}
static bool RunTree(CBuffer& wire, bool known, unsigned installs, QWORD size = 1024)
{
	CDownloadTransferHTTP transfer;
	transfer.Begin(wire.m_nLength);
	transfer.download.m_nSize = size;
	if (known) transfer.download.m_oTiger.fromString(TEST_ROOT);
	transfer.input.Add(wire.m_pBuffer, wire.m_nLength);
	transfer.ReadTiger();
	return transfer.download.installs == installs && (!installs || transfer.download.m_pTigerTree.IsAvailable());
}
static bool test_thex_production_descriptor_identity_matrix()
{
	for (bool known : { false, true })
	{
		for (const char* uri : { " uri=\"uuid:first\"", " uri=\"\"", "" })
		{
			for (std::string_view id : { "uuid:first", "", "uuid:other" })
			{
				CBuffer wire;
				AddXml(wire, Descriptor(uri));
				AddTree(wire, id);
				const bool fullUri = std::string_view(uri) == " uri=\"uuid:first\"";
				const bool accepted = fullUri ? (id.empty() || id == "uuid:first") : known;
				if (!RunTree(wire, known, accepted ? 1 : 0)) return false;
			}
		}
		CBuffer noXml;
		AddTree(noXml);
		noXml.m_pBuffer[0] |= 4; // Valid single-record MB/ME framing, but no XML.
		if (!RunTree(noXml, known, 0)) return false;
	}
	return true;
}
static bool test_thex_production_descriptor_fields()
{
	for (const char* file : { "size=\"1024x\" segmentsize=\"1024\"", "size=\"18446744073709551616\" segmentsize=\"1024\"", "size=\"-1024\" segmentsize=\"1024\"", "size=\"1024\" segmentsize=\"512\"", "size=\"1024\"" })
	{
		CBuffer wire;
		AddXml(wire, Descriptor(" uri=\"uuid:first\"", file));
		AddTree(wire);
		if (!RunTree(wire, true, 0)) return false;
	}
	for (const char* depth : { "", "-1", "1x", "18446744073709551616", "64", "2" })
	{
		CBuffer wire;
		AddXml(wire, Descriptor(" uri=\"uuid:first\"", "size=\"1024\" segmentsize=\"1024\"", depth));
		AddTree(wire);
		if (!RunTree(wire, true, 0)) return false;
	}
	for (const char* depth : { "0", "1" })
	{
		CBuffer wire;
		AddXml(wire, Descriptor(" uri=\"uuid:first\"", "size=\"1024\" segmentsize=\"1024\"", depth));
		AddTree(wire);
		if (!RunTree(wire, false, 1)) return false;
	}
	for (const auto& pair : { std::pair<std::string, std::string>{ "tiger\"", "sha1\"" }, { "outputsize=\"24", "outputsize=\"20" }, { "breadthfirst", "unknown" } })
	{
		std::string xml = Descriptor();
		xml.replace(xml.find(pair.first), pair.first.size(), pair.second);
		CBuffer wire;
		AddXml(wire, xml);
		AddTree(wire);
		if (!RunTree(wire, true, 0)) return false;
	}
	CBuffer invalid;
	AddXml(invalid, "<hashtree>");
	AddTree(invalid);
	return RunTree(invalid, true, 0);
}
static bool test_thex_production_real_hash_and_tree_integrity()
{
	const std::vector<BYTE> data(1025, 'A');
	CTigerTree generated;
	generated.BeginFile(9, data.size());
	generated.AddToFile(data.data(), data.size());
	if (!generated.FinishFile()) return false;
	BYTE* raw = NULL;
	DWORD n = 0;
	if (!generated.ToBytes(&raw, &n)) return false;
	std::vector<BYTE> tree(raw, raw + n);
	GlobalFree(raw);
	if (n != 72) return false;
	Hashes::TigerHash expected;
	expected.fromString(L"PZMRYHGY6LTBEH63ZWAHDORHSYTLO4LEFUIKHWY");
	if (memcmp(tree.data(), &expected[0], 24)) return false;
	for (bool corrupt : { false, true })
	{
		for (const char* depth : { "1", "2" })
		{
			CBuffer wire;
			AddXml(wire, Descriptor(" uri=\"uuid:first\"", "size=\"1025\" segmentsize=\"1024\"", depth));
			auto bytes = tree;
			if (corrupt) bytes.back() ^= 1;
			wire.WriteDIME(2, "uuid:first", 10, TREE_TYPE, sizeof(TREE_TYPE) - 1, bytes.data(), bytes.size());
			CDownloadTransferHTTP transfer;
			transfer.Begin(wire.m_nLength);
			transfer.download.m_nSize = 1025;
			transfer.download.m_oTiger = expected;
			transfer.input.Add(wire.m_pBuffer, wire.m_nLength);
			transfer.ReadTiger();
			if (transfer.download.installs != (corrupt ? 0u : 1u)) return false;
		}
	}
	for (DWORD bytes : { 0u, 23u, 25u, 48u, 96u })
	{
		CBuffer wire;
		AddDescriptor(wire);
		std::vector<BYTE> invalid(bytes, 1);
		wire.WriteDIME(2, "uuid:first", 10, TREE_TYPE, sizeof(TREE_TYPE) - 1, invalid.data(), bytes);
		if (!RunTree(wire, false, 0)) return false;
	}
	CBuffer mismatch;
	AddDescriptor(mismatch);
	BYTE wrong[24] = { 1 };
	mismatch.WriteDIME(2, "uuid:first", 10, TREE_TYPE, sizeof(TREE_TYPE) - 1, wrong, 24);
	if (!RunTree(mismatch, true, 0)) return false;
	// Unknown roots may still be adopted; this proves consistency, not peer authenticity.
	CBuffer unknown;
	AddDescriptor(unknown);
	AddTree(unknown);
	return RunTree(unknown, false, 1);
}
static bool test_thex_production_bare_tree_and_incremental_reuse()
{
	CBuffer tree;
	AddTree(tree);
	// Strip the DIME header to reuse the published binary root.
	CString id, type;
	DWORD flags, length;
	if (!tree.ReadDIME(&flags, &id, &type, &length)) return false;
	for (bool corrupt : { false, true })
	{
		CDownloadTransferHTTP transfer;
		transfer.Begin(length);
		transfer.m_sContentType = L"application/tigertree-breadthfirst";
		transfer.download.m_oTiger.fromString(TEST_ROOT);
		transfer.input.Add(tree.m_pBuffer, length);
		if (corrupt) transfer.input.m_pBuffer[0] ^= 1;
		if (!!transfer.ReadTiger() == corrupt || transfer.download.installs != (corrupt ? 0u : 1u)) return false;
	}
	CDownloadTransferHTTP transfer;
	for (unsigned response = 0; response < 2; ++response)
	{
		CBuffer wire;
		AddDescriptor(wire);
		AddTree(wire);
		transfer.Begin(wire.m_nLength);
		for (DWORD i = 0; i < wire.m_nLength; ++i)
		{
			transfer.input.Add(wire.m_pBuffer + i, 1);
			if (!transfer.ReadTiger() || transfer.closes) return false;
		}
		if (transfer.download.installs != 1 || !transfer.m_sTigerUUID.IsEmpty() || transfer.m_bTigerDescriptor || transfer.m_bTigerDimeStarted) return false;
	}
	CBuffer suffix;
	AddDescriptor(suffix);
	const DWORD boundary = suffix.m_nLength;
	AddTree(suffix);
	suffix.m_pBuffer[boundary] &= ~2;
	const BYTE md4[16] = {};
	constexpr char md4Type[] = "http://edonkey2000.com/spec/md4-hashset";
	suffix.WriteDIME(2, "", 0, md4Type, sizeof(md4Type) - 1, md4, sizeof(md4));
	if (!RunTree(suffix, true, 1)) return false;
	CBuffer selected;
	AddDescriptor(selected);
	const DWORD unrelated = selected.m_nLength;
	AddTree(selected, "uuid:other");
	selected.m_pBuffer[unrelated] &= ~2;
	AddTree(selected);
	if (!RunTree(selected, false, 1)) return false;
	CBuffer duplicate;
	AddDescriptor(duplicate);
	const DWORD start = duplicate.m_nLength;
	AddTree(duplicate);
	duplicate.m_pBuffer[start] &= ~2;
	AddTree(duplicate);
	return RunTree(duplicate, true, 1); // First valid tree remains; duplicate never replaces it.
}
static bool test_thex_production_record_order_and_framing()
{
	CBuffer before;
	AddTree(before);
	AddDescriptor(before);
	if (!RunTree(before, false, 0)) return false;
	CBuffer duplicates;
	AddDescriptor(duplicates);
	AddXml(duplicates, Descriptor(), 0);
	AddTree(duplicates);
	if (!RunTree(duplicates, false, 0)) return false;
	CBuffer two;
	AddDescriptor(two);
	AddTree(two);
	AddTree(two);
	// ME before the end of the HTTP body must reject before installing the tree.
	if (!RunTree(two, false, 0)) return false;
	for (BYTE flags : { BYTE(0), BYTE(1), BYTE(3), BYTE(5) })
	{
		CBuffer bad;
		AddDescriptor(bad);
		AddTree(bad);
		bad.m_pBuffer[0] = 0x08 | flags;
		if (!RunTree(bad, true, 0)) return false;
	}
	CBuffer noEnd;
	AddDescriptor(noEnd);
	AddTree(noEnd);
	// Locate the last record's fixed header, clear ME.
	CBuffer descriptor;
	AddDescriptor(descriptor);
	noEnd.m_pBuffer[descriptor.m_nLength] &= ~2;
	return RunTree(noEnd, false, 0);
}

void RegisterThexProductionTests(TestSuite& suite)
{
	suite.add_test("thex_production_bare_tree_and_incremental_reuse", test_thex_production_bare_tree_and_incremental_reuse);
	suite.add_test("thex_production_descriptor_identity_matrix", test_thex_production_descriptor_identity_matrix);
	suite.add_test("thex_production_descriptor_fields", test_thex_production_descriptor_fields);
	suite.add_test("thex_production_real_hash_and_tree_integrity", test_thex_production_real_hash_and_tree_integrity);
	suite.add_test("thex_production_record_order_and_framing", test_thex_production_record_order_and_framing);
	suite.add_test("thex_production_all_two_read_splits", test_thex_production_all_two_read_splits);
	suite.add_test("thex_production_identity_reset_and_mismatch", test_thex_production_identity_reset_and_mismatch);
	suite.add_test("thex_production_truncation_and_malformed", test_thex_production_truncation_and_malformed);
	suite.add_test("thex_production_rejects_oversize_header_before_body", test_thex_production_rejects_oversize_header_before_body);
	suite.add_test("thex_production_invalid_descriptor_drains_remaining_response", test_thex_production_invalid_descriptor_drains_remaining_response);
}
