# Envy - Modernization Plan (May 2026)

This document is the audit and modernization plan for the Envy source
tree (multi-network P2P client for Windows). It covers the migration
to **Visual Studio 2026 / PlatformToolset v145 / MSVC 14.50**, the
introduction of a complete GitHub Actions CI/CD pipeline, and the
automation of dependency updates.

> **Audit date** : 2026-05-15
> **Target branch** : `claude/code-audit-modernization-nJcTT`

For the day-by-day execution log, see [`docs/DEV_TRACKER.md`](./docs/DEV_TRACKER.md).
For AI assistant rules and conventions, see [`AGENTS.md`](./AGENTS.md).

---

## 1. Audit - state before migration

| Metric | Value |
| --- | --- |
| MSBuild projects (.vcxproj) | 46 |
| `.cpp` / `.h` files | 677 / 709 |
| Toolsets in use | v141_xp (36), v142 (12), v140 (templates) |
| OS target | Windows XP+ (via v141_xp + `_ATL_XP_TARGETING`) |
| Configured C++ Standard | **none** (MSVC default = C++14) |
| `WindowsTargetPlatformVersion` | undefined (SDK default) |
| `_CRT_SECURE_NO_WARNINGS` enabled | yes (all projects) |
| `pragma warning(disable...)` directives | 70 occurrences in `Envy/` |
| Inline ASM | third-party only (UnRAR, BugTrap) |
| CI/CD | **none** (no workflows, no AppVeyor) |
| Source encoding | mixed ISO-8859 / UTF-8, BOM not systematic |
| Third-party deps | **bundled sources** in `Services/` and `Plugins/` |

### Bundled third-party dependencies

| Library | Current version | Status |
| --- | --- | --- |
| zlib | 1.2.10 (2017-01) | Outdated (latest 1.3.1, known CVEs) |
| sqlite3 | 3.30.0 (2019-10) | Outdated (latest 3.46+) |
| bzip2 (`Bzlib`) | 1.0.5 (2007) | **Very outdated** (CVE-2016-3189, CVE-2019-12900) |
| miniupnpc | unknown | Outdated |
| UnRAR | 5.30 (2015-11) | Outdated, contains x86 inline ASM |
| GeoIP | unknown | Legacy MaxMind, should move to libmaxminddb |
| LibUTP | 2010 snapshot | Outdated |
| BugTrap | 2005-2010 | Evaluate; can be replaced by Windows Error Reporting |
| LibGFL | 3.40 (~2003) | **Very outdated** (non-free binary, AGPL conflict) |

---

## 2. Target

| Metric | Target |
| --- | --- |
| Toolset | **v145** (VS 2026 v18.0+ / MSVC 14.50) |
| OS target | **Windows 10 1809 (build 17763)** or newer |
| `WindowsTargetPlatformVersion` | `10.0` (latest SDK installed) |
| C++ standard | **C++20** for first-party code, **C++17** for legacy plugins |
| `/permissive-` | Phase 2 (after warning cleanup) |
| Architectures | x64 (primary), Win32 (compat), **ARM64** (new) |
| Mitigations | `/GS`, `/guard:cf`, `/sdl`, Spectre runtime libs |
| Deps management | **vcpkg manifest** (`vcpkg.json`) |
| Auto-update | **Dependabot** (vcpkg + GitHub Actions) |
| CI | GitHub Actions on `windows-2025` runners |
| Code analysis | CodeQL (cpp), MSVC `/analyze`, clang-tidy (advisory) |

---

## 3. Execution phases

### Phase 0 - Infrastructure bootstrap (this PR)

- [x] Full codebase audit
- [x] Architectural decisions (drop XP, vcpkg manifest)
- [x] `.gitignore`, `.editorconfig`, `.clang-format`, `.clang-format-ignore`
- [x] `vcpkg.json` + `vcpkg-configuration.json` seeded
- [x] `.github/dependabot.yml` (vcpkg + actions, weekly)
- [x] `.github/CODEOWNERS`, `SECURITY.md`, PR template, 3 issue templates
- [x] Workflows: `build.yml`, `codeql.yml`, `dependency-review.yml`,
      `clang-tidy.yml`, `format-check.yml`, `release.yml`, `stale.yml`,
      `labeler.yml`, `copilot-setup-steps.yml`, `dependabot-auto-merge.yml`
