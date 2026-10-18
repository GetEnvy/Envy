# Review digest

Status: active
Last updated: 2026-10-18
Scope: The comment that lists a pull request's unresolved review threads.
Source of truth: `.github/scripts/review-digest.py`, `.github/workflows/review-digest.yml`

## What it does

The **Review digest** workflow keeps one comment per pull request that lists
the unresolved review threads on the current head, grouped as *To treat*,
*Nits* and *Outdated*, with counts and links. Agents and the maintainer read
that comment, or print it locally, instead of paging through the whole
conversation. It refreshes on pushes, submitted or dismissed reviews, and new
review comments; `workflow_dispatch` refreshes one PR on demand.

```bash
python3 .github/scripts/review-digest.py --pr <N> --print
```

`--print` lists every unresolved thread (resolved ones are only counted) and
writes nothing; it needs a token from
`GITHUB_TOKEN`, `GH_TOKEN` or `gh auth token` with GraphQL access.

## Limits

- Advisory only: it never requests reviews, approves, resolves, merges or
  sets a status. Thread classification is a heuristic; agents still read each
  thread before treating it.
- Thread text is rendered as inert code spans, and mentions and leading slash
  commands get a word joiner, so links, mentions and reviewer commands in
  findings act neither in the rendered digest nor in its raw text.
- Resolving or unresolving a thread does not refresh the digest: GitHub Actions
  has no trigger for thread state. The next push, review, review comment or
  manual dispatch refreshes it, and `--print` always shows live state.
- Only comments by `github-actions[bot]` or the repository owner are treated
  as the digest; a pasted marker is ignored.
- The workflow runs the default-branch script and never checks out PR code.
  Automatic events skip fork PRs, whose token is read-only; a maintainer's
  `workflow_dispatch` can still refresh one. A local `--print` runs the script
  in your own checkout.
