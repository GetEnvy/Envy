# Retarget Envy.sln and all .vcxproj files to Visual Studio 2026 / v145.
# PowerShell version of SetVS2026.bat - safer on CI, cross-host, idempotent.
#
# Usage:
#   pwsh -File SetVS2026.ps1
#   pwsh -File SetVS2026.ps1 -DryRun
#   pwsh -File SetVS2026.ps1 -Toolset v143    # downgrade for VS 2022

[CmdletBinding()]
param(
    [string]$Toolset = 'v145',
    [string]$SolutionHeader = '# Visual Studio Version 18',
    [string]$WindowsTargetPlatformVersion = '10.0',
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot   = Split-Path -Parent $scriptRoot
$solution   = Join-Path $scriptRoot 'Envy.sln'

if (-not (Test-Path $solution)) {
    Write-Error "Envy.sln not found next to this script ($solution)."
    exit 1
}

Write-Host "Repo root      : $repoRoot"
Write-Host "Target toolset : $Toolset"
Write-Host "Solution header: $SolutionHeader"
Write-Host "Target SDK     : $WindowsTargetPlatformVersion"
if ($DryRun) { Write-Host '[dry-run mode]' -ForegroundColor Yellow }

# --- Patch the solution -----------------------------------------------------
$slnLines = Get-Content -LiteralPath $solution -Encoding UTF8
$newSln   = @(
    'Microsoft Visual Studio Solution File, Format Version 12.00',
    $SolutionHeader
) + $slnLines[2..($slnLines.Length - 1)]

if (-not $DryRun) {
    [System.IO.File]::WriteAllLines($solution, $newSln, [System.Text.UTF8Encoding]::new($true))
    Write-Host "Patched solution header."
} else {
    Write-Host "Would patch solution header."
}

# --- Patch every .vcxproj ---------------------------------------------------
$projects = Get-ChildItem -LiteralPath $repoRoot -Recurse -Filter '*.vcxproj' `
    | Where-Object { $_.FullName -notmatch [regex]::Escape([IO.Path]::DirectorySeparatorChar + 'PluginWizard' + [IO.Path]::DirectorySeparatorChar) }

$totalRepl = 0
$touched   = 0

foreach ($p in $projects) {
    $content = Get-Content -LiteralPath $p.FullName -Raw -Encoding UTF8

    $replaced = [regex]::Replace(
        $content,
        '<PlatformToolset>[^<]+</PlatformToolset>',
        "<PlatformToolset>$Toolset</PlatformToolset>"
    )

    # Remove _ATL_XP_TARGETING preprocessor define (XP no longer supported).
    $replaced = $replaced -replace '_ATL_XP_TARGETING;', ''

    # Remove ENVY_USE_ASM define - x86-only inline asm doesn't help on v145.
    $replaced = $replaced -replace 'ENVY_USE_ASM;', ''

    # Inject WindowsTargetPlatformVersion if missing.
    if ($replaced -notmatch '<WindowsTargetPlatformVersion>') {
        $replaced = [regex]::Replace(
            $replaced,
            '(<PropertyGroup\s+Label="Globals">)',
            "`$1`r`n    <WindowsTargetPlatformVersion>$WindowsTargetPlatformVersion</WindowsTargetPlatformVersion>",
            1
        )
    }

    if ($replaced -ne $content) {
        $touched++
        $diff = ([regex]::Matches($content, '<PlatformToolset>[^<]+</PlatformToolset>')).Count
        $totalRepl += $diff

        if (-not $DryRun) {
            # Preserve the BOM the file already had so MSBuild stays happy.
            $utf8WithBom = [System.Text.UTF8Encoding]::new($true)
            [System.IO.File]::WriteAllText($p.FullName, $replaced, $utf8WithBom)
        }
        Write-Host ("{0}  ({1} toolset replacements)" -f $p.FullName, $diff)
    }
}

Write-Host ''
Write-Host '============================================================================'
Write-Host (" Projects touched           : {0}" -f $touched)
Write-Host (" PlatformToolset rewrites   : {0}  ->  {1}" -f $totalRepl, $Toolset)
Write-Host '============================================================================'
if ($DryRun) {
    Write-Host 'Re-run without -DryRun to apply changes.' -ForegroundColor Yellow
}
