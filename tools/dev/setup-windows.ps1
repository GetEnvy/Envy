#Requires -Version 5.1
<#
.SYNOPSIS
    Bootstrap and verify a Windows development environment for Envy.

.PARAMETER Check
    Report prerequisites and repository-owned policy only; do not generate files.

.PARAMETER GenerateCompileCommands
    After a successful MSVC build, write gitignored compile_commands.json at repo root.

.PARAMETER Configuration
    Passed to generate-compile-commands.ps1 (default Release|x64).
#>
param(
    [switch]$Check,
    [switch]$GenerateCompileCommands,
    [string]$Configuration = 'Release|x64'
)

$ErrorActionPreference = 'Stop'
$scriptDir = $PSScriptRoot
$repoRoot = (Resolve-Path (Join-Path $scriptDir '..\..')).Path

function Write-Section([string]$Title)
{
    Write-Host ""
    Write-Host "=== $Title ==="
}

function Test-CommandOnPath([string]$Name)
{
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

$issues = @()

Write-Section 'Repository'
Write-Host "Root: $repoRoot"
foreach ($rel in @(
        'AGENTS.md',
        '.editorconfig',
        '.gitattributes',
        'Visual Studio\Envy.sln',
        'vcpkg.json',
        'docs\10_dev\development-environment.md'
    ))
{
    $path = Join-Path $repoRoot $rel
    if (Test-Path -LiteralPath $path)
    {
        Write-Host "  OK   $rel"
    }
    else
    {
        Write-Host "  MISS $rel"
        $issues += "Missing $rel"
    }
}

Write-Section 'Visual Studio / MSBuild'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (Test-Path -LiteralPath $vswhere)
{
    # v145 component IDs vary between Visual Studio channels. Require the
    # generic C++ tools, then verify the installed 14.5x MSVC toolset directly.
    # vswhere lists instances newest-first; prefer the newest instance whose
    # toolset is complete (x64+x86 compilers and ATL/MFC headers) and fall
    # back to the newest with a usable toolset, so one incomplete or
    # component-partial install cannot shadow a fully usable older one.
    $installPath = $null
    $v145Toolsets = @()
    $fallbackPath = $null
    $fallbackToolsets = @()
    $candidateInstances = @(& $vswhere -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath 2>$null)
    foreach ($candidate in $candidateInstances)
    {
        if ([string]::IsNullOrWhiteSpace($candidate)) { continue }
        # Both host/target compiler flavors are required: Release x64 and the
        # legacy Win32 builds use this toolset. Sort toolsets numerically
        # ([version]), not lexicographically: a plain name sort ranks
        # 14.50.9 above 14.50.10. Regex mirrors Select-Msvc145ToolsetDir in
        # generate-compile-commands.ps1 so the [version] cast only sees
        # dotted numeric names.
        $candidateToolsets = @(Get-ChildItem -LiteralPath (Join-Path $candidate 'VC\Tools\MSVC') -Directory -ErrorAction SilentlyContinue |
            Where-Object {
                $_.Name -match '^14\.5\d+(\.\d+)*$' -and
                (Test-Path -LiteralPath (Join-Path $_.FullName 'bin\Hostx64\x64\cl.exe')) -and
                (Test-Path -LiteralPath (Join-Path $_.FullName 'bin\Hostx64\x86\cl.exe'))
            } |
            Sort-Object { [version]$_.Name } -Descending)
        if ($candidateToolsets.Count -eq 0) { continue }
        # atlmfc headers live inside the toolset folder and are arch-neutral,
        # so the component verdict stays tied to the selected instance.
        $hasAtl = Test-Path -LiteralPath (Join-Path $candidateToolsets[0].FullName 'atlmfc\include\atlbase.h')
        $hasMfc = Test-Path -LiteralPath (Join-Path $candidateToolsets[0].FullName 'atlmfc\include\afxwin.h')
        if ($hasAtl -and $hasMfc)
        {
            $installPath = $candidate
            $v145Toolsets = $candidateToolsets
            break
        }
        if ([string]::IsNullOrWhiteSpace($fallbackPath))
        {
            $fallbackPath = $candidate
            $fallbackToolsets = $candidateToolsets
        }
    }
    if ([string]::IsNullOrWhiteSpace($installPath) -and -not [string]::IsNullOrWhiteSpace($fallbackPath))
    {
        $installPath = $fallbackPath
        $v145Toolsets = $fallbackToolsets
    }
    if (-not [string]::IsNullOrWhiteSpace($installPath))
    {
        # Resolve display metadata from the same instance that owns the
        # toolset. vswhere writes pretty-printed multi-line JSON, so join the
        # captured lines before parsing instead of parsing line-by-line.
        $instanceJson = (& $vswhere -prerelease -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -format json 2>$null) -join "`n"
        # Windows PowerShell 5.1 emits the parsed JSON array as one object;
        # ForEach-Object enumerates it so the filter sees each instance.
        $instance = @($instanceJson | ConvertFrom-Json -ErrorAction SilentlyContinue) |
            ForEach-Object { $_ } |
            Where-Object { $_.installationPath -eq $installPath } |
            Select-Object -First 1
        $displayName = $instance.displayName
        $installVersion = $instance.installationVersion
        # Report missing components against the selected toolset instance.
        $missing = @()
        if (-not (Test-Path -LiteralPath (Join-Path $v145Toolsets[0].FullName 'atlmfc\include\atlbase.h'))) { $missing += 'ATL' }
        if (-not (Test-Path -LiteralPath (Join-Path $v145Toolsets[0].FullName 'atlmfc\include\afxwin.h'))) { $missing += 'MFC' }
        if ($missing.Count -gt 0)
        {
            Write-Host ("  MISS Visual Studio C++ {0} for v145" -f ($missing -join '/'))
            $issues += ('Install C++ {0} for the latest MSVC/v145 toolset via Visual Studio Installer (Desktop development with C++ -> ATL/MFC).' -f ($missing -join ' and '))
        }
        else
        {
            Write-Host "  OK   $displayName $installVersion (v145 / $($v145Toolsets[0].Name) + ATL/MFC)"
            Write-Host "       $installPath"
        }
    }
    elseif (@($candidateInstances | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }).Count -gt 0)
    {
        Write-Host '  MISS Visual Studio C++ toolset v145 (no 14.5x MSVC toolset found)'
        $issues += 'Install the MSVC v145 (VS 2026) x86/x64 build tools component.'
    }
    else
    {
        Write-Host '  MISS Visual Studio with VC++ tools (vswhere)'
        $issues += 'Install Visual Studio 2026 with Desktop development with C++ (toolset v145, ATL, and MFC).'
    }
}
else
{
    Write-Host "  MISS vswhere.exe"
    $issues += 'Install Visual Studio 2026.'
}

