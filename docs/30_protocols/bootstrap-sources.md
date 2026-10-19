# Bootstrap sources versus discovered hosts

Status: active
Last updated: 2026-10-15
Scope: Cold-start catalogues shipped with Envy. Not a claim that every protocol is complete.

**Bootstrap sources ≠ discovered peers/servers.** Envy already keeps those layers apart:

| Layer | Files | Role |
| --- | --- | --- |
| A. Shipped catalogue | `Data/DefaultServices.dat`, `Data/DefaultServers.dat` | Minimal sources for a new install or wiped local cache |
| B. Learned runtime data | `Discovery.dat`, `HostCache.dat` | GWC URLs, ED2K servers, G1/G2 hosts, BT DHT nodes, Kad contacts learned while running |
| C. Last-known-good remote catalogue | *planned* | Refresh A independently of the binary, never blocking startup |

The Visual Studio application build stages only `DefaultServices.dat` and
`DefaultServers.dat` into `$(TargetDir)Data` for Debug/Release and x64/Win32.
It does not copy runtime caches or user state. At startup, loaders prefer a
catalogue under `Settings.General.DataPath` and fall back to the packaged
`<executable directory>\Data` copy when the writable copy is absent. Existing
`Discovery.dat`, `HostCache.dat`, and manually added services remain separate
and are not reset to make defaults load.

Do not copy B back into A. Do not compile bootstrap IPs into `.cpp` / `.h`. The old `CDiscoveryServices::AddDefaults()` built-in string list is **commented out**; if both the writable and packaged data files are missing, Envy reports the missing catalogue instead of inventing a second C++ list.

Status vocabulary: catalogue refresh is **implemented** for the shipped files; remote last-known-good is **planned**; local Kad `nodes.dat` v1/v2/v3 parsing is **implemented**; remote validated Kad `nodes.dat` cold-start download is **implemented** (HTTPS `K` rows, size-capped validation, same-directory replace with `.lkg` backup) and does **not** make Kad2 routing/search interop complete.

## Cold-start by protocol

