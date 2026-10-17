## Summary
- What changed?
- Why now?

## Scope
- [ ] Code
- [ ] Docs
- [ ] CI/Tooling
- [ ] Security
- [ ] Dependencies

## Risk (classifier: low / normal / high)
- Wire-format impact: none / see validation
- Breaking changes:
- Security impact:

## Validation
List exact commands/checks run and outcomes.

### Runtime / interoperability (when needed)
Use `stage:live-test` only when manual Envy runtime, installer, UI, or interop
validation is required. Docs-only, workflow-only, test-only, and safe mechanical
PRs do not need a live ENVY run.

- Tested HEAD / artifact (if applicable):
- [ ] Manual runtime validation performed (only when this PR needs it)

## Merge notes
- [ ] Required deterministic CI green for applicable paths
- [ ] Review threads resolved
- [ ] Governance/ruleset PRs: maintainer will apply ruleset patch after merge (no auto-merge on this PR)

## Squash Commit Summary
Paste this into the GitHub squash **Extended description** at merge
(`AGENTS.md` rule 16). Keep it short; do not copy the full PR body.

Issue reference: `Fixes #...` / `Closes #...` / `Related to #...` / `None`

- Root problem:
- Resulting behavior:
- Compatibility / security / reliability (if any):
- Regression coverage (if any):

## Checklist
- [ ] Changes are scoped and reviewable
- [ ] No personal emails; no forged bot identities; avoid AI promo signatures (`AGENTS.md` rule 16)
- [ ] Tests added/updated where practical
- [ ] Documentation updated when behavior or policy changed
- [ ] Changelog updated for user-visible changes