- [x] `Visual Studio/SetVS2026.bat` + `SetVS2026.ps1` (retarget scripts)
- [x] Mechanical migration: 142 `<PlatformToolset>` -> v145 across 45 projects
- [x] Removed `_ATL_XP_TARGETING` (66 sites) and `ENVY_USE_ASM` (2 sites)
- [x] Injected `<WindowsTargetPlatformVersion>10.0</...>` into 44 projects
- [x] `<LanguageStandard>stdcpp20</...>` on Envy + 12 first-party projects
- [x] `<LanguageStandard>stdcpp17</...>` on 19 plugins
- [x] Removed `register` keyword (Envy/Buffer.cpp) - reserved in C++17
- [x] Replaced `throw()` with `noexcept` (Envy/Buffer.{h,cpp}, Connection.h)
- [x] Modernized `Envy/StdAfx.h`: Win 10 baseline, MSVC 14.50 requirement,
      auto-XPSUPPORT detection removed
- [x] AI rules file (`AGENTS.md`) and living dev tracker (`docs/DEV_TRACKER.md`)

### Phase 1 - First green build (next PR)

- [ ] First compile on `windows-2025` with v145
- [ ] Fix C++20 errors (third-party `std::auto_ptr`, `wstring_convert`, ...)
- [ ] Update include / linker paths for vcpkg libs
- [ ] Temporarily disable broken plugins (RatDVD, SWF if SDK unavailable)
- [ ] Verify `Envy.exe` launches and connects to Gnutella

### Phase 2 - Warning cleanup + /permissive-

- [ ] Enable `<ConformanceMode>true</ConformanceMode>` (`/permissive-`)
- [ ] Review the 70 `#pragma warning(disable ...)` directives:
      - Keep only those still necessary
      - Document survivors
- [ ] Enable `<SDLCheck>true</SDLCheck>` (`/sdl`)
- [ ] Enable `<ControlFlowGuard>Guard</ControlFlowGuard>` (`/guard:cf`)
- [ ] Enable `<SpectreMitigation>Spectre</SpectreMitigation>` in Release
- [ ] Drop `_CRT_SECURE_NO_WARNINGS` per project: replace unsafe
      `strcpy`, `sprintf`, `gets` with `_s` variants or `string_view`/`format`

### Phase 3 - Dependency modernization

- [ ] Move every bundled lib to vcpkg:
      - `Services/zlib` -> `vcpkg install zlib`
      - `Services/Bzlib` -> `vcpkg install bzip2`
      - `Services/SQLite` -> `vcpkg install sqlite3`
      - `Services/MiniUPnP` -> `vcpkg install miniupnpc`
      - `Services/GeoIP` -> `vcpkg install libmaxminddb` (modern replacement)
- [ ] Delete the corresponding `Services/<lib>/` subtrees
- [ ] Evaluate `BugTrap` -> Windows Error Reporting (WER) migration
- [ ] Evaluate removing `LibGFL` (non-free binary, AGPL conflict)

### Phase 4 - Runtime robustness

- [ ] Wire HashLib unit tests into CI (vcpkg `tests` feature enabled)
- [ ] AddressSanitizer (`/fsanitize=address`) on Debug configurations
- [ ] Publish crash-dump symbols as GitHub artifacts
- [ ] Verify the 6% MSVC backend improvement advertised by the cppblog

### Phase 5 - ARM64 + signing

- [ ] Add `Release|ARM64` to `Envy.sln` and every `.vcxproj`
- [ ] Code signing via Azure Trusted Signing (in `release.yml`)
- [ ] WiX/MSIX alongside Inno Setup for Microsoft Store distribution

---

## 4. CI/CD infrastructure delivered

### Workflows

| File | Trigger | Main job |
| --- | --- | --- |
| `.github/workflows/build.yml` | push / PR | Matrix x64+Win32 x Release+Debug on windows-2025, v145, vcpkg restore, MSBuild, upload logs |
| `.github/workflows/codeql.yml` | push / PR / weekly | C/C++ analysis with security-extended + security-and-quality queries |
| `.github/workflows/dependency-review.yml` | PR | dependency-review-action + jq validation of `vcpkg.json` |
| `.github/workflows/clang-tidy.yml` | PR | clang-tidy on changed files, **advisory** (continue-on-error) |
| `.github/workflows/format-check.yml` | PR | clang-format --dry-run, **advisory** |
| `.github/workflows/release.yml` | tag `v*` | Build x64+Win32, zip, draft GitHub Release |
| `.github/workflows/stale.yml` | daily | Mark stale after 90d (issue) / 45d (PR) |
| `.github/workflows/labeler.yml` | PR | Auto-label by paths (build, ci, core, plugins, ...) |
| `.github/workflows/copilot-setup-steps.yml` | manual | Pre-warm Copilot environment |
| `.github/workflows/dependabot-auto-merge.yml` | Dependabot PR | Auto-approve+merge actions minor/patch only |

### Bots and automation

- **Dependabot**:
  - vcpkg, weekly (Monday 07:00 Europe/Paris), single "vcpkg-baseline"
    group that advances the `builtin-baseline` commit hash.
  - GitHub Actions, weekly, `actions-minor-patch` group.
