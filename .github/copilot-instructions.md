# GitHub Copilot instructions for the Envy repository

Read `AGENTS.md` for the full ruleset. Highlights:

## Project context

Envy is a Windows MFC peer-to-peer client (Gnutella, eDonkey, BitTorrent,
DC++). The codebase is ~677 .cpp / 709 .h files across 46 MSBuild
projects (`Visual Studio/Envy.sln`).

## Toolchain (mandatory)

- **Visual Studio 2026**, PlatformToolset **v145**, MSVC 14.50.
- **C++20** for first-party code (Envy, HashLib, TorrentEnvy, ...).
- **C++17** for legacy plugins.
- **Windows 10 1809+** target. XP/Vista/7/8 are explicitly unsupported.
- Dependencies via **vcpkg manifest** (`vcpkg.json`).

## Build command

```cmd
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64 ^
  /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0 ^
  /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x64-windows-static
```

## Style guidance

- Indentation: tabs, width 4.
- Braces: Allman (own line).
- Naming: Hungarian-ish (`m_`, `p`, `n`, `b`, `str`, `dw`, `lp`).
- Inside MFC code, prefer `CString`, `CAtlList`, `CMap`, `CCriticalSection`
  over their `std::` equivalents.
- Always `#include "StdAfx.h"` first in every `.cpp`.
- Use `noexcept`, never `throw()` (removed in C++20).
- Never use the `register` keyword (removed in C++17).

## Don't

- Don't reintroduce `_ATL_XP_TARGETING`, `v141_xp`, `XPSUPPORT`, or
  `_WIN32_WINNT < 0x0A00`.
- Don't add `#pragma warning(disable: ...)` to silence new warnings -
  fix the warning instead.
- Don't bulk-reformat files; `.clang-format` is advisory.
- Don't edit anything under `Plugins/PluginWizard/**` - those are
  project templates VS uses to scaffold new plugins.
- Don't hand-edit vendored third-party trees in `Services/`,
  `Plugins/{RatDVDPlugin,SWFPlugin}`. Replace via vcpkg instead.
- Don't push to `main`, `develop`, or `legacy` directly.
- Don't write commits that include AI marketing tags or co-authoring
  trailers unless the user asked for them.

## Living workflow

Every task starts with reading `docs/DEV_TRACKER.md` and ends with updating
it. Use the `In progress` / `Done` sections at the top of the file.

## Languages

- All artifacts (code, comments, commits, PRs, docs, workflow names):
  **English**.
- Chat replies: follow the user's language.

## See also

- `AGENTS.md` - canonical rules
- `MODERNIZATION.md` - multi-phase plan
- `docs/DEV_TRACKER.md` - living progress log
- `.github/CONTRIBUTING.md` - human-facing contributor guide
