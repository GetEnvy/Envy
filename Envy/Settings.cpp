//
// Settings.cpp
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2016 and Shareaza 2002-2008
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#include "StdAfx.h"
#include "Settings.h"
#include "Envy.h"
#include "Registry.h"
#include "Schema.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Debug

#define SMART_VERSION	1000	// 1.0.0.0 (60)		(ToDo: Use INTERNAL_VERSION?)

#define KiloByte		( 1024 )
#define MegaByte		( KiloByte * 1024 )
//#define GigaByte		( MegaByte * 1024 )
//#define TeraByte		( GigaByte * 1024ui64 )
#define KiloFloat		( 1024.0f )
#define MegaFloat		( KiloFloat * 1024.0f )
#define GigaFloat		( MegaFloat * 1024.0f )
#define TeraFloat		( GigaFloat * 1024.0f )
#define PetaFloat		( TeraFloat * 1024.0f )
#define ExaFloat		( PetaFloat * 1024.0f )

CSettings Settings;

//////////////////////////////////////////////////////////////////////
// CSettings construction

CSettings::CSettings()
{
	// Reset 'live' values.
	Live.FirstRun					= false;
	Live.AutoClose					= false;
	Live.AdultWarning				= false;
	Live.MaliciousWarning			= false;
	Live.DiskSpaceStop				= false;
	Live.DiskSpaceWarning			= false;
	Live.DiskWriteWarning			= false;
	Live.UploadLimitWarning			= false;
	Live.QueueLimitWarning			= false;
	Live.DonkeyServerWarning		= false;
	Live.DefaultED2KServersLoaded	= false;
	Live.LoadWindowState			= false;
	Live.BandwidthScaleIn			= 101;
	Live.BandwidthScaleOut			= 101;
	Live.LastDuplicateHash			= L"";
}

CSettings::~CSettings()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings load

