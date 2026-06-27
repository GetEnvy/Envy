# Envy Project Roadmap

This document outlines the planned improvements and features for the Envy P2P client.

## Current Focus: Modernization (Q4 2024)

### ✅ Completed
- [x] Upgrade to C++20 standard across all projects
- [x] Add GitHub Actions CI/CD workflows
- [x] Implement CodeQL security scanning
- [x] Create comprehensive documentation
- [x] Add AI coding assistant configurations (Copilot, Cursor)
- [x] Setup code formatting standards (EditorConfig, clang-format)
- [x] Add issue and PR templates
- [x] Create contribution guidelines
- [x] Setup automated dependency updates (Dependabot)
- [x] Add security policy
- [x] Configure Visual Studio Code for optimal development

### 🔄 In Progress
- [ ] Refactor legacy code to use modern C++ features
- [ ] Improve unit test coverage
- [ ] Update bundled libraries to latest versions
- [ ] Performance profiling and optimization

## Short Term Goals (Q1 2025)

### Code Quality & Modernization
- [ ] Replace raw pointers with smart pointers where appropriate
- [ ] Implement RAII patterns throughout codebase
- [ ] Add `override` and `final` keywords to virtual functions
- [ ] Convert appropriate loops to range-based for loops
- [ ] Use `constexpr` for compile-time constants
- [ ] Add more comprehensive error handling

### Developer Experience
- [ ] Create developer onboarding guide
- [ ] Add more code examples in documentation
- [ ] Improve debugging capabilities
- [ ] Add performance profiling guides
- [ ] Create architecture diagrams

### Build & CI/CD
- [ ] Optimize build times
- [ ] Add more comprehensive automated tests
- [ ] Implement automated release notes generation
- [ ] Add code coverage reporting
- [ ] Setup performance regression testing

## Medium Term Goals (Q2-Q3 2025)

### Features & Enhancements
- [ ] Improve BitTorrent DHT implementation
- [ ] Enhanced magnet link support
- [ ] Better support for large files (>4GB)
- [ ] Improved bandwidth management
- [ ] Better integration with modern Windows features

### UI/UX Improvements
- [ ] Modernize UI with better high DPI support
- [ ] Improve dark mode support
- [ ] Add more customization options
- [ ] Better notification system
- [ ] Improved accessibility

### Network & Protocol
- [ ] Implement uTP (micro Transport Protocol)
- [x] IPv6 support - Foundation implemented (Q1 2025)
  - [x] CAddress class for dual-stack abstraction
  - [x] IPv6 socket support in CNetwork
  - [x] IPv6 TCP connections in CConnection
  - [x] IPv6 UDP datagrams foundation in CDatagrams
  - [x] Configuration settings (EnableIPv6, PreferIPv6)
  - [ ] IPv6 multicast support
  - [ ] UPnP IPv6 support
  - [ ] UI updates for IPv6 display
  - [ ] Happy Eyeballs implementation
- [ ] Better NAT traversal
- [ ] Enhanced encryption options
- [ ] WebRTC for peer discovery

### Security
- [ ] Enhanced malware detection integration
- [ ] Improved sandboxing for plugins
- [ ] Better content verification
- [ ] Enhanced privacy features
- [ ] Security audit and hardening

## Long Term Goals (2026+)

### Architecture
- [ ] Plugin API modernization
- [ ] Component modularization
- [ ] Improved resource management
- [ ] Better separation of concerns
- [ ] Consider cross-platform support (long-term consideration)

### Advanced Features
- [ ] Machine learning for intelligent peer selection
- [ ] Advanced content filtering
- [ ] Distributed hash table improvements
- [ ] Better streaming support
- [ ] Cloud storage integration options

### Performance
- [ ] Multi-threaded optimizations using C++20 features
- [ ] Memory usage optimizations
- [ ] Startup time improvements
- [ ] Reduced idle CPU usage
- [ ] Better SSD optimization

