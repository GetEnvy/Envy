# Development environment (authoritative)

**Last updated:** September 2026

This document is the single source of truth for how to set up a Windows
development machine for Envy, which repository files define editor/build
policy, and what must stay local. Agent workflow rules live in [`AGENTS.md`](../../AGENTS.md); build commands are detailed in [build.md](build.md).

## Configuration hierarchy

| Layer | Owns |
| --- | --- |
| [`AGENTS.md`](../../AGENTS.md) | Mandatory assistant/contributor workflow |
| [`.editorconfig`](../../.editorconfig) | Portable editor conventions per file type |
| [`.gitattributes`](../../.gitattributes) | Git binary/text rules (minimal; no legacy C++ charset) |
| [`.clang-format`](../../.clang-format) / CI | Mechanical C++ formatting on changed hunks |
| [`.vscode/settings.json`](../../.vscode/settings.json) | Small portable VS Code/Cursor workspace safety defaults |
| [`.vscode/extensions.json`](../../.vscode/extensions.json) | Recommended extensions only |
| This document | Human setup, encoding warnings, troubleshooting |
| [`tools/dev/`](../../tools/dev/) | Windows bootstrap and `compile_commands.json` generation |
| GitHub Actions | Enforcement (format, encoding guard, security scans) |

Do not commit machine-specific paths, generated compilation databases, or personal UI/AI settings.

## Fresh clone (Windows)

```powershell
git clone https://github.com/Mika3578/Envy.git
cd Envy
.\tools\dev\setup-windows.ps1 -Check
```

Restore vcpkg **only for the platforms you build**:

- **Win32** (Crashpad): `scripts\bootstrap-vcpkg.cmd -Triplet x86-windows-static`
- **x64** optional manifest deps: `scripts\bootstrap-vcpkg.cmd` (default `x64-windows-static`; not required before the first x64 build when `ThirdParty\BugSplat` is present — see [build.md](build.md))
- **Both** platforms: `scripts\bootstrap-vcpkg.cmd -All`

Open `Visual Studio\Envy.sln` in **Visual Studio 2026** (toolset **v145**, Windows 10 SDK **10.0**). Build **Release \| x64** (or Debug) at least once before generating clangd data.

Authoritative MSBuild command (see [build.md](build.md)):

```cmd
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64 ^
  /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0
```

Do **not** pass `/p:VcpkgEnableManifest` or `/p:VcpkgTriplet` for **x64** builds: `Envy/Envy.vcxproj` disables vcpkg manifest on x64 and command-line `/p:` overrides would trigger an unnecessary manifest restore. Win32 builds still use vcpkg (see [build.md](build.md)).

## Visual Studio

- **Build and debug:** Visual Studio remains the authoritative MSVC environment.
- **Do not commit:** `.vs/`, `*.user`, `*.suo`, per-user options.
- **vcpkg:** `vcpkg_installed/` is gitignored; restore via `scripts\bootstrap-vcpkg.cmd`.

## Cursor / VS Code

The repository versions a **minimal** `.vscode/` policy:

- `files.autoSave`: off (avoid silent saves on legacy encodings)
- `files.autoGuessEncoding`: true with `files.candidateGuessEncodings` limited to `utf8` / `windows1252` — legacy single-byte sources (CP1252 header bytes) usually round-trip byte-preserved instead of being saved back with `U+FFFD`. Detection is heuristic: on failure VS Code falls back to `files.encoding`, so **confirm the detected encoding in the status bar before saving a legacy file** (see encoding below)
- `editor.formatOnSave`: false (formatting is clang-format / CI on changed hunks)
- `editorconfig.enable`: true (with a safe `.editorconfig`; see encoding below)

Recommended extensions (see `.vscode/extensions.json`):

- **EditorConfig** — applies repository editor rules
- **clangd** — C++ navigation when `compile_commands.json` exists
- **YAML**, **GitHub Actions**, and **Actionlint** — workflow editing,
  schema support, and local workflow linting
