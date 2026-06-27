---
name: Envy repository rules
description: Canonical rules for AI assistants operating on the Envy codebase.
alwaysApply: true
---

# Envy - AI rules

Full ruleset: `AGENTS.md`. This file is a pointer.

- **Toolchain**: VS 2026, toolset v145, MSVC 14.50.
- **C++**: C++20 first-party, C++17 legacy plugins.
- **OS target**: Windows 10 1809+ (no XP support).
- **Dependencies**: vcpkg manifest (`vcpkg.json`).
- **Style**: tabs/4, Allman braces, Hungarian-ish naming, CString/CAtlList
  over std::.
- **Language**: English for code, comments, commits, PRs, docs; chat
  follows the user.
- **Tracking**: record strategic progress in `docs/DEVELOPMENT_PLAN.md`;
  session notes go in `.local/DEV_TRACKER.md` (gitignored).
- **Branch discipline**: branch off develop; push only your feature branch.
  Use type/short-kebab-summary names; no tool/agent prefixes.

Build command:

```
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release ^
  /p:Platform=x64 /p:PlatformToolset=v145 ^
  /p:WindowsTargetPlatformVersion=10.0 ^
  /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x64-windows-static
```

Read `MODERNIZATION.md` for the multi-phase plan.
