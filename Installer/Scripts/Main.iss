; Input defines: ConfigurationName (Debug or Release), PlatformName (Win32 or x64)
; Build x64 first then Win32 in Release for unified installer

; Change from "True" to "False" or "Preview" on the next line for public releases.
#define alpha "True"

; Optional: Change to match signing certificate password, if available
#define signpass "XXXXXX"

; Optional: Visual Studio location and VC version number (for Debug Portability)
#define VisualStudioPath  "c:\Program Files (x86)\Microsoft Visual Studio 14.0"
#define VisualCVersion    "14"

#if VER < 0x05030500
  #error Inno Setup version 5.3.5 or higher (2009) is needed for this script
#endif
#if PREPROCVER < 0x05040200
  #error PreProcessor version 5.4.2.0 or higher (2011) is needed for this script
#endif

#define internal_name GetStringFileInfo("..\..\Envy\" + ConfigurationName + " " + PlatformName + "\Envy.exe", INTERNAL_NAME);
#define name          internal_name
#define version       GetFileVersion("..\..\Envy\" + ConfigurationName + " " + PlatformName + "\Envy.exe")
#define publisher     "GetEnvy.com"
#define description   internal_name + " Filesharing"
#define date          GetDateTimeString('yyyy/mm/dd', '-', '')

#if ConfigurationName == "Release"
  #if PlatformName == "Win32"
    #if version == GetFileVersion("..\..\Envy\Release x64\Envy.exe")
      #define unified_build "True"
    #else
      #define unified_build "False"
    #endif
  #else
    #define unified_build "False"
  #endif
#else
  #define unified_build "False"
#endif

#if PlatformName == "x64"
  #define bits "64"
#else
  #define bits "32"
#endif

#if ConfigurationName == "Debug"
  #define output_name    internal_name + "." + version + "." + date + "." + bits + ".Debug"
  #define display_name   internal_name + " " + version + " " + date + " " + bits + "-bit Debug"
#elif alpha == "True"
  #if unified_build == "True"
    #define output_name	 internal_name + "." + version + "." + date
    #define display_name internal_name + " " + version + " " + date
  #else
    #define output_name	 internal_name + "." + version + "." + date + "." + bits
    #define display_name internal_name + " " + version + " " + date + " " + bits + "-bit"
  #endif
#elif alpha == "Preview"
  #if unified_build == "True"
    #define output_name	 internal_name + "." + version + ".Preview"
    #define display_name internal_name + " " + version + " Preview"
  #else
    #define output_name	 internal_name + "." + version + "." + bits + ".Preview"
    #define display_name internal_name + " " + version + " " + bits + "-bit Preview"
  #endif
#elif unified_build == "True"
  #define output_name	 internal_name + "." + version
  #define display_name	 internal_name + " " + version
#else
  #define output_name	 internal_name + "." + version + "." + bits
  #define display_name	 internal_name + " " + version + " " + bits + "-bit"
#endif


