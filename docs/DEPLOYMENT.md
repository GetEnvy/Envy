# Deployment and Release

## Environments
- **Development:** local Windows builds via Visual Studio
- **CI validation:** GitHub Actions workflows for build, quality, security scans
- **Release packaging:** installer and artifacts produced from solution projects/workflows

## CI/CD
Current repository includes workflows for:
- Build and test
- Code quality/static checks
- CodeQL security scanning
- Release packaging/version steps

## Release Process (Current)
1. Update version metadata (`version.json` / scripts as needed).
2. Build release configuration for target architectures.
3. Run tests and security checks.
4. Publish artifacts and release notes.

## Rollback Procedure (Recommended baseline)
1. Identify bad release tag/build.
2. Repoint distribution to last known good artifact.
3. Publish rollback notice and known-issues update.
4. Open remediation issue with root cause and verification checklist.

## Operational Notes
- This repo is Windows-centric; reproducible release validation should occur on Windows runners/hosts.
