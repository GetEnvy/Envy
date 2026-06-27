# IPv6 Dual-Stack Implementation Plan (Living)

_Last Updated: 2026-04-22_
_Status Legend: `todo` | `in-progress` | `done` | `deferred`_

## Goal
Deliver feature-flagged IPv6 dual-stack support across Envy networking surfaces while maintaining backward compatibility for:
- legacy peers and trackers,
- existing on-disk caches/settings,
- IPv4-only deployments.

## Rollout guardrails
- **Primary flag:** `Settings.Connection.EnableIPv6` (default OFF initially) gates all new behavior; enable only after rollout validation confirms backward-compatible behavior.
- **Preference flag:** `Settings.Connection.PreferIPv6` influences dial order only.
- **Fallback policy:** if IPv6 initialization fails, continue in IPv4 mode.
- **Compatibility:** every disk format bump includes dual-reader fallback.

## Phase tracker

| Phase | Scope | Status | Exit criteria |
|---|---|---|---|
| 0 | Scoping, file inventory, migration planning | done | `docs/ipv6/SCOPE.md` + this plan committed |
| 1 | Address abstraction (`CEnvyAddress`) + API conversions (no wire changes) | todo | App behaves identically in IPv4-only mode |
| 2 | Dual-stack socket layer (TCP listen/connect + UDP send) | todo | v4-only, v6-only, dual peers connect locally |
| 3 | Host cache/discovery/filter/GeoIP persistence | todo | New cache versions + old-reader migration passes |
| 4 | BitTorrent BEP-7/BEP-32 IPv6 support | todo | peers6/nodes6 interop validated |
| 5 | ED2K transport-level IPv6 compatibility boundaries | todo | No ED2K payload-format regressions |
| 6 | Gnutella2 optional negotiated IPv6 address payloads | todo | Old peers interop unchanged |
| 7 | Kademlia family-split tables + persistence tags | todo | bootstrap/contact persistence validated |
| 8 | UI + settings + address inputs | todo | IPv6 display + validation complete |
| 9 | Unit/integration tests + CI wiring | todo | CI green with new IPv6 suites |
| 10 | Final docs/security/changelog/audit pass | todo | architecture+user docs + changelog finalized |

## Phase details

### Phase 1 — Address abstraction foundation
- Introduce `CEnvyAddress` wrapper (`sockaddr_storage`, family, scope id, port).
- API: parse/format, family queries, private/loopback/link-local/multicast checks, ordering/hash.
- Serialization helper (`family_tag + raw bytes`).
- Replace `IN_ADDR`/`SOCKADDR_IN` in core structs incrementally (Connection/Network/Datagrams/HostCache/Discovery first).
- Replace `inet_ntoa`/`inet_addr` call sites with `InetNtopW`/`InetPtonW` or abstraction helpers.

**Risks**
- High churn in signatures and map keys can ripple through all protocols.

**Mitigations**
- Add conversion helpers; keep temporary IPv4 bridge overloads during migration.
- Land in small compile-safe commits.

### Phase 2 — Socket layer dual-stack
- TCP listen: `AF_INET6` + `IPV6_V6ONLY=0` with IPv4 fallback if unavailable.
- UDP send/recv path selects family per endpoint.
- Add `Settings.Connection.EnableIPv6` and `Settings.Connection.IPv6Port` settings.
- UPnP/NAT-PMP attempt v6 mapping, log non-support as non-fatal.

**Rollback**
- Disable via `Settings.Connection.EnableIPv6=false` and retain prior IPv4 bind path.

### Phase 3 — Cache/discovery/filter/GeoIP
- Host cache and discovery store `CEnvyAddress`.
- Bump `hostcache.dat` and `discovery.dat` versions; dual-reader migration path.
- Backup legacy file (`.bak`) on first migration write.
- Blocklist parser: v4 and v6 CIDR.
- Integrate IPv6-capable GeoIP provider (GeoIP2) with legacy fallback.

### Phase 4 — BitTorrent IPv6
- Tracker: BEP-7 (`ipv6` param, `peers6`).
- DHT: BEP-32 (`want`, `nodes6`) and split v4/v6 routing tables.
- Accept/dial IPv6 for TCP/µTP.
- Ensure tracker URL parser accepts bracketed IPv6 literals.

### Phase 5 — ED2K IPv6 boundaries
- Respect ED2K wire IPv4 payload constraint.
- Support IPv6 transport to servers where feasible (AAAA/connect path).
- Optional/versioned extension for `server.met` import/export metadata only.
- Document behavior in `docs/ipv6/ED2K.md`.

### Phase 6 — G2 IPv6
- Add optional IPv6 child packets behind capability negotiation.
- Never send IPv6 payload children to peers without explicit support.

### Phase 7 — Kademlia
- Split contact tables and bootstrap sets by family.
- Persist contacts with family tags.

### Phase 8 — UI
- Render IPv6 in bracketed form where endpoint includes port.
- Add family indicator in host/neighbour/source lists.
- Settings toggles and public IPv6 visibility.
- Input validators switched to `getaddrinfo`-based acceptance.

### Phase 9 — Tests
- `CEnvyAddress` unit tests (parse/format, edge cases, mapped v4, link-local scope).
- v6 CIDR filter tests.
- BT peers6/nodes6 parser tests.
- Hostcache migration tests.
- Loopback dual-stack integration tests under `tests/ipv6/`.

### Phase 10 — Final docs and audit
- `docs/ipv6/ARCHITECTURE.md`, `docs/ipv6/USER_GUIDE.md`, `docs/ipv6/ED2K.md`.
- Update security audit for IPv6-specific threat cases.
- Update changelog and developer guidance with `CEnvyAddress` requirement.

## Risk register
1. **High:** API signature churn across legacy class hierarchy.
2. **High:** On-disk migration bugs causing host cache/discovery data loss.
3. **Medium:** Mixed-family firewall assumptions causing false firewalled state.
4. **Medium:** UI truncation/formatting regressions for long IPv6 literals.
5. **Medium:** G2 negotiation mistakes causing interoperability breakage.

## Rollback strategy
- Runtime rollback: set `Settings.Connection.EnableIPv6=false`.
- Code rollback: keep phased commits scoped by subsystem for selective reverts.
- Data rollback: preserve `.bak` of old cache files before first dual-stack write.

## Known limitations at planning stage
- Branch `claude/setup-docs-planning-iR4N0` was not present locally during this planning pass; work is staged on current branch.
- Existing `IPv6Support.*` utility is ED2K-oriented and not yet used as a universal address abstraction.
