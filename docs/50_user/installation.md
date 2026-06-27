# Installation Guide

This guide covers installing and setting up Envy on your Windows system.

## 📋 System Requirements

### Minimum Requirements
- **Operating System**: Windows 7 SP1 or later
- **Processor**: 1 GHz or faster
- **Memory**: 512 MB RAM
- **Storage**: 100 MB available space
- **Display**: 1024x768 resolution

### Recommended Requirements
- **Operating System**: Windows 10 or later
- **Processor**: 2 GHz dual-core or better
- **Memory**: 2 GB RAM or more
- **Storage**: 500 MB available space
- **Display**: 1920x1080 resolution or higher

### Supported Platforms
- **Windows 10** (32-bit and 64-bit)
- **Windows 11** (32-bit and 64-bit)
- **Windows Server** (2016, 2019, 2022)

## 📥 Downloading Envy

### Official Releases

1. Visit the [GitHub Releases](../../releases) page
2. Download the latest version:
   - `Envy-X.X.X-Setup.exe` (Installer)
   - `Envy-X.X.X-Portable.zip` (Portable version)
3. Verify the download (check file hash if provided)

### Beta Versions

For testing new features:
1. Go to [GitHub Actions](../../actions)
2. Find the latest successful build
3. Download the artifact from the build summary

### Alternative Sources

**⚠️ Warning**: Only download from official sources to avoid malware.

- **SourceForge**: https://sourceforge.net/projects/getenvy/
- **Official Website**: https://getenvy.com (if available)

## 🚀 Installation Process

### Using the Installer (Recommended)

1. **Locate** the downloaded `Envy-X.X.X-Setup.exe` file
2. **Right-click** and select "Run as administrator" (recommended)
3. **Follow the setup wizard**:
   - Welcome screen → Click "Next"
   - License agreement → Read and accept terms
   - Installation location → Choose or accept default
   - Component selection → Select desired features
   - Start menu shortcuts → Choose options
   - Ready to install → Click "Install"

4. **Complete installation**:
   - Wait for installation to finish
   - Click "Finish" to launch Envy

### Portable Installation

1. **Extract** the `Envy-X.X.X-Portable.zip` file
2. **Choose** a location (USB drive, folder, etc.)
3. **Run** `Envy.exe` directly from the extracted folder

**Note**: Portable version stores settings in the same folder.

## ⚙️ First-Time Setup

### Initial Configuration Wizard

When you first run Envy, the setup wizard will guide you through:

1. **Welcome Screen**
   - Introduction to Envy
   - Quick start tips
   - Privacy notice

2. **User Profile**
   - Choose username
   - Set user avatar (optional)
   - Configure chat settings

3. **Network Configuration**
   - Select P2P networks to join
   - Configure connection settings
   - Set bandwidth limits

4. **Library Setup**
   - Choose download folder
   - Set up file sharing
   - Configure library organization

5. **Security Settings**
   - Content filtering preferences
   - Privacy options
   - Security features

### Manual Configuration

Skip the wizard and configure manually:

1. **Basic Settings**:
   - General → Interface → Language
   - Downloads → Folders → Choose download location
   - Connection → Networks → Enable desired protocols

2. **Network Setup**:
   - Connection → Advanced → Configure ports
   - Connection → Bandwidth → Set speed limits
   - Security → Access → Configure filters

## 🔧 Post-Installation Tasks

### Windows Firewall Configuration

Envy needs network access for P2P functionality:

1. **Allow through firewall**:
   - First run will prompt for permission
   - Or manually: Windows Security → Firewall → Allow app

2. **Port configuration**:
   - TCP port: Usually 6346 (Gnutella2)
   - UDP port: Usually 6346 (Gnutella2)
   - Additional ports for other networks

### Antivirus Software

Some antivirus programs may flag P2P applications:

1. **Add exceptions** for Envy executable
2. **Exclude** the Envy installation folder
3. **Exclude** download folders if needed

**Note**: Envy is open source and safe, but P2P networks can carry risks.

### Windows Defender Configuration

1. **Open Windows Security**
2. **Virus & threat protection**
3. **Manage settings → Exclusions**
4. **Add exclusion** for Envy folder

