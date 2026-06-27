# 🚀 BitTorrent v2 (BEP-52) Implementation Plan

**Version:** 1.0 | **Last Updated:** January 16, 2026 | **Status:** PLANNING PHASE

---

## 📋 Overview

This document outlines the implementation plan for BitTorrent Protocol v2 (BEP-52), a major upgrade from the original BitTorrent protocol. BEP-52 introduces SHA-256 hashing, Merkle tree-based verification, improved metadata structure, and better support for large files.

## 🎯 Key Improvements in v2

### **Cryptographic Enhancements**
- **SHA-256 hashing** (vs SHA-1 in v1) for stronger security
- **Merkle tree verification** for efficient file integrity checking
- **32-byte infohashes** for unique torrent identification

### **Metadata Structure**
- **File tree hierarchy** instead of flat file lists
- **Piece layers** for efficient metadata distribution
- **Hybrid mode support** (v1 + v2 compatibility)

### **Protocol Features**
- **Enhanced peer handshake** with version negotiation
- **Hash request/response messages** for Merkle tree data
- **16 KiB minimum piece size** for better granularity

---

## 📅 Implementation Timeline

### **Phase 1: Foundation (Feb 2026)** - 4 weeks
**Focus:** Core infrastructure and basic functionality

#### **Week 1: SHA-256 Integration** ✅ COMPLETED
- [x] Implement SHA-256 hashing functions
- [x] Create CSHA256 class (similar to existing hash classes)
- [x] Add SHA-256 to HashLib integration
- [x] Unit tests for SHA-256 operations
- [x] Known vector validation (empty string, "abc", longer message)
- [x] Incremental hashing and state management tests

#### **Week 2: Merkle Tree Implementation**
- [ ] Implement Merkle tree construction (16 KiB blocks)
- [ ] Create MerkleTree class for file verification
- [ ] Piece layer generation and validation
- [ ] Tree balancing and padding logic

#### **Week 3: Basic Metadata Parser**
- [ ] Parse v2 .torrent files (info dictionary)
- [ ] File tree structure handling
- [ ] Pieces root and piece layers processing
- [ ] Basic validation of v2 metadata

#### **Week 4: Infohash Generation**
- [ ] 32-byte SHA-256 infohash computation
- [ ] Truncated 20-byte infohash for compatibility
- [ ] Infohash validation and comparison functions

### **Phase 2: Protocol Core (Mar 2026)** - 4 weeks
**Focus:** Peer wire protocol and networking

#### **Week 5-6: Peer Handshake**
- [ ] Extended handshake message format (32-byte infohash)
- [ ] Reserved bits for version negotiation
- [ ] Hybrid mode handshake (v1 + v2 support)
- [ ] Peer capability detection

#### **Week 7-8: Hash Exchange Messages**
- [ ] `hash request` message (ID 21)
- [ ] `hashes` response message (ID 22)
- [ ] `hash reject` message (ID 23)
- [ ] Merkle tree data transmission

### **Phase 3: File Handling (Apr 2026)** - 3 weeks
**Focus:** File verification and downloading

#### **Week 9: File Verification**
- [ ] Merkle tree verification during download
- [ ] Piece hash validation using tree structure
- [ ] Efficient hash tree traversal
- [ ] Corruption detection and recovery

#### **Week 10: Multi-File Support**
- [ ] File tree alignment to piece boundaries
- [ ] Hierarchical file structure handling
- [ ] Padding file generation (BEP-47 compatibility)
- [ ] Large file support optimization

#### **Week 11: Hybrid Mode**
- [ ] Simultaneous v1/v2 operation
- [ ] Fallback mechanisms
- [ ] Compatibility layer development

### **Phase 4: Integration & Testing (May 2026)** - 3 weeks
**Focus:** System integration and validation

#### **Week 12: Client Integration**
- [ ] Integration with existing BitTorrent client code
- [ ] DHT integration for v2 torrents
- [ ] Magnet link support for v2
- [ ] User interface updates

#### **Week 13: Compatibility Testing**
- [ ] Interoperability with v1 clients
- [ ] Hybrid torrent handling
- [ ] Performance benchmarking
- [ ] Memory usage optimization

