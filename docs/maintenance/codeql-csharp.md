# CodeQL C# manual-build status

## Why this exists

GitHub CodeQL reported low C# analysis quality because C# extraction was
running with `build-mode: none`, which reduces call-target resolution.

This repository now uses a dedicated workflow at
`.github/workflows/codeql-csharp.yml` with:

- `github/codeql-action/init@v4`
- `languages: csharp`
- `build-mode: manual`
- `queries: security-extended,security-and-quality`
- explicit `msbuild` compilation for C# solutions that currently build

## C# solutions/projects inventory

Detected C# artifacts in this repository:

- `Languages/Tools/SkinUpdater/SkinUpdater.sln`
- `Languages/Tools/SkinUpdater/SkinUpdater.csproj`
- `Languages/Tools/SkinUpdater/Common/UpdaterCommon.csproj`
- `Plugins/FictionBookReader/FictionBookReader.sln`
- `Plugins/FictionBookReader/FictionBookReader.csproj`
- `Plugins/NETInterop/FictionBookReader/FictionBookReader.VS2008.sln`
- `Plugins/NETInterop/FictionBookReader/FictionBookReader.csproj`

## Buildability on modern hosted runners

### Build succeeds (included in manual CodeQL build)

- `Languages/Tools/SkinUpdater/SkinUpdater.sln`
  - Built successfully with MSBuild Release / Any CPU.
  - Produces `SkinUpdater.Common.dll` and `SkinUpdater.exe`.

### Build currently blocked (not silently ignored)

- `Plugins/FictionBookReader/FictionBookReader.sln`
- `Plugins/NETInterop/FictionBookReader/FictionBookReader.VS2008.sln`
  - Solution-level build currently fails to parse in modern MSBuild
    (`MSB4025: Root element is missing`).
  - First remediation step is solution-file normalization (header/BOM,
    leading blank-line cleanup, and consistent line endings), then retry
    with modern MSBuild before project retargeting.

- `Plugins/FictionBookReader/FictionBookReader.csproj`
- `Plugins/NETInterop/FictionBookReader/FictionBookReader.csproj`
  - Project-level build fails with missing .NET Framework targeting pack:
    `MSB3644: reference assemblies for .NETFramework,Version=v4.0 were not found`.

## Smallest modernization path (behavior-preserving)

After solution parsing is fixed, for both `FictionBookReader.csproj`
variants:

1. Retarget to `.NET Framework 4.8` in project metadata.
2. Keep `ToolsVersion` upgrade minimal (SDK-style migration not required).
3. Keep existing compile items, pre/post-build scripts, and COM interop
   flow unchanged unless build requires a narrowly scoped adjustment.
4. Rebuild under CI with the same manual CodeQL workflow after retargeting.

This path is intentionally minimal and focuses on buildability for CodeQL
precision, not runtime feature changes.