- **GitHub Pull Requests** — repository review workflow when GitHub access is available
- **PowerShell** — repository build and validation scripts
- **XML** — MSBuild and project-file editing
- **Task Buttons** — status-bar buttons for the versioned build tasks (`.vscode/tasks.json`)
- **Inno Setup** — installer-script editing (use the maintained `chouzz.vscode-innosetup` fork; `idleberg.innosetup` is discontinued)

### Build tasks from Cursor

`.vscode/tasks.json` is **versioned** and exposes two portable MSBuild tasks
driven by [`tools/dev/run-msbuild.ps1`](../../tools/dev/run-msbuild.ps1):

- **Envy: Fast Build** — `/t:Envy` (Envy + solution dependencies, no Installer)
- **Envy: Full Build** — `/t:Build` (whole solution including Installer / ISCC)

Both prompt for Configuration (`Debug` / `Release`, default `Debug`) and
Platform (`x64` / `Win32`, default `x64`). The runner locates MSBuild via
`vswhere.exe` (prerelease-aware, covers Build Tools) and never embeds
machine-specific paths; an `msbuild.exe` found on `PATH` is used only when its
Visual Studio install has a usable v145 toolset. Concurrent runs on the same
checkout are serialized by a cross-process lock (builds started from Visual
Studio do not take it). `Ctrl+Shift+B` runs Fast Build. Rebuild, Clean, and
Installer-only remain available in Visual Studio / MSBuild; Full Build uses
the same Installer CustomBuild path as Visual Studio "Build Solution"
(bundled `Installer/InnoSetup/ISCC.exe`, live repo `SourceDir`). If that
bundled `ISCC.exe` is missing, the Installer project now fails the build
(any configuration) instead of skipping packaging; use Fast Build when no
package is needed.

Use **one** C++ language service: prefer **clangd** for IntelliSense/navigation. Disable duplicate MSVC IntelliSense in personal settings if both are active. The repo lists `ms-vscode.cpptools` under `unwantedRecommendations` only to reduce accidental dual-IntelliSense installs; you may still install it for MSVC debugging if your workflow needs it.

### clangd and `compile_commands.json`

`compile_commands.json` at the repository root is **generated** and **gitignored**. After a successful VS/MSBuild build (`generate-compile-commands.ps1` splits batched `CL.command.*.tlog` markers that list multiple sources with `|`):

```powershell
.\tools\dev\setup-windows.ps1 -GenerateCompileCommands
# or
.\tools\dev\generate-compile-commands.ps1 -Configuration 'Release|x64'
```

clangd resolves the MSVC and Windows SDK headers from the include paths recorded
in `compile_commands.json`. Do **not** configure `--query-driver` with `cl.exe`:
clangd's query-driver mechanism only supports GCC-compatible drivers, and
`cl.exe` does not implement that interface. If system headers stay unresolved,
regenerate the database and verify that its `command` entries carry the full
`/I` include paths for the MSVC toolset and Windows SDK (run MSBuild from an
environment where those paths resolve). Apply any extra include-path workarounds
in **personal** (gitignored) or user settings — never in the repository.

Install LLVM **clangd** via the clangd extension or a system LLVM build.

## Source encoding (#350)

