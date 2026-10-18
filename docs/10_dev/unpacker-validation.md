# Package destination validation

The package installer treats manifest attributes and ZIP entry names as untrusted.
The selected installation root comes from the existing Envy registry settings or
the executable directory. The destination mapping remains:

| Package type | Root and package directory |
| --- | --- |
| Skin, including an omitted type | `Skins/<manifest path or name>` |
| Language | `Skins/Languages` |
| Schemas | `Schemas` |
| Plugin | `Plugins` |
| Data or security | Configured `DataPath`, otherwise `Data` |

ZIP directories are validated but not materialized. Files are flattened to their
basenames, as before. Repeated basenames are installed in archive order; the last
valid entry replaces the earlier one. A missing manifest path uses a separately
owned copy of its validated name. An invalid explicit path is rejected, never
silently replaced with the name.
The manifest name is a folder only in that fallback, so it must not contain a
separator then; with an explicit path it is display text (shipped skins use names
such as `Windows8/Alt` next to `path="Windows"`).

## Path and archive contract

- Reject parent/current-directory components, rooted/drive/UNC/device names in
  package input, empty components, reserved DOS devices, ADS colons, invalid
  characters, trailing spaces/dots and malformed UTF-8 or UTF-16.
- Relative paths and complete destination paths must fit below `MAX_PATH`,
  including their terminating NUL; individual components are at most 255 UTF-16
  code units. UTF-8 ZIP names are measured in bytes before conversion and in
  UTF-16 code units afterwards. Unsupported lengths are rejected, not truncated.
- Validate ZIP metadata before conversion, reject embedded NULs, encryption,
  unsupported compression/entry types and reparse attributes. Check exact byte
  counts, EOF and CRC for every entry before installation, and again while staging
  each output. The first XML entry remains the manifest-bearing document.
- The manifest-bearing XML is limited to 4 MiB. This closes the directly allocated
  manifest-buffer boundary; a complete package expansion/time budget remains
  separate work under #406. Classic minizip and the existing 32-bit archive seek
  interface remain in use; this change does not introduce ZIP64 support.
- Extraction repeats validation using one archive open with write sharing denied.
  A package type changed since preview is rejected rather than using the preview's
  previously selected root.

## Filesystem confinement and replacement

All directory ancestors are opened without following reparse points and held
without write/delete sharing until extraction ends. Directory-list access is
intentional: an attributes-only handle does not enforce the required rename
exclusion. Existing reparse points, including junctions in root ancestors, are
rejected. Newly created directories are opened and checked before use.

Ordinary writable files can be replaced. Existing directories, read-only files,
reparse points and multiply linked files are rejected. Data is written to a new,
exclusively opened staging file in the pinned destination directory. Only after
the byte count, CRC and flush succeed is that open file renamed over its target.
Replacement changes the directory entry rather than truncating the old file, so
a concurrent target alias cannot cause a write through that alias. Replacement
requires delete/rename permission; files held open incompatibly fail safely.

The rename uses native `FileRenameInformation` with a leaf name and a null root
for an in-place rename. The Win32 wrapper resolves relative names against the
process directory and may reopen a parent in conflict with the directory pins.
The native export is checked and absence fails closed. See Microsoft's
[rename structure contract](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_file_rename_information),
[file sharing and reparse flags](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew),
and [Windows name rules](https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file).

This is per-file replacement, not a package-wide transaction: a later filesystem
failure can leave earlier entries installed and newly created empty directories.
An interrupted process may leave a staging file. Network filesystems must honor
the locking/rename contracts; unsupported behavior fails closed. Live SMB and
third-party filesystem compatibility are not established by local NTFS tests.

## Regression commands

Use a Visual Studio 2026 Developer PowerShell with v145, MFC/ATL and the Windows
SDK installed. The main regression groups compile the production `Extract.c`,
`Utils.c` and `PackagePath.c` into EnvyTests and run in the existing Windows CI.
The focused project runs the same cases without the rest of EnvyTests.

```powershell
msbuild "Visual Studio/Envy.sln" /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0
& "tests/Release x64/EnvyTests.exe"
msbuild "Visual Studio/Envy.sln" /m /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v145 /p:WindowsTargetPlatformVersion=10.0 /p:VcpkgEnableManifest=true /p:VcpkgTriplet=x86-windows-static
& "tests/Release Win32/EnvyTests.exe"
msbuild tests/UnpackerTests.vcxproj /p:Configuration=Release /p:Platform=x64
& "tests/Release x64/UnpackerTests.exe"
```

For AddressSanitizer, build the zlib dependency normally first, then build the
focused project with `/p:EnableASAN=true /p:BuildProjectReferences=false` and
`/p:UnpackerTestRoot=<disposable-output-directory>`. Put the matching MSVC
AddressSanitizer runtime directory on `PATH` and run its `bin/UnpackerTests.exe`.
This instruments first-party extraction and test code; it does not claim to
instrument the vendored archive DLL.

Fixtures cover all package types, flattening, mixed separators, omitted paths,
ownership cleanup, traversal/rooted/UNC/ADS/device names, Unicode and aggregate
length limits, malformed metadata/UTF-8, truncated ZIPs, CRC mismatches, manifest
buffer boundaries, regular replacement, read-only/hard-link/junction rejection,
directory rename exclusion, registry failure and changed preview type. Filesystem
tests use disposable fixtures and outside sentinels; registry tests redirect HKCU
to a volatile test key and restore it. Tests do not write the real Envy profile.

The maintainer still validates actual package UI/install behavior and marks the
Draft ready. Draft build deferrals are not native build evidence.
