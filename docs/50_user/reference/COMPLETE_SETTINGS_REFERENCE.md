# Envy Complete Settings Reference

## 📋 Complete Envy Settings Table

This document provides a comprehensive reference of all configurable settings in Envy P2P client, organized by category with default values and detailed explanations.

**Version:** Envy 4.0+
**Last Updated:** January 18, 2026
**Settings Version:** Internal Version (SmartVersion)

---

## 📋 Table of Contents

- [General Settings](#-general-settings)
- [Connection Settings](#-connection-settings)
- [Bandwidth Settings](#-bandwidth-settings)
- [Community Settings](#-community-settings)
- [Search Settings](#-search-settings)
- [Media Player Settings](#-media-player-settings)
- [Web Protocol Settings](#-web-protocol-settings)
- [Library Settings](#-library-settings)
- [Web Services Settings](#-web-services-settings)
- [Download Settings](#-download-settings)
- [Upload Settings](#-upload-settings)
- [IRC Settings](#-irc-settings)
- [Live Status Settings](#-live-status-settings)
- [Remote Access Settings](#-remote-access-settings)
- [Scheduler Settings](#-scheduler-settings)
- [Security Settings](#-security-settings)
- [Experimental Settings](#-experimental-settings)
- [Protocol-Specific Settings](#-protocol-specific-advanced-settings)
  - [Gnutella1 Settings](#-gnutella1-settings)
  - [Gnutella2 Settings](#-gnutella2-settings)
  - [eDonkey2000 Settings](#-edonkey2000-settings)
  - [BitTorrent Settings](#-bittorrent-settings)
- [Interface & Skin Settings](#-interface--skin-settings)
  - [Interface Settings](#-interface-settings)
  - [Skin Settings](#-skin-settings)
  - [Font Settings](#-font-settings)
  - [Window Settings](#-window-settings)
  - [Toolbar Settings](#-toolbar-settings)

---

## 🏠 General Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **General** | Path | Installation Path | Full path to Envy installation directory |
| | UserPath | User Data Path | Path for user-specific data (may differ for multi-user installs) |
| | DataPath | Data Directory | Complete path to user data folder |
| | AntiVirus | AV Path/CLSID | Path to antivirus software or its CLSID |
| | MultiUser | Multi-user Mode | Whether this is a multi-user installation |
| | DialogScan | Dialog Template Scan | Create translation templates in dialog scan mode |
| | DebugLog | Debug Logging | Enable creation of debug log file |
| | SearchLog | Search Logging | Display search facility log information |
| | LogLevel | Log Severity | Logging level (0=Error, 1=Warning, 2=Info, 3=Debug) |
| | MaxDebugLogSize | Max Log Size | Maximum size of debug log file in MB |
| | DiskSpaceWarning | Low Space Warning | Warning threshold for available disk space (MB) |
| | DiskSpaceStop | Low Space Stop | Critical threshold where downloads pause (MB) |
| | MinTransfersRest | Transfer Rest Period | Minimum time between transfer rounds (ms) |
| | SmartVersion | Settings Version | Internal version number for settings compatibility |
| | GUIMode | GUI Mode | Interface mode (Windowed/TABBED/Basic) |
| | CloseMode | Close Behavior | What happens when closing main window |
| | TrayMinimise | Minimize to Tray | Minimize to system tray instead of taskbar |
| | ShowTimestamp | Show Timestamps | Display timestamps in logs and messages |
| | SizeLists | Size List Columns | Auto-size list view columns |
| | HashIntegrity | Hash Verification | Enable file hash integrity checking |
| | RatesInBytes | Speed Units | Show transfer rates in bytes (true) or bits (false) |
| | RatesUnit | Rate Display Unit | Unit for displaying rates (0=B/s, 1=KB/s, 2=MB/s) |
| | LockTimeout | Lock Timeout | Timeout for overloaded lock attempts (ms) |
| | Language | UI Language | Selected user interface language |
| | LanguageRTL | Right-to-Left | Enable RTL layout for certain languages |
| | LanguageDefault | Default Language | Use English as fallback |
| | AlwaysOpenURLs | Auto-open URLs | Automatically open magnet/ed2k links |
| | DebugBTSources | BT Source Debug | Show received sources for BitTorrent seeding |

---

## 📡 Connection Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Connection** | AutoConnect | Auto-connect | Automatically connect to networks on startup |
| | FirewallState | Firewall Status | Current firewall configuration state |
| | OutHost | External IP | User's external IP address |
| | InHost | Listen Address | Local address to bind to |
| | InPort | Listen Port | TCP port for incoming connections |
| | InBind | Bind to Address | Whether to bind to specific local address |
| | RandomPort | Random Port | Use random port selection |
| | InSpeed | Inbound Speed | Internet connection download speed (Kbps) |
| | OutSpeed | Outbound Speed | Internet connection upload speed (Kbps) |
| | IgnoreLocalIP | Ignore LAN IPs | Ignore local network IP addresses |
| | IgnoreOwnIP | Ignore Own IP | Don't accept connections from own external IP |
| | IgnoreOwnUDP | Ignore Own UDP | Don't accept UDP from own external IP |
| | TimeoutConnect | Connect Timeout | Connection establishment timeout |
| | TimeoutHandshake | Handshake Timeout | Protocol handshake timeout |
| | TimeoutTraffic | Traffic Timeout | Data transfer timeout |
| | SendBuffer | Send Buffer | TCP send buffer size |
| | RequireForTransfers | Transfers Require Networks | Only download/upload when connected to networks |
| | ConnectThrottle | Connection Throttle | Delay between neighbor connection attempts (ms) |
| | FailurePenalty | Failure Penalty | Delay after connection failure (seconds) |
| | FailureLimit | Failure Limit | Max allowed connection failures |
| | DetectConnectionLoss | Detect Disconnect | Detect internet connection loss |
| | DetectConnectionReset | Detect Reconnect | Detect internet reconnection |
| | ForceConnectedState | Force Online Mode | Force Windows into online state |
| | SlowConnect | Slow Connect Mode | Connect to one network at a time |
| | EnableFirewallException | Firewall Exception | Add Windows Firewall exception |
| | DeleteFirewallException | Remove Firewall Exception | Remove firewall exception on exit |
| | EnableUPnP | UPnP Support | Enable automatic port forwarding |
| | DeleteUPnPPorts | UPnP Cleanup | Remove port forwarding on exit |
| | UPnPTimeout | UPnP Timeout | Timeout for UPnP device responses (ms) |
| | UPnPRefreshTime | UPnP Refresh | Port mapping refresh interval |
| | SkipWANPPPSetup | Skip WAN PPP | Skip WANPPPConn1 device setup |
| | SkipWANIPSetup | Skip WAN IP | Skip WANIPConn1 device setup |
| | EnableBroadcast | Allow Broadcast | Send/receive broadcast packets |
| | EnableMulticast | Allow Multicast | Send/receive multicast packets |
| | MulticastLoop | Multicast Loopback | Enable multicast loopback |
| | MulticastTTL | Multicast TTL | Time-to-live for multicast packets |
| | ZLibCompressionLevel | ZLib Level | Compression level for ZLib (0-9, default 1) |

---

## 📶 Bandwidth Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Bandwidth** | Request | Request Pipe | Number of simultaneous requests per peer |
| | HubIn | Hub Download Limit | Download limit for hub connections |
| | HubOut | Hub Upload Limit | Upload limit for hub connections |
| | LeafIn | Leaf Download Limit | Download limit for leaf connections |
| | LeafOut | Leaf Upload Limit | Upload limit for leaf connections |
| | PeerIn | Peer Download Limit | Download limit for peer connections |
| | PeerOut | Peer Upload Limit | Upload limit for peer connections |
| | UdpOut | UDP Upload Limit | Upload limit for UDP traffic |
| | Downloads | Download Limit | Total download speed limit (Bytes/s) |
| | Uploads | Upload Limit | Total upload speed limit (Bytes/s) |
| | HubUploads | Hub Upload Limit | Additional upload limit for hubs |

---

## 👥 Community Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Community** | ChatEnable | Chat Enabled | Enable chat with compatible clients |
| | ChatAllNetworks | Chat Cross-Network | Allow chat across all protocols |
| | ChatFilter | Chat Spam Filter | Filter out chat spam |
| | ChatFilterED2K | ED2K Spam Filter | Filter known ED2K spam |
| | ChatCensor | Chat Censoring | Censor inappropriate words (uses adult filter) |
| | Timestamp | Chat Timestamps | Show timestamps in chat messages |
| | ServeProfile | Share Profile | Share user profile with others |
| | ServeFiles | Share Files | Allow others to browse shared files |
| | AwayMessageIdleTime | Away Message Delay | Idle time before showing away message (seconds) |
| | UserPanelSize | User Panel Width | Width of chat user list panel (pixels) |

---

## 🔍 Search Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Search** | LastSchemaURI | Last Schema | URI of last used search schema |
| | BlankSchemaURI | Blank Schema | URI of default blank search schema |
| | AutoPreview | Auto Preview | Automatically preview thumbnails |
| | AdultFilter | Adult Content Filter | Filter adult content from searches |
| | AdvancedPanel | Advanced Panel | Show advanced search options |
| | ResultsPanel | Results Panel | Show search results panel |
| | SearchPanel | Search Panel | Show search input panel |
| | HideSearchPanel | Hide Search Panel | Hide search panel by default |
| | HighlightNew | Highlight New | Highlight new search results |
| | ExpandMatches | Expand Matches | Auto-expand matching results |
| | SwitchToTransfers | Switch to Transfers | Switch to transfers tab when downloading |
| | SchemaTypes | Schema Types | Show schema type options |
| | ShowNames | Show Names | Display file names in results |
| | FilterMask | Filter Mask | Bitmask for result filtering |
| | MonitorSchemaURI | Monitor Schema | Schema for search monitoring |
| | MonitorFilter | Monitor Filter | Filter for search monitoring |
| | MonitorQueue | Monitor Queue | Queue size for search monitoring |
| | BrowseTreeSize | Browse Tree Size | Width of browse tree panel |
| | DetailPanelSize | Detail Panel Size | Width of detail panel |
| | DetailPanelVisible | Detail Panel Visible | Show detail panel |
| | MaxPreviewLength | Max Preview Size | Maximum preview file size |
| | SpamFilterThreshold | Spam Threshold | Spam detection threshold (percentage) |
| | GeneralThrottle | General Throttle | General search throttling |
| | ClearPrevious | Clear Previous | How to handle previous search results |
| | SanityCheck | Sanity Check | Drop hits from banned hosts |

---

## 🎵 Media Player Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **MediaPlayer** | EnablePlay | Allow Play | Enable media playback |
| | EnableEnqueue | Allow Enqueue | Allow adding to playlist |
| | Repeat | Repeat Playback | Repeat current track |
| | Random | Random Playback | Random track selection |
| | Zoom | Zoom Mode | Video zoom/aspect ratio mode |
| | Aspect | Aspect Ratio | Video aspect ratio |
| | Volume | Volume Level | Playback volume level |
| | ListSize | Playlist Size | Height of playlist panel |
| | ListVisible | Playlist Visible | Show playlist panel |
| | StatusVisible | Status Visible | Show media status bar |
| | MediaServicesCLSID | Media CLSID | CLSID for media services |
| | VisCLSID | Visualizer CLSID | CLSID for visualization plugin |
| | VisPath | Visualizer Path | Path to visualization plugin |
| | VisSize | Visualizer Size | Size of visualizer window |
| | ShortPaths | Short Paths | Use 8.3 paths for media players |
| | ServicePath | External Players | List of external media players |
| | FileTypes | Supported Types | Supported media file extensions |

---

## 🌐 Web Protocol Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Web** | Magnet | Magnet Links | Handle magnet: URI links |
| | Gnutella | Gnutella Links | Handle gnutella: URI links |
| | Foxy | Foxy Links | Handle foxy: URI links (magnet variant) |
| | ED2K | ED2K Links | Handle ed2k: URI links |
| | DC | DC Links | Handle dcfile: and dchub: URI links |
| | Torrent | Torrent Files | Handle .torrent files |

---

## 📚 Library Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Library** | WatchFolders | Watch Folders | Monitor folders for new files |
| | WatchFoldersTimeout | Watch Timeout | Timeout for folder watching |
| | VirtualFiles | Virtual Files | Enable virtual file system |
| | SourceMesh | Source Mesh | Enable source mesh sharing |
| | SourceExpire | Source Expiration | How long to keep sources (seconds) |
| | TigerHeight | Tiger Tree Height | Height of Tiger hash tree |
| | QueryRouteSize | Query Route Size | Size of query routing table |
| | HistoryTotal | History Total | Total history entries to keep |
| | HistoryDays | History Days | Days to keep history |
| | ThumbQuality | Thumbnail Quality | JPEG quality for thumbnails |
| | ThumbSize | Thumbnail Size | Maximum thumbnail size |
| | TreeSize | Tree Panel Size | Width of library tree panel |
| | PanelSize | Panel Size | Height of library panel |
| | ShowPanel | Show Panel | Show library panel |
| | ShowVirtual | Show Virtual | Show virtual folders |
| | SchemaURI | Schema URI | URI for library schema |
| | FilterURI | Filter URI | URI for library filter |
| | SafeExecute | Safe Execute | Safe file extensions for execution |
| | PrivateTypes | Private Types | Private file type extensions |
| | GhostLimit | Ghost File Limit | Maximum retained ghost files |
| | CreateGhosts | Create Ghosts | Default ghost file creation behavior |
| | HashWindow | Hash Window | Show annoying hashing progress window |
| | HighPriorityHash | High Priority Hash | Use high priority for hashing |
| | HighPriorityHashing | High Priority Speed | Target speed for high priority hashing (MB/s) |
| | LowPriorityHashing | Low Priority Speed | Target speed for low priority hashing (MB/s) |
| | ManyFilesWarning | File Warning Limit | Threshold for "many files" warning |
| | ExecuteFilesLimit | Execute Warning Limit | Threshold for execute warning |
| | MaliciousFileCount | Malicious Count | Duplicate count for malicious warning |
| | MaliciousFileSize | Malicious Size | Size range for malicious scanning |
| | MaliciousFileTypes | Malicious Types | Extensions for malicious file scanning |
| | MarkFileAsDownload | Mark Downloads | Mark downloaded files |
| | UseFolderGUID | Use Folder GUID | Save folder GUIDs using NTFS streams |
| | UseCustomFolders | Custom Folders | Use desktop.ini for custom folders |
| | UseWindowsLibrary | Windows Libraries | Use Windows 7+ Libraries |
| | ScanAPE | Scan APE | Extract metadata from .ape/.mac/.apl files |
| | ScanASF | Scan ASF | Extract metadata from .asf/.wma/.wmv files |
| | ScanAVI | Scan AVI | Extract metadata from .avi files |
| | ScanCHM | Scan CHM | Extract metadata from .chm files |
| | ScanEXE | Scan EXE | Extract metadata from .exe/.dll files |
| | ScanFLV | Scan FLV | Extract metadata from .flv files |
| | ScanImage | Scan Images | Extract metadata from image files |
| | ScanMP3 | Scan MP3 | Extract metadata from .mp3+.aac/.flac/.mpc files |
| | ScanMPEG | Scan MPEG | Extract metadata from .mpeg/.mpg files |
| | ScanMSI | Scan MSI | Extract metadata from .msi files |
| | ScanOGG | Scan OGG | Extract metadata from .ogg files |
| | ScanPDF | Scan PDF | Extract metadata from .pdf files |
| | ScanProperties | Scan Properties | Extract Windows properties |
| | SmartSeriesDetection | Series Detection | Detect TV series patterns |
| | URLExportFormat | URL Export Format | Template for URL export |
| | LastUsedView | Last View | Name of last used folder view |

---

## 🔗 Web Services Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **WebServices** | BitprintsAgent | Bitprints Agent | User agent for Bitprints service |
| | BitprintsWebView | Bitprints View | Bitprints web view URL |
| | BitprintsWebSubmit | Bitprints Submit | Bitprints submission URL |
| | BitprintsXML | Bitprints XML | Bitprints XML API URL |
| | BitprintsOkay | Bitprints Enabled | Enable Bitprints integration |

---

## 📥 Download Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Downloads** | IncompletePath | Incomplete Folder | Where incomplete downloads are stored |
| | CompletePath | Complete Folder | Where completed downloads are moved |
| | TorrentPath | Torrent Folder | Where .torrent files are stored |
| | CollectionPath | Collection Folder | Where .collection/.co files are stored |
| | BufferSize | Buffer Size | Network buffer size |
| | SparseThreshold | Sparse Threshold | Minimum size for sparse files (0 = disabled) |
| | MaxAllowedFailures | Max Failures | Maximum allowed download failures |
| | MaxFiles | Max Files | Maximum concurrent downloads |
| | MaxTransfers | Max Transfers | Maximum total active transfers |
| | MaxFileTransfers | Max Per File | Maximum transfers per file |
| | MaxFileSearches | Max File Searches | Max files over limit that can search |
| | MaxConnectingSources | Max Connecting | Maximum sources in connecting state |
| | MinSources | Min Sources | Minimum sources before download is troubled |
| | ConnectThrottle | Connect Throttle | Delay between download attempts |
| | QueueLimit | Queue Limit | Longest queue to wait in |
| | RetryDelay | Retry Delay | Delay between download retries |
| | SaveInterval | Save Interval | How often to save download progress |
| | SearchPeriod | Search Period | How often to search for more sources |
| | StarveGiveUp | Starve Timeout | Hours before giving up starved downloads |
| | StarveTimeout | Starve Check | Ticks between starvation checks |
| | PushTimeout | Push Timeout | Timeout for push requests |
| | StaggardStart | Staggered Start | Start downloads at different times |
| | AllowBackwards | Allow Backwards | Permit reverse download mode |
| | ChunkSize | Chunk Size | Size of download chunks |
| | ChunkStrap | Chunk Strap | Number of chunks to maintain |
| | Metadata | Metadata Download | Download file metadata |
| | NeverDrop | Never Drop | Don't drop bad sources |
| | VerifyFiles | Verify Files | Verify downloaded files |
| | VerifyTiger | Verify Tiger | Use Tiger hash verification |
| | VerifyED2K | Verify ED2K | Use ED2K hash verification |
| | VerifyTorrent | Verify Torrent | Use torrent hash verification |
| | RequestHash | Request Hash | Request file hashes |
| | RequestHTTP11 | HTTP/1.1 | Use HTTP/1.1 for requests |
| | RequestURLENC | URL Encoding | Use URL encoding for requests |
| | RenameExisting | Rename Existing | Rename conflicting files |
| | FlushPD | Flush Partial | Force partial file flushing |
| | SimpleBar | Simple Progress | Use simple progress bars |
| | ShowAllSources | Show All Sources | Display all sources in list |
| | ShowPercent | Show Percent | Show completion percentage |
| | ShowGroups | Show Groups | Group downloads by category |
| | AutoExpand | Auto Expand | Auto-expand download groups |
| | AutoClear | Auto Clear | Clear completed downloads |
| | ClearDelay | Clear Delay | Delay before auto-clearing |
| | FilterMask | Filter Mask | Bitmask for download filtering |
| | ShowMonitorURLs | Show Monitor URLs | Show monitor URLs in list |
| | SortColumns | Sort Columns | Allow column sorting |
| | SortSources | Sort Sources | Sort sources by status/protocol |
| | SourcesWanted | Sources Wanted | Desired number of sources |
| | MaxReviews | Max Reviews | Maximum reviews per download |
| | NoRandomFragments | No Random Fragments | Disable random fragment selection |
| | WebHookEnable | Web Hook | Enable web hook notifications |
| | WebHookExtensions | Web Hook Extensions | File extensions for web hooks |

---

## 📤 Upload Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Uploads** | BlockAgents | Blocked Agents | List of blocked user agents |
| | MaxPerHost | Max Per Host | Maximum uploads per remote client |
| | FreeBandwidthValue | Free Bandwidth Value | Bandwidth threshold for free uploads |
| | FreeBandwidthFactor | Free Bandwidth Factor | Bandwidth factor for free uploads |
| | ClampdownFactor | Clampdown Factor | Upload reduction factor |
| | ClampdownFloor | Clampdown Floor | Minimum upload speed floor |
| | ChunkSize | Chunk Size | Size of upload chunks |
| | FairUseMode | Fair Use Mode | Limit unknown audio/video to 10% share |
| | ThrottleMode | Throttle Mode | Enable upload throttling |
| | QueuePollMin | Queue Poll Min | Minimum queue polling interval |
| | QueuePollMax | Queue Poll Max | Maximum queue polling interval |
| | RotateChunkLimit | Rotate Chunk Limit | Limit for chunk rotation |
| | SharePartials | Share Partials | Share partially downloaded files |
| | ShareTiger | Share Tiger | Share Tiger hashes |
| | ShareHashset | Share Hashset | Share hash sets |
| | ShareMetadata | Share Metadata | Share file metadata |
| | SharePreviews | Share Previews | Share file previews |
| | DynamicPreviews | Dynamic Previews | Generate previews dynamically |
| | PreviewQuality | Preview Quality | JPEG quality for previews |
| | PreviewTransfers | Preview Transfers | Maximum simultaneous previews |
| | AllowBackwards | Allow Backwards | Allow reverse upload mode |
| | HubUnshare | Hub Unshare | Don't share with hubs |
| | AutoClear | Auto Clear | Clear completed uploads |
| | ClearDelay | Clear Delay | Delay before auto-clearing uploads |
| | History | Upload History | Number of completed uploads to keep |
| | FilterMask | Filter Mask | Bitmask for upload filtering |
| | RewardQueuePercentage | Reward Queue % | Percentage of queue for rewarded users |

---

## 💬 IRC Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **IRC** | Colors[12] | IRC Colors | Color array for IRC interface |
| | Show | Show IRC | Enable IRC interface |
| | Timestamp | IRC Timestamps | Show timestamps in IRC |
| | FloodEnable | Flood Protection | Enable IRC flood protection |
| | FloodLimit | Flood Limit | Maximum flood messages |
| | OnConnect | Connect Commands | Commands to run on IRC connect |
| | UserName | IRC Username | IRC username |
| | RealName | Real Name | IRC real name |
| | Nick | IRC Nickname | IRC nickname |
| | Alternate | Alt Nickname | Alternate IRC nickname |
| | ServerName | IRC Server | Default IRC server |
| | ServerPort | IRC Port | IRC server port |
| | FontSize | IRC Font Size | Font size for IRC |
| | ScreenFont | IRC Font | Font for IRC interface |

---

## 📊 Live Status Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Live** | DiskSpaceStop | Disk Stop Active | Downloads paused due to low disk space |
| | DiskSpaceWarning | Disk Warning Active | User warned about low disk space |
| | DiskWriteWarning | Write Warning Active | User warned about write problems |
| | AdultWarning | Adult Warning | User warned about adult filter |
| | UploadLimitWarning | Upload Warning | User warned about upload limits |
| | QueueLimitWarning | Queue Warning | User warned about queue limits |
| | DonkeyServerWarning | Server Warning | User warned about empty server list |
| | DefaultED2KServersLoaded | ED2K Servers Loaded | Default ED2K servers loaded |
| | DefaultDCServersLoaded | DC Servers Loaded | Default DC servers loaded |
| | MaliciousWarning | Malicious Warning | Malicious file warning triggered |
| | LastDuplicateHash | Last Duplicate | Hash of last duplicate warning |
| | BandwidthScaleIn | Download Scale | Monitor bar download slider |
| | BandwidthScaleOut | Upload Scale | Monitor bar upload slider |
| | LoadWindowState | Load Window State | Load saved window positions |
| | AutoClose | Auto Close | Automatically close completed items |
| | FirstRun | First Run | Is this the first run of Envy? |

---

## 🌐 Remote Access Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Remote** | Enable | Remote Enabled | Enable remote web interface |
| | Username | Remote Username | Username for remote access |
| | Password | Remote Password | Password for remote access |
| | BindAddress | Bind Address | IP to bind remote interface to |
| | AllowExternal | Allow External | Allow external access (deprecated) |
| | AllowLAN | Allow LAN | Allow LAN/private network access |
| | AllowWAN | Allow WAN | Allow internet/WAN access |
| | AllowedCIDRs | Allowed CIDRs | Whitelist of allowed IP ranges |
| | RateLimitRequests | Rate Limit | Max requests per minute per IP |
| | RateLimitWindow | Rate Window | Rate limit time window (ms) |

---

## ⏰ Scheduler Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Scheduler** | Enable | Scheduler Enabled | Enable the scheduler system |
| | ForceShutdown | Force Shutdown | Force shutdown at scheduled time |
| | ValidityPeriod | Validity Period | Active trigger window (minutes) |

---

## 🔒 Security Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Security** | DefaultBan | Default Ban Time | Default ban duration (seconds) |
| | ListRangeLimit | List Range Limit | Max IPs per blacklist range |

---

## 🧪 Experimental Settings

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| **Experimental** | EnableDIPPSupport | GDNA Support | Enable GDNA host cache exchange |
| | LAN_Mode | LAN Mode | LAN optimizations (force G2 only) |

---

## 📋 Protocol-Specific Advanced Settings

### 🌐 Gnutella1 Settings

| Setting | Default | Description |
|---------|---------|-------------|
| ClientMode | MODE_AUTO | Operation mode (AUTO/LEAF/ULTRAPEER) |
| Enabled | true | Enable Gnutella1 network |
| EnableAlways | false | Always enable on startup |
| ShowInterface | true | Show G1 interface elements |
| NumHubs | 3 | Number of ultrapeers for leaf |
| NumLeafs | 25 | Number of leafs for ultrapeer |
| NumPeers | 6 | Number of peers for ultrapeer |
| PacketBufferSize | 64 | Number of packets in buffer |
| PacketBufferTime | 60000 | Packet buffer lifetime (ms) |
| DefaultTTL | 3 | Default time-to-live |
| SearchTTL | 2 | Search packet TTL |
| TranslateTTL | 2 | Translation TTL |
| MaximumTTL | 10 | Maximum allowed TTL |
| MaximumQuery | 256 | Maximum query size |
| RequeryDelay | 4 | Delay between requeries (hours) |
| HostCount | 10 | Hosts in X-Try-Ultrapeers |
| HostExpire | 2 | Host expiration time (hours) |
| PingFlood | 5 | Ping flood protection |
| PingRate | 30000 | Ping rate (ms) |
| PongCache | 10 | Pong cache size |
| PongCount | 15 | Pongs per ping |
| EnableGGEP | true | Enable GGEP extensions |
| EnableOOB | false | Enable out-of-band queries |
| QueryHitUTF8 | true | Use UTF-8 for QueryHit |
| QuerySearchUTF8 | true | Use UTF-8 for Query |
| QueryThrottle | 60 | Query throttle (seconds) |
| QueryGlobalThrottle | 120 | Global query throttle (ticks) |
| MulticastPingRate | 120 | Multicast ping rate (ticks) |
| MaxHostsInPongs | 25 | Max hosts per pong |

### 🌐 Gnutella2 Settings

| Setting | Default | Description |
|---------|---------|-------------|
| ClientMode | MODE_AUTO | Operation mode (AUTO/LEAF/HUB) |
| Enabled | true | Enable Gnutella2 network |
| EnableAlways | false | Always enable on startup |
| HubVerified | false | Hub verification status |
| NumHubs | 3 | Number of hubs for leaf |
| NumLeafs | 50 | Number of leafs for hub |
| NumPeers | 4 | Number of peers for hub |
| PingRelayLimit | 10 | Max leafs to forward ping to |
| UdpMTU | 500 | UDP maximum transmission unit |
| UdpBuffers | 256 | UDP buffer count |
| UdpInFrames | 256 | UDP input frames |
| UdpOutFrames | 256 | UDP output frames |
| UdpGlobalThrottle | 200 | UDP global throttle |
| UdpOutExpire | 26000 | UDP output expiration (ms) |
| UdpOutResend | 4000 | UDP resend timeout (ms) |
| UdpInExpire | 30000 | UDP input expiration (ms) |
| LNIPeriod | 300000 | Local node info period (ms) |
| KHLPeriod | 300000 | Known hub list period (ms) |
| KHLHubCount | 100 | Hubs in known hub list |
| HAWPeriod | 300000 | Hub address watch period (ms) |
| HubHorizonSize | 100 | Hub horizon size |
| HostCurrent | 0 | Current host count |
| HostCount | 10 | Hosts in X-Try-Hubs |
| HostExpire | 2 | Host expiration time (hours) |
| PingRate | 30000 | Ping rate (ms) |
| QueryThrottle | 60 | Query throttle (seconds) |
| QueryGlobalThrottle | 8 | Global query throttle (per sec) |
| QueryHostDeadline | 30000 | Query host deadline (ms) |
| RequeryDelay | 4 | UDP re-query delay (hours) |
| QueryLimit | 5000 | Query limit |
| HubHorizonSize | 100 | Hub horizon size |

### 🫂 eDonkey2000 Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Enabled | true | Enable ED2K network |
| EnableAlways | false | Always enable on startup |
| EnableKad | true | Enable Kademlia DHT |
| ShowInterface | true | Show ED2K interface elements |
| FastConnect | true | Try 2 servers for faster connection |
| ForceHighID | false | Force reconnection for low ID |
| NumServers | 1 | Number of server connections |
| MaxLinks | 500 | Maximum ED2K client links |
| MaxResults | 500 | Maximum search results |
| MaxShareCount | 1000 | Maximum files shared with server |
| ServerWalk | true | Enable global server UDP walk |
| StatsGlobalThrottle | 30 | Global server stats throttle |
| QueryGlobalThrottle | 120 | Global search throttle |
| StatsServerThrottle | 60 | Per-server stats throttle |
| GetSourcesThrottle | 30 | GetSources throttle |
| QueryFileThrottle | 10 | Per-file GetSources throttle |
| QueueRankThrottle | 10 | Queue rank update throttle |
| QueryThrottle | 60 | Per-server query throttle |
| PacketThrottle | 500 | ED2K packet rate limiter |
| SourceThrottle | 1000 | ED2K source rate limiter |
| AutoDiscovery | true | Auto-discover server lists |
| LearnNewServers | true | Learn new servers from servers |
| LearnNewServersClient | true | Learn new servers from clients |
| ServerListURL | Default URL | Server list download URL |
| RequestPipe | 3 | Simultaneous requests per connection |
| RequestSize | 180000 | Request chunk size |
| FrameSize | 1024 | Frame size |
| ReAskTime | 1800000 | Re-ask time (ms) |
| DequeueTime | 60000 | Dequeue timeout (ms) |
| ExtendedRequest | 2 | Extended request count |
| SendPortServer | false | Send port to servers |
| MagnetSearch | true | Search for magnets over ED2K |
| MinServerFileSize | 0 | Minimum server share size (KB) |
| DefaultServerFlags | 0 | Default server flags |
| LargeFileSupport | true | Support 64-bit file sizes |
| Endgame | true | Enable endgame mode |

### 🧲 BitTorrent Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Enabled | true | Enable BitTorrent protocol |
| EnableAlways | false | Always enable on startup |
| PeerID | "PE" + version | Custom peer ID |
| TorrentCreatorPath | "" | Path to torrent creator |
| DefaultTracker | "" | Default tracker URL |
| DefaultTrackerPeriod | 300000 | Default tracker period (ms) |
| TorrentCodePage | 0 | Code page for torrents |
| LinkTimeout | 30000 | Link timeout (ms) |
| LinkPing | 5000 | Link ping timeout (ms) |
| RequestPipe | 128 | Request pipeline size |
| RequestSize | 16384 | Request chunk size |
| RequestLimit | 256 | Request limit |
| RandomPeriod | 30000 | Random period (ms) |
| SourceExchangePeriod | 60000 | Source exchange period (ms) |
| UtPexPeriod | 60000 | uTorrent PEX period (ms) |
| UploadCount | 4 | Maximum torrent uploads |
| DownloadConnections | 200 | Maximum torrent connections |
| DownloadTorrents | 5 | Maximum simultaneous torrents |
| HostExpire | 7200 | DHT host expiration (seconds) |
| ConnectThrottle | 60 | DHT connect throttle (seconds) |
| QueryHostDeadline | 15000 | DHT query deadline (ms) |
| EnableDHT | true | Enable mainline DHT |
| EnablePromote | true | Promote downloads to torrents |
| Endgame | true | Enable endgame mode |
| AutoSeed | true | Auto-seed completed torrents |
| AutoMerge | true | Auto-merge with local files |
| AutoClear | false | Auto-clear completed torrents |
| ClearRatio | 100 | Share ratio for auto-clear (%) |
| BandwidthPercentage | 100 | Bandwidth usage percentage |
| TrackerKey | false | Send key to trackers |
| PreferenceBTSources | true | Prefer BT sources |
| SkipPaddingFiles | true | Skip padding files |
| SkipTrackerFiles | true | Skip tracker files |

---

## 🎨 Interface & Skin Settings

### 🎨 Interface Settings

| Setting | Default | Description |
|---------|---------|-------------|
| AutoComplete | true | Use text field histories |
| CoolMenuEnable | true | Use skinned menus |
| LowResMode | false | Enable low resolution mode |
| PreferImageServices | false | Skip ATL image loading |
| DisplayScaling | 100 | Windows DPI scaling (%) |
| RefreshRateGraph | 72 | Graph refresh rate (ms) |
| RefreshRateText | 650 | Text refresh rate (ms) |
| RefreshRateUI | varies | UI refresh rate (ms) |
| SaveOpenWindows | varies | Remember open windows |
| TipDelay | 250 | Tooltip delay (ms) |
| TipAlpha | 240 | Tooltip transparency |
| TipSearch | true | Show search tooltips |
| TipDownloads | true | Show download tooltips |
| TipUploads | true | Show upload tooltips |
| TipLibrary | true | Show library tooltips |
| TipNeighbours | true | Show neighbor tooltips |
| TipMedia | true | Show media tooltips |
| Snarl | true | Enable Snarl notifications |
| SearchWindowsLimit | 10 | Max open search windows |
| BrowseWindowsLimit | 12 | Max open browse windows |

### 🎨 Skin Settings

| Setting | Default | Description |
|---------|---------|-------------|
| DropMenu | false | Enable drop-down menus |
| DropMenuLabel | 0 | Drop menu label length |
| MenuBorders | true | Show menu borders |
| MenuGripper | true | Show menu grippers |
| RoundedSelect | false | Use rounded selection |
| FrameEdge | true | Show frame edges |
| ButtonEdge | 4 | Button edge width (px) |
| MenubarHeight | 28 | Menu bar height (px) |
| StatusbarHeight | 0 | Status bar height (px) |
| ToolbarHeight | 28 | Toolbar height (px) |
| TaskbarHeight | 26 | Task bar height (px) |
| TaskbarTabWidth | 200 | Task bar tab width (px) |
| GroupsbarHeight | 24 | Groups bar height (px) |
| HeaderbarHeight | 64 | Header bar height (px) |
| MonitorbarWidth | 120 | Monitor bar width (px) |
| SidebarWidth | varies | Side bar width (px) |
| SidebarPadding | 12 | Side bar padding (px) |
| Splitter | 6 | Splitter width (px) |
| RowSize | varies | List row height (px) |
| LibIconsX | 220 | Library icon width (px) |
| LibIconsY | 56 | Library icon height (px) |
| AltIcons | varies | Use alternate icons |
| HiRes | false | High resolution mode |

### 🔤 Font Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Quality | 0 | Font rendering quality |
| DefaultSize | varies | Default font size (px) |
| DefaultFont | varies | Default font face |
| SystemLogFont | varies | System log font face |
| PacketDumpFont | varies | Packet dump font face |

### 🪟 Window Settings

| Setting | Default | Description |
|---------|---------|-------------|
| RunWizard | false | Show setup wizard |
| RunWarnings | false | Show warning dialogs |
| RunPromote | false | Show promotion dialogs |

### 🛠️ Toolbar Settings

| Setting | Default | Description |
|---------|---------|-------------|
| ShowRemote | true | Show remote toolbar |
| ShowMonitor | true | Show monitor toolbar |

---

## 📋 Configuration Notes

### ⚙️ Settings Storage
- **Location**: Settings are stored in XML format in `%APPDATA%\Envy\`
- **Auto-save**: Most settings are saved automatically when changed
- **Backup**: Settings can be exported/imported via Tools → Settings
- **Reset**: Individual sections or all settings can be reset to defaults

### 🔄 Settings Categories
- **Registry Settings**: Stored in Windows Registry (legacy)
- **XML Settings**: Modern settings stored in XML files
- **Live Settings**: Runtime status and warning flags
- **Protocol Settings**: Network-specific configuration

### ⚠️ Important Notes
- **Restart Required**: Some settings require application restart to take effect
- **Network Impact**: Many settings affect network connectivity and performance
- **Security**: Some settings have security implications (remote access, sharing)
- **Performance**: Bandwidth and connection settings significantly impact performance

### 🛠️ Advanced Configuration
- **Command Line**: Override settings via command line parameters
- **Environment**: Some settings can be controlled via environment variables
- **Registry**: Legacy settings may still exist in Windows Registry
- **Profiles**: Multiple configuration profiles can be maintained

---

**Document Version:** 1.0
**Envy Version:** 4.0+
**Last Updated:** January 18, 2026
**Total Settings:** 500+ configurable options
