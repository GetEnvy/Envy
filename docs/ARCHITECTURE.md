# Architecture

## System Overview
Envy is a Windows desktop monolith with modular subsystems and plugin extension points.

```text
UI (MFC Wnd/Dlg/Page/Ctrl classes)
  ↕
Application Core (EnvyApp, settings, scheduler, lifecycle)
  ↕
Protocol Engines (BT, G2, ED2K, Kad, DC, HTTP/FTP)
  ↕
Storage/Metadata (Library, schemas, SQLite, cache)
  ↕
Services + Plugins (zlib, bzip2, miniupnp, BugTrap, plugin DLLs)
```

## Main Components
- **Application shell (`Envy/Envy.cpp`)**: startup, command-line options, global state, process control.
- **Networking/protocol subsystem**: per-network handlers and transfer state machines.
- **Library subsystem**: file indexing, metadata extraction, schema mapping, sharing rules.
- **Remote management surface (`Remote/`)**: HTML templates and JS assets for remote control UX.
- **Plugin host (`Plugins/`)**: media handlers, readers/builders, integration modules.

## Build Architecture
- **Primary:** Visual Studio solution (`Visual Studio/Envy.sln`) with many native projects.
- **Secondary:** top-level CMake for HashLib and selected tests; full migration is not complete.

## Key Design Constraints
1. Windows + MFC coupling is foundational.
2. Protocol compatibility requires conservative behavior changes.
3. In-tree vendored dependencies reduce external setup but increase maintenance burden.

## ADR Notes (Lightweight)
- **ADR-001 (historical):** Keep monorepo with in-tree dependencies for reproducible Windows builds.
- **ADR-002 (active):** Introduce modern tooling (CI, static checks, tests) without breaking existing release path.
- **ADR-003 (active):** Maintain Visual Studio as canonical build while gradually improving CMake coverage.