[Setup]
AppComments={#description}
AppId={#internal_name}
AppName={#name}
AppVersion={#version}
AppVerName={#display_name}
AppMutex={#internal_name},Global\TorrentEnvy
DefaultDirName={ini:{param:SETTINGS|},Locations,Path|{reg:HKLM\SOFTWARE\{#internal_name},|{pf}\{#internal_name}}}
DirExistsWarning=no
DefaultGroupName={#internal_name}
#if VER > 0x05030200
  DisableProgramGroupPage=auto
#endif
AllowNoIcons=yes
OutputDir=Builds
OutputBaseFilename={#output_name}
SolidCompression=yes
Compression=lzma2/ultra
;InternalCompressLevel=max
PrivilegesRequired=poweruser
ShowLanguageDialog=yes
ShowUndisplayableLanguages=yes
LanguageDetectionMethod=locale
AppModifyPath="{app}\Uninstall\Setup.exe"
UninstallDisplayIcon={app}\Uninstall\Setup.exe
UninstallDisplayName={#internal_name} {#version}
UninstallFilesDir={app}\Uninstall
SetupIconFile=Installer\Res\Install.ico
WizardSmallImageFile=Installer\Res\CornerLogo.bmp
WizardImageFile=Installer\Res\Sidebar.bmp
WizardImageStretch=no
;WizardImageBackColor=clWhite
DisableWelcomePage=no
ShowComponentSizes=no
ChangesAssociations=yes
ChangesEnvironment=yes
CloseApplications=no
MinVersion=0,5.01
#if unified_build == "True"
  OutputManifestFile=Manifest Release.txt
#else
  OutputManifestFile=Manifest ({#ConfigurationName} {#PlatformName}).txt
#endif
#if PlatformName == "x64"
  ArchitecturesAllowed=x64
  ArchitecturesInstallIn64BitMode=x64
#elif unified_build == "True"
  ArchitecturesInstallIn64BitMode=x64
#endif

; Set SVN root as source dir (up 2 levels)
SourceDir=..\..

VersionInfoVersion={#version}
VersionInfoDescription={#description}
AppPublisher={#publisher}
AppCopyright=© Envy Development Team

; Links to website for software panel
AppPublisherURL=http://getenvy.com
AppSupportURL=http://support.getenvy.com
AppUpdatesURL=http://download.getenvy.com

#if signpass != "XXXXXX"
; Dual-signing /as parameter requires a recent signtool.exe and a SHA256 (SHA-2) certificate
SignTool=signtoolparams sign /f cert.pfx /p "{#signpass}" /fd sha1 /t http://timestamp.verisign.com/scripts/timstamp.dll /d $qEnvy$q $f
SignTool=signtoolparams sign /f cert.pfx /p "{#signpass}" /as /fd sha256 /td sha256 /tr http://timestamp.geotrust.com/tsa /d $qEnvy$q $f
#endif


[Messages]
; Overwrite standard ISL entries  (Do not use localized messages)
BeveledLabel=GetEnvy.com
SetupAppTitle=Setup • {#internal_name}


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}";
Name: "desktopicontorrents"; Description: "{cm:CreateDesktopIconTorrents}"; Languages: en en_uk;
Name: "desktopicongetenvy"; Description: "{cm:CreateDesktopIconGetEnvy}"; Languages: en en_uk; Flags: unchecked;
Name: "quicklaunch"; Description: "{cm:CreateQuickLaunchIcon}"; OnlyBelowVersion: 6.1,6.01;
;Name: "pintaskbar"; Description: "{cm:tasks_pintaskbar}"; MinVersion: 0,6.1;
;Name: "pinstartmenu"; Description: "{cm:tasks_pinstartmenu}"; MinVersion: 0,6.1;
;Name: "language"; Description: "{cm:tasks_languages}";
Name: "multiuser"; Description: "{cm:tasks_multisetup}"; Flags: unchecked;
Name: "webhook"; Description: "{cm:tasks_webhook}"; OnlyBelowVersion: 10.0,10.0; Flags: unchecked;
;Name: "firewall"; Description: "{cm:tasks_firewall}"; MinVersion: 0,5.01sp2;
;Name: "upnp"; Description: "{cm:tasks_upnp}"; MinVersion: 0,5.01; Check: CanUserModifyServices;
Name: "resetdiscoveryhostcache"; Description: "{cm:tasks_resetdiscoveryhostcache}"; Check: WasInstalled; Flags: unchecked;
;#if alpha == "False"
Name: "deleteoldsetup"; Description: "{cm:tasks_deleteoldsetup}"; Check: WasInstalled;
;#endif


[Icons]
; Envy Start Menu Shortcuts
Name: "{group}\{#internal_name}"; Filename: "{app}\Envy.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
Name: "{group}\TorrentEnvy"; Filename: "{app}\TorrentEnvy.exe"; WorkingDir: "{app}"; Comment: "Envy Torrent File Creator"
Name: "{group}\GUI Modes\{#internal_name} ({cm:icons_basicmode})"; Filename: "{app}\Envy.exe"; Parameters: "-basic"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
Name: "{group}\GUI Modes\{#internal_name} ({cm:icons_tabbedmode})"; Filename: "{app}\Envy.exe"; Parameters: "-tabbed"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
Name: "{group}\GUI Modes\{#internal_name} ({cm:icons_windowedmode})"; Filename: "{app}\Envy.exe"; Parameters: "-windowed"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
Name: "{group}\GUI Modes\{#internal_name} ({cm:icons_launchtray})"; Filename: "{app}\Envy.exe"; Parameters: "-tray"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
;Name: "{group}\GUI Modes\{#internal_name} ({cm:icons_noskin})"; Filename: "{app}\Envy.exe"; Parameters: "-noskin"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
Name: "{commondesktop}\{#internal_name}"; Filename: "{app}\Envy.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; Tasks: desktopicon; Check: not FileExists(ExpandConstant('{commondesktop}\{#internal_name}.lnk')); AppUserModelID: "Envy"
Name: "{commondesktop}\TorrentEnvy"; Filename: "{app}\TorrentEnvy.exe"; WorkingDir: "{app}"; Comment: "Envy Drag'n'Drop Torrent Creator"; Tasks: desktopicontorrents; Check: not FileExists(ExpandConstant('{commondesktop}\TorrentEnvy.lnk')); AppUserModelID: "TorrentEnvy"
Name: "{commondesktop}\GetEnvy.com Kickbacks"; Filename: "http://getenvy.com/kickbacks"; IconFilename: "{app}\Schemas\Skin.ico"; Tasks: desktopicongetenvy; Check: not FileExists(ExpandConstant('{commondesktop}\GetEnvy.com Kickbacks.url'))
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#internal_name}"; Filename: "{app}\Envy.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; Tasks: quicklaunch

#if alpha == "True"
Name: "{group}\GUI Modes\{#internal_name} (Help Options)"; Filename: "{app}\Envy.exe"; Parameters: "-?"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Envy"
#endif

; Other icons in user language
Name: "{group}\{cm:icons_license}"; Filename: "{app}\Uninstall\AGPL License.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"
Name: "{group}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Envy}"; IconFilename: "{app}\Uninstall\uninstall.ico"
;Name: "{userprograms}\{groupname}\{cm:icons_downloads}"; Filename: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}}"; WorkingDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}}"; Comment: "{cm:icons_downloads}"; Tasks: multiuser; Check: not WizardNoIcons
;Name: "{group}\{cm:icons_downloads}"; Filename: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}}"; WorkingDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}}"; Comment: "{cm:icons_downloads}"; Tasks: not multiuser; Check: not WizardNoIcons


[Files]
; Main files:

#if unified_build == "True"
Source: "Envy\Release x64\Envy.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Envy\Release Win32\Envy.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
Source: "TorrentEnvy\Release x64\TorrentEnvy.exe";	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "TorrentEnvy\Release Win32\TorrentEnvy.exe";	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
Source: "SkinInstaller\Release x64\SkinInstaller.exe";	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "SkinInstaller\Release Win32\SkinInstaller.exe";DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
;Source: "SkinBuilder\Release x64\SkinBuilder.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
;Source: "SkinBuilder\Release Win32\SkinBuilder.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
#else
Source: "Envy\{#ConfigurationName} {#PlatformName}\Envy.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "TorrentEnvy\{#ConfigurationName} {#PlatformName}\TorrentEnvy.exe";	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "SkinInstaller\{#ConfigurationName} {#PlatformName}\SkinInstaller.exe";	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
;Source: "SkinBuilder\{#ConfigurationName} {#PlatformName}\SkinBuilder.exe"; 	DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#endif

; Save/Restore scripts
Source: "Services\SaveSettings.bat"; DestDir: "{app}"; DestName: "SaveSettings.bat"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist
Source: "Services\RestoreSettings.bat"; DestDir: "{app}"; DestName: "RestoreSettings.bat"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist

; Services:

#if unified_build == "True"

Source: "Services\zlib\Release x64\zlibwapi.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\zlib\Release x64\zlibwapi.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\zlib\Release Win32\zlibwapi.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: not Is64BitInstallMode
Source: "Services\zlib\Release Win32\zlibwapi.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: not Is64BitInstallMode
; Using smaller pre-built DLLs:
;Source: "Services\zlibwapi.64.dll"; DestDir: "{app}";         DestName: "zlibwapi.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
;Source: "Services\zlibwapi.64.dll"; DestDir: "{app}\Plugins"; DestName: "zlibwapi.dll"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: Is64BitInstallMode
;Source: "Services\zlibwapi.dll"; DestDir: "{app}";         Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
;Source: "Services\zlibwapi.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: not Is64BitInstallMode

#else

Source: "Services\zlib\{#ConfigurationName} {#PlatformName}\zlibwapi.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\zlib\{#ConfigurationName} {#PlatformName}\zlibwapi.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall
; Using smaller pre-built DLLs:
;#elif PlatformName == "x64"
;Source: "Services\zlibwapi.64.dll"; DestDir: "{app}";         DestName: "zlibwapi.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
;Source: "Services\zlibwapi.64.dll"; DestDir: "{app}\Plugins"; DestName: "zlibwapi.dll"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall
;#else
;Source: "Services\zlibwapi.dll"; DestDir: "{app}";         Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
;Source: "Services\zlibwapi.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall

#endif

#if unified_build == "True"

Source: "Services\Bzlib\Release x64\Bzlib.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\Bzlib\Release Win32\Bzlib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "HashLib\Release x64\HashLib.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "HashLib\Release Win32\HashLib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Services\SQLite\Release x64\SQLite.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\SQLite\Release Win32\SQLite.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Services\MiniUPnP\Release x64\MiniUPnPc.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\MiniUPnP\Release Win32\MiniUPnPc.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Services\GeoIP\Release x64\GeoIP.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\GeoIP\Release Win32\GeoIP.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
;Source: "Data\GeoIP.dat"; DestDir: "{app}\Data"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

Source: "Services\LibGFL\x64\LibGFL340.dll";   DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Services\LibGFL\Win32\LibGFL340.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
Source: "Services\LibGFL\x64\LibGFL340.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: Is64BitInstallMode
Source: "Services\LibGFL\Win32\LibGFL340.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall ; Check: not Is64BitInstallMode

#else

Source: "Services\Bzlib\{#ConfigurationName} {#PlatformName}\Bzlib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "HashLib\{#ConfigurationName} {#PlatformName}\HashLib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\SQLite\{#ConfigurationName} {#PlatformName}\SQLite.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\GeoIP\{#ConfigurationName} {#PlatformName}\GeoIP.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
;Source: "Data\GeoIP.dat"; DestDir: "{app}\Data"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\MiniUPnP\{#ConfigurationName} {#PlatformName}\MiniUPnPc.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\LibGFL\{#PlatformName}\LibGFL340.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "Services\LibGFL\{#PlatformName}\LibGFL340.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension deleteafterinstall

#endif

; Plugins:

#if unified_build == "True"

Source: "Plugins\DocumentReader\Release x64\DocumentReader.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\DocumentReader\Release Win32\DocumentReader.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\ImageViewer\Release x64\ImageViewer.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\ImageViewer\Release Win32\ImageViewer.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\GFLImageServices\Release x64\GFLImageServices.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\GFLImageServices\Release Win32\GFLImageServices.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode
Source: "Plugins\GFLLibraryBuilder\Release x64\GFLLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\GFLLibraryBuilder\Release Win32\GFLLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\MediaImageServices\Release x64\MediaImageServices.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\MediaImageServices\Release Win32\MediaImageServices.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode
Source: "Plugins\MediaLibraryBuilder\Release x64\MediaLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\MediaLibraryBuilder\Release Win32\MediaLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\RARBuilder\Release x64\RARBuilder.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\RARBuilder\Release Win32\RARBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\RARBuilder\Unrar64.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Plugins\RARBuilder\Unrar.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Plugins\7ZipBuilder\Release x64\7ZipBuilder.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\7ZipBuilder\Release Win32\7ZipBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\7ZipBuilder\7zxa.64.dll"; DestDir: "{app}\Plugins"; DestName: "7zxa.dll"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Plugins\7ZipBuilder\7zxa.dll";    DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Plugins\ZIPBuilder\Release x64\ZIPBuilder.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\ZIPBuilder\Release Win32\ZIPBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\SkinScan\Release x64\SkinScan.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\SkinScan\Release Win32\SkinScan.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\SWFPlugin\Release x64\SWFPlugin.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\SWFPlugin\Release Win32\SWFPlugin.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\SearchExport\Release x64\SearchExport.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\SearchExport\Release Win32\SearchExport.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\ShortURL\Release x64\ShortURL.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\ShortURL\Release Win32\ShortURL.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\VirusTotal\Release x64\VirusTotal.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\VirusTotal\Release Win32\VirusTotal.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

Source: "Plugins\WindowsThumbnail\Release x64\WindowsThumbnail.exe";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: Is64BitInstallMode
Source: "Plugins\WindowsThumbnail\Release Win32\WindowsThumbnail.exe"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode

Source: "Plugins\MediaPlayer\Release x64\MediaPlayer.dll";   DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: Is64BitInstallMode
Source: "Plugins\MediaPlayer\Release Win32\MediaPlayer.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver ; Check: not Is64BitInstallMode

#else

Source: "Plugins\DocumentReader\{#ConfigurationName} {#PlatformName}\DocumentReader.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\ImageViewer\{#ConfigurationName} {#PlatformName}\ImageViewer.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\GFLImageServices\{#ConfigurationName} {#PlatformName}\GFLImageServices.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "Plugins\GFLLibraryBuilder\{#ConfigurationName} {#PlatformName}\GFLLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\MediaImageServices\{#ConfigurationName} {#PlatformName}\MediaImageServices.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "Plugins\MediaLibraryBuilder\{#ConfigurationName} {#PlatformName}\MediaLibraryBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\RARBuilder\{#ConfigurationName} {#PlatformName}\RARBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
#if PlatformName == "x64"
Source: "Plugins\RARBuilder\Unrar64.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#else
Source: "Plugins\RARBuilder\Unrar.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#endif

Source: "Plugins\7ZipBuilder\{#ConfigurationName} {#PlatformName}\7ZipBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
#if PlatformName == "x64"
Source: "Plugins\7ZipBuilder\7zxa.64.dll"; DestDir: "{app}\Plugins"; DestName: "7zxa.dll"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension
#else
Source: "Plugins\7ZipBuilder\7zxa.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion uninsremovereadonly sortfilesbyextension
#endif

Source: "Plugins\ZIPBuilder\{#ConfigurationName} {#PlatformName}\ZIPBuilder.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\SkinScan\{#ConfigurationName} {#PlatformName}\SkinScan.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\SWFPlugin\{#ConfigurationName} {#PlatformName}\SWFPlugin.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

;Source: "Plugins\RatDVDPlugin\{#ConfigurationName} {#PlatformName}\RatDVDReader.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\SearchExport\{#ConfigurationName} {#PlatformName}\SearchExport.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\ShortURL\{#ConfigurationName} {#PlatformName}\ShortURL.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\VirusTotal\{#ConfigurationName} {#PlatformName}\VirusTotal.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

Source: "Plugins\WindowsThumbnail\{#ConfigurationName} {#PlatformName}\WindowsThumbnail.exe"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

Source: "Plugins\MediaPlayer\{#ConfigurationName} {#PlatformName}\MediaPlayer.dll"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

#endif

; Don't register WebHook.dll since it will setup Envy as download manager
Source: "Plugins\WebHook\{#ConfigurationName} Win32\WebHook32.dll"; DestDir: "{app}\Plugins"; Flags: noregerror overwritereadonly replacesameversion restartreplace uninsrestartdelete uninsremovereadonly sortfilesbyextension; Tasks: not webhook
Source: "Plugins\WebHook\{#ConfigurationName} x64\WebHook64.dll"; DestDir: "{app}\Plugins"; Flags: noregerror overwritereadonly replacesameversion restartreplace uninsrestartdelete uninsremovereadonly sortfilesbyextension; Tasks: not webhook
Source: "Plugins\WebHook\{#ConfigurationName} Win32\WebHook32.dll"; DestDir: "{app}\Plugins"; Flags: noregerror overwritereadonly replacesameversion restartreplace uninsrestartdelete uninsremovereadonly sortfilesbyextension regserver; Tasks: webhook
Source: "Plugins\WebHook\{#ConfigurationName} x64\WebHook64.dll"; DestDir: "{app}\Plugins"; Flags: noregerror overwritereadonly replacesameversion restartreplace uninsrestartdelete uninsremovereadonly sortfilesbyextension regserver; Tasks: webhook

; Debug Databases:

#if ConfigurationName == "Debug"

Source: "Envy\{#ConfigurationName} {#PlatformName}\Envy.pdb"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
; ** These sections can be uncommented to include the debug database files for all plugins/services
;Source: "Plugins\*.pdb"; DestDir: "{app}\Plugins"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
;Source: "Services\*.pdb"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

Source: "Services\BugTrap\Release {#PlatformName}\BugTrap.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

#if PlatformName == "x64"
Source: "Services\BugTrap\dbghelp.64.dll"; DestDir: "{sys}"; DestName: "dbghelp.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsneveruninstall sortfilesbyextension
#else
Source: "Services\BugTrap\dbghelp.dll"; DestDir: "{sys}"; DestName: "dbghelp.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsneveruninstall sortfilesbyextension
#endif

#if PlatformName == "x64"
Source: "{#VisualStudioPath}\VC\redist\debug_nonredist\x64\Microsoft.VC{#VisualCVersion}0.DebugCRT\vcruntime{#VisualCVersion}0d.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist replacesameversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "c:\Program Files (x86)\Windows Kits\10\bin\x64\ucrt\ucrtbased.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist replacesameversion overwritereadonly uninsrestartdelete uninsremovereadonly sortfilesbyextension
#else
Source: "{#VisualStudioPath}\VC\redist\debug_nonredist\x86\Microsoft.VC{#VisualCVersion}0.DebugCRT\vcruntime{#VisualCVersion}0d.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist replacesameversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "c:\Program Files (x86)\Windows Kits\10\bin\x86\ucrt\ucrtbased.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist replacesameversion overwritereadonly uninsrestartdelete uninsremovereadonly sortfilesbyextension
#endif

#endif
; Debug


; Include Files:

; Main Data Files
Source: "Data\*"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn,*.bak,*.bak.*,*GPL*,WorldGPS.xml"

; Schemas
Source: "Schemas\*"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn,*.bak,*.Safe.ico,ReadMe.txt,SchemaDescriptor.xsd"
#if PlatformName == "Win32"
Source: "Schemas\*.Safe.ico"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
#elif unified_build == "True"
Source: "Schemas\*.Safe.ico"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension ; Check: not Is64BitInstallMode
#endif

; Skins
Source: "Skins\*"; DestDir: "{app}\Skins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn,*.bak"

; Languages
Source: "Languages\*.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Languages\*.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: "en.xml"
Source: "Languages\en.xml"; DestDir: "{app}\Skins\Languages\en.xml.sample"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
; See Languages.iss for by-selection-only

; Templates
Source: "Templates\*"; DestDir: "{app}\Templates"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn,*.bak"

; Remote files
Source: "Remote\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn,*.xlsx,Readme.txt"
;Source: "Remote\Resources\*"; DestDir: "{app}\Remote\Resources"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

; Icons
Source: "Installer\Res\Uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Licenses
;Source: "Installer\License\LICENSE-GeoIP.txt"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Installer\License\License (AGPLv3).html"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Copy Files:

; Copy skins back from {userappdata}\Envy\Skins
Source: "{userappdata}\Envy\Skins\*"; DestDir: "{app}\Skins"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist recursesubdirs; AfterInstall: DeleteFolder('{userappdata}\Envy\Skins')

; Copy templates back from {userappdata}\Envy\Templates
Source: "{userappdata}\Envy\Templates\*"; DestDir: "{app}\Templates"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist recursesubdirs; AfterInstall: DeleteFolder('{userappdata}\Envy\Templates')

; Switch user data between locations
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Library.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Library.bak"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\DownloadGroups.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\TigerTree.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Security.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\UploadQueues.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Searches.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Schedule.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Profile.xml"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Library.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Library.bak"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\DownloadGroups.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\TigerTree.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Security.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\UploadQueues.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Searches.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Schedule.dat"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Profile.xml"; DestDir: "{userappdata}\Envy\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser

Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Library.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Library.bak"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\DownloadGroups.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\TigerTree.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Security.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\UploadQueues.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Searches.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Schedule.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data\Profile.xml"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Library.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Library.bak"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\DownloadGroups.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\TigerTree.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Security.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\UploadQueues.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Searches.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Schedule.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data\Profile.xml"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser

; Copy installer into download and uninstall dir
#if alpha == "False"
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}}"; DestName: "{#output_name}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external onlyifdoesntexist; Tasks: multiuser
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}}"; DestName: "{#output_name}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external onlyifdoesntexist; Tasks: not multiuser
#endif
Source: "{srcexe}"; DestDir: "{app}\Uninstall"; DestName: "setup.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external

; Copy default security rules
Source: "Data\DefaultSecurity.dat"; DestDir: "{userappdata}\Envy\Data"; DestName: "Security.dat"; Flags: onlyifdoesntexist uninsremovereadonly sortfilesbyextension; Tasks: multiuser
Source: "Data\DefaultSecurity.dat"; DestDir: "{app}\Data"; DestName: "Security.dat"; Flags: onlyifdoesntexist uninsremovereadonly sortfilesbyextension; Tasks: not multiuser


[Run]
; Register EXE servers
Filename: "{app}\Envy.exe"; Parameters: "/RegServer"; WorkingDir: "{app}"
;Filename: "{app}\Plugins\WindowsThumbnail.exe"; Parameters: "/RegServer"; WorkingDir: "{app}"
;Filename: "{app}\Plugins\MediaImageServices.exe"; Parameters: "/RegServer"; WorkingDir: "{app}"
;Filename: "{app}\Plugins\MediaPlayer.exe"; Parameters: "/RegServer"; WorkingDir: "{app}"

; Run the skin installer at end of installation
Filename: "{app}\SkinInstaller.exe"; Parameters: "/installsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"
; Run Envy at end of installation
Filename: "{app}\Envy.exe"; Description: "{cm:LaunchProgram,Envy}"; WorkingDir: "{app}"; Flags: postinstall skipifsilent nowait


[UninstallRun]
; Run the skin installer at start of uninstallation and make sure it only runs once
Filename: "{app}\SkinInstaller.exe"; Parameters: "/uninstallsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"; RunOnceId: "uninstallskinexe"
;Filename: "{app}\Plugins\WindowsThumbnail.exe"; Parameters: "/UnRegServer"; WorkingDir: "{app}"
Filename: "{app}\Envy.exe"; Parameters: "/UnRegServer"; WorkingDir: "{app}"


[Registry]
Root: HKLM; Subkey: "Software\Envy\Envy"; ValueType: dword; ValueName: "MultiUser"; ValueData: 1; Flags: deletevalue uninsdeletekey; Tasks: multiuser
Root: HKLM; Subkey: "Software\Envy\Envy"; ValueType: dword; ValueName: "MultiUser"; ValueData: 0; Flags: deletevalue uninsdeletekey; Tasks: not multiuser

; Write installation path to registry
Root: HKLM; Subkey: "Software\Envy"; ValueType: string; ValueName: ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Envy\Envy"; ValueType: string; ValueName: "Path" ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Envy\Envy"; ValueType: string; ValueName: "UserPath" ; ValueData: "{ini:{param:SETTINGS|},Locations,UserPath|{userappdata}\Envy}"; Flags: uninsdeletekey deletevalue ; Tasks: multiuser
Root: HKCU; Subkey: "Software\Envy\Envy"; ValueType: string; ValueName: "UserPath" ; ValueData: "{ini:{param:SETTINGS|},Locations,UserPath|{app}}"; Flags: uninsdeletekey deletevalue; Tasks: not multiuser
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\Envy.exe"; ValueType: string; ValueName: ; ValueData: "{app}\Envy.exe"; Flags: uninsdeletekey deletevalue
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\Envy.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletekey deletevalue

; Set directory locations
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{ini:{param:SETTINGS|},Locations,CompletePath|{userdocs}\Envy Downloads}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{ini:{param:SETTINGS|},Locations,IncompletePath|{localappdata}\Envy\Incomplete}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{ini:{param:SETTINGS|},Locations,TorrentPath|{userappdata}\Envy\Torrents}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{ini:{param:SETTINGS|},Locations,CollectionPath|{userappdata}\Envy\Collections}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser

;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{ini:{param:SETTINGS|},Locations,CompletePath|{app}\Downloads}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{ini:{param:SETTINGS|},Locations,IncompletePath|{app}\Incomplete}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{ini:{param:SETTINGS|},Locations,TorrentPath|{app}\Torrents}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
;Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{ini:{param:SETTINGS|},Locations,CollectionPath|{app}\Collections}"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser

; Install chat notify sound
Root: HKCU; Subkey: "AppEvents\EventLabels\Sound_IncomingChat"; ValueType: string; ValueName: ; ValueData: "{cm:reg_incomingchat}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Envy"; ValueType: string; ValueName: ; ValueData: "{cm:reg_apptitle}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Envy\Sound_IncomingChat\.current"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\Media\notify.wav"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Envy\Sound_IncomingChat\.default"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\Media\notify.wav"; Flags: uninsdeletekey

; Set UPnP by default (disabled choice during setup)
;Root: HKCU; Subkey: "Software\Envy\Envy\Connection"; ValueType: dword; ValueName: "EnableUPnP"; ValueData: 0; Flags: deletevalue; Tasks: not upnp
;Root: HKCU; Subkey: "Software\Envy\Envy\Connection"; ValueType: dword; ValueName: "EnableUPnP"; ValueData: 1; Flags: deletevalue; Tasks: upnp
Root: HKCU; Subkey: "Software\Envy\Envy\Connection"; ValueType: dword; ValueName: "EnableUPnP"; ValueData: 1; Flags: uninsdeletekey createvalueifdoesntexist

; Enable/Disable WebHook
Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: dword; ValueName: "WebHookEnable"; ValueData: 1; Flags: uninsdeletekey deletevalue; Tasks: webhook
Root: HKCU; Subkey: "Software\Envy\Envy\Downloads"; ValueType: dword; ValueName: "WebHookEnable"; ValueData: 0; Flags: uninsdeletekey deletevalue; Tasks: not webhook

; ShareMonkey CID
;Root: HKCU; Subkey: "Software\Envy\Envy\WebServices"; ValueType: string; ValueName: "ShareMonkeyCid"; ValueData: "197506"; Flags: deletevalue uninsdeletekey

; Delete keys at uninstall
Root: HKU; Subkey: ".DEFAULT\Software\Envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Envy\PeerProjce\Plugins"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Envy"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "Software\Envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "Envy"; Flags: dontcreatekey uninsdeletevalue

Root: HKCR; Subkey: "Software\Classes\.envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Software\Classes\.env"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Software\Classes\.psk"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Software\Classes\.sks"; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCR; Subkey: "Software\Classes\.pd"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Software\Classes\.sd"; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCR; Subkey: "Software\Classes\.co"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Software\Classes\.collection"; Flags: dontcreatekey uninsdeletekey
;Root: HKCR; Subkey: "Software\Classes\.torrent"; ValueType: string; ValueName: ""; ValueData: "BitTorrent"; Flags: dontcreatekey uninsdeletevalue; Check: WeOwnTorrentAssoc
Root: HKCR; Subkey: "BitTorrent"; ValueType: string; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCR; Subkey: "Envy.Application"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.Collection"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.PartialData"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.DataSource"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.DocReader"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.DocReader.1"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.IEProtocol"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.IEProtocolRequest"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.SkinInfoExtractor"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.SkinInfoExtractor.1"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.XMLCollection"; Flags: dontcreatekey uninsdeletekey
Root: HKCR; Subkey: "Envy.XML"; Flags: dontcreatekey uninsdeletekey

Root: HKCU; Subkey: "Software\Classes\.envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.env"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.psk"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.sks"; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCU; Subkey: "Software\Classes\.pd"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.sd"; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCU; Subkey: "Software\Classes\.co"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.collection"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\.emulecollection"; Flags: dontcreatekey uninsdeletekey
;Root: HKCU; Subkey: "Software\Classes\.torrent"; ValueName: "BitTorrent"; Flags: dontcreatekey uninsdeletevalue
Root: HKCU; Subkey: "Software\Classes\BitTorrent"; Flags: dontcreatekey uninsdeletekey; Check: WeOwnTorrentAssoc
Root: HKCU; Subkey: "Software\Classes\magnet"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\ed2k"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\gnet"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\gnutella"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\gnutella1"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\gnutella2"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\gwc"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\g2"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\uhc"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\ukhl"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\mp2p"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\peer"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\Envy.*"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\Envy.Collection"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\Applications\Envy.exe"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\Applications\SkinInstaller.exe"; Flags: dontcreatekey uninsdeletekey

Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.envy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psk"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.sks"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.pd"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.torrent"; Flags: dontcreatekey uninsdeletevalue
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Management\ARPCache\EnvySetup"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\ShellNoRoam\MUICache"; ValueName:"{app}\Envy.exe"; Flags: dontcreatekey uninsdeletevalue

Root: HKLM; Subkey: "Software\Magnet"; Flags: dontcreatekey uninsdeletekey

; Delete obsolete entry name on Windows software panel, if present (Legacy)
;Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\Envy_is1"; Flags: dontcreatekey deletekey

; Clear version check key
Root: HKCU; Subkey: "Software\Envy\Envy\VersionCheck"; Flags: dontcreatekey deletekey
Root: HKLM; Subkey: "Software\Envy\Envy\VersionCheck"; Flags: dontcreatekey deletekey

; Create TorrentEnvy default dir locations
Root: HKCU; Subkey: "Software\Envy\TorrentEnvy\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{userappdata}\Envy\Torrents"; Flags: createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Envy\TorrentEnvy\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{userappdata}\Envy\Torrents"; Flags: createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Envy\TorrentEnvy\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\Envy\TorrentEnvy\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\Envy\Envy\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "TorrentEnvy.exe" ; Flags: createvalueifdoesntexist uninsdeletekey

; Disable extensions for plugins which make trouble
; Since it is image services plugin we need to add extensions required for the first run
Root: HKCU; Subkey: "Software\Envy\Envy\Plugins"; ValueType: string; ValueName: "{{C9314782-CB91-40B8-B375-F631FF30C1C8}"; ValueData: "|-.pdf||.bmp||.png||.jpg|"; Flags: createvalueifdoesntexist uninsdeletekey
Root: HKCU; Subkey: "Software\Envy\Envy\Plugins"; Flags: dontcreatekey uninsdeletekey


[Dirs]
; Make complete, incomplete, torrent and collection dir
; Note: download dir will be created when installer is copied but we create also here to be sure
Name: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Envy\Envy,UserPath|{userappdata}\Envy}}\Data"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,IncompletePath|{localappdata}\Envy\Incomplete}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{userappdata}\Envy\Torrents}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{userappdata}\Envy\Collections}}"; Flags: uninsalwaysuninstall; Tasks: multiuser

Name: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Envy\Envy,Path|{app}}}\Data"; Flags: uninsalwaysuninstall; Permissions: users-modify; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}}"; Flags: uninsalwaysuninstall; Permissions: users-modify; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,IncompletePath|{app}\Incomplete}}"; Flags: uninsalwaysuninstall; Permissions: users-modify; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{app}\Torrents}}"; Flags: uninsalwaysuninstall; Permissions: users-modify; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{app}\Collections}}"; Flags: uninsalwaysuninstall; Permissions: users-modify; Tasks: not multiuser

Name: "{app}\Skins"; Flags: uninsalwaysuninstall; Permissions: users-modify


[InstallDelete]
; Very basic malware removal
Type: files; Name: "{app}\Envy.exe"; Check: IsMalwareDetected
Type: files; Name: "{app}\vc2.dll"

; Clean up old files from Envy
Type: files; Name: "{app}\*.pdb"
Type: files; Name: "{app}\LibGFL*.dll"
Type: files; Name: "{app}\Plugins\WebHook.dll"
Type: files; Name: "{app}\Plugins\7zx*.dll"
Type: files; Name: "{app}\Plugins\*.pdb"
Type: files; Name: "{app}\*.dat"
Type: files; Name: "{app}\*.xml"
;Type: files; Name: "{app}\*.png"
;Type: files; Name: "{app}\*.bmp"
;Type: files; Name: "{app}\Data\*.url"
Type: filesandordirs; Name: "{userappdata}\Envy\Remote"
Type: filesandordirs; Name: "{userappdata}\Envy\Schemas"
Type: filesandordirs; Name: "{userappdata}\Envy\Skins"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}\Thumbs.db"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}\Thumbs.db"
Type: files; Name: "{userappdata}\Envy\Data\DefaultAvatar.png"

; Clean up old/unwanted Envy Shortcuts
Type: files; Name: "{userdesktop}\Envy.lnk"; Tasks: not desktopicon
Type: files; Name: "{commondesktop}\Envy.lnk"; Tasks: not desktopicon
Type: files; Name: "{userdesktop}\TorrentEnvy.lnk"; Tasks: not desktopicontorrents
Type: files; Name: "{commondesktop}\TorrentEnvy.lnk"; Tasks: not desktopicontorrents
;Type: files; Name: "{userdesktop}\Start Envy.lnk"; Check: NSISUsed
;Type: filesandordirs; Name: "{userprograms}\Envy"; Check: InnoSetupUsed
;Type: filesandordirs; Name: "{commonprograms}\Envy"; Check: InnoSetupUsed
Type: files; Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Envy.lnk"; Tasks: not quicklaunch
; Following two lines may delete data not created by Envy (Instead)
;Type: filesandordirs; Name: "{userprograms}\{reg:HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EnvySetup,Inno Setup: Icon Group|{groupname}}"; Check: InnoSetupUsed
;Type: filesandordirs; Name: "{commonprograms}\{reg:HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EnvySetup,Inno Setup: Icon Group|{groupname}}"; Check: InnoSetupUsed

; Delete extra components so installer can "uninstall" them
Type: filesandordirs; Name: "{app}\Remote"
;Type: filesandordirs; Name: "{app}\Skins\Languages"; Tasks: not language

; Delete old Envy installers
#if alpha == "False"
Type: files; Name: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}}\Envy*.exe"; Tasks: deleteoldsetup and multiuser
Type: files; Name: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}}\Envy*.exe"; Tasks: deleteoldsetup and not multiuser
#endif

; Delete Discovery.dat and HostCache.dat
Type: files; Name: "{app}\Data\Discovery.dat"; Tasks: resetdiscoveryhostcache
Type: files; Name: "{app}\Data\HostCache.dat"; Tasks: resetdiscoveryhostcache
Type: files; Name: "{userappdata}\Envy\Data\Discovery.dat"; Tasks: resetdiscoveryhostcache and multiuser
Type: files; Name: "{userappdata}\Envy\Data\HostCache.dat"; Tasks: resetdiscoveryhostcache and multiuser


[UninstallDelete]
; Clean up files created after installation
Type: filesandordirs; Name: "{userappdata}\Envy\Data"
Type: filesandordirs; Name: "{app}\Data"
Type: filesandordirs; Name: "{app}\Skins"
Type: filesandordirs; Name: "{app}\Templates"
Type: filesandordirs; Name: "{app}\Schemas"

Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}\Metadata"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}\Thumbs.db"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}\SThumbs.dat"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{userdocs}\Envy Downloads}\desktop.ini"; Tasks: multiuser
Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{userappdata}\Envy\Torrents}\Metadata"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{userappdata}\Envy\Torrents}\desktop.ini"; Tasks: multiuser
Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{userappdata}\Envy\Collections}\Metadata"; Tasks: multiuser
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{userappdata}\Envy\Collections}\desktop.ini"; Tasks: multiuser

Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}\Thumbs.db"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}\SThumbs.dat"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CompletePath|{app}\Downloads}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{app}\Torrents}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,TorrentPath|{app}\Torrents}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{app}\Collections}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Envy\Envy\Downloads,CollectionPath|{app}\Collections}\desktop.ini"

