# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- **Peer XML resource limits** — Bound peer XML to 256 KiB, 32 levels and 4,096 retained nodes/attributes, sharing budgets across G1/G2 metadata fragments. Reject oversized or incomplete HTTP metadata/THEX responses, preserve profile ownership and incremental THEX descriptor identity, reject empty file responses before changing download size, and drain body bytes beyond the header receive cap before connection-close completion. THEX validates descriptor association, segment size, both historical depth conventions and complete binary rows; missing URI fallback requires a known root. Reject malformed numeric lengths and DIME framing. Protocol identifiers and generated wire data are unchanged.
- **BugSplat x64 runtime staging (#362)** — Post-build and release portable staging copy MSVC dependents reported by `dumpbin /dependents` on `BugSplatMonitor.exe` (via `Get-BugSplatOutputRequiredDlls`), with deterministic layout checks in CI; build fails closed when required CRT companions are missing from the output tree.
- **Bootstrap catalogue reconciliation** — Remove stale/duplicate ED2K sources, the retired G1 UHC port and a Kad nodes.dat URL returning 404; add the verified Team Elite HTTPS NMDC hublist and align cold-start documentation with the independent October 10 endpoint audit. Offline tests reject unexpected, duplicate and truncated catalogue entries and preserve blocked services. Existing persisted discovery entries are not migrated.
- **Kad2 SEARCH_SOURCE_REQ wire compatibility (#372)** — Send the maintained aMule-compatible 26-byte request body (`FileHash + StartPosition + FileSize`) with explicit little-endian integers instead of `CPacket::WriteInt64`'s packet-endian behavior. Inbound handling accepts exact 16-byte hash-only, historical 24-byte size-only, and current 26-byte Kad2 bodies, rejects ambiguous/truncated lengths, and honors the 15-bit start position when selecting stored results. Golden-vector tests cover framing, endianness, legacy forms, malformed lengths, and start-position masking.
- **Remote localhost bind policy (#390)** — `Settings.Remote.BindAddress` remains unbound (D-017) but localhost-only values (`127.0.0.0/8`, `::1`, `localhost`) now deny non-loopback Remote HTML clients unless `AllowExternal` is set; IPv6 loopback is recognized from the logged client address when present, and login fails closed for IPv6-shaped peers instead of deriving a placeholder IPv4 security identity (D-017). EnvyTests cover policy precedence.
- **Connection capacity vs transfer limits (#342)** — `Connection.InSpeed` / `OutSpeed` remain declared link capacity (Kb/s) for tuning, warnings, and scheduler-derived caps; `Bandwidth.Downloads` / `Uploads` (bytes/s, `0` = Unlimited) are the only user transfer-rate caps. Unlimited downloads/uploads no longer inherit a hidden ceiling from legacy default capacity (4096/768 Kb/s). Changing capacity on Settings → Connection no longer overwrites explicit upload limits; the first-run wizard still seeds upload headroom once. One canonical capacity preset list (56 Kb/s–10 Gb/s Ki-based) is shared by the Connection page and wizard, with multigig-safe manual parsing.
- **NMDC `$MaxedOut` queue rank handling (#329)** — Treat an empty `$MaxedOut|` token as busy, while numeric tokens carry only a validated queue position; unknown total queue length remains zero. Add EnvyTests coverage for malformed, overflow, and queue-limit cases.
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- **Kad HTTPS `nodes.dat` cold-start bootstrap** — When `HostCache.Kademlia` is empty, Envy downloads validated `K` catalogue URLs (256 KiB cap), imports contacts before persisting `nodes.dat`, and uses same-directory replace with `.lkg` backup. Exponential backoff resets after a cooldown cycle instead of locking out until restart. Kad2 routing/search interop remains partial (#86 / #160).
- **BitTorrent keep-alive framing (#88/#91)** — Preserve the next message length prefix when a keep-alive is followed by a normal BitTorrent frame in the same TCP buffer. The parser now returns the keep-alive without consuming the following frame marker, so the next `ReadBuffer` pass sees the complete message. Added EnvyTests coverage for keep-alive marker removal and keep-alive+choke sequencing.
- **Library collection mount null crash (#300)** — `CAlbumFolder::MountCollection` no longer calls a method on a null subfolder (`this == nullptr`, `0xC0000005`). Child pointers are not inserted into `m_pFolders`; `GetNextFolder` asserts the invariant in Debug; Release still skips a corrupt null entry. `CLibraryFolders::MountCollection` returns false if the album root is not created yet.
- **Interop harness exchange labels + cross-packet evidence (#160, #294)** — Kad HELLO/PING/FIND_NODE use direction-specific labels (req+res both required); packet-evidence aggregates labels across transport-tagged `from-pcap-tcp-<stream>-<src>-<dst>-*` / `from-pcap-udp-*` files; TCP reassembly joins only within the same unidirectional flow (never opposite directions or UDP); SEARCH_RES accepts complete eMule/legacy layouts and fails closed on ambiguity; `PUBLICIP_REQ` empty-body check; COMPRESSEDPART allows partial chunks; Hello `userhash` redacted in attachable evidence; missing configured evidence paths FAIL; `source_exchange` accepts SourceEx or SourceEx2; negative `pcap_duration_sec` rejected; early capture death before requested stop FAILs even if partial bytes exist; live `--operator-hold-sec` / `run-live.ps1 -OperatorHoldSec` pauses after startup for manual GUI steps; malformed labeled evidence FAILs.
- **Interop harness fail-closed Kad evidence (#160, #294)** — Kad HELLO/PING/FIND_NODE require minimum body shapes; Kad hits inside ED2K TCP frame spans are ignored; evidence JSON uses fixed `source_name`; `optional_pcap` PASSes only after usable stopped capture (early capture exit FAILs). SEARCH_RES accepts eMule `<SenderID 16><TargetID 16><Count 2>` and Envy outbound `<Hash 16><Count 1>`; tshark extracts per-packet chunks (not flattened); raw `.bin` skipped only under sanitized captures (logs `stdout.bin` kept); `capability_negotiation` requires Hello+HelloAnswer.
- **Interop harness Kad capture + HELLO opcodes (#160, #294)** — Kad2 HELLO constants are `0x11`/`0x19` (not Bootstrap `0x01`/`0x09`). Optional pcap BPF includes Kad UDP (default 4672). `kad_hello` / `kad_ping_pong` / `kad_find_node` carry evidence labels for packet-evidence PASS. Owned `--enable-pcap` lifecycle can PASS `optional_pcap`; runner extracts payloads via tshark into transport-tagged `captures/evidence/from-pcap-tcp-<stream>-<seq>.bin` / `from-pcap-udp-<seq>.bin`. Evidence JSON stores fixed `source_name` (`packet-evidence.bin`).
- **Interop harness evidence honesty (#160, #294)** — Kad packet-evidence labels use UDP `0xE4` / `SEARCH_RES` `0x3B` (not C5/`0x35`). PUBLICIP/FIREWALLED record `ipv4_present` only (no raw IPv4). `kad_nodes_dat_local` and `optional_pcap` SKIP when only notes/availability exist. CALLBACK body must be exactly 38 bytes; COMPRESSEDPART declared payload length must match remaining body; tcpdump `-G/-W` precede the BPF expression; opcode-only packet evidence cannot PASS transfer/callback/SEARCH_RES-delivery scenarios.
- **PR validation governance** - Keep staged Draft/live-test/Ready validation, one genuine non-author approval, eight required status contexts, CodeQL/Gitleaks, and GitHub Code Quality. Retain `pr-phase`; retire the Dependabot auto-merge workflow in favor of maintainer manual squash with a curated technical body. **Only the maintainer** applies `stage:live-test`, marks Ready, and merges. Privileged governance paths (`AGENTS.md`, `.github/skills/**`, workflows, settings, rulesets, gate scripts) require an independent human `APPROVED` — Copilot must not be the sole counted approval on those diffs; keep them outside the Copilot path allowlist. Review-loop tooling is tracked separately in #397.

### Added

- **BugSplat x64 crash reporting (#353)** — x64 uses BugSplat SDK v8.0.0 (static `/MT`, `BugSplatMonitor`, native consent dialog). Win32 keeps Crashpad. Root `crashpad` vcpkg dependency is Win32-only; import official SDK into `ThirdParty/BugSplat` before building x64.
- **Adaptive Network list columns (#295 Phase 1)** — Pure `AdaptiveListColumns.h` allocator (fixed/bounded/flexible + sticky user widths + DPI scale) with EnvyTests; `WndNeighbours` / `WndHostCache` apply it on resize/skin without registry writes during `WM_SIZE`. Default `InsertColumn` widths use `SCALE()`. Sticky capture prefers `HDN_ENDTRACK`/`cxy` and header `hwndFrom`; `AdaptiveScalePx` uses saturating 64-bit math; `ListStates *.Sticky` persists user columns across restart (non-sticky normalized before `SaveList`). No protocol behavior change.
- **Desktop UI modernization architecture (#295)** — Progressive MFC UI plan (no Qt/WinUI rewrite): adaptive Network lists first, then shell/Transfers/Home/Library/Settings/Media/Chat/assets. Documents verified Host Cache/Neighbors fixed columns, `SizeLists` default off, DPI/`SCALE()` precedent from #134/#135/#137, and Emoticons/Flags/string Debug classifications. Hit-size guidance: pointer floor `SCALE(24)` vs ~40 EP touch. Canonical: `docs/20_arch/UI_MODERNIZATION.md`.
- **ED2K/Kad interop harness preparation (#160)** — Upgrade `tools/interop/` to reflect current develop after #251/#252/#254/#255/#256/#257/#258/#261: capability table with `compression_send=True`, orthogonal production/harness/evidence states, current compression/LowID/Kad scenario hooks, Windows `run-live.ps1` + operator checklist, optional dumpcap/tshark, bounded packet-evidence extractor, report schema v2. Deterministic Cloud self-test/dry-run only; live evidence still pending a Windows operator run. No ED2K/Kad production protocol changes.
- **Kad app-trigger source search (#86)** — ED2K downloads call `CKademlia::SearchSource` from `FindMoreSources` / automatic starve paths when `EnableKad` is on and Kad is initialized. Outbound `KADEMLIA2_SEARCH_SOURCE_REQ` (0x34) now includes little-endian `FileSize` (`<FileHash 16><FileSize 8>`). Inbound still accepts legacy hash-only. Helpers: `Envy/KadSearchSourceRequest.h`. Smoke: `tests/test_kad_search_source_request_smoke.cpp`. Buddy/UDP firewall / Hello Kad nibble unchanged; Kad2 remains partial/unverified.
- **Local vcpkg bootstrap for Visual Studio** — `scripts/bootstrap-vcpkg.ps1` / `.cmd` restores the root `vcpkg.json` manifest (`x64-windows-static` / `x86-windows-static`) the same way CI does before MSBuild. Fresh Debug|x64 checkouts were failing in `PreBuild.cmd` (`vcpkg_installed\… not found`, then missing `EnvyOM.h` because MIDL never ran). Visual Studio does not restore the manifest before `PreBuildEvent`. See `docs/10_dev/build.md`.
- **Kad TCP firewall-detection baseline (#86)** — Parse/emit Kad `FIREWALLED_REQ` (0x50, exact 2-byte TCP port) and `FIREWALLED_RES` (0x58, exact 4-byte observed IPv4). Bounded outbound checks against verified routing contacts; public-IP consensus requires two independent agreeing peers; TCP Open only after two matching ACKs (`FIREWALLED_ACK_RES` 0x59 or ED2K `0xA8`). UDP firewall tester, Buddy, and callback are **not** included. Hello Kad nibble stays 0. `Envy/KadFirewallCheck.h` + `tests/test_kad_firewall_check_smoke.cpp`.
- **ED2K LowID / C2C callback baseline (phase 1, #87)** — Handle `PUBLICIP_REQ`/`PUBLICIP_ANSWER` (0x97/0x98) and Buddy-delivered `CALLBACK` (0x99, 38-byte layout) with bounded query/consume state (`Ed2kLowIdCallback.h`, `CEDClient`). Classic server push unchanged. `REASKCALLBACKTCP`/Buddy/FWCHECK deferred. Docs: `docs/30_protocols/ed2k/ED2K_LOWID_CALLBACK_BASELINE.md`. Smoke: `tests/test_ed2k_lowid_callback_smoke.cpp`. **Partial** — not complete firewalled support.
- **ED2K live interop harness (#160, phase 1)** — Opt-in `python3 tools/interop/run.py` records ENVY ↔ eMule Community / aMule evidence with PASS/FAIL/SKIP/NOT_IMPLEMENTED, isolated profiles, sanitized artifacts, binary identity, process exit states, Hello/MuleInfo evidence fields, and Envy self-golden Hello parse. Live binaries and the public P2P network are **not** required CI. Optional `workflow_dispatch` job `ED2K interop harness` repeats self-test/dry-run only. This does **not** claim ENVY is fully interoperable. See `tools/interop/README.md`.
- **Kad `nodes.dat` v1/v2/v3 parser** — `Envy/KadNodesDat.h` reads eMule/aMule-compatible nodes files (legacy v0 parsed but Kad1 contacts dropped; new-format v1/v2/v3). v3 bootstrap edition keeps at most 50 XOR-closest Kad2 contacts. UDP-key/`verified` fields are consumed and discarded (no runtime UDP-key protocol). File cap 256 KiB; overflow-safe count checks. Local `DataPath\nodes.dat` (and optional eMule/aMule copies) feed `CKademlia::Bootstrap()` via `HostCache.Kademlia`. Remote HTTP download is **not** added. Kad2 remains partial / unverified; Hello Kad nibble stays 0. Tests: `tests/test_kad_nodes_dat.cpp`. See `docs/30_protocols/kad/kad2-compatibility-report.md`.
- **Remote API architecture (docs + state map)** — Audit of native control API vs *arr download-client vs Torznab client. Decisions D-017 (`http.sys` inbound, WinINet outbound, no extra HTTP frameworks) and D-018 (first *arr adapter = qBittorrent Web API v2 **subset**). `Envy/TransferState.h` (`enum class`) maps core predicates to stable states and fail-closed qBit/Transmission strings. Planned OpenAPI stub. **Not** a served JSON API and **not** Radarr support. See `docs/20_arch/AUDIT_REMOTE_API_2026-09.md`. Issues #239–#241.
- **Crashpad local crash reports (#90)** — Crashpad replaces BugTrap. Debug and Release start `crashpad_handler.exe` with a local database under `%LOCALAPPDATA%\Envy\CrashReports\` and **upload disabled**. The next launch can copy sanitized text, open the folder, or open GitHub’s issue page. Dumps are never uploaded. See `docs/50_user/crash-reports.md` and `docs/10_dev/crash-reporting.md`.
- **Uploads Fair-Use (10% media)** — `Uploads.FairUseMode` is opt-in and **implemented**: each remote IPv4 client is clipped to 10% of an audio/video library file. GET/ED2K/DC reserve on the host+path ledger and charge body bytes as they are written; unused reservation rolls back on the next request or close so HTTP keep-alive HEAD / aborted transfers do not burn the quota. HTTP HEAD still clips the advertised range. Checkbox enabled and DDX-bound. HTTP 416 / ED2K skip / DC `$Error` when the host already received its share. BitTorrent and partials are not limited. Mapping: `docs/50_user/transfer-settings.md`.
- **NMDC hub user list + remote file-list browse** — Hub chat user sidebar can private-message and Browse a remote nick (`hub+nick`, not a global nick). Browse reuses `CHostBrowser` `PROTOCOL_DC` (`dchub://nick@hub:port/files.xml.bz2`). `$NickList` is a nick-only seed merged with `$MyINFO` (no duplicates; 20,000-user cap enforced on both paths). Hostile `files.xml.bz2` lists are fail-closed (compressed/uncompressed size capped at transfer time, bounded XML walk for depth/entries/Name/path-traversal; TTH-less File entries skipped). Incoming FileListing directories feed the existing Browse Host left tree from an owned path/index snapshot (not a DC++-identical control). Outgoing DC file lists omit unhashed files (no fake TTH). This is **not** ADC hub support and **not** “full DC++ support”. Live hub interop remains unverified in CI.
- **Transfer settings limit helpers** — `Envy/TransferSettingsLimits.h` plus EnvyTests (`test_transfer_settings_limits_smoke.cpp`) for unlimited bandwidth tokens, DWORD overflow clamp, `MaxPerHost` range, and Fair-Use 10% clip math. See `docs/50_user/transfer-settings.md`.
- **`CTextCtrl` viewport helpers + EnvyTests** — `TextCtrlViewport.h` pure scroll/geometry math with smoke tests; manual UI checklist in `docs/10_dev/textctrl-log-ui-checklist.md`.
- **Cloud Agent Linux dev environment (`.cursor/environment.json`)** — Cloud Agents now boot a ready-to-use Linux environment for Envy's cross-platform checks. The install step provisions `clang-format-18`/`clang-tidy`/`cppcheck` and installs the Remote web UI security test dependencies (`npm --prefix Remote/tests ci`). The MFC/ATL app and HashLib remain Windows-only (MSVC v145); this environment targets the same checks CI runs on `ubuntu-latest` (Format Check uses clang-format 18), with `cppcheck` available as an additional local analysis tool.
- **Cross-platform foundations (docs)** — Canonical plan for progressive EnvyCore / platform abstraction / retained MFC Windows frontend; Linux/macOS marked **planned** (not supported); Win32 legacy Stage A policy; CMake portable-slice priority (D-012…D-015). See `docs/20_arch/PORTABILITY_PLAN.md`.
- **DevSecOps tooling (local + PR advisors)** — `scripts/ci-fast.ps1` / `scripts/ci-verify.ps1` for a Windows-local gate approximating MSVC/tests; Dependabot owns vcpkg, `Remote/tests` npm, and GitHub Actions updates; CodeRabbit (`.coderabbit.yaml`) and clang-tidy→reviewdog (`.github/workflows/clang-tidy-pr.yml`) are advisory PR reviewers. See `docs/10_dev/devsecops-envy.md` and `docs/10_dev/dependency-automation.md`.

- **4.2.0 Preview 1 release packaging** — Unified product version metadata (`version.json` `4.2.0-preview.1`, Windows `FILEVERSION`/`PRODUCTVERSION` `4.2.0.1`, display `4.2.0 Preview 1`). Release workflow builds per-platform Inno Setup installers (`InstallerAlpha=Preview` from tags containing `preview`), publishes `Envy-<version>-{x64|win32}-{setup.exe|.zip}` plus `SHA256SUMS.txt`, and creates a **draft** GitHub Release marked **prerelease** when the tag contains `preview`/`beta`/`rc`/`alpha`. Preview 1 uses separate x64 and Win32 setups (unified universal installer deferred).
- **Release pipeline validation scripts** — `scripts/release/verify-version.ps1`, `stage-portable.ps1`, `verify-artifacts.ps1`, `publish-draft-release.ps1`, and `repair-draft-release.ps1` gate tag/`version.json`/`Envy.rc`/`Envy.exe` consistency, stage a full portable runtime tree, verify SHA256 + ZIP/setup sanity, and support idempotent draft asset repair.

### Fixed
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- **HashStringConversion drop `register` (#84)** — Remove C++17-illegal `register` from `Unhex` in `Envy/Hashes/HashStringConversion.cpp`. No hex decode behavior change.
- **Settings Add/SmartAgent `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on five `CSettings::Add` overloads and `SmartAgent` (`Item` ctors deferred for Format Check). Behavior unchanged.
- **Crashpad Debug double-start** — `CrashReporter::Initialize` only from `InitInstance` (removed ctor call). `CrashPadHost::Start` is idempotent so a second `StartHandler` cannot hit Crashpad’s Debug `DCHECK(ipc_pipe_.empty())`. See `docs/10_dev/crash-reporting.md`.
- **ComObject `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CComObjectPtr` members (Format Check spacing on touched decls). Behavior unchanged.
- **HashTest `register` / `throw()` modernization (#84)** — Drop leftover `register` storage class and replace `throw()` with `noexcept` in `HashLib/HashTest/HashTest.cpp`. Test harness only; no production hash behavior change.
- **SafeRelease `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specification on `SafeRelease` (collapse template spacing for Format Check). Behavior unchanged.
- **HGlobal `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on dtor / `Clean` / conversions / `IsValid` / `Detach` / `Size` (ctors deferred for Format Check). Behavior unchanged.
- **TransferFile `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `IsOpen` / `IsExists` / `IsWritable` / `IsFolder`. Behavior unchanged.
- **StreamArchive `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on dtor / `operator LPSTREAM` / `Detach` / `IsValid` (ctors deferred for Format Check). Behavior unchanged.
- **StdAfx.cpp `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `InitGetMicroCount` / `NoThrowNew` / OOM handlers. Behavior unchanged.
- **CLocked `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CLocked::operator T` / `operator->`. Behavior unchanged.
- **CoolMenu `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `SafeTrackPopupMenu` / `SafeQueryContextMenu`. Behavior unchanged.
- **EnvyFile `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specification on `CEnvyFile::GetSize`. Behavior unchanged.
- **Shell `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specification on `CShellItem::operator LPITEMIDLIST`. Behavior unchanged.
- **BTTrackerRequest `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specification on `CAutoPtr<CBTTrackerRequest>::Free`. Behavior unchanged.
- **ThreadImpl `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on ten `CThreadImpl` inlines (`BeginThread`/`CloseThread`/`Wait`/`Wakeup`/`Doze`/…). Behavior unchanged.
- **UPnPFinder `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CreateFinderInstance` / `ProcessAsyncFind` (header + cpp). Behavior unchanged.
- **EDPacket `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CEDPacketTypes` ctor and `GetAt` in `Envy/EDPacket.cpp`. Behavior unchanged.
- **Library `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CLibrary::SafeReadTime` / `SafeSerialize` (header + cpp). Behavior unchanged.
- **Application `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CApplication::GetApp` / `GetUI` / `GetSettings` (header + cpp). Behavior unchanged.
- **SQLite `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `CDatabase` `operator bool` / `IsBusy` / `GetCount` (header + cpp). Behavior unchanged.
- **VersionChecker `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on `IsUpgradeAvailable` / `IsVerbose`. Behavior unchanged.
- **Handshakes `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specification on `CHandshakes::IsValid`. Behavior unchanged.
- **SkinWindow drop `register` (#84)** — Remove C++17-illegal `register` from the alpha-blend temp in `Envy/SkinWindow.cpp`. No blend behavior change. Remaining first-party uses: HashStringConversion, HashTest, …
- **UTF-16 byte-swap drop `register` (#84)** — Remove C++17-illegal `register` from identical endian-swap temps in `Envy/XML.cpp`, `Envy/DlgLanguage.cpp`, and `Envy/PageSettingsSkins.cpp`. No decode behavior change. Remaining first-party uses: SkinWindow, HashStringConversion, HashTest, …
- **MatchObjects drop `register` (#84)** — Remove C++17-illegal `register` from `CMatchFile::Compare` sort locals in `Envy/MatchObjects.cpp`. No sort behavior change. Remaining first-party uses: XML, PageSettingsSkins, SkinWindow, DlgLanguage, HashStringConversion, …
- **QueryHash drop `register` (#84)** — Remove C++17-illegal `register` from `Envy/QueryHashTable.cpp` / `Envy/QueryHashGroup.cpp` hash merge loops. No behavior change. Remaining first-party uses: MatchObjects, XML, …
- **`Envy/Strings.cpp` drop `register` (#84)** — Remove C++17-illegal `register` storage class from string helpers (ToLower buffer walks / UTF-16 byte swap). No behavior change.
- **HostCache `throw()` → `noexcept` (#84)** — Replace removed-in-C++20 dynamic exception specifications on ten `CHostCacheList` inline accessors (`Begin`/`End`/`Find`/`Check`/`CountHosts`/…). Behavior unchanged; matches existing `noexcept` comparators in the same header.
- **`bootstrap-vcpkg.cmd` exit code** — Read the child `pwsh`/`powershell` status with delayed expansion; a parenthesized `if` was expanding `%ERRORLEVEL%` from `where pwsh` and reporting a failed restore as success.
- **HashLib `CTigerTree` MSVC C5038/C5033 (#84)** — Reorder the `CTigerTree` constructor initializer list to match `TigerTree.h` member order (C5038) and drop the leftover `register` storage class in `CTigerTree::Tiger` (C5033). Tiger/TTH algorithm, hash bytes, and wire format are unchanged. EnvyTests cover constructor defaults plus identical-input / incremental / empty-file root stability (no invented golden digest).
- **Kad2 routing-table maintenance (#86 slice)** — Replaced the fixed 128 XOR-distance bucket array with an aMule/eMule-style zone tree (`Envy/KadRoutingTable.h`): split when a full leaf may split (`CanSplit`: level < 127 and K=10 and (`prefixInteger` < KK or level < KBASE); 128-bit `zonePrefix` so depth ≥ 32 is shift-safe), LRU + type liveness, 1-slot replacement cache (verified healthy contacts kept; stale/dead replaced), bounded stale-zone FIND_NODE refresh (first timer arms +10s), and `/24` diversity (2 per leaf / 10 global / 1 Kad ID per IP). `verified` is outstanding HELLO_RES only. FIND_NODE_RES is fail-closed on truncated lists and must match TargetID before liveness. After rebase onto #256, firewall candidate collection walks the zone tree (`ForEachContact`) instead of `KAD_BUCKET_COUNT`/`buckets`. Packet formats unchanged; ED2K Hello Kad nibble stays 0; Kad2 remains partial/unverified. Tests: `tests/test_kad_routing_table.cpp`. Buddy/callback not in this slice.
- **ED2K C2C CALLBACK review follow-up (#255)** — Accept incomplete downloads in the known-file gate (`Downloads.FindByED2K` without shared-only). Clear the one-shot consume guard when `EDClients.PushTo` fails so a valid callback can retry within the window.
- **TransferState smoke registration** — Register `transfer_state_downloading_finished_not_importable` so the fail-closed `MapTransferStateToQBittorrentFinished(Downloading) → error` assertion from #242 actually runs in EnvyTests (22/22 on Linux g++).

### Changed
- **Agent PR back-pressure** — Hard max of 3 development PRs becomes a **soft target of 5** (Dependabot/Renovate excluded). Opening a sixth PR needs a documented reason in the PR body. Canonical: `AGENTS.md` hard rule 13 / `docs/10_dev/devsecops-envy.md`.
- **Branch naming for agents (hard rule 11)** — `AGENTS.md` / CONTRIBUTING / Cursor workflow pointers forbid tool prefixes (`cursor/`, `claude/`, …) and Cloud-runner slug templates; only `type/short-kebab-summary`. CONTRIBUTING no longer suggests `claude/` branches.
- **SonarCloud analysis scope (#234)** — `.sonarcloud.properties` excludes vendored `Services/*` / wizard / duplicate FictionBook trees from analysis and CPD so New Code duplication is not dominated by third-party clones. See `docs/10_dev/sonarcloud-exclusions.md`.
- **CI always-emit Build x64/Win32 Release on PRs** — Required Build jobs always run a terminal conclusion (never SKIPPED). Draft without `stage:live-test` may defer on ubuntu; Ready and live-test always run real MSBuild + EnvyTests. See `docs/10_dev/CI_AUDIT_2026-09.md`.
- **Architecture / roadmap / status** — Document EnvyCore interface rule (no new MFC/Win32 types on portable core APIs), clarify CMake full-app vs portable-slice, and resolve the Windows-only vs multi-OS open question toward long-term multi-OS with Windows-first delivery.
- **CI PR `ready_for_review` + dispatch Format Check** — Build, Code Quality, PR Gate, Security, CodeQL, and Dependency review also run when a draft PR is marked ready. Format Check / Documentation Check can run on `workflow_dispatch` (Static Analysis stays push/schedule only). *(historical: PR Gate removed)*
- **DC hublist sources (2026-09)** — Default `DC.HubListURL` is `https://dchublist.org/hublist.xml.bz2`. `DefaultServices.dat` `H` rows use that URL plus HTTPS `hublist.pwiam.com` and `dchublist.ru`. Dead `dchublist.com` (HTTP-only, 301 to Team Elite) and `tankafett.biz` (timeout) are commented. Settings > Networks > DC++ > Download uses `CUpdateServersDlg` in hublist mode (no eDonkey/server.met copy). eDonkey `server.met` dialog text is unchanged. `ImportHubList()` still accepts `dchub://` and skips `adc://` / `adcs://`.
- **CI audit + micro-optimizations (2026-09)** — Documented measured PR/push timings and critical path in `docs/10_dev/CI_AUDIT_2026-09.md` (including Protect main API snapshot). Skip empty NuGet restore in `windows-msbuild` (~22s/job measured no-op), upload PR build logs only on failure, always emit required `Documentation Check` (classify no-op when out of scope; cancelled/superseded classify also no-ops success), keep PR Gate `POLL_SEC` at script default 15s (no faster override — Actions API quota), and record live Protect develop approval-count drift vs docs. *(historical: PR Gate removed; signatures optional)*
- **Bootstrap catalogues (2026-09-18)** — Refresh `Data/DefaultServices.dat` / `Data/DefaultServers.dat`: drop static ED2K IPs and dead GWC/hublists; HTTPS `server.met` (eMule-Security, shortypower) and DC hublists (dchublist.org/ru, Team Elite); gtk-gnutella UHCs; Transmission/libtorrent DHT routers. `CDHT::Connect` cold-start sends BEP 5 `find_node` to HostCache BitTorrent hosts instead of a hard-coded DNS name. Kad remote `nodes.dat` is **not** added (Kad2 remains partial). See `docs/30_protocols/bootstrap-sources.md`.
- **Uploads/Downloads limit wording (foundation)** — Bandwidth combo shows localized **Unlimited** (`IDS_SETTINGS_BANDWIDTH_UNLIMITED`; still accepts legacy `MAX`/`NONE`). Uploads “Mode” is **Throttle** (Average soft vs Maximum strict). “Limit per unique host” is **Max uploads per host** (`Uploads.MaxPerHost`, default 2, range 1–64). `Bandwidth.Uploads`/`Downloads` QWORD→DWORD conversion clamps instead of truncating. Mapping: `docs/50_user/transfer-settings.md`.
- **CI CodeQL + Format gates** — Every PR to `develop` always runs CodeQL `Analyze (c-cpp)`, `Analyze (javascript-typescript)`, and `Analyze (csharp)` (no classify-changes dependency). Format Check uses pinned `clang-format-diff-18` on changed hunks only and fails closed on tooling/git errors. PR Gate requires `success` for must_pass (rejects `neutral` / unexpected `skipped`). *(historical: C# now classifier-selected; PR Gate removed)*
- **Format Check legacy encoding** — Format Check invokes `.github/scripts/clang-format-diff-safe` (binary/latin-1 I/O) so ISO-8859 first-party sources no longer crash stock `clang-format-diff` on copyright bytes (`0xA9`).
- **Protect develop review policy (docs + settings alignment)** — Document and template the live Protect develop gate: ≥1 GitHub APPROVED review, dismiss-stale reviews on push, `require_last_push_approval` off, required thread resolution, signed commits, force-push block, CodeQL+Gitleaks code scanning, no GitHub Code Quality ruleset rule. Remove Dependabot `gh pr review --approve`. PR Gate remains CI-only (not a review substitute). *(historical: superseded by current governance)*
- **CI lightweight job timeouts (#97)** — Add explicit `timeout-minutes` to Code Quality Format/Docs/Remote JS jobs (10–15m), Static Analysis (120m), Version Management (20m), and Copilot setup (60m). Required check names unchanged.
- **Agent autonomy policy** — `AGENTS.md` keeps `stage:live-test`, Ready, and squash merge maintainer-only. Correction agents inspect, correct, test, commit/push the feature branch, reply, and resolve treated threads; they never approve, dismiss blocking reviews, merge, or enable auto-merge. Development PR back-pressure is a soft target of 5 (see the back-pressure bullet). See `docs/10_dev/devsecops-envy.md`.
- **Local ci-verify -Full** — Builds and runs EnvyTests Win32; fails if expected binaries are missing; requires `clang-format` on PATH (ci-fast may still warn-only when absent).
- **CodeRabbit** — Skip draft PRs (`drafts: false`); path instructions cover `Remote/**` assets and `Envy/*Remote*` C++ surface.
- **Renovate** — `enabledManagers` is `github-actions` only (no unused `regex` manager). Root config migrated from `renovate.json5` to `renovate.json` with `"forkProcessing": "enabled"` so Mend Renovate can process this fork (API pre-check only reads the default `renovate.json` filename).
- **Gnutella `{deflate}` QueryHit vs G1Packet sizing (#119)** — Documented as intentional Shareaza heritage, not a bug: `CQueryHit::ReadXML` uses `nSize - 10` because fixed `nXMLSize` includes a trailing NUL; `CG1Packet::ReadXML` uses `len - 9` because length stops before HIT_SEP/NUL. No wire-behavior change. Deduped `KNOWN_INCONSISTENCIES.md` entry.

### Security
- **Interop harness cleanup roots (`python:S5443`, #160)** — Isolation refuses POSIX/Windows temp roots by path components instead of constructing world-writable `Path("/tmp")` / `Path("/var/tmp")` literals. Owned scratch under the process temp directory remains deletable; the temp root itself is never removed.
- **SonarCloud New Code gate findings (#234)** — Move CodeQL / Code Quality workflow `permissions` from workflow level to each job (`githubactions:S8233`/`S8264`). Align `Remote/head-modern.html` CSP with `security-config.js` (drop `script-src 'unsafe-inline'`); move downloads page script/style to external assets and replace `onclick` handlers with `data-action` delegation.
- **NMDC `$ADCGET`/`$ADCSND` numeric parse (#81)** — Strict length-aware unsigned decimal offset/length (`DcAdcGetValidate.h`); embedded NULs rejected. `$ADCGET` length may be exactly `-1` (until-EOF); `$ADCSND` forbids `-1` and all-digits `2^64-1` (sentinel reserved). Fixed-length replies must match the request (no silent `min()` truncate in `CDownloadTransferDC::OnDownload`). EnvyTests smoke coverage.
- **Remote Stored XSS HTML escape (#76)** — `CRemote::Add()` HTML-entity-encodes substitution values via `Escape()` before `<% =key %>` output; peer-controlled filenames/nicks/agents/addresses are no longer injected raw. `AddRaw` reserved for trusted markup (filter `checked` attrs, pre-escaped schema `<option>` lists); `AddText` skin strings unchanged. `RemoteHtmlEscape.h` + EnvyTests smoke coverage.
- **Remote password PBKDF2-SHA256 (#79)** — New Remote passwords use `BCryptDeriveKeyPBKDF2` (HMAC-SHA256, 100k iterations, 16-byte salt, 32-byte DK) stored as `pbkdf2-sha256:<iters>:<saltB64>:<dkB64>`. Successful login migrates legacy 40-hex SHA1 (UTF-16LE heritage) and intermediate `sha256-salted:` hashes; Settings UI hashes on Apply/OK. Policy helpers in `RemotePasswordPolicy.h` with EnvyTests smoke coverage.
- **GitHub Actions SHA pinning (#96)** — External actions in `.github/workflows/` and `.github/actions/` are pinned to full commit SHAs (with `# vN` comments). Upgrade process documented in `docs/10_dev/agents-and-automation.md`.
- **Crash reports stay local (#90)** — No automatic dump upload, no BugTrap registry export, no crash-time networking. Crashpad minidumps can still contain private memory fragments; sharing a dump is opt-in.

### Removed
- **BugTrap** — Vendored `Services/BugTrap` tree, prebuilt `BugTrapU` libraries, bundled `dbghelp.dll` / `dbghelp.64.dll`, `Envy/PreBuild.cmd` copies, installer copy into `{sys}`, and Debug-only `BT_*` integration.

### Fixed
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- **ED2K compressed upload parts (#87)** — `CUploadTransferED2K::DispatchNextChunk()` can emit `ED2K_C2C_COMPRESSEDPART` / `COMPRESSEDPART_I64` when the peer negotiated compression version 1 (`m_bEmDeflate`), using zlib `compress2` with eMule/aMule layout (`<hash><start><compressed-total><data>`) and benefit fallback to `SENDINGPART` when compression fails or does not shrink. Logical upload position advances by uncompressed source bytes. Helpers/tests: `Ed2kCompressedUpload.h`, `tests/test_ed2k_compressed_upload_smoke.cpp`. Does **not** claim full ED2K/eMule compatibility; live interop remains `#160`. Remaining #87: AICH C2C, callbacks/LowID, multipacket, SecureIdent.
- **Kad2 source SEARCH_RES → ED2K downloads (#86 slice)** — Inbound `KADEMLIA2_SEARCH_RES` parsed in eMule/aMule form; outstanding search context rejects unsolicited/stale/mismatched/keyword responses; valid HighID sources (types 1/4) go through `AddSourceED2K` / existing acceptance checks. Buddy/callback types 3/5/6 not delivered. Helpers: `Envy/KadSearchResDelivery.h`; EnvyTests: `test_kad_search_res_delivery_smoke.cpp`. Kad capability nibble stays 0; Kad2 status remains partial/unverified.
- **Debug assert `NeighboursBase.cpp` / `Network.m_pSection` after #224** — NMDC code-page lookups called `Neighbours.Get` without holding `Network.m_pSection` (`CDCClient` ctor, `ChatSessionDcCodePage`, `CLocalSearch::AddHitDC`). Debug builds asserted; Release raced the neighbour map. HostCache-first resolution retained; Neighbours fallback now locks (or copies from the live hub under an existing Network lock / `CDCNeighbour`).
- **Remote downloads select-all label (#234)** — Associate a `<label for="select-all">` with the downloads table header checkbox (`Web:InputWithoutLabelCheck`).
- **NMDC file-list browse never loaded** — `CHostBrowser::OnNewFile` only handled DC file lists (`Files of <nick>.xml.bz2`); the previous inverted guard skipped every DC+nick download. `Browse(PROTOCOL_DC)` now sets `hbsConnecting`. File-list `$ADCGET` uses the protocol name `files.xml.bz2` instead of the library display name. Completion matches hub+port (download name includes `ip_port`, plus `dchub://` URL when present) so two hubs with the same nick do not steal each other’s lists. Private-chat Browse is enabled for a selected hub user (G1/G2 browse path unchanged). FileListing folders populate the Browse Host tree; hit names keep the directory path. The tree is built from the owned hit chain **before** `OnQueryHits` / `CNetwork` take ownership (avoids use-after-free). Empty directories still appear; directories count toward the 100k node cap; failed parses discard partial entries.
- **System/Network log top-aligned (`CTextCtrl`)** — Short journals paint from the top of the pane (empty space below); conventional scroll with follow-bottom only when already at the end; `HitTest`/keyboard navigation aligned with the new geometry.
- **G2 SGP UDP reassembly / inflate zip-bomb (#81)** — Cap SGP fragment count (64) on receive and send; enforce cumulative reassembled size in `CDatagramIn::Add` (before buffer copy) and `ToG2Packet` using `min(Settings.Gnutella.MaximumPacket, 256 KiB)` / `G2SgpEffectiveByteCap`; Inflate capped to the same budget so UDP G2 cannot bypass TCP `MaximumPacket`.
- **BitTorrent SourcesWanted re-enforced for PEX/source-exchange/tracker (#81)** — Stop `AddSourceBT` / tracker `m_pSources` growth once effective count reaches `Settings.Downloads.SourcesWanted` in `OnUtPex`, `OnSourceResponse`, tracker HTTP peer parse (dict + compact), HTTP apply, and UDP announce (`BtSourcesWantedAllowsMore`). Compact IPv4 peer blobs validated with `BtCompactPeerListBytesOk` in `OnUtPex`, HTTP compact peers, and UDP announce remaining payload (malformed non-multiple-of-6 rejected).
- **VersionChecker HTTP response size cap (#81/#82)** — Automatic version-check downloads use `LimitContentLength(VERSION_CHECK_HTTP_RESPONSE_MAX)` (64 KiB) and fail-closed oversize bodies (`VersionCheckerHttpResponseOk`) before key=value parse. Exact-limit (64 KiB) bodies are accepted; oversize still discard.
- **Update Servers HTTP response size cap (#81/#82)** — `CUpdateServersDlg` calls `LimitContentLength(UPDATE_SERVERS_HTTP_RESPONSE_MAX)` (32 MiB) and fail-closes oversize bodies before MET/hublist import (`UpdateServersHttpResponseOk`). `CHttpRequest` probes one byte past the cap via `InternetReadFile` so exact-limit bodies are kept while truncated/oversize responses are discarded (avoids the chunked `QueryDataAvailable`/`nMore==0` race).
- **Kademlia store-entry tag length cap (#81)** — Reject per-tag values above 4 KiB (`KadStoreTagLengthOk` / `KAD_STORE_TAG_MAX`) in `CKademlia::ReadEntryTags` before `vector` allocate (WORD×32 tags DoS).
- **BitTorrent MSE len(IA) fail-closed (#81)** — Cap responder `len(IA)` at 96 (`BtMseIaLengthOk` / `BT_MSE_IA_MAX`); stall in new `MSE_AWAITING_IA` until the full IA is buffered before `crypto_select` (avoids RC4 desync from partial IA / oversized WORD lengths).
- **BitTorrent MSE Pad_C/Pad_D receive cap (#81)** — Reject peer-advertised pad lengths above 512 (`BtMsePadLengthOk` / `BT_MSE_PAD_MAX` = `MSE_PAD_MAX_LEN`) before `new[]`/wait-for-bytes during MSE handshake. Decode/encode pad length fields as big-endian (`BtMseBeWordFromWire` / `BtMseBeWordToWire`) per Vuze MSE.
- **Uploads/Downloads bandwidth DWORD truncation** — Applying a parsed volume no longer `static_cast`s a `QWORD` to `DWORD` (wrap). Helpers clamp to `DWORD` max; unlimited tokens store `0`.
- **BitTorrent TCP length-prefix absolute cap (#81)** — Reject single-message length-prefixes above 16 MiB (`BtPacketLengthOk` / `BT_PACKET_LENGTH_MAX`) in `CBTPacket::ReadBuffer`; clear the input and close the peer (`IDS_PROTOCOL_TOO_LARGE`) instead of stalling forever on a multi-GB wait.
- **G2 HIT_WRAP embedded G1 length fail-closed (#81)** — Reject negative wrapped `GNUTELLAPACKET::m_nLength` in `CG1Packet::New` before `(DWORD)` cast/`Write`; `SeekToWrapped` uses `G1WrappedPayloadFits` (negative + 256 KiB ceiling + remaining) for G2→G1 conversion. Null-check `New()` at wrap call sites so HostBrowser `MaximumPacket*8` browse is unchanged.
- **NMDC hub text encoding (#224)** — Decode/encode NMDC user text with an explicit hub code page (`Settings.DC.CodePage`, `0` = Windows ACP; invalid pages fall back to ACP) via `DcNmdcText.h`, instead of forcing `UTF8Decode` on RX and `CP_ACP`/`WriteStringUTF8` on TX. Optional per-host override is persisted on `CHostCacheHost::m_nCodePage` (ser v2; `SetNmdcCodePage`; favorites UI later). Covers hub chat/`$HubName`/`$MyINFO`/`$To`/`$SR`. Stops mojibake and HostCache pollution. ADC/ADCS unchanged. EnvyTests cover ASCII, UTF-8, CP1251, wrong-charset contrast, length, invalid page fallback, and fixtures.
- **G2 compound sub-packet length overflow (#81)** — Order-safe `G2SubpacketPayloadFits` / `G2FrameLengthFits` replace `remaining < body + prefix` checks in `CG2Packet::ReadPacket`, `SkipCompound`, and `ReadBuffer` (defense-in-depth; G2 wire lengths are 2-bit capped below DWORD max).
- **G1 QueryHit QHD XML size fail-closed (#81)** — Reject attacker-controlled `nXMLSize` that does not leave room for the trailing GUID (`G1QueryHitXmlFits`, including zero-length XML); replace soft clamp to 0 and the commented-out size check with `AfxThrowUserException` (same class as invalid public-size).
- **ED2K ViewSharedDirAnswer directory-name sync (#81)** — Consume the WORD-prefixed directory name before reading file `count` in browse-host answers (`Ed2kEdStringHeaderOk` / `Ed2kEdStringPayloadOk`); previously the skip was commented out so `count` was read from the string length/bytes. `OnViewSharedDir` / `OnAskSharedDirsAnswer` reject oversize directory strings fail-closed; `OnServerMessage` applies the same wire checks plus a 5000-byte MOTD cap (`Ed2kServerMessageLengthOk`) before `ReadEDString`.
- **ED2K chat MESSAGE length predicate (#81)** — `CEDClient::OnChatMessage` uses shared `Ed2kChatMessageLengthOk` (non-empty, `<= ED2K_CHAT_MESSAGE_MAX` / `ED2K_MESSAGE_MAX`, exact remaining fit) with EnvyTests smoke coverage; `CChatSession::SendPrivateMessage` clamps outgoing text by encoded wire byte length so UTF-8 frames stay within the max.
- **ED2K unknown-tag skip fail-closed (#81)** — Speculative STRING skip for unknown tag types no longer falls back to an INT-sized seek when a plausible length exceeds remaining (`Ed2kUnknownTagStringSkipOk`); INT fallback kept only when length exceeds the STRING heuristic max.
- **ED2K ReadEDString / ReadLongEDString fail-closed (#81)** — After reading the length prefix, reject when claimed payload exceeds remaining (`Ed2kEdStringPayloadOk` / `Ed2kLongEdStringPayloadOk`) via `AfxThrowUserException` instead of silently clamping in `ReadString*`.
- **GGEP DEFLATE inflate output cap (#81)** — `CGGEPItem::Inflate` passes `GGEP_INFLATE_MAX` (256 KiB) into `CZLib::Decompress` and rejects empty/oversize output (`GgepInflateOutputOk`); closes uncapped zip-bomb path on inbound GGEP items.
- **CBuffer InflateStreamTo default output cap (#81)** — `InflateStreamTo` with `nMaxOutput=0` now applies `CBUFFER_INFLATE_STREAM_MAX` (32 MiB, alias of `CBUFFER_INFLATE_MAX`) so Neighbour G1/G2 `Content-Encoding: deflate` cannot grow an unbounded uncompressed backlog; HostBrowser may still pass an explicit smaller cap. G1/G2/ED/DC `OnRead` propagates inflate failure so the connection closes instead of buffering forever.
- **CBuffer Inflate/Ungzip default output cap (#81)** — `CBuffer::Inflate`/`Ungzip` with `nMaxOutput=0` now apply `CBUFFER_INFLATE_MAX` (32 MiB) via `Decompress2` / growth clamps (`CBufferInflateOutputOk`); closes unlimited datagram/legacy CBuffer inflate zip-bombs while callers may still pass an explicit smaller cap.
- **ED2K FileComment length guards (#81)** — `OnFileComment` requires rating+length header and rejects claimed comment lengths above `ED2K_FILE_COMMENT_MAX` or remaining payload (`Ed2kFileCommentHeaderFits` / `Ed2kFileCommentLengthOk`); fail-closed instead of clamp-then-truncate.
- **ED2K wire TAG_BLOB absolute size cap (#82)** — Packet-path `CEDTag::Read` now uses `Ed2kTagBlobLengthOk` (4 MiB + remaining) instead of remaining-only checks, matching `.met` / collection TAG_BLOB policy. Oversized peer-advertised blobs fail closed.
- **BitTorrent source-response double-free (#92)** — `CDownloadTransferBT::OnSourceResponse` no longer `delete`s `pPacket->m_pNode` when `peers` is missing/non-list (packet owns the node); null-guard `pRoot`/`pPeers` and nested peer nodes before `IsType`.
- **ED2K wire TAG_UINT64 remaining check (#81)** — `CEDTag::Read` required only 1 remaining byte before `ReadInt64()`; now requires 8 via `Ed2kTagUint64RemainingOk` (+ EnvyTests). Fail-closed on truncated packets.
- **Network async job queue cap (#81)** — `CNetwork::m_oJobs` drops oldest owned search/hit trees once depth reaches 2048 (`NETWORK_JOB_QUEUE_MAX` / `EnqueueJob`); `RunJobs` requeues retained jobs through `EnqueueJob` so the cap holds across the unlock window.
- **Chat session undelivered queue cap (#81)** — `CChatSession::m_pMessages` drops oldest owned payloads once depth reaches 1024 (`CHAT_SESSION_QUEUE_MAX` / `EnqueueMessage`) so wire chat floods cannot grow forever when no private window drains the queue.
- **NMDC hub user-list size cap (#81)** — `CDCNeighbour::OnUserInfo` refuses new `$MyINFO` nick inserts once `m_oUsers` reaches 20,000 (`DcHubUserCountOk` / `DC_HUB_USERS_MAX`); existing nick updates still apply.
- **CBuffer UnBZip output cap (#81)** — DC hublist / file-listing loaders use `LoadFromBZipFile` with a 32 MiB decompress cap (`CBUFFER_UNBZIP_MAX`); oversized compressed input is rejected (`CBufferUnBZipInputOk`). Legacy `UnBZip()` with `nMaxOutput=0` remains unlimited.
- **G1 `{deflate}` XML inflate cap (#81)** — `CQueryHit::ReadXML` and `CG1Packet::ReadXML` pass `G1_DEFLATE_XML_INFLATE_MAX` (256 KiB) into `CZLib::Decompress` and reject empty/oversize output (`G1DeflateXmlInflateOk`); closes uncapped zip-bomb path on QueryHit metadata.
- **Gnutella QHT patch inflate zip-bomb (#81)** — Cap compressed QRP/QHT fragment accumulation (`QhtPatchCompressedBudgetOk`) and pass expected patch size to `CBuffer::Inflate(nMaxOutput)` before applying patches (`QueryHashTable::OnPatch`).
- **ED2K packed-protocol inflate cap (#81)** — `CEDPacket::Inflate` defaults to / enforces `ED2K_PACKED_INFLATE_MAX` (512 KiB) for EMULE/KAD/REVCONNECT packed packets (`Ed2kPackedInflateOk`); closes unlimited C2C/UDP inflate zip-bomb while matching the prior server-path guard.
- **BitTorrent tracker HTTP response size cap (#81/#82)** — `CBTTrackerRequest::ProcessHTTP` calls `LimitContentLength(BT_TRACKER_HTTP_RESPONSE_MAX)` (32 MiB) so `CHttpRequest::OnRun` stops buffering hostile announce/scrape bodies before inflate; post-inflate length checked with `BtTrackerHttpResponseOk`.
- **Discovery GWC/server-list HTTP response size cap (#81/#82)** — `SendWebCacheRequest` calls `LimitContentLength(DISCOVERY_HTTP_RESPONSE_MAX)` (32 MiB) and fail-closes on oversize buffered bodies (`DiscoveryHttpResponseOk`); `RunServerList` null/size-checks before `CMemFile` copy.
- **Browse Host HTTP body size cap (#81/#82)** — Reject peer `Content-Length` above 32 MiB (`HostBrowserHttpBodyOk`); require a full decimal Content-Length (no trailing junk); cap reassembled/deflated browse buffers before allocation (`HostBrowserHttpBufferOk` + `InflateStreamTo` nMaxOutput); close-delimited totals use `m_nReceived + GetInputLength()`.
- **ED2K COMPRESSEDPART stream inflate cap (#81)** — `OnCompressedPart` / `OnCompressedPart64` share `AcceptCompressedPartChunk` (`Ed2kCompressedPartInflateBudget` / `Ed2kCompressedPartInflateOk`) to reject uncompressed output past one ED2K part or remaining file size (zero at EOF) before `SubmitData`; `CEDClient::OnPacket` propagates rejection so `OnRead` stops. Closes streaming zip-bomb into the download (`ED2K_C2C_COMPRESSEDPART` / `ED2K_C2C_COMPRESSEDPART_I64`).
- **G1 packet length overflow-safe framing (#81)** — `CG1Neighbour::ProcessPackets` validates signed payload length with `G1PacketTotalLengthOk` before `header + payload` (reject negative / wrap vs `MaximumPacket`).
- **Gnutella1 UDP packet length overflow (#81)** — Reject negative / oversize `GNUTELLAPACKET::m_nLength` on the UDP datagram path (`G1PacketTotalLengthOk` / `G1PacketTotalLength`) before `sizeof + signed` equality and `CG1Packet::New` Write (closes the TCP-only gap of the G1 neighbour framing fix).
- **ED2K .met TAG_STRING / tag-key length bounds (#81/#82)** — `CEDTag::Read(CFile*)` rejects WORD-prefixed key and `ED2K_TAG_STRING` values that exceed remaining file bytes before allocate/Read (`Ed2kTagStringLengthOk`); empty strings accepted. Complements existing TAG_BLOB / hashset caps.
- **Windows Firewall WFAS migration (#166)** — Replace XP-era `INetFwMgr`/`INetFwPolicy`/`INetFwProfile` with `INetFwPolicy2` rules API. Application exceptions use inbound allow rules on Domain+Private+Public; UPnP uses WFAS rule-group enable; `AreExceptionsAllowed` consults BlockAllInboundTraffic on every currently active profile. `FirewallWfasPolicy.h` + EnvyTests smoke coverage.
- **BitTorrent BEP-9 ut_metadata size cap (#82)** — Reject peer-advertised `metadata_size`/`total_size` above 32 MiB and refuse `LoadInfoPiece` for oversize info dicts (`BtUtMetadataSizeOk`).
- **Cooperative thread close without TerminateThread (#92)** — `CEnvyThread::CloseThread` abandons timed-out threads after cancel instead of `TerminateThread` (`EnvyThreadPolicy.h`); `throw()` → `noexcept`.
- **ED2K EDClients-before-Transfers lock order (#92)** — Canonical order `EDClients.m_pSection` then `Transfers.m_pSection` (`Ed2kLockOrder.h`); fix inverted acquisition in `CEDClients::OnAccept`, UDP C2C path, and `CHostBrowser::Browse`.
- **ED2K CryptLayer Hello honesty (#121)** — Confirm MiscOptions2 CryptLayer bits mean TCP protocol obfuscation (not PUBLICKEY packet RC4). Keep Hello advertise at 0; peer Hello crypt bits no longer start PUBLICKEY; peer flag defaults FALSE until Hello. Policy helpers + smoke tests extended.
- **NMDC HubName/HubTopic/chat length underflow (#81)** — Reject frames that would underflow `nLength - prefix - 1` (trailing `|`) before `UTF8Decode` in chat and hub-name parsers; shared predicates in `DcPacketLengthValidate.h` with EnvyTests smoke coverage.
- **Portable ZIP incomplete runtime tree** — Release packaging no longer flattens only `*.exe`/`*.dll` into the ZIP. `stage-portable.ps1` mirrors the Inno install layout (`Envy.exe` + service DLLs at root, plugins under `Plugins\`, plus `Data\`/`Schemas\`/`Skins\`/`Skins\Languages\`/`Templates\`/`Remote\`). `verify-artifacts.ps1` fails flattened or resource-less ZIPs.
- **Release draft asset upload race** — Tag-push packaging no longer uses parallel `softprops/action-gh-release` uploads against a freshly created draft (that failed mid-upload with `Error saving asset` and left Preview 1 missing x64 assets). Uploads now run sequentially via GitHub API against the concrete `release_id`, skip/replace assets idempotently, verify the final remote set, and never auto-publish (`draft` stays true). `workflow_dispatch` remains a dry-run unless `repair_release_id` is set explicitly.

## [4.1.0] - 2026-01-11

### Added
- **Release preparation** (`4012a89`) - Version 4.1.0 milestone preparation
- **Development infrastructure** - Enhanced tooling and build systems
- **C++17 migration** - Modern language standard adoption
- **Security enhancements** - Vulnerability fixes and protection measures
- **Protocol improvements** - Enhanced ED2K and Kademlia support
- **Build automation** - CI/CD pipeline improvements
- **Code quality tools** - Static analysis and formatting utilities

### Changed
- **Build system modernization** - CMake integration and multi-platform support
- **Code organization** - Improved structure and maintainability
- **Development workflows** - Enhanced processes and automation

## [4.0] - 2025-01-01

### Added
- Major architectural improvements and modernization
- Enhanced protocol support and network capabilities
- Improved user interface and user experience
- Extended plugin system capabilities
- Better error handling and stability improvements

### Changed
- Significant codebase refactoring and cleanup
- Updated dependency management
- Improved performance and memory usage
- Enhanced security features

### Fixed
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- Various stability and performance issues
- Protocol compatibility improvements
- User interface bugs and inconsistencies

## [3.0] - 2024-01-01

### Added
- Advanced BitTorrent support and optimizations
- Enhanced Kademlia DHT implementation
- Improved search and discovery mechanisms
- Extended media library capabilities
- Better internationalization support

### Changed
- Major user interface redesign and improvements
- Enhanced network protocol handling
- Improved file management and organization
- Better resource utilization

### Fixed
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- Memory leaks and resource management issues
- Network connectivity problems
- File sharing and transfer reliability issues

## [2.0] - 2023-01-01

### Added
- Multi-protocol support (Gnutella2, eDonkey2000, BitTorrent)
- Advanced chat and community features
- Plugin architecture for extensibility
- Improved download management and queuing
- Enhanced security and privacy features

### Changed
- Complete user interface overhaul
- Improved network performance and stability
- Better file organization and management
- Enhanced search capabilities

### Fixed
- **BitTorrent piece-hash metadata consistency (#118)** — `CBTInfo::Clear()` now deletes owned files without an allocation-dependent staging vector; serialization rejects inconsistent or oversized piece-hash metadata instead of silently writing an empty vector. EnvyTests cover the clear-state, count-cap, mismatch, and reuse invariants.
- **ED2K Source Exchange v2 wire format (#331)** — `REQUESTSOURCES2` (0x83) and `ANSWERSOURCES2` (0x84) now follow eMule/aMule framing (`<Version><Options><Hash>` requests; `<Version><Hash><Count>` answers with v1–v4 record sizes). Removes the incorrect file-size request encoding that caused live `0x83` malformed-packet rejects. **Breaking:** older Envy builds that used the non-standard hash+file-size `0x83` request layout no longer interop with this wire format on SourceEx2; SourceEx1 (0x81/0x82) fallback is unchanged. Golden vectors in `tests/test_ed2k_sourceex2_golden.cpp`. Live eMule/aMule interop remains unverified (#160).
- Numerous stability and compatibility issues
- Network protocol bugs
- User interface responsiveness problems

## [1.0.0.0] - 2022-01-01

### Added
- Initial release of Envy P2P client
- Basic file sharing functionality across multiple networks
- Support for Gnutella, eDonkey, and BitTorrent protocols
- User interface with tabbed browsing and search
- Basic download management and queuing system
- Network connectivity and peer discovery
- Simple chat functionality
- Basic media library and file organization
- Plugin system foundation
- Configuration and settings management
- Basic security features and IP filtering

### Changed
- Project structure and organization
- Codebase refactoring from PeerProject foundation
- Build system improvements
- Documentation and licensing updates

## [1.0.0.0.Pre] - 2021-12-01

### Added
- Pre-release development and testing
- Core P2P functionality implementation
- Network protocol integration
- Basic user interface components
- Foundation for plugin system

### Changed
- Initial project setup and configuration
- Codebase preparation for public release

## [0.x] - 2021-01-01 to 2021-11-30

### Added
- Project foundation as fork of PeerProject
- Initial codebase migration and cleanup
- Basic build system setup
- Core networking infrastructure
- Protocol handler implementations
- User interface framework
- Basic file sharing capabilities

### Changed
- Codebase modernization and refactoring
- Project rebranding from PeerProject to Envy
- Build system improvements
- Documentation updates

---

## Project History

### Origins (Pre-2021)
Envy originated as a fork of **PeerProject**, which was itself derived from the **Shareaza** P2P client. The project represents a continuation of the open-source P2P file sharing tradition with a focus on modernizing the codebase and improving user experience.

### Development Evolution
The project has undergone significant evolution based on git commit history:

- **2016**: Initial development by SkinVista - Project foundation and basic P2P functionality (r1-r42)
- **2017**: Continued development with feature additions and bug fixes (r16-r20)
- **2018**: Additional releases and maintenance (r21)
- **2020**: Major version milestone (4.0) - Significant architectural improvements (r34-r42)
- **2026**: Modernization phase - Complete codebase transformation:
  - **January 2026**: Major development push with Kademlia DHT, AI integration, and security fixes
  - **4.1.0 Release**: Enhanced development infrastructure and tooling
  - **Current development**: Ongoing improvements and new features

### Technical Improvements (2026)
- **Kademlia DHT implementation**: Complete distributed hash table with routing and peer discovery
- **C++17 modernization**: Rule of Five implementation, smart pointers, modern language features
- **Security enhancements**: CVE-2025-8088 vulnerability fix, improved file extraction validation
- **Build system evolution**: CMake support, multi-platform CI/CD (x64/Win32), automated testing
- **Development tooling**: Clang/CppCheck integration, PowerShell automation scripts, AI-assisted development
- **Protocol enhancements**: Source Exchange v2, MultiPacketExt2, KADEMLIA2 support
- **Code quality**: Static analysis, automated formatting, comprehensive testing infrastructure
- **Dependency management**: SQLite/zlib updates, enhanced version checking and reporting

### Protocol Support Evolution
- **2016-2020**: Core P2P protocols (Gnutella, eDonkey, BitTorrent) - Basic multi-network support
- **2026 Q1**: Major protocol enhancements:
  - **ED2K/eDonkey2000**: FileIdentifier, HashsetRequest2, MultiPacket Ext2, AICH support, CryptLayer/SecureID preparation
  - **Kademlia DHT**: Complete implementation with routing, node management, and KADEMLIA2 protocol
  - **Source Exchange**: Version 2 implementation with enhanced peer discovery
  - **Security**: CryptLayer negotiation preparation, SecureID support preparation, enhanced authentication
  - **Code Quality**: Improved packet reading, case-insensitive search filtering
- **Current**: Advanced multi-protocol P2P client with modern DHT, enhanced security preparation, and improved user experience

## Detailed Commit History (2020-2026)

### 2026 Development Phase
- `XXXXXXX` (2026-01-15) - Code quality improvements: packet reading robustness, case-insensitive search
- `XXXXXXX` (2026-01-15) - ED2K CryptLayer and SecureID framework preparation
- `e3ae644` (2026-01-15) - FileIdentifier class and hashset request handling
- `c9b5966` (2026-01-15) - Complete Kademlia DHT implementation and integration
- `63eb0e7` (2026-01-15) - README.md CI/CD capabilities documentation
- `b61e4b0` (2026-01-15) - Comprehensive CI/CD automation and GitHub configuration
- `5779b77` (2026-01-14) - zlib compression and decompression test implementation
- `35b37a1` (2026-01-14) - .gitignore updates and code refactoring
- `e92d930` (2026-01-13) - CVE-2025-8088 security merge
- `eaab22e` (2026-01-13) - CVE-2025-8088 directory traversal protection in UnRAR
- `ad3bd40` (2026-01-13) - AICH/CryptoProvider headers and development agents documentation
- `5314409` (2026-01-12) - SQLite and zlib component updates
- `3de8ce2` (2026-01-12) - GitHub Actions multi-platform support and Rule of Five implementation
- `e870aa8` (2026-01-12) - Source Exchange v2 and MultiPacketExt2 support
- `c82e405` (2026-01-12) - KADEMLIA2 protocol support and validation
- `a818143` (2026-01-12) - Development tools configuration (Clang, CMake, CppCheck)
- `4012a89` (2026-01-11) - Release 4.1.0 preparation

### 2020 Legacy Phase (Eric)
- `43787f3` (2020-03-18) - Release r42
- `cd4c78a` (2020-02-13) - Release r41
- `510a28f` (2020-01-21) - Release r40 (Version 4.0)
- `c67bd17` (2020-01-21) - Release r39
- `29923e1` (2020-01-20) - Release r38
- `8f6af14` (2020-01-18) - Release r37
- `dfad434` (2020-01-04) - Release r36
- `540cd2d` (2020-01-04) - Release r35
- `3aa4e6e` (2020-01-02) - Release r34

### 2016-2018 Development Phase (SkinVista)
- Multiple releases from r1 to r21 focusing on core P2P functionality, UI improvements, and bug fixes
- `091f444` (2016-04-02) - Initial commit establishing project foundation

For the complete git history with file changes and full commit details, please refer to the git repository.
