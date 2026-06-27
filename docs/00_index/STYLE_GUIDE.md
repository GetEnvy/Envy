# Documentation Style Guide (for humans + AI)

## Document types
- Tutorials: learning-oriented.
- How-to: task-oriented.
- Reference: factual, complete, no narrative.
- Explanation: why/how (design, trade-offs).

## Page template (top of file)
Add this metadata block at the top of each *developer* doc:

Status: draft|active|deprecated
Last updated: YYYY-MM-DD
Scope: <one sentence>
Source of truth: <code paths/spec>

## Writing rules
- Use meaningful headings; avoid back-to-back headings with no text.
- Use imperative verb headings for tasks ("Build", "Fix", "Verify").
- Keep paragraphs short; prefer lists.

## Protocol docs
- Always include: packet layout, bounds/caps, unknown-field strategy, and a "When it breaks" section with example log patterns.
- Link to fixtures/tests.
