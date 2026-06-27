# Contributing

## Workflow
1. Create a feature branch from `develop` (default integration branch).
2. Keep changes scoped (docs, code, CI, etc.) and submit focused PRs.
3. Run relevant local checks before opening PR.
4. Use PR template and include risk/testing notes.

## Branch and Commit Strategy
- Prefer small, logical commits on feature branches.
- Rebase feature branches onto `origin/develop` before opening or updating a PR.
- Merge via **squash** (preferred) or **rebase** on GitHub; merge commits are disabled.
- Use `git pull --ff-only` on `develop`; never merge `develop` locally.
- Use imperative, specific commit messages.
- Separate refactors from behavior changes where possible.

See `.github/CONTRIBUTING.md` for the canonical linear-history workflow and exact sync commands.

## Code Style
- Follow existing C++ conventions in touched files.
- Avoid broad rewrites unless necessary.
- Keep security-sensitive changes explicit and reviewed.

## Pull Request Expectations
- Clear summary and rationale
- Test evidence or explicit environment limitation
- Backward-compatibility impact statement
- Documentation updates for behavior/process changes

## Review Checklist
- [ ] Build impacts understood
- [ ] Tests added/updated when feasible
- [ ] Security implications considered
- [ ] Docs and changelog updated
