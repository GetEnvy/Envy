# Envy Development Roadmap

**Last Updated:** March 2026
**Based on:** Full codebase comparison against reference clients (eMule, aMule, libtorrent, qBittorrent, Transmission)
**Approach:** Phased, evidence-based priorities; see `docs/10_dev/status.md` for the detailed comparison


Canonical context: operational dashboard in `docs/DEV_TRACKER.md`; strategic plan in `docs/DEVELOPMENT_PLAN.md`.

---

## Current State Summary

- **Build System:** Visual Studio solution builds (MSVC toolset `v145`), CMake partial (HashLib only)
- **UI Framework:** MFC/Unicode complete
- **G2 / G1 / DC++:** Fully implemented
- **BitTorrent v1:** Solid (DHT, ut_metadata, ut_pex, lt_tex, web seeds, trackers)
- **BitTorrent v2:** Library-only (Merkle tree + SHA-256); no wire protocol
- **ED2K:** Core transfers + CryptLayer + SourceEx2 (0x83/0x84) working; compressed upload, AICH C2C, and several opcodes missing
- **Kademlia:** Bootstrap, ping, find_node, FIND_VALUE (search key/source), PUBLISH (key/source), and DHT store implemented; bucket splitting, LRU, refresh, firewalled handling TODO
- **IPv6:** Utilities exist, core connections IPv4-only
- **Testing:** 13 HashLib unit tests passing; protocol integration tests require core refactoring

---

## Phase 0: Build & Infrastructure (COMPLETE)

- Visual Studio 2026 (v145) migration
- GitHub Actions CI/CD pipeline (Debug + Release, Win32 + x64)
- Google Test / custom test framework integration
- Code formatting (.clang-format), static analysis
- CMake for HashLib + tests
- OpenCppCoverage in CI

---

## Phase 1: Performance & Stability (COMPLETE)

- UI thread blocking fixes (file I/O moved to background)
- UI update batching (250ms timer, 50ms data caching)
- List processing O(n^2) elimination
- Memory management improvements (buffer reuse, smart pointers)
- CryptLayer implementation fixes (decryption ordering, state machine, RSA encryption)

---

## Phase 2: ED2K Protocol Fixes (PARTIALLY COMPLETE)

**Goal:** Fix incorrect advertisements and finish partially implemented features.

### Done
- CryptLayer RSA+RC4 handshake
- SecureID challenge/response flow (crypto-random)
- Large file support (64-bit request/send/compressed)
- HASHSETREQUEST2 / HASHSETANSWER2
- FileIdentifier support
- AICH hash tree building and verification
- Concatenated UDP packet parsing fix

### Done (bugs fixed / features completed)
- **SecureID version advertisement** — `EDPacket.h` now has `ED2K_VERSION_SECUREID = 3` (eMule-compatible).
- **SourceEx2** — REQUESTSOURCES2 (0x83) / ANSWERSOURCES2 (0x84) implemented in `EDClient.cpp`; advertised via nOpt2 bit 10; `DownloadTransferED2K` sends REQUESTSOURCES2 when peer supports it.

### TODO: Bugs / verification

| Item | Detail | Priority |
|------|--------|----------|
| **AICH hash algorithm** | Envy uses SHA-1 for AICH blocks; eMule uses MD4 — verify actual wire compatibility | Medium |

### TODO: Missing features

| Item | Detail | Priority |
|------|--------|----------|
| **Compressed upload** | Can receive COMPRESSEDPART but never sends it; implement zlib deflate in `UploadTransferED2K::DispatchNextChunk()` | Medium |
| **REQUESTSOURCES2 / ANSWERSOURCES2** | Source exchange v2 (0x83/0x84) — implement or stop advertising | Medium |
| **AICH C2C protocol** | AICHFILEHASHREQ (0x9E) / AICHFILEHASHANS (0x9D) handlers — required for AICH corruption recovery from peers | Medium |
| **MULTIPACKET_EXT2 full batching** | Current handler treats each entry as separate FILEREQUEST; implement proper batched processing | Low |
| **PUBLICIP_REQ / PUBLICIP_ANSWER** (0x97/0x98) | Public IP discovery from peers | Low |
| **CALLBACK / REASKCALLBACKTCP** (0x99/0x9A) | Callback mechanism for firewalled clients | Low |
| **BUDDYPING / BUDDYPONG** (0x9F/0xA0) | Buddy system for low-ID clients | Low |
| **FWCHECKUDPREQ** (0xA7) | Firewall check for Kad >=6/7 integration | Low |