; Pull in more Envy settings to write to registry
#include "Settings.iss"

; Pull in languages and localized files
#include "Languages.iss"


; Code section needs to be the last section in a script or compiler will get confused:

[Code]
type
  SERVICE_STATUS = record
    dwServiceType: cardinal;
    dwCurrentState: cardinal;
    dwControlsAccepted: cardinal;
    dwWin32ExitCode: cardinal;
    dwServiceSpecificExitCode: cardinal;
    dwCheckPoint: cardinal;
    dwWaitHint: cardinal;
  end;
  HANDLE = cardinal;
const
  WM_CLOSE = $0010;
  KeyLoc1 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EnvySetup';
  KeyLoc2 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Envy';
  KeyName = 'UninstallString';
  NET_FW_SCOPE_ALL = 0;
  NET_FW_IP_VERSION_ANY  = 2;
//SERVICE_QUERY_CONFIG   = $1;
//SERVICE_CHANGE_CONFIG  = $2;
//SERVICE_QUERY_STATUS   = $4;
//SERVICE_START          = $10;
//SERVICE_STOP           = $20;
//SERVICE_ALL_ACCESS     = $f01ff;
//SC_MANAGER_ALL_ACCESS  = $f003f;
//SERVICE_AUTO_START     = $2;
//SERVICE_DEMAND_START   = $3;
//SERVICE_RUNNING        = $4;
//SERVICE_NO_CHANGE      = $ffffffff;
// Constants not defined for Windows 7+:
  SHELL32_STRING_ID_PIN_TASKBAR = 5386;
  SHELL32_STRING_ID_PIN_STARTMENU = 5381;
  SHELL32_STRING_ID_UNPIN_TASKBAR = 5387;
  SHELL32_STRING_ID_UNPIN_STARTMENU = 5382;
