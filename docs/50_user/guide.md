# User Guide

Welcome to Envy! This guide will help you get started with the multi-network P2P file sharing application.

## 🚀 Getting Started

### Installation

1. **Download** the latest release from the [releases page](../../releases)
2. **Run the installer** and follow the setup wizard
3. **Launch Envy** from your desktop or start menu

### First Time Setup

When you first run Envy, you'll see:

1. **Welcome Screen**: Basic introduction and tips
2. **Settings Wizard**: Configure your preferences
3. **Network Setup**: Choose which P2P networks to join

## 🖥️ User Interface

### Main Window

The main Envy window consists of several key areas:

- **Toolbar**: Quick access to common functions
- **Tab Bar**: Switch between different views
- **Main Panel**: Content area for current tab
- **Status Bar**: Connection status and statistics

### Available Tabs

- **Home**: Quick access to recent activity
- **Library**: Your downloaded and shared files
- **Search**: Find files across P2P networks
- **Downloads**: Active and completed downloads
- **Uploads**: Files you're sharing
- **Chat**: Connect with other users
- **IRC**: Internet Relay Chat channels

## 🔍 Searching for Files

### Basic Search

1. **Click the Search tab**
2. **Enter keywords** in the search box
3. **Select networks** to search (BitTorrent, Gnutella2, etc.)
4. **Click Search** or press Enter

### Advanced Search Options

- **File Type**: Filter by music, video, documents, etc.
- **Size Range**: Specify minimum/maximum file sizes
- **Quality**: Look for high-quality or specific formats
- **Network**: Search specific P2P networks

### Search Results

Results show:
- **File name** and size
- **Source count**: How many users have the file
- **Network**: Which P2P network it's from
- **Rating**: Community feedback (if available)

## 📥 Downloading Files

### Starting a Download

1. **Find a file** using search
2. **Right-click** the result
3. **Select "Download"** or "Download and Open"
4. **Choose save location** (optional)

### Download Management

The Downloads tab shows:
- **Active downloads**: Currently transferring
- **Queued downloads**: Waiting for sources
- **Completed downloads**: Finished successfully
- **Failed downloads**: Need attention

### Download Options

- **Pause/Resume**: Control download progress
- **Priority**: Set download speed priority
- **Sources**: View and manage peer connections
- **Preview**: View file before completion (limited)

## 📤 Sharing Files

### Adding Files to Library

1. **Drag and drop** files into the Library tab
2. **Use "Add Files"** from the toolbar
3. **Browse and select** files to share

### Managing Your Library

- **Organize**: Create folders and categories
- **Metadata**: Edit file information
- **Sharing**: Control what's shared
- **Hashing**: Verify file integrity

## ⚙️ Settings and Configuration

### General Settings

- **Interface**: Language, theme, and layout
- **Downloads**: Speed limits, save locations
- **Uploads**: Bandwidth allocation, queue management
- **Security**: Content filtering, privacy options

### Network Settings

- **Connections**: Maximum peers, ports
- **Networks**: Enable/disable P2P networks
- **Bandwidth**: Upload/download limits
- **Proxy**: Network proxy configuration

### Advanced Options

- **Hashing**: Algorithm preferences
- **Chat**: IRC server settings
- **Plugins**: Extension management
- **Debugging**: Logging and diagnostics

## 🔒 Security and Privacy

### Content Filtering

- **IP Filters**: Block unwanted connections
- **Content Rules**: Filter by file type or name
- **Spam Protection**: Automated spam detection
- **Adult Content**: Optional filtering

### Privacy Features

- **Anonymous Mode**: Hide your identity
- **Connection Encryption**: Secure peer connections
- **No Logs**: Minimal activity logging
- **Custom User Agent**: Disguise your client

## 🌐 Network Features

### Supported Networks

- **BitTorrent**: Popular torrent protocol
- **Gnutella2**: Advanced P2P network
- **eDonkey2000**: Large file sharing network
- **Gnutella**: Classic P2P protocol

### Magnet Links and Torrents

- **Magnet Links**: Click to start download
- **Torrent Files**: Open .torrent files directly
- **DHT**: Decentralized peer discovery
- **PEX**: Peer exchange for better connections

## 💬 Communication

### Chat Features

- **Private Messages**: Direct user communication
- **Chat Rooms**: Join discussion groups
- **File Transfers**: Send files in chat
- **Emoticons**: Express yourself

### IRC Integration

