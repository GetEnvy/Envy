#Requires -Version 5.1
<#
.SYNOPSIS
    Locate a usable MSVC/v145 MSBuild via vswhere and build Visual Studio\Envy.sln.

.DESCRIPTION
    Modes (Visual Studio project/solution files are never modified by this script):

    Fast  Build the Envy project and its solution dependencies only
          (/t:Envy). The Installer project is not built.
    Full  Build the whole solution including the Installer (/t:Build).
          Same as Visual Studio "Build Solution"; Installer packaging uses
          the project's CustomBuild (bundled ISCC.exe reading the live repo).

    Rebuild / Clean / Installer-only targets remain available via Visual Studio
    or a direct MSBuild invocation; they are intentionally not wrapped here.

    Instance selection mirrors setup-windows.ps1 / generate-compile-commands.ps1:
    enumerate all VS/Build Tools instances (prerelease included), prefer the
    newest with a complete MSVC 14.5x toolset (Hostx64 x64+x86 cl.exe and
    ATL/MFC) that also hosts MSBuild, then fall back to the newest usable
    toolset+MSBuild. Never take MSBuild from -latest alone when that instance
    lacks the C++ toolset.

    Parallelism: Fast defaults to /m (multi-project). Full defaults to /m:1
    to avoid the documented Envy.pch C1083 races under parallel MSBuild
    (see .github/workflows/build.yml). Override with -MaxCpuCount or
    ENVY_MSBUILD_MAXCPUCOUNT.

.PARAMETER SelfTest
    Run offline selection/parallelism checks and exit (no MSBuild required).
#>
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Build')]
    [ValidateSet('Fast', 'Full')]
    [string]$Mode,

    [Parameter(ParameterSetName = 'Build')]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',

    [Parameter(ParameterSetName = 'Build')]
    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64',

    [Parameter(ParameterSetName = 'Build')]
    [string]$MaxCpuCount = $env:ENVY_MSBUILD_MAXCPUCOUNT,

    [Parameter(Mandatory = $true, ParameterSetName = 'SelfTest')]
    [switch]$SelfTest
)

$ErrorActionPreference = 'Stop'

function Get-RepoRoot
{
    $here = $PSScriptRoot
    while ($here)
    {
        if (Test-Path -LiteralPath (Join-Path $here 'Visual Studio\Envy.sln'))
        {
            return (Resolve-Path -LiteralPath $here).Path
        }
        $parent = Split-Path -Parent $here
        if (-not $parent -or $parent -eq $here) { break }
        $here = $parent
    }
    throw 'Could not locate repository root (Visual Studio\Envy.sln).'
}

function Get-VsWherePath
{
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) { return $vswhere }
    return $null
}

function Test-Msvc145ToolsetComplete
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$InstallPath,
        [switch]$RequireAtlMfc
    )

    $msvcRoot = Join-Path $InstallPath 'VC/Tools/MSVC'
    if (-not (Test-Path -LiteralPath $msvcRoot)) { return $false }

    $toolsets = @(Get-ChildItem -LiteralPath $msvcRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object {
            $_.Name -match '^14\.5\d+(\.\d+)*$' -and
            (Test-Path -LiteralPath (Join-Path $_.FullName 'bin/Hostx64/x64/cl.exe')) -and
            (Test-Path -LiteralPath (Join-Path $_.FullName 'bin/Hostx64/x86/cl.exe'))
        } |
        Sort-Object { [version]$_.Name } -Descending)

    if ($toolsets.Count -eq 0) { return $false }
    if (-not $RequireAtlMfc) { return $true }

    $top = $toolsets[0].FullName
    return (
        (Test-Path -LiteralPath (Join-Path $top 'atlmfc/include/atlbase.h')) -and
        (Test-Path -LiteralPath (Join-Path $top 'atlmfc/include/afxwin.h'))
    )
}

function Get-MsBuildCandidates
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$InstallPath
    )

    @(
        (Join-Path $InstallPath 'MSBuild/Current/Bin/amd64/MSBuild.exe'),
        (Join-Path $InstallPath 'MSBuild/Current/Bin/MSBuild.exe')
    )
}

