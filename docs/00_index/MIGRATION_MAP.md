# Docs Reorganization Map

This file records the move from the old layout (`docs/user`, `docs/developer`) to the new layout.

## Key changes
- Root `docs/README.md` became `docs/00_index/README.md` (old preserved as `docs/00_index/README_old.md`).
- User docs moved to `docs/50_user/`.
- Developer docs split into:
  - `docs/10_dev/` (workflow, build, standards)
  - `docs/20_arch/` (architecture)
  - `docs/30_protocols/` (ED2K/Kad/BitTorrent)
  - `docs/40_quality/` (analysis, performance, security, testing)

## Follow-ups
- Update any relative links inside moved docs if broken.
- Consider reconciling inconsistent statements between ED2K/Kad reports (see `docs/00_index/KNOWN_INCONSISTENCIES.md`).
