# DEVELOPMENT PLAN (LIVING)

- **Last Updated:** 2026-10-11

- **Changelog Entry:** 2026-10-11 - #405: constrain package destinations with strict manifest/ZIP path checks, bounded construction, pinned non-reparse ancestors and complete-file staged replacement. Production-backed EnvyTests cover package types, malformed input, overwrite policy and alias escapes; the focused project supports AddressSanitizer. Essential #406 ownership and manifest-buffer fixes are included; package-wide expansion/time budgets remain open. Validation commands and limitations: `10_dev/unpacker-validation.md`.

- **Governance observation:** 2026-10-11 - The live Protect develop ruleset (16457466) reports `required_approving_review_count: 0`, differing from AGENTS.md and the documented one-review policy. This security patch does not change governance or rely on that reduction; maintainer review, Draft promotion and manual squash merge remain required.

- **Changelog Entry:** 2026-10-11 - #393: maintainer reports successful validation of ac29717 (EnvyTests 735/735, PeerXmlTests 18/18 and fixture HTTP 21/21); Release x64/Win32 CI builds passed. Correct the validation guide: loopback sources are rejected by the application's reserved-address policy, so fixture self-tests are not application integration evidence. Separate native parser coverage from admitted-peer application tests and document THEX/Tiger/legacy MD4 DIME type identifiers without changing protocol bytes or network protections. Historical Gnutella issues #453/#319 remain outside this PR.

- **Changelog Entry:** 2026-10-11 - #393/#391: distinguish valid THEX descriptor state from an empty identity; retain empty DIME ID compatibility and guard missing/empty URI fallback by an existing TTH. Validate complete binary rows and both historic depth conventions, strict numeric metadata/Content-Length and DIME message framing. Native tests now execute production SetTigerTree with HashLib. Reference source comparisons, client-specific G1/G2 steps, synthetic fixtures and manual Visual Studio instructions are in `docs/10_dev/peer-xml-validation.md`. Full application builds/live interoperability remain maintainer work; local corrections must not be pushed before those results.

- **Changelog Entry:** 2026-10-11 - #393: add isolated native tests compiling the unchanged production THEX/DIME methods; cover every two-read boundary, descriptor identity/reset, truncation, malformed framing, early size rejection and invalid-descriptor draining. Full transfer/profile/live interoperability remains pending.

- **Changelog Entry:** 2026-10-11 — #393/#391: synchronize with current develop, drain HTTP socket data across capped header/body phases before close handling, reject zero-length file responses before mutating download size, and add native MFC XML plus loopback receive regressions. Preserve THEX type identifiers; document the Sonar HTTP warning as a protocol-identifier false positive. Explicit native-suite commands and controlled HTTP/THEX/manual peer checks are in `docs/10_dev/peer-xml-validation.md`; live interoperability remains pending.

- **Changelog Entry:** 2026-10-10 — #422/#419: independently recheck every shipped bootstrap endpoint and ED2K/Kad/DC/DHT alternatives; add the AirDC++-documented Team Elite HTTPS hublist after bounded BZip2/XML and NMDC compatibility validation. Preserve three DHT routers within the existing DNS budget, reject stale/unverified Kad alternatives, and enforce blocked identities and truncated-row rejection offline. Persisted-service migration (#419), private-torrent isolation (#317), defaults (#418), Kad runtime/retry (#86/#430), UDP obfuscation (#431) and live interoperability (#160) remain separate open work.

- **Validation blocker:** 2026-10-10 — #422: Release x64 solution build succeeds; the Release Win32 solution build stops in `Envy/PreBuild.cmd` because the required vcpkg `crashpad_handler.exe` is missing. The independently generated EnvyTests executables pass 708/708 on both platforms; this does not establish a successful Win32 application build. Resolve the existing Crashpad environment/dependency prerequisite separately (#83/#353/#355); no dependency or gate changes are included here.

- **Changelog Entry:** 2026-10-06 — #372: align Kad2 SEARCH_SOURCE_REQ with maintained aMule framing (<FileHash 16><StartPosition uint16 LE><FileSize uint64 LE>), keep bounded legacy hash-only/size-only input compatibility, and use one tested encoder for the production send path so CPacket big-endian state cannot corrupt FileSize.
- **Changelog Entry:** 2026-10-07 — #327: cover the Windows MSBuild composite action in Dependabot; make `-CloneVcpkg` select the pinned local checkout even when absent; four offline selection cases pass. Reconcile with current `develop` without restoring retired classification outputs or changing live protections.