### Community & Ecosystem
- [ ] Plugin marketplace
- [ ] Better documentation portal
- [ ] Video tutorials
- [ ] Community themes and skins
- [ ] Translation management system

## Technology Debt

### High Priority
- [ ] Remove deprecated Win32 API usage
- [ ] Update to latest Windows SDK best practices
- [ ] Modernize COM usage patterns
- [ ] Replace unsafe string operations
- [ ] Fix compiler warnings in Wall mode

### Medium Priority
- [ ] Refactor large functions (>100 lines)
- [ ] Reduce cyclomatic complexity in hot paths
- [ ] Improve code organization and modularity
- [ ] Better naming conventions consistency
- [ ] Remove obsolete compatibility code

### Low Priority
- [ ] Code style consistency improvements
- [ ] Comment and documentation improvements
- [ ] Dead code removal
- [ ] Simplify complex class hierarchies

## Research & Exploration

### Technologies to Evaluate
- [ ] WebRTC for peer connections
- [ ] QUIC protocol for transfers
- [ ] Blockchain for distributed hash table
- [ ] AI/ML for intelligent download optimization
- [ ] Modern UI frameworks (keep MFC or migrate?)

### Standards & Protocols
- [ ] Latest BitTorrent Enhancement Proposals (BEPs)
- [ ] Modern P2P protocols
- [ ] New compression algorithms
- [ ] Better hash algorithms (SHA-256, BLAKE3)
- [ ] Content addressing standards

## Community Priorities

These items are driven by community feedback and contributions:

### Most Requested Features
1. Better automation and scheduling
2. Improved RSS feed support
3. Better remote control capabilities
4. More detailed statistics
5. Better integration with media players

### Most Reported Issues
1. Connection stability on some ISPs
2. Performance with very large libraries
3. UI freezing on some operations
4. High memory usage with many torrents
5. Better handling of corrupted downloads

## Contributing to the Roadmap

We welcome community input on our roadmap:

### How to Contribute
1. **Suggest Features**: Open a feature request issue
2. **Vote on Items**: React to existing issues with 👍
3. **Discuss Priorities**: Participate in discussions
4. **Implement Items**: Pick an item and contribute code
5. **Sponsor Development**: Support specific features

### Priority Criteria
We prioritize items based on:
- User impact and benefit
- Security implications
- Maintenance burden
- Implementation complexity
- Community interest
- Available resources

## Release Schedule

### Versioning
We follow [Semantic Versioning](https://semver.org/):
- **Major** (X.0.0): Breaking changes, major new features
- **Minor** (0.X.0): New features, backward compatible
- **Patch** (0.0.X): Bug fixes, security patches

### Planned Releases
- **Quarterly**: Minor releases with new features
- **Monthly**: Patch releases with bug fixes
- **As Needed**: Security patches
- **Annually**: Major releases with significant changes

## Resources & Dependencies

### Required Resources
- Development time from contributors
- CI/CD infrastructure (GitHub Actions)
- Testing infrastructure
- Documentation time
- Code review capacity

### External Dependencies
- Windows SDK updates
- Compiler toolchain updates
- Third-party library updates
- Protocol specifications
- Community feedback

## Success Metrics

We measure success through:
- Build success rate
- Test coverage percentage
- Security vulnerability count
- Code quality metrics
- Community engagement
- User satisfaction
- Download/usage statistics

## Notes

- This roadmap is a living document and will be updated regularly
- Items may be added, removed, or reprioritized based on feedback
- Actual implementation may differ from planned features
- Community contributions can accelerate any item
- No guarantees on specific timelines
- Security issues always take priority

## Questions?

For questions about the roadmap:
- Open a [Discussion](https://github.com/Mika3578/Envy/discussions)
- Comment on specific issues
- Contact maintainers

---

**Last Updated**: 2024-11-05
**Next Review**: 2025-01-01

This roadmap reflects our current plans and may change based on community needs and available resources.
