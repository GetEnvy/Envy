# AGENTS.md - Rules for AI coding assistants on the Envy repository

This file is the single source of truth for AI assistants working on
Envy. It is read by:

- **GitHub Copilot** (chat + code completions)
- **Claude Code** (CLI, GitHub Action, web/IDE sessions)
- **Cursor**, **Cline**, **Aider**, **Windsurf**, etc., via their
  conventional rule files - see the symlinks / pointers at the bottom of
  this document.

Keep this file short. If you find yourself wanting to add a paragraph,
write it in `MODERNIZATION.md` or `docs/DEVELOPMENT_PLAN.md` and link to it here.

---

## 1. Project overview

Envy is a multi-network peer-to-peer client for Windows. Stack:

- **C++** (MFC + ATL, mostly Windows-specific) - ~677 .cpp / 709 .h files
- **MSBuild** 46 `.vcxproj` projects under `Visual Studio/Envy.sln`
- **Target toolchain**: Visual Studio 2026 (toolset **v145**, MSVC 14.50,
  C++20 for first-party, C++17 for legacy plugins)
- **Target OS**: Windows 10 1809+ (XP/Vista/7/8 deliberately dropped)
- **Dependencies**: managed via **vcpkg manifest** (`vcpkg.json`)
- **License**: AGPL-3.0-or-later (`Envy/AGPL-License.txt`). Some bundled
  resources have additional CC-BY-NC-SA terms - see `ReadMe.txt`.

Branch model:

- **`develop`** is the active integration branch and the repository default.
- Create every work branch from the latest **`origin/develop`**.
- Never push directly to **`develop`**, **`main`**, or **`legacy`**; land
  changes through a pull request to **`develop`**.
- Keep **`develop`** history **linear**: merge PRs with **squash** or
  **rebase** only; ordinary merge commits are not allowed on **`develop`**.
- Branch names use `type/short-kebab-summary` (see hard rule 11).

---

## 2. Hard rules

1. **Language**: all human-readable artifacts you produce or edit -
   code comments, commit messages, PR bodies, issue descriptions,
   `MODERNIZATION.md`, `docs/DEVELOPMENT_PLAN.md`, workflow names, error messages -
   are written in **English**. Reply to the user in the language they
   used in chat; that's separate from the artifacts above.
2. **No XP support**. Do not reintroduce `_ATL_XP_TARGETING`, `v141_xp`,
   `XPSUPPORT`, `_WIN32_WINNT < 0x0A00`. If you need conditional code,
   gate it on Windows feature detection, not on the OS version.
3. **No build files in `Plugins/PluginWizard/**`**. Those are project
   templates Visual Studio uses to scaffold new plugins; they must stay
   un-retargeted.
4. **Third-party trees are read-only** unless you are deliberately
   upgrading them. The relevant trees:
   `Services/{zlib,SQLite,Bzlib,UnRAR,MiniUPnP,GeoIP,LibUTP,BugTrap}`,
   `Plugins/{RatDVDPlugin,SWFPlugin}`, `HashLib/HashLib/*` (HashLib has
   its own first-party wrappers). Touch them only via vcpkg
   replacement, never by hand-editing.
5. **`.clang-format` is conservative**. Do not bulk-reformat existing
   files. New files SHOULD pass `clang-format --dry-run`; existing files
   stay as-is.
6. **No `using namespace std;`** at file scope in new code. Inside MFC
   code prefer `CString`, `CAtlList`, `CMap` over `std::` containers to
   match the surrounding style.
7. **`noexcept`, not `throw()`**. The latter is removed in C++20.
8. **No `register` keyword**. Removed in C++17.
9. **No GPL-3.0 or GPL-2.0 dependencies**. AGPL-3.0 is fine because
   Envy itself is AGPL-3.0+. See
   `.github/workflows/dependency-review.yml`.
10. **Never skip git hooks** (`--no-verify`, `--no-gpg-sign`) and never
    force-push to `main` or `develop`. Always create new commits rather
    than amending.
11. **Branch naming**. Use functional `type/short-kebab-summary` names
    branched off `develop`. Allowed prefixes include `feat/`, `fix/`,
    `docs/`, `refactor/`, `perf/`, `test/`, `build/`, `ci/`, `chore/`,
    and `hotfix/`. Examples: `fix/ed2k-source-validation`,
    `docs/align-development-rules`, `ci/add-pr-quick-checks`,
    `security/configure-scorecard`. Never create or push tool- or
    agent-prefixed branches such as `claude/`, `cursor/`, `codex/`,
    `aider/`, `copilot/`, or `agent/`; the name must describe the change,
    not the tool that produced it.
12. **Human approval at the merge gate**. An assistant may create a work
    branch, commit on that branch, push the branch, and open a **draft**
    PR. An assistant must **not** mark a PR ready-for-review, enable
    auto-merge, or merge without explicit maintainer approval. When a
    maintainer explicitly instructs you to merge a specific PR after its
    checks pass, you may perform that merge. Never push to
    `main`/`develop`/`legacy` directly.

---

## 3. Build & test commands

Local (Windows + VS 2026):

```cmd
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64 ^
  /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0 ^
  /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x64-windows-static
```

CI mirrors this exactly - see `.github/workflows/build.yml`.

HashLib unit tests (post-Phase 4):

```cmd
msbuild HashLib\HashTest\HashTest.vcxproj /p:Configuration=Release /p:Platform=x64
.\HashLib\HashTest\x64\Release\HashTest.exe
```

There is no `make test` or `cargo test` - all building flows through
MSBuild.

