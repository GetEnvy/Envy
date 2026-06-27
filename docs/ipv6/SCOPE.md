# IPv6 Dual-Stack Scope (Phase 0)

_Date: 2026-04-22_

## Objectives of this scope pass
- Identify current IPv6 readiness points (`AF_INET6`, `sockaddr_in6`, `in6_addr`, `getaddrinfo`).
- Identify IPv4-only assumptions (`sockaddr_in`, `AF_INET`, `inet_addr`, `inet_ntoa`, `IN_ADDR`, `DWORD` IP fields).
- Enumerate files that must change for full dual-stack support across core socket layer, BitTorrent, ED2K, Kademlia, G2, on-disk caches, and UI.
- Identify on-disk formats that currently assume 4-byte addresses and define compatibility migration expectations.

## Discovery commands used
```bash
rg -l "AF_INET6|sockaddr_in6|in6_addr|getaddrinfo|getnameinfo|InetNtop|InetPton|AI_V4MAPPED|IPV6_" Envy
rg -l "sockaddr_in|AF_INET\b|inet_addr|inet_ntoa|IN_ADDR\b|\bDWORD\s+\w*IP\w*" Envy
python (symbol density ranking for IPv4-only call sites)
```

## Current readiness snapshot

### Existing IPv6-related code (limited / partial)
- `Envy/IPv6Support.h`, `Envy/IPv6Support.cpp` already provide `IPv6Address` and `CIPv6Manager` helpers.
- BitTorrent DHT C library (`Envy/BitTorrentDHT/dht.*`) contains IPv6 primitives and `sockaddr_storage` usage.
- Settings already contain some ED2K-oriented flags (`PreferIPv6`, `EnableDualStack`, `IPv6ConnectTimeout`) but this is not wired as a global address-family abstraction.

### Structural IPv4 coupling hotspots
- Core types still use `SOCKADDR_IN` and `IN_ADDR` widely in connection, network routing, handshake, datagram, neighbour, and host/discovery caches.
- String conversion/logging is heavily `inet_ntoa`/`inet_addr` based.
- Cache indexes/map keys are `IN_ADDR` (`std::multimap<IN_ADDR,...>`), tightly coupling host cache internals to IPv4.

## Ranked file impact list

## Tier 0 — Foundation blockers (must land first)
1. `Envy/Connection.h`, `Envy/Connection.cpp`
   - Root socket/address carrier for all protocols (`m_pHost`, `ConnectTo`, `AcceptFrom`).
2. `Envy/Network.h`, `Envy/Network.cpp`
   - Resolver, local address acquisition, reserved/firewall checks, socket wrappers.
3. `Envy/Handshakes.h`, `Envy/Handshakes.cpp`
   - TCP listen/accept path and push-connect plumbing.
4. `Envy/Datagrams.h`, `Envy/Datagrams.cpp`
   - UDP send/recv, multicast handling, SGP logic.
5. `Envy/HostCache.h`, `Envy/HostCache.cpp`
   - Central cache key/storage model and serialization.
6. `Envy/DiscoveryServices.h`, `Envy/DiscoveryServices.cpp`
   - Discovery address parsing/resolution and persistence.
7. `Envy/Neighbour.h`, `Envy/Neighbour.cpp` + handshake subclasses
   - Protocol-facing connection initiation state paths.
8. New core abstraction target: `Envy/CEnvyAddress.*` (to be introduced).

## Tier 1 — Protocol wire and routing logic
### BitTorrent
- `Envy/BTTrackerRequest.cpp` (BEP-7 `ipv6`, `peers6`).
- `Envy/BTPacket.h`, `Envy/BTPacket.cpp` (compact peer parsing/emission).
- `Envy/BTClient.cpp`, transfer classes (`DownloadTransferBT.cpp`) for dial/accept handling.
- `Envy/BitTorrentDHT/dht.c`, `dht.h`, `dht.bootstrap.c` integration seams for BEP-32 and dual tables.

### ED2K/eDonkey
- `Envy/EDClient.cpp`, `Envy/EDClients.cpp`, `Envy/EDNeighbour.cpp`, `Envy/EDPacket.cpp`.
- `Envy/NeighboursWithED2K.*`.
- Keep payload compatibility IPv4-only while enabling IPv6 transport where possible.