var
  CurrentPath: string;
  Installed: Boolean;
  MalwareDetected: Boolean;
  FirewallFailed: string;
//HasUserPrivileges: Boolean;

// NT API functions for services (Unused UPnP)
//Function OpenSCManager(lpMachineName, lpDatabaseName: string; dwDesiredAccess: cardinal): HANDLE;
//external 'OpenSCManagerA@advapi32.dll stdcall setuponly';
//
//Function OpenService(hSCManager: HANDLE; lpServiceName: string; dwDesiredAccess: cardinal): HANDLE;
//external 'OpenServiceA@advapi32.dll stdcall setuponly';
//
//Function CloseServiceHandle(hSCObject: HANDLE): Boolean;
//external 'CloseServiceHandle@advapi32.dll stdcall setuponly';
//
//Function StartNTService(hService: HANDLE; dwNumServiceArgs: cardinal; lpServiceArgVectors: cardinal): Boolean;
//external 'StartServiceA@advapi32.dll stdcall setuponly';
//
//Function QueryServiceStatus(hService: HANDLE; var ServiceStatus: SERVICE_STATUS): Boolean;
//external 'QueryServiceStatus@advapi32.dll stdcall setuponly';
//
//Function ChangeServiceConfig(hService: HANDLE; dwServiceType, dwStartType, dwErrorControl: cardinal;
//                             lpBinaryPathName, lpLoadOrderGroup: string; lpdwTagId: cardinal;
//                             lpDependencies, lpServiceStartName, lpPassword, lpDisplayName: string): Boolean;
//external 'ChangeServiceConfigA@advapi32.dll stdcall setuponly';