if (Test-CommandOnPath 'msbuild')
{
    Write-Host "  OK   msbuild on PATH"
}
else
{
    Write-Host '  WARN msbuild not on PATH (open Developer PowerShell or VS Installer)'
}


Write-Section 'Windows SDK'
$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include'
$sdkOk = $false
if (Test-Path -LiteralPath $sdkRoot)
{
    $sdkDirs = @(Get-ChildItem -LiteralPath $sdkRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^10\.' } |
        Sort-Object { [version]($_.Name -replace '[^\d.].*$','') } -Descending)
    if ($sdkDirs.Count -gt 0)
    {
        $sdkOk = $true
        Write-Host "  OK   Windows 10/11 SDK $($sdkDirs[0].Name)"
    }
}
if (-not $sdkOk)
{
    Write-Host '  MISS Windows 10/11 SDK (Windows Kits\10)'
    $issues += 'Install a Windows 10/11 SDK (WindowsTargetPlatformVersion=10.0) via Visual Studio Installer.'
}

Write-Section 'vcpkg manifest'
$bootstrap = Join-Path $repoRoot 'scripts\bootstrap-vcpkg.cmd'
if (Test-Path -LiteralPath $bootstrap)
{
    Write-Host '  Win32: scripts\bootstrap-vcpkg.cmd -Triplet x86-windows-static'
    Write-Host '  x64 manifest (optional): scripts\bootstrap-vcpkg.cmd'
    Write-Host '  Both: scripts\bootstrap-vcpkg.cmd -All'
    Write-Host '  See docs\10_dev\build.md (x64 BugSplat does not require vcpkg_installed\x64-windows-static).'
}
else
{
    $issues += 'scripts\bootstrap-vcpkg.cmd missing'
}

Write-Section 'Optional: clangd (Cursor / VS Code navigation)'
if (Test-CommandOnPath 'clangd')
{
    $ver = & clangd --version 2>$null | Select-Object -First 1
    Write-Host "  OK   $ver"
}
else
{
    Write-Host '  INFO clangd not on PATH; install via llvm-vs-code-extensions.vscode-clangd or LLVM.'
    Write-Host '       See docs/10_dev/development-environment.md'
}

Write-Section 'Generated / local (must not be committed)'
foreach ($rel in @('compile_commands.json', '.vs', 'vcpkg_installed'))
{
    $path = Join-Path $repoRoot $rel
    $present = Test-Path -LiteralPath $path
    Write-Host ("  {0,-4} {1}" -f ($(if ($present) { 'yes' } else { 'no ' })), $rel)
}

if ($GenerateCompileCommands -and -not $Check)
{
    Write-Section 'compile_commands.json'
    $gen = Join-Path $scriptDir 'generate-compile-commands.ps1'
    try
    {
        & $gen -Configuration $Configuration
    }
    catch
    {
        Write-Host "  WARN generate-compile-commands.ps1 failed: $($_.Exception.Message)"
        $issues += 'compile_commands.json was not generated (build in VS first, then re-run with -GenerateCompileCommands).'
    }
}

Write-Section 'Encoding (#350)'
Write-Host '  Legacy C/C++ headers may use non-UTF-8 bytes. Do not force UTF-8 on save.'
Write-Host '  CI blocks NEW U+FFFD (EF BF BD) in changed first-party sources.'

Write-Section 'Summary'
if ($issues.Count -eq 0)
{
    Write-Host 'Environment check: no blocking issues reported.'
    exit 0
}

foreach ($i in $issues) { Write-Host "  - $i" }
Write-Host 'Environment check: action required (see messages above).'
exit 1