void CSettings::Load()
{
	// Add all settings
	Add( L"", L"DebugBTSources", &General.DebugBTSources, false );
	Add( L"", L"DebugLog", &General.DebugLog, false );
	Add( L"", L"DiskSpaceStop", &General.DiskSpaceStop, 50, 1, 0, 1000, L" MB" );
	Add( L"", L"DiskSpaceWarning", &General.DiskSpaceWarning, 550, 1, 1, 2000, L" MB" );
	Add( L"", L"HashIntegrity", &General.HashIntegrity, true );
	Add( L"", L"MaxDebugLogSize", &General.MaxDebugLogSize, 10*MegaByte, MegaByte, 0, 100, L" MB" );
	Add( L"", L"MinTransfersRest", &General.MinTransfersRest, 40, 1, 1, 100, L" ms" );
	Add( L"", L"MultiUser", &General.MultiUser, false, true );
	Add( L"", L"Path", &General.Path, NULL, false, setReadOnly );
	Add( L"", L"UserPath", &General.UserPath, NULL, false, setReadOnly );
	Add( L"", L"DataPath", &General.DataPath, NULL, false, setReadOnly );
	Add( L"", L"LogLevel", &General.LogLevel, MSG_INFO, 1, MSG_ERROR, MSG_DEBUG, L" level" );
	Add( L"", L"SearchLog", &General.SearchLog, true );
	Add( L"", L"DialogScan", &General.DialogScan, false );

#ifndef NOXP
	//Add( L"", L"ItWasLimited", &General.ItWasLimited, false, true );
	Add( L"Settings", L"IgnoreXPLimits", &General.IgnoreXPLimits, false );
#endif

	Add( L"Settings", L"AlwaysOpenURLs", &General.AlwaysOpenURLs, false );
	Add( L"Settings", L"AntiVirus", &General.AntiVirus, L"" );
	Add( L"Settings", L"CloseMode", &General.CloseMode, 0, 1, 0, 3 );
	Add( L"Settings", L"FirstRun", &General.FirstRun, true, true );
	Add( L"Settings", L"GUIMode", &General.GUIMode, GUI_TABBED );
	Add( L"Settings", L"Language", &General.Language, L"en" );
	Add( L"Settings", L"LanguageRTL", &General.LanguageRTL, false );
	Add( L"Settings", L"LanguageDefault", &General.LanguageDefault, true );
	Add( L"Settings", L"LastSettingsPage", &General.LastSettingsPage, NULL, true );
	Add( L"Settings", L"LastSettingsIndex", &General.LastSettingsIndex, 0 );
	Add( L"Settings", L"LockTimeout", &General.LockTimeout, 2000, 1, 1, 30000, L" ms" );
	Add( L"Settings", L"RatesInBytes", &General.RatesInBytes, true );
	Add( L"Settings", L"RatesUnit", &General.RatesUnit, 0, 1, 0, 3 );
	Add( L"Settings", L"Running", &General.Running, false, true );
	Add( L"Settings", L"ShowTimestamp", &General.ShowTimestamp, true );
	Add( L"Settings", L"SizeLists", &General.SizeLists, false );
	Add( L"Settings", L"SmartVersion", &General.SmartVersion, SMART_VERSION );
	Add( L"Settings", L"TrayMinimise", &General.TrayMinimise, false );

	Add( L"VersionCheck", L"NextCheck", &VersionCheck.NextCheck, 0 );
	Add( L"VersionCheck", L"Quote", &VersionCheck.Quote );
	Add( L"VersionCheck", L"UpdateCheck", &VersionCheck.UpdateCheck, true );
//	Add( L"VersionCheck", L"UpdateCheckURL", &VersionCheck.UpdateCheckURL, UPDATE_URL );
	Add( L"VersionCheck", L"UpgradeFile", &VersionCheck.UpgradeFile );
	Add( L"VersionCheck", L"UpgradePrompt", &VersionCheck.UpgradePrompt );
	Add( L"VersionCheck", L"UpgradeSHA1", &VersionCheck.UpgradeSHA1 );
	Add( L"VersionCheck", L"UpgradeSize", &VersionCheck.UpgradeSize );
	Add( L"VersionCheck", L"UpgradeSources", &VersionCheck.UpgradeSources );
	Add( L"VersionCheck", L"UpgradeTiger", &VersionCheck.UpgradeTiger );
	Add( L"VersionCheck", L"UpgradeVersion", &VersionCheck.UpgradeVersion );

	Add( L"Interface", L"AutoComplete", &Interface.AutoComplete, true );
	Add( L"Interface", L"CoolMenuEnable", &Interface.CoolMenuEnable, true );
	Add( L"Interface", L"LowResMode", &Interface.LowResMode, false );
	Add( L"Interface", L"SaveOpenWindows", &Interface.SaveOpenWindows, General.GUIMode != GUI_BASIC );
	Add( L"Interface", L"RefreshRateGraph", &Interface.RefreshRateGraph, 72, 1, 10, 60000, L" ms" );	// 30sec display areas
	Add( L"Interface", L"RefreshRateText", &Interface.RefreshRateText, 650, 1, 10, 10000, L" ms" );	// 3x per 2 sec.
	Add( L"Interface", L"RefreshRateUI", &Interface.RefreshRateUI, theApp.m_nWinVer < WIN_XP_64 ? 300 : 100, 1, 10, 2000, L" ms" );	// 3/10x per sec. (Button status)
	Add( L"Interface", L"TipDelay", &Interface.TipDelay, 500, 1, 100, 5000, L" ms" );
	Add( L"Interface", L"TipAlpha", &Interface.TipAlpha, 240, 1, 50, 255 );
	Add( L"Interface", L"TipDownloads", &Interface.TipDownloads, true );
	Add( L"Interface", L"TipUploads", &Interface.TipUploads, true );
	Add( L"Interface", L"TipLibrary", &Interface.TipLibrary, true );
	Add( L"Interface", L"TipNeighbours", &Interface.TipNeighbours, true );
	Add( L"Interface", L"TipMedia", &Interface.TipMedia, true );
	Add( L"Interface", L"TipSearch", &Interface.TipSearch, true );
	Add( L"Interface", L"TipShadow", &Interface.TipShadow, true );
	Add( L"Interface", L"Snarl", &Interface.Snarl, true );	// Use notifications (getsnarl.info)
	Add( L"Interface", L"SearchWindowsLimit", &Interface.SearchWindowsLimit, 10, 1, 0, 50, L" windows" );
	Add( L"Interface", L"BrowseWindowsLimit", &Interface.BrowseWindowsLimit, 12, 1, 0, 50, L" windows" );

	Add( L"Skin", L"DropMenu", &Skin.DropMenu, false );
	Add( L"Skin", L"DropMenuLabel", &Skin.DropMenuLabel, 0, 1, 0, 100, L" char" );
	Add( L"Skin", L"MenuBorders", &Skin.MenuBorders, true );
	Add( L"Skin", L"MenuGripper", &Skin.MenuGripper, true );
	Add( L"Skin", L"RoundedSelect", &Skin.RoundedSelect, false );
	Add( L"Skin", L"FrameEdge", &Skin.FrameEdge, true );
	Add( L"Skin", L"ButtonEdge", &Skin.ButtonEdge, 4, 1, 0, 100, L" px" );
	Add( L"Skin", L"MenubarHeight", &Skin.MenubarHeight, 28, 1, 0, 100, L" px" );
	Add( L"Skin", L"ToolbarHeight", &Skin.ToolbarHeight, 28, 1, 0, 100, L" px" );
	Add( L"Skin", L"TaskbarHeight", &Skin.TaskbarHeight, 26, 1, 0, 100, L" px" );
	Add( L"Skin", L"TaskbarTabWidth", &Skin.TaskbarTabWidth, 200, 1, 0, 1000, L" px" );
	Add( L"Skin", L"GroupsbarHeight", &Skin.GroupsbarHeight, 24, 1, 0, 100, L" px" );
	Add( L"Skin", L"HeaderbarHeight", &Skin.HeaderbarHeight, 64, 1, 0, 100, L" px" );
	Add( L"Skin", L"MonitorbarWidth", &Skin.MonitorbarWidth, 120, 1, 0, 1000, L" px" );
	Add( L"Skin", L"SidebarWidth", &Skin.SidebarWidth, 200, 1, 0, 500, L" px" );
	Add( L"Skin", L"SidebarPadding", &Skin.SidebarPadding, 12, 1, 0, 100, L" px" );
	Add( L"Skin", L"Splitter", &Skin.Splitter, 6, 1, 1, 100, L" px" );
	Add( L"Skin", L"RowSize", &Skin.RowSize, 17, 1, 16, 20, L" px" );
	Add( L"Skin", L"LibIconsX", &Skin.LibIconsX, 220, 1, 30, 500, L" px" );
	Add( L"Skin", L"LibIconsY", &Skin.LibIconsY, 56, 1, 30, 100, L" px" );
	Add( L"Skin", L"AltIcons", &Skin.AltIcons, theApp.m_nWinVer < WIN_10 );

	Add( L"Windows", L"RunWizard", &Windows.RunWizard, false );
	Add( L"Windows", L"RunWarnings", &Windows.RunWarnings, false );
	Add( L"Windows", L"RunPromote", &Windows.RunPromote, false );

	Add( L"Toolbars", L"ShowRemote", &Toolbars.ShowRemote, true );
	Add( L"Toolbars", L"ShowMonitor", &Toolbars.ShowMonitor, true );

	Add( L"Fonts", L"Quality", &Fonts.Quality, 0, 1, 0, 6 );	// 	CLEARTYPE_QUALITY etc.
	Add( L"Fonts", L"DefaultSize", &Fonts.DefaultSize, 11, 1, 9, 12, L" px" );
#ifdef NOXP
	Add( L"Fonts", L"DefaultFont", &Fonts.DefaultFont, L"Segoe UI" , false, setFont );
	Add( L"Fonts", L"SystemLogFont", &Fonts.SystemLogFont, L"Segoe UI", false, setFont );
	Add( L"Fonts", L"PacketDumpFont", &Fonts.PacketDumpFont, L"Consolas", false, setFont );
#else // XP Supported
	Add( L"Fonts", L"DefaultFont", &Fonts.DefaultFont, theApp.m_bIsWinXP ? L"Tahoma" : L"Segoe UI" , false, setFont );
	Add( L"Fonts", L"SystemLogFont", &Fonts.SystemLogFont, theApp.m_bIsWinXP ? L"Tahoma" : L"Segoe UI", false, setFont );
	Add( L"Fonts", L"PacketDumpFont", &Fonts.PacketDumpFont, theApp.m_bIsWinXP ? L"Lucida Console" : L"Consolas", false, setFont );
#endif

	Add( L"Library", L"CreateGhosts", &Library.CreateGhosts, true );
	Add( L"Library", L"GhostLimit", &Library.GhostLimit, 2000, 1, 0, 100000, L" files" );
	Add( L"Library", L"FilterURI", &Library.FilterURI );
//	Add( L"Library", L"LastUsedView", &Library.LastUsedView );
	Add( L"Library", L"HashWindow", &Library.HashWindow, true );
	Add( L"Library", L"HighPriorityHash", &Library.HighPriorityHash, true );
	Add( L"Library", L"HighPriorityHashing", &Library.HighPriorityHashing, 32, 1, 2, 100, L" MB/s" );
	Add( L"Library", L"LowPriorityHashing", &Library.LowPriorityHashing, 4, 1, 1, 50, L" MB/s" );
	Add( L"Library", L"HistoryDays", &Library.HistoryDays, 10, 1, 0, 365, L" d" );
	Add( L"Library", L"HistoryTotal", &Library.HistoryTotal, 36, 1, 0, 150, L" files" );
	Add( L"Library", L"QueryRouteSize", &Library.QueryRouteSize, 20, 1, 8, 24 );
	Add( L"Library", L"MarkFileAsDownload", &Library.MarkFileAsDownload, true );
	Add( L"Library", L"ManyFilesWarning", &Library.ManyFilesWarning, 0, 1, 0, 2 );
	Add( L"Library", L"ExecuteFilesLimit", &Library.ExecuteFilesLimit, 20, 1, 0, 1000, L" files" );
	Add( L"Library", L"MaliciousFileCount", &Library.MaliciousFileCount, 5, 1, 2, 50, L" files" );
	Add( L"Library", L"MaliciousFileSize", &Library.MaliciousFileSize, 2*MegaByte, KiloByte, KiloByte, 10*MegaByte, L" KB" );
	Add( L"Library", L"MaliciousFileTypes", &Library.MaliciousFileTypes, L"|exe|com|bat|vbs|scr|zip|rar|ace|7z|cab|lzh|tar|tgz|bz2|wma|wmv|" );
	Add( L"Library", L"PrivateTypes", &Library.PrivateTypes, L"|vbs|js|jc!|fb!|bc!|!ut|db3|dbx|part|partial|pst|reget|getright|crdownload|pif|lnk|url|pd|sd|wab|m4p|infodb|racestats|svn|chk|tmp|temp|ini|inf|log|old|manifest|met|bak|$$$|---|~~~|###|__incomplete___|" );
	Add( L"Library", L"SafeExecute", &Library.SafeExecute, L"|3gp|7z|aac|ace|ape|asf|avi|bmp|cbr|cbz|co|collection|divx|envy|flv|flac|gif|iso|jpg|jpeg|lit|mid|mov|m1v|m2v|m3u|m4a|mka|mkv|mp2|mp3|mp4|mpa|mpe|mpg|mpeg|ogg|ogm|pdf|png|psk|qt|rar|rm|sks|swf|rtf|tar|tgz|torrent|txt|wav|zip|" );
	Add( L"Library", L"SchemaURI", &Library.SchemaURI, CSchema::uriAudio );
	Add( L"Library", L"ScanAPE", &Library.ScanAPE, true );
	Add( L"Library", L"ScanASF", &Library.ScanASF, true );
	Add( L"Library", L"ScanAVI", &Library.ScanAVI, true );
	Add( L"Library", L"ScanCHM", &Library.ScanCHM, true );
	Add( L"Library", L"ScanEXE", &Library.ScanEXE, true );
	Add( L"Library", L"ScanFLV", &Library.ScanFLV, true );
	Add( L"Library", L"ScanImage", &Library.ScanImage, true );
	Add( L"Library", L"ScanMP3", &Library.ScanMP3, true );
	Add( L"Library", L"ScanMPEG", &Library.ScanMPEG, true );
	Add( L"Library", L"ScanMSI", &Library.ScanMSI, true );
	Add( L"Library", L"ScanOGG", &Library.ScanOGG, true );
	Add( L"Library", L"ScanPDF", &Library.ScanPDF, true );
	Add( L"Library", L"ScanProperties", &Library.ScanProperties, true );
//	Add( L"Library", L"ShowCoverArt", &Library.ShowCoverArt, true );
	Add( L"Library", L"ShowPanel", &Library.ShowPanel, true );
	Add( L"Library", L"ShowVirtual", &Library.ShowVirtual, true );
	Add( L"Library", L"SourceMesh", &Library.SourceMesh, true );
	Add( L"Library", L"SourceExpire", &Library.SourceExpire, 24*60*60, 60, 60, 7*24*60*60, L" m" );
	Add( L"Library", L"TigerHeight", &Library.TigerHeight, 9, 1, 1, 64 );
	Add( L"Library", L"ThumbQuality", &Library.ThumbQuality, 75, 1, 10, 99, L" %" );
	Add( L"Library", L"ThumbSize", &Library.ThumbSize, 128, 1, 16, 256, L" px" );
	Add( L"Library", L"TreeSize", &Library.TreeSize, 200, 1, 0, 1024, L" px" );
	Add( L"Library", L"PanelSize", &Library.PanelSize, 120, 1, 0, 1024, L" px" );
	Add( L"Library", L"URLExportFormat", &Library.URLExportFormat, L"<a href=\"magnet:?xt=urn:bitprint:[SHA1].[TIGER]&amp;xt=urn:ed2khash:[ED2K]&amp;xt=urn:md5:[MD5]&amp;xl=[ByteSize]&amp;dn=[NameURI]\">[Name]</a><br>" );
	Add( L"Library", L"UseCustomFolders", &Library.UseCustomFolders, true );	// Desktop.ini
	Add( L"Library", L"UseWindowsLibrary", &Library.UseWindowsLibrary, theApp.m_nWinVer >= WIN_7 );
	Add( L"Library", L"UseFolderGUID", &Library.UseFolderGUID, true );
	Add( L"Library", L"VirtualFiles", &Library.VirtualFiles, false );
	Add( L"Library", L"WatchFolders", &Library.WatchFolders, true );
	Add( L"Library", L"WatchFoldersTimeout", &Library.WatchFoldersTimeout, 10, 1, 1, 60, L" s" );
	Add( L"Library", L"SmartSeriesDetection", &Library.SmartSeriesDetection, true );

	Add( L"WebServices", L"BitprintsAgent", &WebServices.BitprintsAgent, L"." );
	Add( L"WebServices", L"BitprintsOkay", &WebServices.BitprintsOkay, false, true );
	Add( L"WebServices", L"BitprintsWebSubmit", &WebServices.BitprintsWebSubmit, L"http://bitprints.getenvy.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&tag.ed2k.ed2khash=(ED2K)&(INFO)&a=(AGENT)&v=Q0.5&ref=envy" );
	Add( L"WebServices", L"BitprintsWebView", &WebServices.BitprintsWebView, L"http://bitprints.getenvy.com/lookup/(URN)?v=detail&ref=envy" );
	Add( L"WebServices", L"BitprintsXML", &WebServices.BitprintsXML, L"http://bitprints.getenvy.com/rdf/(SHA1)" );
//	Add( L"WebServices", L"ShareMonkeyCid", &WebServices.ShareMonkeyCid );
//	Add( L"WebServices", L"ShareMonkeyOkay", &WebServices.ShareMonkeyOkay, false, true );
//	Add( L"WebServices", L"ShareMonkeySaveThumbnail", &WebServices.ShareMonkeySaveThumbnail, false, true );
//	Add( L"WebServices", L"ShareMonkeyBaseURL", &WebServices.ShareMonkeyBaseURL, L"http://tools.sharemonkey.com/xml/" );	// Obsolete: Does not exist

	Add( L"Search", L"AdultFilter", &Search.AdultFilter, false );
	Add( L"Search", L"AutoPreview", &Search.AutoPreview, true );
	Add( L"Search", L"AdvancedPanel", &Search.AdvancedPanel, true );
	Add( L"Search", L"BlankSchemaURI", &Search.BlankSchemaURI, CSchema::uriAudio );
	Add( L"Search", L"BrowseTreeSize", &Search.BrowseTreeSize, 180 );
	Add( L"Search", L"DetailPanelSize", &Search.DetailPanelSize, 100 );
	Add( L"Search", L"DetailPanelVisible", &Search.DetailPanelVisible, true );
	Add( L"Search", L"ExpandMatches", &Search.ExpandMatches, false );
	Add( L"Search", L"FilterMask", &Search.FilterMask, 0x280 );	// 01010000000 (Reverse Order Options)
	Add( L"Search", L"GeneralThrottle", &Search.GeneralThrottle, 200, 1, 200, 1000, L" ms" );
	Add( L"Search", L"HideSearchPanel", &Search.HideSearchPanel, false );
	Add( L"Search", L"HighlightNew", &Search.HighlightNew, true );
	Add( L"Search", L"LastSchemaURI", &Search.LastSchemaURI );
	Add( L"Search", L"MaxPreviewLength", &Search.MaxPreviewLength, 20*KiloByte, KiloByte, 1, 5*KiloByte, L" KB" );
	Add( L"Search", L"MonitorFilter", &Search.MonitorFilter );
	Add( L"Search", L"MonitorQueue", &Search.MonitorQueue, 128, 1, 1, 4096 );
	Add( L"Search", L"MonitorSchemaURI", &Search.MonitorSchemaURI, CSchema::uriAudio );
	Add( L"Search", L"ResultsPanel", &Search.ResultsPanel, true );
	Add( L"Search", L"SearchPanel", &Search.SearchPanel, true );
	Add( L"Search", L"ShowNames", &Search.ShowNames, true );
	Add( L"Search", L"SchemaTypes", &Search.SchemaTypes, true );
	Add( L"Search", L"SpamFilterThreshold", &Search.SpamFilterThreshold, 20, 1, 0, 100, L"%" );
	Add( L"Search", L"SwitchToTransfers", &Search.SwitchToTransfers, true );
	Add( L"Search", L"SanityCheck", &Search.SanityCheck, true );
	Add( L"Search", L"ClearPrevious", &Search.ClearPrevious, 0, 1, 0, 2 );

	Add( L"MediaPlayer", L"Aspect", &MediaPlayer.Aspect, smaDefault );
	Add( L"MediaPlayer", L"EnableEnqueue", &MediaPlayer.EnableEnqueue, true );
	Add( L"MediaPlayer", L"EnablePlay", &MediaPlayer.EnablePlay, true );
	Add( L"MediaPlayer", L"FileTypes", &MediaPlayer.FileTypes, L"|aac|asx|wax|m3u|wvx|wmx|asf|wav|snd|au|aif|aifc|aiff|flac|mp3|ogg|wma|cda|mid|rmi|midi|avi|flv|mkv|mpeg|mpg|m1v|mp2|mp4|mpa|mpe|ogm|wmv|" );
	Add( L"MediaPlayer", L"ListSize", &MediaPlayer.ListSize, 200 );
	Add( L"MediaPlayer", L"ListVisible", &MediaPlayer.ListVisible, true );
	Add( L"MediaPlayer", L"Random", &MediaPlayer.Random, false );
	Add( L"MediaPlayer", L"Repeat", &MediaPlayer.Repeat, false );
	Add( L"MediaPlayer", L"ServicePath", &MediaPlayer.ServicePath, L"" );
	Add( L"MediaPlayer", L"ShortPaths", &MediaPlayer.ShortPaths, false );
	Add( L"MediaPlayer", L"StatusVisible", &MediaPlayer.StatusVisible, true );
	Add( L"MediaPlayer", L"VisPath", &MediaPlayer.VisPath );
	Add( L"MediaPlayer", L"VisSize", &MediaPlayer.VisSize, 1 );
	Add( L"MediaPlayer", L"Volume", &MediaPlayer.Volume, 1.0f );
	Add( L"MediaPlayer", L"Zoom", (DWORD*)&MediaPlayer.Zoom, smzOne );

	Add( L"MediaPlayer", L"MediaServicesCLSID", &MediaPlayer.MediaServicesCLSID, L"{CCE7B109-15D6-4223-B6FF-0C6C851B6680}" );
//	Add( L"MediaPlayer", L"AviPreviewCLSID", &MediaPlayer.AviPreviewCLSID, L"{394011F0-6D5C-42a3-96C6-24B9AD6B010C}" );
//	Add( L"MediaPlayer", L"Mp3PreviewCLSID", &MediaPlayer.Mp3PreviewCLSID, L"{BF00DBCC-90A2-4f46-8171-7D4F929D035F}" );
//	Add( L"MediaPlayer", L"Mpeg1PreviewCLSID", &MediaPlayer.Mpeg1PreviewCLSID, L"{9AA8DF47-B8FE-47da-AB1A-2DAA0DA0B646}" );
//	Add( L"MediaPlayer", L"VisWrapperCLSID", &MediaPlayer.VisWrapperCLSID, L"{C3B7B25C-6B8B-481A-BC48-59F9A6F7B69A}" );
//	Add( L"MediaPlayer", L"VisSoniqueCLSID", &MediaPlayer.VisSoniqueCLSID, L"{D07E630D-A850-4f11-AD29-3D3848B67EFE}" );
	Add( L"MediaPlayer", L"VisCLSID", &MediaPlayer.VisCLSID, L"{591A5CFF-3172-4020-A067-238542DDE9C2}" );

	Add( L"Web", L"Torrent", &Web.Torrent, ( CRegistry::GetString( L"Software\\Classes\\.torrent", NULL, NULL, NULL ).GetLength() < 4 ) );
//	Add( L"Web", L"Metalink", &Web.Metalink, true );
	Add( L"Web", L"Magnet", &Web.Magnet, true );
	Add( L"Web", L"Gnutella", &Web.Gnutella, true );
	Add( L"Web", L"ED2K", &Web.ED2K, true );
	Add( L"Web", L"DC", &Web.DC, true );
	Add( L"Web", L"Foxy", &Web.Foxy, true );
	Add( L"Web", L"Piolet", &Web.Piolet, true );

	Add( L"Connection", L"AutoConnect", &Connection.AutoConnect, true );
	Add( L"Connection", L"ConnectThrottle", &Connection.ConnectThrottle, 0, 1, 0, 5000, L" ms" );
	Add( L"Connection", L"DeleteFirewallException", &Connection.DeleteFirewallException, false );
	Add( L"Connection", L"DeleteUPnPPorts", &Connection.DeleteUPnPPorts, true );
	Add( L"Connection", L"DetectConnectionLoss", &Connection.DetectConnectionLoss, true );
	Add( L"Connection", L"DetectConnectionReset", &Connection.DetectConnectionReset, false );
	Add( L"Connection", L"EnableFirewallException", &Connection.EnableFirewallException, true );
	Add( L"Connection", L"EnableUPnP", &Connection.EnableUPnP, true );
	Add( L"Connection", L"FailureLimit", &Connection.FailureLimit, 3, 1, 1, 512 );
	Add( L"Connection", L"FailurePenalty", &Connection.FailurePenalty, 300, 1, 30, 3600, L" s" );
	Add( L"Connection", L"FirewallState", &Connection.FirewallState, CONNECTION_AUTO, 1, CONNECTION_AUTO, CONNECTION_OPEN_UDPONLY );
	Add( L"Connection", L"ForceConnectedState", &Connection.ForceConnectedState, true );
	Add( L"Connection", L"IgnoreLocalIP", &Connection.IgnoreLocalIP, true );
	Add( L"Connection", L"IgnoreOwnIP", &Connection.IgnoreOwnIP, true );
	Add( L"Connection", L"IgnoreOwnUDP", &Connection.IgnoreOwnUDP, true );
	Add( L"Connection", L"InBind", &Connection.InBind, false );
	Add( L"Connection", L"InHost", &Connection.InHost );
	Add( L"Connection", L"InPort", &Connection.InPort, protocolPorts[ PROTOCOL_NULL ], 1, 1, 65535 );	// 6480... or 6346?
	Add( L"Connection", L"InSpeed", &Connection.InSpeed, 4096 );	// , 25000
	Add( L"Connection", L"OutSpeed", &Connection.OutSpeed, 768 );	// , 15000
	Add( L"Connection", L"OutHost", &Connection.OutHost );
	Add( L"Connection", L"RandomPort", &Connection.RandomPort, false );
	Add( L"Connection", L"RequireForTransfers", &Connection.RequireForTransfers, true );
	Add( L"Connection", L"SendBuffer", &Connection.SendBuffer, 8*KiloByte, 1, 0, 64*KiloByte, L" B" );
	Add( L"Connection", L"SkipWANIPSetup", &Connection.SkipWANIPSetup, false );
	Add( L"Connection", L"SkipWANPPPSetup", &Connection.SkipWANPPPSetup, false );
	Add( L"Connection", L"SlowConnect", &Connection.SlowConnect, false );
	Add( L"Connection", L"TimeoutConnect", &Connection.TimeoutConnect, 15*1000, 1000, 1, 2*60, L" s" );
	Add( L"Connection", L"TimeoutHandshake", &Connection.TimeoutHandshake, 40*1000, 1000, 1, 5*60, L" s" );
	Add( L"Connection", L"TimeoutTraffic", &Connection.TimeoutTraffic, 140*1000, 1000, 10, 60*60, L" s" );
	Add( L"Connection", L"UPnPTimeout", &Connection.UPnPTimeout, 5*1000, 1, 0, 60*1000, L" ms" );
	Add( L"Connection", L"UPnPRefreshTime", &Connection.UPnPRefreshTime, 30*60*1000, 60*1000, 5, 24*60, L" m" );
	Add( L"Connection", L"ZLibCompressionLevel", &Connection.ZLibCompressionLevel, 8, 1, 0, 9 );

	Add( L"Bandwidth", L"Downloads", &Bandwidth.Downloads, 0 );
	Add( L"Bandwidth", L"HubIn", &Bandwidth.HubIn, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"HubOut", &Bandwidth.HubOut, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"HubUploads", &Bandwidth.HubUploads, 50, 1, 1, 90, L" %" );
	Add( L"Bandwidth", L"LeafIn", &Bandwidth.LeafIn, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"LeafOut", &Bandwidth.LeafOut, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"PeerIn", &Bandwidth.PeerIn, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"PeerOut", &Bandwidth.PeerOut, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"Request", &Bandwidth.Request, 32*128, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"UdpOut", &Bandwidth.UdpOut, 0, 128, 0, 8192, L" Kb/s" );
	Add( L"Bandwidth", L"Uploads", &Bandwidth.Uploads, 0 );

	Add( L"Community", L"AwayMessageIdleTime", &Community.AwayMessageIdleTime, 20*60, 60, 5, 60, L" m" );
	Add( L"Community", L"ChatAllNetworks", &Community.ChatAllNetworks, true );
	Add( L"Community", L"ChatCensor", &Community.ChatCensor, false );
	Add( L"Community", L"ChatEnable", &Community.ChatEnable, true );
	Add( L"Community", L"ChatFilter", &Community.ChatFilter, true );
	Add( L"Community", L"ChatFilterED2K", &Community.ChatFilterED2K, true );
	Add( L"Community", L"ServeFiles", &Community.ServeFiles, true );
	Add( L"Community", L"ServeProfile", &Community.ServeProfile, true );
	Add( L"Community", L"Timestamp", &Community.Timestamp, true );
	Add( L"Community", L"UserPanelSize", &Community.UserPanelSize, 150, 1, 0, 1024, L" px" );

	Add( L"Discovery", L"AccessThrottle", &Discovery.AccessThrottle, 60*60, 60, 1, 180, L" m" );
	Add( L"Discovery", L"BootstrapCount", &Discovery.BootstrapCount, 10, 1, 0, 20 );
	Add( L"Discovery", L"CacheCount", &Discovery.CacheCount, 50, 1, 1, 256 );
	Add( L"Discovery", L"DefaultUpdate", &Discovery.DefaultUpdate, 60*60, 60, 1, 60*24, L" m" );
	Add( L"Discovery", L"EnableG1GWC", &Discovery.EnableG1GWC, true );
	Add( L"Discovery", L"FailureLimit", &Discovery.FailureLimit, 2, 1, 1, 512 );
	Add( L"Discovery", L"Lowpoint", &Discovery.Lowpoint, 10, 1, 1, 512 );
	Add( L"Discovery", L"UpdatePeriod", &Discovery.UpdatePeriod, 30*60, 60, 1, 60*24, L" m" );

	Add( L"Gnutella", L"ConnectFactor", &Gnutella.ConnectFactor, 4, 1, 1, 20, L"x" );
	Add( L"Gnutella", L"ConnectThrottle", &Gnutella.ConnectThrottle, 30, 1, 0, 60*60, L" s" );
	Add( L"Gnutella", L"DeflateHub2Hub", &Gnutella.DeflateHub2Hub, true );
	Add( L"Gnutella", L"DeflateHub2Leaf", &Gnutella.DeflateHub2Leaf, true );
	Add( L"Gnutella", L"DeflateLeaf2Hub", &Gnutella.DeflateLeaf2Hub, true );
	Add( L"Gnutella", L"HostCacheSize", &Gnutella.HostCacheSize, 1024, 1, 32, 16384, L" hosts" );
	Add( L"Gnutella", L"HostCacheView", &Gnutella.HostCacheView, PROTOCOL_ED2K );
	Add( L"Gnutella", L"HitsPerPacket", &Gnutella.HitsPerPacket, 8, 1, 1, 255, L" files" );
	Add( L"Gnutella", L"MaxResults", &Gnutella.MaxResults, 150, 1, 1, 300, L" hits" );
	Add( L"Gnutella", L"MaxHits", &Gnutella.MaxHits, 64, 1, 0, 4096, L" files" );
	Add( L"Gnutella", L"MaxHitWords", &Gnutella.MaxHitWords, 30, 1, 3, 100, L" words" );
	Add( L"Gnutella", L"MaxHitLength", &Gnutella.MaxHitLength, 180, 1, 50, 255, L" chars" );
	Add( L"Gnutella", L"MaximumPacket", &Gnutella.MaximumPacket, 64*KiloByte, KiloByte, 32, 256, L" KB" );
	Add( L"Gnutella", L"RouteCache", &Gnutella.RouteCache, 600, 60, 1, 120, L" m" );
	Add( L"Gnutella", L"SpecifyProtocol", &Gnutella.SpecifyProtocol, true );

	Add( L"Gnutella1", L"ShowInterface", &Gnutella1.ShowInterface, true );
	Add( L"Gnutella1", L"ClientMode", &Gnutella1.ClientMode, MODE_LEAF, 1, MODE_AUTO, MODE_HUB );		// ToDo: MODE_LEAF until Ultrapeer updated/validated
	Add( L"Gnutella1", L"DefaultTTL", &Gnutella1.DefaultTTL, 3, 1, 1, 3 );
	Add( L"Gnutella1", L"EnableAlways", &Gnutella1.EnableAlways, true );
	Add( L"Gnutella1", L"EnableGGEP", &Gnutella1.EnableGGEP, true );
	Add( L"Gnutella1", L"EnableOOB", &Gnutella1.EnableOOB, false );	// ToDo: Set true when OOB fully implemented/verified (out of band query hits)
	Add( L"Gnutella1", L"HostCount", &Gnutella1.HostCount, 15, 1, 1, 50 );
	Add( L"Gnutella1", L"HostExpire", &Gnutella1.HostExpire, 2*24*60*60, 24*60*60, 1, 100, L" d" );
	Add( L"Gnutella1", L"MulticastPingRate", &Gnutella1.MulticastPingRate, 60*1000, 1000, 60, 60*60, L" s" );
	Add( L"Gnutella1", L"MaxHostsInPongs", &Gnutella1.MaxHostsInPongs, 10, 1, 5, 30 );
	Add( L"Gnutella1", L"MaximumQuery", &Gnutella1.MaximumQuery, 256, 1, 32, 262144 );
	Add( L"Gnutella1", L"MaximumTTL", &Gnutella1.MaximumTTL, 10, 1, 1, 10 );
	Add( L"Gnutella1", L"NumHubs", &Gnutella1.NumHubs, 3, 1, 1, 6 );
	Add( L"Gnutella1", L"NumLeafs", &Gnutella1.NumLeafs, 50, 1, 5, 1024 );
	Add( L"Gnutella1", L"NumPeers", &Gnutella1.NumPeers, 32, 1, 15, 64 );		// For X-Degree
	Add( L"Gnutella1", L"PacketBufferSize", &Gnutella1.PacketBufferSize, 64, 1, 1, 1024, L" packets" );
	Add( L"Gnutella1", L"PacketBufferTime", &Gnutella1.PacketBufferTime, 60000, 1000, 10, 180, L" s" );
	Add( L"Gnutella1", L"PingFlood", &Gnutella1.PingFlood, 3000, 1000, 0, 30, L" s" );
	Add( L"Gnutella1", L"PingRate", &Gnutella1.PingRate, 30000, 1000, 15, 180, L" s" );
	Add( L"Gnutella1", L"PongCache", &Gnutella1.PongCache, 10000, 1000, 1, 180, L" s" );
	Add( L"Gnutella1", L"PongCount", &Gnutella1.PongCount, 10, 1, 1, 64 );
	Add( L"Gnutella1", L"QueryHitUTF8", &Gnutella1.QueryHitUTF8, true );
	Add( L"Gnutella1", L"QuerySearchUTF8", &Gnutella1.QuerySearchUTF8, true );
	Add( L"Gnutella1", L"QueryThrottle", &Gnutella1.QueryThrottle, 60, 1, 20, 30*60, L" s" );
	Add( L"Gnutella1", L"QueryGlobalThrottle", &Gnutella1.QueryGlobalThrottle, 60*1000, 1000, 60, 60*60, L" s" );
//	Add( L"Gnutella1", L"QueueLimiter", &Gnutella1.HitQueueLimit, 100 );	// Currently unused
	Add( L"Gnutella1", L"RequeryDelay", &Gnutella1.RequeryDelay, 30, 1, 5, 60, L" s" );
	Add( L"Gnutella1", L"SearchTTL", &Gnutella1.SearchTTL, 3, 1, 1, 3 );
	Add( L"Gnutella1", L"TranslateTTL", &Gnutella1.TranslateTTL, 2, 1, 1, 2 );
	Add( L"Gnutella1", L"VendorMsg", &Gnutella1.VendorMsg, true );

	Add( L"Gnutella2", L"ClientMode", &Gnutella2.ClientMode, MODE_AUTO );
	Add( L"Gnutella2", L"EnableAlways", &Gnutella2.EnableAlways, true );
	Add( L"Gnutella2", L"HAWPeriod", &Gnutella2.HAWPeriod, 300*1000, 1000, 1, 60*60, L" s" );
	Add( L"Gnutella2", L"HostCount", &Gnutella2.HostCount, 15, 1, 1, 50 );
	Add( L"Gnutella2", L"HostCurrent", &Gnutella2.HostCurrent, 10*60, 60, 1, 24*60, L" m" );
	Add( L"Gnutella2", L"HostExpire", &Gnutella2.HostExpire, 2*24*60*60, 24*60*60, 1, 100, L" d" );
	Add( L"Gnutella2", L"HubHorizonSize", &Gnutella2.HubHorizonSize, 128, 1, 32, 512 );
	Add( L"Gnutella2", L"HubVerified", &Gnutella2.HubVerified, false );
	Add( L"Gnutella2", L"KHLHubCount", &Gnutella2.KHLHubCount, 50, 1, 1, 256 );
	Add( L"Gnutella2", L"KHLPeriod", &Gnutella2.KHLPeriod, 60*1000, 1000, 1, 60*60, L" s" );
	Add( L"Gnutella2", L"LNIPeriod", &Gnutella2.LNIPeriod, 60*1000, 1000, 1, 60*60, L" s" );
	if ( Experimental.LAN_Mode )	// #ifdef LAN_MODE
	{
		Add( L"Gnutella2", L"NumHubs",  &Gnutella2.NumHubs, 1, 1, 1, 3 );
		Add( L"Gnutella2", L"NumLeafs", &Gnutella2.NumLeafs, 1024, 1, 50, 1024 );
		Add( L"Gnutella2", L"NumPeers", &Gnutella2.NumPeers, 1, 1, 0, 64 );
	}
	else // Default
	{
		Add( L"Gnutella2", L"NumHubs",  &Gnutella2.NumHubs, 2, 1, 1, 3 );
		Add( L"Gnutella2", L"NumLeafs", &Gnutella2.NumLeafs, 300, 1, 50, 1024 );
		Add( L"Gnutella2", L"NumPeers", &Gnutella2.NumPeers, 6, 1, 2, 64 );
	}
	Add( L"Gnutella2", L"PingRate", &Gnutella2.PingRate, 15000, 1000, 5, 180, L" s" );
	Add( L"Gnutella2", L"PingRelayLimit", &Gnutella2.PingRelayLimit, 10, 1, 10, 30 );
	Add( L"Gnutella2", L"QueryThrottle", &Gnutella2.QueryThrottle, 120, 1, 20, 30*60, L" s" );
	Add( L"Gnutella2", L"QueryGlobalThrottle", &Gnutella2.QueryGlobalThrottle, 125, 1, 1, 60*1000, L" ms" );
	Add( L"Gnutella2", L"QueryHostDeadline", &Gnutella2.QueryHostDeadline, 10*60, 1, 1, 120*60, L" s" );
	Add( L"Gnutella2", L"QueryLimit", &Gnutella2.QueryLimit, 2400, 1, 0, 10000 );
	Add( L"Gnutella2", L"RequeryDelay", &Gnutella2.RequeryDelay, 4*60*60, 60*60, 1, 24, L" h" );
	Add( L"Gnutella2", L"UdpBuffers", &Gnutella2.UdpBuffers, 512, 1, 16, 2048 );
	Add( L"Gnutella2", L"UdpGlobalThrottle", &Gnutella2.UdpGlobalThrottle, 1, 1, 0, 10000 );
	Add( L"Gnutella2", L"UdpInExpire", &Gnutella2.UdpInExpire, 30000, 1000, 1, 300, L" s" );
	Add( L"Gnutella2", L"UdpInFrames", &Gnutella2.UdpInFrames, 256, 1, 16, 2048 );
	Add( L"Gnutella2", L"UdpOutExpire", &Gnutella2.UdpOutExpire, 26000, 1000, 1, 300, L" s" );
	Add( L"Gnutella2", L"UdpOutFrames", &Gnutella2.UdpOutFrames, 256, 1, 16, 2048 );
	Add( L"Gnutella2", L"UdpOutResend", &Gnutella2.UdpOutResend, 6000, 1000, 1, 300, L" s" );
	Add( L"Gnutella2", L"UdpMTU", &Gnutella2.UdpMTU, 500, 1, 16, 10*KiloByte );

	Add( L"eDonkey", L"ShowInterface", &eDonkey.ShowInterface, true );
	Add( L"eDonkey", L"DefaultServerFlags", &eDonkey.DefaultServerFlags, 0xFFFFFFFF );
	Add( L"eDonkey", L"DequeueTime", &eDonkey.DequeueTime, 3600, 60, 2, 512, L" m" );
	Add( L"eDonkey", L"EnableAlways", &eDonkey.EnableAlways, true );
	Add( L"eDonkey", L"Endgame", &eDonkey.Endgame, true );
	Add( L"eDonkey", L"ExtendedRequest", &eDonkey.ExtendedRequest, 2, 1, 0, 2 );
	Add( L"eDonkey", L"FastConnect", &eDonkey.FastConnect, false );
	Add( L"eDonkey", L"ForceHighID", &eDonkey.ForceHighID, true );
	Add( L"eDonkey", L"FrameSize", &eDonkey.FrameSize, 30*KiloByte, KiloByte, 1, KiloByte, L" KB" );	// eMule max 10, 90 may work, ~512 others (lugdunum server max 250000 = 244?)
	Add( L"eDonkey", L"GetSourcesThrottle", &eDonkey.GetSourcesThrottle, 8*60*60*1000, 60*60*1000, 1, 24, L" h" );
	Add( L"eDonkey", L"LargeFileSupport", &eDonkey.LargeFileSupport, true );
	Add( L"eDonkey", L"LearnNewServers", &eDonkey.LearnNewServers, false );
	Add( L"eDonkey", L"LearnNewServersClient", &eDonkey.LearnNewServersClient, false );
	Add( L"eDonkey", L"MagnetSearch", &eDonkey.MagnetSearch, true );
	Add( L"eDonkey", L"MaxLinks", &eDonkey.MaxLinks, 200, 1, 1, 2048 );
	Add( L"eDonkey", L"MaxResults", &eDonkey.MaxResults, 400, 1, 1, 999 );
	Add( L"eDonkey", L"MaxShareCount", &eDonkey.MaxShareCount, 1000, 1, 25, 20000 );
	Add( L"eDonkey", L"MetAutoQuery", &eDonkey.MetAutoQuery, true );
	Add( L"eDonkey", L"MinServerFileSize", &eDonkey.MinServerFileSize, 0, 1, 0, 50, L" MB" );
	Add( L"eDonkey", L"NumServers", &eDonkey.NumServers, 1, 1, 0, 2 );
	Add( L"eDonkey", L"PacketThrottle", &eDonkey.PacketThrottle, 500, 1, 250, 5000, L" ms" );
	Add( L"eDonkey", L"QueryFileThrottle", &eDonkey.QueryFileThrottle, 60*60*1000, 60*1000, 30, 120, L" m" );
	Add( L"eDonkey", L"QueryGlobalThrottle", &eDonkey.QueryGlobalThrottle, 1000, 1, 1000, 20000, L" ms" );
	Add( L"eDonkey", L"QueueRankThrottle", &eDonkey.QueueRankThrottle, 2*60*1000, 1000, 60, 600, L" s" );
	Add( L"eDonkey", L"QueryThrottle", &eDonkey.QueryThrottle, 120, 1, 60, 10*60, L" s" );
	Add( L"eDonkey", L"ReAskTime", &eDonkey.ReAskTime, 29*60, 60, 20, 360, L" m" );
	Add( L"eDonkey", L"RequestPipe", &eDonkey.RequestPipe, 3, 1, 1, 10 );
	Add( L"eDonkey", L"RequestSize", &eDonkey.RequestSize, 90*KiloByte, KiloByte, 10, KiloByte, L" KB" );
	Add( L"eDonkey", L"SendPortServer", &eDonkey.SendPortServer, false );
	Add( L"eDonkey", L"ServerListURL", &eDonkey.ServerListURL, L"http://peerates.net/servers.php" );
	Add( L"eDonkey", L"ServerWalk", &eDonkey.ServerWalk, true );
	Add( L"eDonkey", L"SourceThrottle", &eDonkey.SourceThrottle, 1000, 1, 250, 5000, L" ms" );
	Add( L"eDonkey", L"StatsGlobalThrottle", &eDonkey.StatsGlobalThrottle, 30*60*1000, 60*1000, 30, 120, L" m" );
	Add( L"eDonkey", L"StatsServerThrottle", &eDonkey.StatsServerThrottle, 4*60*60, 60, 1, 7*24*60, L" m" );

	Add( L"DC", L"ShowInterface", &DC.ShowInterface, true );
	Add( L"DC", L"EnableAlways", &DC.EnableAlways, false );
	Add( L"DC", L"NumServers", &DC.NumServers, 1, 1, 0, 5 );
	Add( L"DC", L"QueryThrottle", &DC.QueryThrottle, 2*60, 1, 30, 60*60, L" s" );
	Add( L"DC", L"ReAskTime", &DC.ReAskTime, 60*1000, 1000, 30, 60*60, L" s" );
	Add( L"DC", L"DequeueTime", &DC.DequeueTime, 5*60*1000, 1000, 2*60, 60*60, L" s" );
	Add( L"DC", L"HubListURL", &DC.HubListURL, L"http://dchublist.com/hublist.xml.bz2" );

	Add( L"BitTorrent", L"AutoClear", &BitTorrent.AutoClear, false );
	Add( L"BitTorrent", L"AutoMerge", &BitTorrent.AutoMerge, true );
	Add( L"BitTorrent", L"AutoSeed", &BitTorrent.AutoSeed, true );
	Add( L"BitTorrent", L"BandwidthPercentage", &BitTorrent.BandwidthPercentage, 90, 1, 40, 99, L" %" );
	Add( L"BitTorrent", L"ClearRatio", &BitTorrent.ClearRatio, 120, 1, 100, 999, L" %" );
	Add( L"BitTorrent", L"ConnectThrottle", &BitTorrent.ConnectThrottle, 6*60, 1, 0, 60*60, L" s" );	// DHT
	Add( L"BitTorrent", L"DefaultTracker", &BitTorrent.DefaultTracker, L"udp://tracker.openbittorrent.com:80/announce" );
	Add( L"BitTorrent", L"DefaultTrackerPeriod", &BitTorrent.DefaultTrackerPeriod, 5*60000, 60000, 2, 120, L" m" );
	Add( L"BitTorrent", L"DownloadConnections", &BitTorrent.DownloadConnections, 40, 1, 1, 999 );
	Add( L"BitTorrent", L"DownloadTorrents", &BitTorrent.DownloadTorrents, 3, 1, 1, 12 );
	Add( L"BitTorrent", L"EnableDHT", &BitTorrent.EnableDHT, true );
	Add( L"BitTorrent", L"EnablePromote", &BitTorrent.EnablePromote, true );
	Add( L"BitTorrent", L"EnableAlways", &BitTorrent.EnableAlways, true );
	Add( L"BitTorrent", L"Enabled", &BitTorrent.Enabled, true );
	Add( L"BitTorrent", L"Endgame", &BitTorrent.Endgame, true );
	Add( L"BitTorrent", L"PeerID", &BitTorrent.PeerID, L"" );	// Alternate to PE1000 for trackers
	Add( L"BitTorrent", L"PreferenceBTSources", &BitTorrent.PreferenceBTSources, true );
	Add( L"BitTorrent", L"LinkPing", &BitTorrent.LinkPing, 120*1000, 1000, 10, 60*10, L" s" );
	Add( L"BitTorrent", L"LinkTimeout", &BitTorrent.LinkTimeout, 180*1000, 1000, 10, 60*10, L" s" );
	Add( L"BitTorrent", L"HostExpire", &BitTorrent.HostExpire, 60*24*60*60, 24*60*60, 1, 120, L" d" );	// DHT
	Add( L"BitTorrent", L"QueryHostDeadline", &BitTorrent.QueryHostDeadline, 30, 1, 1, 60*60, L" s" );	// DHT
	Add( L"BitTorrent", L"UtPexPeriod", &BitTorrent.UtPexPeriod, 60*1000, 1000, 10, 60*10, L" s" );
	Add( L"BitTorrent", L"RandomPeriod", &BitTorrent.RandomPeriod, 30*1000, 1000, 1, 60*5, L" s" );
	Add( L"BitTorrent", L"RequestLimit", &BitTorrent.RequestLimit, 128*KiloByte, KiloByte, 1, KiloByte, L" KB" );
	Add( L"BitTorrent", L"RequestPipe", &BitTorrent.RequestPipe, 4, 1, 1, 10 );
	Add( L"BitTorrent", L"RequestSize", &BitTorrent.RequestSize, 16*KiloByte, KiloByte, 8, 128, L" KB" );
	Add( L"BitTorrent", L"SourceExchangePeriod", &BitTorrent.SourceExchangePeriod, 10, 1, 1, 60*5, L" m" );
	Add( L"BitTorrent", L"SkipPaddingFiles", &BitTorrent.SkipPaddingFiles, true );
	Add( L"BitTorrent", L"SkipTrackerFiles", &BitTorrent.SkipTrackerFiles, false );	// Breaks seeding?
	Add( L"BitTorrent", L"TorrentCodePage", &BitTorrent.TorrentCodePage, 0, 1, 0, 9999999 );
	Add( L"BitTorrent", L"TorrentCreatorPath", &BitTorrent.TorrentCreatorPath );
	Add( L"BitTorrent", L"TrackerKey", &BitTorrent.TrackerKey, true );
	Add( L"BitTorrent", L"UploadCount", &BitTorrent.UploadCount, 4, 1, 2, 20 );

	Add( L"Downloads", L"AllowBackwards", &Downloads.AllowBackwards, true );
	Add( L"Downloads", L"AutoClear", &Downloads.AutoClear, false );
	Add( L"Downloads", L"AutoExpand", &Downloads.AutoExpand, false );
	Add( L"Downloads", L"BufferSize", &Downloads.BufferSize, 80*KiloByte, KiloByte, 0, 512, L" KB" );
	Add( L"Downloads", L"ChunkSize", &Downloads.ChunkSize, 512*KiloByte, KiloByte, 0, 10*KiloByte, L" KB" );
	Add( L"Downloads", L"ChunkStrap", &Downloads.ChunkStrap, 128*KiloByte, KiloByte, 0, 10*KiloByte, L" KB" );
	Add( L"Downloads", L"ClearDelay", &Downloads.ClearDelay, 60*1000, 1000, 1, 30*60, L" s" );
	Add( L"Downloads", L"ConnectThrottle", &Downloads.ConnectThrottle, 250, 1, 0, 5000, L" ms" );
	Add( L"Downloads", L"CollectionPath", &Downloads.CollectionPath );
	Add( L"Downloads", L"CompletePath", &Downloads.CompletePath );
	Add( L"Downloads", L"IncompletePath", &Downloads.IncompletePath );
	Add( L"Downloads", L"TorrentPath", &Downloads.TorrentPath );
	Add( L"Downloads", L"FilterMask", &Downloads.FilterMask, 0xFFFFFFFF );
	Add( L"Downloads", L"FlushPD", &Downloads.FlushPD, true );
	Add( L"Downloads", L"MaxAllowedFailures", &Downloads.MaxAllowedFailures, 10, 1, 3, 40 );
	Add( L"Downloads", L"MaxConnectingSources", &Downloads.MaxConnectingSources, 28, 1, 5, 99 );
	Add( L"Downloads", L"MaxFileSearches", &Downloads.MaxFileSearches, 2, 1, 0, 8 );
	Add( L"Downloads", L"MaxFileTransfers", &Downloads.MaxFileTransfers, 40, 1, 1, 300 );
	Add( L"Downloads", L"MaxFiles", &Downloads.MaxFiles, 200, 1, 1, 400 );
	Add( L"Downloads", L"MaxTransfers", &Downloads.MaxTransfers, 100, 1, 1, 400 );
	Add( L"Downloads", L"MaxReviews", &Downloads.MaxReviews, 64, 1, 0, 256 );
	Add( L"Downloads", L"Metadata", &Downloads.Metadata, true );
	Add( L"Downloads", L"MinSources", &Downloads.MinSources, 1, 1, 0, 6 );
	Add( L"Downloads", L"NeverDrop", &Downloads.NeverDrop, false );
	Add( L"Downloads", L"PushTimeout", &Downloads.PushTimeout, 45*1000, 1000, 5, 180, L" s" );
	Add( L"Downloads", L"QueueLimit", &Downloads.QueueLimit, 0, 1, 0, 20000 );
	Add( L"Downloads", L"RequestHTTP11", &Downloads.RequestHTTP11, true );
	Add( L"Downloads", L"RequestHash", &Downloads.RequestHash, true );
	Add( L"Downloads", L"RequestURLENC", &Downloads.RequestURLENC, true );
	Add( L"Downloads", L"RenameExisting", &Downloads.RenameExisting, true );
	Add( L"Downloads", L"RetryDelay", &Downloads.RetryDelay, 10*60*1000, 1000, 120, 60*60, L" s" );
	Add( L"Downloads", L"SaveInterval", &Downloads.SaveInterval, 60*1000, 1000, 1, 120, L" s" );
	Add( L"Downloads", L"SearchPeriod", &Downloads.SearchPeriod, 120*1000, 1000, 10, 4*60, L" s" );
	Add( L"Downloads", L"ShowGroups", &Downloads.ShowGroups, true );
	Add( L"Downloads", L"ShowMonitorURLs", &Downloads.ShowMonitorURLs, true );
	Add( L"Downloads", L"ShowPercent", &Downloads.ShowPercent, false );
	Add( L"Downloads", L"ShowSources", &Downloads.ShowSources, false );
	Add( L"Downloads", L"SimpleBar", &Downloads.SimpleBar, false );
	Add( L"Downloads", L"SortColumns", &Downloads.SortColumns, true );
	Add( L"Downloads", L"SortSources", &Downloads.SortSources, true );
	Add( L"Downloads", L"SourcesWanted", &Downloads.SourcesWanted, 800, 1, 10, 5000 );
	Add( L"Downloads", L"SparseThreshold", &Downloads.SparseThreshold, 8*KiloByte, KiloByte, 0, 256, L" MB" );
	Add( L"Downloads", L"StaggardStart", &Downloads.StaggardStart, false );
	Add( L"Downloads", L"StarveGiveUp", &Downloads.StarveGiveUp, 3, 1, 3, 120, L" h" );
	Add( L"Downloads", L"StarveTimeout", &Downloads.StarveTimeout, 30*60*1000, 60*1000, 20, 24*60, L" m" );
	Add( L"Downloads", L"VerifyFiles", &Downloads.VerifyFiles, true );
	Add( L"Downloads", L"VerifyED2K", &Downloads.VerifyED2K, true );
	Add( L"Downloads", L"VerifyTiger", &Downloads.VerifyTiger, true );
	Add( L"Downloads", L"VerifyTorrent", &Downloads.VerifyTorrent, true );
	Add( L"Downloads", L"NoRandomFragments", &Downloads.NoRandomFragments, false );	// ToDo: Streaming Download and Rarest Piece Selection
	Add( L"Downloads", L"WebHookEnable", &Downloads.WebHookEnable, false );
	Add( L"Downloads", L"WebHookExtensions", &Downloads.WebHookExtensions, L"|zip|zipx|7z|rar|r0|ace|z|gz|tgz|tar|arj|lzh|sit|hqx|fml|grs|cbr|cbz|aac|mp3|mp4|mkv|iso|msi|exe|bin|psk|sks|envy" );

	Add( L"Uploads", L"AllowBackwards", &Uploads.AllowBackwards, true );
	Add( L"Uploads", L"AutoClear", &Uploads.AutoClear, false );
	Add( L"Uploads", L"BlockAgents", &Uploads.BlockAgents, L"|Mozilla|Foxy|" );
	Add( L"Uploads", L"ChunkSize", &Uploads.ChunkSize, 1000*1024, 1024, 1, 2048, L" KB" );
	Add( L"Uploads", L"ClampdownFactor", &Uploads.ClampdownFactor, 20, 1, 0, 100, L"%" );
	Add( L"Uploads", L"ClampdownFloor", &Uploads.ClampdownFloor, 8*128, 128, 0, 4096, L" Kb/s" );
	Add( L"Uploads", L"ClearDelay", &Uploads.ClearDelay, 60*1000, 1000, 1, 1800, L" s" );
	Add( L"Uploads", L"DynamicPreviews", &Uploads.DynamicPreviews, true );
	Add( L"Uploads", L"FairUseMode", &Uploads.FairUseMode, false );	// ToDo: Implement this
	Add( L"Uploads", L"FilterMask", &Uploads.FilterMask, 0xFFFFFFFD );
	Add( L"Uploads", L"FreeBandwidthFactor", &Uploads.FreeBandwidthFactor, 8, 1, 0, 99, L"%" );
	Add( L"Uploads", L"FreeBandwidthValue", &Uploads.FreeBandwidthValue, 20*128, 128, 0, 4096, L" Kb/s" );
	Add( L"Uploads", L"HubUnshare", &Uploads.HubUnshare, true );
	Add( L"Uploads", L"History", &Uploads.History, 30, 1, 1, 500, L" Max" );
	Add( L"Uploads", L"MaxPerHost", &Uploads.MaxPerHost, 2, 1, 1, 64 );
	Add( L"Uploads", L"PreviewQuality", &Uploads.PreviewQuality, 80, 1, 5, 100, L"%" );
	Add( L"Uploads", L"PreviewTransfers", &Uploads.PreviewTransfers, 3, 1, 1, 64 );
	Add( L"Uploads", L"QueuePollMax", &Uploads.QueuePollMax, 120*1000, 1000, 30, 180, L" s" );
	Add( L"Uploads", L"QueuePollMin", &Uploads.QueuePollMin, 45*1000, 1000, 0, 60, L" s" );
	Add( L"Uploads", L"RewardQueuePercentage", &Uploads.RewardQueuePercentage, 10, 1, 0, 99, L"%" );
	Add( L"Uploads", L"RotateChunkLimit", &Uploads.RotateChunkLimit, MegaByte, KiloByte, 0, 10*KiloByte, L" KB" );
	Add( L"Uploads", L"ShareHashset", &Uploads.ShareHashset, true );
	Add( L"Uploads", L"ShareMetadata", &Uploads.ShareMetadata, true );
	Add( L"Uploads", L"SharePartials", &Uploads.SharePartials, true );
	Add( L"Uploads", L"SharePreviews", &Uploads.SharePreviews, true );
	Add( L"Uploads", L"ShareTiger", &Uploads.ShareTiger, true );
	Add( L"Uploads", L"ThrottleMode", &Uploads.ThrottleMode, false );

	Add( L"IRC", L"Colors[0]", &IRC.Colors[0], RGB(254,254,252) );		// ID_COLOR_CHATWINDOW
	Add( L"IRC", L"Colors[1]", &IRC.Colors[1], RGB(0,0,0) );			// ID_COLOR_TEXT
	Add( L"IRC", L"Colors[2]", &IRC.Colors[2], RGB(40,40,40) );			// ID_COLOR_TEXTLOCAL
	Add( L"IRC", L"Colors[3]", &IRC.Colors[3], RGB(10,140,10) );		// ID_COLOR_CHANNELACTION
	Add( L"IRC", L"Colors[4]", &IRC.Colors[4], RGB(180,120,220) );		// ID_COLOR_ME
	Add( L"IRC", L"Colors[5]", &IRC.Colors[5], RGB(0,0,0) );			// ID_COLOR_MSG
	Add( L"IRC", L"Colors[6]", &IRC.Colors[6], RGB(240,20,10) );		// ID_COLOR_NEWMSG
	Add( L"IRC", L"Colors[7]", &IRC.Colors[7], RGB(10,20,240) );		// ID_COLOR_SERVERMSG
	Add( L"IRC", L"Colors[8]", &IRC.Colors[8], RGB(200,100,120) );		// ID_COLOR_TOPIC
	Add( L"IRC", L"Colors[9]", &IRC.Colors[9], RGB(240,20,10) );		// ID_COLOR_NOTICE
	Add( L"IRC", L"Colors[10]", &IRC.Colors[10], RGB(200,30,30) );		// ID_COLOR_SERVERERROR
	Add( L"IRC", L"Colors[11]", &IRC.Colors[11], RGB(230,230,230) );	// ID_COLOR_TABS
	Add( L"IRC", L"Show", &IRC.Show, true );
	Add( L"IRC", L"Nick", &IRC.Nick );
	Add( L"IRC", L"Alternate", &IRC.Alternate );
	Add( L"IRC", L"ServerName", &IRC.ServerName, L"irc.p2pchat.net" );
	Add( L"IRC", L"ServerPort", &IRC.ServerPort, 6667, 1, 1024, 65530 );
	Add( L"IRC", L"FloodEnable", &IRC.FloodEnable, true );
	Add( L"IRC", L"FloodLimit", &IRC.FloodLimit, 24, 1, 2, 100 );
	Add( L"IRC", L"Timestamp", &IRC.Timestamp, false );
	Add( L"IRC", L"UserName", &IRC.UserName, L"EnvyIRC" );
	Add( L"IRC", L"RealName", &IRC.RealName, L"EnvyIRC" );
	Add( L"IRC", L"ScreenFont", &IRC.ScreenFont, theApp.m_bIsWinXP ? NULL : L"Segoe UI", false, setFont );
	Add( L"IRC", L"FontSize", &IRC.FontSize, 12, 1, 6, 50, L" px" );
	Add( L"IRC", L"OnConnect", &IRC.OnConnect, L"" );

	Add( L"Remote", L"Enable", &Remote.Enable, false );
	Add( L"Remote", L"Password", &Remote.Password );
	Add( L"Remote", L"Username", &Remote.Username );

	Add( L"Live", L"BandwidthScaleIn", &Live.BandwidthScaleIn, 101, 1, 0, 101, L" %" );
	Add( L"Live", L"BandwidthScaleOut", &Live.BandwidthScaleOut, 101, 1, 0, 101, L" %" );

	Add( L"Security", L"ListRangeLimit", &Security.ListRangeLimit, 100, 1, 1, 65550 );	// 256*256
	Add( L"Security", L"DefaultBan", &Security.DefaultBan, 100*24*3600, 24*3600, 1, 1000, L" d" );
	Add( L"Scheduler", L"ValidityPeriod", &Scheduler.ValidityPeriod, 60, 1, 1, 1400, L" m" );

	Add( L"Experimental", L"EnableDIPPSupport (GDNA)", &Experimental.EnableDIPPSupport, true );
	Add( L"Experimental", L"LAN_Mode", &Experimental.LAN_Mode, false );		// #ifdef LAN_MODE


	// Load settings
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		pItem->Load();
		CString strPath;
		if ( *pItem->m_szSection )	//_tcslen( pItem->m_szSection ) > 0
			strPath.AppendFormat( L"%s.%s", pItem->m_szSection, pItem->m_szName );
		else
			strPath.AppendFormat( L"General.%s", pItem->m_szName );
		m_pSettingsTable.insert( CSettingsMap::value_type( strPath, pItem ) );
	}

	if ( Library.ScanMSI )
	{
		// Check if Windows installer library is present
		HINSTANCE hMSI = LoadLibrary( L"Msi.dll" );
		if ( ! hMSI )
			Library.ScanMSI = false;
		else
			FreeLibrary( hMSI );
	}

	// Set default program and user paths
	if ( General.Path.IsEmpty() || ! PathFileExists( General.Path ) )
		General.Path = theApp.m_strBinaryPath.Left( theApp.m_strBinaryPath.ReverseFind( L'\\' ) );

	if ( General.MultiUser )
	{
		if ( General.UserPath.IsEmpty() )
			General.UserPath = theApp.GetAppDataFolder() + L"\\Envy";
		if ( General.DataPath.IsEmpty() )
			General.DataPath = General.UserPath + L"\\Data\\";
		if ( Downloads.IncompletePath.IsEmpty() )
			Downloads.IncompletePath = theApp.GetLocalAppDataFolder() + L"\\Envy\\Incomplete";
		if ( Downloads.CompletePath.IsEmpty() )
			Downloads.CompletePath = theApp.GetDownloadsFolder();
	}
	else
	{
		if ( General.UserPath.IsEmpty() )
			General.UserPath = General.Path;
		if ( General.DataPath.IsEmpty() )
			General.DataPath = General.UserPath + L"\\Data\\";
		if ( Downloads.IncompletePath.IsEmpty() )
			Downloads.IncompletePath = General.Path + L"\\Incomplete";
		if ( Downloads.CompletePath.IsEmpty() )
			Downloads.CompletePath = theApp.GetDownloadsFolder();	//General.Path + L"\\Downloads";
	}

	if ( Downloads.CollectionPath.IsEmpty() )
		Downloads.CollectionPath = General.UserPath + L"\\Collections";
	if ( Downloads.TorrentPath.IsEmpty() )
		Downloads.TorrentPath = Downloads.CompletePath + L"\\Torrents";

	if ( ! StartsWith( BitTorrent.DefaultTracker, _P( L"http://" ) ) &&
		 ! StartsWith( BitTorrent.DefaultTracker, _P( L"udp://" ) ) )
		SetDefault( &BitTorrent.DefaultTracker );

	Live.FirstRun = General.FirstRun;
	General.FirstRun = false;

	SmartUpgrade();

	//if ( General.Running )
	// ToDo: Detect Envy restarted after a crash ?

	// Make sure some needed paths exist
	CreateDirectory( General.Path + L"\\Data" );
	CreateDirectory( General.DataPath.Left( General.DataPath.GetLength() - 1 ) );		// General.UserPath + L"\\Data"
	CreateDirectory( Downloads.IncompletePath );
	CreateDirectory( Downloads.CompletePath );
	CreateDirectory( Downloads.TorrentPath );
	CreateDirectory( Downloads.CollectionPath );

	// Set interface
	Interface.LowResMode = GetSystemMetrics( SM_CYSCREEN ) < 640;
	if ( Live.FirstRun )
		Search.AdvancedPanel = ! Interface.LowResMode;

	// Re-set languauge (new user)
	if ( Live.FirstRun && IsDefault( &General.Language ) )
	{
		CString strDefaultLanguage = CRegistry::GetString( NULL, L"DefaultLanguage" );
		if ( ! strDefaultLanguage.IsEmpty() )
		{
			General.Language = strDefaultLanguage;
			General.LanguageRTL = CRegistry::GetDword( NULL, L"DefaultLanguageRTL" ) != 0;
		}
	}

	// Set current networks
	Gnutella2.Enabled	= Gnutella2.EnableAlways;
	Gnutella1.Enabled	= Gnutella1.EnableAlways && Gnutella1.ShowInterface;
	eDonkey.Enabled		= eDonkey.EnableAlways && eDonkey.ShowInterface;
	DC.Enabled			= DC.EnableAlways && DC.ShowInterface;
	BitTorrent.Enabled	= BitTorrent.EnableAlways;

	// Reset certain G1/ed2k network variables if bandwidth is too low
	if ( GetOutgoingBandwidth() < 2 )
	{
		Gnutella1.Enabled = Gnutella1.EnableAlways = false;
		eDonkey.Enabled = eDonkey.EnableAlways = false;
		DC.Enabled = DC.EnableAlways = false;
		BitTorrent.Enabled = false;
	}

	// ToDo: Temporary until G1 ultrapeer has been updated
	//	Gnutella1.ClientMode = MODE_LEAF;

	// Set number of torrents
	BitTorrent.DownloadTorrents = min( BitTorrent.DownloadTorrents, ( GetOutgoingBandwidth() / 2u + 2u ) );

	// Enforce a few sensible values to avoid being banned/dropped/etc (in case of registry fiddling)
	Downloads.ConnectThrottle	= max( Downloads.ConnectThrottle, Connection.ConnectThrottle + 50u );

	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( Downloads.IncompletePath, Downloads.CompletePath ) == 0 )
		MsgBox( IDS_SETTINGS_FILEPATH_NOT_SAME, MB_ICONEXCLAMATION );
		// Downloads.IncompletePath = General.Path + L"\\Incomplete";

	// UPnP is not supported on servers? (Obsolete)
	//if ( IsServer )
	//{
	//	Connection.EnableUPnP = false;
	//	Connection.DeleteUPnPPorts = false;
	//}

	// UPnP will setup a random port, so we need to reset values after it sets Connection.InPort
	if ( Connection.RandomPort )
		Connection.InPort = 0;
	else if ( Connection.InPort == 0 )
		Connection.RandomPort = true;

	if ( Experimental.LAN_Mode )	// #ifdef LAN_MODE
	{
		Connection.IgnoreLocalIP = false;
		Gnutella2.Enabled = Gnutella2.EnableAlways = true;
		Gnutella1.Enabled = Gnutella1.EnableAlways = false;
		eDonkey.Enabled = eDonkey.EnableAlways = false;
		DC.Enabled = DC.EnableAlways = false;
		BitTorrent.Enabled = BitTorrent.EnableAlways = false;
		Gnutella.MaxHits = 0;
	}

	if ( Live.FirstRun )
		OnChangeConnectionSpeed();	// This helps if the QuickStart Wizard is skipped.

