# GNU/Linux systems

The instructions to build Slicer for GNU/Linux systems are slightly different
depending on the Linux distribution and the specific configuration of the
system. In the following sections, you can find instructions that will work for
some of the most common Linux distributions in their standard configuration. If
you are using a different distribution, you can use [these instructions](./linux.md#any-distribution)
to adapt the process to your system. You can also ask questions
related to the building process in the [Slicer forum](https://discourse.slicer.org).

## Prerequisites

First, you need to install the tools that will be used for fetching the source
code of Slicer, generating the project files, and building the project.

- Git for fetching the code and version control.
- GNU Compiler Collection (GCC) for code compilation.
- CMake for configuration/generation of the project.
  - (Optional) CMake curses GUI to configure the project from the command line.
  - (Optional) CMake Qt GUI to configure the project through a GUI.
- GNU Make
- GNU Patch

In addition, Slicer requires a set of support libraries that are not included as
part of the *superbuild*:

::::{tab-set}

:::{tab-item} Qt 5
:sync: qt5

- Qt5 with the components listed below. Qt version 5.15.2 is recommended; other Qt versions are not tested and may cause build errors or may cause problems when running the application.
  - Multimedia
  - UiTools
  - XMLPatterns
  - SVG
  - WebEngine
  - Script
  - X11Extras
  - Private
- libXt

:::

:::{tab-item} Qt 6
:sync: qt6

- Qt6 with the components listed below. Qt version **6.8 or later** is required.
  - Multimedia
  - UiTools
  - SVG
  - WebEngine
  - Core5Compat
  - Private
- libXt

:::

::::

### Debian 12 Bookworm (Stable)

*Supported until ~2028.*

Install the development tools and the support libraries:

:::::{tab-set}

::::{tab-item} Qt 5
:sync: qt5

```console
sudo apt update && sudo apt install git build-essential cmake cmake-curses-gui cmake-qt-gui \
  qtmultimedia5-dev qttools5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qtwebengine5-dev qtscript5-dev \
  qtbase5-private-dev libqt5x11extras5-dev libxt-dev libssl-dev
```

::::

::::{tab-item} Qt 6
:sync: qt6

:::{warning}
Debian 12 ships Qt 6.4.2, which is below the officially supported minimum (Qt 6.8). Builds with
the system Qt6 packages are **not officially supported and are used at your own risk**. For a
fully supported Qt6 build, install Qt 6.8 or later from the [Qt online installer](#any-distribution)
and set `Qt6_DIR` accordingly in the configure step.
:::

```console
sudo apt update && sudo apt install git build-essential cmake-curses-gui cmake-qt-gui \
  qt6-base-dev qt6-base-private-dev qt6-multimedia-dev qt6-tools-dev \
  qt6-svg-dev qt6-5compat-dev qt6-webengine-dev qt6-webengine-dev-tools qt6-scxml-dev \
  libxt-dev libssl-dev
```

::::

:::::

:::{note}
The CMake version currently included in Debian 12 Bookworm (Stable) (3.25.1) is not compatible
with the current development version of Slicer — Slicer explicitly blocks CMake 3.25.0–3.25.2
due to a [known CMake bug](https://gitlab.kitware.com/cmake/cmake/-/issues/24567). You will need to
install CMake manually by downloading CMake version >= 3.25.3 from the
[CMake releases page](https://github.com/Kitware/CMake/releases/latest) and following the
installation instructions below.

```console
wget https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-linux-x86_64.sh
chmod +x cmake-3.31.6-linux-x86_64.sh
sudo ./cmake-3.31.6-linux-x86_64.sh --prefix=/usr/local --skip-license --exclude-subdir
```

After installation, verify: `cmake --version` should report `3.31.6`.
:::

:::{note}
On Debian 12 Bookworm (Stable), the included OpenSSL version (3.0.x) requires passing
`-DSlicer_USE_SYSTEM_OpenSSL=ON` to the cmake configuration command (see the
[Configure section](#configure-and-generate-the-slicer-build-project-files) below).
:::

### Ubuntu 25.04 (Plucky Puffin)

*Standard release, EOL Jan 2026.*

Install the development tools and the support libraries:

::::{tab-set}

:::{tab-item} Qt 5
:sync: qt5

```console
sudo apt update && sudo apt install git git-lfs build-essential \
  libqt5x11extras5-dev qtmultimedia5-dev libqt5svg5-dev qtwebengine5-dev libqt5xmlpatterns5-dev qttools5-dev qtbase5-private-dev \
  libxt-dev
```

:::

:::{tab-item} Qt 6
:sync: qt6

```console
sudo apt update && sudo apt install git git-lfs build-essential \
  qt6-base-dev qt6-base-private-dev qt6-multimedia-dev qt6-tools-dev \
  qt6-svg-dev qt6-5compat-dev qt6-webengine-dev qt6-webengine-dev-tools qt6-scxml-dev \
  libxt-dev libssl-dev
```

:::

::::

:::{note}
The CMake version currently included in Ubuntu 25.04 is CMake 3.31.6 which is compatible with the current development version of Slicer. **Last time tested: 2026-03-20.**
:::

### Ubuntu 24.04 (Noble Numbat)

*LTS, supported until Apr 2029.*

Install the development tools and the support libraries:

:::::{tab-set}

::::{tab-item} Qt 5
:sync: qt5

```console
sudo apt update && sudo apt install git git-lfs build-essential \
  libqt5x11extras5-dev qtmultimedia5-dev libqt5svg5-dev qtwebengine5-dev libqt5xmlpatterns5-dev qttools5-dev qtbase5-private-dev \
  libxt-dev
```

::::

::::{tab-item} Qt 6
:sync: qt6

:::{warning}
Ubuntu 24.04 ships Qt 6.4.2, which is below the officially supported minimum (Qt 6.8). Builds with
the system Qt6 packages are **not officially supported and are used at your own risk**. For a
fully supported Qt6 build, install Qt 6.8 or later from the [Qt online installer](#any-distribution)
and set `Qt6_DIR` accordingly in the configure step.
:::

```console
sudo apt update && sudo apt install git git-lfs build-essential \
  qt6-base-dev qt6-base-private-dev qt6-multimedia-dev qt6-tools-dev \
  qt6-svg-dev qt6-5compat-dev qt6-webengine-dev qt6-webengine-dev-tools qt6-scxml-dev \
  libxt-dev libssl-dev
```

::::

:::::

:::{note}
The CMake version currently included in Ubuntu 24.04 is CMake 3.28.3 which is compatible with the current development version of Slicer. **Last time tested: 2026-03-20.**
:::

### Ubuntu 22.04 (Jammy Jellyfish)

*LTS, supported until Apr 2027.*

Install the development tools and the support libraries:

:::::{tab-set}

::::{tab-item} Qt 5
:sync: qt5

```console
sudo apt update && sudo apt install git build-essential cmake cmake-curses-gui cmake-qt-gui \
  libqt5x11extras5-dev qtmultimedia5-dev libqt5svg5-dev qtwebengine5-dev libqt5xmlpatterns5-dev qttools5-dev qtbase5-private-dev \
  libxt-dev libssl-dev
```

::::

::::{tab-item} Qt 6
:sync: qt6

:::{warning}
Ubuntu 22.04 ships Qt 6.2.4, which is below the officially supported minimum (Qt 6.8). Qt6 builds
on Ubuntu 22.04 require installing Qt 6.8 or later from the [Qt online installer](#any-distribution).

Install the system dependencies first:

```console
sudo apt update && sudo apt install git build-essential cmake cmake-curses-gui cmake-qt-gui \
  libxt-dev libssl-dev
```

Then follow the [Any Distribution](#any-distribution) section to install Qt 6.8 or later, and
configure the build with `-DQt6_DIR` pointing to the installer location.
:::

::::

:::::

### ArchLinux

:::{warning}
ArchLinux uses a rolling-release package distribution approach. This means that the versions of the packages will change over time and the following instructions might not be actual. **Last time tested: 2024-01-01.**
:::

You could build Slicer using the `PKGBUILD` from AUR: [3dslicer](https://aur.archlinux.org/packages/3dslicer) and [3dslicer-git](https://aur.archlinux.org/packages/3dslicer-git).

### Any Distribution

This section describes how to install Qt as distributed by *The QT Company*, which can be used for any GNU/Linux distribution.

:::{important}
This process requires an account in [qt.io](https://qt.io)

:::

Download the Qt Linux online installer and make it executable:

```console
 curl -LO https://download.qt.io/official_releases/online_installers/qt-online-installer-linux-x64-online.run
 chmod +x qt-online-installer-linux-x64-online.run
```

:::::{tab-set}

::::{tab-item} Qt 5
:sync: qt5

You can run the installer and follow the instructions in the GUI. Keep in mind that the components needed by 3D Slicer are: `qt.qt5.5152.gcc_64`, `qt.qt5.5152.qtwebengine` and `qt.qt5.5152.qtwebengine.gcc_64`.

Alternatively, you can request the installation of the components with the following command (you will be prompted for license agreements and permissions):

```console
export QT_ACCOUNT_LOGIN=<set your qt.io account email here>
export QT_ACCOUNT_PASSWORD=<set your password here>
./qt-online-installer-linux-x64-online.run \
  install \
    qt.qt5.5152.gcc_64 \
    qt.qt5.5152.qtwebengine \
    qt.qt5.5152.qtwebengine.gcc_64 \
  --root /opt/qt \
  --email $QT_ACCOUNT_LOGIN \
  --pw $QT_ACCOUNT_PASSWORD
```

:::{hint}
When configuring the Slicer build project, set the CMake variable `Qt5_DIR` to the full path of the Qt5 installation directory ending with `5.15.2/gcc_64/lib/cmake/Qt5`. For example, assuming Qt is installed in `/opt/qt`:

```console
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DQt5_DIR:PATH=/opt/qt/5.15.2/gcc_64/lib/cmake/Qt5 ../Slicer
```
:::

::::

::::{tab-item} Qt 6
:sync: qt6

You can run the installer and follow the instructions in the GUI. The components needed by 3D Slicer include the base Qt6 libraries, Qt WebEngine, and Qt5Compat. Select Qt version 6.8 or later.

Alternatively, you can request the installation of the components with the following command (you will be prompted for license agreements and permissions):

```console
export QT_ACCOUNT_LOGIN=<set your qt.io account email here>
export QT_ACCOUNT_PASSWORD=<set your password here>
./qt-online-installer-linux-x64-online.run \
  install \
    qt.qt6.680.gcc_64 \
    qt.qt6.680.addons.qtwebengine \
    qt.qt6.680.addons.qt5compat \
  --root /opt/qt \
  --email $QT_ACCOUNT_LOGIN \
  --pw $QT_ACCOUNT_PASSWORD
```

:::{hint}
When configuring the Slicer build project, set the CMake variable `Qt6_DIR` to the full path of the Qt6 installation directory ending with `6.8.0/gcc_64/lib/cmake/Qt6`. For example, assuming Qt is installed in `/opt/qt`:

```console
cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DSlicer_REQUIRED_QT_VERSION=6 \
  -DQt6_DIR:PATH=/opt/qt/6.8.0/gcc_64/lib/cmake/Qt6 \
  ../Slicer
```
:::

::::

:::::

## Checkout Slicer source files

The recommended way to obtain the source code of Slicer is cloning the repository using `git`:

```console
git clone https://github.com/Slicer/Slicer.git
```

This will create a `Slicer` directory containing the source code of Slicer.
Hereafter we will call this directory the `source directory`.

:::{tip}
It is highly recommended to **avoid** the use of the **space** character in the name of the `source directory` or any of its parent directories.

:::

After obtaining the source code, we need to set up the development environment:

```console
cd Slicer
./Utilities/SetupForDevelopment.sh
cd ..
```

% TODO: Link to the readthedocs equivalent of https://www.slicer.org/wiki/Documentation/Nightly/Developers/DevelopmentWithGit

## Configure and generate the Slicer build project files

Slicer is highly configurable and multi-platform. To support this,
Slicer needs a configuration of the build parameters before the build process
takes place. In this configuration stage, it is possible to adjust variables
that change the nature and behavior of its components. For instance, the type
of build (Debug or Release mode), whether to use system-installed libraries,
let the build process fetch and compile own libraries, or enable/disable some of
the software components and functionalities of Slicer.

The following folders will be used in the instructions below:

| Folder          | Path   |
|-----------------|--------|
| **source**      | `~/Slicer` |
| **build**       |  `~/Slicer-SuperBuild-Debug` |
| **inner-build** |  `~/Slicer-SuperBuild-Debug/Slicer-build` |

To obtain a default configuration of the Slicer build project, create the **build** folder and use `cmake`:

```console
mkdir Slicer-SuperBuild-Debug
cd Slicer-SuperBuild-Debug
cmake ../Slicer
```

It is possible to change variables with `cmake`. In the following example we
change the built type (Debug as default) to Release:

```console
cmake -DCMAKE_BUILD_TYPE:STRING=Release ../Slicer
```

:::::{tab-set}

::::{tab-item} Qt 5
:sync: qt5

If Qt5 was installed from the [Qt Company online installer](#any-distribution), the CMake variable
`Qt5_DIR` must be set:

```console
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DQt5_DIR:PATH=/opt/qt/5.15.2/gcc_64/lib/cmake/Qt5 ../Slicer
```

::::

::::{tab-item} Qt 6
:sync: qt6

To build with Qt6, set `-DQt6_DIR` pointing to the Qt6 cmake directory. Slicer will automatically
enforce the minimum required Qt6 version (6.8). When using distribution-provided Qt6 packages
(e.g. Ubuntu 25.04), `Qt6_DIR` is typically `/usr/lib/x86_64-linux-gnu/cmake/Qt6`:

```console
cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DQt6_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  ../Slicer
```

If Qt6 was installed from the [Qt Company online installer](#any-distribution), set `Qt6_DIR`
to the installer location instead:

```console
cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DQt6_DIR:PATH=/opt/qt/6.8.0/gcc_64/lib/cmake/Qt6 \
  ../Slicer
```

:::{warning}
On distributions shipping Qt6 < 6.8 (e.g. Debian 12, Ubuntu 24.04), the cmake configure step
will fail with a version error. To bypass the check and build anyway — **at your own risk and
without official support** — explicitly set `Slicer_REQUIRED_QT_VERSION=6`:

```console
cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DSlicer_REQUIRED_QT_VERSION=6 \
  -DQt6_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  ../Slicer
```
:::

::::

:::::

:::{warning}
On Debian 12 Bookworm (Stable), the included OpenSSL version (3.0.x) is not compatible with the OpenSSL versions (1.0 - 1.1) used in Slicer, and attempting to run Slicer will emit the following warning, indicating that SSL support is disabled:
```
qt.network.ssl: Incompatible version of OpenSSL (built with OpenSSL >= 3.x, runtime version is < 3.x)
[SSL] SSL support disabled - Failed to load SSL library !
[SSL] Failed to load Slicer.crt
QSslSocket::connectToHostEncrypted: TLS initialization failed
```
To enable SSL, one can use the system OpenSSL as follows:
```console
cmake -DSlicer_USE_SYSTEM_OpenSSL=ON ../Slicer
```
:::

:::{admonition} Tip -- Interfaces to change 3D Slicer configuration variables

Instead of `cmake`, one can use `ccmake`, which provides a text-based interface, or `cmake-gui`, which provides a graphical user interface. These applications will also provide a list of variables that can be changed.

:::

:::{admonition} Tip -- Speed up 3D Slicer build with `ccache`

`ccache` is a compiler cache that can speed up subsequent builds of 3D Slicer. This can be useful if 3D Slicer is built often and there are no large divergences between subsequent builds. This requires `ccache` installed on the system (e.g., `sudo apt install ccache`).

The first time `ccache` is used, the compilation time can marginally increase as it includes the first caching. After the first build, subsequent build times will decrease significantly.

`ccache` is not detected as a valid compiler by the 3D Slicer building process. You can generate local symbolic links to disguise the use of `ccache` as valid compilers:

```console
ln -s /usr/bin/ccache ~/.local/bin/c++
ln -s /usr/bin/ccache ~/.local/bin/cc
```

Then, the Slicer build can be configured to use these compilers:

```console
cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DCMAKE_CXX_COMPILER:STRING=$HOME/.local/bin/c++ \
  -DCMAKE_C_COMPILER:STRING=$HOME/.local/bin/cc \
  ../Slicer
```

:::

## Build Slicer

Once the Slicer build project files have been generated, the Slicer project can
be built by running this command in the **build** folder

```console
make
```

:::{admonition} Tip -- Parallel build

Building Slicer will generally take a long time, particularly on the first build or upon code/configuration changes. To help speed up the process, one can use `make -j<N>`, where `<N>` is the number of parallel builds. As a rule of thumb, many use the `number of CPU threads - 1` as the number of parallel builds.

:::

:::{warning}

Increasing the number of parallel builds generally increases the memory required for the build process. In the event that the required memory exceeds the available memory, the process will either fail or start using swap memory, which may make the system freeze.

:::

:::{admonition} Tip -- Error detection during parallel build

Using parallel builds makes finding compilation errors difficult due to the fact that all parallel build processes use the same screen output, as opposed to sequential builds, where the compilation process will stop at the error. A common technique to have parallel builds and easily find errors is to launch a parallel build followed by a sequential build. For the parallel build, it is advised to run `make -j<N> -k` to have the parallel build keep going as far as possible before doing the sequential build with `make`.

:::

## Run Slicer

After the building process has successfully completed, the executable file to
run Slicer will be located in the **inner-build** folder.

The application can be launched by these commands:

```console
cd Slicer-build
./Slicer`
```

## Test Slicer

After building, run the tests in the **inner-build** folder.

Type the following (you can replace 4 with the number of processor cores in the computer):

```console
ctest -j4
```

## Package Slicer

Start a terminal and type the following in the **inner-build** folder:

```console
make package
```

## Common errors

See a list of issues common to all operating systems on the [Common errors](common_errors.md) page.
