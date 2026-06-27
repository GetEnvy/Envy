# Version Bump Script for Envy Project
# Interactive script for bumping versions locally

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("patch", "minor", "major", "prerelease")]
    [string]$Type = "patch",

    [Parameter(Mandatory=$false)]
    [string]$PreReleaseLabel = "dev",

    [Parameter(Mandatory=$false)]
    [switch]$UpdateFiles,

    [Parameter(Mandatory=$false)]
    [switch]$Interactive
)

$ProjectRoot = Split-Path $PSScriptRoot -Parent
$VersionScript = Join-Path $PSScriptRoot "auto-version.ps1"

# Ensure we're in the project root
Push-Location $ProjectRoot

try {
    if ($Interactive) {
        Write-Host "Envy Project Version Bump Tool" -ForegroundColor Green
        Write-Host "================================" -ForegroundColor Green
        Write-Host ""

        # Show current version
        if (Test-Path "version.json") {
            $currentVersion = Get-Content "version.json" | ConvertFrom-Json
            Write-Host "Current Version: $($currentVersion.displayVersion)" -ForegroundColor Cyan
            Write-Host "Full Version: $($currentVersion.fullVersion)" -ForegroundColor Cyan
            Write-Host ""
        }

        # Interactive menu
        Write-Host "Select version bump type:" -ForegroundColor Yellow
        Write-Host "1. Patch (bug fixes) - 1.2.3 → 1.2.4"
        Write-Host "2. Minor (new features) - 1.2.3 → 1.3.0"
        Write-Host "3. Major (breaking changes) - 1.2.3 → 2.0.0"
        Write-Host "4. Pre-release - 1.2.3 → 1.2.3-dev"
        Write-Host ""

        $choice = Read-Host "Enter your choice (1-4)"
        switch ($choice) {
            "1" { $Type = "patch" }
            "2" { $Type = "minor" }
            "3" { $Type = "major" }
            "4" {
                $Type = "prerelease"
                $PreReleaseLabel = Read-Host "Enter pre-release label (default: dev)"
                if (-not $PreReleaseLabel) { $PreReleaseLabel = "dev" }
            }
            default {
                Write-Host "Invalid choice. Exiting." -ForegroundColor Red
                exit 1
            }
        }

        $updateFilesChoice = Read-Host "Update project files? (y/n, default: y)"
        if ($updateFilesChoice -eq "n" -or $updateFilesChoice -eq "N") {
            $UpdateFiles = $false
        } else {
            $UpdateFiles = $true
        }
    }

    # Build the command
    $command = "& '$VersionScript'"

    switch ($Type) {
        "patch" { $command += " -IncrementPatch" }
        "minor" { $command += " -IncrementMinor" }
        "major" { $command += " -IncrementMajor" }
        "prerelease" {
            $command += " -PreRelease -PreReleaseLabel '$PreReleaseLabel'"
        }
    }

    if ($UpdateFiles) {
        $command += " -UpdateFiles"
    }

    Write-Host "Executing: $command" -ForegroundColor Yellow
    Write-Host ""

    # Execute the command
    Invoke-Expression $command

    # Show final version
    if (Test-Path "version.json") {
        $finalVersion = Get-Content "version.json" | ConvertFrom-Json
        Write-Host ""
        Write-Host "✓ Version successfully updated!" -ForegroundColor Green
        Write-Host "  New Version: $($finalVersion.displayVersion)" -ForegroundColor Cyan
        Write-Host "  Full Version: $($finalVersion.fullVersion)" -ForegroundColor Cyan
        Write-Host "  Build Number: $($finalVersion.build)" -ForegroundColor Cyan

        if ($Interactive) {
            Write-Host ""
            $commitChoice = Read-Host "Commit these changes? (y/n, default: n)"
            if ($commitChoice -eq "y" -or $commitChoice -eq "Y") {
                # Check git status
                $gitStatus = git status --porcelain
                if ($gitStatus) {
                    Write-Host "Committing version changes..." -ForegroundColor Yellow
                    git add .
                    $commitMessage = "chore: bump version to $($finalVersion.displayVersion)"
                    git commit -m $commitMessage
                    Write-Host "✓ Changes committed!" -ForegroundColor Green

                    $pushChoice = Read-Host "Push to remote? (y/n, default: n)"
                    if ($pushChoice -eq "y" -or $pushChoice -eq "Y") {
                        git push
                        Write-Host "✓ Changes pushed to remote!" -ForegroundColor Green
                    }
                } else {
                    Write-Host "No changes to commit." -ForegroundColor Gray
                }
            }
        }
    }

} catch {
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}

# Usage examples
if ($Interactive -and $MyInvocation.ScriptName -eq $MyInvocation.MyCommand.Path) {
    Write-Host ""
    Write-Host "Usage Examples:" -ForegroundColor Gray
    Write-Host "  .\bump-version.ps1 -Type patch -UpdateFiles" -ForegroundColor Gray
    Write-Host "  .\bump-version.ps1 -Interactive" -ForegroundColor Gray
    Write-Host "  .\bump-version.ps1 -Type prerelease -PreReleaseLabel beta" -ForegroundColor Gray
}