//Function InnoSetupUsed(): boolean;
//Begin
//  Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc1);
//End;

//Function NSISUsed(): boolean;
//Begin
//  Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc2);
//End;

//Function OpenServiceManager(): HANDLE;
//begin
//  Result := 0;
//  if (InstallOnThisVersion('0,5.01', '0,0') = irInstall) then
//    Result := OpenSCManager('', 'ServicesActive', SC_MANAGER_ALL_ACCESS);
//end;

//Function CanUserModifyServices(): Boolean;
//var
// hSCManager: HANDLE;
//begin
//  hSCManager := 0;
//  Result := false;
//  HasUserPrivileges := false;
//  if (InstallOnThisVersion('0,5.01', '0,0') = irInstall) then begin
//    hSCManager := OpenSCManager('', 'ServicesActive', SC_MANAGER_ALL_ACCESS);
//    if (hSCManager <> 0) then begin
//      HasUserPrivileges := true;
//      Result := true;
//      CloseServiceHandle(hSCManager);
//    end;
//  end;
//end;

//Function IsServiceInstalled(ServiceName: string): boolean;
//var
// hSCManager: HANDLE;
// hService: HANDLE;
//begin
//  hSCManager := OpenServiceManager();
//  Result := false;
//  if (hSCManager <> 0) then begin
//    hService := OpenService(hSCManager, ServiceName, SERVICE_QUERY_CONFIG);
//    if (hService <> 0) then begin
//      Result := true;
//      CloseServiceHandle(hService);
//    end;
//    CloseServiceHandle(hSCManager);
//  end;
//end;

