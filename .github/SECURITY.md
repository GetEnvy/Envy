# Security Policy

## Supported Versions

Envy is built from this `main` branch. Security fixes are applied to the
latest release line only.

| Version | Supported          |
| ------- | ------------------ |
| 5.0.x   | :white_check_mark: |
| 4.x     | :x:                |
| < 4.0   | :x:                |

## Reporting a Vulnerability

**Do not open public GitHub issues for security problems.**

Please report security vulnerabilities privately through GitHub's
[Security Advisories](https://github.com/mika3578/envy/security/advisories/new)
feature, or by email to the maintainers listed in `CODEOWNERS`.

Include:

- A clear description of the issue
- Steps to reproduce
- Affected files / version / commit hash
- Any known mitigation

You should receive an acknowledgement within 5 business days. We aim to
publish a coordinated fix within 90 days of confirmation, sooner for
actively exploited issues.

## Hardening posture

This repository ships with the following automated defenses:

- **CodeQL** scans on every push and pull request (`.github/workflows/codeql.yml`).
- **Dependabot** version updates for vcpkg dependencies and GitHub Actions
  (`.github/dependabot.yml`).
- **Dependency review** on pull requests
  (`.github/workflows/dependency-review.yml`).
- **Secret scanning** is enabled at the repository level.
- MSVC builds enable `/sdl`, `/GS`, `/guard:cf`, and Spectre-mitigated
  runtime libraries in Release configurations.

## Scope

In-scope:

- Memory safety issues in `Envy/`, `Plugins/`, `Services/`, `HashLib/`,
  `TorrentEnvy/`, `Unpacker/`.
- Network protocol parsing bugs (Gnutella, eDonkey, BitTorrent, DC++, etc.).
- Issues in vendored third-party libraries when actually exercised by Envy.

Out of scope:

- Social engineering / phishing of contributors.
- Findings against bundled binary blobs that are no longer compiled in
  (legacy `Plugins/PluginWizard/` templates etc.).
- Reports requiring physical access to the user's machine.
