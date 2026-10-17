#Requires -Version 5.1
<#
.SYNOPSIS
    Build compile_commands.json at the repository root from MSVC CL.command.*.tlog files.

.DESCRIPTION
    Machine-local output (gitignored). Run after a Visual Studio / MSBuild build
    of Envy, HashLib, and TorrentEnvy for the chosen configuration.

    Rooting markers may list multiple sources separated by `|` (batched CL).
    Those are split so each translation unit gets its own compile_commands entry.

.PARAMETER Configuration
    MSBuild-style pair, e.g. Release|x64 (default) or Debug|x64.

.PARAMETER SelfTest
    Run offline parsing checks (no VS build / tlog required) and exit.
#>
param(
    [string]$Configuration = 'Release|x64',
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

function Select-Msvc145ToolsetDir
{
    param(
        [Parameter(Mandatory = $true)]
        [object[]]$VersionDirs
    )

    # Envy.sln PlatformToolset v145 -> MSVC 14.5x only. Accept only dotted numeric
    # folder names (e.g. 14.50.35710). Never cast non-numeric names to [version].
    $candidates = @(
        foreach ($dir in @($VersionDirs))
        {
            if ($null -eq $dir) { continue }
            $name = [string]$dir.Name
            if ($name -notmatch '^14\.5\d+(\.\d+)*$') { continue }
            [pscustomobject]@{
                Dir     = $dir
                Version = [version]$name
            }
        }
    )
    if ($candidates.Count -eq 0)
    {
        return $null
    }

    return (
        $candidates |
            Sort-Object -Property Version -Descending |
            Select-Object -First 1 |
            ForEach-Object { $_.Dir }
    )
}

function Find-MsvcClExe
{
    param(
        [string]$Configuration
    )

    $targetArch = 'x64'
    if ($Configuration -match '\|Win32$')
    {
        $targetArch = 'x86'
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere))
    {
        throw "vswhere.exe not found at: $vswhere. Install Visual Studio 2026 with the C++ workload."
    }

    $candidateInstances = @(& $vswhere -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath 2>$null)
    foreach ($candidate in $candidateInstances)
    {
        if ([string]::IsNullOrWhiteSpace($candidate)) { continue }

        $msvcRoot = Join-Path $candidate 'VC\Tools\MSVC'
        if (-not (Test-Path -LiteralPath $msvcRoot))
        {
            continue
        }

        # Skip toolset folders without the requested architecture's compiler
        # before choosing the newest one, so an incomplete newest folder
        # cannot shadow an older usable toolset.
        $versionDirs = @(Get-ChildItem -LiteralPath $msvcRoot -Directory -ErrorAction SilentlyContinue |
            Where-Object {
                Test-Path -LiteralPath (Join-Path $_.FullName "bin\Hostx64\$targetArch\cl.exe")
            })
        $versionDir = Select-Msvc145ToolsetDir -VersionDirs $versionDirs
        if ($versionDir)
        {
            return (Resolve-Path -LiteralPath (Join-Path $versionDir.FullName "bin\Hostx64\$targetArch\cl.exe")).Path
        }
    }

    throw "No Visual Studio installation with a usable MSVC 14.5x (v145) x86/x64 cl.exe was found (vswhere). Install Visual Studio 2026 with toolset v145."
}

function Split-TlogSourceMarker
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$Marker
    )

    # MSBuild batched CL rooting markers list sources separated by '|'.
    $parts = @(
        $Marker -split '\|' |
            ForEach-Object {
                $segment = $_.Trim()
                if ($segment.Length -ge 2 -and $segment.StartsWith('"') -and $segment.EndsWith('"'))
                {
                    $segment = $segment.Substring(1, $segment.Length - 2).Trim()
                }
                $segment
            } |
            Where-Object { $_ -ne '' }
    )
    if ($parts.Count -eq 0)
    {
        throw "Empty CL.command tlog source marker: '$Marker'"
    }
    return $parts
}

