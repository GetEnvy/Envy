# Local Setup

## Requirements
- Windows host (native) for full build + runtime validation
- Visual Studio with:
  - Desktop development with C++
  - MFC/ATL components
  - Windows SDK 10.x
- NuGet (for restore step used in workflows)

## Clone and Build
```bash
git clone <repo-url>
cd Envy
```

### Visual Studio path (recommended)
1. Open `Visual Studio/Envy.sln`.
2. Restore NuGet packages if prompted.
3. Build `Release|x64` (or desired configuration).

### CMake path (limited)
```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Runtime Assets
The app expects data/resources from repository folders such as `Data/`, `Languages/`, `Skins/`, and service DLL outputs. Visual Studio post-build steps copy selected assets (e.g., `Vendors.xml`) into output directories.

## Environment Variables
No mandatory project-wide env var file is currently defined in-repo. Most configuration appears to be runtime settings managed by the application itself.