**Build authority:** `Visual Studio/Envy.sln` is the authoritative build
definition. Visual Studio 2026, MSBuild, and toolset **v145** are the
primary path. Existing CMake files (`CMakePresets.json`, partial
`CMakeLists.txt` trees) are auxiliary or experimental until Phase 5
migration completes. Do not treat CMake as equivalent to the Visual Studio
solution, do not modify the solution solely to satisfy CMake, and do not
add new CMake changes outside a PR explicitly dedicated to CMake work.

---

## 4. Code conventions

This is a legacy MFC codebase. **Match the style around you.** General
patterns you will see and should preserve:

- Indentation: **tabs**, width 4.
- Braces: Allman (opening brace on its own line).
- Naming: `m_` for members, `p` pointer, `n` numeric, `b` boolean,
  `str` string, `dw` DWORD, `lp` long pointer.
- Headers: include `StdAfx.h` first in every `.cpp`, then own header,
  then project headers, then SDK headers.
- Strings: `CString` (MFC) inside Envy/. New code may use `std::string`
  only at clean boundaries.
- Concurrency: `CCriticalSection`, `CSingleLock` (MFC), not
  `std::mutex`, unless wrapping pure standalone helpers.
- Comments: write **why**, not **what**. Don't paraphrase the code.
- File headers: keep the existing copyright block intact when editing
  an existing file; do not add new copyright lines for incremental
  edits.

---

## 5. Workflow expectations

When you take on a task you are expected to:

1. **Record progress** in `docs/DEVELOPMENT_PLAN.md` for strategic scope.
   Session-level notes belong in `.local/DEV_TRACKER.md` (gitignored).
2. **Push only to your feature branch** (never `develop`, `main`, or
   `legacy`) with `git push -u origin <branch>`.
3. **Open a draft PR** if one does not exist. Match the PR template at
   `.github/pull_request_template.md`.
4. **Tick the checkboxes** in the PR template that genuinely apply -
   don't blanket-check them.
5. **Cite file:line** in chat replies when discussing code:
   `Envy/Buffer.cpp:782`, never paraphrased.
6. **Never edit a vendored third-party file** to suppress a warning -
   either fix it upstream (vcpkg port), add a `/wd<num>` per-project,
   or leave the warning.
7. **Cluster mechanical edits**. If you are going to rewrite a token
   across N files, write a Python/PowerShell script under
   `Visual Studio/` (or a tmp script you delete), run it, commit the
   resulting diff. Don't hand-edit 40 files.

---

## 6. Repository map (top of tree)

| Path | What's there |
| --- | --- |
| `Envy/` | Main MFC application (UI, networking, DB, search). |
| `TorrentEnvy/` | BitTorrent-only standalone variant. |
| `HashLib/` | First-party hashing (Tiger, ED2K, SHA1, MD4). |
| `Unpacker/` | Archive extraction harness. |
| `SkinBuilder/` | Skin authoring tool. |
| `Installer/` | Inno Setup scripts (.iss). |
| `Plugins/` | 24 plugins: media, image, archive, web hooks, etc. |
| `Services/` | Vendored third-party libs (will move to vcpkg, Phase 3). |
| `Languages/` | 62 XML translation files + Poedit tooling. |
| `Skins/` | UI skins. |
| `Data/` | Default settings, GeoIP DB. |
| `Repository/` | Auxiliary tools (RTF compaction, keyword tests, ...). |
| `Remote/` | Web-based remote-control UI assets. |
| `Schemas/` | XML schemas for files/folders metadata. |
| `Visual Studio/` | `Envy.sln`, `SetVS20XX.bat` retarget scripts. |
| `Templates/` | Help and CHM source. |

---

## 7. Specific don'ts

- **Don't** add `#pragma warning(disable: ...)` to silence a new
  warning. Fix the warning or document why it must stay.
- **Don't** add new dependencies to `vcpkg.json` without first
  discussing in `docs/DEVELOPMENT_PLAN.md` (architectural decisions block).
- **Don't** expand CMake beyond its current auxiliary role. Existing
  partial CMake files and `CMakePresets.json` are not authoritative;
  do not add new CMake changes outside a PR explicitly dedicated to
  CMake work, and do not modify `Visual Studio/Envy.sln` solely to
  satisfy CMake.
- **Don't** rewrite `MODERNIZATION.md` from scratch. Update the
  checklists, don't reflow the prose.
- **Don't** translate translated XML files in `Languages/`. Only
  developers fluent in the target language should change those.
- **Don't** delete files in `Skins/` or `Data/`. They are runtime
  resources and may be referenced by hard-coded paths.

---

## 8. AI-tool-specific notes

The following tools all read this file. Where they need a private
config you'll find a thin pointer file that delegates here.

- `.github/copilot-instructions.md` - GitHub Copilot context.
- `.cursorrules` and `.cursor/rules/*.mdc` - Cursor IDE.
- `.clinerules` - Cline.
- `.aider.conf.yml` - Aider.
- `.windsurfrules` - Windsurf.
- `CLAUDE.md` - Claude Code (CLI / web / IDE).
- `.continue/rules/*.md` - Continue.

Keep these pointers; do not let them drift into independent rule sets.
If a rule needs to change, change it in **this file** and let the
others continue to delegate.

---

## 9. Escalation

If you are blocked and can't make progress on a task:

1. Write the dead-end into `docs/DEVELOPMENT_PLAN.md` under "Blockers"
   or open a GitHub issue.
2. Open or update an issue using the `build_failure.yml` template if
   the blocker is a build error.
3. Stop. Do not invent workarounds (`/* TODO */`, dummy returns,
   `// suppressed for now`) just to make the build green; those rot
   silently. Surface the problem and wait for guidance.