function Find-MsBuildInInstall
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$InstallPath
    )

    foreach ($candidate in (Get-MsBuildCandidates -InstallPath $InstallPath))
    {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }
    return $null
}

function Select-MsBuildFromInstallPaths
{
    <#
    .SYNOPSIS
        Pure selection over an ordered list of VS install roots (production + SelfTest).

    .DESCRIPTION
        Walks InstallPaths in the given order. Prefers the first instance that
        has MSBuild and a complete v145 toolset with ATL/MFC; otherwise the
        first with MSBuild and a usable v145 toolset (x64+x86 cl). MSBuild-only
        roots are never selected as a valid C++ v145 environment.
    #>
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [AllowNull()]
        [string[]]$InstallPaths
    )

    if ($null -eq $InstallPaths) { return $null }

    $completePath = $null
    $usablePath = $null
    foreach ($candidate in $InstallPaths)
    {
        if ([string]::IsNullOrWhiteSpace($candidate)) { continue }
        if (-not (Find-MsBuildInInstall -InstallPath $candidate)) { continue }

        if (Test-Msvc145ToolsetComplete -InstallPath $candidate -RequireAtlMfc)
        {
            $completePath = $candidate
            break
        }
        if (-not $usablePath -and (Test-Msvc145ToolsetComplete -InstallPath $candidate))
        {
            $usablePath = $candidate
        }
    }

    $chosen = if ($completePath) { $completePath } else { $usablePath }
    if (-not $chosen) { return $null }
    return (Find-MsBuildInInstall -InstallPath $chosen)
}

function Get-VsInstallRootFromMsBuild
{
    <#
    .SYNOPSIS
        Map an MSBuild.exe path back to the VS install root that hosts it.

    .DESCRIPTION
        Returns the ancestor directory whose MSBuild/Current/Bin[/amd64]
        candidate is exactly MsBuildPath, or $null when the executable does
        not live in a VS/Build Tools layout (e.g. a standalone .NET SDK).
    #>
    param(
        [Parameter(Mandatory = $true)]
        [string]$MsBuildPath
    )

    $target = [System.IO.Path]::GetFullPath($MsBuildPath)
    $dir = Split-Path -Parent $target
    while ($dir)
    {
        foreach ($candidate in (Get-MsBuildCandidates -InstallPath $dir))
        {
            if ([string]::Equals([System.IO.Path]::GetFullPath($candidate), $target, [System.StringComparison]::OrdinalIgnoreCase))
            {
                return $dir
            }
        }
        $parent = Split-Path -Parent $dir
        if (-not $parent -or $parent -eq $dir) { break }
        $dir = $parent
    }
    return $null
}

function Select-PathMsBuild
{
    <#
    .SYNOPSIS
        Validate an MSBuild.exe found on PATH against the v145 policy.

    .DESCRIPTION
        PATH fallback is only accepted when its owning VS install passes the
        same Select-MsBuildFromInstallPaths checks as vswhere candidates.
        Returns the selected MSBuild of that install, or $null.
    #>
    param(
        [AllowNull()]
        [string]$MsBuildPath
    )

    if ([string]::IsNullOrWhiteSpace($MsBuildPath)) { return $null }
    $root = Get-VsInstallRootFromMsBuild -MsBuildPath $MsBuildPath
    if (-not $root) { return $null }
    return (Select-MsBuildFromInstallPaths -InstallPaths @($root))
}

function Find-MsBuild
{
    # Prefer a complete v145 instance that also hosts MSBuild. Do not stop at
    # vswhere -latest: that can return MSBuild-only or incomplete C++ installs
    # and shadow an older usable toolset (same policy as setup-windows.ps1).
    $vswhere = Get-VsWherePath
    if ($vswhere)
    {
        $candidateInstances = @(& $vswhere -prerelease -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath 2>$null)

        $msb = Select-MsBuildFromInstallPaths -InstallPaths $candidateInstances
        if ($msb) { return $msb }
    }

    # PATH fallback (no vswhere / no match): accept it only when its owning
    # VS install also has a usable v145 toolset; otherwise the build would
    # start and fail later with an incompatible toolset.
    $onPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($onPath)
    {
        $msb = Select-PathMsBuild -MsBuildPath $onPath.Source
        if ($msb) { return $msb }
        Write-Warning "Ignoring MSBuild on PATH without a usable MSVC v145 toolset: $($onPath.Source)"
    }

    throw 'MSBuild.exe not found with a usable MSVC v145 (14.5x) x86/x64 toolset. Install Visual Studio 2026 (or Build Tools) with Desktop development with C++, toolset v145, ATL, and MFC.'
}

