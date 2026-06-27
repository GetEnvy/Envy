# System Architecture

This document provides an overview of the Envy P2P client's system architecture and design principles.

## 🏛️ Overview

Envy is a multi-network peer-to-peer file sharing application supporting BitTorrent, Gnutella2 (G2), Gnutella, and eDonkey2000 protocols. It uses C++17 with MFC (Microsoft Foundation Classes) for the Windows desktop environment.

## 🧩 Core Components

### Network Layer

**Purpose**: Handles all network communications and protocol implementations.

**Key Classes**:
- `CNetwork`: Core network management
- `CConnection`: Individual TCP connections
- `CDatagrams`: UDP datagram handling
- `CHostCache`: Peer discovery and caching

**Protocols Supported**:
- **BitTorrent**: DHT, magnet links, μTP
- **Gnutella2**: Advanced query routing, ultra-peers
- **eDonkey2000**: Kad network, file sources
- **HTTP/FTP**: Direct downloads

### Library System

**Purpose**: File management, metadata handling, and content organization.

**Key Classes**:
- `CLibrary`: Main library interface
- `CLibraryFile`: Individual file representation
- `CLibraryFolder`: Directory management
- `CSchema`: Metadata schema handling

**Features**:
- File hashing and verification
- Metadata extraction
- Search indexing
- Content organization

### Download Manager

**Purpose**: Manages file downloads from multiple sources.

**Key Classes**:
- `CDownload`: Download task management
- `CDownloadSource`: Individual download sources
- `CDownloadTransfer`: Active transfer handling
- `CHashChecker`: File integrity verification

**Capabilities**:
- Multi-source downloads
- Bandwidth management
- Resume support
- Hash verification

### User Interface

**Purpose**: Windows desktop interface with extensive customization.

**Key Classes**:
- `CMainWnd`: Main application window
- `CWndSearchMonitor`: Real-time search activity monitoring with advanced filtering
- `CWnd`: Various dialog and control classes
- `CSkin`: UI theming system

**Framework**: Microsoft Foundation Classes (MFC)

### Plugin System

**Purpose**: Extensible architecture for additional functionality.

**Architecture**: COM-based plugin system

**Plugin Types**:
- Document readers (PDF, DOC, etc.)
- Image services
- Media libraries
- Network extensions

## 🔄 Data Flow

### File Download Process

1. **Discovery**: File located via search or magnet link
2. **Source Acquisition**: Peer sources gathered from networks
3. **Download Initiation**: CDownload object created
4. **Transfer Management**: Sources coordinated for optimal speed
5. **Verification**: Hash checking during and after download
6. **Completion**: File moved to library, metadata extracted

### Network Communication

1. **Connection Establishment**: TCP/UDP connections to peers
2. **Protocol Handshake**: Protocol-specific negotiation
3. **Data Exchange**: File chunks, metadata, peer information
4. **Connection Management**: Keep-alive, timeouts, reconnection
5. **Resource Cleanup**: Proper connection teardown

## 🗂️ Data Storage

### SQLite Database

**Purpose**: Persistent storage for application data.

**Key Tables**:
- `Files`: Library file metadata
- `Downloads`: Active download state
- `Sources`: Peer source information
- `Searches`: Search history and results

### Configuration Files

**Locations**:
- `DefaultSettings.dat`: Default configuration
- `Security.dat`: Security settings and filters
- `GeoIP.dat`: Geographic IP data

## 🔒 Security Architecture

### Content Filtering

- **IP Filtering**: Block unwanted peer connections
- **Content Blocking**: Filter based on file types/metadata
- **Spam Prevention**: Automated spam detection

### Network Security

- **Protocol Validation**: Strict adherence to protocol specifications
- **Input Sanitization**: Validation of all network input
- **Resource Limits**: Prevent DoS through resource exhaustion

### Code Security

- **Memory Safety**: Smart pointers, bounds checking
- **Thread Safety**: Proper synchronization primitives
- **Error Handling**: Robust error recovery

## ⚡ Performance Considerations

### Memory Management

- **Object Pooling**: Reuse of frequently allocated objects
- **Lazy Loading**: Defer loading of non-essential data
- **Smart Pointers**: Automatic memory management

### Network Optimization

- **Connection Pooling**: Reuse of network connections
- **Bandwidth Throttling**: Configurable transfer limits
- **Parallel Downloads**: Multiple simultaneous transfers

### UI Responsiveness

- **Background Processing**: Non-blocking operations
- **Progress Updates**: Asynchronous UI updates
- **Resource Prioritization**: Critical operations prioritized

## 🔧 Extensibility

### Plugin Architecture

**COM Interface**: Standard Windows component model

**Extension Points**:
- File type handlers
- Network protocol extensions
- UI customizations
- Search providers

### Configuration System

**XML-based**: Skin and configuration files

**Hot Reloading**: Many changes apply without restart

## 📊 Monitoring and Diagnostics

### Logging System

- **Multiple Levels**: Error, Warning, Info, Debug
- **Categorized**: Network, Library, UI, Security
- **Performance**: Minimal overhead in release builds

### Performance Metrics

- **Transfer Statistics**: Speed, efficiency, success rates
- **Network Health**: Connection success, peer quality
- **System Resources**: Memory usage, CPU utilization

## 🚀 Future Architecture

### Planned Improvements

- **Modern UI Framework**: Migration from MFC
- **Cross-platform Support**: Linux/macOS compatibility
- **Advanced Networking**: WebRTC, QUIC protocols
- **Cloud Integration**: Distributed storage options

### Technical Debt

- **Legacy Code**: MFC modernization
- **Protocol Updates**: Latest specification compliance
- **Testing Coverage**: Comprehensive automated testing

## Related

- [Guide](../10_dev/guide.md) · [Build](../10_dev/build.md) · [Status](../10_dev/status.md) · [Standards](../10_dev/standards.md)

---

**Last Updated:** January 2026