//Function StartService(ServiceName: string): boolean;
//var
//  hSCManager: HANDLE;
//  hService: HANDLE;
//begin
//  hSCManager := OpenServiceManager();
//  Result := false;
//  if (hSCManager <> 0) then begin
//    hService := OpenService(hSCManager, ServiceName, SERVICE_START);
//    if (hService <> 0) then begin
//      Result := StartNTService(hService, 0, 0);
//      CloseServiceHandle(hService);
//    end;
//    CloseServiceHandle(hSCManager);
//  end;
//end;

//Function IsServiceRunning(ServiceName: string): boolean;
//var
//  hSCManager: HANDLE;
//  hService: HANDLE;
//  sStatus: SERVICE_STATUS;
//begin
//  hSCManager := OpenServiceManager();
//  Result := false;
//  if (hSCManager <> 0) then begin
//    hService := OpenService(hSCManager, ServiceName, SERVICE_QUERY_STATUS);
//    if (hService <> 0) then begin
//      if (QueryServiceStatus(hService, sStatus)) then
//        Result := (sStatus.dwCurrentState = SERVICE_RUNNING);
//      CloseServiceHandle(hService);
//    end;
//    CloseServiceHandle(hSCManager);
// end;
//end;

//Function ChangeServiceStartup(ServiceName: string; dwStartType: cardinal): boolean;
//var
//  hSCManager: HANDLE;
//  hService: HANDLE;
//begin
//  hSCManager := OpenServiceManager();
//  Result := false;
//  if (hSCManager <> 0) then begin
//    hService := OpenService(hSCManager, ServiceName, SERVICE_CHANGE_CONFIG);
//    if (hService <> 0) then begin
//       Result := ChangeServiceConfig(hService, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, '','',0,'','','','');
//       CloseServiceHandle(hService);
//    end;
//    CloseServiceHandle(hSCManager);
//  end;
//end;