#### **Week 14: Final Validation**
- [ ] End-to-end testing
- [ ] Protocol compliance verification
- [ ] Documentation completion
- [ ] Production readiness assessment

---

## 🏗️ Technical Architecture

### **Core Components**

#### **1. Hashing System**
```cpp
class CSHA256Hash {
    // SHA-256 hash computation
    // Similar to existing hash classes
};

class CMerkleTree {
    // Merkle tree construction and verification
    // 16 KiB block processing
    // Tree balancing and traversal
};
```

#### **2. Metadata Parser**
```cpp
class CBTTorrent_v2 {
    // Parse v2 .torrent files
    // File tree structure
    // Piece layers handling
    // Infohash generation
};
```

#### **3. Peer Protocol**
```cpp
class CBTClient_v2 {
    // Extended handshake
    // Hash request/response handling
    // Merkle tree data exchange
    // Hybrid mode support
};
```

### **File Structure Changes**

```
Envy/BitTorrent/
├── BTClient_v2.h/.cpp        # v2 client implementation
├── BTTorrent_v2.h/.cpp       # v2 metadata parser
├── MerkleTree.h/.cpp         # Merkle tree utilities
├── SHA256Hash.h/.cpp         # SHA-256 hash class
└── HybridSupport.h/.cpp      # v1/v2 compatibility layer
```

---

## 🔧 Implementation Details

### **SHA-256 Integration**

**Requirements:**
- Windows CryptoAPI support for SHA-256
- Consistent API with existing hash classes
- Performance optimization for large files

**Implementation:**
```cpp
// Use Windows CNG or CryptoAPI for SHA-256
// Fallback to alternative implementations if needed
// Maintain compatibility with existing hash interfaces
```

### **Merkle Tree Construction**

**Algorithm:**
1. Divide file into 16 KiB blocks
2. Hash each block with SHA-256
3. Build binary tree by hashing pairs
4. Continue until single root hash
5. Balance tree with padding if needed

**Optimization:**
- Lazy evaluation (build only requested branches)
- Memory-efficient tree representation
- Piece layer caching for faster access

### **Metadata Format Handling**

**Key Fields:**
- `name`: Display name
- `piece length`: Power of 2, ≥16 KiB
- `file tree`: Hierarchical file structure
- `pieces root`: Merkle tree root per file
- `piece layers`: Optimization for large files

**Validation:**
- Strict UTF-8 encoding
- Sorted dictionary keys
- No leading zeros in bencoding
- Piece boundary alignment

### **Peer Wire Protocol**

**Handshake Extension:**
```
<reserved bytes> + <infohash_v1?> + <infohash_v2>
```

**New Messages:**
- Hash Request: `<base layer><index><length><proof layers>`
- Hashes: `<hashes>` (concatenated hash data)
- Hash Reject: Empty response (rate limiting)

---

## 🧪 Testing Strategy

### **Unit Testing**
- SHA-256 hash correctness
- Merkle tree construction accuracy
- Metadata parsing validation
- Protocol message formatting

### **Integration Testing**
- End-to-end file transfers
- Peer handshake compatibility
- Hash verification during download
- Hybrid mode operation

### **Compatibility Testing**
- v1 client interoperability
- Hybrid torrent handling
- Large file performance
- Network edge cases

---

## 📊 Performance Considerations

### **Memory Usage**
- Merkle trees for large files (GB+) need efficient representation
- Piece layer caching to reduce memory footprint
- Lazy tree construction to minimize initial overhead

### **Network Efficiency**
- Hash request batching to reduce round trips
- Incremental verification to avoid full tree downloads
- Connection pooling for hash data exchange

### **CPU Utilization**
- SHA-256 is more expensive than SHA-1
- Optimize tree construction algorithms
- Parallel processing for multi-core systems

---

## 🔄 Backward Compatibility

### **Hybrid Mode Strategy**
1. Generate both v1 and v2 metadata
2. Support both infohash types in handshake
3. Fallback to v1 for incompatible peers
4. Seamless upgrade path for existing torrents