---

## Phase 3: Kademlia DHT Completion (PARTIALLY COMPLETE)

**Goal:** Move from "wire-compatible for bootstrap" to "fully functional DHT participant."

### Done
- BOOTSTRAP_REQ/RES, PING/PONG, FIND_NODE, HELLO
- Routing table (XOR distance, K=10)
- nodes.dat import (v0-3)
- Rate limiting, blacklist integration
- Request tracking, IP endianness

### Done (Kad2 search/publish)
- **FIND_VALUE** — SEARCH_KEY_REQ, SEARCH_SOURCE_REQ, SEARCH_RES implemented in `Kademlia.cpp`.
- **PUBLISH (KEY/SOURCE)** — PUBLISH_KEY_REQ, PUBLISH_SOURCE_REQ, PUBLISH_RES implemented; DHT store (`KadStore`) for keyword/source entries.

### TODO

| Item | Detail | Priority |
|------|--------|----------|
| **Bucket splitting** | Split buckets when full (if bucket contains own ID); current implementation uses simple add | High |
| **LRU replacement** | Replace least-recently-used contact when bucket is full; currently rejects new contacts | Medium |
| **Bucket refresh** | Periodic refresh of stale buckets (eMule uses 15-minute intervals) | Medium |
| **Eclipse protection** | Limit contacts from same /24 subnet to prevent eclipse attacks | Medium |
| **FIREWALLED_REQ/RES** | Handle firewalled node detection and relay | Medium |
| **FINDBUDDY_REQ/RES** | Buddy system for NAT traversal | Low |
| **CALLBACK_REQ/RES** | Kad callback mechanism | Low |
| **UDP hole punching** | NAT traversal for firewalled nodes | Low |
| **Firewall self-check** | Detect own firewall status via Kademlia | Low |

---

## Phase 4: BitTorrent Upgrades (TODO)

**Goal:** Close the gap with modern BT clients (libtorrent, qBittorrent, Transmission).

### 4.1 Transport & Security (no v2 dependency)

| Item | Detail | Priority |
|------|--------|----------|
| ~~Protocol encryption (MSE/PE)~~ | Done: `BTCrypto.h/cpp` with DH+RC4, integrated into `CBTClient`; `Settings.BitTorrent.Encryption` | Done |
| **uTP (BEP-29)** | UDP-based congestion-controlled transport; required by many modern peers | High |
| **HTTPS tracker support** | Accept `https://` tracker URLs | Medium |
| **HTTP scrape** | Uncomment and fix `ScrapeTracker()` in `BTInfo.cpp` | Medium |
| **Fast peers (BEP-6)** | Have/Allowed-Fast/Suggest-Piece/Reject — flag defined but never set | Low |
| **Local peer discovery (BEP-14)** | mDNS/LPD for LAN peers | Low |

### 4.2 BitTorrent v2 / BEP-52

| Item | Detail | Priority |
|------|--------|----------|
| **32-byte SHA-256 infohash** | Uncomment `m_oBTHv2`, implement `IsBitTorrentV2()` properly | High |
| **v2 .torrent parsing** | Parse `file tree`, per-file Merkle roots, `meta version` | High |
| **Hash request/response** (BEP-52 msg 21-23) | Wire protocol for Merkle piece verification | High |
| **Hybrid mode** (v1+v2 in one torrent) | Dual infohash, shared piece data | Medium |
| **v2 magnet download path** | Connect `btmh:` parsing to actual v2 metadata fetch | Medium |

### 4.3 Peer Exchange & Connectivity

| Item | Detail | Priority |
|------|--------|----------|
| **IPv6 peers in PEX** | Currently 6-byte compact (IPv4 only) | Medium |
| **NAT traversal (BEP-10)** | Flag defined but not set; implement holepunch | Low |

---

## Phase 5: IPv6 Integration (TODO)

