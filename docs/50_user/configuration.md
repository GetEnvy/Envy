# Configuration Guide

This guide explains how to configure Envy for optimal performance and your specific needs.

## ⚙️ Accessing Settings

### Settings Dialog

1. **Tools → Settings** (or Ctrl+P)
2. **Navigate** through the category tree
3. **Modify** settings as needed
4. **Apply** or **OK** to save changes

### Advanced Settings

For power users:
1. **Tools → Advanced Settings**
2. **Edit** configuration files directly
3. **Backup** before making changes

## 🌐 General Settings

### Interface

- **Language**: Choose from available translations
- **Theme**: Select visual theme/skin
- **Layout**: Customize window layout
- **Tooltips**: Enable/disable help text
- **Sounds**: Configure notification sounds

### Downloads

- **Save Location**: Default download folder
- **File Organization**: Auto-create subfolders
- **Completion Action**: What to do when downloads finish
- **Verification**: Hash checking options

### Uploads

- **Bandwidth Limit**: Maximum upload speed
- **Queue Management**: How many uploads to allow
- **Share Limits**: Maximum shares per user
- **Prioritization**: Favorite users get priority

## 🔌 Connection Settings

### Network Configuration

#### Basic Settings
- **Networks**: Enable/disable P2P networks
  - BitTorrent: Modern torrent protocol
  - Gnutella2: Advanced P2P network
  - eDonkey2000: Large file network
  - Gnutella: Classic P2P

- **Ports**:
  - TCP Port: Usually 6346 (Gnutella2)
  - UDP Port: Usually 6346 (Gnutella2)
  - Randomize: Let system choose ports

#### Advanced Settings
- **Maximum Connections**: Total peer connections
- **Per Network Limits**: Connections per protocol
- **Timeout Settings**: Connection timeouts
- **Reconnection**: Auto-retry failed connections

### Bandwidth Management

- **Global Limits**:
  - Download speed cap
  - Upload speed cap
  - Apply to all networks

- **Per-Network Limits**:
  - Different speeds for different protocols
  - Prioritize certain networks

- **Throttling**:
  - Time-based limits (day/night)
  - Application priority settings

## 🔒 Security Settings

### Content Filtering

- **IP Filters**:
  - Block countries/regions
  - Block specific IP ranges
  - Allow only trusted IPs

- **Content Rules**:
  - Filter by file type
  - Filter by file name patterns
  - Block adult content

- **Spam Protection**:
  - Automated spam detection
  - User reporting system
  - Blacklist management

### Privacy Options

- **Anonymous Mode**:
  - Hide user identity
  - Disable chat features
  - Minimal information sharing

- **Connection Security**:
  - Encrypted connections when possible
  - Certificate validation
  - Secure protocol preferences

## 📁 Library Settings

### File Management

- **Library Folders**:
  - Add/remove watched folders
  - Automatic file detection
  - Exclude patterns

- **File Organization**:
  - Auto-categorization
  - Custom folder structure
  - Metadata management

- **Sharing Options**:
  - Share ratio limits
  - Partial file sharing
  - Private files exclusion

### Hashing and Verification

- **Hash Algorithms**:
  - SHA-1 (default)
  - MD5, Tiger, ED2K
  - Multiple hash support

- **Verification Settings**:
  - Verify on completion
  - Re-verify existing files
  - Hash cache management

## 💬 Communication Settings

### Chat Configuration

- **User Profile**:
  - Username and avatar
  - Away messages
  - Profile information

- **Chat Options**:
  - Timestamps on messages
  - Emoticon support
  - Message history

- **Privacy**:
  - Block unwanted users
  - Ignore lists
  - Private message controls

### IRC Settings

- **Server Configuration**:
  - Default IRC servers
  - Custom server addition
  - Auto-connect options

- **Channel Management**:
  - Auto-join channels
  - Channel passwords
  - Favorite channels

## 🔧 Advanced Settings

### Performance Tuning

#### Memory Management
- **Cache Sizes**: Various internal caches
- **Buffer Sizes**: Network and file buffers
- **Thread Pools**: Worker thread counts

#### CPU Optimization
- **Thread Priority**: Process priority level
- **Background Processing**: When to use CPU
- **Multi-threading**: Enable parallel operations

