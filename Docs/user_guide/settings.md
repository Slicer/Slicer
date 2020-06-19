# Application settings

## Overview

The application settings dialog allows users to customize application behavior.

After starting Slicer, it can be accessed clicking in menu: `Edit` / `Application Settings`.

## Panels

### General

Application startup script can be used to launch any custom Python code when Slicer application is started.

### Modules

#### Skip loading

Select which [type of modules](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Modules) to not load at startup. It is also possible to start slicer by temporarily disabling those modules (not saved in settings) by passing the arguments in the command line.

For example, this command will start Slicer without any CLI loaded:

    Slicer.exe --disable-cli-modules

#### Prefer Executable CLIs

Use the executable version of a CLI instead of its shared version. CLI modules typically come in 2 forms, as shared (dll)and as executable (exe). By default, if there is a shared version, it is the one loaded by Slicer, ignoring the executable version. Loading a shared runs the module faster but increases the memory consumption. For some configurations (e.g. Windows 32b), memory is critical. Toggling this option to ON skips the loading of shared CLIs and loads executable version of CLIs instead. If there is no executable for a given CLI, the shared version is used.

#### Show hidden modules

Some modules don't have a user interface, they are hidden from the module's list. For debugging purpose, it is possible to force their display

#### Temporary directory

Directory where modules can store their temporary outputs if needed.

#### Additional module paths

List of directories scanned at startup to load additional modules. Any CLI, Loadable or scripted modules located in these paths will be loaded. Extensions are listed in the list, to remove an extension, use the [Extension Manager](extension_manager) instead.

It is also possible to start slicer by temporarily adding module paths (not saved in settings) by passing the arguments in the command line.

For example this command will start Slicer trying to load CLIs found in the specified directory:

    Slicer.exe --additional-module-paths C:\path\to\lib\Slicer-X.Y\cli-modules

#### Modules

List of modules loaded, ignored or failed to load in Slicer. An unchecked checkbox indicates that module should not be loaded (ignored) next time Slicer starts. A text color code is used to describe the state of each module:

-  Black: module successfully loaded in Slicer
-  Gray: module not loaded because it has been ignored (unchecked)
-  Red: module failed to load. There are multiple reasons why a module can fail to load.

Look at startup [log outputs](https://www.slicer.org/wiki/Documentation/Nightly/SlicerApplication/ErrorLog) to have more informations.
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
  For example, on Windows if [dark mode][(https://blogs.windows.com/windowsexperience/2016/08/08/windows-10-tip-personalize-your-pc-by-enabling-the-dark-theme/)]
  is turned on for apps, then the `Dark Slicer` style will be used upon launching Slicer. Currently, automatic detection of dark mode is not available on Linux,
  therefore use needs to manually select `Dark Slicer` style for a dark color scheme.
- Light Slicer: application window background is bright, regardless of operating system settings.
- Dark Slicer: application window background is dark, regardless of operating system settings.

## Information for Advanced Users

### Settings file location

Settings are stored in a *.ini files located in directory like these ones:

-  Windows: `%USERPROFILE%\AppData\Roaming\NA-MIC\` (typically `C:\Users\<your_user_name>\AppData\Roaming\NA-MIC\`)
-  Linux: `~/.config/NA-MIC/`
-  Mac: `~/.config/www.na-mic.org/`

Deleting the *.ini files restores all the settings to default.

There are two types of settings:

#### User specific settings

This file is named `Slicer.ini` and it stores settings applying to *all versions* of Slicer installed by the *current user*.

To display the exact location of this settings file, open a terminal and type:

    ./Slicer --settings-path

On Windows:

    Slicer.exe --settings-path | more

or enter the following in the Python interactor:

    slicer.app.slicerUserSettingsFilePath

#### User and revision specific settings

This file is named like `Slicer-<REVISION>.ini` and it stores settings applying to a *specific revision* of Slicer installed by the *current user*.

To display the exact location of this settings file, enter the following in the Python interactor:

    slicer.app.slicerRevisionUserSettingsFilePath
