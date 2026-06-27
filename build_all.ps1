# Build all configurations for Envy project
Write-Host "Building all configurations for Envy project..."

$builds = @(
    @{Config="Debug"; Platform="Win32"},
    @{Config="Debug"; Platform="x64"},
    @{Config="Release"; Platform="Win32"},
    @{Config="Release"; Platform="x64"}
)

foreach ($build in $builds) {
    $config = $build.Config
    $platform = $build.Platform
    Write-Host "Starting build: $config $platform"

    $process = Start-Process -FilePath "MSBuild.exe" -ArgumentList "Visual Studio\Envy.sln", "/p:Configuration=$config", "/p:Platform=$platform", "/verbosity:minimal", "/m:4" -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -eq 0) {
        Write-Host "✓ Build successful: $config $platform" -ForegroundColor Green
    } else {
        Write-Host "✗ Build failed: $config $platform (Exit code: $($process.ExitCode))" -ForegroundColor Red
    }
}

Write-Host "All builds completed!"