- **Channels**: Join interest-based rooms
- **Bots**: Automated helpers and services
- **Commands**: Special IRC commands
- **Logging**: Save conversation history

## 🎨 Customization

### Skins and Themes

- **Built-in Themes**: Choose from included skins
- **Custom Skins**: Download community themes
- **Color Schemes**: Customize colors
- **Layout Options**: Adjust window layout

### Plugins and Extensions

- **Document Readers**: View different file types
- **Media Players**: Enhanced media support
- **Search Extensions**: Additional search sources
- **Utility Plugins**: Extra functionality

## 📊 Monitoring and Statistics

### Search Monitor

The **Search Monitor** provides real-time visibility into search activity across all connected P2P networks:

#### Accessing Search Monitor
- Go to **View → Search Monitor** or press `Ctrl+F2`
- Right-click in the main window and select **Search Monitor**

#### Features
- **Real-time Search Tracking**: See all search queries happening on the network
- **Protocol Identification**: Distinguish between Gnutella2, Gnutella1, eDonkey, and BitTorrent searches
- **Endpoint Information**: View IP addresses and ports of searching clients

#### Advanced Filtering
The Search Monitor includes powerful filtering capabilities:

1. **Text Filter**: Search for specific keywords within search queries
2. **Protocol Filter**: Show only searches from specific networks (G2, G1, ED2K, DC++)
3. **Size Filter**: Filter by minimum/maximum file size ranges
4. **IP Filter**: Filter searches from specific IP addresses or ranges (supports wildcards)
5. **Schema Filter**: Filter by metadata schema types

**Using Filters:**
- Right-click in Search Monitor → **Advanced Filter...**
- Configure your filtering criteria
- Click **Apply** to activate filters
- Use **Remove Filter** to clear all active filters

**Note:** The filter options appear in the context menu after the "Clear Buffer" option.

### Network Statistics

- **Connection Status**: Current network health
- **Transfer Rates**: Upload/download speeds
- **Peer Count**: Connected users
- **File Count**: Available content

### Performance Metrics

- **CPU Usage**: System resource consumption
- **Memory Usage**: RAM utilization
- **Disk I/O**: File system activity
- **Network I/O**: Bandwidth usage

## 🔧 Troubleshooting

### Common Issues

#### Connection Problems
- Check firewall settings
- Verify port configuration
- Try different network settings
- Restart the application

#### Slow Downloads
- Increase connection limits
- Check bandwidth settings
- Try different sources
- Verify peer quality

#### Search Issues
- Ensure networks are enabled
- Try different keywords
- Check network connectivity
- Wait for network stabilization

### Getting Help

- **Built-in Help**: Press F1 for context help
- **Online Documentation**: Visit the project website
- **Community Forums**: Ask other users
- **Issue Reports**: Report bugs on GitHub

## 📚 Advanced Features

### Multi-Source Downloads

Envy can download from multiple sources simultaneously:
- **Automatic source management**
- **Bandwidth optimization**
- **Redundant connections**
- **Resume support**

### Hash Verification

- **SHA-1, MD5, Tiger**: Standard hash algorithms
- **ED2K**: eDonkey2000 hashing
- **BitTorrent**: Piece hash verification
- **AICH**: Advanced Intelligent Corruption Handler

### Queue Management

- **Download Queues**: Organize downloads
- **Priority Levels**: Control download order
- **Bandwidth Allocation**: Share upload capacity
- **Scheduling**: Time-based controls

## 🔄 Updates and Maintenance

### Checking for Updates

- **Automatic Updates**: Configure in settings
- **Manual Check**: Use Help → Check for Updates
- **Beta Versions**: Opt into beta testing
- **Release Notes**: Read about new features

### Maintenance Tasks

- **Library Cleanup**: Remove unwanted files
- **Cache Clearing**: Free up disk space
- **Database Optimization**: Improve performance
- **Security Updates**: Install latest patches

## 📞 Support and Community

### Official Resources

- **Documentation**: Comprehensive user guides
- **Website**: News and announcements
- **Forums**: Community discussions
- **Bug Tracker**: Report issues and request features

### Community Support

- **User Forums**: Help from other users
- **IRC Channels**: Real-time assistance
- **Social Media**: Follow for updates
- **Mailing Lists**: Subscribe for announcements

---

**Enjoy using Envy!** 🎉

For the latest information, visit the [project website](https://getenvy.com) or check the [GitHub repository](../../).

---

**Last Updated:** January 15, 2026
