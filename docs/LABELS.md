# Recommended GitHub Labels

These labels improve triage, risk signaling, and planning.

## Area Labels
- `area: ci`
- `area: protocols`
- `area: security`
- `area: docs`

## Automation Labels (Do Not Remove)
- `dependencies`
- `ci`

## Risk Labels
- `risk: low`
- `risk: medium`
- `risk: high`

## Needs / Blocker Labels
- `needs: local-vs-build`
- `blocked: toolchain`

## Usage Guidance
- Apply at least one `area:*` and one `risk:*` label to each PR.
- Keep `dependencies` and `ci` available for Dependabot automation (`.github/dependabot.yml`).
- Use `blocked:*` only when the blocker prevents meaningful progress.
- Use `needs:*` for explicit human or environment validation needs.
