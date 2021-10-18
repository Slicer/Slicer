# Windows

## Install prerequisites

- [CMake](http://www.cmake.org/cmake/resources/software.html) version >= 3.21.1 (or 3.15.1 <= version < 3.20.4)
- [Git](https://git-scm.com/download/win) >= 1.7.10
  - Note: CMake must be able to find `git.exe` and `patch.exe`. If git is installed in the default location then they may be found there, but if they are not found then either add the folder that contains them to `PATH` environment variable; or set `GIT_EXECUTABLE` and `Patch_EXECUTABLE` as environment variables or as CMake variables at configure time.
- [Visual Studio](https://visualstudio.microsoft.com/downloads/)
  - any edition can be used (including the free Community edition)
  - when configuring the installer, enable `Desktop development with C++` and in installation details, check `MSVC v142 - VS2019 C++ x64...` (Visual Studio 2019 v142 toolset with 64-bit support) - in some distributions, this option is not enabled by default
- [Qt5](https://www.qt.io/download-open-source): Download Qt universal installer and install Qt 5.15.2 components: `MSVC2019 64-bit`, `Qt Script`, `Qt WebEngine`. Installing Sources and Qt Debug Information Files are recommended for debugging (they allow stepping into Qt files with the debugger in debug-mode builds).
  - Note: These are all free, open-source components with LGPL license which allow free usage for any purpose, for any individuals or companies.
- [NSIS](http://nsis.sourceforge.net/Download) (optional): Needed if packaging Slicer. Make sure you install the language packs.

:::{note}

**Other Visual Studio IDE and compiler toolset versions**

- Visual Studio 2022 (v143) toolset is occasionally tested and appears to work well.
  When configuring the Visual Studio Installer, enable `Desktop development with C++` and in installation details, check `MSVC v143 - VS2022 C++ x64...` (Visual Studio 2022 v143 toolset with 64-bit support) - in some distributions, this option is not enabled by default.
  Use `Visual Studio 17 2022` instead of `Visual Studio 16 2019` in the build instructions below.
- Visual Studio 2017 (v141) toolset is not tested anymore but probably still works. Qt-5.15.2 requires v142 redistributables, so either these extra DLL files need to be added to the installation package or each user may need to install "Microsoft Visual C++ Redistributable" package.
- Cygwin and Mingw: not tested and not recommended. Building with cygwin gcc not supported, but the cygwin shell environment can be used to run utilities such as git.

:::

## Set up source and build folders

- Create source folder. This folder will be referred to as `<Slicer_SOURCE>` in the following. Recommended path: `C:\D\S4`
  - Due to maximum path length limitations during build the build process, source and build folders must be located in a folder with very short (couple of characters) total path length.
  - While it is not enforced, we strongly recommend you to avoid the use of spaces for both the source directory and the build directory.
- Create build folder. This folder will be referred to as `<Slicer_BUILD>` in the following. Recommended path: `C:\D\S4R` for release-mode build, `C:\D\S4D` for debug-mode build.
  - You cannot use the same build tree for both release or debug mode builds. If both build types are needed, then the same source directory can be used, but a separate build directory must be created and configured for each build type.
- Download source code into _Slicer source_ folder from GitHub: https://github.com/Slicer/Slicer.git
  - The following command can be executed in _Slicer source_ folder to achieve this: `git clone https://github.com/Slicer/Slicer.git .`
- Configure the repository for developers (optional): Needed if changes need to be contributed to Slicer repository.
  - Right-click on `<Slicer_SOURCE>/Utilities` folder in Windows Explorer and select `Git bash here`
  - Execute this command in the terminal (and answer all questions): `./SetupForDevelopment.sh`
  - Note: more information about how to use git in Slicer can be found on [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/DevelopmentWithGit)

## Configure and build Slicer

### Using command-line (recommended)

Specify source, build, and Qt location and compiler version and start the build using the following commands (these can be put into a .bat file so that they can be executed again easily), assuming default folder locations:

Release mode:

```
mkdir C:\D\S4R
cd /d C:\D\S4R
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 16 2019" -A x64 -DQt5_DIR:PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
```

Debug mode:

```
mkdir C:\D\S4D
cd /d C:\D\S4D
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 16 2019" -A x64 -DQt5_DIR:PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Debug
```

### Using graphical user interface (alternative solution)

- Run `CMake (cmake-gui)` from the Windows Start menu
- Set `Where is the source code` to `<Slicer_SOURCE>` location
- Set `Where to build the binaries` to `<Slicer_BUILD>` location. Do not configure yet!
- Add `Qt5_DIR` variable pointing to Qt5 folder: click Add entry button, set `Name` to `Qt5_DIR`, set `Type` to `PATH`, and set `Value` to the Qt5 folder, such as `C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5`.
- Click `Configure`
- Select your compiler: `Visual Studio 16 2019`, and click `Finish`
- Click `Generate` and wait for project generation to finish (may take a few minues)
- Click `Open Project`
- If building in release mode:
  - Open the top-level Slicer.sln file in the build directory in Visual Studio
  - Set active configuration to Release. Visual Studio will select Debug build configuration by default when you first open the solution in the Visual Studio GUI. If you build Slicer in release mode and accidentally forget to switch the build configuration to Release then the build will fail. Note: you can avoid this manual configuration mode selection by setting `CMAKE_CONFIGURATION_TYPES` to Release in cmake-gui.
- Build the `ALL_BUILD` project

## Run Slicer

Run `<Slicer_BUILD>/Slicer-build/Slicer.exe` application.

Note: `Slicer.exe` is a "launcher", which sets environment variables and launches the real executable: `<Slicer_BUILD>/Slicer-build\bin\Release\SlicerApp-real.exe` (use `Debug` instead of `Release` for debug-mode builds).

## Test Slicer

- Start Visual Studio with the launcher:
```
Slicer.exe --VisualStudioProject
```
- Select build configuration. Usually `Release` or `Debug`.
- In the "Solution Explorer", right click on `RUN_TESTS` project (in the CMakePredefinedTargets folder) and then select `Build`.

## Package Slicer (create installer package)

- Start Visual Studio with the launcher:
```
Slicer.exe --VisualStudioProject
```
- Select `Release` build configuration.
- In the "Solution Explorer", right click on `PACKAGE` project (in the CMakePredefinedTargets folder) and then select `Build`.

## Debug Slicer

- C++ debugging: Visual Studio is recommended on Windows, see [instructions](../debugging/windowscpp).
- Python debugging: multiple development environments can be used, see [instructions](../debugging/overview.md#python-debugging).

## Common errors

### Errors related to Python

Errors due to missing Python libraries (or other Python related errors) may be caused by the build system detecting Python installations somewhere on the system, instead of Slicer's own Python environment. To resolve such issues, remove all references to Python in the environment variables (PATH, PYTHONPATH, PYTHONHOME) and maybe even temporarily rename or remove other Python installations. Other Python installations must be removed/renamed before starting to build Slicer and can be restored after Slicer build completes.

### Other problems

See list of issues common to all operating systems on [Common errors](common_errors.md) page.
