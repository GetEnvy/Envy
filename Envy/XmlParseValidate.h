//
// XmlParseValidate.h
//
// Depth / size budgets for untrusted peer-sourced XML.
// Part of Envy (getenvy.com) © 2016-2026
//

#pragma once

#include <windows.h>

// Align with G1 deflate XML inflate cap (PacketLengthValidate.h).
constexpr DWORD XML_PEER_PARSE_CHARS_MAX = 256u * 1024u;
constexpr DWORD XML_PEER_PARSE_DEPTH_MAX = 32u;
constexpr DWORD XML_PEER_PARSE_NODES_MAX = 4096u;
// THEX/DIME may carry a tiger tree after the XML descriptor; bound the full
// receive buffer (XML budget + tree slack) before allocation/buffering.
constexpr DWORD XML_PEER_THEX_TREE_SLACK = 16u * 1024u * 1024u;
constexpr DWORD XML_PEER_THEX_BODY_CAP = XML_PEER_PARSE_CHARS_MAX + XML_PEER_THEX_TREE_SLACK;

// Production admission gate for CXMLElement::FromPeerBytes (and EnvyTests).
inline bool AdmitPeerXmlBytes(DWORD nByte) noexcept
{
	return nByte > 0 && nByte <= XML_PEER_PARSE_CHARS_MAX;
}

// Known-length THEX Content-Length gate used by OnHeadersComplete / EnvyTests.
inline bool AdmitThexBodyLength(ULONGLONG nLength) noexcept
{
	return nLength > 0 && nLength <= XML_PEER_THEX_BODY_CAP;
}

struct XmlParseBudget
{
	DWORD m_nMaxDepth;
	DWORD m_nMaxNodes;
	DWORD m_nMaxChars;
	DWORD m_nDepth;
	DWORD m_nNodes;
	DWORD m_nChars;
	// Sticky: set when EnterElement refuses at m_nMaxDepth. Depth is restored
	// after each parse, so callers cannot detect a depth-cap miss from m_nDepth
	// afterwards; like the persistent node count, this lets them treat the
	// failure as a budget miss instead of malformed XML.
	bool m_bDepthCapped;

	XmlParseBudget(DWORD nMaxDepth, DWORD nMaxNodes, DWORD nMaxChars)
	    : m_nMaxDepth(nMaxDepth)
	    , m_nMaxNodes(nMaxNodes)
	    , m_nMaxChars(nMaxChars)
	    , m_nDepth(0)
	    , m_nNodes(0)
	    , m_nChars(0)
	    , m_bDepthCapped(false)
	{
	}

	static XmlParseBudget PeerDefaults()
	{
		return XmlParseBudget(XML_PEER_PARSE_DEPTH_MAX, XML_PEER_PARSE_NODES_MAX, XML_PEER_PARSE_CHARS_MAX);
	}

	bool ConsumeChars(DWORD nChars)
	{
		if (nChars == 0)
			return true;
		const DWORD nNext = m_nChars + nChars;
		if (nNext < m_nChars || nNext > m_nMaxChars)
			return false;
		m_nChars = nNext;
		return true;
	}

	bool EnterElement()
	{
		const DWORD nNext = m_nDepth + 1;
		if (nNext > m_nMaxDepth)
		{
			m_bDepthCapped = true;
			return false;
		}
		m_nDepth = nNext;
		return true;
	}

	void LeaveElement()
	{
		if (m_nDepth > 0)
			--m_nDepth;
	}

	bool AddNode()
	{
		const DWORD nNext = m_nNodes + 1;
		if (nNext > m_nMaxNodes)
			return false;
		m_nNodes = nNext;
		return true;
	}

	// Models FromPeerString depth accounting: root EnterElement, then each child.
	// Returns false when a chain of nElementCount elements would exceed m_nMaxDepth.
	bool AcceptsElementChain(DWORD nElementCount) const
	{
		return nElementCount > 0 && nElementCount <= m_nMaxDepth;
	}
};

// Restore m_nDepth to the value captured at construction. FromPeerString uses
// this so a throw from ParseString cannot leave a shared budget deeper than
// it was before the fragment, even when nested LeaveElement calls are skipped.
struct XmlParseDepthRestore
{
	XmlParseBudget& m_oBudget;
	const DWORD m_nSavedDepth;

	explicit XmlParseDepthRestore(XmlParseBudget& oBudget) noexcept
	    : m_oBudget(oBudget)
	    , m_nSavedDepth(oBudget.m_nDepth)
	{
	}

	XmlParseDepthRestore(const XmlParseDepthRestore&) = delete;
	XmlParseDepthRestore& operator=(const XmlParseDepthRestore&) = delete;

	~XmlParseDepthRestore() noexcept
	{
		m_oBudget.m_nDepth = m_nSavedDepth;
	}
};

// One EnterElement with matching LeaveElement on all exit paths, including
// CException unwind between EnterElement and the corresponding LeaveElement.
struct XmlEnterElement
{
	XmlParseBudget* m_pBudget;
	bool m_bEntered;

	XmlEnterElement() noexcept
	    : m_pBudget(NULL)
	    , m_bEntered(false)
	{
	}

	XmlEnterElement(const XmlEnterElement&) = delete;
	XmlEnterElement& operator=(const XmlEnterElement&) = delete;

	bool Enter(XmlParseBudget* pBudget)
	{
		m_pBudget = pBudget;
		if (!pBudget)
			return true;
		if (!pBudget->EnterElement())
			return false;
		m_bEntered = true;
		return true;
	}

	~XmlEnterElement() noexcept
	{
		if (m_bEntered && m_pBudget)
			m_pBudget->LeaveElement();
	}
};

// Character gate used by FromPeerString when no shared budget is supplied.
inline bool AdmitPeerXmlChars(XmlParseBudget& budget, DWORD nChars) noexcept
{
	return budget.ConsumeChars(nChars);
}

// Shared-budget callers charge before ReadString / FromPeer* so sibling
// METADATA/COMMENT/fragments cannot materialize past the aggregate cap.
inline bool ChargeSharedPeerXmlChars(XmlParseBudget& budget, DWORD nChars) noexcept
{
	if (nChars == 0)
		return false;
	if (budget.m_nChars >= budget.m_nMaxChars)
		return false;
	if (nChars > budget.m_nMaxChars - budget.m_nChars)
		return false;
	return budget.ConsumeChars(nChars);
}
