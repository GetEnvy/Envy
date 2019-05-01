//
// Settings.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008-2016
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

#include "Envy.h"


enum
{
	bits = 1, Bytes = 8, Kilobits = 1024, KiloBytes = 8192
};

class CSettingsItem;


class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

public:

	struct sGeneral
	{
		CString		Path;					// Installation path for Envy
		CString		UserPath;				// Path for user data. (May be the same as above for single user installs)
		CString		DataPath;				// Full path for user data. (Same as above plus \Data\)
		CString		AntiVirus;				// Anti-virus path or CLSID
		bool		MultiUser;				// Multiuser installation
		bool		DialogScan;				// Create "C:\Dialogs.xml" translation template, with Skin engine in "dialog scan" mode.
		bool		DebugLog;				// Create a log file
		bool		SearchLog;				// Display search facility log information
		DWORD		LogLevel;				// Log severity (0 - MSG_ERROR .. 4 - MSG_DEBUG)
		DWORD		MaxDebugLogSize;		// Max size of the log file
		DWORD		DiskSpaceWarning;		// Value at which to warn the user about low disk space
		DWORD		DiskSpaceStop;			// Value at which to pause all downloads due to low disk space
		DWORD		MinTransfersRest;		// For how long at least to suspend Transfers each round
		DWORD		SmartVersion;			// Settings version
		DWORD		GUIMode;
		DWORD		CloseMode;
		bool		TrayMinimise;
		bool		ShowTimestamp;
		bool		SizeLists;
		bool		HashIntegrity;
		bool		RatesInBytes;			// Show speeds in bits or Bytes per second
		DWORD		RatesUnit;				// Units that rates are to be displayed in (B/KB/MB)
		DWORD		LockTimeout;			// Timeout release for overloaded lock attempts
		DWORD		LastSettingsIndex;		// Top item index of Advanced Settings list
		CString		LastSettingsPage;		// Last selected Settings dialog page
		CString		Language;
		bool		LanguageRTL;			// Right-to-Left GUI
		bool		LanguageDefault;		// Assume English
		bool		DebugBTSources;			// Display received sources for BT download when seeding
		bool		AlwaysOpenURLs;
#ifdef XPSUPPORT
		bool		IgnoreXPLimits;			// Ignore the presence of Windows XPsp2 limits  (Was IgnoreXPsp2)
		bool		ItWasLimited;			// If user patches half-open connection limit change settings back to full speed
#endif
		bool		FirstRun;				// Original installation
		bool		Running;				// ToDo: Detect abnormal shutdown on startup
	} General;

	struct sVersionCheck
	{
		DWORD		NextCheck;
		CString		Quote;
		bool		UpdateCheck;			// Does Envy check for new versions?
	//	CString		UpdateCheckURL;			// UPDATE_URL (Envy.h)
		CString		UpgradeVersion;
		CString		UpgradeFile;
		CString		UpgradeSize;
		CString		UpgradeDate;
		CString		UpgradeSHA1;
		CString		UpgradeTiger;
		CString		UpgradePrompt;
		CString		UpgradeSources;
	} VersionCheck;

	struct sInterface
	{
		bool		AutoComplete;			// Use text field histories
		bool		CoolMenuEnable;			// Use skinned menus
		bool		LowResMode;
		bool		SaveOpenWindows;		// Remember open windows next session
		DWORD		RefreshRateGraph;		// Data display update in milliseconds (set speed)
		DWORD		RefreshRateText;		// Data display update in milliseconds
		DWORD		RefreshRateUI;			// Button availability update in milliseconds (~4 chances per second)
		DWORD		DisplayScaling;			// Windows High DPI Modes 100/125/150/200 (96dpi = 100%)
		DWORD		TipDelay;
		DWORD		TipAlpha;
		bool		TipShadow;
		bool		TipSearch;
		bool		TipLibrary;
		bool		TipDownloads;
		bool		TipUploads;
		bool		TipNeighbours;
		bool		TipMedia;
		bool		Snarl;					// Use Snarl notifications (getsnarl.info)
		DWORD		SearchWindowsLimit;		// Maximum number of opened Search windows
		DWORD		BrowseWindowsLimit;		// Maximum number of opened Browse Host windows
	} Interface;

	struct sSkin
	{
		bool		DropMenu;
		DWORD		DropMenuLabel;
		bool		MenuBorders;
		bool		MenuGripper;
		bool		RoundedSelect;
		bool		AltIcons;
		bool		FrameEdge;
		DWORD		ButtonEdge;
		DWORD		MenubarHeight;
		DWORD		ToolbarHeight;
		DWORD		TaskbarHeight;
		DWORD		TaskbarTabWidth;
		DWORD		GroupsbarHeight;
		DWORD		HeaderbarHeight;
		DWORD		MonitorbarWidth;
		DWORD		SidebarWidth;
		DWORD		SidebarPadding;
		DWORD		Splitter;
		DWORD		LibIconsX;
		DWORD		LibIconsY;
		DWORD		RowSize;				// ~17 pixel ITEM_HEIGHT in custom-draw lists (or ~26 HiRes)
		DWORD		IconSize;				// 16 (or 24 HiRes)
		bool		HiRes;					// 1.5x Mode
	} Skin;

	struct sWindows
	{
		bool		RunWizard;
		bool		RunWarnings;
		bool		RunPromote;
	} Windows;

	struct sToolbars
	{
		bool		ShowRemote;
		bool		ShowMonitor;
	} Toolbars;

	struct sFonts
	{
		CString		DefaultFont;			// Main font
		CString		PacketDumpFont;			// Packet Window font
		CString		SystemLogFont;			// System Window font
		DWORD		DefaultSize;			// Basic font size
		DWORD		Quality;				// Cleartype
	} Fonts;

	struct sLibrary
	{
		bool		WatchFolders;
		DWORD		WatchFoldersTimeout;
		bool		VirtualFiles;
		bool		SourceMesh;
		DWORD		SourceExpire;
		DWORD		TigerHeight;
		DWORD		QueryRouteSize;
		DWORD		HistoryTotal;
		DWORD		HistoryDays;
		DWORD		ThumbQuality;
		DWORD		ThumbSize;
		DWORD		TreeSize;
		DWORD		PanelSize;
		bool		ShowPanel;
		bool		ShowVirtual;
	//	bool		ShowCoverArt;
		CString		SchemaURI;
		CString		FilterURI;
		string_set	SafeExecute;
		string_set	PrivateTypes;
		DWORD		GhostLimit;				// Maximum number of retained files (~1000)
		bool		CreateGhosts;			// Default action in the delete file dialog
		bool		HashWindow;				// Display annoying hashing window
		bool		HighPriorityHash;		// Use high priority hashing or not
		DWORD		HighPriorityHashing;	// desired speed in MB/s when hashing with hi priority
		DWORD		LowPriorityHashing;		// desired speed in MB/s when hashing with low priority
		DWORD		ManyFilesWarning;		// Too many files selected warning.  0 -ask user, 1 -no, 2 -yes
		DWORD		ExecuteFilesLimit;		// Minimum number of selected files to trigger warning  (TOO_MANY_FILES_LIMIT)
		DWORD		MaliciousFileCount;		// Minimum number of duplicate files to trigger warning
		DWORD		MaliciousFileSize;		// Size range for which to trigger malicious software search
		string_set	MaliciousFileTypes;		// Malicious software file extensions
		bool		MarkFileAsDownload;		// Mark downloaded file using NTFS stream as Internet Explorer
		bool		UseFolderGUID;			// Save/Load folder GUID using NTFS stream
		bool		UseCustomFolders;		// Use desktop.ini
		bool		UseWindowsLibrary;		// Use Windows 7 Libraries
		bool		ScanAPE;				// Enable .ape,.mac,.apl metadata extraction by internals
		bool		ScanASF;				// Enable .asf,.wma,.wmv metadata extraction by internals
		bool		ScanAVI;				// Enable .avi metadata extraction by internals
		bool		ScanCHM;				// Enable .chm metadata extraction by internals
		bool		ScanEXE;				// Enable .exe,.dll metadata extraction by internals
		bool		ScanFLV;				// Enable .flv metadata extraction by internals
		bool		ScanImage;				// Enable .jpg,.jpeg,.gif,.png,.bmp metadata extraction by internals
		bool		ScanMP3;				// Enable .mp3 +.aac,.flac,.mpc metadata extraction by internals
		bool		ScanMPEG;				// Enable .mpeg,.mpg metadata extraction by internals
		bool		ScanMSI;				// Enable .msi metadata extraction by internals
		bool		ScanOGG;				// Enable .ogg metadata extraction by internals
		bool		ScanPDF;				// Enable .pdf metadata extraction by internals
		bool		ScanProperties;			// Enable Windows properties metadata extraction by internals
		bool		SmartSeriesDetection;	// Organize video files in Library by using predefined patterns
		CString		URLExportFormat;		// Template for URL export
		CString		LastUsedView;			// Name of last folder view used
	} Library;

	struct sWebServices
	{
		CString		BitprintsAgent;
		CString		BitprintsWebView;
		CString		BitprintsWebSubmit;
		CString		BitprintsXML;
		bool		BitprintsOkay;
	//	bool		ShareMonkeyOkay;
	//	bool		ShareMonkeySaveThumbnail;
	//	CString		ShareMonkeyCid;			// Affiliate ID
	//	CString		ShareMonkeyBaseURL;		// Redirect obsolete sharemonkey*com, does not exist
	} WebServices;

	struct sSearch
	{
		CString		LastSchemaURI;
		CString		BlankSchemaURI;
		bool		AutoPreview;			// Default thumbnail selected hit
		bool		AdultFilter;
		bool		AdvancedPanel;
		bool		ResultsPanel;			// Search Results Box state (open or closed) (Was Settings.General.SearchPanelResults)
		bool		SearchPanel;
		bool		HideSearchPanel;
		bool		HighlightNew;
		bool		ExpandMatches;
		bool		SwitchToTransfers;
		bool		SchemaTypes;
		bool		ShowNames;
		DWORD		FilterMask;
		CString		MonitorSchemaURI;
		CString		MonitorFilter;
		DWORD		MonitorQueue;
		DWORD		BrowseTreeSize;
		DWORD		DetailPanelSize;
		bool		DetailPanelVisible;
		DWORD		MaxPreviewLength;
		DWORD		SpamFilterThreshold;	// Percentage of spam hits which triggers file sources to be treated as a spam
		DWORD		GeneralThrottle;		// A general throttle for how often each individual search may run. Low values may cause source finding to get overlooked.
		DWORD		ClearPrevious;			// Clear previous search results? 0 - ask user; 1 - no; 2 - yes.
		bool		SanityCheck;			// Drop hits of banned hosts
	} Search;

	struct sMediaPlayer
	{
		bool		EnablePlay;
		bool		EnableEnqueue;
		bool		Repeat;
		bool		Random;
		MediaZoom	Zoom;
		double		Aspect;
		double		Volume;
		DWORD		ListSize;
		bool		ListVisible;
		bool		StatusVisible;
		CString		MediaServicesCLSID;
	//	CString		Mpeg1PreviewCLSID;
	//	CString		Mp3PreviewCLSID;
	//	CString		AviPreviewCLSID;
	//	CString		VisWrapperCLSID;
	//	CString		VisSoniqueCLSID;		// No Sonique Vis plugin support available
		CString		VisCLSID;
		CString		VisPath;
		DWORD		VisSize;
		bool		ShortPaths;				// Rarely set true, some players handle unicode paths differently but can launch files using 8.3 paths
		string_set	ServicePath;			// Keeps track of external players, trailing asterisk indicates selected
		string_set	FileTypes;
	} MediaPlayer;

	struct sWeb
	{
		bool		Magnet;					// Handle magnet: links  (.magma 0.2 files?)
		bool		Gnutella;				// Handle gnutella: links
		bool		Foxy;					// Handle foxy: links (slightly altered magnet)
		bool		ED2K;					// Handle ed2k: links
		bool		DC;						// Handle dcfile: dchub: links
		bool		Piolet;					// Handle .mp2p links (obsolete?)
		bool		Torrent;				// Handle .torrent files
	//	bool		Metalink;				// ToDo: Handle .metalink, .meta4, .magma files
	} Web;

	struct sConnection
	{
		bool		AutoConnect;
		DWORD		FirewallState;
		CString		OutHost;
		CString		InHost;
		DWORD		InPort;
		bool		InBind;
		bool		RandomPort;
		DWORD		InSpeed;				// Inbound internet connection speed in Kilobits/seconds
		DWORD		OutSpeed;				// Outbound internet connection speed in Kilobits/seconds
		bool		IgnoreLocalIP;			// Ingnore all 'local' (LAN) IPs
		bool		IgnoreOwnIP;			// Do not accept any ports on your external IP as a source
		bool		IgnoreOwnUDP;			// Do not accept any ports on your external IP as a source of UDP
		DWORD		TimeoutConnect;
		DWORD		TimeoutHandshake;
		DWORD		TimeoutTraffic;
		DWORD		SendBuffer;
		bool		RequireForTransfers;	// Only upload/download to connected networks
		DWORD		ConnectThrottle;		// Delay between connection attempts to neighbors (milliseconds)
		DWORD		FailurePenalty;			// Delay after connection failure (seconds, default = 300) (Neighbour connections)
		DWORD		FailureLimit;			// Max allowed connection failures (default = 3) (Neighbour connections)
		bool		DetectConnectionLoss;	// Detect loss of internet connection
		bool		DetectConnectionReset;	// Detect regaining of internet connection
		bool		ForceConnectedState;	// Force WinINet into a connected state on startup. (Put IE into online mode)
		bool		SlowConnect;			// Connect to one network at a time. Don't download while connecting. (XPsp2)
		bool		EnableFirewallException;// Create Firewall exception at startup
		bool		DeleteFirewallException;// Delete Firewall exception on shutdown
		bool		EnableUPnP;				// UPnP: Enable forwarded ports on startup
		bool		DeleteUPnPPorts;		// UPnP: Delete forwarded ports on shutdown
		DWORD		UPnPTimeout;			// UPnP: Maximum time for waiting on any device response (milliseconds)
		DWORD		UPnPRefreshTime;		// UPnP: Refresh time of port mappings
		bool		SkipWANPPPSetup;		// UPnP: Skip WANPPPConn1 device setup
		bool		SkipWANIPSetup;			// UPnP: Skip WANIPConn1 device setup
		bool		EnableBroadcast;		// Send and accept broadcast packets (default false, for LAN)
		bool		EnableMulticast;		// Send and accept multi-cast packets (default true)
		bool		MulticastLoop;			// Use multi-cast loopback (default false, for debugging)
		DWORD		MulticastTTL;			// TTL for multi-cast packets (default 1)
		DWORD		ZLibCompressionLevel;	// ZLib compression level: 0-9 (default 1)
	} Connection;

	struct sBandwidth
	{
		DWORD		Request;
		DWORD		HubIn;
		DWORD		HubOut;
		DWORD		LeafIn;
		DWORD		LeafOut;
		DWORD		PeerIn;
		DWORD		PeerOut;
		DWORD		UdpOut;
		DWORD		Downloads;				// Inbound speed limit in Bytes/seconds
		DWORD		Uploads;				// Outbound speed limit in Bytes/seconds
		DWORD		HubUploads;
	} Bandwidth;

	struct sCommunity
	{
		bool		ChatEnable;				// Is chat enabled with compatible clients?
		bool		ChatAllNetworks;		// Is chat allowed over other protocols? (ed2k, etc)
		bool		ChatFilter;				// Filter out chat spam
		bool		ChatFilterED2K;			// Filter known ed2k spam. (pretty bad- always on)
		bool		ChatCensor;				// Censor 'bad' words from chat. (Uses adult filter)
		bool		Timestamp;
		bool		ServeProfile;
		bool		ServeFiles;
		DWORD		AwayMessageIdleTime;	// Time in secs of idle system time before showing away message
		DWORD		UserPanelSize;			// Width of chat users sidepanel (in pixels)
	} Community;

	struct sDiscovery
	{
		DWORD		AccessThrottle;
		DWORD		Lowpoint;
		DWORD		FailureLimit;
		DWORD		UpdatePeriod;
		DWORD		DefaultUpdate;
		DWORD		BootstrapCount;
		DWORD		CacheCount;				// Limit ability to learn new caches
		bool		EnableG1GWC;
	} Discovery;

	struct sGnutella
	{
		DWORD		ConnectFactor;			// Number of hosts simultaneously tried when connecting to single hub
		bool		DeflateHub2Hub;
		bool		DeflateLeaf2Hub;
		bool		DeflateHub2Leaf;
		DWORD		MaxResults;				// Maximum new results we want on single Search button press
		DWORD		MaxHits;				// Maximum file hits in search result (divided to packets by HitsPerPacket)
		DWORD		MaxHitWords;			// Maximum number of words in a hit filename  ~30
		DWORD		MaxHitLength;			// Maximum number of chars in a hit filename  ~160
		DWORD		MaximumPacket;			// Drop packets large than specified (32-256 KB)
		DWORD		HitsPerPacket;			// Maximum file hits in single search result packet
		DWORD		RouteCache;				// Life time of node route (seconds)
		DWORD		HostCacheSize;			// Number of hosts of each type in Host cache
		DWORD		HostCacheView;
		DWORD		ConnectThrottle;		// Delay between connection attempts for same host (seconds)
		bool		SpecifyProtocol;		// Specify G1 or G2 when initiating a connection
	} Gnutella;

	struct sGnutella1
	{
		DWORD		ClientMode;				// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_ULTRAPEER
		bool		Enabled;				// Was Gnutella1.EnableToday
		bool		EnableAlways;			// Do  Gnutella1.EnableStartup or EnableDefault?
		bool		ShowInterface;			// Allow hiding some UI features
		DWORD		NumHubs;				// Number of ultrapeers a leaf has
		DWORD		NumLeafs;				// Number of leafs an ultrapeer has
		DWORD		NumPeers;				// Number of peers an ultrapeer has
		DWORD		PacketBufferSize;		// Number of packets in packet buffer
		DWORD		PacketBufferTime;		// Life time of packet in packet buffer before drop (milliseconds)
		DWORD		DefaultTTL;
		DWORD		SearchTTL;
		DWORD		TranslateTTL;
		DWORD		MaximumTTL;
		DWORD		MaximumQuery;
		DWORD		RequeryDelay;
		DWORD		HostCount;				// Number of hosts in X-Try-Ultrapeers
		DWORD		HostExpire;
		DWORD		PingFlood;
		DWORD		PingRate;
		DWORD		PongCache;
		DWORD		PongCount;
		bool		EnableGGEP;
		bool		EnableOOB;
		bool		VendorMsg;
	//	DWORD		HitQueueLimit;			// Protect G1 clients from badly configured queues
		bool		QueryHitUTF8;			// Use UTF-8 encoding to read Gnutella1 QueryHit packets
		bool		QuerySearchUTF8;		// Use UTF-8 encoding to create Gnutella1 Query packets
		DWORD		QueryThrottle;
		DWORD		QueryGlobalThrottle;	// Multicast query rate (ticks)
		DWORD		MulticastPingRate;		// Multicast ping rate (ticks)
		DWORD		MaxHostsInPongs;		// The number of hosts included in the response of pings having SCP GGEP block
	} Gnutella1;

	struct sGnutella2
	{
		DWORD		ClientMode;				// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_HUB
		bool		Enabled;				// Was Gnutella2.EnableToday
		bool		EnableAlways;			// Do  Gnutella2.EnableStartup ?
	//	bool		ShowInterface;			// Never allow hiding some UI features
		bool		HubVerified;
		DWORD		NumHubs;				// Number of hubs a leaf has
		DWORD		NumLeafs;				// Number of leafs a hub has
		DWORD		NumPeers;				// Number of peers a hub has
		DWORD		PingRelayLimit;			// Number of other leafs to forward a /PI/UDP to: 10 - 30
		DWORD		UdpMTU;
		DWORD		UdpBuffers;
		DWORD		UdpInFrames;
		DWORD		UdpOutFrames;
		DWORD		UdpGlobalThrottle;
		DWORD		UdpOutExpire;
		DWORD		UdpOutResend;
		DWORD		UdpInExpire;
		DWORD		LNIPeriod;
		DWORD		KHLPeriod;
		DWORD		KHLHubCount;
		DWORD		HAWPeriod;
		DWORD		HubHorizonSize;
		DWORD		HostCurrent;
		DWORD		HostCount;				// Number of hosts in X-Try-Hubs
		DWORD		HostExpire;
		DWORD		PingRate;
		DWORD		QueryThrottle;			// Throttle for G2 neighbor searches (sec) (was QueryHostThrottle)
		DWORD		QueryGlobalThrottle;	// Max G2 query rate (Cannot exceed 8/sec)
		DWORD		QueryHostDeadline;
		DWORD		RequeryDelay;			// Throttle for G2 neighbor UDP searches (hr?)
		DWORD		QueryLimit;
	} Gnutella2;

	struct seDonkey
	{
		bool		Enabled;				// Was eDonkey.EnableToday
		bool		EnableAlways;			// Do  eDonkey.EnableStartup ?
		bool		ShowInterface;			// Allow hiding some UI features
		bool		FastConnect;			// Try connecting to 2 servers to get online faster
		bool		ForceHighID;			// Reconnect if low-id
		DWORD		NumServers;				// 1 Connection
		DWORD		MaxLinks;				// Max ed2k client links
		DWORD		MaxResults;
		DWORD		MaxShareCount;			// Hard limit on file list sent to server
		bool		ServerWalk;				// Enable global UDP walk of servers
		DWORD		StatsGlobalThrottle;	// Global throttle for server UDP stats requests
		DWORD		QueryGlobalThrottle;	// Global throttle for all ed2k searches (TCP, UDP, manual and auto)
		DWORD		StatsServerThrottle;	// Max rate at which an individual server can be asked for stats
		DWORD		GetSourcesThrottle;		// Max rate a general GetSources can done
		DWORD		QueryFileThrottle;		// Max rate a file can have GetSources done
		DWORD		QueueRankThrottle;		// How frequently queue ranks are sent
		DWORD		QueryThrottle;			// Max rate at which an individual server can be queried (sec) (was QueryServerThrottle)
		DWORD		PacketThrottle;			// ED2K packet rate limiter
		DWORD		SourceThrottle;			// ED2K source rate limiter
		bool		MetAutoQuery;			// Auto query for a new server list
		bool		LearnNewServers;		// Get new servers from servers
		bool		LearnNewServersClient;	// Get new servers from clients
		CString		ServerListURL;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		FrameSize;
		DWORD		ReAskTime;
		DWORD		DequeueTime;
		DWORD		ExtendedRequest;
		bool		SendPortServer;			// Send port in tag to ed2k servers. (not needed for newer servers)
		bool		MagnetSearch;			// Search for magnets over ed2k (lower server load)
		DWORD		MinServerFileSize;		// Minimum size a file in the library must be in order to be included in the server file list. (In KB)
		DWORD		DefaultServerFlags;		// Default server flags (for UDP searches)
		bool		LargeFileSupport;		// Allow 64 bit file sizes (for server)
		bool		Endgame;				// Allow endgame mode when completing downloads. (Download same chunk from multiple sources)
	} eDonkey;

	struct sDC
	{
		bool		Enabled;				// Was DC.EnableToday
		bool		EnableAlways;			// Do  DC.EnableStartup ?
		bool		ShowInterface;			// Allow hiding some UI features
		DWORD		NumServers;				// Default 1 hub
		DWORD		QueryThrottle;			// Throttle for DC++ neighbor searches (s), default 2 min delay
		DWORD		ReAskTime;				// How often to re-ask a remote client about download (ms), default every minute
		DWORD		DequeueTime;			// Timeout for remote client confirmation of upload queue (ms), default 5 min
	//	DWORD		HubListQueryPeriod; 	// Auto-update hublist fetching (d), default should be ~weeks
		bool		HubListAutoQuery;		// Auto query for a new server list
		CString		HubListURL;				// Default hublist.xml.bz2 location
	} DC;

	struct sBitTorrent
	{
		bool		Enabled;				// Was BitTorrent.EnableToday
		bool		EnableAlways;			// Do  BitTorrent.EnableStartup ?
	//	bool		ShowInterface;			// Never need hiding some UI features?
		CString		PeerID;					// Use this peer ID for trackers in form of "CCvvvv" where "CC" = agent code ("PE" "SZ" "UT" etc.), v.v.v.v = version
		CString		TorrentCreatorPath;		// Location of the program used to create .torrent files
		CString		DefaultTracker;
		DWORD		DefaultTrackerPeriod;	// Delay between tracker contact attempts if one is not specified by tracker
		DWORD		TorrentCodePage;		// The code page to assume for a .torrent file if it isn't UTF-8
		DWORD		LinkTimeout;
		DWORD		LinkPing;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		RequestLimit;
		DWORD		RandomPeriod;
		DWORD		SourceExchangePeriod;
		DWORD		UtPexPeriod;			// uTorrent Peer Exchange in Seconds
		DWORD		UploadCount;			// Number of active torrent uploads allowed
		DWORD		DownloadConnections;	// Number of active torrent connections allowed
		DWORD		DownloadTorrents;		// Number of torrents to download at once
		DWORD		HostExpire;				// DHT hosts expiration time (seconds)	Was DhtPruneTime
		DWORD		ConnectThrottle;		// Throttle for DHT request (sec)
		DWORD		QueryHostDeadline;		// Time to wait for DHT reply (sec)
		bool		EnableDHT;				// Enable Mainline DHT protocol
		bool		EnablePromote;			// Enable regular to torrent download promotion
		bool		Endgame;				// Allow endgame mode when completing torrents. (Download same chunk from multiple sources)
		bool		AutoSeed;				// Automatically re-seed most recently completed torrent on start-up
		bool		AutoMerge;				// Automatically merge download with local files on start-up
		bool		AutoClear;				// Clear completed torrents when they meet the required share ratio
		DWORD		ClearRatio;				// Share ratio a torrent must reach to be cleared. (Minimum 100%)
		DWORD		BandwidthPercentage;	// Percentage of bandwidth to use when BT active.
		bool		TrackerKey;				// Send a key (random value) to trackers
		bool		PreferenceBTSources;	// Preference downloading from BT sources where appropriate
		bool		SkipPaddingFiles;		// Deselect BitComet "____padding_file_..."
		bool		SkipTrackerFiles;		// Deselect "Torrent downloaded from ... .txt" files
	} BitTorrent;

	struct sDownloads
	{
		CString		IncompletePath;			// Where incomplete downloads are stored
		CString		CompletePath;			// Where downloads are moved when they complete
		CString		TorrentPath;			// Where .torrent files are stored
		CString		CollectionPath;			// Where .collection and .co files are stored
		DWORD		BufferSize;
		DWORD		SparseThreshold;		// NTFS 'sparse files' are not used on files below this size. (0 = Disable)
		DWORD		MaxAllowedFailures;
		DWORD		MaxFiles;				// How many files download at once
		DWORD		MaxTransfers;			// How many total transfers take place
		DWORD		MaxFileTransfers;		// How many transfers are allowed per file
		DWORD		MaxFileSearches;		// Number number of files over the download limit that prepare to start. (Search, etc)
		DWORD		MaxConnectingSources;	// The maximum number of sources that can be in the 'connecting' state. (Important for XPsp2)
		DWORD		MinSources;				// The minimum number of sources a download has before Envy regards it as having a problem
		DWORD		ConnectThrottle;		// Delay between download attempts. (Very important for routers)
		DWORD		QueueLimit;				// Longest queue to wait in. (0 to disable. This should be >800 or 0 to get good performance from ed2k)
		DWORD		RetryDelay;
		DWORD		SearchPeriod;
		DWORD		StarveGiveUp;			// How long (in hours) before Envy will give up and try another download if it gets no data. (+ 0-9 h, depending on sources)
		DWORD		StarveTimeout;			// How long (in ticks) Envy will starve without new downloaded data before trying to search for more sources
		DWORD		PushTimeout;
		bool		StaggardStart;
		bool		AllowBackwards;			// Permit download to run in reverse when appropriate
		DWORD		ChunkSize;
		DWORD		ChunkStrap;
		bool		Metadata;
		bool		NeverDrop;				// Do not drop bad sources (may pollute source list with many dead sources)
		bool		VerifyFiles;
		bool		VerifyTiger;
		bool		VerifyED2K;
		bool		VerifyTorrent;
		bool		RequestHash;
		bool		RequestHTTP11;
		bool		RequestURLENC;
		bool		RenameExisting;			// Append to filename on move when confilict (or fail)	ToDo: CString for "%s (%i)" or "%s.%i" or "" ?
		bool		FlushPD;				// Force .pd flush (Used once, why change at all?)
		DWORD		SaveInterval;
		bool		ShowSources;
		bool		SimpleBar;				// Displays a simplified progress bar (lower CPU use)
		bool		ShowPercent;			// Display small green % complete bar on progress graphic
		bool		ShowGroups;
		bool		AutoExpand;
		bool		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
		bool		ShowMonitorURLs;
		bool		SortColumns;			// Allow user to sort downloads by clicking column headers
		bool		SortSources;			// Automatically sort sources (Status, protocol, queue)
		DWORD		SourcesWanted;			// Number of sources Envy 'wants'. (Will not request more than this number of sources from ed2k)
		DWORD		MaxReviews;				// Maximum number of reviews to store per download
		bool		NoRandomFragments;		// ToDo: Streaming Download and Rarest Piece Selection
		bool		WebHookEnable;
		string_set	WebHookExtensions;
	} Downloads;

	struct sUploads
	{
		string_set	BlockAgents;
		DWORD		MaxPerHost;				// Simultaneous uploads per remote client
		DWORD		FreeBandwidthValue;
		DWORD		FreeBandwidthFactor;
		DWORD		ClampdownFactor;
		DWORD		ClampdownFloor;
		DWORD		ChunkSize;
		bool		FairUseMode;			// Limit unknown audio/video to 10% share per remote client
		bool		ThrottleMode;
		DWORD		QueuePollMin;
		DWORD		QueuePollMax;
		DWORD		RotateChunkLimit;
		bool		SharePartials;
		bool		ShareTiger;
		bool		ShareHashset;
		bool		ShareMetadata;
		bool		SharePreviews;
		bool		DynamicPreviews;
		DWORD		PreviewQuality;
		DWORD		PreviewTransfers;
		bool		AllowBackwards;			// Data sent from end of range to beginning where supported
		bool		HubUnshare;
		bool		AutoClear;				// Remove completed uploads
		DWORD		ClearDelay;				// Delay between auto-clears
		DWORD		History;				// Completed uploads display count
		DWORD		FilterMask;
		DWORD		RewardQueuePercentage;	// The percentage of each reward queue reserved for uploaders
	} Uploads;

	struct sIRC
	{
		COLORREF	Colors[12];
		bool		Show;
		bool		Timestamp;
		bool		FloodEnable;
		DWORD		FloodLimit;
		CString		OnConnect;
		CString		UserName;
		CString		RealName;
		CString		Nick;
		CString		Alternate;
		CString		ServerName;
		DWORD		ServerPort;
		DWORD		FontSize;
		CString		ScreenFont;
	} IRC;

	struct sLive
	{
		bool		DiskSpaceStop;			// Has Envy paused all downloads due to critical disk space?
		bool		DiskSpaceWarning;		// Has the user been warned of low disk space?
		bool		DiskWriteWarning;		// Has the user been warned of write problems?
		bool		AdultWarning;			// Has the user been warned about the adult filter?
		bool		UploadLimitWarning;		// Has the user been warned about the ed2k/BT ratio?
		bool		QueueLimitWarning;		// Has the user been warned about limiting the max Q position accepted?
		bool		DonkeyServerWarning;	// Has the user been warned about having an empty server list?
		bool		DefaultED2KServersLoaded; // Has Envy already loaded default ED2K servers?
		bool		DefaultDCServersLoaded;	  // Has Envy already loaded default DC++ servers?
		bool		MaliciousWarning;		// Is the warning dialog triggered? (Single case at startup)
		CString		LastDuplicateHash;		// Stores the hash of the file about which the warning was shown
		DWORD		BandwidthScaleIn;		// MonitorBar Download slider setting
		DWORD		BandwidthScaleOut;		// MonitorBar Upload slider setting
		bool		LoadWindowState;
		bool		AutoClose;
		bool		FirstRun;				// Is this the first time Envy is being run?
	} Live;

	struct sRemote
	{
		bool		Enable;
		CString		Username;
		CString		Password;
	} Remote;

	struct sScheduler
	{
		bool		Enable;					// Enable the scheduler
		bool		ForceShutdown;
		DWORD		ValidityPeriod;			// Active trigger window in minutes
	} Scheduler;

	struct sSecurity
	{
		DWORD		DefaultBan;				// Custom Ban expiration in seconds
		DWORD		ListRangeLimit;			// External Blacklist max load for a given IP range (single-line limit)
	} Security;

	struct sExperimental
	{
		bool		EnableDIPPSupport;		// Handle GDNA host cache exchange
		bool		LAN_Mode;				// Local Private Network optimizations  (Force G2 only, was #ifdef LAN_MODE)
	} Experimental;


