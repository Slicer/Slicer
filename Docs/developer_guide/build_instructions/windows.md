# Windows

## Install prerequisites

- [CMake](http://www.cmake.org/cmake/resources/software.html) >= 3.15.1
- [Git](https://git-scm.com/download/win) >= 1.7.10
  - Note CMake must be able to find `git.exe` and `patch.exe`. If git is installed in the default location then they may be found there, but if they are not found then either add the folder that contains them to `PATH` environment variable; or set `GIT_EXECUTABLE` and `Patch_EXECUTABLE` as environment variables or as CMake variables at configure time.
- [NSIS](http://nsis.sourceforge.net/Download) (optional): Needed if packaging Slicer. Make sure you install the language packs.
- [Qt5](https://www.qt.io/download-open-source): Download Qt universal installer and install Qt 5.15 components: MSVC2019 64-bit, Qt Script, Qt WebEngine. Installing Sources and Qt Debug Information Files are recommended for debugging (they allow stepping into Qt files with the debugger in debug-mode builds).
- [Visual Studio](https://visualstudio.microsoft.com/downloads/)
  - any edition can be used (including the free Community edition)
  - when configuring the installer, enable `Desktop development with C++` and in installation details, check `MSVC v142 - VS2019 C++ x64...` (Visual Studio 2019 v142 toolset with 64-bit support) - in some distributions, this option is not enabled by default

Other versions:
- Visual Studio 2017 (v141) toolset is not tested anymore but probably still works. Qt-5.15 requires v142 redistributables, so either these extra DLL files need to be added to the installation package or each user may need to install "Microsoft Visual C++ Redistributable" package.
- Visual Studio 2015 (v140) toolset is not tested anymore and probably does not work. Requires Qt 5.10.x to build due to QtWebEngine.
- Cygwin: not tested and not recommended. Building with cygwin gcc not supported, but the cygwin shell environment can be used to run git, svn, etc.

## Set up source and build folders

- Create source folder. This folder will be referred to as `<Slicer_SOURCE>` in the followings. Recommended path: `C:\D\S4`
  - Due to maximum path length limitations during build the build process, source and build folders must be located in a folder with very short (couple of characters) total path length.
  - While it is not enforced, we strongly recommend you to avoid the use of spaces for both the source directory and the build directory.
- Create build folder. This folder will be referred to as `<Slicer_BUILD>` in the followings. Recommended path: `C:\D\S4R` for release-mode build, `C:\D\S4D` for debug-mode build.
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
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 16 2019 Win64" -DQt5_DIR:PATH=C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
```

Debug mode:

```
mkdir C:\D\S4D
cd /d C:\D\S4D
"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 16 2019 Win64" -DQt5_DIR:PATH=C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5 C:\D\S4
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Debug
```

### Using graphical user interface (alternative solution)

- Run `CMake (cmake-gui)` from the Windows Start menu
- Set `Where is the source code` to `<Slicer_SOURCE>` location
- Set `Where to build the binaries` to `<Slicer_BUILD>` location. Do not configure yet!
- Add `Qt5_DIR` variable pointing to Qt5 folder: click Add entry button, set `Name` to `Qt5_DIR`, set `Type` to `PATH`, and set `Value` to the Qt5 folder, such as `C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5`.
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

1. To run Slicer, the launcher needs to set certain environment variables. The easiest is to use the launcher to set these and start Visual Studio in this environment. All these can be accomplished by running the following command in `<Slicer_BUILD>/c:\D\S4R\Slicer-build` folder:

```
Slicer.exe --VisualStudioProject
```

Notes:
- If you just want to start VisualStudio with the launcher (and then load project file manually), run: `Slicer.exe --VisualStudio`
- To debug an extension that builds third-party DLLs, also specify `--launcher-additional-settings` option.
- While you can launch debugger using Slicer's solution file, it is usually more convenient to load your extension's solution file (because your extension solution is smaller and most likely you want to have that solution open anyway for making changes in your code). For example, you can launch Visual Studio to debug your extension like this:

```
.\S4D\Slicer-build\Slicer.exe --VisualStudio --launcher-no-splash --launcher-additional-settings ./SlicerRT_D/inner-build/AdditionalLauncherSettings.ini c:\d\_Extensions\SlicerRT_D\inner-build\SlicerRT.sln
```

2. In Solution Explorer window in Visual Studio, expand `App-Slicer`, right-click on `SlicerApp` (NOT qSlicerApp) and select "Set as Startup Project"

To debug in an extension's solution: set `ALL_BUILD` project as startup project and in project settings, set `Debugging` / `Command` field to the full path of `SlicerApp-real.exe` - something like `.../Slicer-build/bin/Debug/SlicerApp-real.exe`

3. Run Slicer in debug mode by Start Debugging command (in Debug menu).

Note that because CMake re-creates the solution file from within the build process, Visual Studio will sometimes need to stop and reload the project, requiring manual button pressing on your part (just press Yes or OK whenever you are asked). To avoid this, you can use a script to complete the build process and then re-start the Visual Studio.

For more debugging tips and tricks, check out [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Troubleshooting).

### Debug a test

Once VisualStudio is open with the Slicer environment loaded, it is possible to run and debug tests. To run all tests, build the `RUN_TESTS` project.

- To debug a test, find its project:
  - `Libs/MRML/Core` tests are in the `MRMLCoreCxxTests` project
  - CLI module tests are in `<CLI_NAME>Test` project (e.g. `ThresholdScalarVolumeTest`)
  Loadable module tests are in `qSlicer<LOADABLE_NAME>CxxTests` project (e.g. `qSlicerVolumeRenderingCxxTests`)
  - Module logic tests are in `<MODULE_NAME>LogicCxxTests` project (e.g. `VolumeRenderingLogicCxxTests`)
  - Module widgets tests are in `<MODULE_NAME>WidgetsCxxTests` project (e.g. `VolumesWidgetsCxxTests`)
- Go to the project debugging properties (right click -> Properties, then Configuration Properties/Debugging)
- In `Command Arguments`, type the name of the test (e.g. `vtkMRMLSceneImportTest` for project `MRMLCoreCxxTests`)
- If the test takes argument(s), enter the argument(s) after the test name in `Command Arguments` (e.g. `vtkMRMLSceneImportTest C:\Path\To\Slicer4\Libs\MRML\Core\Testing\vol_and_cube.mrml`)
  - You can see what arguments are passed by the dashboards by looking at the test details in CDash.
  - Most VTK and Qt tests support the `-I` argument, it allows the test to be run in "interactive" mode. It doesn't exit at the end of the test.
- Make the project Set As Startup Project
- Start Debugging (F5)

### Debugging Python scripts

See [Python scripting page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Python_scripting#How_can_I_use_a_visual_debugger_for_step-by-step_debugging) for detailed instructions.

## Common errors

See list of issues common to all operating systems on [Common errors](common_errors) page.
