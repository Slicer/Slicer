# Windows

:::{note}

Slicer relies on a number of large third-party libraries (such VTK, ITK, DCMTK), which take a long time to build and use a lot of disk space. Currently, build requires disk space of 15GB (for release mode) or 60GB (for debug mode). Build time on a desktop computer is typically 3-4 hours, on a laptop it may take 8-12 hours.

:::

## Install prerequisites

- [CMake](https://www.cmake.org/cmake/resources/software.html) version that meets at least the minimum required CMake version >= 3.21.1 (or 3.16.3 <= version < 3.20.4)
- [Git](https://git-scm.com/download/win) >= 1.7.10
  - Note: CMake must be able to find `git.exe` and `patch.exe`. If git is installed in the default location then they may be found there, but if they are not found then either add the folder that contains them to `PATH` environment variable; or set `GIT_EXECUTABLE` and `Patch_EXECUTABLE` as environment variables or as CMake variables at configure time.
- [Visual Studio](https://visualstudio.microsoft.com/downloads/): any edition can be used (including the free Community edition), when configuring the installer:
  - Enable `Desktop development with C++` and in installation details
  - Enable the `MSVC v143 - VS2022 C++ x64...` (Visual Studio 2022 v143 toolset with 64-bit support) component - in some distributions, this option is not enabled by default.
  - Enable the latest Windows10 SDK component - without this CMake might not find a compiler during configuration step.
- [Qt5](https://www.qt.io/download-open-source): Download Qt universal installer and install Qt 5.15.2 components: `MSVC2019 64-bit`, `Qt Script`, `Qt WebEngine`. Installing Sources and Qt Debug Information Files are recommended for debugging (they allow stepping into Qt files with the debugger in debug-mode builds).
  - Note: These are all free, open-source components with LGPL license which allow free usage for any purpose, for any individuals or companies.
- [NSIS](https://nsis.sourceforge.io/Download) (optional): Needed if packaging Slicer. Make sure you install the language packs.

:::{note}

**Other Visual Studio IDE and compiler toolset versions**

- Visual Studio 2019 (v142) toolset is not tested anymore but probably still works.
- Visual Studio 2017 (v141) toolset is not tested anymore but probably still works. Qt-5.15.2 requires v142 redistributables, so either these extra DLL files need to be added to the installation package or each user may need to install "Microsoft Visual C++ Redistributable" package.
- Cygwin and Mingw: not tested and not recommended. Building with cygwin gcc not supported, but the cygwin shell environment can be used to run utilities such as git.

:::

## Set up source and build folders

- Create source folder. This folder will be referred to as `<Slicer_SOURCE>` in the following. Recommended path: `C:\D\S4`
  - Due to maximum path length limitations during build the build process, source and build folders must be located in a folder with very short (couple of characters) total path length.
  - While it is not enforced, we strongly recommend you to avoid the use of spaces for both the source directory and the build directory.
- Create build folder. This folder will be referred to as `<Slicer_BUILD>` in the following. Recommended path: `C:\D\S4R` for release-mode build, `C:\D\S4D` for debug-mode build.
  - You cannot use the same build tree for both release or debug mode builds. If both build types are needed, then the same source directory can be used, but a separate build directory must be created and configured for each build type.
  - How to decide between Debug and Release mode?
    - Release mode build: runs at same speed as official build, requires less disk space (about 15GB); but step-by-step debugging is not available
    - Debug mode build: allows debugging (adding breakpoints, step through the code line by line during execution, watch variables); but it may run 5x or more slower, requires more space (about 60GB), and user interface editing in Qt designer is not available
- Download source code into _Slicer source_ folder from GitHub: https://github.com/Slicer/Slicer.git
  - The following command can be executed in _Slicer source_ folder to achieve this: `git clone https://github.com/Slicer/Slicer.git .` (note the dot at the end of the command; the `.` is needed because without that git would create a `Slicer` subfolder in the current directory)
- Configure the repository for developers (optional): Needed if changes need to be contributed to Slicer repository.
  - Right-click on `<Slicer_SOURCE>/Utilities` folder in Windows Explorer and select `Git bash here`
  - Execute this command in the terminal (and answer all questions): `./SetupForDevelopment.sh`
  - Note: more information about how to use git in Slicer can be found on [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/DevelopmentWithGit)

## Configure and build Slicer

Build takes several hours. Warnings will appear during the build (it is practically not feasible to have warning-free builds in large multi-platform projects that rely on third-party libraries), but there must not be any errors. If any problems occur, read the [Common errors](#common-errors) section.

### Using command-line (recommended)

Specify source, build, and Qt location and compiler version and start the build using the following commands (these can be put into a .bat file so that they can be executed again easily), assuming default folder locations:

Release mode:

```
mkdir C:\D\S4R
cd /d C:\D\S4R
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 17 2022" -A x64 -DQt5_DIR:PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
```

Debug mode:

```
mkdir C:\D\S4D
cd /d C:\D\S4D
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 17 2022" -A x64 -DQt5_DIR:PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Debug
```

### Using graphical user interface (alternative solution)

- Run `CMake (cmake-gui)` from the Windows Start menu
- Set `Where is the source code` to `<Slicer_SOURCE>` location
- Set `Where to build the binaries` to `<Slicer_BUILD>` location. Do not configure yet!
- Add `Qt5_DIR` variable pointing to Qt5 folder: click Add entry button, set `Name` to `Qt5_DIR`, set `Type` to `PATH`, and set `Value` to the Qt5 folder, such as `C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5`.
- Click `Configure`
- Select your compiler: `Visual Studio 17 2022`, and click `Finish`
- Click `Generate` and wait for project generation to finish (may take a few minutes)
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

- C++ debugging: Visual Studio is recommended on Windows, see [instructions](../debugging/windowscpp.md).
- Python debugging: multiple development environments can be used, see [instructions](../debugging/python.md#python-debugging).

## Common errors

### Errors related to Python

Errors due to missing Python libraries (or other Python related errors, such as building a `python-...-requirements` project or Python-wrapping SimpleITK) may be caused by the build system detecting Python installations somewhere on the system, instead of Slicer's own Python environment. To resolve such issues, remove all references to Python in the environment variables (PATH, PYTHONPATH, PYTHONHOME). Alternatively, temporarily rename or remove other Python installations before starting to build Slicer; they can be restored after Slicer build is completed.

### Custom Slicer and CTK widgets do not show up in Qt designer

Qt Designer can only use designer plugins (in `c:\D\S4R\Slicer-build\bin\designer` and `c:\D\S4R\CTK-build\CTK-build\bin\designer`) that are built in the same mode (e.g. `Debug` or `Release`).

In pre-built Qt packages (downloaded from Qt website) Qt Designer is only provided in `Release` mode. If Qt is built from source in `Release` mode (default) or `Release and Debug` mode then Qt designer will be in `Release` mode. In all these cases, Qt Designer can be used only if Slicer is built in `Release` mode.

If Qt is built from source in `Debug` mode mode then Qt designer will be in `Debug` mode. In this case, Qt Designer can be used only if Slicer is built in `Debug` mode.

### Other problems

See list of issues common to all operating systems on [Common errors](common_errors.md) page.