**Goal:** Full dual-stack networking.

| Item | Detail | Priority |
|------|--------|----------|
| **Core connection layer** | Change `CConnection` from `AF_INET` to dual-stack (`AF_INET6` with `IPV6_V6ONLY=0`) | High |
| **IPv6 in BT PEX** | 18-byte compact format for IPv6 peers | Medium |
| **IPv6 UPnP** | Port mapping for IPv6 | Medium |
| **UPnP GetExternalIP** | Currently returns empty; implement | Medium |
| **IPv6 in Kad DHT** | DHT already has IPv6 paths; integrate with core | Medium |
| **IPv6 in G2/G1** | Hub/ultrapeer connections over IPv6 | Low |

---

## Phase 6: Testing & Quality (IN PROGRESS)

**Goal:** Expand test coverage beyond HashLib.

### Done
- 13 HashLib unit tests (MD4, MD5, SHA-1, SHA-256, ED2K) — all passing
- CI/CD test execution (Release + Debug, Win32 + x64)
- OpenCppCoverage code coverage in CI
- Code analysis, formatting checks, markdown link checking

### TODO

| Item | Detail | Priority |
|------|--------|----------|
| **Envy core static library** | Refactor core into a static lib to enable protocol unit tests | High |
| **ED2K packet parsing tests** | Test handshake, concatenated UDP, compressed parts | High |
| **BT protocol tests** | Test bencode, magnet parsing, DHT messages | Medium |
| **Kad routing table tests** | XOR distance, bucket operations, node eviction | Medium |
| **Integration tests** | Protocol interop with reference clients (manual or containerized) | Low |
| **Performance benchmarks** | Baseline metrics for download throughput, UI responsiveness, memory | Low |

---

## Phase 7: Modern C++ & Long-Term (FUTURE)

| Item | Detail | Priority |
|------|--------|----------|
| **C++20 adoption** | Concepts, ranges, coroutines where beneficial | Low |
| **Smart pointer migration** | Replace raw `new`/`delete` with `unique_ptr`/`shared_ptr` | Low |
| **Constexpr usage** | Compile-time computation where applicable | Low |
| **CMake for full project** | Extend CMake to main app, services, plugins | Low |

---

## Priority Summary

### Immediate (High Priority)
1. ~~Fix SecureID version advertisement~~ -- Done (`ED2K_VERSION_SECUREID = 3`).
2. ~~Fix SourceEx2 / Kad FIND_VALUE + PUBLISH~~ -- Done (SourceEx2 and Kad search/publish implemented).
3. ~~BT protocol encryption~~ -- Done (MSE/PE integrated into `CBTClient` via `BTCrypto.h/cpp`; `Settings.BitTorrent.Encryption` setting added).
4. BT uTP transport -- Deferred (requires full UDP reliable transport implementation).

### Next (Medium Priority)
5. Compressed upload for ED2K (send COMPRESSEDPART).
6. Kad bucket splitting + LRU + refresh.
7. AICH C2C protocol.
8. BT v2 infohash + .torrent parsing.
9. IPv6 core connection layer.

### Later (Low Priority)
10. BT v2 wire protocol (hash req/res, hybrid mode)
11. Remaining ED2K opcodes (CALLBACK, BUDDY, PUBLICIP)
12. BT local peer discovery
13. HTTPS trackers
14. Full test suite expansion

---

## Development Principles

- **Evidence-based:** All priorities from code inspection against reference clients
- **Fix bugs before features:** Mis-advertised capabilities can cause interop failures
- **Incremental delivery:** Each phase delivers working functionality
- **Backward compatibility:** Maintain existing protocol compatibility
- **Profile before optimizing:** Performance changes guided by profiling

---

## Reference Implementations (Examples/)

| Protocol | Reference Clients | Location |
|----------|-------------------|----------|
| ED2K / Kad2 | eMule (srchybrid), aMule | `Examples/eMule/`, `Examples/aMule/` |
| BitTorrent / DHT | libtorrent, qBittorrent, Transmission | `Examples/libtorrent/`, `Examples/qbittorrent/`, `Examples/transmission/` |

These are gitignored reference sources used for protocol verification and compatibility checking.