### Kademlia
- `Envy/Kademlia.h`, `Envy/Kademlia.cpp`, `Envy/KadProtocol.h`, `Envy/KadProtocol.cpp`, `Envy/KBucket.*`.
- Separate contact tables/bootstrap per family, serialization tags.

### Gnutella2
- `Envy/G2Packet.h`, `Envy/G2Packet.cpp`, `Envy/G2Neighbour.cpp`.
- `Envy/QueryHit.cpp`, `Envy/RouteCache.*`, `Envy/PongCache.*` for embedded address nodes.

## Tier 2 — Persistence / filtering / services
- `Envy/Security.cpp`, `Envy/SecureRule.*` (IP filter/block list parsing and range checks).
- `Envy/HostBrowser.*`, `Envy/RemoteSecurity.*` (address matching and remote policy checks).
- `Services/GeoIP/*` (new GeoIP2 integration area for IPv6 support).

## Tier 3 — UI and user settings
- `Envy/PageSettingsConnection.*` (Enable IPv6, prefer IPv6, IPv6 port display).
- `Envy/WndHostCache.*`, `Envy/WndNeighbours.*`, downloads/search source display controls (`CtrlDownloadTip.cpp`, `CtrlUploadTip.cpp`, etc.) to bracket IPv6 literals and show family.
- Address-input flows (`DlgDiscoveryService.*`, manual connect fields, allow/deny list editors).

## Tier 4 — Additional medium/low-impact IPv4 assumptions
High-density additional files detected by scan (selected):
- `Envy/QueryHit.cpp`, `Envy/QuerySearch.cpp`, `Envy/ManagedSearch.cpp`
- `Envy/DCClient.cpp`, `Envy/DCNeighbour.cpp`
- `Envy/G1Packet.cpp`, `Envy/G1Neighbour.cpp`
- `Envy/RouteCache.cpp`, `Envy/HubHorizon.cpp`
- `Envy/Remote.cpp`

These should be addressed after Tier 0/1 conversion establishes `CEnvyAddress` and conversion utilities.

## On-disk format assumptions and migration plan requirements

### 1) `hostcache.dat`
- Serialized by `CHostCache::Serialize`, `CHostCacheList::Serialize`, `CHostCacheHost::Serialize`.
- Currently stores `IN_ADDR` semantics and map keys by IPv4 address.
- Migration required:
  - Bump cache serialization version.
  - New record format: `family_tag (1 byte) + address (4 or 16 bytes) + port fields`.
  - Reader must support old (IPv4-only) and new (dual-stack) versions.
  - On first successful migration, write `.bak` backup of prior file.

### 2) `discovery.dat`
- Serialized by `CDiscoveryServices::Serialize`, `CDiscoveryService::Serialize`.
- `CDiscoveryService` currently carries `IN_ADDR m_pAddress`.
- Migration required:
  - Version bump and dual reader.
  - Support persisted literal IPv6 endpoints and resolved AAAA addresses.

### 3) eDonkey `.met` / server list imports (`server.met`, node imports)
- `CHostCache::ImportMET` and related importers assume IPv4 layout for server entries.
- Constraint: maintain wire compatibility while allowing optional versioned extension field for IPv6 server endpoint metadata.
- Reader must remain tolerant of legacy records and extensions.

### 4) Settings storage
- `Settings.Connection` currently has IPv4-centric host/port fields.
- Add global `Settings.Connection.EnableIPv6` + `Settings.Connection.IPv6Port` (and preference toggle) with defaults preserving existing behavior.
- Preserve old `Settings.Connection` keys; new `Settings.Connection.*` keys remain optional with safe defaults.

## Compatibility requirements captured for implementation phases
- No protocol regression in IPv4-only mode.
- Any new wire payloads must be optional/negotiated (especially G2).
- ED2K payload IP structures remain IPv4 as per spec; IPv6 only where transport permits.
- All serialization/wire changes require versioning + backward readers.

## Notes for phase sequencing
- Implement address abstraction first, then socket/listen changes, then per-protocol rollout.
- Defer broad UI conversion until core and caches are stable.
- Prioritize tests for parse/format + migration before protocol-specific changes.
