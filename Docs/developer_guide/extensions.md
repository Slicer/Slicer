# Extensions

Developers can create extensions to provide additional features to users. See an overview of extensions in the [Extensions manager page](../user_guide/extensions_manager).

## Create an extension

If you have developed a script or module that you would like to share with others then it is recommended to submit it to the Slicer Extensions Index. Indexed extensions get listed in the Extensions Manager in Slicer and user can install them by a few mouse clicks.

- Scan through the [user](../user_guide/extensions_manager.md) and [developer](https://www.slicer.org/wiki/Documentation/Nightly/Developers/FAQ/Extensions) extension FAQs
- Inform a community about your plans on the [Slicer forum](https://discourse.slicer.org) to get information about potential parallel efforts (other developers may already work on a similar idea and you could join or build on each other's work), past efforts (related tools might have been available in earlier Slicer versions or in other software that you may reuse), and get early feedback from prospective users. You may also seek advice on the name of your extension and how to organize features into modules. All these can save you a lot of time in the long term.
- If you have not done already, use the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) module in Slicer to create an extension that will contain your module(s).
- If developing [C++ loadable or CLI modules](module_overview.md) (not needed if developing in Python):
  - [build Slicer application](build_instructions/index.md).
  - [build your extension](#build-an-extension)

(extension-build)=
## Build an extension

:::{note}
To compile extensions containing C++-implemented modules, it's essential to [build Slicer from source](build_instructions/index.md) on your local machine. These extensions cannot be compiled against a binary download. However, if your modules are solely developed in Python, there's no need to build the extension.

Once your work is ready for sharing, refer to [](#extension-distribute) for guidelines.
:::

Similarly to the building of Slicer core, multi-configuration builds are not supported: one build tree can be only used for one build mode (Release or Debug or RelWithDebInfo or MinSizeRel). If a release and debug mode build are needed then the same source code folder can be used (e.g., `C:\D\SlicerHeart`) but a separate binary folder must be created for each build mode (e.g., `C:\D\SlicerHeart-R` and `C:\D\SlicerHeart-D` for release and debug modes).

Assuming that the source code of your extension is located in folder `MyExtension`, an extension can be built by the following steps.

:::{tip}
For testing purposes, it is possible to force the Slicer revision associated with the extension build by setting the `Slicer_REVISION`
environment variable before configuring the project:

```bash
$ cd MyExtension-debug
$ export Slicer_REVISION=31806
$ cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DSlicer_DIR:PATH=/path/to/Slicer-SuperBuild-Debug/Slicer-build ../MyExtension
[...]
-- SlicerConfig: Forcing Slicer_REVISION to '31806'
[...]
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/MyExtension-debug
```
:::

### Linux and macOS

Start a terminal.

```bash
$ mkdir MyExtension-debug
$ cd MyExtension-debug
$ cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DSlicer_DIR:PATH=/path/to/Slicer-SuperBuild-Debug/Slicer-build ../MyExtension
$ make
```

#### CMAKE_OSX_ variables

On macOS, the extension must be configured specifying `CMAKE_OSX_*` variables matching the one used to configure Slicer: `-DCMAKE_OSX_ARCHITECTURES:STRING=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=/same/as/Slicer -DCMAKE_OSX_SYSROOT:PATH=SameAsSlicer`

Instead of manually setting these variables, within your extension, including the <code>ConfigurePrerequisites</code> component before the project statement should ensure it uses the same CMAKE_OSX_* variables as Slicer:

```cmake
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)

project(Foo)

# [...]

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

# [...]
```

For more details, see [here](https://github.com/Slicer/Slicer/blob/6f4e2946bb129d317dfdb1116f06f5308b449044/CMake/SlicerConfig.cmake.in#L10-L38).

### Windows

Run `CMake (cmake-gui)` from the Windows Start menu.

- Select source and build directory
- Click `Configure`
- Select generator (just accept the default if you only have one compiler toolset installed)
- Choose to create build directory if asked
- The configuration is expected to display an error message due to `Slicer_DIR` variable not specified yet.
- Specify `Slicer_DIR` by replacing `Slicer_DIR-NOTFOUND` by the Slicer inner-build folder (for example `c:/D/SD/Slicer-build`).
- Click `Configure`. No errors should be displayed.
- Click `Generate` button.
- Click `Open project` button to open `MyExtension.sln` in Visual Studio.
- Select build configuration (Debug, Release, ...) that matches the build configuration (Release, Debug, ...) of the chosen Slicer build.
- In the menu choose Build / Build Solution.

## Test an extension

### Run Slicer with your custom modules

To test an extension, you need to specify location(s) where Slicer should look for additional modules.

- If the extension is not built: append all source code folders that contain module `.py` files to "additional module paths".
  - Option A (recommended): Drag-and-drop the `.py` files (or the parent folder, or that folder's parent) to the application window, select `Add Python scripted modules to the application` in the popup window, click OK. Select which modules to add to load immediately and click `Yes`. The selected modules will be immediately loaded and made available in the module list. If you want to load the modules only in the current session, then uncheck `Add selected modules to 'Additional module paths'` option before clicking `Yes`.
  - Option B: In menu: Edit / Application settings / Modules panel, drag-and-drop files to the `Additional module paths` list.
- If the extension is built:
  - Option A: start the application using the `SlicerWithMyExtension` executable in your build directory. This starts Slicer, specifying additional module paths via command-line arguments.
  - Option B: specify additional module paths manually in application settings. Assuming your extension has been built into folder `MyExtension-debug`, add these module folders (if they exist) to the additional module paths in Slicer's application settings:
    - `C:\path\to\MyExtension-debug\lib\Slicer-4.13\qt-scripted-modules`
    - `C:\path\to\MyExtension-debug\lib\Slicer-4.13\qt-loadable-modules\Debug` or `C:\path\to\MyExtension-debug\lib\Slicer-4.13\qt-loadable-modules\Release` or (on systems where multi-configuration builds are not used, such as linux) simply `C:\path\to\MyExtension-debug\lib\Slicer-4.13\qt-loadable-modules`:
    - `C:\path\to\MyExtension-debug\lib\Slicer-4.13\cli-modules\Debug`, `C:\path\to\MyExtension-debug\lib\Slicer-4.13\cli-modules\Release`, or `C:\path\to\MyExtension-debug\lib\Slicer-4.13\cli-modules`

### Run automatic tests

Automatic tests of your extension can be launched by following the instructions below.

#### Linux and macOS

Start a terminal.

```bash
$ ctest -j<NUMBEROFCORES>
```

#### Windows

##### Run all tests

Open a command prompt.

```bat
cd C:\path\to\MyExtension-debug
"c:\Program Files\CMake\bin\ctest.exe" -C Release -V
```

Replace `Release` with the build mode of your extension build (`Debug`, `Release`, ...).

##### To debug a test

- Launch Visual Studio from the Command Line Prompt: `C:\D\SD\Slicer-build\Slicer.exe --VisualStudio --launcher-no-splash --launcher-additional-settings C:\path\to\MyExtension-debug\AdditionalLauncherSettings.ini C:\path\to\MyExtension-debug\MyExtension.sln`
- Find the project of the test you want to debug (e.g., `qSlicerMODULE_NAMEModuleGenericCxxTests`).
- Go to the project debugging properties (right-click -> Properties, then Configuration Properties / Debugging).
- In `Command Arguments`, type the name of the test you want to run (e.g., `qSlicerMODULE_NAMEModuleGenericTest`).
- If the test takes arguments, enter the arguments after the test name in `Command Arguments`.
- Set the project as the StartUp Project (right-click -> Set As StartUp Project).
- Start debugging (F5).

(extension-continuous-integration)=
### Continuous integration

If you shared your extension by using the ExtensionWizard, make sure you know about the Slicer testing dashboard:
- [Dashboard for Slicer Stable Releases](https://slicer.cdash.org/index.php?project=SlicerStable)
- [Dashboard for Slicer Preview Releases](https://slicer.cdash.org/index.php?project=SlicerPreview)

The dashboard will attempt to check out the source code of your extension, build, test and package it on Linux, macOS and Windows platforms.

To find your extension, use the following link replacing `SlicerMyExtension` with the name of your extension:

`https://slicer.cdash.org/index.php?project=SlicerStable&filtercount=1&showfilters=1&field1=buildname&compare1=63&value1=SlicerMyExtension`

For example, here is the link to check the status of the `SlicerDMRI` extension:

`https://slicer.cdash.org/index.php?project=SlicerStable&filtercount=1&showfilters=1&field1=buildname&compare1=63&value1=SlicerDMRI`

If you see red in any of the columns for your extension, click on the hyperlinked number of errors to see the details.

Always check the dashboard after you first introduce your extension, or after you make any changes to the code.

## Create an extension package

Assuming your extension has been built into folder `MyExtension-release` (redistributable packages must be built in release mode), this could be achieved doing:

### Linux and macOS

Start a terminal.

```bash
$ make package
```

### Windows

- Open `MyExtension.sln` in Visual Studio.
- Right-click on `PACKAGES` project, then select `Build`.

## Write documentation for an extension

Keep documentation with your extension's source code and keep it up-to-date whenever the software changes.

Add at least a README.md file in the root of the source code repository, which describes what the extension is for and how it works. Minimum information that is needed to make your extension usable is described in the [extension submission checklist](https://github.com/Slicer/ExtensionsIndex/blob/main/.github/PULL_REQUEST_TEMPLATE.md#todo-list-for-submitting-a-new-extension).

Extension documentation examples:
- [SegmentMesher](https://github.com/lassoan/SlicerSegmentMesher)
- [SequenceRegistration](https://github.com/moselhy/SlicerSequenceRegistration)
- [AI-assisted annotation client](https://github.com/NVIDIA/ai-assisted-annotation-client/tree/master/slicer-plugin)
- [SlicerDMRI](https://dmri.slicer.org/) - large extension documented using Github pages

Thumbnails to YouTube videos can be generated by using an URL of the form `https://img.youtube.com/vi/<insert-youtube-video-id-here>/0.jpg` and
adding a playback button using [this free service](https://addplaybuttontoimage.way4info.net/) (the second red arrow is recommended).

(extension-distribute)=
## Distribute an extension

- Upload source code of your extension to a publicly available repository. It is recommended to start the repository name with "Slicer" (to make Slicer extensions easier to identify) followed by your extension name (for example, "Sequences" extension is stored in "SlicerSequences" repository). However, this is not a mandatory requirement. If you have a compelling reason not to use Slicer prefix, please make a note while making the pull request. See more requirements in the [new extension submission checklist](https://github.com/Slicer/ExtensionsIndex/blob/main/.github/PULL_REQUEST_TEMPLATE.md#todo-list-for-submitting-a-new-extension).
  - GitHub is recommended (due to large user community, free public project hosting): [join Github](https://github.com/join) and [setup Git](https://docs.github.com/get-started/quickstart/set-up-git).
- If developing an extension that contains [C++ loadable or CLI modules](module_overview.md) (not needed if developing in Python):
  - Build the `PACKAGE` target to create a package file.
  - Test your extension by installing the created package file using the Extensions Manager.
- Complete the [extension submission checklist](https://github.com/Slicer/ExtensionsIndex/blob/main/.github/PULL_REQUEST_TEMPLATE.md#todo-list-for-submitting-a-new-extension)) then submit it to the Slicer Extensions Index:
- Submit the extension to the Extensions Index:
  - Fork ExtensionIndex repository on GitHub by clicking ''Fork'' button on the [Slicer Extensions Index](https://github.com/Slicer/ExtensionsIndex) page
  - Create an [extension catalog entry file (json)](#extension-catalog-entry-file)
  - Add your `.json` file to your forked repository: it can be done using a git client or simply by clicking ''Upload files'' button
    - To make the extension appear in the latest Slicer Preview Release: upload the file into the `main` branch.
    - To make the extension appear in the latest Slicer Stable Release: upload the file into the branch corresponding to the stable release version, for example: `4.10`.
  - Create a pull request: by clicking ''Create pull request'' button
  - Follow the instructions in the pull request template

:::{tip}
Once your extension is successfully integrated into the Extensions Index, you gain access to daily build and test reports for verification. For additional information, refer to [](#extension-continuous-integration).

While it is possible to develop Python-based extensions without a [local extension build](#extension-build), creating one can significantly expedite the development cycle. This allows you to promptly confirm that the extension can be properly packaged and distributed, providing a quicker feedback loop compared to waiting for the daily build and test reports.
:::

## Application settings

After installing an extension, the directories are added to revision-specific settings:
- Folders containing modules bundled within an extension are added to Modules / AdditionalPaths. This ensures that libraries associated with modules are found.
- Folders containing third-party dynamic libraries, Python libraries, etc. are added to LibraryPaths, Paths, PYTHONPATH, QT_PLUGIN_PATH. This ensures that libraries associated with modules can be successfully loaded.

(extension-catalog-entry-file)=
## Extension catalog entry file

An extension catalog entry file is a JSON file describing how to build and package a Slicer extension.

It is named after the extension name (e.g `ExtensionName.json`) and may include the following entries:


```{list-table}
:header-rows: 1

* - Name
  - Description
  - Required
* - category
  - Extension category.
  - Y
* - scm_url
  - Read-only url used to checkout the extension source code.
  - Y
* - scm_revision
  - Revision allowing to checkout the expected source code.
  - N
* - scm_type
  - Type of revision control system. Default to `git`.
  - N
* - build_dependencies
  - List of extensions required to build this extension. For example: `["extensionA", "extensionB"]`.
  - N
* - build_subdirectory
  - Name of the inner build directory in case of superbuild based extension. Default to `.`.
  - N
* - enabled
  - Specify if the extension should be enabled after its installation. Valid values are `true` (default) = enabled; `false` = disabled.
  - N
```

The full schema for the extension catalog entry file is below:

```{jsonschema} ../../Schema/slicer-extension-catalog-entry-schema-v1.0.0.json
```

(extension-description-file)=
## Extension description file

:::{warning}
This file is used internally by [](#extensions-build-system) and it has been superseded by [](#extension-catalog-entry-file).
:::

An extension description file is a text file with `s4ext` extension allowing to specify metadata associated with an extensions.

The description file is automatically generated by the build system in the build tree of the extension from the metadata specified in the top-level `CMakeLists.txt` file in the source code of the extension. Since the description is a very simple text file, the description can be created using a text editor. It may be simpler to use a text editor to create the file if the extension only contains scripted modules, which can be developed without building the extension.

Note that the Extension manager ignores many fields of the extension description file and instead uses information specified in `CMakeLists.txt`. Therefore, when making any changes to the extension description file, it has to be done in the `CMakeLists.txt` file as well.

For superbuild-type extensions (that build their own dependencies, such as external libraries implemented in C++), it is critical that the build_subdirectory is initialized to the inner build location in s4ext. The value of this variable in `CMakeLists.txt` is not used in all places by the dashboard scripts.

### Syntax

- A metadata is specified using a keyword followed by at least one spaces and the associated value.
- Multiline values are not supported.
- Empty lines are ignored
- Lines starting with a `#` are considered comments and ignored.

The following code block illustrates how comments, metadata and associated value can be specified:

```
# This is a comment
metadataname This is the value associated with 'metadataname'

# This is an other comment
anothermetadata This is the value associated with 'anothermetadata'
```

### Supported metadata fields

```{list-table}
:header-rows: 1

* - Name
  - Description
  - Required
* - scm
  - Source code management system. Must be `git`.
  - Y
* - scmurl
  - Read-only url used to checkout the extension source code.
  - Y
* - scmrevision
  - Revision allowing to checkout the expected source code.
  - Y
* - depends
  - List of extensions required to build this extension. Specify "NA" if there are no dependency. Extension names should be separated by a single space. For example: extensionA extensionB.
  - N
* - build_subdirectory
  - Name of the inner build directory in case of superbuild based extension. Default to `.`.
  - N
* - homepage
  - URL of the web page describing the extension.
  - Y
* - contributors
  - Extension contributor specified as `Firstname1 Lastname1 ([SubOrg1, ]Org1), Firstname2 Lastname2 ([SubOrg2, ]Org2)`.
  - N
* - category
  - Extension category.
  - Y
* - iconurl
  - URL to an icon (png file format and size of 128x128 pixels is recommended).
  - N
* - description
  - One line describing what is the purpose of the extension.
  - N
* - screenshoturls
  - Space separated list of urls to images.
  - N
* - enabled
  - Specify if the extension should be enabled after its installation. Valid values are `1` (default) = enabled; `0` = disabled.
  - N
* - status
  - Give people an idea what to expect from this code. This is currently not used.
  - N
```

Note: Parameters in URLS (such as `&foo=bar`) are not supported. URL shortener services can be used if necessary.

## Extensions server

The official Slicer extensions server, the "Extensions Catalog" is available at <https://extensions.slicer.org/>. To get a list of extensions, specify the Slicer revision and platform in the URL, for example: <https://extensions.slicer.org/catalog/All/30117/win>.

The Extensions Catalog is implemented a web application ([source code](https://github.com/KitwareMedical/slicer-extensions-webapp)), which connects
to a [Girder server](https://slicer-packages.kitware.com/#collection/5f4474d0e1d8c75dfc70547e/folder/5f4474d0e1d8c75dfc705482), a general-purpose
storage server with the Slicer Package Manager plugin ([source code](https://github.com/girder/slicer_package_manager)), which provides a
convenient REST API for accessing Slicer extension packages and metadata.

The "Manage extensions" tab in the Extensions Manager in Slicer uses this REST API to get information on updates and get packages to install or update.

The extension server is designed so that organizations can set up and maintain their own extensions servers, for example to distribute
extensions for custom applications. Extensions server address can be set in the Application Settings, in the Extensions section.

Until August 2021, a Midas-based server at `https://slicer.kitware.com/midas3` was used. This server is not online anymore, as it was not feasible to perform all software updates that would have kept it secure.

## Extensions Index

The ExtensionsIndex is a repository containing a list of [extension catalog entry files](#extension-catalog-entry-file) `*.json` used by the Slicer [Extensions build system](#extensions-build-system) to build, test, package and upload extensions on the [extensions server](#extensions-server).

The ExtensionsIndex is hosted on GitHub: <https://github.com/Slicer/ExtensionsIndex>

Each branch of the repository contains extension description files that corresponds to the same branch in the Slicer repository. For example, `main` branch contains descriptions for Slicer `main` branch, and `4.11` branch contains extension descriptions for Slicer's `4.11` branch.

Extension developers have to make sure that the extension description in each branch of the Extensions index is compatible with the corresponding Slicer version. Extension developers often create the same branches (`main`, `4.11`, `4.13`, ...) in their repository and they specify this branch name in the extensions descriptor file.

(extensions-build-system)=
## Extensions build system

The extensions build system allows to drive the build, test, packaging and upload of Slicer extensions.

Using the [extensions build system source code](https://github.com/Slicer/Slicer/tree/main/Extensions/CMake), it is possible to build extensions using either manual build or dashboard-driven automatic build. The extension description files must be simply placed in a folder, the same way as they are in the Extensions Index repository.

### Build list of extensions manually

Locally building a list of extensions is a convenient way to test building of extensions and get extension packages for a custom-build Slicer application.

Given a directory containing one or more extension description files, with the help of the extensions build system it is possible to configure and build the associated extensions specifying the following CMake options:

```{list-table}
:header-rows: 1

* - CMake variable
  - Description
* - Slicer_DIR
  - Path to Slicer build tree. Required.
* - Slicer_EXTENSION_DESCRIPTION_DIR
  - Path to folder containing extension description files. Required.
* - [CMAKE_BUILD_TYPE](https://www.cmake.org/cmake/help/v2.8.8/cmake.html#variable:CMAKE_BUILD_TYPE)
  - Build type of the associated Slicer build directory
* - CTEST_MODEL
  - By default set to `Experimental`.
    Allow to choose on which CDash track results are submitted as well as setting the submission type associated with the uploaded extension.
* - Slicer_UPLOAD_EXTENSIONS
  - By default set to `OFF`.
    If enabled, extension builds will be submitted to Slicer dashboard and associated packages will be uploaded to extensions server.
* - SLICER_PACKAGE_MANAGER_URL
  - Slicer extensions server URL specifying where the extension should be uploaded. For example `https://slicer-packages.kitware.com`. source code.
    Note that this variable is expected to be set in place of `MIDAS_PACKAGE_URL`.
* - SLICER_PACKAGE_MANAGER_API_KEY
  - Token allowing to authenticate to the extensions server.
    Note that this variable is expected to be set in place of `MIDAS_PACKAGE_API_KEY` and `MIDAS_PACKAGE_EMAIL`.
```

The following folders will be used in the examples below:

| Folder                                                                               | Linux/macOS                                      | Windows  |
|--------------------------------------------------------------------------------------|--------------------------------------------------|----------|
| Slicer source code tree (checked out from https://github.com/Slicer/Slicer.git)      | `~/Slicer`                                       | `C:\D\S`               |
| Slicer build tree (built by following Slicer build instructions)                     |  `~/Slicer-SuperBuild-Release`                   | `C:\D\SR`              |
| List of extension description files (for example checked out from https://github.com/Slicer/ExtensionsIndex.git) |  `~/ExtensionsIndex` | `C:\D\ExtensionsIndex`  |
| Folder to store built extensions (new empty folder)                                  |  `~/ExtensionsIndex-Release`                     | `C:\D\ExtensionsIndexR` |

#### Build, test, and package

Linux and macOS:

```
cd ~/ExtensionsIndex-Release

cmake -DSlicer_DIR:PATH=~/Slicer-SuperBuild-Release/Slicer-build \
 -DSlicer_EXTENSION_DESCRIPTION_DIR:PATH=~/ExtensionsIndex \
 -DCMAKE_BUILD_TYPE:STRING=Release \
 ~/Slicer/Extensions/CMake

make
```

Windows:

```
cd /d C:\D\ExtensionsIndexR

"c:\Program Files\CMake\bin\cmake.exe" -DSlicer_DIR:PATH=C:/D/SR/Slicer-build ^
 -DSlicer_EXTENSION_DESCRIPTION_DIR:PATH=C:/D/ExtensionsIndex ^
 -DCMAKE_BUILD_TYPE:STRING=Release ^
 C:/D/S/Extensions/CMake

"c:\Program Files\CMake\bin\cmake.exe" --build . --config Release
```

#### Build, test, package, and upload to extensions server

Submit the configure/build/test results to the Slicer Dashboard `Extensions-Experimental` track and upload the extension to a custom Extensions Server.

Linux and macOS:

```
cd ~/ExtensionsIndex-Release
cmake -E env \
  SLICER_PACKAGE_MANAGER_CLIENT_EXECUTABLE=/path/to/slicer_package_manager_client \
  SLICER_PACKAGE_MANAGER_URL=https://slicer-packages.kitware.com \
  SLICER_PACKAGE_MANAGER_API_KEY=a0b012c0123d012abc01234a012345a0 \
  \
cmake -DSlicer_DIR:PATH=~/Slicer-SuperBuild-Release/Slicer-build \
 -DSlicer_EXTENSION_DESCRIPTION_DIR:PATH=~/ExtensionsIndex \
 -DCMAKE_BUILD_TYPE:STRING=Release \
 -DCTEST_MODEL:STRING=Experimental \
 -DSlicer_UPLOAD_EXTENSIONS:BOOL=ON \
 ~/Slicer/Extensions/CMake

make
```

Windows:

```
cd /d C:\D\ExtensionsIndexR
"c:\Program Files\CMake\bin\cmake.exe" -E env ^
  SLICER_PACKAGE_MANAGER_CLIENT_EXECUTABLE=/path/to/slicer_package_manager_client ^
  SLICER_PACKAGE_MANAGER_URL=https://slicer-packages.kitware.com ^
  SLICER_PACKAGE_MANAGER_API_KEY=a0b012c0123d012abc01234a012345a0 ^
  ^
"c:\Program Files\CMake\bin\cmake.exe" -DSlicer_DIR:PATH=~/Slicer-SuperBuild-Release/Slicer-build ^
 -DSlicer_EXTENSION_DESCRIPTION_DIR:PATH=C:/D/ExtensionsIndex ^
 -DCMAKE_BUILD_TYPE:STRING=Release ^
 -DCTEST_MODEL:STRING=Experimental ^
 -DSlicer_UPLOAD_EXTENSIONS:BOOL=ON ^
 C:/D/S/Extensions/CMake

make
```

### Build complete Extensions Index with dashboard submission

Continuous and nightly extension dashboards are setup on the Slicer factory machine maintained by [Kitware](https://www.kitware.com). Developers can set up similar infrastructure privately for their custom applications.

By customizing the [extension template dashboard script](https://github.com/Slicer/Slicer/blob/main/Extensions/CMake/SlicerExtensionsDashboardScript.TEMPLATE.cmake), it is possible to easily setup dashboard client submitting to [CDash](https://slicer.cdash.org/index.php?project=SlicerPreview). See example dashboard scripts that are used on official Slicer build machines [here](https://github.com/Slicer/DashboardScripts). Note that these scripts are more complex than the template to allow code reuse between different configurations, but they are tested regularly and so guaranteed to work.

## Frequently asked questions

### Can an extension contain different types of modules?

Yes. Extensions are used to package together all types of Slicer modules.

### Should the name of the source repository match the name of the extension?

Assuming your extension is named `AwesomeTool`, generally, we suggest to name the extension repository `SlicerAwesomeTool`. Doing so will minimize confusion by clearly stating that the code base is associated with Slicer.

We suggest to use the `Slicer` prefix in the extension name, too, when the extension is a Slicer interface to some third-party library (such as SlicerOpenIGTLink, SlicerElastix, SlicerANTs SlicerOpenCV).

### Where can I find the extension templates?

The module and extension templates are available in the Slicer source tree: <https://github.com/Slicer/Slicer/tree/main/Utilities/Templates/>

Using the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) module, developers can easily create a new extension without having to copy, rename and update manually every files.

### How are Superbuild extension packaged?

Extensions using the Superbuild mechanism build projects in two steps:
- First, the project dependencies are built in an outer-build directory.
- Then, the project itself is built in an inner-build directory.

Extensions can use the Superbuild (i.e., CMake's ExternalProject_Add) mechanism. However, developers have to be careful that the packaging macros clean the project before reconfiguring it. This means that if one uses the Slicer extension packaging macros inside the inner-build directory, when packaging and uploading the extension package, the project will be reconfigured, and variables passed from the outer-build directory will be lost. If the project only depends on libraries that Slicer builds, this is not an issue. If the project has specific dependencies that Slicer does not compile on its own, the developer should be careful to instantiate the Slicer extension packaging macros only in the outer-build directory. This only means that in the latter case, tests should be instantiated in the outer-build directory to allow the Slicer extension building process to test the extension before uploading the extension and the tests results.

### How to build a custom Slicer package with additional extensions bundled?

To build custom Slicer versions, it is recommended to use the [Slicer Custom Application Template](https://blog.kitware.com/slicercat-creating-custom-applications-based-on-3d-slicer/).

### Can an extension depend on other extensions?

Yes. The dependency should be specified as a list by setting the variable `EXTENSION_DEPENDS` in the extension `CMakeLists.txt` and in the `dependency` field in the extension description. If the user installs `Extension2` that depends on `Extension1` then the extension manager will install `Extension1` automatically.

If you have `ModuleA`, `ModuleB` and `ModuleC` and `ModuleA` can be used as standalone one. You could create the following extensions:

- `Extension1` containing `ModuleA`.
- `Extension2` containing `ModuleB` and `ModuleC`, depending on `Extension1`.

Add the following variable to `Extension2/CMakeLists.txt`:

```cmake
set(EXTENSION_DEPENDS Extension1)
```

### How dependent extensions are configured and built?

If an ExtensionB depends on an ExtensionA, ExtensionA should be listed as dependency in the metadata of ExtensionB.

This can be done setting <code>EXTENSION_DEPENDS</code> in the <code>CMakeLists.txt</code> or by specifying <code>depends</code> field in the [[Documentation/{{documentation/version}}/Developers/Extensions/DescriptionFile|description file]].

Doing so will ensure that:

*(1) the extension build system configure the extensions in the right order
*(2) ExtensionB is configured with option <code>ExtensionA_DIR</code>.


### What are the extension specific targets: INSTALL, PACKAGE, packageupload, ...?

Slicer extension build system provides the developer with a set of convenient targets allowing to build and upload extensions.

- `RUN_TESTS`: Locally execute all tests.
- `PACKAGE` or `package`: Locally package the extension.
- `packageupload`: Locally package and upload the extension and upload to the extensions server. Requires access privileges to the server.
- `Experimental`: Configure, build, test the extension and publish result on CDash.
- `Continuous` / `Nightly`: retrieves the latest / latest nightly revision and runs the same steps as for the `Experimental` target.

### Is --launch flag available for a MacOSX installed Slicer.app?

On MacOSx, running Slicer with the `--help` argument does NOT list the usual launcher related options.

```bash
 $ ./Slicer.app/Contents/MacOS/Slicer --help
 Usage
  Slicer [options]

  Options
    --, --ignore-rest                     Ignores the rest of the labeled arguments following this flag. (default: false)
    -h, --help                            Display available command line arguments.
    [...]
    --version                             Displays version information and exits.
```

To provide some background information, when generating the package that will be distributed, an application bundle `Slicer.app` is created. As explained [here](https://developer.apple.com/library/mac/#documentation/CoreFoundation/Conceptual/CFBundles/Introduction/Introduction.html), a bundle is a directory with a standardized hierarchical structure that holds executable code and the resources used by that code. It means that since all libraries contained within a bundle are referenced relatively to the location of either the CLI or the Slicer executable, the use of launcher does NOT make sens.

To help fixing-up the libraries, executables and plugins so that they reference each other in a relative way, CMake provides us with the [BundleUtilities](https://www.cmake.org/cmake/help/v2.8.8/cmake.html#module:BundleUtilities) module.

This module is used in two situations:

- Fixup of Slicer application itself. See [SlicerCPack.cmake#L36-68](https://github.com/Slicer/Slicer/blob/main/CMake/SlicerCPack.cmake#L36-68) and [SlicerCPackBundleFixup.cmake.in](https://github.com/Slicer/Slicer/blob/main/CMake/SlicerCPackBundleFixup.cmake.in).
- Fixup of an extension package. See [SlicerExtensionCPack.cmake#L126-143](https://github.com/Slicer/Slicer/blob/main/CMake/SlicerExtensionCPack.cmake#L126-143) and [SlicerExtensionCPackBundleFixup.cmake.in](https://github.com/Slicer/Slicer/blob/main/CMake/SlicerExtensionCPackBundleFixup.cmake.in).

### How to check if an extension is built by Slicer Extensions build system?

Sometimes it is desirable to build the same source code in two different modes: as a standalone package or as a Slicer extension. To differentiate the two cases, the developer could check for the value of `<ExtensionName>_BUILD_SLICER_EXTENSION` CMake variable. This variable will be set to ON when the extension is built by the Slicer Extensions build system and it is not set otherwise. See details [here](https://github.com/Slicer/Slicer/blob/0ac1d7c71e2faf6dc01262e48a9d18b93f731da4/Extensions/CMake/SlicerBlockBuildPackageAndUploadExtension.cmake#L142)

### How often extensions are uploaded on the extensions server?

Slicer extensions are built and uploaded to the extensions server every day.

- Packages for Slicer stable release are rebuilt and uploaded during the day (Eastern time). Results are available at <https://slicer.cdash.org/index.php?project=SlicerStable>
- Packages for the latest Slicer Preview Release is built every night (Eastern time). Results are available at <https://slicer.cdash.org/index.php?project=SlicerPreview>

Note that packages are not updated for previous Slicer Preview Releases. To get latest extensions for a Slicer Preview Release, install the latest Slicer Preview Release.

### Will an extension be uploaded if associated tests are failing?

Independently of the extension test results, if the extension could be successfully packaged, it will be uploaded on the extensions server.

### How do I associate a remote with my local extension git source directory?

- Start a terminal (or Git Bash on Windows)
- Get the associated SSH remote url. [Need help?](https://docs.github.com/get-started/getting-started-with-git/about-remote-repositories#cloning-with-ssh)
- Associate the remote URL with your local git source tree

    ```bash
    git remote add origin https://github.com/<username>/MyExtension
    ```

### Which remote name is expected for extension git checkout?

When packaging an extension and generating the associated extension description file, the system will look for a remote named `origin`.

In case you get the error reported below, you will have to either rename or add a remote. [Need help?](https://git-scm.com/book/en/Git-Basics-Working-with-Remotes)

```text
CMake Warning at /path/to/Slicer/CMake/FindGit.cmake:144 (message):
No remote origin set for git repository: /path/to/MyExtension
Call Stack (most recent call first):
/path/to/Slicer/CMake/SlicerMacroExtractRepositoryInfo.cmake:99 (GIT_WC_INFO)
/path/to/Slicer/CMake/SlicerExtensionCPack.cmake:55 (SlicerMacroExtractRepositoryInfo)
CMakeLists.txt:25 (include)
```

### Why ExtensionWizard failed to describe extension: "script does not set 'EXTENSION_HOMEPAGE'"?

The issue is that the your extension has a "non standard" layout and the wizard has no way of extracting the expected information.

Similar issue has been discussed and reported for the SPHARM-PDM or UKF extension.

First half of the solution would be to move the metadata from `Common.cmake` to `CMakeLists.txt` as it is done in [here](https://github.com/NIRALUser/SPHARM-PDM/commit/15badbabd930573a9e251d1fd525313ea000d028). Then, you could make sure there is a project() statement by following what is suggested [here](https://mantisarchive.slicer.org/view.php?id=3737#c12081).

If you prefer not to re-organize your extension, you could still contribute extension description file by creating it manually.

### Is project() statement allowed in extension CMakeLists.txt?

Following [Slicer r22038](https://github.com/Slicer/Slicer/commit/93cc799c97ca0528fc4cf25a1e49791bd3742677), the project statement is required.

### Is call to "if(NOT Slicer_SOURCE_DIR)" required to protect "find_package(Slicer)" in extension CMakeLists.txt?

Following [Slicer r22063](https://github.com/Slicer/Slicer/commit/d1d0699aeaff85d7500269c7aaa78afdf8d3aa4a), protecting call to `find_package(Slicer)` with `if(NOT Slicer_SOURCE_DIR)` is no longer needed and should be removed to keep code simpler and easier to maintain.

Before:

```cmake
cmake_minimum_required(VERSION 2.8.9)

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer COMPONENTS ConfigurePrerequisites)
endif()

if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME EmptyExtensionTemplate)
  set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/EmptyExtensionTemplate")
  set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware)")
  set(EXTENSION_DESCRIPTION "This is an example of extension bundling N module(s)")
  set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/EmptyExtensionTemplate/EmptyExtensionTemplate.png?revision=21746&view=co")
  set(EXTENSION_SCREENSHOTURLS "https://www.slicer.org/w/img_auth.php/4/42/Slicer-r19441-EmptyExtensionTemplate-screenshot.png")
endif()

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

add_subdirectory(ModuleA)

if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
```

After:

```cmake
cmake_minimum_required(VERSION 2.8.9)

find_package(Slicer COMPONENTS ConfigurePrerequisites)

project(EmptyExtensionTemplate)

set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/EmptyExtensionTemplate")
set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware)")
set(EXTENSION_DESCRIPTION "This is an example of empty extension")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/EmptyExtensionTemplate/EmptyExtensionTemplate.png?revision=21746&view=co")
set(EXTENSION_SCREENSHOTURLS "https://www.slicer.org/w/img_auth.php/4/42/Slicer-r19441-EmptyExtensionTemplate-screenshot.png")

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

add_subdirectory(ModuleA)

include(${Slicer_EXTENSION_CPACK})
```

### Why is the --contribute option is not available with the ExtensionWizard?

Wizard contribute option is available only (1) if Slicer is built with OpenSSL support or (2) directly from the nightly.

To build Slicer with SSL support, you need to build (or download) Qt with SSL support and configure Slicer with `-DSlicer_USE_PYTHONQT_WITH_OPENSSL:BOOL=ON`

### How to package third party libraries?

Extensions integrating third party libraries should follow the [SuperBuild extension template](https://github.com/Slicer/Slicer/tree/main/Utilities/Templates/Extensions/SuperBuild).

Each third party libraries will be configured and built using a dedicated `External_MyLib.cmake` file, the install location of binaries and libraries should be set to `Slicer_INSTALL_BIN_DIR` and `Slicer_INSTALL_LIB_DIR`.

Also, starting with [Slicer v4.8.0](https://github.com/Slicer/Slicer/releases/tag/v4.8.0) (introduced in [this commit](https://github.com/Slicer/Slicer/commit/b16753775e49bca14c456a4b45803e8fb9e97eab)), extension can package python modules and packages using `PYTHON_SITE_PACKAGES_SUBDIR` CMake variable to specify the install destination.

These relative paths are the one that the extensions manager will consider when generating the launcher and application settings for a given extension.

### Can I use C++14/17/20 language features?

We try to balance between compatibility and using new features. As a result, currently C++14 features are allowed, but usage of C++17/20 language features are discouraged in extensions (relying on C++17/20 features may lead to build errors on some build configurations).

If your extension can be compiled as a standalone project where you would like to use newer feature, you could rely on CMake detecting compile features. See [cmake-compile-features](https://cmake.org/cmake/help/v3.5/manual/cmake-compile-features.7.html) for more details.

See the labs topic on [upgrading compiler infrastructure](https://www.slicer.org/wiki/Documentation/Labs/UpgradingCompilerInfrastructure) for additional information/status.

### How do I publish a paper about my extension?

Consider publishing a paper describing your extension. [This page](https://www.software.ac.uk/resources/guides/which-journals-should-i-publish-my-software) contains a list of journals that publish papers about software. [Citing 3D Slicer](../user_guide/about.md#how-to-cite) in all papers that use 3D Slicer is greatly appreciated.

### How to force Slicer to download extensions corresponding to a different Slicer revision?

Since extensions available from the Extensions Manager are associated with a particular Slicer revision, for development versions, typically no extension builds will appear in the Extensions Manager. For testing purposes, the current revision can be overridden in the Python console in Slicer:

```python
>>> extensionManagerModel = slicer.app.extensionsManagerModel()
>>> extensionManagerModel.slicerRevision = "25742"
```

On other approach is to re-configure and build Slicer setting the `Slicer_FORCED_WC_REVISION` option.

### How to address ITK test driver caught an ITK exception "Could not create IO object for reading file"?

If the following exception is reported when trying to run tests associated with a CLI modules:

```text
 ITK test driver caught an ITK exception:

 itk::ImageFileReaderException (0x1bd8430)
 Location: "unknown"
 File: /path/to/Slicer-SuperBuild/ITK/Modules/IO/ImageBase/include/itkImageFileReader.hxx
 Line: 139
 Description:  Could not create IO object for reading file /path/to/image.nrrd
   Tried to create one of the following:
     MRMLIDImageIO
   You probably failed to set a file suffix, or
     set the suffix to an unsupported type.
```

It most likely means that the test driver is not linking against `ITKFactoryRegistration` library and/or registering the ITK factories. To address this, the test driver should be updated:
- link against `${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES}`
- include `itkFactoryRegistration.h`
- call `itk::itkFactoryRegistration();` in its main function.

For more details, read [What is the ITKFactoryRegistration library?](https://www.slicer.org/wiki/Documentation/Nightly/Developers/FAQ#What_is_the_ITKFactoryRegistration_library_.3F).