// Attributes : Item List
public:
	enum Type
	{
		setNull, setBool, setString, setFont, setPath, setReadOnly
	};

	class Item
	{
	public:
		inline Item(const LPCTSTR szSection, const LPCTSTR szName, bool* const pBool, const bool bDefault, const bool bHidden) throw()
			: m_szSection		( szSection )
			, m_szName			( szName )
			, m_pBool			( pBool )
			, m_pDword			( NULL )
			, m_pFloat			( NULL )
			, m_pString 		( NULL )
			, m_pSet			( NULL )
			, m_BoolDefault		( bDefault )
			, m_StringDefault	( NULL )
			, m_DwordDefault	( 0 )
			, m_FloatDefault	( 0.0 )
			, m_nScale			( 1 )
			, m_nMin			( 0 )
			, m_nMax			( 1 )
			, m_szSuffix		( NULL )
			, m_bHidden 		( bHidden )
			, m_nType			( setBool )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, DWORD* const pDword, const DWORD nDefault, const DWORD nScale, const DWORD nMin, const DWORD nMax, const LPCTSTR szSuffix, const bool bHidden) throw()
			: m_szSection		( szSection )
			, m_szName			( szName )
			, m_pBool			( NULL )
			, m_pDword			( pDword )
			, m_pFloat			( NULL )
			, m_pString 		( NULL )
			, m_pSet			( NULL )
			, m_BoolDefault		( false )
			, m_StringDefault	( NULL )
			, m_DwordDefault	( nDefault )
			, m_FloatDefault	( 0.0 )
			, m_nScale			( nScale )
			, m_nMin			( nMin )
			, m_nMax			( nMax )
			, m_szSuffix		( szSuffix )
			, m_bHidden 		( bHidden )
			, m_nType			( setNull )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, DOUBLE* const pFloat, const DOUBLE dDefault, const bool bHidden) throw()
			: m_szSection		( szSection )
			, m_szName			( szName )
			, m_pBool			( NULL )
			, m_pDword			( NULL )
			, m_pFloat			( pFloat )
			, m_pString 		( NULL )
			, m_pSet			( NULL )
			, m_BoolDefault 	( false )
			, m_StringDefault	( NULL )
			, m_DwordDefault	( 0 )
			, m_FloatDefault	( dDefault )
			, m_nScale			( 0 )
			, m_nMin			( 0 )
			, m_nMax			( 0 )
			, m_szSuffix		( NULL )
			, m_bHidden 		( bHidden )
			, m_nType			( setNull )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, CString* const pString, const LPCTSTR szDefault, const bool bHidden, const Type nType = setString) throw()
			: m_szSection		( szSection )
			, m_szName			( szName )
			, m_pBool			( NULL )
			, m_pDword			( NULL )
			, m_pFloat			( NULL )
			, m_pString 		( pString )
			, m_pSet			( NULL )
			, m_BoolDefault		( false )
			, m_StringDefault	( szDefault )
			, m_DwordDefault	( 0 )
			, m_FloatDefault	( 0.0 )
			, m_nScale			( 0 )
			, m_nMin			( 0 )
			, m_nMax			( 0 )
			, m_szSuffix		( NULL )
			, m_bHidden 		( bHidden )
			, m_nType			( nType )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, string_set* const pSet, const LPCTSTR szDefault, const bool bHidden) throw()
			: m_szSection		( szSection )
			, m_szName			( szName )
			, m_pBool			( NULL )
			, m_pDword			( NULL )
			, m_pFloat			( NULL )
			, m_pString 		( NULL )
			, m_pSet			( pSet )
			, m_BoolDefault		( false )
			, m_StringDefault	( szDefault )
			, m_DwordDefault	( 0 )
			, m_FloatDefault	( 0.0 )
			, m_nScale			( 0 )
			, m_nMin			( 0 )
			, m_nMax			( 0 )
			, m_szSuffix		( NULL )
			, m_bHidden 		( bHidden )
			, m_nType			( setNull )
		{
		}

		inline bool operator==(LPVOID p) const
		{
			return ( m_pBool == p || m_pDword == p || m_pFloat == p || m_pString == p || m_pSet == p );
		}

		void	Load();
		void	Save() const;
		void	Normalize();
		bool	IsDefault() const;
		void	SetDefault();
		template< class T > void	SetRange(T& pCtrl);

		const LPCTSTR		m_szSection;
		const LPCTSTR		m_szName;

		bool* const			m_pBool;
		DWORD* const		m_pDword;
		DOUBLE* const		m_pFloat;
		CString* const		m_pString;
		string_set* const	m_pSet;

		const bool			m_BoolDefault;
		const DWORD			m_DwordDefault;
		const DOUBLE		m_FloatDefault;
		const LPCTSTR		m_StringDefault;

		const DWORD			m_nScale;
		const DWORD			m_nMin;
		const DWORD			m_nMax;
		const LPCTSTR		m_szSuffix;

		const bool			m_bHidden;
		const Type			m_nType;
	};

