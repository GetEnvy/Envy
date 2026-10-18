#!/usr/bin/env python3
"""Regression tests for the review digest (no network)."""

import importlib.util
from pathlib import Path
import unittest


SCRIPTS = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("review_digest", SCRIPTS / "review-digest.py")
DIGEST = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(DIGEST)


def thread(body, resolved=False, outdated=False, line=7):
    return {
        "isResolved": resolved,
        "isOutdated": outdated,
        "path": "Envy/Foo.cpp",
        "line": line,
        "author": "bot",
        "body": body,
        "url": "https://example.invalid/t",
    }


class DigestTests(unittest.TestCase):
    def test_digest_separates_blocking_nits_outdated_and_resolved(self):
        threads = [
            thread("_⚠️ Potential issue_\n\nBuffer overflow when nLength is 0"),
            thread("**nitpick:** rename variable"),
            thread("old finding", outdated=True),
            thread("fixed", resolved=True),
        ]
        body = DIGEST.render_digest(12, "e" * 40, threads)
        self.assertIn("**1 to treat** | 1 nits | 1 outdated | 1 resolved", body)
        self.assertIn("Buffer overflow when nLength is 0", body.split("<details>")[0])
        self.assertIn("rename variable", body)
        self.assertTrue(body.startswith(DIGEST.DIGEST_MARKER))

    def test_digest_keeps_thread_text_inert(self):
        body = DIGEST.render_digest(1, "f" * 40, [
            thread("@coderabbitai review https://evil.invalid [here](https://evil.invalid)"),
            thread("use `rm -rf` /gemini review"),
        ])
        # Code spans: no autolink, mention, command, or link renders; a word
        # joiner also breaks the mention and command in the raw comment text.
        self.assertIn("`@\u2060coderabbitai review https://evil.invalid [here](https://evil.invalid)`", body)
        self.assertIn("`use 'rm -rf' /\u2060gemini review`", body)

    def test_inline_label_keeps_finding_text(self):
        self.assertEqual(DIGEST.summarize("Potential issue: buffer overflow"),
                         "Potential issue: buffer overflow")

    def test_not_a_nit_stays_a_finding(self):
        self.assertFalse(DIGEST.is_nit("This is not a nit: the loop overflows."))
        self.assertTrue(DIGEST.is_nit("_🧹 Nitpick_\n\nRename the variable."))
        self.assertFalse(DIGEST.is_nit("Overflow.\n\nDetails.\n\nUnrelated nit at the end."))

    def test_digest_lists_overflow_note_and_print_lists_all(self):
        threads = [thread(f"finding {i}") for i in range(DIGEST.MAX_LISTED_THREADS + 3)]
        capped = DIGEST.render_digest(1, "f" * 40, threads)
        self.assertIn("3 more not listed", capped)
        full = DIGEST.render_digest(1, "f" * 40, threads, limit=None)
        self.assertNotIn("more not listed", full)
        self.assertIn(f"finding {DIGEST.MAX_LISTED_THREADS + 2}", full)

    def test_summary_strips_markup_and_truncates(self):
        text = "<!-- meta -->\n![high](x.png)\n**issue (bug_risk):** " + "x" * 200
        summary = DIGEST.summarize(text)
        self.assertTrue(summary.startswith("issue (bug_risk):"))
        self.assertLessEqual(len(summary), 140)


class CubicFindingsTests(unittest.TestCase):
    def test_short_finding_after_a_label_word_is_kept(self):
        self.assertEqual(DIGEST.summarize("Minor typo in docstring"), "Minor typo in docstring")
        badge = "**Functional Correctness** | **Minor** | **Quick win**\n\nFix the bound."
        self.assertEqual(DIGEST.summarize(badge), "Fix the bound.")
        self.assertEqual(DIGEST.summarize("Potential issue\n\nBuffer overflow."), "Buffer overflow.")
        self.assertEqual(DIGEST.summarize("Severity: MEDIUM\n\nLeak."), "Leak.")

    def test_nit_needs_an_opening_label(self):
        self.assertFalse(DIGEST.is_nit("The nit is that this buffer overflows."))
        self.assertTrue(DIGEST.is_nit("**nitpick:** rename variable"))
        self.assertTrue(DIGEST.is_nit("Nitpick comments (2)\n\n- rename"))
        # CodeRabbit badges glue an underscore to the label.
        self.assertTrue(DIGEST.is_nit("_\U0001F9F9 Nitpick_ | _\U0001F535 Trivial_\n\nRename it."))
        self.assertEqual(DIGEST.summarize("_Potential issue_ | _Major_\n\nFix the bound."),
                         "Fix the bound.")

    def test_cpp_scopes_survive_shortcode_stripping(self):
        self.assertEqual(DIGEST.summarize(":warning: use std::vector::push_back here"),
                         "use std::vector::push_back here")

    def test_raw_text_cannot_carry_mentions_or_commands(self):
        text = DIGEST.inert("@coderabbitai review then /gemini review")
        self.assertNotIn("@coderabbitai", text)
        self.assertNotIn(" /gemini", text)
        self.assertIn("coderabbitai review", text)


if __name__ == "__main__":
    unittest.main()
