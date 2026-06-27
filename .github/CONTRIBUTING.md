# Contributing to Envy

Thanks for your interest in improving Envy. This file describes the
ground rules for contributing code, translations, skins, or docs.

## License agreement

Envy is licensed under **AGPL-3.0-or-later**. Some visual resources and
the `LibGFL` library carry additional CC-BY-NC-SA terms (see `ReadMe.txt`).
By submitting a pull request you confirm that your contribution can be
distributed under those terms.

## Quick start

1. Install **Visual Studio 2026 v18.0+** (Community is fine) with:
   - Workload "Desktop development with C++"
   - Components "MFC for v145", "ATL for v145", "C++ Spectre-mitigated libs (v145)"
   - "C++ CMake tools for Windows"
   - "Windows 10/11 SDK (latest)"
2. Clone with submodules and bootstrap vcpkg:
   ```
   git clone https://github.com/mika3578/envy.git
   cd envy
   git clone https://github.com/microsoft/vcpkg.git
   .\vcpkg\bootstrap-vcpkg.bat
   ```
3. Open `Visual Studio\Envy.sln` and build (Ctrl+Shift+B).

If you migrated from VS 2017/2019: run `Visual Studio\SetVS2026.bat` once
to retarget every project to v145.

## Branch model

- `main` - stable, releases tagged `v*` from here.
- `develop` - integration branch (default).
- `legacy` - frozen pre-modernization snapshot for historical builds.
- Feature branches : `feature/<short-name>` or `claude/<short-name>` for
  AI-assisted work.

## Git workflow (linear history)

`develop` keeps a **linear history** with no merge commits. Keep local pulls
fast-forward-only and rebase feature branches before opening or updating PRs.

### Repository merge settings (GitHub)

- Merge commits: **disabled**
- Squash merge: **enabled** (preferred)
- Rebase merge: **enabled** (optional)
- Require linear history on `develop`: **enabled** via the active `Protect develop` ruleset
- Force pushes and branch deletions on `develop`: **blocked**

### Local settings

Configure fast-forward-only pulls so a plain `git pull` never creates merge
commits:

```bash
git config pull.ff only
```

This configures fast-forward-only pulls for this local clone. To apply
it globally across all your repositories instead, run
`git config --global pull.ff only`.

### Sync `develop` locally

```bash
git fetch origin
git checkout develop
git pull --ff-only origin develop
```

If `git pull --ff-only` fails, your local `develop` has diverged. Reset it
to the remote (**ensure your working tree is clean first — `git reset --hard`
discards all uncommitted changes**; stash or commit any work in progress):

```bash
git fetch origin
git checkout develop
git reset --hard origin/develop
```

Never commit directly on `develop`; always use a feature branch and PR.

### Feature branch workflow (before opening or updating a PR)

```bash
git fetch origin
git checkout your-feature-branch
git rebase origin/develop
git push --force-with-lease
```

Use `--force-with-lease`, not `--force`, so you do not overwrite someone
else's pushed work.

### Protected branch policy

The active `Protect develop` ruleset requires pull requests, linear history,
passing status checks, and blocks force-pushes and branch deletion. The
declarative template in `.github/settings.yml` mirrors this intended policy
for Probot Settings or manual audits.

## Pull request checklist

- [ ] Builds Release x64 and Release Win32 with toolset v145.
- [ ] No new compiler warnings (use the CI build log).
- [ ] CodeQL passes without new HIGH/CRITICAL findings.
- [ ] If you touched anything in `Services/` or `Plugins/`, ping a
      CODEOWNER for review.
- [ ] If you bumped a vcpkg dependency, note the version delta in the
      PR description.
- [ ] Translations stay in sync with the keys in `Envy.exe`
      (run `Languages\Tools\extract.bat`).

## Coding style

The codebase predates modern C++ conventions; please match the surrounding
code rather than introducing a new style island:

- Tabs for indentation, width 4.
- Allman braces (`{` on its own line).
- Hungarian-ish notation (`m_`, `p`, `n`, `b`, `str`).
- Prefer `CString` / `CAtlList` over `std::string` / `std::list` inside MFC code.
- Use `noexcept`, not `throw()`.
- Avoid `using namespace std;` at file scope.
- New code SHOULD compile with `/std:c++20`; legacy plugins may stay on
  `/std:c++17`.

`.clang-format` is provided for advisory checks. It is intentionally
conservative - do not bulk-reformat existing files.

## Reporting bugs

Use the bug report issue template. Include the BugTrap dump under
`%APPDATA%\Envy\` when relevant.

## Security

See `.github/SECURITY.md`. Never open public issues for security bugs.
