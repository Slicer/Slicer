# Application settings

## Editing application settings

The application settings dialog allows users to customize application behavior.

After starting Slicer, it can be accessed clicking in menu: `Edit` / `Application Settings`.

### General

Application startup script can be used to launch any custom Python code when Slicer application is started.

### Modules

#### Skip loading

Select which [type of modules](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Modules) to not load at startup. It is also possible to start Slicer by temporarily disabling those modules (not saved in settings) by passing the arguments in the command line.

For example, this command will start Slicer without any CLI loaded:

    Slicer.exe --disable-cli-modules

#### Show hidden modules

Some modules don't have a user interface, they are hidden from the module's list. For debugging purpose, it is possible to force their display

#### Temporary directory

Directory where modules can store their temporary outputs if needed.

#### Additional module paths

List of directories scanned at startup to load additional modules. Any CLI, Loadable or scripted modules located in these paths will be loaded.

Module folders of extensions are included in this list. To remove modules of an extension, it is recommended to use the [Extensions Manager](extensions.md#extensions-manager) instead of just removing its module paths.

It is also possible to start Slicer by temporarily adding module paths (not saved in settings) by passing the arguments in the command line.

For example this command will start Slicer trying to load CLIs found in the specified directory:

    Slicer.exe --additional-module-paths C:\path\to\lib\Slicer-X.Y\cli-modules

#### Modules

List of modules loaded, ignored or failed to load in Slicer. An unchecked checkbox indicates that module should not be loaded (ignored) next time Slicer starts. A text color code is used to describe the state of each module:

-  Black: module successfully loaded in Slicer
-  Gray: module not loaded because it has been ignored (unchecked)
-  Red: module failed to load. There are multiple reasons why a module can fail to load.

Look at startup [log outputs](https://www.slicer.org/wiki/Documentation/Nightly/SlicerApplication/ErrorLog) to have more information.
If a module is not loaded in Slicer (ignored or failed), all dependent modules won't be loaded. You can verify the dependencies of a module in the tooltip of the module.

You can filter the list of modules by untoggling in the advanced (>>) panel the "To Load", "To Ignore", "Loaded", "Ignored" and "Failed" buttons.

#### Home

Module that is shown when Slicer starts up.

#### Favorites

List of modules that appear in the Favorites toolbar:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/settings_favorites_toolbar.png)

To add a module, drag&drop it from the *Modules* list above. Then use the advanced panel (>>) to reorganize/delete the modules within the toolbar.

### Appearance

#### Style

The overall theme of Slicer is controlled by the selected Style:
- Slicer (default): it sets the style based on theme settings set by the operating system.
  For example, on Windows if [dark mode](https://blogs.windows.com/windowsexperience/2016/08/08/windows-10-tip-personalize-your-pc-by-enabling-the-dark-theme/)
  is turned on for apps, then the `Dark Slicer` style will be used upon launching Slicer. Currently, automatic detection of dark mode is not available on Linux,
  therefore use needs to manually select `Dark Slicer` style for a dark color scheme.
- Light Slicer: application window background is bright, regardless of operating system settings.
- Dark Slicer: application window background is dark, regardless of operating system settings.

### Developer

#### Developer mode

Enable the following features:

* Testing modules are loaded: modules whose name ends with `Test` or `Tests` are only loaded at application startup if developer mode is enabled. Skipping them shortens the application startup time, in particular for scripted testing modules, which would need to be imported just to be registered. Testing modules are always loaded if the application is started in testing mode, as automated tests rely on them.
* [Module selection toolbar](user_interface.md#toolbar): Modules associated with the _Testing_ category are visible by default.
* [WebServer module](modules/webserver.md): Javascript logging is enabled by default.
* [Module panel](user_interface.md#module-panel): `Reload & Test` module panel section is displayed for scripted modules. It includes controls for reloading, testing and editing scripted modules as well as restarting the application.

modules/webserver.html


## Information for Advanced Users

### Settings file location

Settings are stored in *.ini files. If the settings file is found in application home directory (within organization name or domain subfolder) then that .ini file is used. This can be used for creating a portable application that contains all software and settings in a relocatable folder. Relative paths in settings files are resolved using the application home directory, and therefore are portable along with the application.

If .ini file is not found in the application home directory then it is searched in user profile:

-  Windows: `%USERPROFILE%\AppData\Roaming\slicer.org\` (typically `C:\Users\<your_user_name>\AppData\Roaming\slicer.org\`)
-  Linux: `~/.config/slicer.org/`
-  Mac: `~/.config/slicer.org/`

Deleting the *.ini files restores all the settings to default.

There are two types of settings: `user specific settings` and `user and revision specific settings`.

#### User specific settings

This file is named `Slicer.ini` and it stores settings applying to *all versions* of Slicer installed by the *current user*.

To display the exact location of this settings file, open a terminal and type:

    ./Slicer --settings-path

On Windows:

    Slicer.exe --settings-path | more

or enter the following in the Python console:

    slicer.app.slicerUserSettingsFilePath

#### User and revision specific settings

This file is named like `Slicer-<REVISION>.ini` and it stores settings applying to a *specific revision* of Slicer installed by the *current user*.

To display the exact location of this settings file, enter the following in the Python console:

    slicer.app.slicerRevisionUserSettingsFilePath

### Application startup file

Each time Slicer starts, it will look up for a startup script file named <code>.slicerrc.py</code>. Content of this file is executed automatically at each startup of Slicer.

The file is searched at multiple location and the first one that is found is used. Searched locations:
- Application home folder (`slicer.app.slicerHome`)
- Path defined in `SLICERRC` environment variable
- User profile folder (`~/.slicerrc.py`)

You can find the path to the startup script in Slicer by opening in the menu: Edit / Application Settings. ''Application startup script'' path is shown in the ''General'' section (or running `getSlicerRCFileName()` command in Slicer Python console).

### Startup performance optimization

If the application takes a long time to start up then the `--report-startup-timing` command-line option can be used to find out where that time is spent. Once the startup is complete, a report is written to the application log (it can be displayed by opening in the menu: View / Error Log), containing:
- how much time was spent before the application's entry point was reached, which is the time the operating system needed to load the application's own libraries,
- how much time each startup phase took (initializing the application, registering, instantiating and loading modules, initializing the user interface, and showing the main window),
- the slowest 15 modules, each with the time it took to instantiate and to load it,
- the total time measured from the creation of the process,
- what the startup file prefetcher did, if it is available in the build (see below).

For example:

    Slicer.exe --report-startup-timing

The option can be combined with `--exit-after-startup`, which quits the application as soon as the startup is complete, and with `--disable-modules` or `--disable-scripted-loadable-modules`, to see how much of the startup time a group of modules is responsible for.

The application libraries can be read into the file system cache on background threads while the application starts up, so that the work the operating system does when a library is first touched overlaps with the startup instead of delaying it. On Windows that work is on-access virus scanning, which is slow enough that this prefetching is enabled by default there. On Linux and macOS there is no such scanning and only the file cache is gained, which is worth a lot on a cold cache, a network file system or a spinning disk and nothing on a warm one, so prefetching is disabled by default and can be turned on with the `SLICER_STARTUP_FILE_PREFETCH` environment variable described below. Startup timing reports include how many libraries were prefetched and how many of the libraries this startup needed were predicted by the previous run, which tells if the prefetching could help at all.

Testing modules are not loaded unless [developer mode](#developer-mode) is enabled, which also shortens the startup time.

### Runtime environment variables

The following environment variables can be set before the application is started to fine-tune its behavior:
- `PYTHONNOUSERSITE`: if it is set to `1` then import of user site packages is disabled. For example, this will prevent Slicer to reuse packages downloaded/built by Anaconda.
- `QT_SCALE_FACTOR`: see [Qt documentation](https://doc.qt.io/qt-5/highdpi.html). For example, font size can be reduced by running `set QT_SCALE_FACTOR=0.5` in the command console and then starting Slicer in that console.
- `QT_ENABLE_HIGHDPI_SCALING`: see [Qt documentation](https://doc.qt.io/qt-5/highdpi.html)
- `QT_SCALE_FACTOR_ROUNDING_POLICY`: see [Qt documentation](https://doc.qt.io/qt-5/highdpi.html)
- `QTWEBENGINE_REMOTE_DEBUGGING`: port number for Qt webengine remote debugger. Default value is `1337`.
- `SLICER_OPENGL_PROFILE`: Requested OpenGL profile. Valid values are `no` (no profile), `core` (core profile),
  and `compatibility` (compatibility profile). Default value is `compatibility` on Windows systems.
- `SLICER_BACKGROUND_THREAD_PRIORITY`: Set priority for background processing tasks. On Linux, it may affect the
  entire process priority. An integer value is expected, default = `20` on Linux and macOS, and `-1` on Windows.
- `SLICER_STARTUP_FILE_PREFETCH`: If it is set to `1` then the application libraries are read into the file system cache
  while the application starts up, and if it is set to `0` then they are not. If the variable is left unset (or set to any
  other value) then prefetching is enabled on Windows, where parallel file prefetching significantly improve startup time, and
  disabled on Linux and macOS, where only the file cache is gained and whether that helps depends on the computer.
  It has an effect only in builds configured with `Slicer_BUILD_STARTUP_FILE_PREFETCH` enabled.
- `SLICER_STARTUP_FILE_PREFETCH_THREADS`: Number of threads used for prefetching the application libraries at startup.
  A positive integer value is expected, by default the number of CPU cores is used, limited to the range of `2` to `8`.
- `SLICERRC`: Custom application startup file path. Contains a full path to a Python script. By default it is `~/.slicerrc.py` (where ~ is the user profile a.k.a user home folder).
- `SLICER_EXTENSIONS_MANAGER_SERVER_URL`: URL of the extensions manager backend with the `/api` path. Default value is retrieved from the settings using the key `Extensions/ServerUrl`.
- `SLICER_EXTENSIONS_MANAGER_FRONTEND_SERVER_URL`: URL of the extension manager frontend displaying the web page. Default value is retrieved from the settings using the key `Extensions/FrontendServerUrl`.
- `SLICER_EXTENSIONS_MANAGER_SERVER_API`: Supported value is `Girder_v1`. Default value is hard-coded to `Girder_v1`.

### Qt built-in command-line options

Slicer application accepts standard Qt command-line arguments that specify how Qt interacts with the windowing system.

Examples of options:
- `-qwindowgeometry geometry`, specifies window geometry for the main window using the X11-syntax. For example: `-qwindowgeometry 100x100+50+50`.
- `-display hostname:screen_number`, switches displays on X11 and overrides the `DISPLAY` environment variable.
- `-platform windows:dpiawareness=[0|1|2]`, sets the [DPI awareness](https://doc.qt.io/qt-5/highdpi.html#microsoft-windows) on Windows.
- `-widgetcount`, prints debug message at the end about number of widgets left undestroyed and maximum number of widgets existed at the same time.
- `-reverse`, sets the application's layout direction to `Qt::RightToLeft`.

To learn about the supported options:
- https://doc.qt.io/qt-5/qapplication.html#QApplication
- https://doc.qt.io/qt-5/qguiapplication.html#supported-command-line-options

:::{note}

Since the Slicer launcher is itself a Qt application and the Qt built-in command-line options are expected to **only** be passed to
the launched application `SlicerApp-real` and not the Slicer launcher, the list of arguments to filter is specified in the [Main.cpp](https://github.com/commontk/AppLauncher/blob/master/Main.cpp#L35) found in the `commontk/AppLauncher` project.

:::
