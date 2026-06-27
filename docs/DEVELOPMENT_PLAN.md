# DEVELOPMENT PLAN (LIVING)

> **LIVING DOCUMENT** — Must be updated after every meaningful change (feature, architectural decision, scope change, blocker resolution).

- **Last Updated:** 2026-05-27
- **Changelog Entry:** 2026-05-27 — Documented linear-history workflow for `develop`: squash/rebase merges only, `git pull --ff-only`, feature-branch rebase commands; aligned `.github/settings.yml` with GitHub merge settings.
- **Changelog Entry:** 2026-05-17 — Improved CodeQL C# analysis precision by introducing a dedicated manual-build workflow and documenting legacy FictionBookReader build blockers plus minimal .NET Framework 4.8 retarget path.
- **Changelog Entry:** 2026-05-15 — Synced repository hygiene status for `develop`: documented branch state, CI gate maturity, Dependabot labels requirement, and dependency register status (`docs/DEPENDENCIES.md` exists but remains an incomplete seed).
- **Changelog Entry:** 2026-05-15 — Hardened legacy Kad publish packet construction by replacing unsafe keyword copy with bounded copy and explicit terminator in `KadProtocol::CreatePublishRequest`, preserving wire format.

- **Changelog Entry:** 2026-05-15 — ED2K Source Exchange hardening: added shared bounds validation for SourceEx/SourceEx2 source lists, modernized SourceEx2 request length handling, and documented current IPv4-only SourceEx wire limitation.

## Update Protocol
1. Update **Last Updated** date on every meaningful change.
2. Add a one-line entry to the changelog section above.
3. Reflect status changes in **Current Status** and **Roadmap**.
4. Record consequential technical decisions in **Decisions Log**.
5. Close or refresh **Open Questions** explicitly.


## Repository Status (develop)
- Default branch is `develop`.
- `main` is currently behind `develop`.
- **Merge policy (GitHub):** merge commits disabled; squash and rebase merges enabled. Prefer squash for PRs.
- **History:** `develop` was rewritten to a linear history with no merge commits; the old tree is preserved in the `backup/develop-before-linear-rewrite` branch (and tag of the same name) created before the rewrite.
- **Local hygiene:** use `git pull --ff-only` on `develop`; rebase feature branches with `git rebase origin/develop` and `git push --force-with-lease`.
- **Branch protection:** the active `Protect develop` ruleset requires pull requests, linear history, passing checks, and blocks force-pushes/deletions. `.github/settings.yml` mirrors the intended policy for Probot Settings or manual audits.
- CI workflows exist, but should not yet be treated as mandatory merge gates until required checks are consistently emitted and stable in GitHub Actions.
- Dependabot expects GitHub labels `ci` and `dependencies` to exist for automated PR labeling.


## Canonical Documentation Split
- `docs/DEVELOPMENT_PLAN.md`: strategic roadmap, major decisions, and sequencing.
- `docs/DEV_TRACKER.md`: operational dashboard, near-term status, blockers, and PR queue.
- `docs/10_dev/status.md`: deep protocol comparison and implementation evidence.
- `docs/10_dev/roadmap.md`: technical modernization roadmap details.

## Vision & Goals
- Maintain Envy as a stable multi-network P2P client for Windows.
- Reduce modernization risk by improving testability and dependency hygiene.
- Increase release confidence through clearer architecture boundaries and measurable quality gates.

## Current Status
### Done
- CI workflows for build/quality/security exist.
- Hash-focused unit tests integrated in repo and workflows.
- Audit and core documentation baseline established.
- Remote CRITICAL/HIGH security items remediated (CSRF, XSS sanitization, CSP hardening, redirects, rate limiter, API input validation).
- Remote JS security regression tests wired into `code-quality.yml`.

### In Progress
- C++ modernization across legacy modules.
- Incremental protocol compatibility and robustness improvements.

### Blocked / At Risk
- Full CMake parity with Visual Studio build graph.
- Dependency refresh for older vendored components without regressions.

## Roadmap

### Phase 1 — Stability & Visibility (P0)
- [ ] Create dependency register + ownership map (2d) — **In progress on develop** (`docs/DEPENDENCIES.md` exists but remains an incomplete seed).
- [x] Add threat model and secure-coding checklist (2d)
- [ ] Expand tests for protocol parser/state-machine paths (5d)
- [ ] Establish baseline metrics (startup, memory, throughput) (3d)

### Phase 2 — Build/Quality Convergence (P1)
- [ ] Define CMake migration boundary and milestones (3d)
- [ ] Reduce duplicated CI workflow logic (2d)
- [ ] Promote selected static-analysis checks to required gates (2d)

### Phase 3 — Architecture Hardening (P2)
- [ ] Isolate core transfer engine interfaces from UI classes (10d)
- [ ] Version plugin-facing APIs and compatibility policy (5d)
- [ ] Create automated dependency/SBOM release artifact (3d)

## Backlog
- [ ] Replace unsafe string operations in first-party code (incremental: bounded keyword copy in legacy Kad publish packet builder completed)
- Consolidate duplicate roadmap/status markdown into canonical set
- Document remote API implementation status endpoint-by-endpoint
- Add long-running memory/regression test scenario
- Archive legacy `.vcproj` files once migration is complete

## Decisions Log
- **2026-05-27:** Rewrote `develop` into a linear history with no merge commits while preserving the final tree through backup refs; enforce linear history going forward via the active `Protect develop` ruleset, GitHub merge settings (no merge commits; squash/rebase only), and contributor `git pull --ff-only` hygiene.
- **2026-05-15:** Repository hygiene baseline on `develop` requires explicit branch-state tracking and GitHub label prerequisites (`ci`, `dependencies`) before enforcing CI as mandatory gates.
- **2026-04-22:** Added IPv6 dual-stack Phase 0 scoping inventory and phased rollout plan under `docs/ipv6/`.
- **2026-04-22:** Remote web UI must use cryptographic token generation (`crypto.getRandomValues`) and allowlist-based redirect validation for all client-side navigation paths.
- **2026-04-22:** Keep Visual Studio solution as authoritative full-build path while CMake remains partial.
- **2026-04-22:** Standardize new audit reports under `docs/audit/`.
- **2026-04-22:** Treat this plan as a required living artifact for project management continuity.

## Open Questions
1. Should this project explicitly remain Windows-only, or is cross-platform parity still a target?
2. What is the acceptable backward-compatibility policy for legacy protocols/features?
3. Which dependency update cadence (monthly/quarterly) is realistic for maintainers?
4. Should remote API documentation be strict contract-first or implementation-first?
