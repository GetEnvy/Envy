# Contributing Guide

Thank you for your interest in contributing to Envy! This guide explains how to contribute to the project effectively.

## 📋 Ways to Contribute

### Code Contributions
- Bug fixes and improvements
- New features and enhancements
- Performance optimizations
- Security improvements

### Non-Code Contributions
- Documentation improvements
- Bug reports and testing
- Design and user experience
- Translation and localization

## 🚀 Getting Started

### 1. Choose an Issue
- Check [GitHub Issues](../../issues) for open tasks
- Look for issues labeled `good first issue` or `help wanted`
- Comment on the issue to indicate you're working on it

### 2. Fork and Clone
```bash
# Fork the repository on GitHub
# Then clone your fork
git clone https://github.com/YOUR_USERNAME/Envy.git
cd Envy
```

### 3. Set Up Development Environment
- Follow the [Development Guide](guide.md)
- Build with Visual Studio (`Visual Studio\Envy.sln`) or `.\build_all.ps1`
- Optionally run `tests\run_integration_tests.bat` to verify setup

### 4. Create a Feature Branch
```bash
# Create and switch to a new branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/issue-number-description
```

## 💻 Development Process

### Code Changes
1. **Make incremental changes** - Small, focused commits
2. **Test frequently** - Build and test after each change
3. **Follow code standards** - See [Code Standards](standards.md)
4. **Update documentation** - If your changes affect docs

### Commit Guidelines
```bash
# Good commit messages
git commit -m "Fix Kademlia XOR distance calculation bug"

git commit -m "Add IPv6 support to connection handling"

git commit -m "Update documentation for new feature"

# Bad commit messages
git commit -m "Fix bug"
git commit -m "Update code"
git commit -m "Changes"
```

### Testing Your Changes
- **Build verification**: `.\build_all.ps1` (Debug/Release × Win32/x64)
- **Formatting**: run clang-format (config: `.clang-format`) via IDE or clang-format executable
- **Integration tests (manual)**: `tests\run_integration_tests.bat`

## 🔄 Pull Request Process

### Before Submitting
- ✅ Build succeeds (`.\build_all.ps1` or equivalent)
- ✅ Code follows [standards](standards.md); formatted with clang-format
- ✅ No compiler warnings
- ✅ Docs updated if behaviour or setup changes

### Creating a Pull Request
1. **Push your branch** to your fork
2. **Create PR** on GitHub from your branch to `develop` (or default branch)
3. **Fill out the PR template**
4. **Link related issues** if applicable

### PR Template Requirements
- **Title**: Clear, descriptive summary
- **Description**: Detailed explanation of changes
- **Testing**: How you tested the changes
- **Screenshots**: For UI changes
- **Breaking Changes**: If any backward compatibility breaks

### Review Process
1. **Automated checks** run (build, tests, formatting)
2. **Code review** by maintainers
3. **Feedback addressed** and changes made
4. **Approval and merge** when ready

## 🎯 Areas for Contribution

### High Priority
- **Protocol fixes**: Kademlia, CryptLayer, SecureID
- **Security**: Vulnerability fixes, input validation
- **Performance**: Memory usage, CPU optimization
- **Testing**: Unit tests, integration tests

### Medium Priority
- **UI improvements**: Modern Windows features, accessibility
- **Documentation**: User guides, API docs, tutorials
- **Build system**: CMake improvements, CI/CD
- **Code quality**: Refactoring, modernization

### Future Opportunities
- **New protocols**: WebRTC, QUIC, modern P2P
- **Cross-platform**: Linux/macOS support
- **Advanced features**: AI optimization, cloud integration
- **Plugin ecosystem**: New plugin types and APIs

## 📏 Code Standards

### Language and Style
- **C++17** baseline (C++20 planned)
- **MFC conventions** followed
- **Unicode** (UTF-16) throughout
- **Smart pointers** for memory management

### Naming Conventions
```cpp
class CDownloadTask;        // Classes: PascalCase with C prefix
void OnDownload();          // Functions: PascalCase
CString m_sFileName;        // Members: m_s* for strings in this codebase
```

### Code Organization
- **Header guards** or `#pragma once`
- **Logical grouping** of functions
- **Clear comments** for complex logic
- **Error handling** with exceptions

See [Code Standards](standards.md) for complete guidelines.

## 🐛 Reporting Bugs

### Bug Report Template
Use the [bug report template](../../issues/new?template=bug_report.yml) and include:

- **Clear title** describing the issue
- **Steps to reproduce** the problem
- **Expected behavior** vs actual behavior
- **Environment**: OS, Visual Studio version, build type
- **Logs or error messages** if applicable
- **Screenshots** for UI issues

### Security Issues
- **DO NOT** create public issues for security vulnerabilities
- **Email** maintainers directly with details
- **Wait for response** before public disclosure

## 📚 Documentation

### Improving Documentation
- Fix typos and unclear explanations
- Add missing information
- Update outdated content
- Translate to new languages

### Documentation Standards
- Use Markdown format
- Include code examples where helpful
- Keep language clear and concise
- Update table of contents when adding sections

## 🌍 Translation

### Adding Languages
1. Copy an existing `.po` file from `Languages/`
2. Update the language code and name
3. Translate the strings
4. Test the translation in the application

### Translation Guidelines
- Maintain technical accuracy
- Use appropriate terminology
- Consider cultural context
- Test for text length (UI may need adjustment)

## 🤝 Community Guidelines

### Communication
- Be respectful and constructive
- Use clear, professional language
- Provide context for questions
- Help others when possible

### Getting Help
- **Documentation**: Check docs first
- **Issues**: Search existing issues
- **Discussions**: Use for questions and ideas
- **Discord/Slack**: For real-time chat (if available)

## 🎖️ Recognition

Contributors are recognized through:
- **GitHub contributor statistics**
- **Changelog entries** for significant contributions
- **Credits in documentation**
- **Community recognition**

## 📞 Support

### For Contributors
- **General questions**: [GitHub Discussions](../../discussions)
- **Technical issues**: [GitHub Issues](../../issues)
- **Security issues**: Contact maintainers privately

### For Maintainers
- **Review requests**: Ping specific maintainers
- **Process questions**: Check this guide or ask in discussions
- **Tooling issues**: Report in infrastructure issues

## 📋 Checklist

### Before Starting Work
- [ ] Issue selected and assigned
- [ ] Development environment set up
- [ ] Repository forked and cloned
- [ ] Feature branch created

### During Development
- [ ] Changes tested frequently
- [ ] Code follows standards
- [ ] Documentation updated
- [ ] Commits are focused and descriptive

### Before Submitting
- [ ] Build succeeds; integration tests run if relevant
- [ ] Code formatted (clang-format)
- [ ] No compiler warnings
- [ ] PR template completed; related issues linked

---

**Related:** [Guide](guide.md) · [Build](build.md) · [Standards](standards.md) · [Status](status.md)

**Last Updated:** January 2026
