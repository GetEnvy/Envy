# Advanced ED2K Protocol Features

## Overview

Envy now supports advanced ED2K protocol features that enhance compatibility with the latest eMule implementations and provide improved file integrity and network performance.

## Features

### 1. Advanced Integrity Check Hashing (AICH)

**Purpose**: Provides additional file integrity verification beyond standard ED2K hashing.

**Benefits**:
- Detects file corruption with higher accuracy
- Enables recovery from partially corrupted downloads
- Compatible with eMule's AICH system

**Configuration**:
- **Enable AICH**: Enable/disable AICH functionality
- **Trust Every Hash**: Accept all received AICH hashes (less secure but faster)
- **Hash Set Timeout**: Timeout for AICH hash set requests (seconds)
- **AICH Recovery**: Attempt to recover from AICH verification failures

**Technical Details**:
- Uses 180KB chunks for hashing
- Merkle tree-based verification system
- Compatible with eMule AICH version 1 and 2

### 2. Enhanced Kademlia DHT

**Purpose**: Improved Distributed Hash Table for peer discovery and file location.

**New Features**:
- **KADEMLIA2_HELLO_REQ/RES**: Proper node identification protocol
- **KADEMLIA_FIND_VALUE**: Direct file search capability in DHT
- **Enhanced Timeouts**: Configurable timeout settings for better performance

**Configuration**:
- **Enable Kad Hello**: Enable advanced node identification
- **Kad Find Value**: Enable file searches in DHT
- **Kad Hello Timeout**: Timeout for hello requests (seconds)
- **Kad Find Value Timeout**: Timeout for find value requests (seconds)

**Benefits**:
- Faster peer discovery
- Improved file location accuracy
- Better network resilience

### 3. MultiPacket Extensions

**Purpose**: Efficient bulk data transfer using extended packet formats.

**Features**:
- **MultiPacket Ext2**: Send multiple file requests/responses in single packets
- **HashSetRequest2**: Improved hash set requests using FileIdentifier

**Benefits**:
- Reduced network overhead
- Faster bulk operations
- Better bandwidth utilization

### 4. IPv6 Network Support

**Purpose**: Extended network connectivity with IPv6 support.

**Features**:
- **Dual Stack Operation**: Simultaneous IPv4/IPv6 connectivity
- **IPv6 Address Resolution**: Native IPv6 hostname resolution
- **Configurable Timeouts**: IPv6-specific connection timeouts

**Configuration**:
- **Prefer IPv6**: Prefer IPv6 connections when available
- **Enable Dual Stack**: Allow both IPv4 and IPv6 connections
- **IPv6 Connect Timeout**: Connection timeout for IPv6 (seconds)

**Benefits**:
- Future-proof networking
- Access to IPv6-only networks
- Improved connectivity in modern networks

### 5. UPnP Port Forwarding

**Purpose**: Automatic port forwarding for better connectivity.

**Existing Features** (Enhanced):
- Automatic router detection
- Dynamic port mapping
- UPnP device compatibility

### 6. Modernized Search Functionality (January 2026)

**Purpose**: Complete overhaul of ED2K search functionality for improved reliability and compatibility.

**Features**:
- **UDP Search GUID Tracking**: Server-specific mapping ensures search results match correct searches
  - Tracks search GUID by server IP, UDP port, and opcode
  - Automatic cleanup of stale entries (60-second timeout)
  - Supports multiple concurrent searches without conflicts
- **Concatenated UDP Packet Support**: eMule-compatible handling of multiple sub-packets
  - Parses multiple search results in single UDP datagram
  - Correctly extracts Unicode flag from server flags (bit 8)
  - Handles compressed packets (ED2K_PROTOCOL_EMULE_PACKED)
- **Safe Packet Construction**: Null checks prevent crashes during packet creation
- **Vendor Cache Robustness**: Multiple fallback paths ensure Vendors.xml is always available
  - Primary: User DataPath
  - Fallback 1: Install directory
  - Fallback 2: Binary folder
  - Automatic file copying to user directory

**Benefits**:
- ✅ UDP search results correctly associated with originating search
- ✅ Support for eMule servers that concatenate multiple results
- ✅ No more "Unknown Vendor Code" log spam
- ✅ Improved reliability with safe packet construction
- ✅ Better developer experience with automatic file copying

## Performance Optimizations

### AICH Optimizations
- **Streaming Hash Calculation**: Reduced memory usage during hashing
- **Optimized Tree Building**: Faster hash tree construction
- **Memory Pool**: Reusable buffers for chunk processing

### Kademlia Optimizations
- **Binary Search**: O(log n) contact lookups instead of O(n)
- **Sorted Buckets**: Improved routing table organization
- **Memory Efficiency**: Reduced memory allocations

## Configuration

Access the advanced ED2K settings through:
1. **Tools** → **Settings**
2. **Networks** → **eDonkey** → **Advanced**

## Compatibility

- **eMule**: Full compatibility with eMule 0.50+ features
- **aMule**: Compatible with aMule's protocol extensions
- **Network**: Backward compatible with standard ED2K clients

## Troubleshooting

### AICH Issues
- **Hash Verification Fails**: Check "AICH Recovery" setting
- **Slow Performance**: Reduce "Trust Every Hash" or increase timeouts
- **Memory Usage**: AICH uses additional memory for hash trees

### Kademlia Issues
- **Poor Connectivity**: Check firewall settings and port forwarding
- **Slow Searches**: Verify Kad Hello and Find Value timeouts
- **High CPU Usage**: Reduce outstanding request limits

### Network Issues
- **IPv6 Problems**: Disable "Prefer IPv6" if connectivity issues
- **UPnP Failures**: Check router UPnP support and enable manually
- **Connection Timeouts**: Increase timeout values for slow networks

## Technical Specifications

### AICH Parameters
- **Chunk Size**: 180KB (180 * 1024 bytes)
- **Hash Algorithm**: MD4
- **Tree Structure**: Binary Merkle tree
- **Recovery**: Automatic chunk-level recovery

### Kademlia Parameters
- **Bucket Size**: 10 contacts (K=10)
- **ID Size**: 128 bits (16 bytes)
- **Buckets**: 128 total (one per bit position)
- **Timeouts**: Configurable per operation type

### Network Parameters
- **IPv6 Support**: RFC 4291 compliant
- **UPnP Support**: UPnP Internet Gateway Device v1.0
- **Dual Stack**: RFC 6555 Happy Eyeballs

## Version History

- **v4.1**: Initial advanced ED2K features implementation
- **v4.2**: AICH and Kademlia optimizations
- **v4.3**: IPv6 and MultiPacket support
- **v4.4**: Performance optimizations and UI improvements

---

*For additional support, visit the Envy project documentation or community forums.*