protected:
	CList< Item* >	m_pItems;

	typedef std::map< CString, Item* > CSettingsMap;
	CSettingsMap m_pSettingsTable;

public:
	void	Load();
	void	Save(BOOL bShutdown = FALSE);
	inline POSITION	GetHeadPosition() const
	{
		return m_pItems.GetHeadPosition();
	}
	inline Item*	GetNext(POSITION& rPosition) const
	{
		return m_pItems.GetNext( rPosition );
	}
	void	Normalize(LPVOID pSetting);
	bool	IsDefault(LPVOID pSetting) const;
	void	SetDefault(LPVOID pSetting);

	template< class T >
	void	SetRange(LPVOID pSetting, T& pCtrl)
	{
		for ( POSITION pos = m_pItems.GetHeadPosition(); pos; )
		{
			Item* pItem = m_pItems.GetNext( pos );
			if ( *pItem == pSetting )
			{
				pItem->SetRange< T >( pCtrl );
				break;
			}
		}
	}

	BOOL	LoadWindow(LPCTSTR pszName, CWnd* pWindow);
	void	SaveWindow(LPCTSTR pszName, CWnd* pWindow);
	BOOL	LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort = 0);
	void	SaveList(LPCTSTR pszName, CListCtrl* pCtrl);

	static void		LoadSet(string_set* pSet, LPCTSTR pszString);
	static CString	SaveSet(const string_set* pSet);

	const CString	SmartSpeed(QWORD nVolume, int nVolumeUnits = Bytes, bool bTruncate = false) const;	// Convert speeds into formatted strings
	const CString	SmartVolume(QWORD nVolume, int nVolumeUnits = Bytes, bool bTruncate = false) const;	// Convert sizes into formatted strings
	QWORD	ParseVolume(const CString& strVolume, int nReturnUnits = Bytes) const;						// Convert size string into desired units
	DWORD	GetOutgoingBandwidth() const;																// Returns available outgoing bandwidth in KB/s
	void	OnChangeConnectionSpeed();
	bool	GetValue(LPCTSTR pszPath, VARIANT* value);

	BOOL	CheckStartup();
	void	SetStartup(BOOL bStartup);

	void	ClearSearches();	// Delete search history