- **Changelog Entry:** 2026-10-07 - P2P reference registry refresh (#387 resumed): `REFERENCE_IMPLEMENTATIONS.md` expanded (amule-org, eMuleBB/padMule/MorphXT/heritage/BT/DC); October ecosystem audit; optional ignored clones via `tools/references/` -> `Examples/References/` (D-023). No AGENTS/governance change; no protocol code.
- **Changelog Entry:** 2026-10-01 — Finalization of #395 preserves one non-author approval, all eight live required contexts, CodeQL/Gitleaks and GitHub Code Quality (severity All). Retain the staged Draft/live-test/Ready workflow and manual maintainer transitions; squash summaries remain curated. #397 owns review outcome tracking, not merge authority.

- **Changelog Entry:** 2026-10-06 — #393: ChatSession profile ownership matches HostBrowser; move ordinary CL=0 reject after HTTP control classification; clear breadth-first THEX remainder; `ReadDIME` reuses `PeekDIME`; shared HTTP/THEX admission helpers + EnvyTests.

- **Changelog Entry:** 2026-10-06 — #393: restore historical copyright bytes on edited ISO-8859 sources; RAII restore of shared XML parse depth after failure; share G2 query METADATA budget; EnvyTests depth-restore and DWORD ConsumeChars overflow cases.

- **Changelog Entry:** 2026-10-05 — #393: sync onto `develop` after #392; `ChargeSharedPeerXmlChars`/`AdmitPeerXml*` entry helpers; THEX `XML_PEER_THEX_BODY_CAP` pre-accept reject; Metadata wrapper node funding; expanded EnvyTests peer-entry smoke (helpers only — no MFC `CXMLElement` link).


- **Changelog Entry:** 2026-10-01 — Proactive Draft stabilization: remove fixed correction-attempt stops, retain complete history and human decisions, automate supported reviewer requests and validated lifecycle transitions, and preserve privacy/naming and native review/security gates. #397 supplies the host coordinator; external app entitlement and live pilot remain explicit rollout evidence.

> **LIVING DOCUMENT** — Must be updated after every meaningful change (feature, architectural decision, scope change, blocker resolution).

- **Changelog Entry:** 2026-09-30 — #393: share one `XmlParseBudget` across G1 per-hit `ReadXML` extensions and the trailer; THEX/MetaFetch receive caps; reject keep-alive unknown THEX length; finish buffered known-length MetaFetch on drop; parse THEX headers before the 16 KiB header-phase fail-closed; `PeekDIME` rejects oversized DIME `text/xml` before buffering the body.
- **Changelog Entry:** 2026-09-30 — #393: gate G1 hit XML raw allocation and charge shared G2 METADATA character budget before `ReadString`; add EnvyTests filter entry for the peer-XML smoke test.
- **Changelog Entry:** 2026-09-30 — #393: share peer XML parse budgets across multi-fragment G1/G2 metadata and gate profile/query `ReadString` behind the 256 KiB pre-materialization cap.
- **Changelog Entry:** 2026-09-30 — PR governance simplification (D-022): keep one non-author approving review as the trust boundary against PR-controlled CI self-bypass; path-based conditional Windows builds; Sonar/advisory bots; ruleset-as-code audit; `docs/10_dev/pr-workflow.md`.
- **Changelog Entry:** 2026-09-29 — (superseded wording) earlier D-022 draft considered zero mandatory approvals; rejected after review of PR-controlled classify/build bypass risk.
- **Changelog Entry:** 2026-09-29 — #327: retire Renovate; Dependabot owns vcpkg, `Remote/tests` npm, and GitHub Actions; docs/ownership inventory aligned; Dependency Review on PRs targeting `main`/`master`/`develop`; **Repository Status** CI/merge text aligned with post-#381 (no PR Gate poller; optional branch-commit signatures).
- **Changelog Entry:** 2026-09-30 — #394: expand always-on PR CodeQL to python and GitHub Actions (`actions`); keep C# deferred in cheap Draft (live-test/Ready); document Default-setup migration and classifier comments.
- **Changelog Entry:** 2026-09-30 — #351: rename-aware encoding diffs, valid-to-invalid UTF-8 and invalid-byte mutation guards, BOM/header-safe Languages UTF-8, encoding-migration label path, and Windows/vcpkg setup docs aligned with the active x64/Win32 policy.
- **Changelog Entry:** 2026-09-28 — #351: make encoding selftests exercise mojibake/C1 detectors on non-header lines, align vcpkg build guidance with the active x64/Win32 policy, and parse changed-line metadata without dropping `++`/`--` source lines.
- **Changelog Entry:** 2026-09-28 — #367: separate declared connection capacity from explicit transfer limits, preserve user caps, share capacity presets, and add transfer-limit regression coverage.
- **Changelog Entry:** 2026-09-28 — #363: correct ED2K SourceEx2 request/answer framing, add versioned record validation and golden vectors, and document the live interop gap.
- **Changelog Entry:** 2026-09-28 — #381 simplification: remove the duplicate PR Gate poller and conclusion helpers; keep Draft → `stage:live-test` → Ready phase selection and native workflow checks. Protect develop now removes PR Gate, required signatures, duplicate gitleaks status, and required Documentation Check, with Copilot review-on-push disabled. Cursor automations remain external manual operations.
- **Changelog Entry:** 2026-09-28 — Staged validation: Draft defers Windows builds/C# analysis; manual `stage:live-test` and Ready run both Release builds, EnvyTests and provenance-tagged artifacts. One subscription-based fixer is specified with Draft 3 / Ready 2 batch limits. Rollout blockers: maintainer must disable the existing approval automation, create lifecycle labels, verify subscriptions/signing and perform runtime/Ready dogfood. See `10_dev/agents-and-automation.md`.
- **Changelog Entry:** 2026-09-26 — #341 transfer bandwidth correctness: `TransferSettingsLimits.h` Kb/s→bytes/s and upload-headroom helpers (64-bit intermediates, DWORD saturation); fix Connection page integer-order bug (`92/100→0`); align Scheduler/Downloads/Uploads/UploadQueues/Datagrams paths. Slot/queue presets (#343/#344/#345) unchanged.
- **Changelog Entry:** 2026-09-25 — #298: rebase BitTorrent keep-alive framing onto current `develop`; resolve CHANGELOG/DEVELOPMENT_PLAN with library (#301) and UI (#297) entries.
- **Changelog Entry:** 2026-09-20 — #88/#91 BitTorrent keep-alive framing: `CBTPacket::ReadBuffer` now leaves the following frame prefix intact when returning a keep-alive; EnvyTests cover keep-alive marker removal and keep-alive+choke sequencing.
- **Changelog Entry:** 2026-09-21 — UI Phase 1 (#297): merge `develop` after #301; keep both the adaptive-list changelog and the library MountCollection null-guard entry.
- **Changelog Entry:** 2026-09-21 — #300: `CAlbumFolder::MountCollection` guards null `GetNextFolder` results; `m_pFolders` rejects null inserts; album-root mount is skipped when `m_pAlbumRoot` is null. EnvyTests cannot construct `CAlbumFolder` (MFC/`Library.m_pSection`); policy smoke `test_albumfolder_mountcollection_smoke.cpp`.
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#297) Copilot: load ListStates/sticky only once; later OnSkinChange keeps in-session sticky.
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#297) Copilot: suppress sticky during SizeLists sizer resize; document sticky clamp to [min,max].
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#297) Copilot: sticky mask `ListStates *.Sticky` + normalize non-sticky before SaveList; accept header hwndFrom for HDN; architecture doc status PARTIAL.
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#297) Copilot: AdaptiveScalePx saturates via 64-bit product; HDN_ENDTRACK sticky prefers NMHEADER cxy; Bounded docs (no leftover grow).
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#297) Copilot: flex remainder redistributes until exhausted; suppress sticky during LoadList; smoke test asserts fixed/bounded sum 668 + capped-peer remainder case.
- **Changelog Entry:** 2026-09-20 — UI Phase 1 (#295): adaptive column allocator + Neighbors/Host Cache DPI-aware fill (`AdaptiveListColumns.h` / `AdaptiveListLayout.*`); EnvyTests; no protocol change. Architecture doc landed via #296.
- **Changelog Entry:** 2026-09-20 — UI modernization plan (#296): foundation table distinguishes pointer floor `SCALE(24)` from ~40 EP touch targets (remove “touch-friendly” mislabel).
- **Changelog Entry:** 2026-09-20 — UI modernization plan (#296): align preflight audit SHA with header (`ccb4d20`); clarify SCALE(24) is not a Windows touch target (~40 EP).
- **Changelog Entry:** 2026-09-20 — UI modernization plan (#296): address Copilot review — historical soft-PR-target wording; Neighbors OnSize sizer+SizeListAndBar; PROTOCOL_NULL = G2 Horizon; DisplayScaling 100–200; ListStates sticky/adaptive persistence caveat.
- **Changelog Entry:** 2026-09-20 — Desktop UI modernization architecture (#295): audit on `develop` `d3095e9`; staged PR sequence (docs → adaptive Network lists → shell → Transfers → Home → Library → Settings → Media → Chat → assets); reusable adaptive-column foundation; resource-load failures classified. Canonical: `docs/20_arch/UI_MODERNIZATION.md`. No production UI code in Phase 0.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 9: early pcap death fail-closed; SEARCH_RES ambiguity fail-closed; TCP reassembly by unidirectional flow; operator-hold-sec after startup; Copilot re-request preserves human/team reviewers.
- **Changelog Entry:** 2026-09-20 — #294 Copilot re-request workflow: clear bot reviewers with `requestReviews(union:false, botIds:[])` then re-request (REST DELETE cannot resolve Bot ids; prior union:true was a no-op with no ReviewRequestedEvent on HEAD).
- **Changelog Entry:** 2026-09-20 — #294 Copilot re-request workflow: remove existing request then requestReviews (union:true was a no-op when Copilot already requested; no fresh ReviewRequestedEvent on HEAD aafc2a3).
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 8: PUBLICIP_REQ empty body; COMPRESSEDPART allows partial chunks; transport-dispatched extractors; UDP single-datagram mode; redacted userhash; sanitized config paths; dry-run validate_ports; fail on missing configured evidence / malformed labels; kad_search_source not opcode-only PASS.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 7: TCP reassembly only within `from-pcap-tcp-<stream>-*` (UDP never joined); capability_negotiation reuses the same helper.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 6: `run-live.ps1` rejects negative `-PcapDurationSec` (was silently treated as unbounded 0).
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 5: direction-specific Kad HELLO/PING/FIND_NODE labels; cross-file evidence aggregation + TCP concat reassembly; SourceEx2 accepted for source_exchange; README from-pcap-NNNN; reject negative pcap_duration; malformed labeled evidence FAIL.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 4: per-packet tshark chunks (no flatten), SEARCH_RES dual layout (eMule 34 + Envy outbound 17), capability needs Hello+HelloAnswer, .bin skip only on captures sanitize.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round 3: SEARCH_RES production layout (34-byte header), exclude .bin from sanitized captures, capability_negotiation Hello-compare path only.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot round: Kad body min-shape + exclude Kad-in-TCP spans; fixed evidence source_name; optional_pcap PASS only after usable stop; early capture exit FAIL.
- **Changelog Entry:** 2026-09-20 — #160/#294 Copilot follow-up: Kad HELLO opcodes 0x11/0x19, pcap TCP+UDP 4672, kad_hello/ping/find_node evidence labels, optional_pcap owned lifecycle PASS, tshark post-capture extract, sanitized evidence source_name.
- **Changelog Entry:** 2026-09-20 — #294 temporary Actions workflow gated to PR #294 to re-request Copilot via GITHUB_TOKEN (agent requestReviewers is 403); delete after land.
- **Changelog Entry:** 2026-09-20 — #160/#294 interop evidence honesty: tcpdump BPF-last, CALLBACK exact 38, COMPRESSEDPART payload length match, optional_pcap availability SKIP, opcode-only packet evidence cannot PASS transaction scenarios.
- **Changelog Entry:** 2026-09-20 — #160 interop harness v1.1: recalculate capability truth after #251/#252/#254/#255/#256/#257/#258/#261 (compression_send=True); separate production/harness/evidence states; current ED2K/Kad scenario hooks; Windows operator helper + checklist; optional dumpcap/tshark; bounded packet evidence; no ED2K/Kad production protocol changes; live evidence still pending Windows operator run.
- **Changelog Entry:** 2026-09-20 — Agent PR back-pressure: replace the hard max-3 development PR cap with a soft target of 5, with documented overflow only.
- **Changelog Entry:** 2026-09-20 — #293 Copilot review: document the live mixed C++17/C++20 Release|x64 baseline (trailing `stdcpp17` wins on Envy/HashLib/TorrentEnvy/Unpacker); replace `feature/` example with `feat/` in the versioning guide. Copilot `APPROVED` is not proof of correctness.
- **Changelog Entry:** 2026-09-20 — #293 Copilot review: unclassified paths default to high-risk code/infra (Visual Studio, scripts, Remote, Unpacker, SkinBuilder, Repository, root build files).
- **Changelog Entry:** 2026-09-20 — #293 Copilot review: review-governance files are high-risk and excluded from the Copilot counting allowlist; `Wire-format impact: none` no longer covers crypto/auth/threading/locking/memory.
- **Changelog Entry:** 2026-09-20 — #293 Copilot review: risk-specific evidence (workflows/settings.yml cannot use wire-format text); ARM64 planned/unsupported in modernization table; C++20 first-party in standards.md; CHANGELOG playbook aligned with AGENTS.md.
- **Changelog Entry:** 2026-09-20 — #293 Copilot review: remaining UI gaps are approve/count, Balanced effort, and path allowlist only; automatic review is already live on Protect develop; drop deleted `.cursor/rules/08-dev-workflow.mdc` pointer.
- **Changelog Entry:** 2026-09-20 — #293: safe Copilot approval policy — assessment ≠ APPROVED; path-allowlist Stage-3 docs/rules globs; high-risk C++/workflows need evidence; Copilot-authored PRs are not self-approved; independent CI/security checks stay required.
- **Changelog Entry:** 2026-09-20 — #293: Copilot Code Review is the solo-maintainer required reviewer; auto-review runs on ready-for-review and on push (draft review off). Approve only with a real `APPROVED` review when repository Copilot approve/count settings are on. Stale #164 consolidation-deferral note removed.
- **Changelog Entry:** 2026-09-20 — Agent/workflow governance: consolidate repository-wide AI rules into `AGENTS.md`; remove legacy/duplicate Cursor and GitHub agent rule copies; add mandatory live-state preflight; Protect develop live now enforces 1 approval, Code Quality `All`, Copilot review-on-push on (draft off); coverage gate deferred until measured; Copilot UI approve/count, Balanced effort, and optional path allowlist still need manual verify; mark ARM64 planned/unsupported in issue forms; refresh developer guide.
- **Changelog Entry:** 2026-09-20 — #84 HashStringConversion: drop leftover `register` from `Unhex` (C++17). No hex decode behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Settings: replace `throw()` with `noexcept` on five `Add` overloads and `SmartAgent` (C++20; Item ctors deferred). No behavior change.
- **Changelog Entry:** 2026-09-20 — Crashpad: idempotent `CrashPadHost::Start`; single `CrashReporter::Initialize` from `InitInstance` (fix Debug double `StartHandler` DCHECK).
- **Changelog Entry:** 2026-09-20 — #84 ComObject: replace `throw()` with `noexcept` on CComObjectPtr members (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 HashTest: drop `register` and replace `throw()` with `noexcept` (C++17/C++20). Test harness only.
- **Changelog Entry:** 2026-09-20 — #84 SafeRelease: replace `throw()` with `noexcept` (C++20; template spacing collapsed). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 HGlobal: replace `throw()` with `noexcept` on dtor/Clean/conversions/IsValid/Detach/Size (C++20; ctors deferred). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 TransferFile: replace `throw()` with `noexcept` on IsOpen/IsExists/IsWritable/IsFolder (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 StreamArchive: replace `throw()` with `noexcept` on dtor/LPSTREAM/Detach/IsValid (C++20; ctors deferred). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 StdAfx.cpp: replace `throw()` with `noexcept` on InitGetMicroCount/NoThrowNew/OOM handlers (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 CLocked: replace `throw()` with `noexcept` on operator T / operator-> (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 CoolMenu: replace `throw()` with `noexcept` on SafeTrackPopupMenu/SafeQueryContextMenu (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 EnvyFile: replace `throw()` with `noexcept` on `CEnvyFile::GetSize` (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Shell: replace `throw()` with `noexcept` on `CShellItem::operator LPITEMIDLIST` (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 BTTrackerRequest: replace `throw()` with `noexcept` on `CAutoPtr<CBTTrackerRequest>::Free` (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 ThreadImpl: replace ten `throw()` specs with `noexcept` on CThreadImpl inlines (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 UPnPFinder: replace `throw()` with `noexcept` on CreateFinderInstance/ProcessAsyncFind (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 EDPacket: replace `throw()` with `noexcept` on CEDPacketTypes/GetAt (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Library: replace `throw()` with `noexcept` on SafeReadTime/SafeSerialize (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Application: replace `throw()` with `noexcept` on GetApp/GetUI/GetSettings (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 SQLite: replace `throw()` with `noexcept` on `CDatabase` bool/IsBusy/GetCount (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 VersionChecker: replace `throw()` with `noexcept` on `IsUpgradeAvailable` / `IsVerbose` (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Handshakes: replace `throw()` with `noexcept` on `CHandshakes::IsValid` (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — #84 SkinWindow: drop leftover `register` from alpha-blend temp (C++17). No blend behavior change.
- **Changelog Entry:** 2026-09-20 — #84 UTF-16 byte-swap: drop leftover `register` from XML/DlgLanguage/PageSettingsSkins endian-swap temps (C++17). No decode behavior change.
- **Changelog Entry:** 2026-09-20 — #84 MatchObjects: drop leftover `register` storage class from `CMatchFile::Compare` (C++17). No sort behavior change.
- **Changelog Entry:** 2026-09-20 — #84 QueryHashTable/QueryHashGroup: drop leftover `register` storage class (C++17). No hash-table behavior change.
- **Changelog Entry:** 2026-09-20 — #84 Strings.cpp: drop leftover `register` storage class (C++17). No string behavior change.
- **Changelog Entry:** 2026-09-20 — #84 HostCache: replace ten `throw()` exception specs with `noexcept` on `CHostCacheList` inlines (C++20). No behavior change.
- **Changelog Entry:** 2026-09-20 — Kad #86 app-trigger: ED2K downloads call `SearchSource` when EnableKad + Kad initialized; outbound SEARCH_SOURCE_REQ includes FileSize; `KadSearchSourceRequest.h` + EnvyTests. Buddy/UDP firewall / Hello nibble unchanged.
- **Changelog Entry:** 2026-09-19 — Local Debug x64 bootstrap: `scripts/bootstrap-vcpkg.ps1` mirrors CI `vcpkg install` because VS/MSBuild does not restore `vcpkg_installed` before `PreBuildEvent` (Crashpad copy). `EnvyOM.h` remains MIDL-generated. Installer `Main.iss` code 2 on a failed Envy build is a cascade (ISCC present, missing payload).
- **Changelog Entry:** 2026-09-19 — HashLib `CTigerTree` warning cleanup (#84 slice): constructor init order matches `TigerTree.h` (C5038); drop `register` in `CTigerTree::Tiger` (C5033). No Tiger/TTH algorithm or wire-format change. EnvyTests: constructor + identical-input/incremental/empty-file root stability.
- **Changelog Entry:** 2026-09-19 — #255 review follow-up: C2C CALLBACK known-file gate includes incomplete downloads; clear consume guard when `EDClients.PushTo` fails.
- **Changelog Entry:** 2026-09-19 — Kad2 routing × firewall rebase: `CollectFirewallCheckCandidates` walks the zone tree (`ForEachContact`); `SendFirewalledRequest` uses `KadContactGetSockAddr` (no `GetSockAddr` / `KAD_BUCKET_COUNT` / `buckets`).
- **Changelog Entry:** 2026-09-19 — Kad2 routing review: 128-bit `zonePrefix` (no uint32 shift UB at depth ≥ 32); first zone refresh arms `+10s` then fires; HELLO_RES requires outstanding HELLO_REQ; FIND_NODE_RES matches TargetID and is fail-closed on truncated contact lists before liveness.
- **Changelog Entry:** 2026-09-19 — Kad2 routing-table maintenance (#86 slice): XOR zone tree with aMule/eMule `CanSplit` (K=10, KBASE=4, KK=5, max depth 127), LRU/type liveness, bounded replacement cache, stale-zone FIND_NODE refresh, /24 diversity (2 per bin / 10 global, 1 IP). `KadRoutingTable.h` + EnvyTests. Kad2 remains partial/unverified; no capability advertise; UDP firewall/Buddy/callback still open.
- **Changelog Entry:** 2026-09-19 — Kad2 TCP firewall-detection baseline (#86 phase 1): `FIREWALLED_REQ`/`RES` exact framing, bounded outbound checks, public-IP consensus (2 independent peers), TCP Open only after 2 ACKs (UDP 0x59 or C2C 0xA8). UDP tester / Buddy / callback deferred. Kad2 remains partial/unverified; Hello Kad nibble stays 0. `KadFirewallCheck.h` + EnvyTests.
- **Changelog Entry:** 2026-09-19 — #87 phase-1 LowID/callback baseline: `PUBLICIP_REQ`/`ANSWER` (0x97/0x98) and C2C `CALLBACK` (0x99, 38-byte Buddy layout) with bounded state (`Ed2kLowIdCallback.h`); classic server push preserved; `REASKCALLBACKTCP`/Buddy/FWCHECK deferred. Not complete firewalled support.
- **Changelog Entry:** 2026-09-19 — #160/#253: record binary identity + process exits; Hello evidence fields; opt-in `workflow_dispatch` harness job (never required, never live on GitHub-hosted runners).
- **Changelog Entry:** 2026-09-19 — #160/#253: interop isolation refuses OS temp roots by path components (`python:S5443`); no ED2K/Kad production protocol changes.
- **Changelog Entry:** 2026-09-19 — #160 phase 1: opt-in ENVY ↔ eMule/aMule interop harness (`tools/interop/`) with dry-run CI self-tests; live reference binaries remain operator-provided; no ED2K/Kad production protocol changes.
- **Changelog Entry:** 2026-09-19 — Kad `nodes.dat` import copies the profile GUID through a local `Hashes::Guid` (`CGuarded` has no `operator[]`).
- **Changelog Entry:** 2026-09-19 — Kad `nodes.dat` review hardening: drop all legacy v0 contacts (no Kad2 version); bound XOR-closest insert to `nMaxOut`; copy Kad IDs with `Hashes::Guid::byteCount`.
- **Changelog Entry:** 2026-09-19 — Kad bootstrap: local modern `nodes.dat` v1/v2/v3 parser (`KadNodesDat.h`); v3 edition 1 bounded to 50 XOR-closest contacts; UDP-key fields parsed and discarded; no remote HTTP source; Kad2 still partial / unverified (#86 slice, not SEARCH_RES / routing / firewall).
- **Changelog Entry:** 2026-09-19 — #90: `CopyCrashpadHandler.cmd` copies the configuration-matching vcpkg handler (`tools/crashpad_handler.exe` for Release, `debug/tools/crashpad_handler.exe` for Debug; `tools/crashpad/` accepted as fallback) instead of the first recursive `dir /s /b` hit.
- **Changelog Entry:** 2026-09-19 — #90: `Envy.vcxproj` points `VcpkgManifestRoot` at the repo root and adds `vcpkg_installed` include/lib dirs so Crashpad `client/*.h` resolve (C1083 on PR #243). Handler copy and Inno Setup now require `crashpad_handler.exe`.
- **Changelog Entry:** 2026-09-19 — #90: replace BugTrap with Crashpad (out-of-process `crashpad_handler.exe`, local DB, upload off; D-019). Next-launch GitHub UX kept; `CrashDumpWin.h` removed. Isolated crash-class probe in `tools/crash-probe/`.
- **Changelog Entry:** 2026-09-19 — #87 slice: send-side ED2K `COMPRESSEDPART` / `COMPRESSEDPART_I64` in `CUploadTransferED2K::DispatchNextChunk()` (`Ed2kCompressedUpload.h`); peer `m_bEmDeflate==1` gate; eMule/aMule benefit fallback; EnvyTests smoke. Not full #87; live interop still #160.
- **Changelog Entry:** 2026-09-19 — Kad2 source SEARCH_RES → `AddSourceED2K` (#86 slice): outstanding search context (keyword vs source); inbound eMule/aMule SEARCH_RES parse; HighID types 1/4 only; `KadSearchResDelivery.h` + EnvyTests. Kad2 remains partial/unverified; no capability advertise.
- **Changelog Entry:** 2026-09-19 — Register `transfer_state_downloading_finished_not_importable` so #242 fail-closed Finished(Downloading) smoke actually runs (22/22 Linux g++).
- **Changelog Entry:** 2026-09-19 — Reinforce branch naming hard rule 11: forbid `cursor/`/`claude/`/Cloud-runner slug templates; CONTRIBUTING aligned to `type/short-kebab-summary` only.
- **Changelog Entry:** 2026-09-19 — Debug assert fix: NMDC code-page `Neighbours.Get` paths take `Network.m_pSection` (or copy from live hub); regression from #224.
- **Changelog Entry:** 2026-09-19 — #234: SonarCloud develop QG remediation — job-scoped GHA permissions, Remote CSP/label alignment, `.sonarcloud.properties` third-party exclusions (`docs/10_dev/sonarcloud-exclusions.md`).
- **Changelog Entry:** 2026-09-19 — #224: NMDC text uses per-hub/settings code page (`DcNmdcText.h`, default CP_ACP; invalid pages fall back to ACP); HostCache ser v2 `m_nCodePage` + `SetNmdcCodePage` (favorites UI later); ADC unchanged.
- **Changelog Entry:** 2026-09-19 — Remote API / *arr / Torznab audit: native REST vs qBit subset vs Torznab client; D-017 http.sys inbound + WinINet outbound; D-018 first *arr adapter is a qBittorrent Web API v2 **subset** (not a compatibility claim). `TransferState.h` + EnvyTests; docs `20_arch/AUDIT_REMOTE_API_2026-09.md`, `remote-api.md`, `arr-integration.md`, `torznab.md`, `docs/api/openapi.yaml` (planned).
- **Changelog Entry:** 2026-09-19 — Uploads Fair-Use is live: `Uploads.FairUseMode` (default off) clips each remote IPv4 client to 10% of an audio/video library file; checkbox bound; HTTP/ED2K/DC consumers; GET/ED2K/DC reserve then charge body bytes; unused reservation rolls back on `ClearRequest`/`Close` (keep-alive HEAD does not burn quota); BitTorrent and partials excluded.
- **Changelog Entry:** 2026-09-19 — Transfer settings foundation: Uploads page labels match the core (`Unlimited`, throttle Average/Maximum, max uploads per host); `TransferSettingsLimits.h` + EnvyTests; mapping in `docs/50_user/transfer-settings.md`.
- **Changelog Entry:** 2026-09-19 — Cloud Agent Linux env: add `.cursor/environment.json` installing clang-format-18/clang-tidy (CI-aligned) + cppcheck (local extra) + `Remote/tests` npm deps (MFC/HashLib remain Windows-only).
- **Changelog Entry:** 2026-09-19 — `CTextCtrl` System/Network log: top-aligned short journals, conventional scroll + follow-bottom only when already at end (`TextCtrlViewport.h` + EnvyTests); see `docs/10_dev/textctrl-log-ui-checklist.md`.
- **Changelog Entry:** 2026-09-19 — CI: always emit required `Build x64 Release` / `Build Win32 Release` on PRs so Protect develop does not block on SKIPPED Builds. Superseded gating: Draft may defer; Ready/`stage:live-test` always run real Windows builds (classify does not skip them).
- **Changelog Entry:** 2026-09-19 — Cross-platform foundations: document EnvyCore / platform / UI target, D-012…D-015, Win32 legacy policy (no removal), CMake portable-slice priority; Linux/macOS remain `planned` not `supported` (`docs/20_arch/PORTABILITY_PLAN.md`); trackers #177–#180 (do not duplicate #91/#161/#89).
- **Changelog Entry:** 2026-09-19 — DC hublist bootstrap: default URL `https://dchublist.org/hublist.xml.bz2`; `DefaultServices.dat` H rows refreshed (org/pwiam/ru HTTPS); `CUpdateServersDlg` DC mode (skin `CUpdateHubListDlg`) so Settings > DC++ > Download is not the eDonkey server.met dialog. Parser unchanged (`dchub://` kept, `adc://`/`adcs://` skipped). Not ADC/hublist-complete.
- **Changelog Entry:** 2026-09-19 — CI: required PR workflows also listen for `ready_for_review`; Format Check can run on `workflow_dispatch` against `origin/develop`.
- **Changelog Entry:** 2026-09-19 — Renovate fork enablement: migrate `renovate.json5` → root `renovate.json` with `forkProcessing: "enabled"` (Mend App API pre-check); Dependabot remains vcpkg-only.
- **Changelog Entry:** 2026-09-19 — #81: G2 SGP UDP reassembly/inflate capped (64 fragments; byte cap = min(MaximumPacket, 256 KiB) enforced in `Add`/`ToG2Packet`; outbound fragment count fail-closed).
- **Changelog Entry:** 2026-09-19 — #81: re-enable BitTorrent `SourcesWanted` caps for ut_pex, LTEP source-exchange, tracker HTTP apply, and UDP announce (`BtSourcesWantedAllowsMore`).
- **Changelog Entry:** 2026-09-19 — #81/#82: `CHttpRequest` exact-limit probe — keep bodies of size `== LimitContentLength` after one-byte `InternetReadFile` EOF; discard on further data or probe failure (VersionChecker 64 KiB / Update Servers 32 MiB predicates stay `<= max`).
- **Changelog Entry:** 2026-09-19 — #81/#82: VersionChecker HTTP bodies capped at 64 KiB (`LimitContentLength` + `VersionCheckerHttpResponseOk`).
- **Changelog Entry:** 2026-09-19 — #81/#82: Update Servers dialog HTTP bodies capped at 32 MiB (`LimitContentLength` + `UpdateServersHttpResponseOk`).
- **Changelog Entry:** 2026-09-19 — CI audit (`docs/10_dev/CI_AUDIT_2026-09.md`): measured ~7 min PR critical path (Build x64 ∥ CodeQL c-cpp); skip empty NuGet (~22s/job), failure-only PR build logs, always-emit Documentation Check; PR Gate keeps `POLL_SEC` script default 15s (no 10s override — API quota); Protect develop + Protect main API snapshots; live Protect develop approval-count drift (API 0 vs intended 1).
- **Changelog Entry:** 2026-09-19 — #81: Kademlia store-entry tags capped at 4 KiB (`KadStoreTagLengthOk`) in `ReadEntryTags`.
- **Changelog Entry:** 2026-09-19 — #81: BitTorrent MSE responder `len(IA)` capped at 96 (`BtMseIaLengthOk`); partial IA waits in `MSE_AWAITING_IA` before crypto_select (RC4 desync fix).
- **Changelog Entry:** 2026-09-19 — Bootstrap catalogue review follow-up: migrate obsolete `ServerListURL`/`HubListURL` defaults in `SmartUpgrade`; reject extra `host:port` colons; validate UHC/UKHL ports 1–65535. Kad validated HTTPS `nodes.dat` cold-start discovery is implemented; local ImportNodes v1/v2/v3 is implemented. Missing catalogue/DNS must not restore a C++ DHT seed (D-012).
- **Changelog Entry:** 2026-09-19 — #81: BitTorrent MSE receive Pad_C/Pad_D capped at 512 (`BtMsePadLengthOk` / `MSE_PAD_MAX_LEN`); pad length fields decoded/encoded big-endian.
- **Changelog Entry:** 2026-09-19 — NMDC hub user list + remote file-list browse: bounded `$NickList` merge with `$MyINFO`, hub-user Browse via existing `CHostBrowser` `PROTOCOL_DC`, fail-closed `files.xml.bz2` (transfer-time compressed cap + bounded XML walker in `LoadDC`), owned path/index snapshot for Browse Host tree before `CNetwork` owns hits, skip unhashed TTH on outgoing lists and skip TTH-less incoming File entries. Completion is hub+port not nick-only. ADC hubs / TLS / `$MyINFO` slots remain separate. Live hub interop unverified.
- **Changelog Entry:** 2026-09-19 — #81: NMDC `$ADCGET`/`$ADCSND` strict asymmetric numeric parse (`DcAdcGetValidate.h`); GET allows `-1` until-EOF; SND requires real length; length-aware tokens reject embedded NUL / `2^64-1`; fail-closed SND vs fixed request (no `min()`).

- **Changelog Entry:** 2026-09-18 — Bootstrap catalogue refresh (PR A): `DefaultServices.dat` / `DefaultServers.dat` audited; static ED2K IPs and dead GWC/hublists removed; HTTPS `server.met` + hublists; gtk-gnutella UHCs; Transmission/libtorrent DHT routers. Kad remote `nodes.dat` not added (partial Kad2; ImportNodes v1-only). See `docs/30_protocols/bootstrap-sources.md`.
- **Changelog Entry:** 2026-09-19 — #81: BitTorrent TCP length-prefix capped at 16 MiB (`BtPacketLengthOk`); oversize clears buffer and closes peer (`PROTOCOL_TOO_LARGE`).
- **Changelog Entry:** 2026-09-19 — #81: G2 HIT_WRAP / wrapped G1 fail-closed via `G1WrappedPayloadFits` / negative `m_nLength` reject in `CG1Packet::New` + null-check call sites.
- **Changelog Entry:** 2026-09-19 — #81: G2 compound/frame length checks order-safe (`G2SubpacketPayloadFits` / `G2FrameLengthFits`) in ReadPacket/SkipCompound/ReadBuffer (defense-in-depth).
- **Changelog Entry:** 2026-09-19 — #81: G1 QueryHit QHD `nXMLSize` fail-closed via `G1QueryHitXmlFits` (must leave trailing GUID, including zero-length XML); no soft clamp to 0.
- **Changelog Entry:** 2026-09-19 — #81: ED2K `VIEWSHAREDDIRANSWER` consumes WORD-prefixed directory name before `count`; `OnViewSharedDir` / `OnAskSharedDirsAnswer` / `OnServerMessage` fail-closed via `Ed2kEdString*` + `Ed2kServerMessageLengthOk` (5000-byte MOTD cap).
- **Changelog Entry:** 2026-09-19 — #81: ED2K chat `MESSAGE` length checks centralized in `Ed2kChatMessageLengthOk` (+ EnvyTests); valid-frame wire behavior unchanged, malformed lengths rejected; outgoing `SendPrivateMessage` clamps by encoded byte length.
- **Changelog Entry:** 2026-09-19 — #81: unknown ED2K tag skip fail-closed when STRING length claim exceeds remaining (`Ed2kUnknownTagStringSkipOk`); INT fallback only above skip-max.
- **Changelog Entry:** 2026-09-19 — #81: `CEDPacket::ReadEDString` / `ReadLongEDString` fail-closed when length prefix exceeds remaining (`Ed2kEdStringPayloadOk` / `Ed2kLongEdStringPayloadOk`).
- **Changelog Entry:** 2026-09-19 — #81: G1 TCP framing uses overflow-safe `G1PacketTotalLengthOk` for signed payload length vs `MaximumPacket`.
- **Changelog Entry:** 2026-09-19 — #81: G1 UDP datagram path uses overflow-safe G1PacketTotalLengthOk (rejects negative m_nLength wrap before CG1Packet::New).
- **Changelog Entry:** 2026-09-19 — Agent workflow: after every PR push use `gh pr checks --required --watch --fail-fast --interval 5` (no arbitrary CI sleeps); see `AGENTS.md` §5.
- **Changelog Entry:** 2026-09-19 — #81: ED2K `COMPRESSEDPART` / `COMPRESSEDPART_I64` stream inflate capped at one part or the remaining file size, whichever is smaller (`ED2K_COMPRESSEDPART_INFLATE_MAX` / `Ed2kCompressedPartInflateBudget` / `Ed2kCompressedPartInflateOk`) before `SubmitData`; `CEDClient::OnPacket` propagates inflate rejection.
- **Changelog Entry:** 2026-09-19 — #81: GGEP DEFLATE inflate capped at 256 KiB (`GGEP_INFLATE_MAX` / `GgepInflateOutputOk`).
- **Changelog Entry:** 2026-09-19 — #81: `CBuffer::InflateStreamTo` default `nMaxOutput=0` to `CBUFFER_INFLATE_STREAM_MAX` (32 MiB) for Neighbour G1/G2 deflate backlog; G1/G2/ED/DC `OnRead` fail-closes on inflate error; `CBufferInflateStreamOutputOk` smoke coverage.
- **Changelog Entry:** 2026-09-19 — #81: `CBuffer::Inflate`/`Ungzip` default `nMaxOutput=0` to `CBUFFER_INFLATE_MAX` (32 MiB); `CBufferInflateOutputOk` smoke coverage.
- **Changelog Entry:** 2026-09-19 — #81/#82: Browse Host HTTP peer `Content-Length` / buffered body capped at 32 MiB (`HostBrowserHttpBodyOk` / `HostBrowserHttpBufferOk`); strict decimal Content-Length; InflateStreamTo output cap on deflate path.
- **Changelog Entry:** 2026-09-18 — Bootstrap catalogue refresh (PR A): `DefaultServices.dat` / `DefaultServers.dat` audited; static ED2K IPs and dead GWC/hublists removed; HTTPS `server.met` + hublists; gtk-gnutella UHCs; Transmission/libtorrent DHT routers. `CDHT::Connect` sends BEP 5 `find_node` to BitTorrent hosts from HostCache (no extra C++ DNS list). Kad remote `nodes.dat` not added (partial Kad2; ImportNodes accepts old format and new-format version 1 only). See `docs/30_protocols/bootstrap-sources.md`.
- **Changelog Entry:** 2026-09-18 — #82: wire-path ED2K `ED2K_TAG_BLOB` uses `Ed2kTagBlobLengthOk` (4 MiB + remaining), matching `.met` policy.
- **Changelog Entry:** 2026-09-18 — Format Check: encoding-safe `clang-format-diff-safe` wrapper so ISO-8859 Envy sources do not UnicodeDecodeError under stock clang-format-diff.
- **Changelog Entry:** 2026-09-18 — #92: remove erroneous `delete pRoot` in BT `OnSourceResponse` (packet-owned `m_pNode`); null-check `GetNode("peers")` and nested peer fields before `IsType`.
- **Changelog Entry:** 2026-09-18 — CI polish on #164: Format Check fail-closed + clang-format-diff (changed hunks, clang-format-18 pinned); CodeQL decoupled from classifier; PR Gate rejects neutral/unexpected skip; C# suite simplified to security-and-quality.
- **Changelog Entry:** 2026-09-18 — CI: CodeQL always emits c-cpp + javascript-typescript + csharp on every PR to `develop` (fixes Code Scanning "configuration not found"); Format Check is blocking (`--Werror`, no `continue-on-error`).
- **Changelog Entry:** 2026-09-18 — Protect develop docs aligned to live ruleset: ≥1 APPROVED review, dismiss-stale on push, `require_last_push_approval` off, signed commits + force-push block, CodeQL/Gitleaks code scanning, no GitHub Code Quality rule; Dependabot auto-approve removed.
- **Changelog Entry:** 2026-09-18 — #97: explicit `timeout-minutes` on lightweight Code Quality / version / Copilot setup jobs.
- **Changelog Entry:** 2026-09-18 — #81: ED2K FileComment header/length fail-closed vs remaining (`Ed2kFileCommentLengthOk`).
- **Changelog Entry:** 2026-09-18 — #81: wire `ED2K_TAG_UINT64` remaining check fixed (need 8 bytes, not 1) via `Ed2kTagUint64RemainingOk`.
- **Changelog Entry:** 2026-09-18 — #81: `CNetwork::m_oJobs` capped at 2048 (`NetworkJobQueueCountOk`); drop oldest owned search/hit on overflow.
- **Changelog Entry:** 2026-09-18 — #81: `CChatSession` undelivered message queue capped at 1024 (`ChatSessionQueueCountOk`); drop oldest on overflow.
- **Changelog Entry:** 2026-09-18 — #81: NMDC hub `m_oUsers` capped at 20,000 new `$MyINFO` nick inserts (`DcHubUserCountOk`) to stop MyINFO flood DoS.
- **Changelog Entry:** 2026-09-18 — #81: hublist / DC `.bz2` loaders use `LoadFromBZipFile` / `UnBZip(CBUFFER_UNBZIP_MAX)` (32 MiB); legacy `UnBZip()` with nMaxOutput=0 stays unlimited.
- **Changelog Entry:** 2026-09-18 — #81: G1 `{deflate}` XML inflate capped at 256 KiB (`G1_DEFLATE_XML_INFLATE_MAX` / `G1DeflateXmlInflateOk`) in QueryHit and G1Packet readers.
- **Changelog Entry:** 2026-09-18 — #81: Gnutella QHT/QRP patch compressed budget + Inflate output cap to expected patch size (`QhtPatchCompressedBudgetOk`).
- **Changelog Entry:** 2026-09-18 — #81: `CEDPacket::Inflate` defaults to 512 KiB (`ED2K_PACKED_INFLATE_MAX` / `Ed2kPackedInflateOk`) for packed C2C/UDP/server paths; 0 no longer means unlimited.
- **Changelog Entry:** 2026-09-18 — #81/#82: BitTorrent tracker HTTP announce/scrape bodies capped at 32 MiB via `LimitContentLength` + `BtTrackerHttpResponseOk` (closes unused limit API for live tracker downloads).
- **Changelog Entry:** 2026-09-18 — #81/#82: Discovery GWC/server-list HTTP bodies capped at 32 MiB (`LimitContentLength` + `DiscoveryHttpResponseOk`).
- **Changelog Entry:** 2026-09-18 — #81/#82: file-backed ED2K tag key / TAG_STRING lengths checked against remaining `.met` bytes (`Ed2kTagStringLengthOk`) before allocate/Read.
- **Changelog Entry:** 2026-09-18 — #166 / D-009 P1: Windows Firewall exceptions via WFAS `INetFwPolicy2` (all Domain/Private/Public profiles); drop legacy `INetFwMgr`.
- **Changelog Entry:** 2026-09-18 — #76: Remote UI HTML-escapes `CRemote::Add()` substitutions (`Escape`); `AddRaw` for trusted markup; `RemoteHtmlEscape.h` + EnvyTests smoke coverage.
- **Changelog Entry:** 2026-09-18 — #82 partial: BEP-9 ut_metadata advertised size capped at 32 MiB (`BtUtMetadataSizeOk`) before accepting metadata pieces.
- **Changelog Entry:** 2026-09-18 — #79: Remote passwords stored with PBKDF2-HMAC-SHA256 (`BCryptDeriveKeyPBKDF2`); legacy SHA1 / sha256-salted verify + migrate on login.
- **Changelog Entry:** 2026-09-18 — #92 cooperative close: abandon timed-out threads without `TerminateThread` (`EnvyThreadPolicy.h`); completes remaining #92 slice after lock-order.
- **Changelog Entry:** 2026-09-18 — #92 lock-order: EDClients before Transfers (`Ed2kLockOrder.h`); remaining #92 item is cooperative thread close without `TerminateThread`.
- **Changelog Entry:** 2026-09-18 — #121: CryptLayer Hello bits stay 0 (TCP obfuscation unimplemented); peer Hello crypt bits no longer start PUBLICKEY packet crypto (`Ed2kCryptLayerHelloBitsMayStartPacketCrypto`).
- **Changelog Entry:** 2026-09-18 — DevSecOps finalize: `AGENTS.md` controlled autonomy + max-3 development PR cap; Renovate `enabledManagers` = github-actions only; CodeRabbit `drafts: false` + `Envy/*Remote*` path; `ci-verify.ps1 -Full` builds/runs Win32 EnvyTests and requires clang-format; Merge Queue documented as optional.
- **Changelog Entry:** 2026-09-18 — #96: pin external GitHub Actions to immutable commit SHAs under `.github/workflows` and `.github/actions` (documented upgrade path in `docs/10_dev/agents-and-automation.md`).
- **Changelog Entry:** 2026-09-18 — Restored historical `CHANGELOG.md` body truncated by #148 squash (kept current Unreleased; reattached from `## [4.1.0]` onward).
- **Changelog Entry:** 2026-09-18 — #81: NMDC HubName/HubTopic/chat prefixed-payload length guards before trailing-`|` arithmetic (`DcPacketLengthValidate.h` + EnvyTests).
- **Changelog Entry:** 2026-09-18 — Restored full `docs/DEVELOPMENT_PLAN.md` body accidentally truncated by #159 squash; retained post-#147/#153 changelog lines and #119 `{deflate}` resolution note.
- **Changelog Entry:** 2026-09-17 — #119: QueryHit `nSize-10` vs G1Packet `len-9` `{deflate}` sizing documented as intentional (trailing NUL vs HIT_SEP framing); no wire change.
- **Changelog Entry:** 2026-09-17 — #92 slice: Remote Base64 empty-input safety (`RemoteBase64.h`), CHM `LocalAlloc`/`LocalFree` pairing, TorrentEnvy clipboard wide-NUL size. #92 TerminateThread and lock-order slices delivered.
- **Changelog Entry:** 2026-09-17 — Portable ZIP staging now mirrors the Inno runtime tree (`stage-portable.ps1`: `Data`/`Skins`/`Schemas`/`Plugins`/…); flattened EXE/DLL-only ZIPs are rejected by `verify-artifacts.ps1`.
- **Changelog Entry:** 2026-09-17 — Hardened `release.yml`: replaced parallel `softprops/action-gh-release` asset uploads with idempotent sequential `gh api` uploads by `release_id` (`scripts/release/*`); draft stays unpublished; `workflow_dispatch` remains dry-run unless explicit `repair_release_id`.
- **Changelog Entry:** 2026-09-17 — #120: ED2K preview frames capped at 4 MiB and written with a bounded bulk copy (`Ed2kPreviewFrameAcceptable`).
- **Changelog Entry:** 2026-09-17 — #77: Remote CSRF enforced for mutating query keys (`connect`/`disconnect`, filters, group/queue UI actions); `_method` no longer bypasses CSRF.
- **Changelog Entry:** 2026-09-17 — #141: listen sockets open before NAT completes (`OnRun` no longer waits on `IsAsyncFindRunning`); `MapPorts` starts after successful bind/listen (D-011).
- **Changelog Entry:** 2026-09-17 — #78 security: remove weak `rand()` fallbacks for session/CSRF/salt and protocol anti-spoof nonces; single CSPRNG helper (`SecureRandom.h` / `BCryptGenRandom`) with fail-closed contracts. Out of scope: #79 PBKDF2, #77 CSRF policy, #76 XSS.
- **Changelog Entry:** 2026-09-17 — #140 docs: clarify MiniUPnPc 2.0 SSDP is one discovery receive phase (`searchalltypes=1`), not a strict wall-clock deadline; #142 / P3 covers absolute SSDP and HTTP timeout bounding.
- **Changelog Entry:** 2026-09-17 — #140 P0 runtime PASS on post-squash HEAD: targeted IGD discovery (single MiniUPnPc receive phase), gateway-only rootdevice fallback, non-IGD devices never receive WAN mapping commands. Known debt: MiniUPnPc 2.0 SSDP/HTTP latency (#142 / P3). Listen-before-NAT delivered as #141. Next after #166 WFAS: P2 ports.
- **Changelog Entry:** 2026-09-17 — #140 P0 strategy: targeted IGD SSDP (`searchalltypes=1`, one discovery receive phase) + gateway-only rootdevice fallback. SSDP success != usable IGD; discovery fails cleanly when no IGD/WAN service is exposed.
- **Changelog Entry:** 2026-09-16 — UPnP SSDP discovery selects Internet-facing IPv4 via `GetAdaptersAddresses` + `GetBestRoute2` (`NetworkInterfaceSelector`), replacing `GetAdaptersInfo` fallback on PR #140. Phased NAT/Firewall plan: P0 interface → P1 WFAS firewall → P2 local/external ports → P3 MiniUPnPc 2.3.x vendored → P4 PCP/NAT-PMP → P5 CGNAT/diagnostics. MiniUPnPc stays vendored (not vcpkg-only) for now.
- **Changelog Entry:** 2026-09-16 — Started **Envy 4.2.0 Preview 1** release preparation: single version source (`4.2.0-preview.1` / Windows `4.2.0.1`), Inno `alpha` driven from CI (`/p:InstallerAlpha=Preview`), `release.yml` publishes per-platform setup + ZIP + `SHA256SUMS.txt` as draft prerelease. Universal installer and Authenticode deferred. See README Preview section. (Still gated on interactive installer smoke before merge; includes #139 WebHook BHO fix.)
- **Changelog Entry:** 2026-09-16 — Fixed legacy IE WebHook startup registration: skip `WebHook32.dll`/`WebHook64.dll` (and historical `WebHook.dll`) when `WebHookEnable` is false; BHO key registered in code under HKCU (per-user) or HKLM (machine). Documented as IE-only legacy; candidate for removal in favor of a modern browser extension + `envy://url:`.
- **Changelog Entry:** 2026-09-16 — Search Input/Advanced panel layout is font/DPI-aware (`GetPreferredHeight` + progressive Y); combo drop-down height kept separate from visible stacking; hash/prefix anchored to the search edit. No change to `GetSearchPanelWidth()` / SidebarWidth floor.
- **Changelog Entry:** 2026-09-16 — Skin engine P0: StatusbarHeight pointer fix, ParseRect point/size + FindOneOf, roundRect size validation, LoadFromXML section-failure aggregation (non-transactional), strict metric parse/clamp aligned with Settings bounds; `SkinEngineP0.h` + EnvyTests. HiDPI/logical units deferred to P1+.
- **Changelog Entry:** 2026-09-16 — Search window left panel clamps to `max(SidebarWidth, SCALE(200))` only in `CSearchWnd` (Shareaza PANEL_WIDTH); avoids Advanced two-column collapse without raising the global SidebarWidth floor used by other panes / ~182 px PeerProject skins.
- **Changelog Entry:** 2026-09-16 — Fixed CoolMenu selected-item double blue band: `DrawButton`/`DrawButtonMap` stretch one skin state vertically (no vertical tile) when destination height exceeds asset height; CoolMenu `rcItem` stays within `DRAWITEMSTRUCT` and icon offsets use `SCALE()`.
- **Changelog Entry:** 2026-09-15 — Deterministic `About.htm.gz` / `Browser.htm.gz` generation (`gzip -n`), restore valid binary blobs, `*.gz binary` in `.gitattributes`. See `docs/10_dev/build.md`.
- **Changelog Entry:** 2026-09-15 — Added Envy self-golden Hello/HelloAnswer TCP vectors (`Ed2kHelloWire.h` + EnvyTests) freezing honest MiscOptions bits; compression advertise left frozen for post-interop decision. No wire behavior change.
- **Changelog Entry:** 2026-09-15 — Reconciled status/roadmap: live Kad2 is `Kademlia.cpp` only (`KadProtocol` legacy inactive); SEARCH/PUBLISH wire-only; ADC/ADCS hub not implemented (NMDC preserved). Prevents roadmap drift from the September 2026 current-code audit.
- **Changelog Entry:** 2026-09-15 — ED2K Hello honesty: stop advertising Ext Multipacket (MiscOptions2 bit 5) until 0x92/0xA4 (or wired Ext2) handlers exist; `Ed2kExtMultipacketAdvertised()` + smoke tests.
- **Changelog Entry:** 2026-09-15 — Keep C++ RTTI disabled (`/GR-`). `CUploadQueue::StartImpl` uses a construction-proven `static_cast` instead of `dynamic_cast` for ED2K uploads. Future protocol actions should move to virtual methods; do not enable global RTTI.
- **Changelog Entry:** 2026-09-15 — Fixed ED2K regression: `CEDPacket::WriteFile` no longer uses `dynamic_cast` (Envy builds with `/GR-`, so it crashed in `__RTDynamicCast` during `SendSharedFiles`); restored the historical `static_cast` contract (complete files are always `CLibraryFile`). No RTTI enablement.
- **Changelog Entry:** 2026-09-13 — Fixed Debug splash assertion: `nSplashSteps` now accounts for the conditional `Kademlia DHT` step when `eDonkey.EnableKad` is enabled (miscount since Kad init splash was added; surfaced after EnableKad defaulted true).
- **Changelog Entry:** 2026-09-11 — Documented external P2P reference implementations (eMule Community, aMule, eMule Qt, eMule AI, aria2-next, Ember, Rucio, eMule eSE), Envy’s multi-network positioning, specification-first policy (D-008), and the P0–P3 interoperability/architecture sequence. Restored the missing `docs/10_dev/status.md` matrix. Corrected remaining SecureIdent “active/complete” claims: RSA SecureIdent is not implemented (#75).
- **Changelog Entry:** 2026-09-10 — Runtime performance audit backlog: reproducible benchmarks (#111), then CBuffer front-consume (#112), network hot-path copies/locks (#113), TransferFiles I/O contention (#114). IOCP and dedicated hashing optimization deferred pending evidence. #102 remains CI runner latency only.
- **Changelog Entry:** 2026-09-10 — Two-speed GitHub Actions: change-aware PR
  gate (skip Windows/CodeQL/Remote/C# when unrelated), CodeQL C++ `build-mode: none`
  on PRs with full manual analysis on `develop`/weekly, vcpkg files binary
  cache (Win32 included; `x-gha` is gone upstream), differential Format Check, clang-tidy moved off PRs, EnvyTests after
  MSBuild. Live required check names include Format, Documentation, secret-scan,
  gitleaks, PR Gate, Analyze (c-cpp), and SonarCloud (see `.github/settings.yml`).
  `PR Gate` is a required CI wait job; it does not replace GitHub review rules.
- **Changelog Entry:** 2026-09-10 — Safely disabled invalid ED2K/eMule SecureIdent verification (#75): no SecureIdent advertisement, no MD5/non-zero accept path, peers never marked verified without future RSA validation. Documented ED2K SecureIdent RSA roadmap and separate ED2K/Kad interop checklists. SecureIdent remains authentication/trust only — not required for ED2K connectivity.
- **Changelog Entry:** 2026-09-08 — Restored inbound packet length validation (closed PR #69) on current `develop`: ED2K `ReadBuffer`, BitTorrent extension framing, Gnutella QueryHit `{deflate}`, GGEP `H`/`M` type-byte guards, and ED2K preview frame unsigned bounds. Shared predicates in `PacketLengthValidate.h` with EnvyTests smoke coverage. Documented QueryHit vs G1Packet `{deflate}` -10/-9 sizing as a known inconsistency (functional follow-up, not fixed here).
- **Changelog Entry:** 2026-05-27 — Documented linear-history workflow for `develop`: squash/rebase merges only, `git pull --ff-only`, feature-branch rebase commands; aligned `.github/settings.yml` with GitHub merge settings.
- **Changelog Entry:** 2026-05-17 — Improved CodeQL C# analysis precision by introducing a dedicated manual-build workflow and documenting legacy FictionBookReader build blockers plus minimal .NET Framework 4.8 retarget path.
- **Changelog Entry:** 2026-05-15 — Synced repository hygiene status for `develop`: documented branch state, CI gate maturity, Dependabot labels requirement, and dependency register status (`docs/DEPENDENCIES.md` exists but remains an incomplete seed).
- **Changelog Entry:** 2026-05-15 — Hardened legacy Kad publish packet construction by replacing unsafe keyword copy with bounded copy and explicit terminator in `KadProtocol::CreatePublishRequest`, preserving wire format.

- **Changelog Entry:** 2026-05-15 — ED2K Source Exchange hardening: added shared bounds validation for SourceEx/SourceEx2 source lists, modernized SourceEx2 request length handling, and documented current IPv4-only SourceEx wire limitation.

## Update Protocol
1. Update **Last Updated** date on every meaningful change.
2. Add a one-line entry to the changelog section above.
3. Reflect status changes in **Current Status** and **Roadmap**.
4. Record consequential technical decisions in **Decisions Log**.
5. Close or refresh **Open Questions** explicitly.

## Repository Status (develop)
- Default branch is `develop`.
- `main` is currently behind `develop`.
- **Merge policy (GitHub):** merge commits disabled; squash and rebase merges
  enabled globally. Protect develop forces **squash-only** onto `develop`.
- **History:** `develop` was rewritten to a linear history with no merge commits; the pre-rewrite snapshot is preserved as the immutable tag `backup/develop-before-linear-rewrite` (local mutable backup/rollback branches were removed after the rewrite stabilized).
- **Local hygiene:** use `git pull --ff-only` on `develop`; rebase feature branches with `git rebase origin/develop` and `git push --force-with-lease`.
- **Branch protection:** `Protect develop` requires pull requests, linear
  history, conversation resolution, code
  scanning (CodeQL+Gitleaks), passing required checks, squash-only, and blocks
  force-pushes/deletions. Desired status contexts are listed in
  `.github/rulesets/protect-develop.desired.json` (live may still list Sonar
  until post-merge migration). There is **no** PR Gate poller — native workflow
  conclusions are the merge gate. See `docs/10_dev/pr-workflow.md`.
- **Approval mismatch (verified 2026-10-07):** live `Protect develop` requires
  zero approving reviews; repository policy still requires one independent
  approval and a human approval for privileged governance paths. This PR
  records the discrepancy without changing the live ruleset.
- CI uses a two-speed model: change-aware PR jobs for Windows/Remote/deps plus
  full integration on `develop` / scheduled analysis. Every PR always runs
  CodeQL Analyze (c-cpp), (javascript-typescript), (python), and (actions), plus blocking Format
  Check. C# analysis and Windows Release eligibility follow staged pr-phase
  policy; Ready/live-test runs actual analysis and both Windows builds.
  Classification outputs are advisory and never replace these required jobs
  with successful not-applicable results. Those workflows also run on
  `ready_for_review`; see `docs/10_dev/pr-workflow.md`.
- Dependency Review runs on every pull request targeting `main`, `master`, or `develop`; see `docs/10_dev/dependency-automation.md`.
- Dependabot expects GitHub labels `ci` and `dependencies` to exist for automated PR labeling.

## Canonical Documentation Split
- `docs/DEVELOPMENT_PLAN.md`: strategic roadmap, major decisions, and sequencing (this file).
- `docs/10_dev/status.md`: protocol/architecture status matrix (evidence-based; no “complete” without proof).
- `docs/10_dev/roadmap.md`: technical modernization itemization aligned with the P0–P3 sequence below.
- `docs/30_protocols/REFERENCE_IMPLEMENTATIONS.md`: external P2P reference projects and spec-first policy.
- `.local/DEV_TRACKER.md`: session notes (gitignored). `docs/DEV_TRACKER.md` is also gitignored and is not a committed source of truth.

## Vision & Goals
- Keep Envy a stable **Windows-native multi-network** client today: BitTorrent, Gnutella, Gnutella2, ED2K, Kad, Direct Connect, Remote/Web, plus library and multi-network search.
- Evolve toward a portable `EnvyCore` + platform abstraction so Linux x86_64 and macOS ARM64 can become real targets later (**planned**, not supported yet). See D-012 and `docs/20_arch/PORTABILITY_PLAN.md`.
- Improve ED2K/Kad interoperability against eMule Community and aMule without dropping other networks.
- Reduce modernization risk with incremental core/UI boundaries, testability, and dependency hygiene.
- Increase release confidence through clearer architecture boundaries and measurable quality gates.
- Stop reinforcing unnecessary MFC/Win32 coupling in **new** core interfaces (D-013) while preserving the MFC Windows frontend.

## Positioning
Envy is not an eMule fork and is not replaced by aMule, eMule Qt, aria2-next, Ember, or Rucio. Those projects are **references**:

- ED2K/Kad behaviour: eMule Community and aMule
- core/UI split: eMule Qt (incremental, not a rewrite)
- modern reachability: eMule AI
- engine/API/headless: aria2-next
- DHT/security research: Ember (Envy-specific extensions only; never labelled Kad2)
- IPv6/Kad6 experiments: eMule eSE (P3 only)

Policy: specification first, interoperability implementation second. See D-008 in `docs/DECISIONS.md`.

## Current Status
### Done
- CI workflows for build/quality/security exist with staged validation:
  Draft runs the cheap lane, `stage:live-test` and Ready run full Windows/C#
  validation, and native workflow conclusions provide the merge checks.
- Hash-focused unit tests integrated in repo and workflows.
- Audit and core documentation baseline established.
- Remote CRITICAL/HIGH security items: CSRF (#77), PBKDF2 passwords (#79), CSPRNG (#78), CSP/redirects/rate limit, and Remote XSS HTML escape on `Add()` (#76). Remaining Remote security follow-ups tracked separately if found.
- Remote JS security regression tests wired into `code-quality.yml`.
- Skin engine **P0** input hardening (`SkinEngineP0.h`): StatusbarHeight registration,
  ParseRect `point`/`size`, roundRect validation, LoadFromXML section-failure
  aggregation (non-transactional), strict metric parse/clamp. HiDPI deferred.
- **#90 / #354 crash reporting** — BugTrap removed. **x64:** BugSplat SDK v8.0.0 (`ThirdParty/BugSplat`, optional upload when `EnvyBugSplatDatabase` is set). **Win32:** Crashpad local database + next-launch opt-in GitHub issue workflow (upload off). See `docs/10_dev/crash-reporting.md` (D-019, D-020).

### In Progress
- **BugSplat runtime reliability (#359 / PR #362)** — synchronize deterministic x64 dependency discovery with installer and portable packaging; require staged CRT files and preserve Win32 Crashpad. Current-head CI and maintainer runtime validation remain required. Live Protect develop currently records zero required approvals, contrary to the documented one-approval policy; independent human approval remains required for workflow changes.
- **Envy 4.2.0 Preview 1 release readiness** — version/packaging PR; install/uninstall + network smoke tests still required before tagging `v4.2.0-preview.1` and publishing the draft GitHub prerelease.
- C++ modernization across legacy modules.
- Incremental protocol compatibility and robustness improvements.
- **P0 ED2K/Kad interoperability baseline** against eMule Community and aMule (live interop unverified; harness exists — `tools/interop/README.md`; see `docs/10_dev/status.md`).
- **Bootstrap catalogues** — shipped `DefaultServices.dat` / `DefaultServers.dat` refreshed 2026-09-18. Remaining: importer hardening (P0 potential); last-known-good remote catalogue (`docs/30_protocols/bootstrap-sources.md`). Validated HTTPS Kad `nodes.dat` cold-start fallback and local `ImportNodes` v1/v2/v3 are implemented; Kad2 interop remains partial. Do not restore C++ DHT DNS constants when the catalogue is missing (D-012).
- Transfer settings UX: first slice (labels + validation + mapping) in `docs/50_user/transfer-settings.md`; no fake capabilities.
- **Modern Transfer Capacity / Broadband 2026** (#341–#345, #332, #111): #341 correctness foundation (multigig-safe conversions + upload headroom) in flight; #342 capacity vs user limits UX; #343 connection-speed presets; #344 elastic ED2K/HTTP slots; #345 BitTorrent choking; measure (#111) before raising concurrency (#113/#114).

### Blocked / At Risk
- Full CMake parity with Visual Studio build graph.
- Dependency refresh for older vendored components without regressions.

## Roadmap

Protocol and architecture work uses the sequence below. Engineering phases (tests, CI, CMake, performance) remain in parallel and must not be dropped for ED2K-only work.

Priorities must match `docs/10_dev/status.md` and `docs/10_dev/roadmap.md`.

### P0 — ED2K / Kad interoperability baseline

Absolute priority before new ED2K/Kad extensions. Validate against eMule Community and aMule (live Envy ↔ eMule, Envy ↔ aMule, ideally eMule ↔ Envy ↔ aMule). Status today: **partial / unverified live**. Phase-1 evidence collection: `python3 tools/interop/run.py` (`tools/interop/README.md`). The harness existing is **not** an interoperability claim.

ED2K: Hello / HelloAnswer, MuleInfo / MuleInfoAnswer, userhash, ClientID, HighID / LowID, callbacks, capability negotiation, compression, multipacket, search, Source Exchange, publish-as-source, upload/download, large files, unsupported-extension handling.

Kad2: bootstrap, routing table, node ID, UDP, HELLO / HELLO_RES, PING / PONG, FIND_NODE, keyword search, source search, publish, firewall check, Buddy, NAT traversal, HighID / LowID / firewalled.

Do not advertise an eMule capability that Envy does not implement.

### P0/P1 — Real RSA SecureIdent

Only after a reliable ED2K baseline without SecureIdent. **Not implemented today** (`ED2K_VERSION_SECUREID = 0`, #75). Primary reference: eMule Community; secondary: aMule. Details in the SecureIdent section below.

### P1 — IPv6 / reachability

References: eMule AI, eMule Qt, eMule eSE, aria2-next where relevant. Start with a clean dual-stack architecture (address type, sockets, DNS A/AAAA, connect/listen, source exchange, host cache, bans, dedup, UI/logging, UPnP / NAT-PMP / PCP, CGNAT). **Do not start Kad6** before this foundation. See `docs/ipv6/PLAN.md`.

### P1 — Incremental core / UI separation

Inspired by eMule Qt, aMule, and aria2-next. Long-term shape:

```text
EnvyCore (protocols / transfers / library-search)
    → Platform Abstraction
    → Windows | Linux | macOS
         ↳ MFC frontend (Windows, retained)
EnvyCore → Local API / headless → WebUI / CLI / future GUI
```

Incremental extraction only. No full rewrite. No Qt (or other GUI toolkit) introduction in this track. First multiplatform goal is core/headless, not a new desktop GUI. Tracked with #91 (test seam) and #161 (EnvyCore/headless); portability sequencing in `docs/20_arch/PORTABILITY_PLAN.md`.

### P1 — Cross-platform foundations (docs + sequencing)

Formalize platforms (Windows x64 primary; Win32 legacy Stage A; Linux/macOS **planned**), CMake portable-slice priority (D-015), and future CI order: Windows x64 → portable tests → Linux x86_64 → macOS ARM64. Do not add required non-Windows CI until portable code exists. Do not remove Win32 without Stage B/C evidence (D-014).

### P1 — Skin engine HiDPI / modern display (after P0)

Do not mix with metric validation P0. Remaining backlog from the 2026-09 skin display audit:

- Logical 96-DPI XML units → per-window `MulDiv(..., dpi, 96)` for fonts, frames, anchors, regions
- Multi-monitor maximize using the window's `MONITORINFO.rcWork` (not primary-only)
- Black mask `000000` vs absent mask; restore or reject `LVSIL_MID` 24px; command-image index bounds
- Dialog skinning by control ID; real PNG alpha; transactional skin load / rollback
- Mark Slim / Win7–8 / Vista skins Legacy until the DPI layer exists

### P1 — Headless / API

Native versioned REST `/api/v1` (D-017/D-018, 2026-09-19 audit). HTML Remote is not that API. First *arr on-ramp is a **qBittorrent Web API v2 subset**, after transfer services exist — never labelled “qBittorrent-compatible” until tests pass. Torznab is a separate **client**. Tracked: #161 plus [#239](https://github.com/Mika3578/Envy/issues/239) native `/api/v1`, [#240](https://github.com/Mika3578/Envy/issues/240) qBit subset, [#241](https://github.com/Mika3578/Envy/issues/241) Torznab. Today: MFC GUI plus limited Remote web UI.

### P1/P2 — BitTorrent modernization

Preserve the existing BitTorrent track. Continue v1, BEP 10, DHT, PEX, UDP tracker, uTP, magnet, BEP 52 / v2, hybrid torrents. Never sacrifice BitTorrent to make Envy an eMule-only client.

### P2 — DHT / security research

References: Ember, Rucio, original Kademlia papers. Possible studies: stronger node verification, anti-amplification, routing-table diversity, subnet limits, signed records, modern crypto identities, BLAKE3 for **Envy-specific** features. Must not break Kad2/eMule compatibility. Any Envy-only extension must be versioned, optional, backward compatible, and distinct from Kad2.

### P3 — Experimental Kad6 / next overlay

Reference: eMule eSE. Not a short-term goal. Preconditions: stable Envy IPv6, stable Kad2 interop, clean NAT/reachability, and a protocol test harness.

### Phase 1 — Stability & Visibility (P0)
- [ ] Create dependency register + ownership map (2d) — **In progress on develop** (`docs/DEPENDENCIES.md` exists but remains an incomplete seed).
- [x] Add threat model and secure-coding checklist (2d)
- [ ] Expand tests for protocol parser/state-machine paths (5d)
- [ ] Establish baseline metrics (startup, memory, throughput) (3d) — tracked as [#111](https://github.com/Mika3578/Envy/issues/111)

### Runtime performance (audit 2026-09-10)

Measure before optimizing. Recommended order:

1. [#111](https://github.com/Mika3578/Envy/issues/111) — reproducible runtime benchmarks (P0)
2. [#112](https://github.com/Mika3578/Envy/issues/112) — `CBuffer` amortized front consume (P1)
3. [#113](https://github.com/Mika3578/Envy/issues/113) — network hot-path copies / lock hold times (P1; stability dependency [#92](https://github.com/Mika3578/Envy/issues/92))
4. [#114](https://github.com/Mika3578/Envy/issues/114) — global `TransferFiles` I/O lock contention (P1)
5. Hashing throughput — only if #111 measurements justify a dedicated issue; keep protocol hashes unchanged
6. IOCP — deferred until scalability evidence after Buffer/lock work; sockets are already non-blocking

Do not reuse [#102](https://github.com/Mika3578/Envy/issues/102) (CI runner latency) for `Envy.exe` runtime performance.

### Phase 2 — Build/Quality Convergence (P1)
- [x] Define CMake migration boundary and milestones — **clarified 2026-09-18**: full-app CMake low priority; portable `EnvyCore`/HashLib/tests/headless CMake is foundational (D-015 / `PORTABILITY_PLAN.md`). Implementation still open under #91/#161.
- [ ] Reduce duplicated CI workflow logic (2d)
- [ ] Promote selected static-analysis checks to required gates (2d)
- [ ] Bootstrap non-Windows CI only after portable tests exist (Linux x86_64, then macOS ARM64); keep jobs advisory until green

### Phase 3 — Architecture Hardening (P2)
- [ ] Isolate core transfer engine interfaces from UI classes (10d) — same intent as **P1 core/UI separation** above; incremental only (#161)
- [ ] Introduce portable platform abstractions (sockets/DNS/FS/threads/RNG) behind EnvyCore — no mass `#ifdef` rewrite
- [ ] Version plugin-facing APIs and compatibility policy (5d)
- [ ] Create automated dependency/SBOM release artifact (3d)
- [ ] Evaluate Win32 Stage B/C only after multi-compiler 64-bit + non-Windows test evidence (D-014)

## ED2K / eMule SecureIdent roadmap

**Important:** SecureIdent is an authentication/trust feature. It must not be
confused with ED2K connectivity or Kad. Short-term goal: Envy remains visible,
can communicate, search, exchange sources, and transfer files with compatible
ED2K clients even when SecureIdent is unavailable. RSA SecureIdent comes later
to improve authentication and eMule credit-system compatibility.

### Current (#75 — safe disable)
- [x] Stop advertising SecureIdent (`ED2K_VERSION_SECUREID = 0`).
- [x] Never accept MD5/non-zero/legacy responses as verified.
- [x] Ignore inbound SecureIdent packets without dropping ED2K connections.
- [x] Keep ED2K transfer independent of SecureIdent.

### Current (#87 — honest Hello advertising + LowID callback baseline)
- [x] AICH: local hash support may exist; **do not advertise** AICH FeatureVersions
  until C2C request/answer handlers are implemented (`Ed2kAichAdvertisedVersion() = 0`).
- [x] CryptLayer Hello bits (SUPPORTS/REQUESTS/REQUIRES) stay **0** until TCP
  protocol-obfuscation interop with eMule/aMule is proven; packet PUBLICKEY
  crypto is not treated as equivalent to MiscOptions2 crypt bits.
- [x] CryptLayer Hello bit alignment (separate PR after obfuscation audit — #121).
- [x] Phase-1 PUBLICIP_REQ/ANSWER + C2C CALLBACK baseline (`Ed2kLowIdCallback.h`,
  `CEDClient` handlers). Classic server callback unchanged. **Partial** only —
  not Buddy / REASKCALLBACKTCP / FWCHECK / complete LowID.
- [ ] REASKCALLBACKTCP + Buddy bond (phase 2).
- [ ] BUDDYPING / BUDDYPONG / FWCHECKUDPREQ (phase 2 / Kad).
- [ ] Live Hello / LowID capture vs eMule Community / aMule (#160).

### Current (#124 — EnableKad settings binding)
- [x] Register `eDonkey.EnableKad` with `Settings.Add` (default `true`) so
  `InitKademlia()` can run; distinct from `EnableKadHello`.
- [ ] UI checkbox for EnableKad (optional follow-up; registry/settings dump works).
- [x] Kad search/source hits → `AddSourceED2K` (HighID SEARCH_RES delivery + tests; keyword excluded; buddy/callback types deferred; outbound TagList framing still open — not full #86).
- [x] App-trigger `SearchSource` from ED2K download source acquisition (`DownloadWithSearch::MaybeSearchKadSources`); outbound SEARCH_SOURCE_REQ includes FileSize.
- [x] Kad TCP firewall-detection baseline (`FIREWALLED_REQ`/`RES`, bounded checks, public-IP consensus, TCP ACK count). UDP firewall verification, Buddy and callback remain incomplete (#86 remainder).
- [x] Kad routing-table maintenance (zone split, LRU/type liveness, bounded replacement, stale-zone FIND_NODE refresh, /24 diversity + tests).
- [x] Local `nodes.dat` v1/v2/v3 parser (`KadNodesDat.h`); v3 bootstrap edition bounded; UDP-key/`verified` bytes discarded. Remote HTTP `nodes.dat` still open.
- [ ] Kad UDP firewall tester / Buddy / callback / UDP-key runtime (#86 remainder).

### Future RSA SecureIdent (separate workstream)
1. Baseline ED2K interoperability with eMule/aMule without SecureIdent.
2. Confirm Envy works correctly when SecureIdent is unsupported/unavailable.
3. Implement real eMule SecureIdent (not the removed MD5 stub).
4. Remote public-key handling (`ED2K_C2C_PUBLICKEY` / peer key store).
5. Local private-key generation/persistence if required by the protocol.
6. Protocol-correct SecureIdent challenge (`ED2K_C2C_SECIDENTSTATE`).
7. RSA signature generation (`ED2K_C2C_SIGNATURE`).
8. RSA signature verification against the peer public key.
9. Correct binding of identity / userhash / challenge / peer context.
10. Explicit SecureIdent state machine: unsupported, unavailable/incomplete,
    unverified, verified, failed.
11. Rejection tests: invalid signature, wrong key, wrong challenge, replay,
    response from another client.
12. Live interop tests: Envy ↔ eMule, Envy ↔ aMule, ideally eMule ↔ Envy ↔ aMule.

Protocol references for that future work: specifications first (eDonkey
paper, aMule ED2K wiki, ED2K URI spec), then eMule Community as de-facto
wire reference and aMule as second interop target. See
`docs/30_protocols/REFERENCE_IMPLEMENTATIONS.md`. Adapt behaviour; do not
copy blindly.

### Future ED2K interoperability checklist (document only)
Hello/HelloAnswer, MuleInfo/MuleInfoAnswer, userhash, ClientID, HighID/LowID,
LowID callback, advertised capabilities/extensions, extended protocol,
compression, multipacket, search, source exchange, publish-as-source,
upload/download, large files, and clean handling of unsupported extensions.
**Rule:** Envy must advertise an eMule capability only when it is actually
implemented and sufficiently tested.

### Future Kad interoperability checklist (document only — out of #75 scope)
Bootstrap, routing table, Node ID, UDP, HELLO/HELLO_RES, PING/PONG, FIND_NODE,
keyword search, source search, publish, firewall check, UDP firewall, Buddy,
NAT traversal, Kad versions, HighID/LowID/firewalled behavior, and
eMule/aMule interop. No Kad code changes in the SecureIdent safe-disable work.

## Backlog
- [ ] Replace unsafe string operations in first-party code (incremental: bounded keyword copy in legacy Kad publish packet builder completed)
- Consolidate duplicate roadmap/status markdown into canonical set
- [x] Document remote API implementation status endpoint-by-endpoint — 2026-09-19 audit (`docs/20_arch/AUDIT_REMOTE_API_2026-09.md`); OpenAPI remains `planned` until routes are served
- Add long-running memory/regression test scenario
- Archive legacy `.vcproj` files once migration is complete

## Decisions Log
- **2026-09-19:** Remote automation (D-017, D-018): native REST `/api/v1` as source of truth; inbound API on Windows HTTP Server API (`http.sys`) dedicated port, not `CRemote`/P2P HTTP; outbound Torznab via `CHttpRequest`; first *arr adapter is a qBittorrent Web API v2 **subset**. Details: `docs/20_arch/AUDIT_REMOTE_API_2026-09.md`.
- **2026-09-19 / #354:** Hybrid crash reporting — **x64** uses BugSplat SDK v8.0.0 (`ThirdParty/BugSplat`, `/MT` static lib + out-of-process monitor); **Win32** keeps Crashpad (vcpkg, local DB, upload off). Not Sentry Native, Breakpad, CrashRpt, or homemade `MiniDumpWriteDump`. Canonical: `docs/10_dev/crash-reporting.md` (D-019, D-020).
- **2026-09-18:** Adopt cross-platform foundations (D-012…D-015): EnvyCore + platform abstraction + retained MFC Windows frontend; Linux/macOS `planned` not `supported`; Win32 legacy Stage A only; CMake portable slice prioritized over full-app CMake. Canonical doc: `docs/20_arch/PORTABILITY_PLAN.md`. Open question #1 resolved toward multi-OS **as a long-term target**, with Windows-first delivery.
- **2026-09-17:** #140 wording: MiniUPnPc 2.0 targeted discovery is one receive phase for requested ST values, not a strict wall-clock SSDP deadline; absolute SSDP and HTTP timeout bounding → [#142](https://github.com/Mika3578/Envy/issues/142) / P3.
- **2026-09-18:** #166 / D-009 P1 delivered: WFAS `INetFwPolicy2` replaces `INetFwMgr`; application rules on all profiles; UPnP via rule-group enable. Next: P2 LocalPort/ExternalPort.
- **2026-09-17:** #140 P0 runtime PASS (post-squash HEAD): targeted discovery + gateway-only rootdevice fallback; non-IGD devices never receive WAN mapping commands. Remaining MiniUPnPc 2.0 SSDP/HTTP latency → [#142](https://github.com/Mika3578/Envy/issues/142) / P3. Separate backlog: bind/listen must not wait for NAT completion → [#141](https://github.com/Mika3578/Envy/issues/141). Order remains P1 WFAS → P2 ports → P3 MiniUPnPc 2.3.x.
- **2026-09-17:** SSDP success != usable IGD. #140 uses targeted `upnpDiscoverDevices` with `searchalltypes=1` (one discovery receive phase), filters to explicit IGD/WAN ST, and limits rootdevice fallback to the selected gateway IP with exact LOCATION string dedupe. `UPNP_GetValidIGD` runs once on filtered candidates; results `0`/`3` never issue WAN commands.
- **2026-09-16:** NAT/Firewall recovery is phased PRs (not a monolith): P0 modern interface selection (#140), P1 Windows Firewall WFAS (`INetFwPolicy2`), P2 LocalPort/ExternalPort model (behavior-preserving first), P3 MiniUPnPc 2.3.x **vendored** update (no vcpkg introduction for this alone), P4 NatTraversalManager + PCP/NAT-PMP, P5 CGNAT/diagnostics. Do not declare inbound reachability fixed without runtime evidence on a non-CGNAT path.
- **2026-09-15:** Keep C++ RTTI disabled project-wide (`RuntimeTypeInfo=false` / `/GR-`). Do not enable `/GR` to make `dynamic_cast` work. Protocol-specific actions belong on the transfer/neighbour classes via `virtual`/`override`. Localized `static_cast` is acceptable only when the construction path proves the concrete type (first case: `PROTOCOL_ED2K` uploads are always `CUploadTransferED2K` from `CEDClient::OnQueueRequest`). The protocol enum remains valid for UI, stats, logs, filtering, and serialization.
- **2026-09-11:** Persist `eDonkey.EnableKad` via `Settings.Add` (#124) separately
  from Kad routing-table work (#86) and from Kad→`AddSourceED2K` source delivery.
  Default remains `true` per settings reference; no migration (key never existed).
- **2026-09-11:** For #87 Hello honesty, stop advertising AICH until C2C
  handlers exist. Keep CryptLayer MiscOptions2 bits at 0 until TCP
  obfuscation interop is audited separately from packet PUBLICKEY crypto.
  SecureIdent advertisement remains 0 (#75).
- **2026-09-11:** Adopt an explicit reference-implementation policy (D-008): specifications first; eMule Community/aMule as ED2K/Kad de-facto interop; eMule Qt/aria2-next/eMule AI as architecture; Ember/eSE as experimental only. Envy stays multi-network.
- **2026-09-10:** Runtime performance work starts with reproducible benchmarks (#111). No IOCP rewrite issue until peer-scalability evidence after Buffer/lock optimizations. No dedicated LibraryBuilder hashing issue until measurements show a user-visible bottleneck. #92 stays correctness/stability (lock order); #113 tracks contention separately.
- **2026-09-10:** Adopt a two-speed CI: path-aware PR jobs (`if:`, never
  `paths-ignore` on required workflows), CodeQL C++ `build-mode: none` on PRs
  and manual traced builds on `develop`/schedule, vcpkg files binary cache
  (`x-gha` removed upstream). Required Protect develop check names are listed
  in `.github/settings.yml` (including `PR Gate` as a required CI wait).
- **2026-09-10:** For issue #75, choose safe disable of fake SecureIdent over
  implementing RSA in the same PR. Advertisement stays at version 0 until a
  dedicated RSA SecureIdent workstream lands. ED2K connectivity must not depend
  on SecureIdent.
- **2026-05-27:** Rewrote `develop` into a linear history with no merge commits while preserving the final tree through backup refs; enforce linear history going forward via the active `Protect develop` ruleset (squash-only on `develop`), global GitHub merge settings (no merge commits; squash/rebase enabled), and contributor `git pull --ff-only` hygiene.
- **2026-05-15:** Repository hygiene baseline on `develop` requires explicit branch-state tracking and GitHub label prerequisites (`ci`, `dependencies`) before enforcing CI as mandatory gates.
- **2026-04-22:** Added IPv6 dual-stack Phase 0 scoping inventory and phased rollout plan under `docs/ipv6/`.
- **2026-04-22:** Remote web UI must use cryptographic token generation (`crypto.getRandomValues`) and allowlist-based redirect validation for all client-side navigation paths.
- **2026-04-22:** Keep Visual Studio solution as authoritative full-build path while CMake remains partial.
- **2026-04-22:** Standardize new audit reports under `docs/audit/`.
- **2026-04-22:** Treat this plan as a required living artifact for project management continuity.

## Open Questions
1. ~~Should this project explicitly remain Windows-only, or is cross-platform parity still a target?~~ **Resolved 2026-09-18 (D-012):** long-term multi-OS via EnvyCore; Windows remains the only supported product OS until Linux/macOS meet support criteria in `PORTABILITY_PLAN.md`. Headless/API still proceeds on Windows first (#161).
2. What is the acceptable backward-compatibility policy for legacy protocols/features? Working default: preserve G1/G2/DC/BitTorrent; ED2K/Kad changes must remain eMule/aMule-compatible unless versioned as optional Envy extensions.
3. Which dependency update cadence (monthly/quarterly) is realistic for maintainers?
4. ~~Should remote API documentation be strict contract-first or implementation-first?~~ **Resolved 2026-09-19:** OpenAPI describes **implemented** routes plus explicitly `planned` ones (`x-envy-status`). No fictional served surface. See `docs/api/openapi.yaml`.
5. ~~REST versus JSON-RPC for a future Envy daemon API?~~ **Resolved 2026-09-19 (D-017/D-018):** native **REST** `/api/v1` (OpenAPI). qBittorrent-shaped REST subset for *arr. Transmission JSON-RPC is not the first adapter. aria2 JSON-RPC is a reference only.
6. When is Win32 Stage B (drop from user releases) justified relative to Preview/stable channels?

- **2026-10-01 review finalization:** Explicit disposition of complete Copilot
  overviews is mandatory, including Previously missed and Suppressed findings.
  Governance remains subject to maintainer review and manual squash; PR history
  cleanup preserves trees and budgets with scoped authorization. Review-loop
  assessments use read-only permissions and cannot publish authoritative state.
  Runtime validation and a valid non-author approval remain external checkpoints.
