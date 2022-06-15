# Extensions Manager

## Overview

New features can be added to 3D Slicer by installing "extensions". An extension is a delivery package bundling together one or more Slicer modules. After installing an extension, the associated modules will be presented to the user the same way as built-in modules.

The Slicer community maintains a website referred to as the [Slicer Extensions Catalog](https://extensions.slicer.org/catalog/All/30822/win) for finding and downloading extensions. Extensions manager in Slicer makes the catalog available directly in the application and allows extension install, update, or uninstall extensions by a few clicks.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/extensions_manager.png)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/extensions_manager_update.png)

## How to

### Install extensions

- Open Extensions manager using menu: View / Extensions manager. On macOS, Extensions manager requires the [application to be installed](getting_started.md#mac).
- To install new extensions:
  - Go to "Install extensions" tab
  - Click "Install" button for each extension to be installed.
- To restore previously "bookmarked" extensions:
  - Go to "Manage Extensions" tab
  - Click "Install bookmarked" button to install all bookmarked extensions; or click "Install" button of specific extensions.
- If "Install dependencies" window appears, click "Yes" to install extensions that the selected extension requires. If dependencies are not installed then the chosen extension may not work properly. Automatic installation of dependencies can be enabled by using the "Automatically install dependencies" option in the "Settings" button's menu.
- Wait until "Restart" button in the lower-right corner becomes enabled, then click "Restart".
- Click "OK" if you are asked to restart Slicer, unless you have valuable changes in the scene. If there are changes in the scene that needs to be saved then choose "Cancel", save the scene, and restart the application manually.

:::{note}

Extensions can be "bookmarked" so that they can be easily reinstalled later, even in other Slicer versions. Bookmarks can be added or removed by clicking the orange bookmark icon in the extensions list in the "Manage extensions" tab.

All current bookmarks can be conveniently accessed and multiple bookmarks can be added using "Edit bookmarks..." in the "Settings" button's menu. This feature is also useful to quickly share the list of bookmarked extensions with others, or easily add multiple bookmarked extensions for easy installation.

:::

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/extensions_manager_settings.png)

:::{note}

If the extension that you want to install is disabled and has "Not found for this version of the application" message is displayed below its name then follow instructions in [troubleshooting section](#extension-is-not-found-for-current-slicer-version).

:::

### Update extensions

#### Update extensions for Slicer Stable Releases

- Open Extensions manager using menu: View / Extensions manager.
- Go to "Manage extensions" tab.
- Click "Check for updates" button for looking for updates for all installed extensions.
- For each extension that has an available update, "An update is available..." note is displayed.
- Click "Update all" button to install all available updates or click "Update" button for specific extensions.
- Click "Restart" button to restart the application.

:::{note}

By enabling "Automatically check for updates" in the "Settings" button's menu, Slicer will check if extension updates are available at start-up and display a small orange notification marker on the Extensions Manager icon on the toolbar.

By enabling "Automatically install updates" in the "Settings" button's menu, Slicer will automatically install updates when an update is found.

:::

Extensions are updated every night for the latest Slicer Stable Release. Extensions are not updated for earlier Slicer Stable Releases.

#### Update extensions for Slicer Preview Releases

Each Slicer Preview Release and all corresponding extensions are built at once, and only once (every night, from the latest version of Slicer).
To get latest extensions for a Slicer Preview Release, download and install the latest Slicer Preview Release and reinstall the extensions.

### Uninstall extensions

- Open Extensions manager using menu: View / Extensions manager
- Go to "Manage extensions" tab.
- Click "Uninstall" button for each extension to be uninstalled.
- Click "Restart" button to restart the application.

### Disable extensions

Extensions can be temporarily disabled. Disabled extensions are not loaded into the application but still kept on the system. They can be easily re-enabled, without downloading from the extension manager.

- Open Extensions manager using menu: View / Extensions manager
- Go to "Manage extensions" tab.
- Click "Disable" button for each extension to be uninstalled.
- Click "Restart" button to restart the application.

### Install extensions without network connection

Extensions can be downloaded from the Extensions Catalog website and can be installed manually, without network connection, in Slicer using the Extensions manager.

#### Download extension packages

- Opening the Extension Catalog in the default web browser on your system by clicking on "Open Extensions Catalog website" in the "Settings" button's menu
  - Alternatively, open the website in any web browser using a link like this: `https://extensions.slicer.org/catalog/All/<revision>/<os>`, for example <https://extensions.slicer.org/catalog/All/30117/win>. `revision` refers to the application revision number that can be obtained from menu: Help / About 3D Slicer (the revision is a number preceded by r character, for example: if the complete version string is 4.13.0-2021-03-05 r29749 / 121ffbd the application revision number is 29749). `os` refers to the operating system, use `win`, `linux`, or `macosx`.
- Click "Download" button of the selected extension(s) to download the extension package.

#### Install downloaded extension packages

- Open Extensions manager using menu: View / Extensions manager.
- Click the "Install fom file..." button.
- Select the the previously downloaded extension package(s). Multiple extension packages can be selected at once.
- Wait for the installations to complete.
- Click "Restart" button to restart the application.

## Troubleshooting

### Extensions manager takes very long time to start

When starting the extensions manager, the "Extensions manager is starting, please wait..." message is displayed immediately and normally list of extensions should show up within 10-20 seconds. If startup takes longer (several minutes) then most likely the the Slicer Extensions Catalog server is temporarily overloaded. Retry in an hour. If the problem persists after 6 hours then report it on the [Slicer forum](https://discourse.slicer.org).

### Extensions manager does not show any extensions

This can be due to several reasons:
- On macOS: Extensions manager displays the message "Extensions can not be installed" if the application is not installed. See [application installation instructions](getting_started.md#mac).
- Extensions Catalog server is temporarily overloaded (indicated by extensions manager taking several minutes to start and having dozens of `Error retrieving extension metadata` messages in the application log)
  - Recommended action: retry installing extensions an hour later.
- Extensions have not yet been built for the installed Slicer Preview Release (extensions are made available for latest Slicer Preview Release each day at around 12pm EST)
  - Recommended action: wait a few hours until the extension becomes available.
  - Alternative solution A: [install Slicer Preview Release created the day before](https://download.slicer.org/?offset=-1) (this is a special link that uses `offset=-1` to request builds from a day before)
  - Alternative solution B: [install latest Slicer Stable Release](https://download.slicer.org).
- Extensions manager does not have network access
  - Recommended action: Make sure you have internet access. Check your system proxy settings (Slicer uses these system proxy settings by default) and/or set proxy server information in environment variables `http_proxy` and `https_proxy` ([more information](https://doc.qt.io/qt-5/qnetworkproxyfactory.html#systemProxyForQuery)). On Windows, it may be necessary to re-apply the proxy settings by disabling and re-enabling "Automatically detect settings".
  - Alternative solution: download extension package using a web browser (possibly on a different computer) and install the extension manually. See instructions [here](#install-extensions-without-network-connection).
- On macOS: on some older macbooks, the extension manager window appears very bright, washed out (more information is in [this issue](https://github.com/Slicer/Slicer/issues/5118))
  - Recommended action: setting the operating system to dark mode fixed the issue for several users.
- On Linux: `Install Extensions` tab is blank if your linux kernel may not fulfill [Chromium sandboxing requirements](https://doc.qt.io/Qt-5/qtwebengine-platform-notes.html#sandboxing-support).
  - Recommended action: turn off sandboxing by setting this environment variable before launching Slicer `QTWEBENGINE_DISABLE_SANDBOX=1`

### Extension is not found for current Slicer version

For Slicer Stable Releases: If certain extensions are available, but a particular extension is not available for your current Slicer version then contact the maintainer of the extension. If the maintainer cannot be reached then ask for help on the [Slicer forum](https://discourse.slicer.org).

For Slicer Preview Releases, due to the constantly updating nature of the preview release, extensions may be missing at times:
- Extensions for latest Slicer Preview Release are uploaded by about 10am EST each day. If you need complete set of extensions then either wait or install previous releases as described [above](#extensions-manager-does-not-show-any-extensions).
- Factory system errors: Occasionally, issues with the factory system will prevent some or all extensions from building. See [dashboard](../developer_guide/extensions.md#continuous-integration) for status information.
- Extension build errors: The extension may not have been updated to work with your Slicer version or a problem may have been introduced recently. Contact the extension's maintainer. If the maintainer cannot be reached then ask for help on the [Slicer forum](https://discourse.slicer.org).

### Extensions manager is not visible in the menu

If extension manager is not visible then make sure that "Enable extension manager" option is enabled in Application Settings (menu: Edit / Application Settings / Extensions). If you changed the setting, Slicer has to be restarted for it to become effective.

## Extensions settings

Settings of extensions manager can be edited in menu: Edit / Application settings / Extensions.
- Extensions manager: if unchecked then Extensions manager is not shown in the menu.
- Extensions server URL: address of the server used to download and install extensions.
- Extensions installation path: directory where extension packages should be extracted and installed.

Modules associated with an extension can also be disabled one by one in menu: Edit / Application settings / Modules.