Tracked in [issue #350](https://github.com/Mika3578/Envy/issues/350).

**Current tree (first-party `Envy/`, `HashLib/`, `TorrentEnvy/`, `Unpacker/`, `SkinBuilder/`, `Installer/**/*.iss`, `tests/` smoke sources — same prefixes as `FIRST_PARTY_PREFIXES` / `SOURCE_SUFFIXES` + `.iss` in the encoding guard, ~1066 files audited on this branch):**

- ~764 files are **not** valid UTF-8 (historical single-byte header/comments)
- ~73 files already contain **U+FFFD** (`EF BF BD`) in the committed blob
- ~229 files are valid UTF-8 without new replacement debt
- These three mutually exclusive buckets sum to the audited total (~1066).

**Rules:**

- Do **not** mass-convert encodings or declare all C++ CP1252/Latin-1 globally.
- `.editorconfig` does **not** set `charset = utf-8` on legacy C/C++, legacy `.iss`, PowerShell under `Services/`, or `Languages/*.xml` (many still carry a UTF-8 BOM); it uses `charset = unset` there and UTF-8 only where content is genuinely UTF-8 without a byte-preservation conflict (YAML, JSON, Markdown, …).
- Prefer **Visual Studio** to edit/save legacy sources until a file is explicitly migrated under #350.
- CI runs [`.github/scripts/check-source-encoding.sh`](../../.github/scripts/check-source-encoding.sh) with [`.github/scripts/check_source_encoding_lib.py`](../../.github/scripts/check_source_encoding_lib.py): on changed first-party C/C++ and `Installer/**/*.iss`, it **fails when HEAD introduces new corruption vs the merge base** (baseline debt on `develop` is allowed). Detected classes include new `U+FFFD` (`EF BF BD`), new UTF-8 `C2 9D` / Unicode C1 controls (`U+0080`–`U+009F`) in valid UTF-8 text, common mojibake (`Â©`, …), and byte changes to Envy copyright/installer metadata lines (for example legacy `©` / `0xA9` becoming `U+009D` or `?`).
- **Functional PR rule:** do not rewrite, normalize, transliterate, or otherwise alter unrelated non-ASCII bytes, copyright/license headers, BOMs, or line endings in legacy sources. Encoding migration belongs in dedicated PRs tracked under #350. Maintainers apply the `encoding-migration` label on that PR; Code Quality re-runs on `labeled` / `unlabeled`, and the base-pinned encoding guard reads the label from the GitHub event payload (`encoding_migration_pr_from_github_event`) so intentional header-byte preservation is warn-only on those PRs only.

**Before you finish a change**, inspect files you **actually modified** (not merely opened) for unintended encoding conversion, new `U+FFFD`, new `U+0080`–`U+009F` / mojibake, accidental copyright byte changes, EOL-only churn, secrets, or local paths. A functional PR must not rewrite unrelated non-ASCII bytes or copyright/license header lines in legacy C/C++; use a dedicated encoding migration PR (#350) instead.

## Project-owned vs local

| Versioned | Local / gitignored |
| --- | --- |
| `.editorconfig`, `.gitattributes` | Personal VS/Cursor user settings |
| `.vscode/settings.json`, `extensions.json`, `tasks.json` | Other `.vscode/*` files |
| `tools/dev/*.ps1` | `compile_commands.json` |
| CI scripts | `vcpkg_installed/`, build outputs, `.vs/` |

## Verification

```powershell
.\tools\dev\setup-windows.ps1 -Check
```

On Linux CI, encoding self-test: `bash .github/scripts/check-source-encoding.selftest.sh`

## Security

Secret scanning and gitleaks run in existing workflows (see `.github/workflows/security.yml`). This document does not duplicate those controls.

## Troubleshooting

| Symptom | Likely cause | Action |
| --- | --- | --- |
| git pull refuses: untracked `.vscode/settings.json` would be overwritten | Local untracked copy predates tracked policy | Move or remove your local `.vscode/settings.json` / `extensions.json`, then pull |
| Unicode replacement character (U+FFFD) or mojibake in copyright line after save | UTF-8 forced on legacy bytes | Restore from git; edit in VS; see #350 |
| clangd no compile flags | Missing `compile_commands.json` | Build in VS, run `generate-compile-commands.ps1` |
| Format CI fails | Changed hunk not clang-format clean | Format **modified lines only** |
| Encoding CI fails | New `EF BF BD` in diff | Revert corruption; do not commit replacement bytes |

## Related

- [build.md](build.md) — MSBuild, vcpkg, CI parity
- [standards.md](standards.md) — C++ style and tooling overview
- [contributing.md](contributing.md) — PR process