// Check if current install path exists
Function DoesPathExist(): boolean;
Begin
    if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Envy','', CurrentPath) then
        Result := DirExists(CurrentPath)
    else
        Result := False;
End;

Function NextButtonClick(CurPageID: integer): Boolean;
var
  Wnd: HWND;
  Shutdownmessage: string;
begin
  Result := True;
  if (CurPageID = wpWelcome) then begin
    Wnd := FindWindowByClassName('EnvyMainWnd');
    if Wnd <> 0 then begin
      Shutdownmessage := ExpandConstant('{cm:dialog_shutdown,Envy}');
      if MsgBox(Shutdownmessage, mbConfirmation, MB_OKCANCEL) = IDOK then begin
        SendMessage(Wnd, WM_CLOSE, 0, 0);
        while Wnd <> 0 do begin
          Sleep(100);
          Wnd := FindWindowByClassName('EnvyMainWnd');
        end;
      end else Result := False;
    end;
  end;
end;

Function MalwareCheck(MalwareFile: string): Boolean;
Begin
  Result := False;
  if FileExists( MalwareFile ) then Begin
    if MsgBox(ExpandConstant( '{cm:dialog_malwaredetected,' + MalwareFile + '}' ), mbConfirmation, MB_YESNO) = IDYES then begin
      Result := True;
    End;
    MalwareDetected := True;
  End;
End;

Function InitializeSetup: Boolean;
Begin
  // Malware checks
  Installed := (RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc1, KeyName) or RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc2, KeyName)) and DoesPathExist();
  MalwareDetected := False;
  Result := True;
  Result := NOT MalwareCheck( ExpandConstant('{win}\vgraph.dll') );
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{win}\Shareaza*') ); End;
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{sys}\Shareaza*') ); End;
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{win}\Envy*') ); End;
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{sys}\Envy*') ); End;
  //if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{pf}\Envy\vc2.dll') ); End;
End;

Function IsMalwareDetected: Boolean;
Begin
  Result := MalwareDetected;
End;

// Was EnableDeleteOldSetup
Function WasInstalled: Boolean;
Begin
  Result := Installed;
End;

Function ShouldSkipPage(PageID: Integer): Boolean;
Begin
  Result := False;
  if PageID = wpSelectDir then Result := Installed;
End;

Procedure DeleteFolder(Param: String);
var
  Foldername: string;
Begin
  Foldername := ExpandConstant(Param);
  DelTree(Foldername, True, True, True);
End;

Procedure DeleteFile(Param: String);
var
  Filename: string;
Begin
  Filename := ExpandConstant(Param);
  DelayDeleteFile(Filename,3);
End;

// Update Tasks Page
// We don't allow to modify the setting of MultiUser if already selected.
// See http://www.jrsoftware.org/ishelp/index.php?topic=scriptevents & Pages.iss
Procedure CurPageChanged(const CurrentPage: integer);
var
  i : integer;
  MultiUserValue: DWORD;
Begin
  if CurrentPage = wpSelectTasks then begin
    i := WizardForm.TasksList.Items.IndexOf(ExpandConstant('{cm:tasks_multisetup}'));
    if i <> -1 then begin
      if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Envy\Envy', 'MultiUser', MultiUserValue) then begin
        Wizardform.TasksList.Checked[i] := (MultiUserValue = 1);
        WizardForm.TasksList.ItemEnabled[i] := false;
      End;
    End;
    i := WizardForm.TasksList.Items.IndexOf(ExpandConstant('{cm:tasks_webhook}'));
    if i <> -1 then begin
      if RegQueryDWordValue(HKEY_CURRENT_USER, 'SOFTWARE\Envy\Envy\Downloads', 'WebHookEnable', MultiUserValue) then begin
        Wizardform.TasksList.Checked[i] := (MultiUserValue = 1);
      End;
    End;
#if alpha == "True" | ConfigurationName == "Debug"
    i := WizardForm.TasksList.Items.IndexOf(ExpandConstant('{cm:tasks_deleteoldsetup}'));
    if i <> -1 then begin
       Wizardform.TasksList.Checked[i] := false;
       WizardForm.TasksList.ItemEnabled[i] := false;
    End;
#endif
  End;
End;

Function WeOwnTorrentAssoc: boolean;
var
  CommandString: string;
  Position: Integer;
Begin
  Result := False;
  if RegQueryStringValue(HKEY_CLASSES_ROOT, 'BitTorrent\shell\open\command','', CommandString) then
    Begin
      Position := Pos('Envy.exe', CommandString);
      Result := (Position > 0);
    End
End;

Procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  InstallFolder: string;
  FirewallManager: Variant;
  FirewallProfile: Variant;
  Wnd: HWND;
Begin
  if CurUninstallStep = usUninstall then begin
    if InstallOnThisVersion('0,5.01sp2','0,0') = irInstall then begin
      try
        InstallFolder := ExpandConstant('{app}\Envy.exe');
        FirewallManager := CreateOleObject('HNetCfg.FwMgr');
        FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
        FirewallProfile.AuthorizedApplications.Remove(InstallFolder);
      except
      End;
    End;
    Wnd := FindWindowByClassName('EnvyMainWnd');
    if Wnd <> 0 then begin
      SendMessage(Wnd, WM_CLOSE, 0, 0);
      while Wnd <> 0 do
        begin
          Sleep(100);
          Wnd := FindWindowByClassName('EnvyMainWnd');
        End;
    End;
    if WeOwnTorrentAssoc then begin
      RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT,'.torrent');
      RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT,'BitTorrent');
    End;
  End;
End;

// Languages:

Function IsLanguageRTL(LangCode: String): String;
Begin
  if ( (LangCode = 'he') or (LangCode = 'ar') ) then
    Result := '1'
  else
    Result := '0';
End;

Function GetRelFilePath(LangCode: String): String;
Begin
  StringChangeEx(LangCode, '_', '-', True);

  if ( LangCode = 'en-uk' ) then
    Result := 'Languages\alt.xml'
  else if ( LangCode = 'pt-br' ) then
    Result := 'Languages\pt.xml'
  else
    Result := 'Languages\' + LangCode + '.xml';
End;

Function ResetLanguages: boolean;
var
  Names: TArrayOfString;
  I: Integer;
  S: String;
  Value: String;
