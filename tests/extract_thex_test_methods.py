#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Compile unchanged production methods against isolated transfer services.

Extraction avoids maintaining a second parser or refactoring legacy runtime
code solely for a test. Missing/ambiguous method boundaries fail the build.
The harness replaces service objects, not XML, DIME, or receive-state logic.
"""

import argparse
from pathlib import Path
import re


def extract(source, signature):
    matches = list(re.finditer(r"^" + re.escape(signature), source, re.M))
    if len(matches) != 1:
        raise ValueError(f"Expected one method: {signature}")
    start = matches[0].start()
    end = source.find("\n}\n", start)
    if end < 0 or "\n{" not in source[start:end]:
        raise ValueError(f"Missing method boundary: {signature}")
    return source[start:end + 3], source[:start].count("\n") + 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    methods = {
        "Envy/Buffer.cpp": [
            "CBuffer::CBuffer()", "CBuffer::~CBuffer()",
            "void CBuffer::Add(", "void CBuffer::Remove(",
            "bool CBuffer::EnsureBuffer(", "CString CBuffer::ReadString(",
            "void CBuffer::WriteDIME(", "BOOL CBuffer::ReadDIME(",
            "BOOL CBuffer::PeekDIME(",
        ],
        "Envy/DownloadTransferHTTP.cpp": [
            "BOOL CDownloadTransferHTTP::StartNextFragment()",
            "BOOL CDownloadTransferHTTP::ReadTiger(bool bDropped)",
        ],
        "Envy/DownloadWithTiger.cpp": ["BOOL CDownloadWithTiger::SetTigerTree("],
    }
    output = ["// Extracted unchanged from the current checkout; do not edit.\n"]
    for path, signatures in methods.items():
        # Legacy sources use single-byte copyright characters. Preserve source
        # bytes apart from line endings; MSVC accepts them in comments.
        source = (root / path).read_bytes().replace(b"\r\n", b"\n").decode("latin1")
        for signature in signatures:
            body, line = extract(source, signature)
            output.append(f'#line {line} "{path}"\n{body}\n')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes("".join(output).encode("latin1"))


if __name__ == "__main__":
    main()