function Resolve-TlogSourcePath
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,
        [Parameter(Mandatory = $true)]
        [string]$Marker
    )

    try
    {
        return [System.IO.Path]::GetFullPath($Source)
    }
    catch
    {
        throw "Invalid CL.command tlog source path '$Source' in marker '$Marker': $($_.Exception.Message)"
    }
}

function Get-ClCommandForSource
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$ClExe,
        [Parameter(Mandatory = $true)]
        [string]$ClArgs,
        [Parameter(Mandatory = $true)]
        [string]$SourcePath,
        [Parameter(Mandatory = $true)]
        [string[]]$BatchSources
    )

    $argsOut = $ClArgs
    if ($BatchSources.Count -gt 1)
    {
        # Drop sibling TUs from a batched command line so clangd sees one file.
        foreach ($other in $BatchSources)
        {
            if ([string]::Equals($other, $SourcePath, [System.StringComparison]::OrdinalIgnoreCase))
            {
                continue
            }
            $leaf = [System.IO.Path]::GetFileName($other)
            foreach ($candidate in @($other, $leaf))
            {
                # MSVC tlogs often quote sources; optional quotes around the path.
                $pattern = '(?i)(?<=^|\s)"?' + [regex]::Escape($candidate) + '"?(?=\s|$)'
                $argsOut = [regex]::Replace($argsOut, $pattern, ' ')
            }
        }
        $argsOut = ($argsOut -replace '\s+', ' ').Trim()
        $leafSelf = [System.IO.Path]::GetFileName($SourcePath)
        if ($argsOut -notmatch [regex]::Escape($leafSelf) -and
            $argsOut -notmatch [regex]::Escape($SourcePath))
        {
            $argsOut = "$argsOut `"$SourcePath`""
        }
    }

    return "`"$ClExe`" $argsOut"
}

function Convert-TlogFilesToCompileCommands
{
    param(
        [Parameter(Mandatory = $true)]
        [System.IO.FileInfo[]]$TlogFiles,
        [Parameter(Mandatory = $true)]
        [string]$ClExe
    )

    $entries = @{}
    foreach ($tlog in $TlogFiles)
    {
        # MSVC layout: <Project>\<IntDir>\<Project>.tlog\CL.command.*.tlog
        $projectDir = $tlog.Directory.Parent.Parent.FullName
        # MSVC CL.command tlogs are UTF-16 LE (BOM); match production readers.
        $lines = Get-Content -LiteralPath $tlog.FullName -Encoding unicode
        for ($i = 0; $i -lt $lines.Count - 1; $i++)
        {
            $srcLine = $lines[$i]
            if (-not $srcLine.StartsWith('^')) { continue }
            $src = $srcLine.Substring(1).Trim()
            $clArgs = $lines[$i + 1].Trim()
            if ([string]::IsNullOrWhiteSpace($clArgs)) { continue }

            $srcParts = @(Split-TlogSourceMarker -Marker $src)
            $resolvedSources = @(
                foreach ($part in $srcParts)
                {
                    Resolve-TlogSourcePath -Source $part -Marker $src
                }
            )

            foreach ($srcPath in $resolvedSources)
            {
                $command = Get-ClCommandForSource -ClExe $ClExe -ClArgs $clArgs `
                    -SourcePath $srcPath -BatchSources $resolvedSources
                $entries[$srcPath] = [ordered]@{
                    directory = $projectDir
                    file      = $srcPath
                    command   = $command
                }
            }
        }
    }

    return $entries
}

function Write-CompileCommandsJson
{
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$Entries,
        [Parameter(Mandatory = $true)]
        [string]$OutPath
    )

    $list = @($Entries.Values)
    if ($list.Count -eq 1)
    {
        $json = '[' + (ConvertTo-Json -InputObject $list[0] -Depth 4 -Compress) + ']'
    }
    else
    {
        $json = ConvertTo-Json -InputObject $list -Depth 4
    }
    [System.IO.File]::WriteAllText($OutPath, $json, [System.Text.UTF8Encoding]::new($false))
}

function Invoke-GenerateCompileCommandsSelfTest
{
    # Mixed valid 14.5x folders, older toolsets, and non-numeric junk must not throw;
    # selection is numeric (not lexicographic) and picks the newest 14.5x only.
    $mixedNames = @(
        [pscustomobject]@{ Name = 'latest' }
        [pscustomobject]@{ Name = '14.44.35207' }
        [pscustomobject]@{ Name = '14.50.9' }
        [pscustomobject]@{ Name = 'not-a-version' }
        [pscustomobject]@{ Name = '14.50.10' }
        [pscustomobject]@{ Name = '14.5x-preview' }
        [pscustomobject]@{ Name = '14.51.0' }
        [pscustomobject]@{ Name = '' }
        [pscustomobject]@{ Name = 'MSVC' }
    )
    $picked = Select-Msvc145ToolsetDir -VersionDirs $mixedNames
    if ($null -eq $picked -or $picked.Name -ne '14.51.0')
    {
        throw "SelfTest: expected newest 14.5x '14.51.0', got '$($picked.Name)'"
    }
    # Lexicographic trap: '14.50.9' > '14.50.10' as strings; numeric sort must keep 14.50.10.
    $lexTrap = @(
        [pscustomobject]@{ Name = '14.50.9' }
        [pscustomobject]@{ Name = '14.50.10' }
        [pscustomobject]@{ Name = 'bogus' }
    )
    $lexPicked = Select-Msvc145ToolsetDir -VersionDirs $lexTrap
    if ($null -eq $lexPicked -or $lexPicked.Name -ne '14.50.10')
    {
        throw "SelfTest: numeric sort failed; expected '14.50.10', got '$($lexPicked.Name)'"
    }
    $none = Select-Msvc145ToolsetDir -VersionDirs @(
        [pscustomobject]@{ Name = '14.44.1' }
        [pscustomobject]@{ Name = 'tools' }
    )
    if ($null -ne $none)
    {
        throw "SelfTest: expected no 14.5x candidate, got '$($none.Name)'"
    }

    $single = @(Split-TlogSourceMarker -Marker 'C:\src\a.cpp')
    if ($single.Count -ne 1 -or $single[0] -ne 'C:\src\a.cpp')
    {
        throw 'SelfTest: single-source marker split failed'
    }

    $batch = @(Split-TlogSourceMarker -Marker 'C:\src\a.cpp|C:\src\b.cpp| C:\src\c.cpp ')
    if ($batch.Count -ne 3)
    {
        throw "SelfTest: expected 3 batch sources, got $($batch.Count)"
    }
    if ($batch[0] -ne 'C:\src\a.cpp' -or $batch[1] -ne 'C:\src\b.cpp' -or $batch[2] -ne 'C:\src\c.cpp')
    {
        throw 'SelfTest: batch marker split values mismatch'
    }

    $batchQuoted = @(Split-TlogSourceMarker -Marker '"C:\src\a.cpp"|"C:\src\b.cpp"| "C:\src\c.cpp" ')
    if ($batchQuoted.Count -ne 3)
    {
        throw "SelfTest: expected 3 quoted batch sources, got $($batchQuoted.Count)"
    }
    if ($batchQuoted[0] -ne 'C:\src\a.cpp' -or $batchQuoted[1] -ne 'C:\src\b.cpp' -or $batchQuoted[2] -ne 'C:\src\c.cpp')
    {
        throw 'SelfTest: quoted batch marker split values mismatch'
    }

    # Composite markers must not be passed whole to GetFullPath ('|' is illegal on Windows).
    $resolved = @(
        foreach ($part in $batch)
        {
            Resolve-TlogSourcePath -Source $part -Marker ($batch -join '|')
        }
    )
    if ($resolved.Count -ne 3)
    {
        throw 'SelfTest: Resolve-TlogSourcePath batch count mismatch'
    }

    $cmd = Get-ClCommandForSource -ClExe 'C:\cl.exe' `
        -ClArgs '/c /nologo C:\src\a.cpp C:\src\b.cpp' `
        -SourcePath 'C:\src\a.cpp' `
        -BatchSources @('C:\src\a.cpp', 'C:\src\b.cpp')
    if ($cmd -match [regex]::Escape('b.cpp'))
    {
        throw "SelfTest: batched command still references sibling TU: $cmd"
    }
    if ($cmd -notmatch [regex]::Escape('a.cpp'))
    {
        throw "SelfTest: batched command missing target TU: $cmd"
    }

    $cmdQuoted = Get-ClCommandForSource -ClExe 'C:\cl.exe' `
        -ClArgs '/c /nologo "C:\src\a.cpp" "C:\src\b.cpp"' `
        -SourcePath 'C:\src\a.cpp' `
        -BatchSources @('C:\src\a.cpp', 'C:\src\b.cpp')
    if ($cmdQuoted -match [regex]::Escape('b.cpp'))
    {
        throw "SelfTest: quoted batched command still references sibling TU: $cmdQuoted"
    }
    if ($cmdQuoted -notmatch [regex]::Escape('a.cpp'))
    {
        throw "SelfTest: quoted batched command missing target TU: $cmdQuoted"
    }

    # End-to-end: UTF-16 LE fixture through Convert-TlogFilesToCompileCommands + JSON.
    $fixtureRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
        'envy-compile-commands-selftest-' + [guid]::NewGuid().ToString('n'))
    try
    {
        $projName = 'FakeProj'
        $intDir = 'Release x64'
        $tlogDir = Join-Path $fixtureRoot "$projName\$intDir\$projName.tlog"
        New-Item -ItemType Directory -Path $tlogDir -Force | Out-Null
        $srcDir = Join-Path $fixtureRoot 'sources'
        New-Item -ItemType Directory -Path $srcDir -Force | Out-Null

        $srcA = [System.IO.Path]::GetFullPath((Join-Path $srcDir 'a.cpp'))
        $srcB = [System.IO.Path]::GetFullPath((Join-Path $srcDir 'b.cpp'))
        $srcC = [System.IO.Path]::GetFullPath((Join-Path $srcDir 'c.cpp'))
        $clExe = [System.IO.Path]::GetFullPath((Join-Path $fixtureRoot 'cl.exe'))

        $tlogPath = Join-Path $tlogDir 'CL.command.1.tlog'
        $tlogText = @(
            "^$srcA"
            "/c /nologo /Foa.obj $srcA"
            "^$srcB|$srcC"
            "/c /nologo $srcB $srcC"
            "^`"$srcA`"|`"$srcB`""
            "/c /nologo `"$srcA`" `"$srcB`""
        ) -join "`r`n"
        [System.IO.File]::WriteAllText($tlogPath, $tlogText + "`r`n", [System.Text.Encoding]::Unicode)

        $tlogFile = Get-Item -LiteralPath $tlogPath
        $entries = Convert-TlogFilesToCompileCommands -TlogFiles @($tlogFile) -ClExe $clExe
        $expectedProjectDir = [System.IO.Path]::GetFullPath((Join-Path $fixtureRoot $projName))

        if ($entries.Count -ne 3)
        {
            throw "SelfTest: expected 3 compile entries from fixture tlog, got $($entries.Count)"
        }

        foreach ($path in @($srcA, $srcB, $srcC))
        {
            if (-not $entries.ContainsKey($path))
            {
                throw "SelfTest: missing compile entry for $path"
            }
            $entry = $entries[$path]
            if ($entry.directory -ne $expectedProjectDir)
            {
                throw "SelfTest: directory mismatch for ${path}: got '$($entry.directory)', expected '$expectedProjectDir'"
            }
            if ($entry.file -ne $path)
            {
                throw "SelfTest: file mismatch for ${path}: got '$($entry.file)'"
            }
            if ($entry.command -notmatch [regex]::Escape($clExe))
            {
                throw "SelfTest: command missing cl.exe for ${path}: $($entry.command)"
            }
            if ($entry.command -notmatch [regex]::Escape([System.IO.Path]::GetFileName($path)))
            {
                throw "SelfTest: command missing target TU for ${path}: $($entry.command)"
            }
        }

        # Last batched record wins for a.cpp; sibling b.cpp must be stripped.
        $cmdA = $entries[$srcA].command
        if ($cmdA -match [regex]::Escape([System.IO.Path]::GetFileName($srcB)))
        {
            throw "SelfTest: e2e batched command for a.cpp still references b.cpp: $cmdA"
        }

        $cmdB = $entries[$srcB].command
        if ($cmdB -match [regex]::Escape([System.IO.Path]::GetFileName($srcC)))
        {
            throw "SelfTest: e2e batched command for b.cpp still references c.cpp: $cmdB"
        }
        if ($cmdB -notmatch [regex]::Escape([System.IO.Path]::GetFileName($srcB)))
        {
            throw "SelfTest: e2e batched command for b.cpp missing b.cpp: $cmdB"
        }

        $outJson = Join-Path $fixtureRoot 'compile_commands.json'
        Write-CompileCommandsJson -Entries $entries -OutPath $outJson
        $parsed = Get-Content -LiteralPath $outJson -Raw -Encoding utf8 | ConvertFrom-Json
        if (@($parsed).Count -ne 3)
        {
            throw "SelfTest: JSON entry count mismatch: $(@($parsed).Count)"
        }
        foreach ($item in @($parsed))
        {
            if ([string]::IsNullOrWhiteSpace($item.directory) -or
                [string]::IsNullOrWhiteSpace($item.file) -or
                [string]::IsNullOrWhiteSpace($item.command))
            {
                throw "SelfTest: JSON entry missing required fields: $($item | ConvertTo-Json -Compress)"
            }
        }
    }
    finally
    {
        if (Test-Path -LiteralPath $fixtureRoot)
        {
            Remove-Item -LiteralPath $fixtureRoot -Recurse -Force
        }
    }

    Write-Host 'generate-compile-commands.ps1 -SelfTest OK'
}

if ($SelfTest)
{
    Invoke-GenerateCompileCommandsSelfTest
    return
}

$repoRoot = Get-RepoRoot
$clExe = Find-MsvcClExe -Configuration $Configuration

$configMap = @{
    'Release|x64'   = @('Release x64')
    'Debug|x64'     = @('Debug x64')
    'Release|Win32' = @('Release Win32')
    'Debug|Win32'   = @('Debug Win32')
}
if (-not $configMap.ContainsKey($Configuration))
{
    throw "Unknown configuration '$Configuration'. Use Release|x64, Debug|x64, etc."
}

$intDirNames = $configMap[$Configuration]
$tlogFiles = @()
foreach ($dir in $intDirNames)
{
    $searchRoots = @(
        Join-Path $repoRoot "Envy\$dir"
        Join-Path $repoRoot "HashLib\$dir"
        Join-Path $repoRoot "TorrentEnvy\$dir"
    )
    foreach ($root in $searchRoots)
    {
        if (Test-Path -LiteralPath $root)
        {
            $tlogFiles += Get-ChildItem -LiteralPath $root -Recurse -Filter 'CL.command.*.tlog' -ErrorAction SilentlyContinue
        }
    }
}

if ($tlogFiles.Count -eq 0)
{
    throw "No CL.command.*.tlog found. Build Envy in Visual Studio ($Configuration) first."
}

$entries = Convert-TlogFilesToCompileCommands -TlogFiles $tlogFiles -ClExe $clExe
$outPath = Join-Path $repoRoot 'compile_commands.json'
Write-CompileCommandsJson -Entries $entries -OutPath $outPath
Write-Host "Wrote $($entries.Count) compile commands to $outPath (from $($tlogFiles.Count) tlog file(s), config $Configuration)."
