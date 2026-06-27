# Known inconsistencies

- **ED2K/Kad scope:** `ED2K_KAD_GAP_ANALYSIS` describes historical gaps; `kad2-compatibility-report` focuses on wire-level Kad2 compatibility with eMule/aMule. Both are valid; canonical high-level status is `docs/10_dev/status.md`. No merge required.

- **EDClient.h comments:** Member `m_bEmSupportsSourceEx2` is commented "Not supported" but SourceEx2 (REQUESTSOURCES2/ANSWERSOURCES2) is implemented in `EDClient.cpp` and advertised; consider updating the comment to "Source Exchange v2".