- **Auto-merge bot**: minor/patch GitHub Actions only - human review
  required for vcpkg baseline bumps (can change native lib ABI).
- **Stale bot**: auto-close after inactivity (issues 90+14d, PRs 45+21d).
- **Labeler**: auto-tag PRs based on touched paths.
- **CodeQL**: weekly analysis + per-PR.

### Templates and conventions

- `.github/CODEOWNERS`: required review for CI / build files / third-party.
- `.github/SECURITY.md`: responsible disclosure policy.
- `.github/pull_request_template.md`: build x64/Win32 + analysis checklist.
- `.github/ISSUE_TEMPLATE/{bug_report,feature_request,build_failure}.yml`.
- `.editorconfig`: tab/4 for C++, space/2 for XML/JSON, LF for YAML.
- `.clang-format` (Microsoft, conservative) + `.clang-format-ignore` to
  exclude third-party sources.

---

## 5. How to build locally with VS 2026

```cmd
:: Prerequisites: Visual Studio 2026 v18.0+ with:
::   - Desktop development with C++
::   - MSVC v145 (default toolset)
::   - MFC / ATL for v145
::   - Windows 10/11 SDK (latest)
::   - C++ Spectre-mitigated libs (v145)
::   - C++ CMake tools for Windows
::   - vcpkg (bundled with VS 2026)

:: 1. Clone and switch to the branch
git clone https://github.com/mika3578/envy.git
cd envy
git checkout claude/code-audit-modernization-nJcTT

:: 2. Bootstrap vcpkg (manifest mode auto-enabled by VS 2026)
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

:: 3. (Optional) Force re-target of every vcxproj
cd "Visual Studio"
SetVS2026.bat
cd ..

:: 4. Open the solution
start "" "Visual Studio\Envy.sln"

:: 5. Build > Build Solution (Ctrl+Shift+B)
```

Or from CLI:

```cmd
msbuild "Visual Studio\Envy.sln" /m /p:Configuration=Release /p:Platform=x64 ^
  /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0 ^
  /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x64-windows-static
```

---

## 6. How to enable Dependabot and the workflows

1. **Push the branch** to GitHub:
   ```
   git push -u origin claude/code-audit-modernization-nJcTT
   ```
2. **Create the draft PR** against `main`.
3. In the repo **Settings -> Security and analysis**:
   - Enable "Dependency graph"
   - Enable "Dependabot alerts"
   - Enable "Dependabot security updates"
   - Enable "Dependabot version updates" (uses `.github/dependabot.yml`)
   - Enable "Code scanning" (CodeQL through the workflow)
   - Enable "Secret scanning" and "Push protection"
4. **Branch protection** on `main`:
   - Require pull request reviews (1+)
   - Require status checks: `Build x64 Release`, `Analyze C/C++`,
     `Dependency review`
   - Require CODEOWNERS review for `.github/` and `Visual Studio/`
5. First Dependabot run: the placeholder `0000...` `builtin-baseline`
   value in `vcpkg.json` will be replaced with a recent commit from the
   vcpkg registry automatically.

---

## 7. Known risks

| Risk | Mitigation |
| --- | --- |
| C++20 breakages in legacy MFC code (200+ files) | Phase 1 = build + incremental fixes, `<ConformanceMode>` disabled at first |
| Plugins depending on obsolete SDKs (RatDVD, SWF, DirectShow) | Already commented out in `Envy.sln`, separate audit |
| x86 inline ASM in UnRAR / BugTrap | Already conditional on Win32 - x64 uses C variants |
| Win 10+ target breaks the historical XP user base | Documented in `SECURITY.md`; existing `legacy` branch preserved |
| First `vcpkg install` is slow | GitHub Actions cache keyed on `vcpkg.json` hash |
| LibGFL non-free vs AGPL license | Track an issue; may need to drop from default build |
| Inno Setup absent from hosted runners | Add `chocolatey install innosetup` step or move to a separate job |

---

## 8. Backout

All changes are localized and reversible:

```cmd
:: Restore the v141_xp toolset for legacy VS 2017 builds
"Visual Studio\SetVS2017.bat"

:: Or disable the workflows
git rm -r .github/workflows
```

The `legacy` branch keeps the pre-modernization snapshot.

---

## 9. Success metrics

- [ ] Green CI on `windows-2025` for `Release|x64` and `Release|Win32`
- [ ] First CodeQL scan completes with no CRITICAL findings
- [ ] Dependabot opens its first vcpkg PR within 7 days
- [ ] No `_ATL_XP_TARGETING` or `v141_xp` remains in `git grep`
- [ ] Local VS 2026 build green with no manual intervention