function Get-MsBuildParallelSwitch
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$Mode,
        [string]$MaxCpuCount
    )

    if (-not [string]::IsNullOrWhiteSpace($MaxCpuCount))
    {
        if ($MaxCpuCount -notmatch '^[1-9]\d*$')
        {
            throw "MaxCpuCount must be a positive integer (got '$MaxCpuCount')."
        }
        return "/m:$MaxCpuCount"
    }

    # Fast: project graph is small; keep multi-proc. Full solution rebuilds
    # many PCH producers in parallel and hits C1083 on some hosts (CI).
    switch ($Mode)
    {
        'Fast' { return '/m' }
        default { return '/m:1' }
    }
}

function Get-BuildLockName
{
    # One lock per checkout (not per Configuration|Platform): ISCC writes
    # Installer\Scripts\Preprocessed.iss and the Builds\ package for every
    # configuration (only the iscc.stamp marker is per $(IntDir)), and obj/PCH
    # outputs are shared by Fast and Full.
    param(
        [Parameter(Mandatory = $true)]
        [string]$RepoRoot
    )

    $key = [System.IO.Path]::GetFullPath($RepoRoot).TrimEnd('\', '/').ToLowerInvariant()
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try
    {
        $hash = $sha.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($key))
    }
    finally
    {
        $sha.Dispose()
    }
    $hex = -join ($hash[0..15] | ForEach-Object { $_.ToString('x2') })
    return "Local\Envy-run-msbuild-$hex"
}

function New-FakeVsInstall
{
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [string]$ToolsetVersion = '14.50.10',
        [switch]$WithX86,
        [switch]$WithAtlMfc,
        [switch]$WithMsBuildAmd64,
        [switch]$WithMsBuildX86,
        [switch]$MultiToolset
    )

    New-Item -ItemType Directory -Path $Root -Force | Out-Null
    $versions = @($ToolsetVersion)
    if ($MultiToolset)
    {
        $versions = @('14.50.0', $ToolsetVersion)
    }
    foreach ($ver in $versions)
    {
        $toolset = Join-Path $Root "VC/Tools/MSVC/$ver"
        New-Item -ItemType Directory -Path (Join-Path $toolset 'bin/Hostx64/x64') -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $toolset 'bin/Hostx64/x64/cl.exe') -Force | Out-Null
        if ($WithX86)
        {
            New-Item -ItemType Directory -Path (Join-Path $toolset 'bin/Hostx64/x86') -Force | Out-Null
            New-Item -ItemType File -Path (Join-Path $toolset 'bin/Hostx64/x86/cl.exe') -Force | Out-Null
        }
        if ($WithAtlMfc)
        {
            New-Item -ItemType Directory -Path (Join-Path $toolset 'atlmfc/include') -Force | Out-Null
            New-Item -ItemType File -Path (Join-Path $toolset 'atlmfc/include/atlbase.h') -Force | Out-Null
            New-Item -ItemType File -Path (Join-Path $toolset 'atlmfc/include/afxwin.h') -Force | Out-Null
        }
    }
    if ($WithMsBuildAmd64)
    {
        New-Item -ItemType Directory -Path (Join-Path $Root 'MSBuild/Current/Bin/amd64') -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $Root 'MSBuild/Current/Bin/amd64/MSBuild.exe') -Force | Out-Null
    }
    if ($WithMsBuildX86)
    {
        New-Item -ItemType Directory -Path (Join-Path $Root 'MSBuild/Current/Bin') -Force | Out-Null
        New-Item -ItemType File -Path (Join-Path $Root 'MSBuild/Current/Bin/MSBuild.exe') -Force | Out-Null
    }
}

