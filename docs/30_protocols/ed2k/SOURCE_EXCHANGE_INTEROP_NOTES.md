# ED2K/eMule Source Exchange Interoperability Notes

## Scope
This note documents current Envy behavior for Source Exchange v1/v2 packets and pragmatic compatibility expectations with eMule/aMule style peers.

## Packet Compatibility
- `OP_REQUESTSOURCES` (0x81) and `OP_ANSWERSOURCES` (0x82) remain supported and unchanged on-wire.
- `OP_REQUESTSOURCES2` (0x83) and `OP_ANSWERSOURCES2` (0x84) remain supported and preferred when peer capability negotiation reports SourceEx2 support (`nOpt2` bit 10).
- Source list entries remain serialized in legacy IPv4 shape: `<ClientID 4><Port 2><ServerIP 4><ServerPort 2>[GUID 16]`.

## Defensive Parsing
- Source answer handlers now validate list body length against count and per-entry size before reading any source tuples.
- Malformed or truncated source packets are rejected through existing bad-packet handling/logging path.

## Current Limitation (Explicit)
- SourceEx and SourceEx2 source tuples are currently IPv4-only on-wire in Envy.
- No IPv6 source tuple encoding/decoding is currently implemented for Source Exchange packets.
- This is intentional for compatibility with the existing legacy ED2K/eMule tuple format and to avoid wire-format regressions in this incremental hardening pass.
