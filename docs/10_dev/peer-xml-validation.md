# Peer XML validation

Scope: bounded G1/G2 XML parsing and HTTP metadata/THEX receive handling.
Issue #391 and PR #393 own this change. This guide describes validation;
it does not establish that a particular build has passed the live checks.

## Automated checks

Build the full Windows application with `Visual Studio/Envy.sln` using the
normal Release x64/Win32 commands in [build.md](build.md). Run EnvyTests on
each platform. Its peer XML cases exercise budget admission, shared accounting,
HTTP control-response classification and the production receive-loop helper.

After the solution build has generated `Envy/EnvyOM.h`, run the native suite
from a Visual Studio 2026 Developer shell at the repository root:

```powershell
msbuild tests/PeerXmlTests.vcxproj /m /p:Configuration=Release /p:Platform=x64
& 'tests/Release x64/PeerXmlTests.exe'
msbuild tests/PeerXmlTests.vcxproj /m /p:Configuration=Release /p:Platform=Win32
& 'tests/Release Win32/PeerXmlTests.exe'
```

This standalone MFC test target compiles the real `XML.cpp` and `Strings.cpp`.
It is separate from the full application solution and must be run explicitly.
It tests valid XML against the local parser, depth 32/33 and a 4096-level
attack, 4096-node admission, attribute accounting, exact character limits,
shared-budget/depth restoration, malformed input, UTF-8/UTF-16 BOM handling,
and the production HTTP receive-loop helper on a real loopback socket after
remote shutdown.

The THEX cases compile unchanged `ReadTiger` and `StartNextFragment` method
bodies extracted at build time from the current `DownloadTransferHTTP.cpp`,
and `SetTigerTree` from `DownloadWithTiger.cpp`,
plus the production `CBuffer` DIME/buffer methods. The XML implementation is
linked normally. Missing method boundaries fail the test build; no maintained
copy of the parser is used. The real HashLib DLL and hash string conversion
are linked; tree integrity, known-root comparison and unknown-root adoption
execute production code. Download scheduling, logging and other connection
services remain test doubles. Installation uses a real critical section.

These cases compare a complete response with every possible two-read split
(including descriptor/tree boundary, partial record header, ID/type, binary
body and padding), check mismatched identity and reset before a second
response, fail closed on truncated/malformed records, reject oversized XML
and padding overflow from headers alone, and drain an invalid descriptor's
remaining response. They exercise the production parser, not just policy
helpers. They also exercise URI absent/empty, empty/matching/mismatched IDs,
both historical depth conventions, invalid decimal attributes, wrong segment
size, partial/extra hashes, corrupted internal hashes, published 1024/1025-byte
TTH vectors, bare trees, duplicate records, MB/ME/CF flags and bytewise reads.
They do not instantiate the complete application transfer, execute
`OnDropped` or `OnHeaderLine`/`OnHeadersComplete`, or exercise session/profile
ownership. EnvyTests' admission/framing cases are helper tests; they do not
execute the complete G1/G2 dispatchers or HTTP header parser.
The ten THEX cases bring this native suite to 18 tests. Python 3 must be on
PATH for the source-extraction build step.

## Loopback fixture checks (not application tests)

The fixture listens only on `127.0.0.1`. Envy rejects loopback download sources:
`CDownloadWithSources::AddSourceURL` checks `Network.IsReserved`, which always
classifies IPv4 `127/8` as reserved. Adding these URLs in Envy does not exercise
the HTTP transfer parser. Changing `Connection.IgnoreOwnIP` or private-address
filtering does not remove this reserved-address check. Do not disable or bypass
network protections to make the fixture work in the application.

Run the fixture's independent HTTP-client self-test:

```powershell
python tests/peer_http_fixture.py --self-test
```

Its 21 response checks validate the fixture, not Envy. To inspect the responses
with a separate HTTP client, start the loopback-only server:

```powershell
python tests/peer_http_fixture.py
```

The fixture prints expected SHA256 hashes; an independent client can verify
downloaded bodies with `Get-FileHash -Algorithm SHA256 <file>`.
The tables below describe intended Envy behavior if equivalent responses reach
an admitted source. They are test vectors, not executable loopback application
instructions or evidence that application header handling has passed.