### Network Advanced

#### Protocol Specific
- **BitTorrent**:
  - DHT enable/disable
  - PEX (Peer Exchange)
  - Magnet link support

- **Gnutella2**:
  - Hub routing preferences
  - Query routing options
  - Network size limits

- **eDonkey2000**:
  - Kad network settings
  - Server list management
  - CryptLayer options

#### Connection Tuning
- **TCP Optimization**:
  - Window sizes
  - Keep-alive settings
  - Congestion control

- **UDP Settings**:
  - Packet sizes
  - Retransmission timeouts
  - Multicast options

### Debugging and Diagnostics

- **Logging Levels**:
  - Error, Warning, Info, Debug
  - Per-component logging
  - Log file rotation

- **Performance Monitoring**:
  - Transfer statistics
  - Memory usage tracking
  - Network diagnostics

## 🎨 Appearance Settings

### Skins and Themes

- **Built-in Themes**:
  - Light and dark themes
  - High contrast options
  - Accessibility themes

- **Custom Skins**:
  - Skin installation
  - Skin management
  - Skin updates

### Window Layout

- **Main Window**:
  - Tab arrangement
  - Panel sizes
  - Toolbar customization

- **Dialog Windows**:
  - Default positions
  - Size preferences
  - Modal behavior

## 🔄 Automation Settings

### Scheduled Tasks

- **Download Scheduling**:
  - Time-based download limits
  - Bandwidth throttling schedules
  - Network availability checks

- **Maintenance Tasks**:
  - Automatic library cleanup
  - Cache clearing schedules
  - Database optimization

### Startup and Shutdown

- **Startup Options**:
  - Auto-start with Windows
  - Minimize to tray
  - Connect to networks automatically

- **Shutdown Behavior**:
  - Complete active downloads
  - Save settings automatically
  - Clean shutdown procedures

## 📊 Statistics and Monitoring

### Data Collection

- **Usage Statistics**:
  - Transfer volumes
  - Network activity
  - File sharing metrics

- **Performance Metrics**:
  - CPU and memory usage
  - Network throughput
  - Error rates

### Privacy Considerations

- **Data Sharing**:
  - Anonymous statistics
  - Crash reporting
  - Feature usage analytics

- **Local Storage**:
  - Log retention policies
  - Cache size limits
  - Temporary file cleanup

## 🔧 Configuration Files

### Manual Editing

**⚠️ Warning**: Backup files before editing manually.

#### Settings.xml
```xml
<Settings>
  <Interface>
    <Language>en</Language>
    <Theme>default</Theme>
  </Interface>
  <Connection>
    <Port>6346</Port>
    <MaxConnections>128</MaxConnections>
  </Connection>
</Settings>
```

#### Advanced Configuration
- Located in `%APPDATA%\Envy\`
- XML format for most settings
- Restart required for some changes

### Backup and Restore

- **Automatic Backups**: Settings backed up on changes
- **Manual Backup**: Tools → Export Settings
- **Restore**: Tools → Import Settings
- **Reset to Defaults**: Tools → Reset Settings

## 🧪 Testing Configuration

### Configuration Validation

1. **Check Settings**: Tools → Validate Configuration
2. **Network Test**: Tools → Test Network
3. **Performance Test**: Tools → Run Diagnostics

### Troubleshooting

- **Reset Problematic Settings**: Individual section reset
- **Configuration Repair**: Automatic repair of corrupted settings
- **Log Analysis**: Check logs for configuration errors

## 📞 Support

### Configuration Help

- **Built-in Help**: Press F1 in settings dialogs
- **Online Documentation**: This guide and [User Guide](guide.md)
- **Community Support**: Forums and IRC channels
- **Issue Reports**: [GitHub Issues](../../issues)

### Common Issues

- **Settings Not Saving**: Check file permissions
- **Network Problems**: Verify port and firewall settings
- **Performance Issues**: Adjust memory and CPU settings
- **UI Problems**: Try different themes or reset layout

---

**Configuration Complete!** ⚙️

Your Envy installation is now configured for optimal performance.

---

**Last Updated:** January 15, 2026