function Invoke-SelfTest
{
    # Offline checks: no vswhere / MSBuild required. Exercises the same
    # Select-MsBuildFromInstallPaths algorithm Find-MsBuild uses in production.
    $tmpRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("envy-msbuild-selftest-" + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $tmpRoot | Out-Null
    try
    {
        $msbOnly = Join-Path $tmpRoot 'msbonly'
        $oldComplete = Join-Path $tmpRoot 'old-complete'
        $newX64Only = Join-Path $tmpRoot 'new-x64-only'
        $newNoAtl = Join-Path $tmpRoot 'new-no-atl'
        $buildTools = Join-Path $tmpRoot 'build-tools'
        $empty = Join-Path $tmpRoot 'empty'

        New-FakeVsInstall -Root $msbOnly -WithMsBuildAmd64
        New-FakeVsInstall -Root $oldComplete -ToolsetVersion '14.50.10' -WithX86 -WithAtlMfc -WithMsBuildAmd64 -MultiToolset
        New-FakeVsInstall -Root $newX64Only -ToolsetVersion '14.51.0' -WithMsBuildAmd64
        New-FakeVsInstall -Root $newNoAtl -ToolsetVersion '14.51.0' -WithX86 -WithMsBuildX86
        New-FakeVsInstall -Root $buildTools -ToolsetVersion '14.50.20' -WithX86 -WithAtlMfc -WithMsBuildAmd64
        New-Item -ItemType Directory -Path $empty | Out-Null

        if (-not (Test-Msvc145ToolsetComplete -InstallPath $oldComplete -RequireAtlMfc))
        {
            throw 'SelfTest: complete toolset was not recognized.'
        }
        if (Test-Msvc145ToolsetComplete -InstallPath $newX64Only -RequireAtlMfc)
        {
            throw 'SelfTest: x64-only toolset was incorrectly accepted as complete.'
        }
        if (Test-Msvc145ToolsetComplete -InstallPath $newX64Only)
        {
            throw 'SelfTest: x86-less toolset must not count as usable v145.'
        }
        if (Test-Msvc145ToolsetComplete -InstallPath $newNoAtl -RequireAtlMfc)
        {
            throw 'SelfTest: ATL/MFC-less toolset must not count as complete.'
        }
        if (-not (Test-Msvc145ToolsetComplete -InstallPath $newNoAtl))
        {
            throw 'SelfTest: x64+x86 without ATL should still count as usable v145.'
        }
        if (Test-Msvc145ToolsetComplete -InstallPath $msbOnly)
        {
            throw 'SelfTest: MSBuild-only install must not count as usable v145.'
        }

        # Recent MSBuild-only + older complete -> older complete wins.
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($msbOnly, $oldComplete)
        $expectOld = Find-MsBuildInInstall -InstallPath $oldComplete
        if ($sel -ne $expectOld)
        {
            throw "SelfTest: MSBuild-only must not win over complete v145 (got '$sel')."
        }

        # Recent x64-only + older complete -> older complete.
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($newX64Only, $oldComplete)
        if ($sel -ne $expectOld)
        {
            throw "SelfTest: x64-only must not win over complete v145 (got '$sel')."
        }

        # Recent without ATL/MFC + older complete -> older complete (ATL preferred).
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($newNoAtl, $oldComplete)
        if ($sel -ne $expectOld)
        {
            throw "SelfTest: incomplete ATL install must not win over complete (got '$sel')."
        }

        # Only usable (no ATL) available -> select usable.
        $expectNoAtl = Find-MsBuildInInstall -InstallPath $newNoAtl
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($msbOnly, $newNoAtl)
        if ($sel -ne $expectNoAtl)
        {
            throw "SelfTest: usable non-ATL should be selected when no complete exists (got '$sel')."
        }

        # Build Tools complete install is valid.
        $expectBt = Find-MsBuildInInstall -InstallPath $buildTools
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($buildTools)
        if ($sel -ne $expectBt)
        {
            throw "SelfTest: Build Tools complete install should be selected (got '$sel')."
        }

        # No valid candidate.
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($msbOnly, $newX64Only, $empty)
        if ($null -ne $sel)
        {
            throw "SelfTest: expected no selection for invalid-only list (got '$sel')."
        }
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @()
        if ($null -ne $sel)
        {
            throw 'SelfTest: empty install list must yield null.'
        }

        # Candidate order: first complete in list wins (not "newest path" heuristics).
        $otherComplete = Join-Path $tmpRoot 'other-complete'
        New-FakeVsInstall -Root $otherComplete -ToolsetVersion '14.50.30' -WithX86 -WithAtlMfc -WithMsBuildX86
        $expectOther = Find-MsBuildInInstall -InstallPath $otherComplete
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($otherComplete, $oldComplete)
        if ($sel -ne $expectOther)
        {
            throw "SelfTest: first complete candidate in order should win (got '$sel')."
        }
        $sel = Select-MsBuildFromInstallPaths -InstallPaths @($oldComplete, $otherComplete)
        if ($sel -ne $expectOld)
        {
            throw "SelfTest: reordered candidates should change selection (got '$sel')."
        }

        # Non-regression: a buggy "prefer first MSBuild path" would pick msbOnly.
        $buggyFirstMsb = $null
        foreach ($c in @($msbOnly, $oldComplete))
        {
            $buggyFirstMsb = Find-MsBuildInInstall -InstallPath $c
            if ($buggyFirstMsb) { break }
        }
        $correct = Select-MsBuildFromInstallPaths -InstallPaths @($msbOnly, $oldComplete)
        if ($buggyFirstMsb -eq $correct)
        {
            throw 'SelfTest: order non-regression failed — correct selector matched buggy first-MSBuild pick.'
        }
        if ($buggyFirstMsb -ne (Find-MsBuildInInstall -InstallPath $msbOnly))
        {
            throw 'SelfTest: buggy baseline should have selected MSBuild-only.'
        }
        if ($correct -ne $expectOld)
        {
            throw 'SelfTest: correct selector must still prefer complete v145.'
        }

        # PATH fallback: owning install must pass the same v145 checks.
        $pathMsbOnly = Find-MsBuildInInstall -InstallPath $msbOnly
        if ($null -ne (Select-PathMsBuild -MsBuildPath $pathMsbOnly))
        {
            throw 'SelfTest: PATH MSBuild from an MSBuild-only install must be rejected.'
        }
        if ($null -ne (Select-PathMsBuild -MsBuildPath (Find-MsBuildInInstall -InstallPath $newX64Only)))
        {
            throw 'SelfTest: PATH MSBuild from an x86-less install must be rejected.'
        }
        if ((Get-VsInstallRootFromMsBuild -MsBuildPath $pathMsbOnly) -ne $msbOnly)
        {
            throw 'SelfTest: PATH MSBuild did not map back to its install root.'
        }
        $sel = Select-PathMsBuild -MsBuildPath (Find-MsBuildInInstall -InstallPath $newNoAtl)
        if ($sel -ne $expectNoAtl)
        {
            throw "SelfTest: PATH MSBuild from a usable v145 install should be accepted (got '$sel')."
        }
        $stray = Join-Path $tmpRoot 'dotnet-sdk/MSBuild.exe'
        New-Item -ItemType File -Path $stray -Force | Out-Null
        if ($null -ne (Select-PathMsBuild -MsBuildPath $stray))
        {
            throw 'SelfTest: PATH MSBuild outside a VS layout must be rejected.'
        }
        if ($null -ne (Select-PathMsBuild -MsBuildPath ''))
        {
            throw 'SelfTest: empty PATH MSBuild must yield null.'
        }

        if ((Get-BuildLockName -RepoRoot 'C:\Repo\Envy') -ne (Get-BuildLockName -RepoRoot 'c:\repo\envy\'))
        {
            throw 'SelfTest: build lock name must not depend on case or trailing separator.'
        }
        if ((Get-BuildLockName -RepoRoot 'C:\Repo\Envy') -eq (Get-BuildLockName -RepoRoot 'C:\Repo\Envy2'))
        {
            throw 'SelfTest: distinct checkouts must use distinct build locks.'
        }

        $goodMsb = Find-MsBuildInInstall -InstallPath $oldComplete
        if (-not $goodMsb -or $goodMsb -notmatch '[/\\]amd64[/\\]MSBuild\.exe$')
        {
            throw "SelfTest: expected amd64 MSBuild under complete install, got '$goodMsb'."
        }

        foreach ($pair in @(
            @{ Mode = 'Fast'; Expected = '/m' },
            @{ Mode = 'Full'; Expected = '/m:1' }
        ))
        {
            $got = Get-MsBuildParallelSwitch -Mode $pair.Mode -MaxCpuCount ''
            if ($got -ne $pair.Expected)
            {
                throw "SelfTest: Mode $($pair.Mode) expected $($pair.Expected), got $got"
            }
        }
        $override = Get-MsBuildParallelSwitch -Mode 'Full' -MaxCpuCount '4'
        if ($override -ne '/m:4')
        {
            throw "SelfTest: MaxCpuCount override failed, got $override"
        }
        foreach ($bad in @('0', '-1', '2x'))
        {
            $rejected = $false
            try { Get-MsBuildParallelSwitch -Mode 'Full' -MaxCpuCount $bad | Out-Null }
            catch { $rejected = $true }
            if (-not $rejected)
            {
                throw "SelfTest: MaxCpuCount '$bad' should be rejected"
            }
        }

        Write-Host 'run-msbuild.ps1 -SelfTest passed.'
    }
    finally
    {
        Remove-Item -LiteralPath $tmpRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}

if ($SelfTest)
{
    Invoke-SelfTest
    exit 0
}

$repoRoot   = Get-RepoRoot
$solution   = Join-Path $repoRoot 'Visual Studio\Envy.sln'
$msbuildExe = Find-MsBuild
$parallel   = Get-MsBuildParallelSwitch -Mode $Mode -MaxCpuCount $MaxCpuCount

$solutionTarget = switch ($Mode)
{
    'Fast' { 'Envy' }
    'Full' { 'Build' }
}

$msbArgs = [System.Collections.Generic.List[string]]::new()
$msbArgs.Add("/t:$solutionTarget")
$msbArgs.Add($parallel)
$msbArgs.Add('/nologo')
$msbArgs.Add('/v:m')
$msbArgs.Add("/p:Configuration=$Configuration")
$msbArgs.Add("/p:Platform=$Platform")
if ($Platform -eq 'Win32')
{
    $msbArgs.Add('/p:VcpkgEnableManifest=true')
    $msbArgs.Add('/p:VcpkgTriplet=x86-windows-static')
}

Write-Host "MSBuild : $msbuildExe"
Write-Host "Mode    : $Mode ($Configuration|$Platform)"
Write-Host "Parallel: $parallel"
if ($Mode -eq 'Full')
{
    Write-Host 'Output  : installer package is written to <repo>\Builds when ISCC succeeds.'
}
Write-Host ''

# Serialize builds of this checkout across processes: /m:1 only serializes
# projects inside one MSBuild, so two concurrent runs would race on ISCC's
# package/stamp (and shared obj/PCH). Builds started from Visual Studio do
# not take this lock.
$lock = [System.Threading.Mutex]::new($false, (Get-BuildLockName -RepoRoot $repoRoot))
$exitCode = 1
try
{
    $acquired = $false
    try
    {
        $acquired = $lock.WaitOne(0)
        if (-not $acquired)
        {
            Write-Host 'Waiting for another run-msbuild.ps1 build of this checkout to finish (Ctrl+C to cancel)...'
            # Short timed waits: an unbounded native WaitOne() would not
            # return to PowerShell, so Ctrl+C could not stop the script.
            while (-not $acquired)
            {
                $acquired = $lock.WaitOne(1000)
            }
        }
    }
    catch [System.Threading.AbandonedMutexException]
    {
        # Previous holder died; ownership passes to us.
        $acquired = $true
    }

    try
    {
        & $msbuildExe $solution @($msbArgs.ToArray())
        $exitCode = if ($null -eq $LASTEXITCODE) { 0 } else { $LASTEXITCODE }
    }
    finally
    {
        if ($acquired) { $lock.ReleaseMutex() }
    }
}
finally
{
    $lock.Dispose()
}
exit $exitCode