| URL suffix under `http://127.0.0.1:18793` | Expected observation |
| --- | --- |
| `/file-known.bin` | 65,536-byte download completes with the printed normal-file SHA256. |
| `/file-close.bin` | Same bytes/hash without Content-Length; no completion at a 16 KiB prefix. |
| `/file-empty.bin` | Explicit zero Content-Length is rejected before changing an unknown download size; no empty file is reported complete. |
| `/file-metadata-valid.bin` | Log shows `/metadata-valid` requested; valid bounded XML does not crash or stall the transfer. |
| `/file-metadata-close.bin` | Close-delimited metadata completes after shutdown. |
| `/file-metadata-deep.bin` | Depth-33 metadata is discarded; the application remains responsive. |
| `/file-metadata-large.bin` | Oversized metadata is rejected without unbounded buffering. |
| `/file-thex-single.bin` | Log shows `/thex-single`; the descriptor and matching nonempty-ID tree arrive together. |
| `/file-thex-split.bin` | Same tree/download result when the descriptor and a partial tree record arrive separately. |
| `/file-thex-short.bin` | Truncated known-length THEX fails closed; no successful tree installation is reported. |

THEX fixtures use the specification's 1024-byte `A` test vector and a padded
XML descriptor larger than the header receive cap. Confirm the THEX request
in the server log: a file download alone does not demonstrate tree handling.
The fixture emits HTTP responses and does not emulate a Gnutella client.
Stop it with Ctrl+C. No Internet-facing listener is created.

Additional `/file-thex-<case>.bin` cases:

| Case | Expected tree observation (independent of eventual file completion) |
| --- | --- |
| `empty-id`, `depth-zero`, `uri-empty`, `uri-missing` | Tree installed; URI omission/emptiness is accepted only against the advertised, already known root. |
| `unknown-uri-empty`, `wrong-id`, `no-descriptor`, `invalid-xml` | No tree installed. File transfer may continue after semantic rejection. |
| `bad-segment`, `bad-depth`, `partial-hash`, `wrong-root` | No tree installed; inconsistent tree/descriptor rejected. |
| `multiple` | First matching valid tree installed once; duplicate cannot replace it. |
| `close`, `bare` | Close-delimited DIME / bare breadth-first tree installs the expected root. |
| `chunked`, `gzip`, `bad-length` | HTTP transfer rejected; header paths require application testing. |

`short` sends one byte less than Content-Length; `split` schedules separate
writes around the descriptor/tree boundary. TCP does not guarantee matching
receive boundaries. The native suite explicitly tests every two-read split
and bytewise reads of the production parser. Malformed fixture inputs stay local.

## Actual application checks

Record the PR HEAD, base, built commit, artifact/run, platform and results.
Use an independent reference client on another controlled machine, with an
address admitted by Envy's existing network policy, as described below.
Keep the normal network protections enabled. Enable download metadata and
Tiger verification for their respective cases. Remove the previous test
download before repeating a case, so cached ranges or hash trees cannot mask
the request. Check the completed file's SHA256 and actual THEX exchange/logs.

Distinguish the evidence: fixture self-tests check generated HTTP responses;
native tests execute production parser methods with service doubles; only an
Envy transfer from an admitted peer tests application integration. Native tests
do not cover the complete HTTP header/socket lifecycle or G1/G2 dispatch.
Malformed application cases require a separately controlled, admitted source;
the supplied loopback-only server cannot provide that application coverage.
Mark those cases not exercised unless such evidence is recorded.

## Association policy and reference comparison

The identifier schemes remain unchanged. Receive acceptance is intentionally
tighter, so this is a wire-acceptance change, not a new wire format.

| Descriptor state | Matching ID | Empty ID | Different nonempty ID |
| --- | --- | --- | --- |
| Valid, nonempty URI | Accept | Accept (Shareaza compatibility) | Skip this tree |
| Valid, empty URI | Only with a known TTH | Only with a known TTH | Only with a known TTH |
| Valid, URI absent | Only with a known TTH | Only with a known TTH | Only with a known TTH |
| Absent or invalid | Never install | Never install | Never install |

Each acceptance also requires complete binary rows, descriptor/file agreement
and successful `SetTigerTree`. Empty ID with valid nonempty URI preserves the
historical adoption path when no root is known. Adopting a root establishes
internal consistency, not authenticity: even a previously announced root is
not independent trust when it came from the same peer. `SetTigerTree` compares
known roots and clears mismatches. It retains at most `Library.TigerHeight`;
HashLib checks the retained prefix's internal hashes, not discarded deeper
levels. Row/byte validation prevents partial hashes and structurally extra
records from being silently accepted as that prefix.

