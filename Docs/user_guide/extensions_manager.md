# Extensions Manager

## Overview

New features can be added to 3D Slicer by installing "extensions". An extension is a delivery package bundling together one or more Slicer modules. After installing an extension, the associated modules will be presented to the user the same way as built-in modules.

The Slicer community maintains a website referred to as the [Slicer Extensions Catalog](https://extensions.slicer.org/catalog/All/30117/win) for finding and downloading extensions. Extensions manager in Slicer makes the catalog available directly in the application and allows extension install, update, or uninstall extensions by a few clicks.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/extensions_manager.png)

## How to

### Install extensions

- Open Extensions manager using menu: View / Extensions manager. On macOS, Extensions manager requires the [application to be installed](getting_started.md#mac).
- To restore previously installed extensions:
  - Go to "Restore Extensions" tab
  - Check the checkbox on the left side of each extension that should be installed. If the extension that you want to install is disabled and has "not found for current Slicer version" message is displayed below its name then follow instructions in [troubleshooting section](#extension-is-not-found-for-current-slicer-version).
  - Click "Install Selected" button.
- To install new extensions: go to "Install extensions" tab and click "Install" button for each extension to be installed.
- If "Install dependencies" window appears, click "Yes" to install extensions that the selected extension requires. If dependencies are not installed then the chosen extension may not work properly.
- Wait until "Restart" button in the lower-right corner becomes enabled, then click "Restart".
- Click "OK" if you are asked to restart Slicer, unless you have valuable changes in the scene. If there are changes in the scene that needs to be saved then choose "Cancel", save the scene, and restart the application manually.

### Update extensions

#### Update extensions for Slicer Stable Releases

- Open Extensions manager using menu: View / Extensions manager.
- Go to "Manage extensions" tab.
- Click the tools button ("wrench" icon in the top-right corner) and select "Check for updates".
- Click "Update" button for each extension to update. If "An update is available" message is not displayed for any of the extensions then it means that extensions are already up-to-date.
- Click "Restart" button to restart the application.

By selecting "Install Updates Automatically" from the tools menu, Slicer will check if extension updates are available at start-up and choose all of them for update.

Extensions are updated every night for the latest Slicer Stable Release. Extensions are not updated for earlier Slicer Stable Releases.

#### Update extensions for Slicer Preview Releases

Each Slicer Preview Release and all corresponding extensions are built at once, and only once (every night, from the latest version of Slicer).
To get latest extensions for a Slicer Preview Release, download and install the latest Slicer and reinstall the extensions.

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

Extensions can be downloaded from the Extensions catalog using any web browser then installed manually.

#### Download extension packages

- Get application revision number from menu: Help / About 3D Slicer. The revision is a number preceded by r character, for example: if the complete version string is 4.13.0-2021-03-05 r29749 / 121ffbd the application revision number is 29749.
- Start a web browser and open the extensions catalog (app store) website. The default Slicer extensions catalog is available at: [https://extensions.slicer.org](https://extensions.slicer.org). To see a list of extensions, operating system (`win`, `linux`, `macosx`) and Slicer reviesion must be provided in the format: `https://extensions.slicer.org/catalog/All/<revision>/<os>`, for example <https://extensions.slicer.org/catalog/All/30117/win>
- Select operating system (Windows, Linux, or Mac OSX), bitness (64-bit), and revision (the application revision number retrieved in the first step). If "No extensions found..." message is displayed then check [troubleshooting section](#extensions-manager-does-not-show-any-extensions).
- Click "Download" button of the selected extension(s) to download the extension package.

#### Install downloaded extension packages

- Open Extensions manager using menu: View / Extensions manager.
- Click the "Install extension from file" button (folder icon in the top-right corner).
- Select the the previously downloaded extension package.
- Wait for the "Installed extension..." window and click "OK".
- Install additional extensions as needed, then click "Restart" button to restart the application.

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