| Network | Status | Cold-start path |
| --- | --- | --- |
| ED2K | implemented (list download) | `DefaultServices.dat` `D` URLs → `server.met` import into HostCache. No static server IPs. |
| Kad2 | partial | Empty `HostCache.Kademlia` → import local `DataPath\nodes.dat` (and optional eMule/aMule `nodes.dat`); if no usable contacts remain, try HTTPS `K` rows in `DefaultServices.dat` (`nodes.dat`, size-capped, validated, atomic replace with `.lkg`). Parser accepts legacy v0 + new-format v1/v2/v3 (`Envy/KadNodesDat.h`). v3 edition 1 contributes at most 50 XOR-closest contacts. `CKademlia::Bootstrap()` still needs live routing; Kad2 remains partial / unverified (#86 / #160). |
| Gnutella 1 | implemented (bootstrap) | gtk-gnutella UHCs (`U uhc:…`) plus multi-network GWCs. |
| Gnutella2 | implemented (bootstrap) | Independent GWCs (jayl.de, bj.ddns.net, 4octets, trillinux). |
| DC NMDC | implemented (hublist) | Four HTTPS hublists. `adc://` / `adcs://` skipped until #163. |
| ADC/ADCS | not implemented | `adc://` / `adcs://` rows are ignored; NMDC rows in the same hublists are still imported. Do not treat import as ADC support. |
| BitTorrent DHT | implemented (bootstrap) | `DefaultServers.dat` `B` DNS routers loaded into HostCache. `CDHT::Connect` inserts cached node IDs; if none exist it sends BEP 5 `find_node` to up to 8 HostCache BitTorrent hosts (blocking DNS capped at 3 names; empty cache reloads `DefaultServers.dat`). No extra C++ DNS list. If the catalogue file is missing or every name fails DNS, DHT still marks connected and waits for later PEX/tracker nodes — D-012 forbids compiling `router.bittorrent.com` (or any replacement) into `.cpp`. Successful nodes persist in `HostCache.dat`. |

### Gnutella 1 cache compatibility

G1 GWebCaches and UDP Host Caches (UHCs) are separate discovery mechanisms.
GWebCache uses HTTP requests to exchange peer addresses and cache URLs; a UHC
answers Gnutella UDP ping/pong traffic and is identified through the UDPHC GGEP
extension. A historical `Discovery.dat` with enough G1 GWebCaches but no UHC
therefore still needs repair. `EnoughServices()` checks the UHC minimum
independently, `Load()` adds the shipped UHC rows and saves the augmented
cache, and the following startup sees that persisted UHC and does not inject
the defaults again. `AddDefaults()` adds missing identities; it does not clear
learned or manually added services.

This distinction matches the [GWebCache HTTP specification](https://www.gnucleus.org/gwebcache/specs.html)
and the [Gnutella UDP Host Cache description](https://www.gnutellaforums.com/attachments/general-gnutella-development-discussion/7007d1589595075-resources-gnutella-developers-udp-host-cache-gnutella-developers.pdf).
The maintained gtk-gnutella source keeps these paths in separate [GWebCache](https://gtk-gnutella.sourceforge.io/doxygen/gwcache_8c.html)
and [UHC](https://gtk-gnutella.sourceforge.io/doxygen/uhc_8c.html) modules,
and its changelog records UHC-list/port maintenance
([source project](https://github.com/gtk-gnutella/gtk-gnutella)).
This catalogue distinction does not claim complete Gnutella 1 or Gnutella2
network support or end-to-end interoperability.

## Shipped sources (latest inventory: 2026-10-10)

The dated observations below are historical snapshots. The independent
2026-10-10 inventory later in this document supersedes endpoint health and
counts; it does not establish end-to-end Envy connectivity.

### ED2K `server.met`

| URL | Operator | HTTPS | Notes |
| --- | --- | --- | --- |
| `https://upd.emule-security.org/server.met` | eMule-Security | yes | Current bounded check: HTTP 200, binary `server.met` response; maintained default documented by aMule |
| `https://shortypower.org/server.met` | shortypower | yes | Independent mirror documented by aMule; timed out in the 2026-10-09 bounded check and must not be treated as the sole source |

Removed: static IPs in `DefaultServers.dat` (TV Underground / eDonkey Server No1–3 / 2019 Peerates IP); `peerates.net` (last-modified 2021); `gruk.org` (cleartext alternative; 2026-10-04 returned binary `.met`, correcting the earlier HTML observation); `emule-server.de` (last-modified 2009); `www.emule-security.org/server.met` (404). Setting `eDonkey.ServerListURL` default is the eMule-Security HTTPS URL.

### Kad `nodes.dat`

| URL | Operator | HTTPS | Notes |
| --- | --- | --- | --- |
| `https://upd.emule-security.org/nodes.dat` | eMule-Security | yes | Current bounded check: HTTP 200, binary v2 response with 189 contacts; maintained default documented by aMule; import only when local Kad sources leave no usable contacts |

Removed: `https://shortypower.org/nodes.dat` returned HTTP 404 on 2026-10-04. The remaining source returned 5112 bytes, v2, 150 records. One HTTPS source satisfies `BootstrapMinKadNodesDat`; independent fallback remains desirable but must be validated before inclusion.

### Gnutella 1 UHC

From gtk-gnutella `devel/src/core/uhc.c` (not HTTP GWC):

- `uhc:useast.gnutella.dyslexicfish.net:3558`
- `uhc:uswest.gnutella.dyslexicfish.net:3558`
- `uhc:uk.gnutella.dyslexicfish.net:3558`
- `uhc:au.gnutella.dyslexicfish.net:3558`
- `uhc:1.uhc.gtk-gnutella.nl:19104`
- `uhc:2.uhc.gtk-gnutella.nl:4876`

Removed: `uhc.gtk-gnutella.nl:15749` (obsolete port); HTTP GWC rows that pointed at UDP UHC ports `:3558` / `:3559`.

### Gnutella / G2 GWebCache

The first three returned HTTP 200 with `H|address:port|age` records on 2026-10-04. `dkac.trillinux.org` timed out in this bounded check; retained as an independently operated source previously responsive on 2026-09-18, not certified live. HTTP remains an ecosystem limitation; no silent TLS downgrade was added.

| URL | Type | Operator |
| --- | --- | --- |
| `http://midian.jayl.de/g2/bazooka.php` | M | jayl.de |
| `http://bj.ddns.net/beacon/gwc.php` | M | bj.ddns.net |
| `http://gweb3.4octets.co.uk/gwc.php` | M | 4octets |
| `http://dkac.trillinux.org/dkac/dkac.php` | 2 | trillinux (G2 only) |

Removed as dead or parked: `cache.getenvy.com/*` (lander), `gwctest.zapto.org`, `disobscure.velum-ultra.com`, `tenafly5k.com`, `gwc.centrump2p.com`, `cache.ce3c.be`, `cache.ibel.de`, `k33bz.com` (HTTP 500). `skulls.gwc.dyslexicfish.net` is an HTML directory, not a queryable GWC.

### Cross-project candidate sweep

The current gtk-gnutella source remains the maintained reference for UHC
bootstrap names. Its `boot_hosts` list matches the six UHC rows above; the
other ports in that source are peer endpoints, not additional host-cache
bootstrap services, so they were not promoted into `DefaultServices.dat`.

Historical PeerProject defaults were also used as a candidate source. The
following candidates were checked with Envy's GWC2 request shape on
2026-10-09: `webcache.peerproject.org`, `cache.w3-hidden.cc`,
`gwebcache.ns1.net`, `gweb.dwbo.nl`, `cache.trillinux.org/g2/bazooka.php`,
`silvers.zyns.com/gwc/dkac.php`, `brov.mine.nu/Beacon/gwc.php`,
`gwc2.wodi.org/skulls.php`, `gwc.dyndns.info:28960/gwc.php`,
`cache2.bazookanetworks.com/g2/bazooka.php`, `cache3.leite.us`,
`cache5.leite.us`, and `gwc.iblinx.com:2108/gwc/cgi-bin/fc`.
None returned both a usable HTTP response and a GWebCache host record;
`gwebcache.ns1.net` returned a Cloudflare 403 page. They remain excluded.

This negative result is intentional: the GWebCache specification requires
clients to remove caches that do not respond correctly and warns against
repeated requests to unavailable volunteer services. A new shipped URL is
added only after a bounded request returns the expected `I|`/`H|` response.

### Direct Connect hublists (NMDC)

| URL | Operator | Mix |
| --- | --- | --- |
| `https://dchublist.org/hublist.xml.bz2` | dchublist.org | HTTP 200, valid BZip2/XML, 273 hub entries; mixed contents require NMDC filtering |
| `https://dchublist.ru/hublist.xml.bz2` | dchublist.ru | HTTP 200, valid BZip2/XML, 91 hub entries; NMDC rows are the supported subset |
| `https://hublist.pwiam.com/hublist.xml.bz2` | PWiAM | HTTP 200, valid BZip2/XML, 267 hub entries; mixed contents require NMDC filtering |
| `https://www.te-home.net/?do=hublist&get=hublist.xml.bz2` | Team Elite | Added 2026-10-10: verified HTTPS, BZip2/XML, 258 hubs including 244 bare NMDC addresses; 14 ADC/ADCS rows are skipped |

`DC.HubListURL` default is the dchublist.org HTTPS URL. `dchublist.com` 301s to Team Elite; `tankafett.biz` 301s to an HTML page, not `hublist.xml.bz2`.

### BitTorrent DHT routers

| Host | Reference |
| --- | --- |
| `dht.transmissionbt.com:6881` | libtorrent documented alternative; BEP 5 response observed |
| `router.bittorrent.com:6881` | libtorrent documented alternative; DNS resolves, one UDP probe timed out |
| `dht.libtorrent.org:25401` | libtorrent default; BEP 5 response observed |

Removed: `router.utorrent.com`, `router.bitcomet.com` (no DNS), `dht.aelitis.com` (Vuze). Persistence of working nodes remains `CDHT::Disconnect` → `HostCache.dat`.

`CHostCacheList::Add` stores unresolved DNS names with `m_pAddress = INADDR_ANY` (`Network.Resolve(..., FALSE)` does not call DNS). `CHostCacheMap` is a `std::multimap`, so the three shipped `B` rows all insert under `0.0.0.0` without dropping later names; `Find(IN_ADDR)` returns NULL for `INADDR_ANY` by design; `CDHT::Connect` iterates `m_HostsTime` and still copies all three. Debug `ASSERT(m_Hosts.size() == m_HostsTime.size())` remains true. Rewriting HostCache keying for hostname-only rows is a follow-up, not this catalogue slice.


## October audit: hard-coded defaults and validation limits

Audited `develop` at `59695b173bac6510388aa90f29cd88d60db35b74`. Existing work: #215 (catalogue/parser refresh) and #365 (Kad cold-start) are implemented; #419 owns catalogue reconciliation and the still-open persisted-service migration. #420 owns service display/provenance. No protocol or loader change is included in this reconciliation.

| Location | Value / role | Decision |
| --- | --- | --- |
| `Envy/Settings.cpp`, `eDonkey.ServerListURL` | HTTPS eMule-Security `server.met` | Keep; fresh binary list observed |
| `Envy/DcHublistSources.h`, `DC.HubListURL` | HTTPS dchublist.org | Keep; operator publishes this URL, BZip2/XML observed |
| `Envy/Settings.cpp`, `BitTorrent.DefaultTracker` | `udp://tracker.openbittorrent.com:80/announce` | DNS failed in this audit; replacement deferred for private-torrent safety (#317) |
| `Envy/BTInfo.cpp`, rejected single tracker fallback | Uses `DefaultTracker` before the private flag is processed | Do not make a public replacement live until #317 prevents this fallback for private torrents |
| `Envy/BTPacket.h` | LSD `239.192.152.143:6771` | BEP 14 protocol constant; retain, not a service catalogue |
| `Envy/EDPacket.h` | ED2K LAN multicast `224.0.0.1:5000` | Existing protocol constant; retain in this data-only slice |
| `Envy/Settings.cpp`, `SmartUpgrade` | Old ED2K/DC URL comparisons | Migration identities, not active bootstrap providers; retain |
| `Envy/BTPacket.cpp`, DHT connect | HostCache/catalogue routers | No independent active C++ DNS seed list found |
| Other `Data/` resources | Vendor links, message/security filters, schemas, images, GeoIP | Not bootstrap sources; do not refresh them as host catalogues |
| `Envy/Envy.h`, Bitprints/website/update/help URLs | Product/metadata services | Separate maintenance scope; not peer discovery |
| `TorrentEnvy/` | User-supplied tracker builder fields | No second shipped bootstrap list found |

[OpenTrackr](https://opentrackr.org/) publishes `udp://tracker.opentrackr.org:1337/announce`; a BEP 15 connect probe returned a matching transaction and connection ID. This probe announces **no** infohash and is not proof of announce/scrape interoperability. The default is deliberately not changed in this PR: `CBTInfo::LoadTorrentTree` can replace an unsupported tracker (including HTTPS) before it reads `private=1`. Making the fallback reachable could expose private-torrent announces. Track the required guard under #317, HTTPS support under #88, and default migration/custom-value preservation under #418.

Validation used bounded HTTP GETs (20 s, 1 MiB read cap), normal TLS certificate verification, binary headers/counts, BZip2/XML decoding, DNS and one bounded BEP 5 `find_node` probe per router. On the 2026-10-09 recheck, eMule-Security `server.met` returned HTTP 200 with 12 records and `nodes.dat` returned HTTP 200 with version 2 and 189 contacts. The direct shortypower `server.met` and `nodes.dat` URLs timed out in this check. The maintained aMule documentation still identifies eMule-Security and shortypower as compatible `server.met` sources and eMule-Security as the maintained `nodes.dat` default. The three retained hublists returned HTTP 200, valid BZip2/XML, and 273, 267, and 91 hub entries respectively. Transmission and libtorrent replied with matching transactions; BitTorrent timed out. These are point-in-time observations, not guarantees of uptime or full Envy runtime interoperability. No peer IPs from responses are added to shipped data. No live UHC UDP interoperability is claimed; the six retained endpoints are present in maintained gtk-gnutella `boot_hosts` (three additional peer ports there are not needed for the minimum catalogue). Candidate alternatives such as `router.bt.ouinet.work:6881` were documented by libtorrent but not added because they were not independently validated in this audit.

The offline catalogue test compares all parsed source identities with the vetted set, rejecting invalid, truncated, duplicate or unexpected extra rows rather than merely checking minima. The three blocked `X` identities must also remain present; they are security entries, not bootstrap providers. Existing `Discovery.dat` is never cleared or rewritten wholesale: missing shipped identities (including the G1 UHC row) are added additively, and the broader persisted-service migration remains open in #419.

Primary references consulted:

- [BEP 5 DHT](https://www.bittorrent.org/beps/bep_0005.html), [BEP 14 LSD](https://www.bittorrent.org/beps/bep_0014.html), [BEP 15 UDP tracker](https://www.bittorrent.org/beps/bep_0015.html), [BEP 27 private torrents](https://www.bittorrent.org/beps/bep_0027.html).
- [eMule-Security server list](https://www.emule-security.org/serverlist/) and [shortypower](https://shortypower.org/). The operator page advertises HTTP; working HTTPS was checked directly and retained.
- [aMule getting started](https://wiki.amule.org/wiki/Getting_Started): explains server.met/nodes.dat roles, but its old gruk/peerates examples are not evidence of current endpoint health.
- [gtk-gnutella UHC implementation](https://github.com/gtk-gnutella/gtk-gnutella/blob/devel/src/core/uhc.c), [Shareaza discovery lineage](https://github.com/ivan386/Shareaza/blob/master/shareaza/DiscoveryServices.cpp): references for G1/UHC and GWC roles; not a requirement to import their entire catalogue.
- [dchublist.org operator instructions](https://dchublist.org/), [PWiAM operator](https://hublist.pwiam.com/), [AirDC++ maintained defaults](https://github.com/airdcpp/airdcpp-windows/blob/master/airdcpp/airdcpp/settings/SettingsManager.cpp): its `HUBLIST_SERVERS` includes all three retained operators; hublist/client references; mixed hublist contents do not grant ENVY ADC/ADCS support.
- [libtorrent bootstrap settings](https://libtorrent.org/reference-Settings.html#dht_bootstrap_nodes), [qBittorrent session implementation](https://github.com/qbittorrent/qBittorrent/blob/master/src/base/bittorrent/sessionimpl.cpp), [Transmission DHT implementation](https://github.com/transmission/transmission/blob/main/libtransmission/tr-dht.cc). The current libtorrent documentation, rather than older qBittorrent/Transmission attribution, directly supports all three retained names.

## Independent endpoint inventory — 2026-10-10 UTC

This run started at PR head `3719b635086278026e9122b7128db5430d51ca61`.
HTTP observations were made at 06:55–07:06 UTC. DNS resolved for every
endpoint below. HTTPS used normal certificate/hostname verification; no
certificate bypass or silent downgrade was used. Downloads used a 12-second
socket timeout, at most three redirects and a 1 MiB read cap; hublist expansion
was capped at 4 MiB. DNS uses the operating system resolver's timeout, not a
separate application deadline. UDP used one request per endpoint and a
four-second receive timeout. No user infohash, tracker announce, peer crawl,
user setting or persistent cache was involved.

Evidence levels are distinct: **mentioned** by an operator/client;
**reachable** over the transport; **conformant** payload; **compatible** with
Envy's existing importer; and **effective discovery**, which additionally
requires live Envy import and peer connections. None of these probes reaches
the final level. `Validated` below means the observed payload/transport,
not complete protocol interoperability. `Uncertain` sources already shipped
are retained when a timeout cannot establish retirement. New candidates need
positive compatibility and freshness evidence before inclusion.

Origin keys (also identify the source of each exact endpoint):

- **A**: [aMule server sources](https://amule-org.github.io/docs/p2p-networks/ed2k), [nodes.dat format/default](https://amule-org.github.io/lv/docs/developer/file-formats/nodes-dat), [eMule-Security operator](https://www.emule-security.org/), [shortypower operator](https://shortypower.org/).
- **G**: [gtk-gnutella maintained UHC implementation](https://github.com/gtk-gnutella/gtk-gnutella/blob/devel/src/core/uhc.c). Its `boot_hosts` includes all six retained identities; alternate peer ports are not substituted for cache ports.
- **W**: [Shareaza discovery lineage](https://github.com/ivan386/Shareaza/blob/master/shareaza/DiscoveryServices.cpp) and the exact retained cache URL itself. Envy request construction: `Envy/DiscoveryServices.cpp:1456`; case-insensitive `h`/`i` parsing: `:1524` / `:1672`.
- **H**: [AirDC++ maintained hublist defaults](https://github.com/airdcpp/airdcpp-windows/blob/master/airdcpp/airdcpp/settings/SettingsManager.cpp). The defaults explicitly list all four shipped URLs and the DCNF candidate.
- **B**: [libtorrent bootstrap setting](https://www.libtorrent.org/reference-Settings.html#dht_bootstrap_nodes), [BEP 5](https://www.bittorrent.org/beps/bep_0005.html). Libtorrent explicitly documents all four tested router names.

| Protocol | Exact endpoint | Operator / origin | Verified UTC | DNS/TLS/HTTP/UDP | Format | Status | Envy compatibility | Decision | Evidence / limitation |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ED2K | `https://upd.emule-security.org/server.met` | eMule-Security / A | 2026-10-10 | DNS, TLS, 200 | All 12 records and tags consumed; 12 public addresses/nonzero ports; no duplicates or trailing bytes | Validated | Existing MET importer | Keep | Last-Modified 2026-10-09; endpoint reachability is not server connectivity |
| ED2K | `https://shortypower.org/server.met` | shortypower / A | 2026-10-10 | DNS; HTTPS connection timeout | No fresh payload | Uncertain | Previously verified MET | Keep under reservation | No retirement conclusion from bounded timeout; not the sole provider |
| Kad | `https://upd.emule-security.org/nodes.dat` | eMule-Security / A | 2026-10-10 | DNS, TLS, 200 | v2, 6438 bytes, 189 exact 34-byte records; no duplicate records; 175 public/nonzero-port Kad2 candidates before Envy filtering | Validated | Production `KadNodesDatParse`: status Ok, 167 accepted with synthetic zero own-ID; 256 KiB / 5000-contact caps, normal import cap 200, /24 cap 2 | Keep | Last-Modified 2026-10-09; routing remains partial; no contacts were inserted into runtime caches |
| G1 | `uhc:useast.gnutella.dyslexicfish.net:3558` | dyslexicfish / G | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Maintained UHC identity | Keep under reservation | Single TTL-1 SCP leaf ping; no permanent failure claim |
| G1 | `uhc:uswest.gnutella.dyslexicfish.net:3558` | dyslexicfish / G | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Maintained UHC identity | Keep under reservation | Same bounded probe |
| G1 | `uhc:uk.gnutella.dyslexicfish.net:3558` | dyslexicfish / G | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Maintained UHC identity | Keep under reservation | Same bounded probe |
| G1 | `uhc:au.gnutella.dyslexicfish.net:3558` | dyslexicfish / G | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Maintained UHC identity | Keep under reservation | Same bounded probe |
| G1 | `uhc:1.uhc.gtk-gnutella.nl:19104` | gtk-gnutella / G | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Maintained UHC identity | Keep under reservation | Not an HTTP cache |
| G1 | `uhc:2.uhc.gtk-gnutella.nl:4876` | gtk-gnutella / G | 2026-10-10 | DNS; UDP response | 273-byte pong, matching GUID and payload length; IPP extension present | Validated envelope | G1 UDP pong path; contact import untested | Keep | Not a peer handshake or a full GGEP/contact-validation test |
| G1/G2 | `http://midian.jayl.de/g2/bazooka.php` | jayl.de / W | 2026-10-10 | DNS; HTTP 200 for both networks | G2: 17 public host rows; G1: 8; info rows present | Validated | Existing GWC request/parser | Keep | Cleartext discovery can be modified in transit |
| G1/G2 | `http://bj.ddns.net/beacon/gwc.php` | bj.ddns.net / W | 2026-10-10 | DNS; HTTP 200 for both networks | G2: 20 public host rows; G1: 12; info rows present | Validated | Existing GWC request/parser | Keep | Cleartext risk; returned peers not contacted |
| G1/G2 | `http://gweb3.4octets.co.uk/gwc.php` | 4octets / W | 2026-10-10 | DNS; HTTP 200 for both networks | G2: 12 public host rows; G1: 6; info rows present | Validated | Existing GWC request/parser | Keep | Cleartext risk; no HTTPS URL inferred |
| G2 | `http://dkac.trillinux.org/dkac/dkac.php` | trillinux / W | 2026-10-10 | DNS; HTTP 200 | `i` pong and 12 lowercase `h` rows in follow-up | Validated | Envy compares record prefixes without case | Keep | Earlier timeout superseded; initial uppercase-only probe undercounted these rows |
| NMDC | `https://dchublist.org/hublist.xml.bz2` | dchublist.org / H | 2026-10-10 | DNS, TLS, 200 | BZip2/XML, 273 hubs, 244 `dchub://`; no repeated Address values | Validated | Supported NMDC subset | Keep | Last-Modified 2026-10-10; skip ADC/ADCS |
| NMDC | `https://hublist.pwiam.com/hublist.xml.bz2` | PWiAM / H | 2026-10-10 | DNS, TLS, 200 | BZip2/XML, 267 hubs, 252 `dchub://`; 4 repeated Address values | Validated | Supported NMDC subset; normal host-cache deduplication | Keep | No Last-Modified supplied; duplicate list rows are not catalogue duplicates |
| NMDC | `https://dchublist.ru/hublist.xml.bz2` | dchublist.ru / H | 2026-10-10 | DNS, TLS, 200 | BZip2/XML, 91 hubs, all `dchub://`; no repeated Address values | Validated | Supported NMDC subset | Keep | Last-Modified 2026-10-03 |
| NMDC | `https://www.te-home.net/?do=hublist&get=hublist.xml.bz2` | Team Elite / H | 2026-10-10 | DNS, TLS, 200; no redirect | BZip2/XML `Hublist/Hubs`, 258 unique addresses: 244 bare NMDC, 3 ADC, 11 ADCS | Validated | `ImportHubList` accepts bare NMDC addresses and skips ADC/ADCS | Add | Last-Modified 2026-10-10; exact URL published by AirDC++; no importer/default-setting change |
| BT DHT | `dht.transmissionbt.com:6881` | Transmission / B | 2026-10-10 | DNS; BEP 5 UDP response | Matching transaction, response type and 20-byte ID; 8 compact nodes, all public/nonzero-port | Validated | Existing router path | Keep | No node crawl or torrent announcement |
| BT DHT | `router.bittorrent.com:6881` | BitTorrent / B | 2026-10-10 | DNS; UDP timeout | No reply | Uncertain | Documented router | Keep under reservation | A single timeout is not retirement evidence |
| BT DHT | `dht.libtorrent.org:25401` | libtorrent / B | 2026-10-10 | DNS; BEP 5 UDP response | Matching transaction/type/ID; 3 compact nodes, all public/nonzero-port | Validated | Existing router path | Keep | No user infohash transmitted |

### Candidates examined but not shipped

| Protocol | Exact endpoint | Operator / origin | Verified UTC | Transport | Format | Status | Envy compatibility | Decision | Reason |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ED2K | `http://www.synology.com/server.met` | [Synology documentation](https://kb.synology.com/fr-fr/DSM/help/DownloadStation/emule_server?version=7) | 2026-10-10 | Redirects to `https://www.synology.com/en-global`, 200 | HTML, not MET | Obsolete as download | Not importable | Exclude | Documentation gives an example, not evidence of a currently usable download |
| ED2K | `https://www.gruk.org/server.met` | [gruk operator](https://www.gruk.org/) / historical aMule example | 2026-10-10 | TLS, 200 | 11 fully consumed MET records, public/nonzero ports, no duplicates | Valid format; freshness uncertain | MET compatible | Document only | HTTPS works; exclusion must no longer be justified by HTML or cleartext alone. No Last-Modified or independent recent list-maintenance evidence obtained |
| ED2K | `https://server-met.emulefuture.de/` | eMuleFuture operator | 2026-10-10 | DNS; HTTPS timeout | No payload | Uncertain | Not verified | Exclude | The candidate page and tentative `download.php?file=server.met` path both timed out; no exact maintained binary URL established |
| Kad | `https://shortypower.org/nodes.dat` | shortypower | 2026-10-10 | DNS; HTTPS timeout | No fresh payload | Uncertain now; previously 404 | Not revalidated | Keep excluded | Timeout does not reverse the recorded 404 retirement evidence |
| Kad | `https://www.nodes-dat.com/dl.php?load=nodes` and `https://nodes-dat.com/dl.php?load=nodes` | [nodes-dat operator page](https://www.nodes-dat.com/) | 2026-10-10 | TLS, redirect to HTML homepage | Not nodes.dat | Inactive for tested HTTPS URLs | Not importable | Exclude | Page advertises HTTP download with a changing trace parameter; no validated HTTPS binary equivalent found; `/nodes.dat` returns 404 |
| Kad | `https://kademlia.ru/download/nodes.dat` | Operator linked by nodes-dat.com (HTTP link) | 2026-10-10 | TLS, 404 | No binary | Inactive at HTTPS path | Not importable | Exclude | Do not silently downgrade the HTTPS-only K catalogue to HTTP |
| Kad | `https://emulemods.altervista.org/download/nodes.dat` | [Emule-Mods operator announcement](https://www.emulemods.altervista.org/index.php?topic=2757.0) | 2026-10-10 | TLS, 200 | v2, 6744 bytes, 198 exact records; 187 public/nonzero-port Kad2 candidates; no duplicate records | Structurally valid; old | v2 compatible; live contacts unverified | Exclude | Last-Modified 2025-11-02 agrees with dated announcement; insufficient freshness for a new reliable fallback |
| NMDC | `https://dcnf.github.io/Hublist/hublist.xml.bz2` | DCNF / H | 2026-10-10 | TLS, 200 | BZip2/XML, 732 hubs, 663 `dchub://`; 9 repeated Address values | Valid format; freshness uncertain | Supported NMDC subset | Document only | Last-Modified 2026-06-08; four fresher shipped providers suffice; no additional host reachability evidence |
| NMDC | `https://hublist.te-home.net/hublist.xml.bz2` | Guessed Team Elite subdomain, not the AirDC++ URL | 2026-10-10 | TLS hostname mismatch | Not downloaded | Invalid candidate | Not verified | Exclude | Use the exact validated `www.te-home.net` query URL above; never bypass certificate verification |
| BT DHT | `router.bt.ouinet.work:6881` | Ouinet / B | 2026-10-10 | DNS; BEP 5 UDP response | Matching transaction/type/ID; 16 public/nonzero-port compact nodes | Validated | Protocol compatible; fourth unresolved name exceeds current cold-start DNS budget | Document only | Keep three shipped names and existing caps; do not retire BitTorrent on one timeout or alter engine limits to admit a fourth |

The three `X` rows were preserved without probing blocked services. Historical
retired GWC/ED2K identities remain excluded as recorded above. No static peer
addresses from any response enter the catalogues. No new Kad source passed
the combined HTTPS, format and freshness criteria. HTTP GWCs remain an
explicit integrity risk, independent of whether their payloads parse.

### Offline/build validation of the final catalogue

- Initial-head standalone MSVC C++17 catalogue module: 28/28 passed x64.
  Updated module: 30/30 passed x64 and Win32, using the actual shipped files.
- Seven mutations fail as intended: remove an `X` block, duplicate a service,
  restore retired Peerates, append a truncated row, append an unknown type,
  duplicate a DHT router, and append a synthetic static ED2K seed. Original
  bytes were restored after each mutation; the final 30/30 run passes.
- VS 2026 Insiders MSBuild, solution Release x64, v145, Windows SDK 10.0:
  exit 0 (incremental build). Fresh EnvyTests: 708/708 passed, exit 0.
- Solution Release Win32 with manifest enabled and `x86-windows-static`:
  exit 1 because required `crashpad_handler.exe` is absent. The independently
  generated Win32 EnvyTests still pass 708/708; this is **not** a successful
  Win32 application build. Existing dependency/environment work remains
  separate; no third-party source or gate was altered.
- Explicit x64 installer-project rebuild: exit 0, generated
  `Envy.4.2.0.1.64.exe`. `Installer/Scripts/Main.iss` selects repository
  `Data/*`; its compile log includes both tested catalogue files. No installer
  execution or application runtime test was performed. Existing BugSplat
  missing-`dumpbin` copy and installer language/privilege warnings remain;
  installer compilation alone does not establish complete runtime packaging.
- `Settings.cpp`, `BTInfo.cpp`, `BootstrapCatalog.h`, packet constants and
  protocol/importer engines are unchanged by this PR. Private-torrent safety
  remains #317; runtime source migration remains #419. Maintainer live-test,
  Ready transition and manual merge remain pending.

## Remaining C++ network addresses (not this catalogue)

Justified runtime / product URLs, **not** P2P bootstrap:

- `WEB_SITE`, `UPDATE_URL`, `UPDATE_URL_ALT` in `Envy.h` - website and version check
- Schema `xmlns` and `getenvy.com` copyright comments
- `BitTorrent.DefaultTracker` - torrent tracker default, not DHT bootstrap

## Follow-ups (not this slice)

- Parser hardening for `server.met` / hublist BZip2 / GWC (size, inflate, entry caps) — P0 potential; see #82 and related importer PRs
- Kad importer/runtime hardening beyond cold-start `nodes.dat` download; bootstrap HTTP timeout enforcement remains bounded through WinINet `InternetSetOption` options (live interop still #86 / #160)
- Last-known-good remote catalogue (async, ETag, atomic replace, never block startup)
- Scheduled GitHub workflow that **reports** source health and never auto-merges `develop`

## Wire-format impact

None. This change edits shipped catalogues, documentation and offline tests only. DHT still uses BEP 5 `find_node` toward bootstrap routers.