## 📁 File Locations

### Installation Directories

**Standard Installation:**
```
C:\Program Files\Envy\
├── Envy.exe          # Main executable
├── Plugins\          # Plugin DLLs
├── Languages\        # Translation files
├── Skins\           # UI themes
└── Data\            # Default data files
```

**User Data:**
```
%APPDATA%\Envy\
├── Settings.xml     # User configuration
├── Library.xml      # File library data
├── Security.dat     # Security settings
├── GeoIP.dat        # Geographic data
└── Logs\           # Application logs
```

### Portable Version
All files are contained in the extraction folder:
```
Envy-Portable\
├── Envy.exe
├── Settings.xml     # Stored locally
├── Data\
└── User Library\
```

## 🔄 Upgrading from Previous Versions

### From Official Installer

1. **Download** new version installer
2. **Run installer** - it will detect existing installation
3. **Choose upgrade option**
4. **Settings preserved** automatically

### Manual Upgrade

1. **Backup settings** (optional, usually preserved)
2. **Uninstall old version**
3. **Install new version**
4. **Restore settings** if needed

### Migrating from Other Clients

Envy can import from other P2P clients:

1. **File → Import**
2. **Select client type** (Shareaza, etc.)
3. **Choose import options**
4. **Import library and settings**

## 🧪 Testing Installation

### Basic Functionality Test

1. **Launch Envy**
2. **Check connection status** (bottom of window)
3. **Try a simple search** ("test" or similar)
4. **Verify download folder** is accessible

### Network Connectivity Test

1. **View network statistics** (Help → System Info)
2. **Check peer connections** (Search tab → Network panel)
3. **Test file download** (find and download a small file)
4. **Verify upload functionality**

### Troubleshooting Tests

- **Port test**: Tools → Check Firewall
- **Hash verification**: Test with known good file
- **Plugin loading**: Check Plugins → About
- **Skin loading**: Try different themes

## 🚫 Uninstalling Envy

### Using Windows Programs and Features

1. **Open Settings → Apps → Apps & features**
2. **Find Envy in the list**
3. **Click Uninstall**
4. **Follow uninstaller prompts**

### Manual Removal

If uninstaller fails:

1. **Delete installation folder**
2. **Remove start menu shortcuts**
3. **Delete user data** (%APPDATA%\Envy)
4. **Remove registry entries** (use regedit carefully)

### Data Preservation

To keep your settings and library:

1. **Don't delete** %APPDATA%\Envy folder
2. **Backup** important data before uninstalling
3. **Restore** data when reinstalling

## 🆘 Installation Issues

### Common Problems

#### "Setup failed" Error
- Run installer as administrator
- Disable antivirus temporarily
- Check available disk space
- Try different installation location

#### "Application won't start"
- Check Windows Event Viewer for errors
- Verify all files were installed
- Try running as administrator
- Check for missing dependencies

#### Network Connection Issues
- Verify firewall settings
- Check port availability
- Try different port numbers
- Restart router/modem

### Getting Help

- **Installation logs**: Check %TEMP% for setup logs
- **System requirements**: Verify your system meets minimum specs
- **Compatibility mode**: Try running in Windows 7 compatibility mode
- **Clean boot**: Test with minimal startup programs

### Advanced Troubleshooting

1. **Dependency check**:
   - Visual C++ Redistributables
   - .NET Framework (if required)
   - Windows updates

2. **System file check**:
   ```
   sfc /scannow
   ```

3. **Clean installation**:
   - Remove all Envy files and registry entries
   - Restart computer
   - Fresh installation

## 📞 Support

### Installation Support

- **Documentation**: This guide and [User Guide](guide.md)
- **GitHub Issues**: [Report installation problems](../../issues)
- **Community Forums**: Ask other users
- **IRC Channel**: Real-time help (if available)

### System Information

When reporting issues, include:
- Windows version and architecture
- Envy version number
- Installation method (installer/portable)
- Error messages or symptoms
- Steps to reproduce the problem

---

**Installation Complete!** 🎉

Envy is now ready to use. Continue with the [User Guide](guide.md) to learn how to search for and download files.

---

**Last Updated:** January 15, 2026
