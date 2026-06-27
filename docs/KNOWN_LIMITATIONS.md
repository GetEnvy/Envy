# Known Limitations

This document tracks important current constraints that affect development and validation.

- **Toolchain runner dependency:** Authoritative `v145` parity is not guaranteed in every hosted CI context.
- **Incomplete CMake:** Top-level CMake does not yet model the full authoritative application build.
- **Limited protocol parser tests:** Automated coverage for malformed/edge packet paths is still shallow.
- **Legacy UI/core coupling:** MFC and core protocol logic remain tightly coupled in key paths.
- **Partial IPv6 support:** IPv6 capability is not fully integrated end-to-end.
- **CI limitations:** CI signals are useful but not a complete substitute for full local/VS validation.
- **Protocol gaps:** Some advanced ED2K/Kad/BitTorrent features remain partial or planned.