### **Migration Path**
- Gradual rollout with hybrid support
- v2-only torrents for new content
- Legacy support for existing torrents
- User preference for protocol version

---

## 📋 Success Criteria

### **Functional Requirements**
- ✅ Parse and validate v2 .torrent files
- ✅ Generate correct 32-byte infohashes
- ✅ Build and verify Merkle trees
- ✅ Handle peer handshake negotiation
- ✅ Exchange hash data with peers
- ✅ Verify downloads using tree structure

### **Compatibility Requirements**
- ✅ Work with existing v1 clients
- ✅ Support hybrid torrent format
- ✅ Maintain v1 performance levels
- ✅ Graceful degradation on errors

### **Performance Requirements**
- ✅ SHA-256 operations within 2x of SHA-1
- ✅ Memory usage scales with file size
- ✅ Network overhead acceptable (<10%)
- ✅ Tree construction time reasonable

---

## 🚨 Risk Assessment

### **High Risk Items**

#### **SHA-256 Performance Impact**
- **Impact:** Slower hashing affects overall performance
- **Mitigation:** Optimize implementation, hardware acceleration
- **Contingency:** Fallback to SHA-1 for performance-critical paths

#### **Memory Usage for Large Files**
- **Impact:** Merkle trees consume significant RAM
- **Mitigation:** Lazy loading, efficient data structures
- **Contingency:** Limit tree depth for very large files

#### **Protocol Compatibility**
- **Impact:** v2 clients may not connect to v1-only swarms
- **Mitigation:** Robust hybrid mode implementation
- **Contingency:** Force v1 fallback when v2 fails

---

## 👥 Team Assignments

### **Core Development Team**
- **Lead:** David Kim (Protocol Team)
- **Members:** Alex Chen, Maria Gonzalez
- **Support:** Sarah Johnson (Build/Performance)

### **Responsibilities**
- **David Kim:** Merkle tree implementation, protocol messages
- **Alex Chen:** Metadata parsing, infohash generation
- **Maria Gonzalez:** Peer handshake, network integration
- **Sarah Johnson:** Performance optimization, memory management

---

## 📈 Milestones & Deliverables

### **Phase 1 Milestones (End Feb 2026)**
- [ ] SHA-256 hashing implementation complete
- [ ] Basic Merkle tree construction working
- [ ] v2 metadata parsing functional
- [ ] Infohash generation validated

### **Phase 2 Milestones (End Mar 2026)**
- [ ] Peer handshake protocol implemented
- [ ] Hash exchange messages working
- [ ] Basic file verification operational
- [ ] Unit tests passing (80% coverage)

### **Phase 3 Milestones (End Apr 2026)**
- [ ] Multi-file support complete
- [ ] Hybrid mode functional
- [ ] Performance benchmarks met
- [ ] Integration tests passing

### **Phase 4 Milestones (End May 2026)**
- [ ] Full client integration complete
- [ ] Compatibility testing finished
- [ ] Documentation updated
- [ ] Production ready for testing

---

## 🔗 Related Documentation

- **[BEP-52 Specification](https://wiki.theory.org/BitTorrentSpecification#BEP-52)** - Official v2 protocol spec
- **[BEP-47](https://wiki.theory.org/BitTorrentSpecification#BEP-47)** - Padding file spec for alignment
- **[docs/DEV_TRACKER.md](../../DEV_TRACKER.md)** - Overall project progress
- **[Architecture](../../20_arch/architecture.md)** - System design

---

## 📞 Dependencies & Prerequisites

### **Required Before Starting**
- [x] Kademlia DHT implementation (for peer discovery)
- [x] CryptoProvider (for SHA-256 operations)
- [x] Existing BitTorrent v1 client code
- [ ] Windows CryptoAPI SHA-256 support verification

### **External Dependencies**
- Windows CNG/CryptoAPI for SHA-256
- Existing hash library integration
- BitTorrent protocol framework
- Network infrastructure

---

**Last Updated:** January 16, 2026
**Planning Complete:** January 2026
**Development Start:** February 2026
**Expected Completion:** May 2026
**Document Owner:** BitTorrent v2 Development Team