Only the first XML descriptor (MB record) is valid for association. A matching
second tree cannot replace the installed tree; unrelated IDs are skipped.
An appended ED2K MD4 hashset inside the same message remains allowed. Response
state is reset by `StartNextFragment`. CF, repeated MB, premature/missing ME
and impossible record lengths fail closed before tree installation where the
offending header precedes that tree. Installation is per record, not a
transaction over the whole response: later truncation cannot undo a valid
tree installed before an appended MD4 record. No claim of whole-message
atomicity is made.

Source snapshots inspected (not proof that any released executable was run):

- Shareaza [ansani `f24b0ce`](https://github.com/ansani/Shareaza/blob/f24b0ce3753067e02fb6ee7dbf82d8c214c291d2/shareaza/DownloadTransferHTTP.cpp)
  and [ivan386 `0c2f2f5`](https://github.com/ivan386/Shareaza/blob/0c2f2f56c8294c930f26463a522057d00ed319fa/shareaza/DownloadTransferHTTP.cpp):
  ID match or empty ID; one-based upload depth. Their local descriptor identity
  does not survive separate `ReadTiger` calls; Envy's response state does.
- [gtk-gnutella `3ce50bf`, reader](https://github.com/gtk-gnutella/gtk-gnutella/blob/3ce50bf0dd25e88e0d470eda61a0283a23ed49c6/src/core/thex_download.c),
  [writer](https://github.com/gtk-gnutella/gtk-gnutella/blob/3ce50bf0dd25e88e0d470eda61a0283a23ed49c6/src/core/thex_upload.c),
  [DIME](https://github.com/gtk-gnutella/gtk-gnutella/blob/3ce50bf0dd25e88e0d470eda61a0283a23ed49c6/src/core/dime.c)
  and [depth calculation](https://github.com/gtk-gnutella/gtk-gnutella/blob/3ce50bf0dd25e88e0d470eda61a0283a23ed49c6/src/lib/tigertree.c):
  zero-based depth, mandatory expected TTH, first matching identity then
  fallback by type for BearShare 5.2, plus malformed XML fallback for GnucDNA
  1.1.1.4. Envy does not add those broad fallbacks: malformed descriptors and
  mismatched nonempty IDs with a nonempty URI remain rejected/skipped.
- [LimeWire `a123277`, reader](https://github.com/metapirate/LimeWire-Pirate-Edition/blob/a12327753c0a72b815b5e828dd1be9f39b2b7fc9/components/gnutella-core/src/main/java/com/limegroup/gnutella/tigertree/dime/TigerDimeReadUtils.java)
  and [writer](https://github.com/metapirate/LimeWire-Pirate-Edition/blob/a12327753c0a72b815b5e828dd1be9f39b2b7fc9/components/gnutella-core/src/main/java/com/limegroup/gnutella/tigertree/dime/TigerDimeWriteHandler.java):
  no explicit UUID comparison, expected root and reconstructed hashes checked;
  `HashTreeImpl` uses `allNodes.size()-1` for its exported depth.
- [HERMES WireShare `db7169a`](https://github.com/nmatavka/hermes-wireshare/blob/db7169a497d15530d3c3b130380d176508ea54f0/main/components/gnutella-core/src/main/java/com/limegroup/gnutella/tigertree/dime/TigerDimeReadUtils.java):
  inspected reader/writer preserve the LimeWire behavior.

All inspected writers send segment size 1024, Tiger output size 24 and the
unchanged serialization identifier. Binary rows are admitted for either
one-based or zero-based declared depth; the file length determines the actual
node counts. Size/segment/depth/Content-Length decimals reject signs, junk and
overflow. Conflicting Content-Length headers and chunked THEX are rejected.
Compression stays unsupported. Historic missing descriptors, malformed XML
and unusually advertised depths may lose THEX acceleration; live testing must
establish whether a real supported client sends these forms before widening
acceptance. Normal file-transfer fallback does not prove THEX compatibility.

## Peer interoperability checks

With a second Envy or Shareaza-lineage client, verify G1/G2 search results with
metadata, Browse Host folders/results, a valid profile, and private chat.
Repeat search/browse after a rejected malformed profile or metadata payload;
the session must remain usable and the application must remain responsive.
Use a controlled peer fixture for malicious cases; ordinary successful
connections do not establish that the rejection paths ran. Record client
versions and which G1/G2 paths were actually observed. Do not claim universal
client compatibility from one reference-client pass.

### Manual Visual Studio build (maintainer)

1. Record `git branch --show-current` and `git rev-parse HEAD`. Open
   `Visual Studio/Envy.sln` in Visual Studio 2026 with MFC/ATL and v145.
2. In the solution toolbar select **Release**, **x64**. Use **Build > Build
   Solution** (Ctrl+Shift+B). In **View > Output**, select **Build** and retain
   the final summary and the first actual error if it fails. The authoritative
   target is the solution; successful standalone tests do not prove this build.
3. Repeat with **Release**, **Win32**. Its manifest requires
   `x86-windows-static`; see [build.md](build.md) for dependency setup. Do not
   remove Crashpad/BugSplat prerequisites or change gates to get a pass.
4. Execute `Envy/Release x64/Envy.exe` and then the Win32 counterpart from its
   complete build output, with sibling runtime files. Confirm the launched
   executable's path locally and **Help > About Envy** version. Keep logs per
   architecture. Do not silently run an older installed executable.
5. In a VS 2026 Developer PowerShell at the repository root, run:

```powershell
msbuild tests/EnvyTests.vcxproj /m /p:Configuration=Release /p:Platform=x64
& 'tests/Release x64/EnvyTests.exe'
msbuild tests/PeerXmlTests.vcxproj /m /p:Configuration=Release /p:Platform=x64
& 'tests/Release x64/PeerXmlTests.exe'
msbuild tests/EnvyTests.vcxproj /m /p:Configuration=Release /p:Platform=Win32
& 'tests/Release Win32/EnvyTests.exe'
msbuild tests/PeerXmlTests.vcxproj /m /p:Configuration=Release /p:Platform=Win32
& 'tests/Release Win32/PeerXmlTests.exe'
```

Require a successful build before each run and zero failed tests. The expected
current counts are 735 EnvyTests and 18 PeerXmlTests per platform. Python must
be available for extraction. The solution generates the COM header needed by
PeerXmlTests; it is gitignored. Report errors rather than using stale binaries.
Agent validation here builds these test projects/HashLib only, not the full app.

### Reference clients and topology

Availability checked on 2026-10-11; record the actually installed version and
platform, since source snapshots and binary releases can differ.

| Client | Available baseline | Protocol plan |
| --- | --- | --- |
| Shareaza | [2.7.10.2 official Windows x64/Win32 installers](https://sourceforge.net/projects/shareaza/files/Shareaza/Shareaza-2.7.10.2/) | First choice: separate G1 and G2 sessions; THEX/DIME in both directions. |
| gtk-gnutella | [1.3.1 official downloads](https://gtk-gnutella.sourceforge.io/?lang=en&page=download), Windows installer or Linux package/source; macOS has no current official DMG | G1, then G2 **leaf** attached to an Envy/Shareaza hub. [Official G2 scope](https://gtk-gnutella.sourceforge.io/?lang=en&page=faq). |
| HERMES WireShare | [7.0 release JAR/source and SHA256SUMS](https://github.com/nmatavka/hermes-wireshare/releases/tag/release/7.0) | Optional G1/THEX. Source confirms the reader; launch requirements/usability of this asset have not been tested here. Record JVM/OS and a successful start before including it. No G2 capability established. |
| Second Envy | Same tested build or explicitly recorded older build | Useful controlled G1/G2, profile/chat and upload reference; not independent-client evidence. |

No client was installed or run during the source comparison. Do not substitute
unverified legacy download sites. For HERMES, inspect its shipped instructions
and Java requirement before launching; if the JAR cannot start, mark the row
blocked and proceed with Shareaza/gtk-gnutella. The historical LimeWire source
is a comparison, not a recommendation to install an abandoned binary.

Use two disposable machines/VMs on a controlled network. Share **only** the
synthetic directory. Give each a fixed, distinct listening port (example:
Envy 18791, reference 18792). Use an endpoint admitted by the existing network
policy; loopback and self-address rejection are not parser failures. If the
chosen topology is filtered, record the case as not exercised and use a
permitted topology without weakening network protections.

1. In Envy choose **View > Windowed Mode** if needed to expose the full menus
   (skin names can vary; the default XML defines the commands below).
2. **Tools > Envy Settings... > Connection**: clear **Random**, set **Port**,
   and use the permitted test interface. Keep **Ignore private/local IP
   addresses**, self-address filtering and firewall protections at their normal
   settings; keep personal shares out of the test. Verify TCP listening
   with `Get-NetTCPConnection -State Listen -LocalPort 18791`. UDP is used by
   Gnutella queries; verify the existing permitted UDP path too. No public router mapping
   is needed on one LAN. Router/NAT behavior is a separate test.
3. **G2/Gnutella** settings: choose **Connect to Gnutella now** or **Connect
   to G2 now**, one network at a time. G1 modes are **Optimal**, **Leaf**,
   **Ultrapeer**; G2 modes **Optimal**, **Leaf**, **Hub**. A leaf-to-leaf
   handshake rejection is not a parsing regression. Use one ultrapeer/hub
   and one leaf; gtk-gnutella cannot serve as a G2 hub.
4. Open **View > Neighbors** (Ctrl+F11 in Windowed mode). Its real columns are
   **Address**, **Port**, **Mode**, **Client**, **Traffic**, **Packets**. Modes
   include **G1 Leaf**, **G1 Ultrapeer**, **G1 Peer**, **G2 Hub**, **G2 Leaf**.
5. Use **Network > Connect To...** (Ctrl+T), enter the other test endpoint and
   select **Preferred Network Protocol** explicitly. Runtime protocol choices
   are populated by `DlgConnectTo.cpp`, not inferred from the address.
6. Confirm the expected Mode/Client on both endpoints. Disconnect that link,
   initiate from the reference client instead, and verify the TCP SYN direction
   in the capture. For an incoming connection Envy's local port is its listening
   port; for an outgoing one the reference's remote port is its listener.
   `Get-NetTCPConnection -State Established` exposes LocalPort/RemotePort.
   Tooltip **Inbound/Outbound** labels measure traffic in both directions and
   do not identify which side initiated the connection.
   Count each direction independently. If auto-connections obscure this test,
   disconnect unrelated neighbors and retain only the controlled link.
7. Open **System** (F11), right-click **Verbose Mode > Debug**, enable
   **Timestamp**. Keep the requesting/receiving/tree-ready messages locally.

UI evidence: `Envy/Res/Default.xml` (menus and log verbosity), `Envy/Envy.rc`
(`IDD_SETTINGS_CONNECTION`, `IDD_SETTINGS_GNUTELLA`, `IDD_CONNECT_TO`),
`WndNeighbours.cpp` (columns/modes), `CtrlDownloads.cpp` (download columns).
Do not infer G2 from a successful HTTP transfer: the transfer protocol is
shared across G1/G2 and direct HTTP sources.

### Synthetic sharing, search and download cycle

Generate reproducible samples in a new, empty local test directory:

```powershell
python tests/peer_http_fixture.py --write-samples .local/peer-interop-samples
Get-FileHash .local/peer-interop-samples/*.bin -Algorithm SHA256
```

The generator refuses overwrite. Keep its SHA256 results. The 1024-byte file
is all `A`, with published TTH `L66Q4YVNAFWVS23X2HJIRA5ZJ7WXR3F26RSASFA`.
The 64-MiB file repeats bytes 0..255 and permits resume/segmentation testing.
Keep a reference copy outside download/incomplete folders. SHA256 compares
actual file bytes; it is distinct from TTH advertised by the P2P client.

For **each client and protocol row** above, execute this cycle:

1. Share only those files in the reference client's library; wait for hashing
   to finish. In Envy the counterpart is **Tools > Share Files...** / Library
   **Shared Folders...**. Record advertised SHA1/TTH from a file link or file
   properties, rather than guessing that indexing has completed.
2. Ensure only the selected G1 or G2 test connection is active. Use
   **Network > New Search...** (Ctrl+S), search `envy393-interop-64m`, and
   expand the result's sources. Verify the reference client/endpoint and
   67,108,864-byte size. Search the small vector separately.
3. Result context menu **Get P2P Link (URI)** exposes announced URNs. Compare
   the small file's TTH with the published value; compare the large file's
   announced TTH between both reference copies. Search/browse metadata must
   remain visible when bounded/valid; a raw `.bin` may legitimately have none.
4. Select **Download**. Expand the entry in **Downloads**; watch **Client**,
   **Status**, **Transfer**, **Progress**, **Speed** and the source address.
   In Advanced settings verify `Downloads.VerifyTiger=true` and
   `Downloads.Metadata=true` for the relevant cases. These are setting names,
   not invented dedicated checkboxes in the Downloads page.
5. Capture the first request, before the tree can be cached. Require the
   evidence ladder below. A 1024-byte file may finish too quickly to trigger
   THEX; use the 64-MiB file and throttle the reference upload (e.g. 128 KiB/s).
   If no THEX request occurs, record **not exercised**, not failure/pass.
6. On completion, compare `Get-FileHash '<completed synthetic file>'
   -Algorithm SHA256` with the generator's value. Repeat in the other direction:
   share the generated file in Envy, search/download with the reference client,
   and compare its completed SHA256. Observe Envy's **Uploads** and the
   reference client's tree verification log; an Envy download log cannot prove
   the reference receiver installed Envy's uploaded tree.

### Scenario checklist and failure diagnosis

Apply rows S1-S6 separately to Shareaza/G1, Shareaza/G2, gtk-gnutella/G1,
gtk-gnutella/G2-leaf, and optional HERMES/G1. Mark unsupported profile/chat
capabilities **N/A** rather than claiming protocol support.

| ID / manipulation | Expected result and observation | Regression evidence / diagnostic |
| --- | --- | --- |
| S1 connect both directions | Neighbors Mode/Client correct, incoming and outgoing confirmed; search works both ways | TCP connects but handshake rejects: record roles/protocol/status; no SYN response: listener/firewall issue first |
| S2 normal/reverse download | Correct source, tree evidence when requested, matching final SHA256 in both directions | Wrong known TTH accepted, corrupt final bytes, hang/crash or valid descriptor rejected |
| S3 Pause / Resume | Use download context **Pause**, wait for disconnection, **Resume**; retains completed ranges, final hash matches | Starts from zero every time or stalls; save HTTP Range/Content-Range and source Status, not just percentage |
| S4 interrupted source | During a throttled transfer, stop the reference process; resume it later | UI remains responsive, retries/preserves ranges; repeatable crash or poisoned metadata is failure |
| S5 segmentation / source switch | Put identical 64-MiB sample on two controlled sources; **Advanced > Add Source...** or **Find More Sources**; set **Downloads > Transfers per File** above 1; observe two source entries and distinct HTTP ranges. Stop one source, finish with the other | Verify actual simultaneous ranges; two entries alone do not prove segmentation. Hash mismatch, stalled surviving source, or all ranges lost needs diagnosis |
| S6 browse/profile/chat | Result **Browse User**, **Chat with User**; bounded profile/metadata survives repeated search/browse/chat | G2/profile ownership is separate from HTTP tests; report unsupported client features distinctly. Do not inject hostile profiles into public peers |
| L1 valid/malformed XML | Native XML limit tests; fixture `file-metadata-valid/deep/large/close` checks generated responses only | Application metadata handling remains untested without equivalent responses from an admitted controlled source |
| L2 THEX identifiers | Native production-parser identity matrix: empty/mismatched IDs, URI absent/empty, missing descriptor | Compare against association table; full application installation needs separate admitted-peer evidence |
| L3 THEX framing/hash | Native production-parser split/truncation, descriptor, hash integrity, duplicate, close and bare-tree cases | Parser evidence only; full socket close/retry behavior needs application testing |
| L4 unsupported HTTP | Fixture `chunked`, `gzip`, `bad-length`, plus `file-empty.bin` generates header cases | Fixture success does not prove application rejection; full header/socket paths require an admitted controlled source |

A semantic THEX rejection can permit ordinary file download afterward. Evaluate
tree installation and final file integrity separately. An unavailable peer is
not a compatibility failure until the same endpoint/roles work on a baseline.
Native tests cover malformed XML/parser inputs. Loopback fixture checks prove
response construction only; ordinary reference-client traffic does not prove
that malformed G1/G2 profile dispatch or hostile HTTP headers were exercised.

### Observe the actual protocol

Capture only the controlled interface/endpoints with Wireshark. The loopback
capture adapter observes fixture-client traffic, not an Envy download. Start
before connecting to an admitted reference peer.
Replace example ports with the local test values without publishing addresses.

```text
tcp.port == 18791 || tcp.port == 18792 || udp.port == 18791 || udp.port == 18792
tcp.port == 18793
http.request || http.response
http.request.uri contains "thex" || http.request.uri contains "/uri-res/N2X"
http.content_type contains "dime" || http.content_type contains "tigertree"
tcp contains "X-Thex-URI"
```

Use **Decode As... > HTTP** on the transfer stream if a nonstandard port is
not decoded; **Follow TCP Stream** shows reassembled headers and binary data.
`tcp contains` searches individual segments and can miss a split header; it
is a locator, not exhaustive proof. Gnutella stream compression/TLS may hide
packet contents; do not disable security on public traffic to obtain a trace.
Encrypted transfers need local logs/debugging for this evidence.

Record three distinct levels:

1. **Announced:** a file response contains `X-Thex-URI` (or legacy
   `X-TigerTree-Path`) and TTH URN. This advertises capability only.
2. **Exchanged:** Envy sends GET to that advertised URI and receives a body.
   Envy's uploader uses `/gnutella/thex/v1`; LimeWire-lineage clients can use
   `/uri-res/N2X?...`. Follow the advertised path rather than requiring one
   spelling from every client. Content-Type is `application/dime` (or legacy
   `application/binary`) or bare `application/tigertree-breadthfirst`.
   DIME fixed headers contain lengths/MB/ME; XML `serializedtree uri` and
   binary ID can be compared in stream bytes. Missing ID is legal only under
   the table's receiver policy. Initial G1/G2 handshake and Neighbors Mode
   establish the overlay network; HTTP by itself does not.
3. **Installed:** System Info log contains **Received tiger-tree volume for
   ...**, the level count and minimum verifiable chunk; this message comes
   after production `SetTigerTree` succeeds. **Requesting...** or
   **Receiving...** alone is insufficient. Wrong-root tests must show
   **does not match expected root value**, with no successful installation.
   Internal inconsistency reports **corrupt or internally inconsistent**.

For every failure, retain privately: tested HEAD, configuration, client/JVM/OS
versions, protocol/roles/direction, synthetic sample SHA256 and advertised TTH,
exact fixture case, first build error or relevant timestamped System messages,
HTTP status/framing and whether levels 1/2/3 occurred. Share only a minimal
redacted extract; captures may contain private addresses and filenames.
Restore test-only settings/shares/firewall rules afterward. Live results,
full application builds and maintainer approval remain prerequisites; this
guide and passing isolated tests do not establish merge readiness.

## Protocol evidence

- [THEX draft, §3.3.1](https://adc.sourceforge.io/draft-jchapweske-thex-02.html):
  `http://open-content.net/spec/thex/breadthfirst` is the serialization **type
  identifier**, compared as data. It is not fetched. Replacing its scheme with
  HTTPS changes the identifier and breaks existing peers. The same applies to
  the Tiger digest identifier. Qualify cpp:S5332 on that comparison as a false
  positive with this evidence; do not suppress the rule or rewrite the value.
- The XML `serializedtree type` comparison and the DIME binary record type
  comparison both consume this breadth-first identifier as data. The Tiger
  `digest algorithm` comparison uses the identifier explicitly defined in
  THEX §3.2.3: `http://open-content.net/spec/digest/tiger`. None is dereferenced.
- The native trailing-record test uses
  `http://edonkey2000.com/spec/md4-hashset` as a legacy DIME record type passed
  to `CBuffer::WriteDIME`. This is the MD4 hashset extension, not the Tiger
  algorithm identifier. Both Envy and the inspected
  [Shareaza uploader](https://github.com/ansani/Shareaza/blob/f24b0ce3753067e02fb6ee7dbf82d8c214c291d2/shareaza/UploadTransferHTTP.cpp)
  emit that exact value; their readers compare it as a type. The test performs
  no HTTP fetch and must retain the historical spelling to exercise the
  production trailing-hashset branch. Changing HTTP to HTTPS in any of these
  identifiers would select a different, unsupported type, not secure transport.
- [XML 1.0](https://www.w3.org/TR/REC-xml/): limits are Envy resource policy,
  not new XML syntax. Local `FromString`/`FromFile` behavior stays separate.
- [HTTP/1.1 framing, RFC 9112 §6.3](https://www.rfc-editor.org/rfc/rfc9112.html#section-6.3):
  close-delimited responses end at connection close; known-length responses
  must contain their declared body. The bounded receive path must drain queued
  socket bytes before treating FD_CLOSE as completion.
