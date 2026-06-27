# Decisions Log (Lightweight ADR)

This is a concise decision register. Use for major repo-level operating decisions.

| ID | Date | Decision | Rationale | Status |
|---|---|---|---|---|
| D-001 | 2026-05-15 | `develop` is the authoritative default branch; any `.github/settings.yml` entry still naming `master` is stale/not applied until synchronized. | Align active development and repository operations, and avoid governance/configuration contradictions. | Active |
| D-002 | 2026-05-15 | `Visual Studio/Envy.sln` is authoritative for full builds. | Legacy Windows/MFC project structure and release confidence requirements. | Active |
| D-003 | 2026-05-15 | MSVC `v145` is required for authoritative Windows builds. | Ensures deterministic build compatibility with project configuration. | Active |
| D-004 | 2026-05-15 | CMake remains partial and non-authoritative. | Current CMake coverage does not represent full runtime build graph. | Active |
| D-005 | 2026-05-15 | Protocol changes must preserve wire compatibility by default. | Interoperability across legacy P2P networks is critical. | Active |
| D-006 | 2026-05-15 | Modernization proceeds incrementally via small reviewable PRs. | Reduces regression risk in legacy coupled codebase. | Active |