begin
  if RegGetValueNames(HKEY_CURRENT_USER, 'Software\Envy\Envy\Skins', Names) then
  begin
    S := '';
    Value := LowerCase(GetRelFilePath(ExpandConstant('{language}')));
    for I := 0 to GetArrayLength(Names)-1 do
    begin
      S := LowerCase(Names[I]);
      if Pos('languages', S) <> 0 then
        if Value <> S then
          RegWriteDWordValue(HKEY_CURRENT_USER, 'Software\Envy\Envy\Skins', S, 0);
    end;
    RegWriteDWordValue(HKEY_CURRENT_USER,  'Software\Envy\Envy\Skins', Value, 1);
    Value := IsLanguageRTL(ExpandConstant('{language}'));
    RegWriteDWordValue(HKEY_CURRENT_USER,  'Software\Envy\Envy\Settings', 'LanguageRTL', StrToInt(Value));
    RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Envy\Envy\Settings', 'Language', ExpandConstant('{language}'));
    // Set default values for other users
    RegWriteDWordValue(HKEY_LOCAL_MACHINE,  'Software\Envy\Envy', 'DefaultLanguageRTL', StrToInt(Value));
    RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Envy\Envy', 'DefaultLanguage', ExpandConstant('{language}'));
  end;
  Result := True;
end;

// Taskbar Pinning

//#ifdef UNICODE
//Function LoadString(hInstance: LongInt; uID: UINT; lpBuffer: string; nBufferMax: Integer): Integer;
//  external 'LoadStringW@user32.dll stdcall delayload';
//Function LoadLibrary(lpFileName: string): LongInt;
//  external 'LoadLibraryW@kernel32.dll stdcall delayload';
//#else
//Function LoadString(hInstance: LongInt; uID: UINT; lpBuffer: string; nBufferMax: Integer): Integer;
//  external 'LoadStringA@user32.dll stdcall delayload';
//Function LoadLibrary(lpFileName: string): LongInt;
//  external 'LoadLibraryA@kernel32.dll stdcall delayload delayload';
//#endif
//Function FreeLibrary(hModule: LongInt): BOOL;
//  external 'FreeLibrary@kernel32.dll stdcall delayload';

//procedure PinAppTaskbar;
//var
//  vShell, vFolder, vFolderItem, vItemVerbs: Variant;
//  vPath, vApp: Variant;
//  i: Integer;
//  h: LongInt;
//  VerbName: String;
// sItem: String;
//  szPinName: String;
//  filenameEnd: Integer;
//  filename: String;
//  strEnd: String;
//begin
//  vShell := CreateOleObject('Shell.Application');
//  vFolder := vShell.Namespace(ExpandConstant('{app}'));
//  vFolderItem := vFolder.ParseName('Envy.exe');
//  vItemVerbs := vFolderItem.Verbs();
//  for i := 0 to vItemVerbs.count() do
//  begin
//     VerbName := lowercase(vItemVerbs.item(i).name);
//     StringChangeEx(VerbName,'&','',true);
//     if (CompareText(VerbName, 'Pin to Taskbar') = 0) then
//       vItemVerbs.item(i).DoIt
//  // if (CompareText(Verbname, 'Pin to Start Menu') = 0) then
//  //   vItemVerbs.item(i).DoIt
//  end;
//end;

// Do Tasks (Pinning, Firewall, Download Paths):

Procedure CurStepChanged(CurStep: TSetupStep);
var
  InstallFolder: string;
  FirewallObject: Variant;
  FirewallManager: Variant;
  FirewallProfile: Variant;
//Success: boolean;
  Reset: boolean;
  Path: string;
Begin
  if CurStep=ssPostInstall then begin
//  if IsTaskSelected('firewall') then begin
    if (not Installed) then begin
      if WizardSilent = True then begin
        try
          FirewallObject := CreateOleObject('HNetCfg.FwAuthorizedApplication');
          InstallFolder := ExpandConstant('{app}\Envy.exe');
          FirewallObject.ProcessImageFileName := InstallFolder;
          FirewallObject.Name := 'Envy';
          FirewallObject.Scope := NET_FW_SCOPE_ALL;
          FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
          FirewallObject.Enabled := True;
          FirewallManager := CreateOleObject('HNetCfg.FwMgr');
          FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
          FirewallProfile.AuthorizedApplications.Add(FirewallObject);
        except
        End;
      End else begin
        FirewallFailed := ExpandConstant('{cm:dialog_firewall}')
        try
          FirewallObject := CreateOleObject('HNetCfg.FwAuthorizedApplication');
          InstallFolder := ExpandConstant('{app}\Envy.exe');
          FirewallObject.ProcessImageFileName := InstallFolder;
          FirewallObject.Name := 'Envy';
          FirewallObject.Scope := NET_FW_SCOPE_ALL;
          FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
          FirewallObject.Enabled := True;
          FirewallManager := CreateOleObject('HNetCfg.FwMgr');
          FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
          FirewallProfile.AuthorizedApplications.Add(FirewallObject);
        except
          MsgBox(FirewallFailed, mbInformation, MB_OK);
        End;
      End;
    End;

//  if IsTaskSelected('upnp') then begin
//    if (HasUserPrivileges) then begin
//      Success := false;
//      if (IsServiceInstalled('SSDPSRV') and IsServiceInstalled('upnphost')) then begin
//        if (not IsServiceRunning('SSDPSRV')) then begin
//          // Change the startup type to manual if it was disabled;
//          // we don't need to start it since UPnP Device Host service depends on it;
//          // assuming the user didn't modify the dependencies manually.
//          // Note: we could probably elevate user rights with AdjustTokenPrivileges(?)
//          Success := ChangeServiceStartup('SSDPSRV', SERVICE_DEMAND_START);
//        end else
//          Success := true;
//        if (Success) then begin
//          // We succeeded to change the startup type, so we will change another service
//          Success := ChangeServiceStartup('upnphost', SERVICE_AUTO_START);
//          if (Success and not IsServiceRunning('upnphost')) then
//            StartService('upnphost');
//        end;
//      end;
//    end;
//  end;
  End;

  // XP Firewall
//if CurStep=ssInstall then begin
//  if not IsTaskSelected('firewall') then begin
//    if InstallOnThisVersion('0,5.01sp2','0,0') = irInstall then begin
//      try
//        InstallFolder := ExpandConstant('{app}\Envy.exe');
//        FirewallManager := CreateOleObject('HNetCfg.FwMgr');
//        FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
//        FirewallProfile.AuthorizedApplications.Remove(InstallFolder);
//      except
//      End;
//    End;
//  End;
//End;

  // Win 7+ Shortcut Pinning
  //if CurStep=ssPostInstall then begin
  //  if IsTaskSelected('pintaskbar') then begin
  //    PinAppTaskbar;
  //  End;
  //End;

  // Check if the needed paths exist otherwise delete it from the registry (They will be recreated later in the installation process)
  if CurStep=ssInstall then begin
    if RegQueryStringValue(HKEY_CURRENT_USER,  'SOFTWARE\Envy\Envy\Downloads', 'CompletePath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Envy\Envy\Downloads', 'CompletePath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,complete}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER,  'SOFTWARE\Envy\Envy\Downloads', 'IncompletePath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Envy\Envy\Downloads', 'IncompletePath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,incomplete}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER,  'SOFTWARE\Envy\Envy\Downloads', 'CollectionPath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Envy\Envy\Downloads', 'CollectionPath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,collection}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER,  'SOFTWARE\Envy\Envy\Downloads', 'TorrentPath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Envy\Envy\Downloads', 'TorrentPath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,torrent}'), mbError, MB_OK);
      End;
    End;
  End;

  if CurStep=ssDone then Reset := ResetLanguages;
End;

{ Pull in custom wizard pages }
#include "Pages.iss"

#expr SaveToFile("Preprocessed.iss")