//	Save();
}

void CSettings::Save(BOOL bShutdown)
{
	General.Running = ! bShutdown;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		pItem->Save();
	}
}

void CSettings::Normalize(LPVOID pSetting)
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
		{
			pItem->Normalize();
			break;
		}
	}
}

bool CSettings::IsDefault(LPVOID pSetting) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
			return pItem->IsDefault();
	}
	return false;
}

void CSettings::SetDefault(LPVOID pSetting)
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
		{
			pItem->SetDefault();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings smart upgrade

void CSettings::SmartUpgrade()
{
	// This function resets certain values when upgrading, obsolete depending on version.

	// Set next update check
//	if ( General.SmartVersion < SMART_VERSION )
//	{
//		// Don't check for a week if we've just upgraded
//		CTimeSpan tPeriod( 7, 0, 0, 0 );
//		CTime tNextCheck = CTime::GetCurrentTime() + tPeriod;
//		VersionCheck.NextCheck = (DWORD)tNextCheck.GetTime();
//	}

	// Add OGG handling if needed
//	if ( Live.FirstRun && ! IsIn( MediaPlayer.FileTypes, L"ogg" ) )
//	{
//		LONG nReg = 0;
//
//		if ( RegQueryValue( HKEY_CLASSES_ROOT,
//			L"CLSID\\{02391F44-2767-4E6A-A484-9B47B506F3A4}", NULL, &nReg )
//			== ERROR_SUCCESS && nReg > 0 )
//		{
//			MediaPlayer.FileTypes.insert( L"ogg" );
//		}
//	}

	if ( General.SmartVersion < SMART_VERSION )
	{
		// 'SmartUpgrade' setting updates:
		// Change any settings that were mis-set in previous versions
		// Starts at 1000, Prior to Version 60 is obsolete Shareaza code

		// BEGIN ENVY UPDATES @ 60 (1000):
	//	if ( General.SmartVersion < 1000 )
	//		;
	}

	General.SmartVersion = SMART_VERSION;
}

void CSettings::OnChangeConnectionSpeed()
{
	bool bLimited =
#ifndef NOXP
		theApp.m_bLimitedConnections && theApp.m_bIsWinXP && ! General.IgnoreXPLimits;
#else
		false;
#endif

	if ( Connection.OutSpeed < 1000 && Uploads.ChunkSize > 512*1024 )
		Uploads.ChunkSize				= 256*1024;
	else if ( Uploads.ChunkSize <= 256*1024 )
		Uploads.ChunkSize				= 512*1024;

	if ( Connection.InSpeed <= 100 ) 			// Dial-up Modem users
	{
		Downloads.MaxFiles				= 8;
		Downloads.MaxTransfers			= 24;
		Downloads.MaxFileTransfers		= 4;
		Downloads.MaxConnectingSources	= 16;
		Downloads.MaxFileSearches		= 0;
		Downloads.SourcesWanted			= 200;	// Don't bother requesting so many sources
		Search.GeneralThrottle			= 300;	// Slow searches a little so we don't get flooded
		Uploads.ChunkSize				= 128*1024;

		Gnutella2.NumLeafs				= 50;
		BitTorrent.DownloadTorrents		= 2;	// Best not to try too many torrents
	}
	else if ( Connection.InSpeed <= 900 )		// Slow broadband
	{
		Downloads.MaxFiles				= 20;
		Downloads.MaxTransfers			= 64;
		Downloads.MaxFileTransfers		= 8;
		Downloads.MaxConnectingSources	= 24;
		Downloads.MaxFileSearches		= 1;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 250;

		Gnutella2.NumLeafs				= 250;
		BitTorrent.DownloadTorrents		= 3;
	}
	else if ( Connection.InSpeed <= 3000 || bLimited )
	{
		Downloads.MaxFiles				= 30;
		Downloads.MaxTransfers			= 100;
		Downloads.MaxFileTransfers		= 10;
		Downloads.MaxConnectingSources	= 28;
		Downloads.MaxFileSearches		= 2;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 200;

		Gnutella2.NumLeafs				= 300;
		BitTorrent.DownloadTorrents		= 4;
	}
	else if ( Connection.InSpeed <= 10500 )
	{
		Downloads.MaxFiles				= 100;
		Downloads.MaxTransfers			= 250;
		Downloads.MaxFileTransfers		= 50;
		Downloads.MaxConnectingSources	= 50;
		Downloads.MaxFileSearches		= 5;
		Downloads.SourcesWanted			= 600;
		Search.GeneralThrottle			= 150;

		Gnutella1.NumHubs				= 4;
		Gnutella2.NumLeafs				= 400;	// Can probably support more leaves
		BitTorrent.DownloadTorrents 	= 8;	// Should be able to handle several torrents
	}
	else // Extreme Broadband
	{
		Downloads.MaxFiles				= 200;
		Downloads.MaxTransfers			= 300;
		Downloads.MaxFileTransfers		= 100;
		Downloads.MaxConnectingSources	= 100;
		Downloads.MaxFileSearches		= 6;
		Downloads.SourcesWanted			= 999;
		Search.GeneralThrottle			= 100;

		Gnutella1.NumHubs				= 4;
		Gnutella2.NumLeafs				= 500;	// Can probably support more leaves
		BitTorrent.DownloadTorrents 	= 10;	// Should be able to handle several torrents
	}

#ifndef NOXP
	if ( bLimited )	// Windows XP SP2+
	{
		Connection.ConnectThrottle		= max( Connection.ConnectThrottle, 250ul );
		Downloads.ConnectThrottle		= max( Downloads.ConnectThrottle, 800ul );
		Gnutella.ConnectFactor			= min( Gnutella.ConnectFactor, 3ul );
		Connection.SlowConnect			= true;
		Connection.RequireForTransfers	= true;
		Downloads.MaxConnectingSources	= 8;
		Gnutella1.EnableAlways			= false;
		Gnutella1.Enabled				= false;

		General.ItWasLimited			= true;
	}
	else if ( General.ItWasLimited )
	{
		// Revert Settings if Half-Open Limit Patch Applied
		SetDefault( &Connection.ConnectThrottle );
		SetDefault( &Downloads.ConnectThrottle );
		SetDefault( &Gnutella.ConnectFactor );
		SetDefault( &Connection.SlowConnect );

		General.ItWasLimited			= false;
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// CSettings window position persistance

BOOL CSettings::LoadWindow(LPCTSTR pszName, CWnd* pWindow)
{
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	int nShowCmd = CRegistry::GetInt( L"Windows", strEntry + L".ShowCmd", -1 );
	if ( nShowCmd == -1 ) return FALSE;

	WINDOWPLACEMENT pPos = {};
	pPos.length = sizeof( pPos );

	pPos.rcNormalPosition.left		= CRegistry::GetInt( L"Windows", strEntry + L".Left", 0 );
	pPos.rcNormalPosition.top		= CRegistry::GetInt( L"Windows", strEntry + L".Top", 0 );
	pPos.rcNormalPosition.right		= CRegistry::GetInt( L"Windows", strEntry + L".Right", 0 );
	pPos.rcNormalPosition.bottom	= CRegistry::GetInt( L"Windows", strEntry + L".Bottom", 0 );

	if ( pPos.rcNormalPosition.right && pPos.rcNormalPosition.bottom )
	{
		pPos.showCmd = 0;
		pWindow->SetWindowPlacement( &pPos );
	}

	if ( Live.LoadWindowState && nShowCmd == SW_SHOWMINIMIZED )
		pWindow->PostMessage( WM_SYSCOMMAND, SC_MINIMIZE );
	else if ( ! Live.LoadWindowState && nShowCmd == SW_SHOWMAXIMIZED )
		pWindow->PostMessage( WM_SYSCOMMAND, SC_MAXIMIZE );

	return TRUE;
}

void CSettings::SaveWindow(LPCTSTR pszName, CWnd* pWindow)
{
	WINDOWPLACEMENT pPos;
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	pWindow->GetWindowPlacement( &pPos );

	CRegistry::SetInt( L"Windows", strEntry + L".ShowCmd", pPos.showCmd );

	if ( pPos.showCmd != SW_SHOWNORMAL ) return;

	CRegistry::SetInt( L"Windows", strEntry + L".Left", pPos.rcNormalPosition.left );
	CRegistry::SetInt( L"Windows", strEntry + L".Top", pPos.rcNormalPosition.top );
	CRegistry::SetInt( L"Windows", strEntry + L".Right", pPos.rcNormalPosition.right );
	CRegistry::SetInt( L"Windows", strEntry + L".Bottom", pPos.rcNormalPosition.bottom );
}

//////////////////////////////////////////////////////////////////////
// CSettings list header persistance

BOOL CSettings::LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort)
{
	LV_COLUMN pColumn;

	pColumn.mask = LVCF_FMT;
	int nColumns = 0;
	for ( ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	CString strOrdering, strWidths, strItem;
	BOOL bSuccess = FALSE;

	strItem.Format( L"%s.Ordering", pszName );
	strOrdering = CRegistry::GetString( L"ListStates", strItem );
	strItem.Format( L"%s.Widths", pszName );
	strWidths = CRegistry::GetString( L"ListStates", strItem );
	strItem.Format( L"%s.Sort", pszName );
	nSort = CRegistry::GetInt( L"ListStates", strItem, nSort );

	if ( strOrdering.GetLength() == nColumns * 2 &&
		 strWidths.GetLength() == nColumns * 4 )
	{
		UINT* pOrdering = new UINT[ nColumns ];

		for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
		{
			_stscanf( strWidths.Mid( nColumn * 4, 4 ), L"%x", &pOrdering[ nColumn ] );
			pCtrl->SetColumnWidth( nColumn, pOrdering[ nColumn ] );
			_stscanf( strOrdering.Mid( nColumn * 2, 2 ), L"%x", &pOrdering[ nColumn ] );
		}

		pCtrl->SendMessage( LVM_SETCOLUMNORDERARRAY, nColumns, (LPARAM)pOrdering );
		delete [] pOrdering;
		bSuccess = TRUE;
	}

	SetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA, nSort );

	CHeaderCtrl* pHeader = (CHeaderCtrl*)pCtrl->GetWindow( GW_CHILD );
	if ( pHeader ) ::Skin.Translate( pszName, pHeader );

	return bSuccess;
}

void CSettings::SaveList(LPCTSTR pszName, CListCtrl* pCtrl)
{
	LV_COLUMN pColumn;

	pColumn.mask = LVCF_FMT;
	int nColumns = 0;
	for ( ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	UINT* pOrdering = new UINT[ nColumns ];
	ZeroMemory( pOrdering, nColumns * sizeof( UINT ) );
	pCtrl->SendMessage( LVM_GETCOLUMNORDERARRAY, nColumns, (LPARAM)pOrdering );

	CString strOrdering, strWidths, strItem;

	for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
	{
		strItem.Format( L"%.2x", pOrdering[ nColumn ] );
		strOrdering += strItem;
		strItem.Format( L"%.4x", pCtrl->GetColumnWidth( nColumn ) );
		strWidths += strItem;
	}

	delete [] pOrdering;

	int nSort = (int)GetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA );

	strItem.Format( L"%s.Ordering", pszName );
	CRegistry::SetString( L"ListStates", strItem, strOrdering);
	strItem.Format( L"%s.Widths", pszName );
	CRegistry::SetString( L"ListStates", strItem, strWidths);
	strItem.Format( L"%s.Sort", pszName );
	CRegistry::SetInt( L"ListStates", strItem, nSort );
}

//////////////////////////////////////////////////////////////////////
// CSettings startup

BOOL CSettings::CheckStartup()
{
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
		return FALSE;

	BOOL bStartup = ( RegQueryValueEx( hKey, L"Envy", NULL, NULL, NULL, NULL ) == ERROR_SUCCESS );

	RegCloseKey( hKey );

	return bStartup;
}

void CSettings::SetStartup(BOOL bStartup)
{
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		return;

	if ( bStartup )
	{
		CString strCommand;
		strCommand.Format( L"\"%s\" -tray", theApp.m_strBinaryPath );
		RegSetValueEx( hKey, L"Envy", 0, REG_SZ, (const BYTE*)(LPCTSTR)strCommand,
			( strCommand.GetLength() + 1 ) * sizeof( TCHAR ) );
	}
	else
	{
		RegDeleteValue( hKey, L"Envy" );
	}

	RegCloseKey( hKey );
}

void CSettings::ClearSearches()
{
	CString strEntry;
	for ( int i = 1 ; ; i++ )
	{
		strEntry.Format( L"Search.%.2i", i );
		if ( theApp.GetProfileString( L"Search", strEntry ).IsEmpty() )
			break;
		theApp.WriteProfileString( L"Search", strEntry, NULL );
	}
}


//////////////////////////////////////////////////////////////////////
// CSettings speed
//
//	Returns a nicely formatted string displaying a given transfer speed

const CString CSettings::SmartSpeed(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
{
	CString strVolume;
	CString strUnit( L"b/s" );
	int nUnits = bits;

	// Convert to bits or bytes
	nVolume *= nVolumeUnits;
	if ( General.RatesInBytes )
	{
		strUnit = L"B/s";
		nVolume /= Bytes;
		nUnits = Bytes;
	}

	switch ( General.RatesUnit )
	{
	// Smart units
	case 0:
		return SmartVolume( nVolume, nUnits, bTruncate ) + L"/s";

	// bits - Bytes
	case 1:
		strVolume.Format( L"%I64u %s", nVolume, strUnit );
		break;

	// Kilobits - KiloBytes
	case 2:
		strVolume.Format( L"%.1lf K%s", nVolume / KiloFloat, strUnit );
		break;

	// Megabits - MegaBytes
	case 3:
		strVolume.Format( L"%.2lf M%s", nVolume / MegaFloat, strUnit );
		break;

	default:
		TRACE( L"Unknown RatesUnit - %i", General.RatesUnit );
		break;
	}

	// Add Unicode RTL marker if required
	return Settings.General.LanguageRTL ? L"\x200E" + strVolume : strVolume;
}

//////////////////////////////////////////////////////////////////////
// CSettings volume
//
//	Returns a nicely formatted string displaying a given volume

const CString CSettings::SmartVolume(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
{
	LPCTSTR szUnit = ( ! General.RatesInBytes && nVolumeUnits == bits ) ? L"b" : L"B";
	CString strVolume;
	CString strPrecision( bTruncate ? L"%.0f" : L"%.2f" );

	switch ( nVolumeUnits )
	{
	// nVolume is in bits - Bytes
	case bits:
	case Bytes:
		if ( nVolume < KiloByte )					// bits - Bytes
		{
			strVolume.Format( L"%I64u %s", nVolume, szUnit );
			break;
		}
		else if ( nVolume < 10*KiloByte )			// 10 Kilobits - KiloBytes
		{
			strVolume.Format( strPrecision + L" K%s", (double)nVolume / KiloFloat, szUnit );
			break;
		}

		// Convert to KiloBytes and drop through to next case
		nVolume /= KiloByte;

	// nVolume is in Kilobits - Kilobytes
	case Kilobits:
	case KiloBytes:
		if ( nVolume < KiloByte )					// Kilo
		{
			strVolume.Format( L"%I64u K%s", nVolume, szUnit );
		}
		else if ( nVolume < MegaFloat )				// Mega
		{
			strVolume.Format( strPrecision + L" M%s", (double)nVolume / KiloFloat, szUnit );
		}
		else
		{
			if ( nVolume < GigaFloat )				// Giga
				strVolume.Format( strPrecision + L" G%s", (double)nVolume / MegaFloat, szUnit );
			else if ( nVolume < TeraFloat )			// Tera
				strVolume.Format( strPrecision + L" T%s", (double)nVolume / GigaFloat, szUnit );
			else if ( nVolume < PetaFloat )			// Peta
				strVolume.Format( strPrecision + L" P%s", (double)nVolume / TeraFloat, szUnit );
			else									// Exa
				strVolume.Format( strPrecision + L" E%s", (double)nVolume / PetaFloat, szUnit );
		}
	}

	// Add Unicode RTL marker if required
	return Settings.General.LanguageRTL ? L"\x200E" + strVolume : strVolume;
}

QWORD CSettings::ParseVolume(const CString& strVolume, int nReturnUnits) const
{
	double val = 0;
	CString strSize( strVolume );

	// Skip Unicode RTL marker if it's present
	if ( strSize[ 0 ] == L'\x200E' ) strSize = strSize.Mid( 1 );

	// Return early if there is no number in the string
	if ( _stscanf( strSize, L"%lf", &val ) != 1 ) return 0ul;

	// Return early if the number is negative
	if ( val < 0 ) return 0ul;

	// Convert to bits if Bytes were passed in
	if ( _tcsstr( strSize, L"B" ) )
		val *= 8.0f;
	// If bits or Bytes are not indicated return 0
	else if ( !_tcsstr( strSize, L"b" ) )
		return 0ul;

	// Work out what units are represented in the string
	if ( _tcsstr( strSize, L"K" ) || _tcsstr( strSize, L"k" ) )		// Kilo
		val *= KiloFloat;
	else if ( _tcsstr( strSize, L"M" ) || _tcsstr( strSize, L"m" ) )	// Mega
		val *= MegaFloat;
	else if ( _tcsstr( strSize, L"G" ) || _tcsstr( strSize, L"g" ) )	// Giga
		val *= GigaFloat;
	else if ( _tcsstr( strSize, L"T" ) || _tcsstr( strSize, L"t" ) )	// Tera
		val *= TeraFloat;
	else if ( _tcsstr( strSize, L"P" ) || _tcsstr( strSize, L"p" ) )	// Peta
		val *= PetaFloat;
	else if ( _tcsstr( strSize, L"E" ) || _tcsstr( strSize, L"e" ) )	// Exa
		val *= ExaFloat;

	// Convert to required Units
	val /= nReturnUnits;

	// Convert double to DWORD and return
	return static_cast< QWORD >( val );
}

//////////////////////////////////////////////////////////////////////
// CSettings::CheckBandwidth

DWORD CSettings::GetOutgoingBandwidth() const
{	// This returns the available (Affected by limit) outgoing bandwidth in KB/s
	if ( Settings.Bandwidth.Uploads == 0 )
		return ( Settings.Connection.OutSpeed / 8 );

	return ( min( ( Settings.Connection.OutSpeed / 8 ), ( Settings.Bandwidth.Uploads / KiloByte ) ) );
}

bool CSettings::GetValue(LPCTSTR pszPath, VARIANT* value)
{
	if ( value->vt != VT_EMPTY ) return false;

	CSettingsMap::const_iterator i = m_pSettingsTable.find( pszPath );
	if ( i == m_pSettingsTable.end() ) return false;
	Item* pItem = (*i).second;

	if ( pItem->m_pBool )
	{
		value->vt = VT_BOOL;
		value->boolVal = *pItem->m_pBool ? VARIANT_TRUE : VARIANT_FALSE;
	}
	else if ( pItem->m_pDword )
	{
		value->vt = VT_I4;
		value->lVal = (LONG)*pItem->m_pDword;
	}
	else if ( pItem->m_pFloat )
	{
		value->vt = VT_R4;
		value->fltVal = (float)*pItem->m_pFloat;
	}
	else if ( pItem->m_pString )
	{
		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString( CT2CW( *pItem->m_pString ) );
	}
	else if ( pItem->m_pSet )
	{
		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString( CT2CW( SaveSet( pItem->m_pSet ) ) );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CSettings::Item construction and operations

void CSettings::Item::Load()
{
	// Assert above Add() parameters
	if ( m_pBool )
	{
		ASSERT( ! m_pDword && ! m_pFloat && ! m_pString && ! m_pSet );
		ASSERT( m_nScale == 1 && m_nMin == 0 && m_nMax == 1 );
		*m_pBool = CRegistry::GetBool( m_szSection, m_szName, m_BoolDefault );
	}
	else if ( m_pDword )
	{
		ASSERT( ! m_pFloat && ! m_pString && ! m_pSet );
		ASSERT( ( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 ) \
			 || ( m_nScale && m_nMin < m_nMax ) );
		*m_pDword = CRegistry::GetDword( m_szSection, m_szName, m_DwordDefault );
		if ( m_nScale && m_nMin < m_nMax )
		{
			ASSERT( ( m_DwordDefault >= m_nMin * m_nScale ) \
				 && ( m_DwordDefault <= m_nMax * m_nScale ) );
			*m_pDword = max( min( *m_pDword, m_nMax * m_nScale ), m_nMin * m_nScale );
		}
	}
	else if ( m_pFloat )
	{
		ASSERT( ! m_pString && ! m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		*m_pFloat = CRegistry::GetFloat( m_szSection, m_szName, m_FloatDefault );
	}
	else if ( m_pString )
	{
		ASSERT( ! m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		*m_pString = CRegistry::GetString( m_szSection, m_szName, m_StringDefault );
	}
	else
	{
		ASSERT( m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		CString tmp( CRegistry::GetString( m_szSection, m_szName ) );
		LoadSet( m_pSet, tmp.IsEmpty() ? m_StringDefault : (LPCTSTR)tmp );
	}
}

void CSettings::Item::Save() const
{
	if ( m_pBool )
		CRegistry::SetBool( m_szSection, m_szName, *m_pBool );
	else if ( m_pDword )
		CRegistry::SetDword( m_szSection, m_szName, *m_pDword );
	else if ( m_pFloat )
		CRegistry::SetFloat( m_szSection, m_szName, *m_pFloat );
	else if ( m_pString )
		CRegistry::SetString( m_szSection, m_szName, *m_pString );
	else
		CRegistry::SetString( m_szSection, m_szName, SaveSet( m_pSet ) );
}

void CSettings::LoadSet(string_set* pSet, LPCTSTR pszString)
{
	pSet->clear();
	for ( LPCTSTR start = pszString ; start && *start ; start++ )
	{
		LPCTSTR c = _tcschr( start, L'|' );
		int len = c ? (int) ( c - start ) : (int) _tcslen( start );
		if ( len > 0 )
		{
			CString tmp;
			tmp.Append( start, len );
			pSet->insert( tmp );
		}
		if ( ! c )
			break;
		start = c;
	}
}

CString CSettings::SaveSet(const string_set* pSet)
{
	if ( pSet->begin() == pSet->end() )
		return CString();

	CString tmp( L"|" );
	for ( string_set::const_iterator i = pSet->begin() ; i != pSet->end() ; i++ )
	{
		tmp += *i;
		tmp += L'|';
	}
	return tmp;
}

void CSettings::Item::Normalize()
{
	if ( m_pDword && m_nScale && m_nMin < m_nMax )
		*m_pDword = max( min( *m_pDword, m_nMax * m_nScale ), m_nMin * m_nScale );
}

bool CSettings::Item::IsDefault() const
{
	if ( m_pDword )
		return ( m_DwordDefault == *m_pDword );
	if ( m_pBool )
		return ( m_BoolDefault == *m_pBool );
	if ( m_pFloat )
		return ( m_FloatDefault == *m_pFloat );
	if ( m_pString )
		return ( m_StringDefault == *m_pString );

	return ( SaveSet( m_pSet ).CompareNoCase( m_StringDefault ) == 0 );
}

void CSettings::Item::SetDefault()
{
	if ( m_pDword )
		*m_pDword = m_DwordDefault;
	else if ( m_pBool )
		*m_pBool = m_BoolDefault;
	else if ( m_pFloat )
		*m_pFloat = m_FloatDefault;
	else if ( m_pString )
		*m_pString = m_StringDefault;
	else
		LoadSet( m_pSet, m_StringDefault );
}

template<>
void CSettings::Item::SetRange< CSpinButtonCtrl >(CSpinButtonCtrl& pCtrl)
{
	if ( m_pBool )
		pCtrl.SetRange32( 0, 1 );
	else if ( m_pDword )
		pCtrl.SetRange32( m_nMin, m_nMax );
}

template<>
void CSettings::Item::SetRange< CSliderCtrl >(CSliderCtrl& pCtrl)
{
	if ( m_pBool )
		pCtrl.SetRange( 0, 1 );
	else if ( m_pDword )
		pCtrl.SetRange( m_nMin, m_nMax );
}