protected:
	void	SmartUpgrade();

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, bool* const pBool, const bool bDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pBool, bDefault, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, DWORD* const pDword, const DWORD nDefault, DWORD nScale = 0, DWORD nMin = 0, DWORD nMax = 0, LPCTSTR szSuffix = NULL, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pDword, nDefault, nScale, nMin, nMax, szSuffix, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, DOUBLE* const pDouble, const DOUBLE dDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pDouble, dDefault, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, CString* const pString, const LPCTSTR szDefault = NULL, const bool bHidden = false, const Type nType = setString) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pString, szDefault, bHidden, nType ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, string_set* const pSet, const LPCTSTR szDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pSet, szDefault, bHidden ) );
	}

// Inlines
public:
	// CSettings configurable user agent (Client Name + Version)
	inline const CString& SmartAgent() const throw()
	{
		return theApp.m_sSmartAgent;
	}

private:
	CSettings(const CSettings&);
	CSettings& operator=(const CSettings&);
};

extern CSettings Settings;

enum
{
	GUI_WINDOWED, GUI_TABBED, GUI_BASIC
};

enum
{
	MODE_AUTO, MODE_LEAF, MODE_HUB, MODE_ULTRAPEER = MODE_HUB
};

enum
{
	CONNECTION_AUTO, CONNECTION_FIREWALLED, CONNECTION_OPEN, CONNECTION_OPEN_TCPONLY, CONNECTION_OPEN_UDPONLY
};
