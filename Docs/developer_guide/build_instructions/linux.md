# GNU/Linux Systems

The instructions to build Slicer for GNU/Linux systems are slightly different
depending on the linux distribution and the specific configuration of the
system. In the following sections you can find instructions that will work for
some of the most common linux distributions in their standard configuration. If
you are using a different distribution you can use these instructions as
guidelines to adapt the process to your system. You can also ask questions
related to the building process in the [Slicer forum](https://discourse.slicer.org).

## Pre-requisites

First, you need to install the tools that will be used for fetching the source
code of slicer, generating the project files and build the project.

- Git and Subversion for fetching the code and version control.
- GNU Compiler Collection (GCC) for code compilation.
- CMake for configuration/generation of the project.
  - (Optional) CMake curses gui to configure the project from the command line.
  - (Optional) CMake Qt gui to configure the project through a GUI.
- GNU Make
- GNU Patch
  
In addition, Slicer requires a set of support libraries that are not includes as
part of the *superbuild*:

- Qt5 with the following components:
  - Multimedia
  - UiTools
  - XMLPatterns
  - SVG
  - WebEngine
  - Script
  - X11Extras
  - Private
- libXt
  
### Debian 10 Stable (Buster)

Install the development tools and the support libraries:
```
sudo apt install git subversion build-essential cmake cmake-curses-gui cmake-qt-gui qt5-default qt5multimedia-dev qttools5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qtwebengine5-dev qtscript5-dev  lqtbase5-private-dev libqt5x11extras5-dev libxt-dev libssl-dev
```

### Debian Testing (Bullseye) and Debian 9

*This option is not suggested since it does not work with standard packages.  Debian 9 Qt 5.7 packages will not work with current Slicer 4.11.  Checked 2020-08-19.  May be possible to build from source or install other packages.  In addition, for Debian 9 you also need to build cmake from source as [described here](https://cmake.org/install/) or otherwise get a newer version than is supplied by the distribution.*

Install the development tools and the support libraries:
```
sudo apt install git subversion build-essential cmake cmake-curses-gui cmake-qt-gui qt5-default qtmultimedia5-dev qttools5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qtwebengine5-dev qtscript5-dev qtbase5-private-dev libqt5x11extras5-dev libxt-dev libssl-dev
```

### Ubuntu 20.04 (Focal Fossa)

Install the development tools and the support libraries:
```
sudo apt install git subversion build-essential cmake cmake-curses-gui cmake-qt-gui qt5-default qtmultimedia5-dev qttools5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qtwebengine5-dev qtscript5-dev qtbase5-private-dev libqt5x11extras5-dev libxt-dev 
```
### ArchLinux

Install the development tools and the support libraries:

```
sudo pacman -S git make patch subversion gcc cmake qt5-base qt5-multimedia qt5-tools qt5-xmlpatterns qt5-svg qt5-webengine qt5-script qt5-x11extras libxt
```

## Checkout Slicer source files

The recommended way to obtain the source code of SLicer is cloning the repository using `git`:

```
git clone git://github.com/Slicer/Slicer.git
```

This will create a `Slicer` directory contaning the source code of Slicer.
Hereafter we will call this directory the `source directory`.

<div class="admonition warning">
 
  <p class="admonition-title">Warning</p>

  <p>It is highly recommended to <b>avoid</b> the use of the <b>space</b>
  character in the name of the <code class="docutils literal notranslate"><span
  class="pre">source directory</span></code> or any of its parent
  directories.</p> </div>


After obtaining the source code, we need to set up the development environment:

```
cd Slicer
./Utilities/SetupForDevelopment.sh
```

[comment]: <> (TODO: Link to the readthedocs equivalent of https://www.slicer.org/wiki/Documentation/Nightly/Developers/DevelopmentWithGit)

## Configure and generate the Slicer build project files
 
Slicer is highly configurable and multi-platform. To support this,
Slicer needs a configuration of the build parameters before the build process
takes place. In this configuration stage, it is possible to adjust variables
that change the nature and behaviour of its components. For instance, the type
of build (Debug or Release mode), whether to use system-installed libraries,
let the build process fetch and compile own libraries, or enable/disable some of
the software components and functionalities of Slicer.

To obtain a default configuration of the Slicer build project use `cmake`:

```
mkdir Slicer-SuperBuild-Debug
cd Slicer-SuperBuild-Debug
cmake ../Slicer
```
It is possible to change variables with `cmake`. In the following example we
change the built type (Debug as default) to Release:

```
cmake -DCMAKE_BUILD_TYPE:STRING=Release ../Slicer
```

<div class="admonition tip">
 
  <p class="admonition-title">Tip</p>

  <p>Instead of <code class="docutils literal notranslate"><span
  class="pre">cmake</span></code>, one can use <code class="docutils literal
  notranslate"><span class="pre">ccmake</span></code>, which provides a
  text-based interface or <code class="docutils literal notranslate"><span
  class="pre">cmake-gui</span></code>, which provides a graphical user interface.
  These applications will also provide a list of variables that can be changed.
  </p>

</div>

## Build Slicer

Once the Slicer build project files have been generated, the Slicer project can
be build:

```
cd Slicer-SuperBuild-Debug
make
```

<div class="admonition tip">
 
  <p class="admonition-title">Tip</p> 
  
  <p>Building Slicer will generally take long
  time, particularly on the first build or upon code/configuration changes. To
  help speeding up the process one can use <code class="docutils literal
  notranslate"><span class="pre">make -j&lt;N&gt;</span></code>, where <code
  class="docutils literal notranslate"><span class="pre">N</span></code> is the
  number of parallel builds. As a rule of thumb, many uses the <em>number of CPU
  threads + 1</em> as the number of parallel builds.</p>
  

</div>


<div class="admonition warning">
 
  <p class="admonition-title">Warning</p>

  <p>Increasing the number of parallel builds generally
  increases the memory required for the build process. In the event that the
  required memory exceeds the available memory, the process will either fail or
  start using swap memory, which will make in practice the system to freeze.</p>

</div>

<div class="admonition tip">
 
  <p class="admonition-title">Tip</p> 
  
  <p>Using parallel builds makes finding compilation errors difficult due to the
  fact that all parallel build processes use the same screen otput, as opposed
  to sequential builds, where the compilation process will stop at the error. A
  common technique to have parallel builds and easily find errors is launch a
  parallel build followed by a sequential build. For the parallel build, it is adviced to run <code
  class="docutils literal notranslate"><span class="pre">make
  -j&lt;N&gt; -k</span></code> to have the parallel build keep going as far as
  possible before doing the sequential build with<code class="docutils literal
  notranslate"><span class="pre">make</span></code></p>

</div>


## Run Slicer

After the building process has successfully completed, the executable file to
run slicer will be located in `./Slicer-build/Slicer`

## Test Slicer

After building, run the tests in the  `Slicer-SuperBuild/Slicer-build` directory.

Start a terminal and type the following (you can replace 4 by the number of processor cores in the computer):
```
cd Slicer-SuperBuild-Debug/Slicer-build
ctest -j4
```

## Package Slicer

Start a terminal and type the following:
```
cd Slicer-SuperBuild-Debug/Slicer-build
make package
```

## Common errors

See list of issues common to all operating systems on [Common errors](common_errors) page.
