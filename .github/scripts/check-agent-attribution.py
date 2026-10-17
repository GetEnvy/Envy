#!/usr/bin/env python3
"""Scan PR git metadata and added text for authorship/privacy policy.

Severity model:
  FAIL — privacy leaks, forged/malformed identity, scanner integrity failures
  WARN — editorial conventions, unsigned commits, weak automation provenance
  PASS — human noreply, trusted automation with established provenance

Positive allowlist only. Do not maintain a denylist of personal addresses.
Git author/committer fields are forgeable; GitHub verification and noreply
linkage raise confidence but are not perfect authentication.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

SEVERITY_FAIL = "FAIL"
SEVERITY_WARN = "WARN"

AI_TOOL_NAME_RE = re.compile(
    r"(?i)(Cursor(?:\s+Agent)?|cursoragent|GitHub\s+Copilot|copilot-swe-agent|"
    r"Copilot|Codex|Claude|ChatGPT|OpenAI|Anthropic|CodeRabbit|cubic\.dev|"
    r"Cubic|Aider|Cline|Windsurf|Sourcery|Gemini|Grok|Amazon\s+Q(?:\s+Developer)?)"
)
_GENERATED_PREFIX = (
    r"(?:Generated(?:[ \t]+(?:with|by)|-by)|Created[ \t]+with|AI-generated|Made-with)"
)
_GENERATED_TOOL = (
    r"(Cursor(?:[ \t]+Agent)?|Copilot|Codex|Claude|ChatGPT|OpenAI|Aider|"
    r"CodeRabbit|Cline|Windsurf|Anthropic|Cubic|Sourcery|Gemini|Grok|Amazon[ \t]+Q(?:[ \t]+Developer)?)"
)
GENERATED_LINE_RE = re.compile(
    rf"(?im)^[ \t]*(?://|/\*|\*)?[ \t]*{_GENERATED_PREFIX}[ \t:]+{_GENERATED_TOOL}\b"
)
GENERATED_DIFF_TEXT_RE = re.compile(
    rf"(?i)\b{_GENERATED_PREFIX}[ \t:]+{_GENERATED_TOOL}\b"
)
TRAILER_RE = re.compile(
    r"(?im)^(Co-authored-by|Signed-off-by|Reviewed-by|Acked-by|Reported-by|"
    r"Tested-by|Helped-by|Suggested-by)[ \t]*:[ \t]*(.+)$"
)
EMAIL_RE = re.compile(r"(?i)\b([A-Z0-9._%+\-]+)@([A-Z0-9.\-]+\.[A-Z]{2,})\b")
URL_USERINFO_RE = re.compile(r"(?i)[a-z][a-z0-9+\-.]*://[^\s/]*@")
PROCESS_COMMENT_RE = re.compile(
    r"(?i)^\+[ \t]*(//|/\*|\*)[ \t].*(Requested by Copilot|Cursor changed this|"
    r"CodeRabbit complained|Generated with Cursor|Generated with Copilot)"
)
CONTRIBUTOR_AGENT_MARKER_RE = re.compile(
    r"(?i)<!--\s*(CURSOR_AGENT_|CODEX_|COPILOT_)[A-Z0-9_-]*\s*-->"
)
AUTOMATED_PR_SECTION_RES = (
    re.compile(
        r"<!--\s*This is an auto-generated description by cubic\.\s*-->"
        r".*?<!--\s*End of auto-generated description by cubic\.\s*-->",
        re.DOTALL | re.IGNORECASE,
    ),
    re.compile(
        r"<!--\s*CURSOR_AGENT_PR_BODY_BEGIN\s*-->.*?<!--\s*CURSOR_AGENT_PR_BODY_END\s*-->",
        re.DOTALL | re.IGNORECASE,
    ),
    re.compile(
        r"<!--\s*This is an auto-generated comment by CodeRabbit\s*-->.*?<!--\s*End of auto-generated comment by CodeRabbit\s*-->",
        re.DOTALL | re.IGNORECASE,
    ),
)
# GitHub / Actions mailboxes only.
ALLOWED_TECHNICAL_EMAILS = {
    "noreply@github.com",
    "support@github.com",
    "notifications@github.com",
    "security@github.com",
}
ALLOWED_PROJECT_EMAILS: set[str] = set()
# Known automation runners that are not GitHub App [bot] noreply identities.
# Provenance is established by GitHub commit verification when available.
TRUSTED_RUNNER_EMAILS = {
    "cursoragent@cursor.com",
}
GIT_FULL_SHA_RE = re.compile(r"^[0-9a-fA-F]{40}$")
GH_API_LIST_PATH_RE = re.compile(
    r"^repos/(?P<owner>[A-Za-z0-9_.-]+)/(?P<repo>[A-Za-z0-9_.-]+)/"
    r"(?P<kind>issues|pulls)/(?P<number>[0-9]+)/(?P<tail>comments|reviews)$"
)
GH_COMMIT_PATH_RE = re.compile(
    r"^repos/[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+/commits/[0-9a-fA-F]{40}$"
)
GITHUB_REPO_RE = re.compile(r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")
BOT_NOREPLY_RE = re.compile(
    r"(?i)^(?P<id>\d+)\+(?P<login>[^@]+)@users\.noreply\.github\.com$"
)

# Explicit allowlist of automated GitHub actors. Matching git name/email alone
# is not cryptographic proof; prefer verified signatures when present.
TRUSTED_BOT_LOGINS = {
    "dependabot[bot]",
    "github-actions[bot]",
    "copilot-pull-request-reviewer[bot]",
    "copilot-swe-agent[bot]",
    "cursor[bot]",
    "coderabbitai[bot]",
    "cubic-dev-ai[bot]",
    "sonarcloud[bot]",
    "snyk-bot",
    "amazon-q-developer[bot]",
    "sourcery-ai[bot]",
    "github-copilot[bot]",
    "renovate[bot]",
}

BOT_COMMENT_LOGINS = set(TRUSTED_BOT_LOGINS) | {
    "copilot-pull-request-reviewer",
    "copilot-swe-agent",
}


@dataclass
class Finding:
    severity: str
    message: str


def is_allowed_email(email: str) -> bool:
    e = email.strip().lower()
    if e in ALLOWED_TECHNICAL_EMAILS or e in ALLOWED_PROJECT_EMAILS:
        return True
    if e in TRUSTED_RUNNER_EMAILS:
        return True
    return e.endswith(("@users.noreply.github.com", "@noreply.github.com"))


def looks_like_ai_tool_name(name: str) -> bool:
    return bool(AI_TOOL_NAME_RE.search(name or ""))


def parse_bot_noreply(email: str) -> str | None:
    """Return GitHub App bot login from noreply mail, else None.

    Human noreply also uses ``{id}+{login}@users.noreply.github.com``. Only
    treat addresses whose login contains ``[bot]`` (or known bot slugs) as
    automation noreply identities.
    """
    match = BOT_NOREPLY_RE.fullmatch(email.strip())
    if not match:
        return None
    login = match.group("login").lower()
    if "[bot]" in login or login in {item.lower() for item in TRUSTED_BOT_LOGINS}:
        return login
    return None


def is_trusted_bot_identity(name: str, email: str) -> bool:
    login = parse_bot_noreply(email)
    if not login:
        return False
    if login not in {item.lower() for item in TRUSTED_BOT_LOGINS}:
        return False
    name_l = name.strip().lower()
    return name_l == login or name_l == login.removesuffix("[bot]")


def is_trusted_runner_email(email: str) -> bool:
    return email.strip().lower() in TRUSTED_RUNNER_EMAILS


# Cap per-job GitHub commit lookups when --github-repo is set (rate limits / time).
MAX_PROVENANCE_API_CALLS = 64


def trusted_runner_github_logins(email: str) -> set[str]:
    """GitHub logins that may corroborate an allowlisted runner mailbox."""
    local = email.strip().lower().split("@", 1)[0]
    return {local}


DOC_OR_REMOTE_DOMAINS = {
    "example.com",
    "example.org",
    "example.net",
    "example.edu",
    "invalid",
    "test",
    "localhost",
}


NATIVE_MODULE_SUFFIXES = (".dll", ".exe", ".sys", ".ocx", ".drv", ".cpl")


def emails_in_text(text: str) -> list[str]:
    # Drop URL userinfo matches so https://user@host/path is not treated as mail.
    masked = URL_USERINFO_RE.sub(" ", text)
    found = []
    for match in EMAIL_RE.finditer(masked):
        email = match.group(0)
        domain = match.group(2).lower()
        # git@github.com:org/repo.git is an SCP-style remote, not a mailbox.
        if email.lower() == "git@github.com":
            continue
        # Func@<module>.dll is a native import spec (Inno Setup `external`,
        # linker .def files), not a mailbox; no such TLDs exist.
        if domain.endswith(NATIVE_MODULE_SUFFIXES):
            continue
        if domain in DOC_OR_REMOTE_DOMAINS or domain.endswith(".example.com"):
            continue
        found.append(email)
    return found


def require_full_sha(value: str, label: str) -> str:
    value = value.strip()
    if not GIT_FULL_SHA_RE.fullmatch(value):
        raise ValueError(f"{label} must be a 40-character git object id")
    return value.lower()


def normalize_to_sha(to_sha: str) -> str:
    to_sha = to_sha.strip()
    if to_sha == "HEAD":
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"], text=True, errors="replace"
        ).strip()
    return require_full_sha(to_sha, "to")


def git_diff(parent: str, child: str) -> str:
    parent = require_full_sha(parent, "parent")
    child = require_full_sha(child, "child")
    return subprocess.check_output(
        ["git", "diff", "-U0", parent, child],
        text=True,
        errors="replace",
    )


def git_rev_list_range(from_sha: str, to_sha: str) -> list[str]:
    start = require_full_sha(from_sha, "from")
    end = require_full_sha(to_sha, "to")
    out = subprocess.check_output(
        ["git", "rev-list", f"{start}..{end}"],
        text=True,
        errors="replace",
    )
    return [line.strip() for line in out.splitlines() if line.strip()]


def git_rev_parse_parent(commit: str) -> str:
    commit = require_full_sha(commit, "commit")
    return subprocess.check_output(
        ["git", "rev-parse", f"{commit}^"],
        text=True,
        errors="replace",
    ).strip()


def git_log_field(commit: str, fmt: str) -> str:
    commit = require_full_sha(commit, "commit")
    allowed = {"%ae", "%an", "%ce", "%cn", "%B"}
    if fmt not in allowed:
        raise RuntimeError(f"unsupported git log format: {fmt}")
    return subprocess.check_output(
        ["git", "log", "-1", f"--format={fmt}", commit],
        text=True,
        errors="replace",
    ).strip()


def read_bounded_input_file(path_str: str, label: str, findings: list[Finding]) -> str | None:
    if "\0" in path_str:
        report(findings, SEVERITY_FAIL, f"{label}: invalid path")
        return None
    path = Path(path_str)
    if any(part == ".." for part in path.parts):
        report(findings, SEVERITY_FAIL, f"{label}: path traversal is not allowed")
        return None
    try:
        resolved = path.expanduser().resolve(strict=False)
    except OSError:
        report(findings, SEVERITY_FAIL, f"{label}: invalid path")
        return None
    if not resolved.is_file():
        report(findings, SEVERITY_FAIL, f"{label} file not found: {resolved}")
        return None
    return resolved.read_text(encoding="utf-8", errors="replace")


def report(findings: list[Finding], severity: str, msg: str) -> None:
    line = f"{severity}: {msg}"
    print(line, file=sys.stderr)
    if os.environ.get("ATTRIBUTION_CI_ANNOTATE") == "1":
        ann = "error" if severity == SEVERITY_FAIL else "warning"
        print(f"::{ann}::{msg}", file=sys.stderr)
    findings.append(Finding(severity, msg))


def strip_automated_pr_sections(text: str) -> str:
    """Remove known third-party PR summary blocks (bots), not contributor prose."""
    out = text
    for pattern in AUTOMATED_PR_SECTION_RES:
        out = pattern.sub("", out)
    return out


def _reject_email(
    findings: list[Finding], message: str, email: str, *, allow_technical: bool
) -> None:
    if allow_technical and email.lower() in ALLOWED_TECHNICAL_EMAILS:
        return
    if not is_allowed_email(email):
        report(findings, SEVERITY_FAIL, message)


def _scan_trailers(findings: list[Finding], label: str, text: str) -> None:
    for trailer, value in TRAILER_RE.findall(text):
        emails = emails_in_text(value)
        if trailer.lower() == "co-authored-by" and looks_like_ai_tool_name(value):
            report(
                findings,
                SEVERITY_WARN,
                f"{label}: AI-tool Co-authored-by trailer is discouraged "
                "(tools are not Git co-authors).",
            )
        # Privacy applies to every trailer, including AI-named ones.
        for email in emails:
            _reject_email(
                findings,
                f"{label}: attribution trailer {trailer} uses a non-allowlisted email.",
                email,
                allow_technical=False,
            )


def scan_text(
    findings: list[Finding],
    label: str,
    text: str,
    *,
    allow_ai_author_email: bool = False,
    privacy_text: str | None = None,
) -> None:
    privacy_source = privacy_text if privacy_text is not None else text
    if GENERATED_LINE_RE.search(text):
        report(
            findings,
            SEVERITY_WARN,
            f"{label}: AI-tool generation signature is discouraged.",
        )
    _scan_trailers(findings, label, privacy_source)
    for email in emails_in_text(privacy_source):
        _reject_email(
            findings,
            f"{label}: non-allowlisted email address in generated git/GitHub text.",
            email,
            allow_technical=allow_ai_author_email,
        )


def scan_pr_text(findings: list[Finding], label: str, text: str) -> None:
    """Scan contributor-authored PR text; ignore recognized bot summary blocks."""
    contributor = strip_automated_pr_sections(text)
    if contributor != text:
        report(
            findings,
            SEVERITY_WARN,
            f"{label}: bot auto-summary delimiters in contributor text are not "
            "trusted; privacy checks still apply to the full body.",
        )
    if CONTRIBUTOR_AGENT_MARKER_RE.search(contributor):
        report(
            findings,
            SEVERITY_WARN,
            f"{label}: agent automation marker in contributor text is discouraged.",
        )
    # Editorial signatures may ignore stripped bot blocks; privacy must not.
    scan_text(findings, label, contributor, privacy_text=text)


def scan_pr_title(findings: list[Finding], label: str, text: str) -> None:
    """Scan PR title: technical tool names are allowed; privacy/promo still checked."""
    title = strip_automated_pr_sections(text).strip()
    if not title:
        return
    if CONTRIBUTOR_AGENT_MARKER_RE.search(title):
        report(
            findings,
            SEVERITY_WARN,
            f"{label}: agent automation marker in contributor text is discouraged.",
        )
    # Tool names in titles (e.g. "fix Copilot review classification") are OK,
    # but a signature after a "type(scope):" prefix is still promotional.
    if GENERATED_DIFF_TEXT_RE.search(title) and not GENERATED_LINE_RE.search(title):
        report(
            findings,
            SEVERITY_WARN,
            f"{label}: AI tool generation signature in contributor text is discouraged.",
        )
    scan_text(findings, label, title)


@dataclass
class CommitGithubMeta:
    author_login: str = ""
    author_type: str = ""
    verified: bool | None = None
    reason: str = ""
    committer_login: str = ""
    committer_type: str = ""

    def for_committer(self, author_email: str = "", committer_email: str = "") -> "CommitGithubMeta":
        """Provenance as seen by the committer identity check."""
        login = self.committer_login
        utype = self.committer_type
        if not login and author_email and committer_email:
            if author_email.strip().lower() == committer_email.strip().lower():
                login = self.author_login
                utype = self.author_type
        return CommitGithubMeta(
            author_login=login,
            author_type=utype,
            verified=self.verified,
            reason=self.reason,
        )


def classify_identity(
    name: str,
    email: str,
    *,
    meta: CommitGithubMeta | None = None,
    require_provenance_link: bool = False,
) -> list[tuple[str, str]]:
    """Return (severity, message) pairs for a git author/committer identity."""
    out: list[tuple[str, str]] = []
    email = email.strip()
    name = name.strip()
    if not email:
        out.append((SEVERITY_FAIL, "missing email."))
        return out

    meta = meta or CommitGithubMeta()
    bot_login = parse_bot_noreply(email)
    trusted_bot = is_trusted_bot_identity(name, email)
    trusted_runner = is_trusted_runner_email(email)
    name_l = name.strip().lower()
    trusted_logins = {item.lower() for item in TRUSTED_BOT_LOGINS}

    if trusted_bot:
        if require_provenance_link or meta.author_login:
            if not meta.author_login:
                out.append(
                    (
                        SEVERITY_FAIL,
                        "trusted automation identity without linked GitHub author login.",
                    )
                )
                return out
            if meta.author_login.lower() != bot_login:
                out.append(
                    (
                        SEVERITY_FAIL,
                        "claimed trusted bot identity does not match GitHub author login "
                        f"(git={bot_login}, github={meta.author_login}).",
                    )
                )
                return out
            if meta.author_type and meta.author_type != "Bot" and not meta.author_login.lower().endswith(
                "[bot]"
            ):
                out.append(
                    (
                        SEVERITY_WARN,
                        "trusted bot noreply email is linked to a non-Bot GitHub account; "
                        "treat provenance cautiously.",
                    )
                )
        # Identity format accepted. Signature handled separately.
        return out

    # Display name claims a trusted bot, but mailbox login does not match.
    if name_l in trusted_logins and (bot_login is None or bot_login != name_l):
        out.append(
            (
                SEVERITY_FAIL,
                "git display name claims a trusted bot but the email login does not match "
                "(possible impersonation).",
            )
        )
        return out

    if trusted_runner:
        if looks_like_ai_tool_name(name) or name_l in {"cursor agent", "cursoragent"}:
            expected = trusted_runner_github_logins(email)
            login = meta.author_login.lower() if meta.author_login else ""
            if require_provenance_link or meta.author_login or meta.verified is not None:
                if not login:
                    out.append(
                        (
                            SEVERITY_FAIL,
                            "trusted automation runner identity without linked GitHub "
                            "author login.",
                        )
                    )
                    return out
                if login not in expected:
                    out.append(
                        (
                            SEVERITY_FAIL,
                            "trusted automation runner identity does not match the "
                            "GitHub account linked to its mailbox "
                            f"(github={meta.author_login}).",
                        )
                    )
                    return out
                if meta.verified is not True:
                    out.append(
                        (
                            SEVERITY_WARN,
                            "trusted automation runner identity without verified GitHub "
                            "commit signature; prefer a human GitHub noreply identity.",
                        )
                    )
                    return out
                return out
            out.append(
                (
                    SEVERITY_WARN,
                    "trusted automation runner identity without verified GitHub "
                    "commit signature; prefer a human GitHub noreply identity.",
                )
            )
            return out
        out.append(
            (
                SEVERITY_WARN,
                "automation runner email used with unexpected display name.",
            )
        )
        return out

    if bot_login is not None:
        # Plausible GitHub App noreply, but not on the explicit allowlist.
        out.append(
            (
                SEVERITY_WARN,
                f"unknown automation noreply login '{bot_login}' is not on the "
                "trusted-bot allowlist; independent review recommended.",
            )
        )
        return out

    if name_l.endswith("[bot]") or looks_like_ai_tool_name(name):
        # Claimed automation/AI identity without trusted noreply/runner email.
        if is_allowed_email(email) and email.lower().endswith("@users.noreply.github.com"):
            if not name_l.endswith("[bot]"):
                # A tool word alone is not proof of automation: human display
                # names such as "Claude Martin" match it. The noreply mailbox
                # still belongs to a human account, so flag it for review only.
                out.append(
                    (
                        SEVERITY_WARN,
                        "display name matches an AI tool on a human GitHub noreply "
                        "identity; confirm it is a person, not an agent.",
                    )
                )
                return out
            out.append(
                (
                    SEVERITY_FAIL,
                    "[bot] git identity must use a trusted automation mailbox "
                    "(GitHub App noreply or allowlisted runner), not a generic "
                    "human noreply spoof.",
                )
            )
            return out
        out.append(
            (
                SEVERITY_FAIL,
                "AI-tool or [bot] git identity is not on the trusted automation allowlist.",
            )
        )
        return out

    if not is_allowed_email(email):
        out.append((SEVERITY_FAIL, "non-allowlisted git identity email."))
    return out


def classify_signature(meta: CommitGithubMeta) -> tuple[str, str] | None:
    if meta.verified is True:
        return None
    if meta.verified is False:
        if meta.reason in {"unsigned", "unverified_email", "unknown_signature_type"}:
            return (
                SEVERITY_WARN,
                f"commit signature not verified (reason={meta.reason or 'unknown'}); "
                "signatures are optional on branch commits under the live ruleset.",
            )
        return (
            SEVERITY_WARN,
            f"commit signature present but not verifiable (reason={meta.reason or 'unknown'}).",
        )
    return None


def scan_identity(
    findings: list[Finding],
    label: str,
    name: str,
    email: str,
    *,
    meta: CommitGithubMeta | None = None,
    require_provenance_link: bool = False,
) -> None:
    for severity, msg in classify_identity(
        name,
        email,
        meta=meta,
        require_provenance_link=require_provenance_link,
    ):
        report(findings, severity, f"{label}: {msg}")


def load_commit_meta_file(path_str: str, findings: list[Finding]) -> dict[str, CommitGithubMeta]:
    text = read_bounded_input_file(path_str, "commit meta", findings)
    if text is None:
        return {}
    try:
        raw = json.loads(text)
    except json.JSONDecodeError:
        report(findings, SEVERITY_FAIL, "commit meta: invalid JSON")
        return {}
    if not isinstance(raw, dict):
        report(findings, SEVERITY_FAIL, "commit meta: expected object keyed by full SHA")
        return {}
    out: dict[str, CommitGithubMeta] = {}
    for sha, value in raw.items():
        try:
            key = require_full_sha(str(sha), "commit meta key")
        except ValueError as exc:
            report(findings, SEVERITY_FAIL, str(exc))
            continue
        if not isinstance(value, dict):
            report(findings, SEVERITY_FAIL, f"commit meta {key[:12]}: expected object")
            continue
        verified = value.get("verified")
        if verified is not None and not isinstance(verified, bool):
            report(findings, SEVERITY_FAIL, f"commit meta {key[:12]}: verified must be bool")
            continue
        out[key] = CommitGithubMeta(
            author_login=str(value.get("author_login") or ""),
            author_type=str(value.get("author_type") or ""),
            verified=verified,
            reason=str(value.get("reason") or ""),
            committer_login=str(value.get("committer_login") or ""),
            committer_type=str(value.get("committer_type") or ""),
        )
    return out


def gh_api_json(url: str) -> dict:
    url = url.strip()
    if not GH_COMMIT_PATH_RE.fullmatch(url):
        raise ValueError(f"unsupported GitHub API commit path: {url}")
    raw = subprocess.check_output(["gh", "api", url], text=True)
    obj = json.loads(raw)
    if not isinstance(obj, dict):
        raise RuntimeError(f"unexpected JSON type for {url}")
    return obj


def fetch_commit_github_meta(repo: str, sha: str) -> CommitGithubMeta:
    data = gh_api_json(f"repos/{repo}/commits/{sha}")
    author = data.get("author") or {}
    committer = data.get("committer") or {}
    verification = (data.get("commit") or {}).get("verification") or {}
    verified = verification.get("verified")
    return CommitGithubMeta(
        author_login=str(author.get("login") or ""),
        author_type=str(author.get("type") or ""),
        verified=verified if isinstance(verified, bool) else None,
        reason=str(verification.get("reason") or ""),
        committer_login=str(committer.get("login") or ""),
        committer_type=str(committer.get("type") or ""),
    )


def needs_provenance(name: str, email: str) -> bool:
    """Identities accepted only after GitHub links them to the claimed account."""
    return is_trusted_bot_identity(name, email) or is_trusted_runner_email(email)


def scan_commits(
    findings: list[Finding],
    from_sha: str,
    to_sha: str,
    *,
    github_repo: str | None = None,
    meta_by_sha: dict[str, CommitGithubMeta] | None = None,
) -> None:
    try:
        commits = git_rev_list_range(from_sha, to_sha)
    except ValueError as exc:
        report(findings, SEVERITY_FAIL, str(exc))
        return
    meta_by_sha = dict(meta_by_sha or {})
    provenance_api_calls = 0
    for commit in commits:
        email = git_log_field(commit, "%ae")
        name = git_log_field(commit, "%an")
        cemail = git_log_field(commit, "%ce")
        cname = git_log_field(commit, "%cn")
        body = git_log_field(commit, "%B")
        short = commit[:12]
        meta = meta_by_sha.get(commit)
        needs_link = needs_provenance(name, email) or needs_provenance(cname, cemail)
        provenance_loaded = meta is not None
        if meta is None and github_repo and needs_link:
            if provenance_api_calls >= MAX_PROVENANCE_API_CALLS:
                report(
                    findings,
                    SEVERITY_FAIL,
                    f"commit {short}: exceeded GitHub provenance lookup budget "
                    f"({MAX_PROVENANCE_API_CALLS} commits).",
                )
                meta = CommitGithubMeta()
            else:
                provenance_api_calls += 1
                try:
                    meta = fetch_commit_github_meta(github_repo, commit)
                    meta_by_sha[commit] = meta
                    provenance_loaded = True
                except (ValueError, RuntimeError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
                    # Trusted automation is only accepted with provenance; without it
                    # a forged bot or runner identity would pass unchecked.
                    report(
                        findings,
                        SEVERITY_FAIL,
                        f"commit {short}: could not load GitHub provenance ({exc}).",
                    )
                    meta = CommitGithubMeta()
        meta = meta or CommitGithubMeta()
        require_link = provenance_loaded and needs_link
        scan_identity(
            findings,
            f"commit {short} author",
            name,
            email,
            meta=meta,
            require_provenance_link=require_link,
        )
        scan_identity(
            findings,
            f"commit {short} committer",
            cname,
            cemail,
            meta=meta.for_committer(email, cemail),
            require_provenance_link=require_link and needs_provenance(cname, cemail),
        )
        # Signature warnings require explicit provenance meta (file or GitHub API).
        if meta.verified is not None:
            sig = classify_signature(meta)
            if sig is not None:
                report(findings, sig[0], f"commit {short}: {sig[1]}")
        scan_text(findings, f"commit {short} message", body)


def _exemption_flags_for_path(path: str) -> tuple[bool, bool]:
    if path == ".github/scripts/check-agent-attribution.selftest.sh":
        return True, False
    if path == ".github/scripts/check-agent-attribution.py":
        # Scanner source embeds forbidden phrases inside regex literals.
        return True, True
    return False, False


class _DiffPatchParser:
    """Stateful unified-diff walker; hunk lines never update file-header metadata."""

    def __init__(self) -> None:
        self.in_file = False
        self.in_hunk = False
        self.skip_signatures = False
        self.skip_policy_emails = False

    def on_diff_git(self) -> None:
        self.in_file = True
        self.in_hunk = False
        self.skip_signatures = False
        self.skip_policy_emails = False

    def on_binary(self) -> None:
        self.in_file = False
        self.in_hunk = False

    def on_header_line(self, line: str) -> None:
        if line.startswith("+++ b/"):
            self.skip_signatures, self.skip_policy_emails = _exemption_flags_for_path(line[6:])
            return
        if line.startswith("@@"):
            self.in_hunk = True

    def on_hunk_line(self, findings: list[Finding], line: str) -> None:
        if not line.startswith("+"):
            return
        _scan_added_diff_line(
            findings, line[1:], self.skip_signatures, self.skip_policy_emails
        )


def _scan_diff_patch(findings: list[Finding], patch: str) -> None:
    parser = _DiffPatchParser()
    for line in patch.splitlines():
        if line.startswith("diff --git "):
            parser.on_diff_git()
            continue
        if not parser.in_file:
            continue
        if line.startswith("Binary files ") and line.endswith(" differ"):
            parser.on_binary()
            continue
        if not parser.in_hunk:
            parser.on_header_line(line)
            continue
        parser.on_hunk_line(findings, line)


def _scan_added_diff_line(
    findings: list[Finding], payload: str, skip_signatures: bool, skip_policy_emails: bool
) -> None:
    if PROCESS_COMMENT_RE.search("+" + payload):
        report(
            findings,
            SEVERITY_WARN,
            "Added C++ comment looks like development-process text, not a technical comment.",
        )
    if not skip_signatures and GENERATED_DIFF_TEXT_RE.search(payload):
        report(
            findings,
            SEVERITY_WARN,
            "added diff line: AI-tool generation signature is discouraged.",
        )
    policy_emails = set(ALLOWED_TECHNICAL_EMAILS) | set(TRUSTED_RUNNER_EMAILS) | {
        "copilot@github.com",
    }
    if skip_signatures or skip_policy_emails:
        for email in emails_in_text(payload):
            if email.lower() in policy_emails:
                continue
            _reject_email(
                findings,
                "Added diff contains a non-allowlisted email address.",
                email,
                allow_technical=False,
            )
        if not skip_signatures and GENERATED_LINE_RE.search(payload):
            report(
                findings,
                SEVERITY_WARN,
                "added diff line: AI-tool generation signature is discouraged.",
            )
        if not skip_signatures:
            _scan_trailers(findings, "added diff line", payload)
        return
    scan_text(findings, "added diff line", payload)


def scan_diff(findings: list[Finding], from_sha: str, to_sha: str) -> None:
    try:
        start = require_full_sha(from_sha, "from")
        end = require_full_sha(to_sha, "to")
        commits = git_rev_list_range(start, end)
    except ValueError as exc:
        report(findings, SEVERITY_FAIL, str(exc))
        return
    _scan_diff_patch(findings, git_diff(start, end))
    for commit in commits:
        try:
            parent = git_rev_parse_parent(commit)
        except subprocess.CalledProcessError:
            continue
        patch = git_diff(parent, commit)
        if patch.strip():
            _scan_diff_patch(findings, patch)


def scan_file(findings: list[Finding], label: str, path: Path) -> None:
    if not path.is_file():
        report(findings, SEVERITY_FAIL, f"{label} file not found: {path}")
        return
    text = path.read_text(encoding="utf-8", errors="replace")
    scan_text(findings, label, text)


def scan_pr_file(findings: list[Finding], label: str, path_str: str) -> None:
    text = read_bounded_input_file(path_str, label, findings)
    if text is None:
        return
    scan_pr_text(findings, label, text)


def parse_paginated_json(raw: str, url: str) -> list:
    text = raw.strip()
    if not text:
        raise RuntimeError(f"empty GitHub API response for {url}")
    items: list = []
    decoder = json.JSONDecoder()
    idx = 0
    length = len(text)
    while idx < length:
        while idx < length and text[idx].isspace():
            idx += 1
        if idx >= length:
            break
        obj, idx = decoder.raw_decode(text, idx)
        if not isinstance(obj, list):
            raise RuntimeError(f"unexpected JSON type for {url}")
        items.extend(obj)
    return items


def gh_api_list(url: str) -> list:
    url = url.strip()
    if not GH_API_LIST_PATH_RE.fullmatch(url):
        raise ValueError(f"unsupported GitHub API list path: {url}")
    raw = subprocess.check_output(["gh", "api", "--paginate", url], text=True)
    return parse_paginated_json(raw, url)


def _is_bot_user(user: dict) -> bool:
    login = (user.get("login") or "").lower()
    utype = user.get("type") or ""
    if utype == "Bot" or login.endswith("[bot]"):
        return True
    return login in {item.lower() for item in BOT_COMMENT_LOGINS}


def collect_human_review_text(repo: str, number: str) -> str:
    chunks: list[str] = []
    for url in (
        f"repos/{repo}/issues/{number}/comments",
        f"repos/{repo}/pulls/{number}/comments",
        f"repos/{repo}/pulls/{number}/reviews",
    ):
        for item in gh_api_list(url):
            user = item.get("user") or {}
            if _is_bot_user(user):
                continue
            body = item.get("body") or ""
            if body.strip():
                chunks.append(body)
    return "\n\n".join(chunks)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--from", dest="from_sha")
    parser.add_argument("--to", dest="to_sha", default="HEAD")
    parser.add_argument("--diff", action="store_true")
    parser.add_argument("--pr-body")
    parser.add_argument("--pr-title")
    parser.add_argument("--pr-comments")
    parser.add_argument("--github-repo")
    parser.add_argument("--github-pr")
    parser.add_argument(
        "--commit-meta",
        help="JSON object keyed by full SHA with GitHub provenance fields "
        "(author_login, author_type, verified, reason)",
    )
    args = parser.parse_args()
    findings: list[Finding] = []
    meta_by_sha: dict[str, CommitGithubMeta] = {}

    if args.commit_meta:
        meta_by_sha.update(load_commit_meta_file(args.commit_meta, findings))

    github_repo = args.github_repo.strip() if args.github_repo else None
    if github_repo and not GITHUB_REPO_RE.fullmatch(github_repo):
        report(findings, SEVERITY_FAIL, "github repo must be owner/name")
        github_repo = None

    if args.from_sha:
        try:
            to_sha = normalize_to_sha(args.to_sha)
        except ValueError as exc:
            report(findings, SEVERITY_FAIL, str(exc))
            to_sha = ""
        if to_sha:
            scan_commits(
                findings,
                args.from_sha,
                to_sha,
                github_repo=github_repo,
                meta_by_sha=meta_by_sha,
            )
            if args.diff:
                scan_diff(findings, args.from_sha, to_sha)
    if args.pr_body:
        scan_pr_file(findings, "pull request body", args.pr_body)
    if args.pr_title:
        title_text = read_bounded_input_file(args.pr_title, "pull request title", findings)
        if title_text is not None:
            scan_pr_title(findings, "pull request title", title_text)
    if args.pr_comments:
        scan_pr_file(findings, "pull request comments", args.pr_comments)
    if github_repo and args.github_pr:
        review_text = ""
        if not re.fullmatch(r"[0-9]+", args.github_pr.strip()):
            report(findings, SEVERITY_FAIL, "github pr number must be digits only")
        else:
            try:
                review_text = collect_human_review_text(
                    github_repo, args.github_pr.strip()
                )
            except (RuntimeError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
                # JSONDecodeError is a ValueError subclass; keep it with transport errors.
                # Fail closed: an unscanned conversation could hide a personal email.
                report(
                    findings,
                    SEVERITY_FAIL,
                    f"could not load pull request comments for scan ({exc}).",
                )
            except ValueError as exc:
                report(findings, SEVERITY_FAIL, str(exc))
        if review_text:
            scan_pr_text(findings, "pull request comments", review_text)

    fails = [f for f in findings if f.severity == SEVERITY_FAIL]
    warns = [f for f in findings if f.severity == SEVERITY_WARN]
    if fails:
        print(
            f"Authorship hygiene check failed ({len(fails)} FAIL, {len(warns)} WARN). "
            "See AGENTS.md hard rule 16.",
            file=sys.stderr,
        )
        return 1
    if warns:
        print(
            f"Authorship hygiene check passed with {len(warns)} warning(s).",
            file=sys.stderr,
        )
        print("Authorship hygiene check passed with warnings.")
        return 0
    print("Authorship hygiene check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
