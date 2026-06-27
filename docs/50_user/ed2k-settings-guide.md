# ED2K Settings Guide

## Accessing ED2K Settings

1. Open Envy
2. Go to **Tools** → **Settings**
3. Navigate to **Networks** → **eDonkey**
4. For advanced features: **Networks** → **eDonkey** → **Advanced**

## Basic ED2K Settings

### Network Configuration
- **Enable eDonkey**: Turn ED2K network on/off
- **Connect on Startup**: Automatically connect when Envy starts
- **Max Connections**: Maximum number of ED2K connections (50-500)
- **Server Reconnect**: How often to reconnect to servers (minutes)

### Server Settings
- **Auto Server List Update**: Automatically download new server lists
- **Min Server File Size**: Minimum file size for server sharing (MB)
- **Server Reask Time**: How often to re-query servers (minutes)

### Download Settings
- **Request Parts**: Number of simultaneous download requests (1-10)
- **Request Size**: Size of each download request (10-200 KB)
- **Reask Time**: How often to re-request failed parts (minutes)

## Advanced ED2K Features

### AICH (Advanced Integrity Check Hashing)

#### What is AICH?
AICH provides additional file verification beyond standard ED2K hashing, helping detect and recover from file corruption.

#### Settings:
- **Enable AICH**: ✅ Enable AICH for better file integrity
- **Trust Every Hash**: ⚠️ Accept all AICH hashes (faster but less secure)
- **Hash Set Timeout**: 30 seconds (timeout for hash requests)
- **AICH Recovery**: ✅ Attempt to recover corrupted files

#### Recommended Configuration:
```ini
Enable AICH = true
Trust Every Hash = false
Hash Set Timeout = 30
AICH Recovery = true
```

### Enhanced Kademlia DHT

#### What is Kademlia?
Kademlia is a distributed hash table that helps find other users and files on the ED2K network.

#### Settings:
- **Enable Kad Hello**: ✅ Enable advanced node identification
- **Kad Find Value**: ✅ Enable file searches in DHT
- **Kad Hello Timeout**: 10 seconds
- **Kad Find Value Timeout**: 15 seconds

#### Recommended Configuration:
```ini
Enable Kad Hello = true
Kad Find Value = true
Kad Hello Timeout = 10
Kad Find Value Timeout = 15
```

### Protocol Extensions

#### MultiPacket Features
- **Enable MultiPacket Ext2**: ✅ Efficient bulk transfers
- **Enable HashSetRequest2**: ✅ Improved hash requests

### Network Features

#### IPv6 Support
- **Prefer IPv6**: Use IPv6 when available
- **Enable Dual Stack**: Allow both IPv4 and IPv6
- **IPv6 Connect Timeout**: 30 seconds

#### UPnP (Universal Plug and Play)
- **Enable UPnP**: ✅ Automatic port forwarding
- **Delete Ports on Exit**: Clean up port mappings when closing

## Performance Tuning

### For Fast Connections (100+ Mbps)
```ini
Max Connections = 200
Request Parts = 5
Request Size = 150
Reask Time = 20
Enable AICH = true
Prefer IPv6 = true
```

### For Slow Connections (<10 Mbps)
```ini
Max Connections = 50
Request Parts = 2
Request Size = 50
Reask Time = 40
Enable AICH = false
Prefer IPv6 = false
```

### For Maximum Security
```ini
Enable AICH = true
Trust Every Hash = false
AICH Recovery = true
Enable Kad Hello = true
Enable MultiPacket Ext2 = true
```

## Troubleshooting

### Can't Connect to Network
1. Check **Enable eDonkey** is checked
2. Verify server list is current
3. Try different server in server list
4. Check firewall/antivirus blocking connections

### Slow Downloads
1. Increase **Max Connections**
2. Reduce **Reask Time**
3. Enable **Endgame** mode
4. Check if source limits are too low

### AICH Problems
1. Disable **Trust Every Hash** for security
2. Increase **Hash Set Timeout**
3. Enable **AICH Recovery**
4. Check available disk space

### Kademlia Issues
1. Enable **Enable Kad Hello**
2. Check firewall allows UDP port 4672
3. Wait for Kad network to bootstrap (can take time)
4. Try different bootstrap nodes

### IPv6 Issues
1. Disable **Prefer IPv6** if having problems
2. Check ISP supports IPv6
3. Verify firewall allows IPv6 traffic
4. Increase **IPv6 Connect Timeout**

## Advanced Configuration

### Custom Server Lists
```
Primary Server List URL: http://serverlist.com/list.php
Backup Server List URL: http://backup.com/list.php
```

### Bandwidth Limiting
```
Download Limit: 0 (unlimited) or set in KB/s
Upload Limit: 0 (unlimited) or set in KB/s
```

### Queue Management
```
Min. Queue Rating: 1 (accept low-rated sources)
Max. Queue Rating: 10 (prefer high-rated sources)
Queue Refresh Time: 10 minutes
```

## Command Line Options

Override settings via command line:
```
envy.exe /ed2k=true /kad=true /aich=true /ipv6=false
```

## Export/Import Settings

1. **Export**: Settings → Advanced → Export Settings
2. **Import**: Settings → Advanced → Import Settings
3. **Reset**: Settings → Advanced → Reset to Defaults

## Monitoring

### Network Status Indicators
- **Green**: Connected and active
- **Yellow**: Connecting or limited connectivity
- **Red**: Disconnected or error state

### Kad Status
- **Fire**: Kad network active and firewalled
- **Leaf**: Connected to Kad network
- **4**: Kad version 4+ (good)
- **?**: Kad initializing

## Support

If you encounter issues:
1. Check the [Envy FAQ](faq.md)
2. Visit the [Community Forums](forums.md)
3. Review the [Troubleshooting Guide](troubleshooting.md)
4. Enable debug logging for detailed error reports

---

*Settings are automatically saved when changed. Some changes require restart to take effect.*
