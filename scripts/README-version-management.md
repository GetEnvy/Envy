# Version Management System

This directory contains scripts for automatic version management in the Envy project.

## Overview

The version management system automatically increments version numbers based on:
- Git commit count (build number)
- Semantic versioning (major.minor.patch)
- Pre-release labels
- Branch information

## Files

- `auto-version.ps1` - Core version management script
- `bump-version.ps1` - Interactive version bumping tool
- `README-version-management.md` - This documentation

## Version File Format

The `version.json` file contains:

```json
{
  "major": 4,
  "minor": 1,
  "patch": 0,
  "build": 53,
  "preRelease": null,
  "fullVersion": "4.1.0",
  "displayVersion": "4.1.0.53",
  "lastUpdated": "2026-01-15T00:00:00Z",
  "git": {
    "commitCount": 53,
    "lastTag": "v4.1.0",
    "branch": "develop"
  }
}
```

## Usage

### Automatic (CI/CD)

Version management is automatically handled in GitHub Actions:
- **Build workflow**: Updates build numbers on every build
- **Release workflow**: Increments patch version on releases
- **Version management workflow**: Manual version control

### Manual (Local Development)

#### Interactive Mode
```powershell
.\scripts\bump-version.ps1 -Interactive
```

#### Command Line
```powershell
# Patch version (bug fixes)
.\scripts\bump-version.ps1 -Type patch -UpdateFiles

# Minor version (new features)
.\scripts\bump-version.ps1 -Type minor -UpdateFiles

# Major version (breaking changes)
.\scripts\bump-version.ps1 -Type major -UpdateFiles

# Pre-release
.\scripts\bump-version.ps1 -Type prerelease -PreReleaseLabel beta -UpdateFiles
```

#### Advanced Usage
```powershell
# Just update build number without incrementing version
.\scripts\auto-version.ps1

# Increment and update project files
.\scripts\auto-version.ps1 -IncrementPatch -UpdateFiles
```

## Version Increment Rules

### Automatic Build Numbers
- Build number = Git commit count
- Updated on every commit/build
- Never decrements

### Semantic Versioning
- **Patch** (`x.y.z` → `x.y.z+1`): Bug fixes, no API changes
- **Minor** (`x.y.z` → `x.(y+1).0`): New features, backward compatible
- **Major** (`x.y.z` → `(x+1).0.0`): Breaking changes
- **Pre-release**: Adds label (e.g., `1.2.3-dev`)

## Integration

### Project Files Updated
When `-UpdateFiles` is used, the following files are automatically updated:
- `CMakeLists.txt` - Project version
- `Visual Studio\SetReleaseVersion.bat` - Version variables
- `Envy.h` - Version defines (may need manual adjustment)

### GitHub Actions
- **build.yml**: Updates build numbers on every build
- **release.yml**: Increments patch version on releases
- **version-management.yml**: Manual version control workflow

### CI/CD Integration
```yaml
- name: Auto-Version Management
  shell: pwsh
  run: |
    .\scripts\auto-version.ps1 -UpdateFiles
```

## Best Practices

### When to Bump Versions

#### Patch Version
- Bug fixes
- Performance improvements
- Documentation updates
- Internal refactoring

#### Minor Version
- New features
- API additions (backward compatible)
- Enhanced functionality

#### Major Version
- Breaking API changes
- Significant architectural changes
- Major feature overhauls

### Pre-release Labels
- `dev`: Development builds
- `beta`: Beta testing
- `rc`: Release candidate
- `alpha`: Early testing

### Commit Messages
Version bumps should be committed with:
```
chore: bump version to 4.1.0.53
```

## Troubleshooting

### Version File Issues
```powershell
# Reset version file
Remove-Item version.json
.\scripts\auto-version.ps1 -UpdateFiles
```

### Git Integration Issues
```powershell
# Check git status
git status

# Reset local changes
git checkout -- version.json
```

### Build Integration Issues
- Ensure PowerShell execution policy allows script running
- Check file permissions on version.json
- Verify git is available in PATH

## Examples

### Development Workflow
```powershell
# Start new feature branch
git checkout -b feature/new-feature

# Make changes...

# Bump minor version for new feature
.\scripts\bump-version.ps1 -Type minor -UpdateFiles

# Commit changes
git add .
git commit -m "feat: implement new feature

Bump version to 4.2.0"
```

### Release Workflow
```powershell
# Prepare release
.\scripts\bump-version.ps1 -Type patch -UpdateFiles

# Create release commit
git add .
git commit -m "chore: prepare release 4.1.1"

# Create tag
git tag v4.1.1
git push origin v4.1.1
```

## Migration from Manual Versioning

### Previous System
- Manual updates to `SetReleaseVersion.bat`
- Hardcoded version numbers
- Manual build number management

### New System Benefits
- Automatic build number management
- Consistent version formatting
- Git integration
- CI/CD automation
- Semantic versioning compliance

### Migration Steps
1. Run `.\scripts\auto-version.ps1 -UpdateFiles` to initialize
2. Update any hardcoded version references
3. Test build process
4. Update documentation references
