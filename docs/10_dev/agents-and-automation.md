# Envy Development Agents & Automation

**Last Updated:** January 2026

## What exists today

- **CI/CD:** `.github/workflows/` — `build.yml` (Release/Debug × Win32/x64), `code-quality.yml` (analysis, format, link check, dependency review), `codeql.yml`, `release.yml`, `stale.yml`, `version-management.yml`
- **Versioning:** `scripts/auto-version.ps1`, `scripts/bump-version.ps1`, `version.json`
- **Build:** `build_all.ps1` (local full-matrix build via MSBuild)
- **AI:** `.github/copilot-instructions.md`, `.cursor/rules/`

## Automation reference

| Area | Tools / config |
|------|----------------|
| **Code analysis** | MSBuild Code Analysis, CodeQL (CI); `.clang-tidy`, `.cppcheck-suppressions` |
| **Format / docs** | clang-format; markdown link check in `code-quality.yml` |
| **Dependencies** | Dependabot, GitHub dependency review (CI) |
| **Testing** | Standalone integration tests in `tests/` (manual); CI runs build only |
| **Security** | CodeQL, dependency review |

## Related

- [Guide](guide.md) · [Build](build.md) · [Status](status.md) · [AI Coding Guide](ai-coding-guide.md)
- [GitHub Issues](https://github.com/Mika3578/Envy/issues) · [Discussions](https://github.com/Mika3578/Envy/discussions)
