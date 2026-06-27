# Auto-Version Management for Envy Project
# Automatically increments version numbers based on git history and build type

param(
    [Parameter(Mandatory=$false)]
    [string]$VersionFile = "version.json",
    [Parameter(Mandatory=$false)]
    [switch]$IncrementPatch,
    [Parameter(Mandatory=$false)]
    [switch]$IncrementMinor,
    [Parameter(Mandatory=$false)]
    [switch]$IncrementMajor,
    [Parameter(Mandatory=$false)]
    [switch]$PreRelease,
    [Parameter(Mandatory=$false)]
    [string]$PreReleaseLabel = "dev",
    [Parameter(Mandatory=$false)]
    [switch]$UpdateFiles
)

# Configuration
$VersionFilePath = Join-Path $PSScriptRoot ".." $VersionFile
$ProjectRoot = Split-Path $PSScriptRoot -Parent

function Update-ProjectFiles {
    param($VersionData)

    Write-Host "Updating project files..." -ForegroundColor Yellow

    # Update CMakeLists.txt
    $cmakePath = Join-Path $ProjectRoot "CMakeLists.txt"
    if (Test-Path $cmakePath) {
        $cmakeContent = Get-Content $cmakePath -Raw
        $cmakeContent = $cmakeContent -replace 'project\(Envy VERSION [\d\.]+\)', "project(Envy VERSION $($VersionData.major).$($VersionData.minor).$($VersionData.patch))"
        Set-Content $cmakePath $cmakeContent -Encoding UTF8
        Write-Host "  Updated CMakeLists.txt" -ForegroundColor Gray
    }

    # Update Visual Studio version script
    $vsScriptPath = Join-Path $ProjectRoot "Visual Studio" "SetReleaseVersion.bat"
    if (Test-Path $vsScriptPath) {
        $scriptContent = Get-Content $vsScriptPath -Raw
        $scriptContent = $scriptContent -replace 'set "version=[\d\.]+"', "set `"version=$($VersionData.major).$($VersionData.minor)`""
        $scriptContent = $scriptContent -replace 'set "internalver=[\d]+"', "set `"internalver=$($VersionData.major)$($VersionData.minor)`""
        Set-Content $vsScriptPath $scriptContent -Encoding UTF8
        Write-Host "  Updated SetReleaseVersion.bat" -ForegroundColor Gray
    }

    # Update Envy.h if it exists
    $envyHeaderPath = Join-Path $ProjectRoot "Envy" "Envy.h"
    if (Test-Path $envyHeaderPath) {
        $headerContent = Get-Content $envyHeaderPath -Raw
        Write-Host "  Note: Manual update may be needed for Envy.h version defines" -ForegroundColor Yellow
    }

    Write-Host "Project files updated successfully!" -ForegroundColor Green
}

# Ensure we're in the project root
Push-Location $ProjectRoot

try {
    # Get git information
    $commitCount = git rev-list --count HEAD 2>$null
    if ($LASTEXITCODE -ne 0) { $commitCount = "0" }

    $currentBranch = git branch --show-current 2>$null
    if ($LASTEXITCODE -ne 0) { $currentBranch = "unknown" }

    $latestTag = git describe --tags --abbrev=0 2>$null
    if ($LASTEXITCODE -ne 0) { $latestTag = "v0.0.0" }

    $isDirty = git status --porcelain 2>$null
    $hasUncommitted = if ($isDirty) { $true } else { $false }

    # Load or create version file
    if (Test-Path $VersionFilePath) {
        $versionData = Get-Content $VersionFilePath | ConvertFrom-Json
    } else {
        $versionData = @{
            major = 4
            minor = 1
            patch = 0
            build = 0
            preRelease = $null
            lastUpdated = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ssZ")
            git = @{
                commitCount = 0
                lastTag = "v0.0.0"
                branch = "main"
            }
        }
    }

    # Parse current version
    $currentVersion = [version]"$($versionData.major).$($versionData.minor).$($versionData.patch)"

    # Increment version based on parameters
    if ($IncrementMajor) {
        $versionData.major++
        $versionData.minor = 0
        $versionData.patch = 0
        $versionData.preRelease = $null
    } elseif ($IncrementMinor) {
        $versionData.minor++
        $versionData.patch = 0
        $versionData.preRelease = $null
    } elseif ($IncrementPatch) {
        $versionData.patch++
        $versionData.preRelease = $null
    }

    # Set pre-release if requested
    if ($PreRelease) {
        $versionData.preRelease = $PreReleaseLabel
    }

    # Update build number based on commits
    $versionData.build = [int]$commitCount

    # Update git information
    $versionData.git.commitCount = [int]$commitCount
    $versionData.git.lastTag = $latestTag
    $versionData.git.branch = $currentBranch
    $versionData.lastUpdated = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ssZ")

    # Create version strings
    $baseVersion = "$($versionData.major).$($versionData.minor).$($versionData.patch)"
    $fullVersion = $baseVersion
    if ($versionData.preRelease) {
        $fullVersion += "-$($versionData.preRelease)"
    }
    if ($hasUncommitted) {
        $fullVersion += "+dirty"
    }

    $versionData.fullVersion = $fullVersion
    $versionData.displayVersion = "$baseVersion.$($versionData.build)"

    # Save version file
    $versionData | ConvertTo-Json -Depth 10 | Set-Content $VersionFilePath -Encoding UTF8

    # Update project files if requested
    if ($UpdateFiles) {
        Update-ProjectFiles -VersionData $versionData
    }

    # Output version information
    Write-Host "Version Information:" -ForegroundColor Green
    Write-Host "  Base Version: $baseVersion" -ForegroundColor Cyan
    Write-Host "  Full Version: $fullVersion" -ForegroundColor Cyan
    Write-Host "  Display Version: $($versionData.displayVersion)" -ForegroundColor Cyan
    Write-Host "  Build Number: $($versionData.build)" -ForegroundColor Cyan
    Write-Host "  Git Commits: $commitCount" -ForegroundColor Cyan
    Write-Host "  Branch: $currentBranch" -ForegroundColor Cyan
    if ($hasUncommitted) {
        Write-Host "  Status: Working directory has uncommitted changes" -ForegroundColor Yellow
    }

} finally {
    Pop-Location
}
