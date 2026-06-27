# Claude Code - Envy repository

Authoritative rules for AI assistants live in [`AGENTS.md`](./AGENTS.md).
Read it first; everything below is just shortcuts.

## Quick context

- **Stack**: C++ MFC, MSBuild, Visual Studio 2026 (toolset **v145**).
- **C++ standard**: C++20 for first-party, C++17 for legacy plugins.
- **OS target**: Windows 10 1809+ (XP/Vista/7/8 dropped).
- **Deps**: vcpkg manifest (`vcpkg.json`).
- **CI**: GitHub Actions on `windows-2025` (`.github/workflows/`).

## Build command (copy-paste)

```cmd
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64 ^
  /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0 ^
  /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x64-windows-static
```

## Mandatory steps when you act on this repo

1. **Read** [`docs/DEVELOPMENT_PLAN.md`](./docs/DEVELOPMENT_PLAN.md) for strategic scope.
2. **Record progress** in `docs/DEVELOPMENT_PLAN.md` (strategic) and
   `.local/DEV_TRACKER.md` (session notes, gitignored).
3. **Branch & merge gate.** Branch off `develop` using conventional
   `type/short-kebab-summary` names (`feat/`, `fix/`, `docs/`, `ci/`, ...);
   never use tool- or agent-prefixed branches. You may push your own branch
   and open **draft** PRs, but never mark ready-for-review, enable
   auto-merge, or merge without explicit maintainer approval.
   (See AGENTS.md section 2, rules 11-12.)
4. **Reply to the user in the language they used in chat**, but all
   commits, comments, docs, and PR text in **English**.

## Common pitfalls in this codebase

- `throw()` is removed in C++20 - use `noexcept`.
- `register` is no longer a storage class - drop it.
- Source files mix ISO-8859 and UTF-8 - never bulk-convert; respect the
  BOM you find.
- `Plugins/PluginWizard/**` are project templates - do **not** retarget
  their `.vcxproj` files.
- `_CRT_SECURE_NO_WARNINGS` is set repo-wide; do not rely on it as
  permission to use unsafe APIs in new code.

## See also

- [`MODERNIZATION.md`](./MODERNIZATION.md) - full plan and phases.
- [`AGENTS.md`](./AGENTS.md) - canonical rules.
- [`docs/DEVELOPMENT_PLAN.md`](./docs/DEVELOPMENT_PLAN.md) - strategic plan.
- [`.github/CONTRIBUTING.md`](./.github/CONTRIBUTING.md) - human-facing.
