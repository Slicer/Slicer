# Mac OS

## Prerequisites

The prerequisites listed below are required to be able to configure/build/package/test Slicer.

- XCode command line tools must be installed:
```
xcode-select --install
```
- El Capitan is what most developers use.
- CMake 3.12.2 is recommended. Check the minimum required CMake version [here](https://github.com/Slicer/Slicer/blob/master/CMakeLists.txt#L1)
- Large File Storage for git is required. (`brew install git-lvs`)
- Qt 5: **tested and recommended**.
  - For building Slicer: download and execute [qt-unified-mac-x64-online.dmg](https://download.qt.io/official_releases/online_installers/qt-unified-mac-x64-online.dmg), install Qt 5.10, make sure to select `qtscript` and `qtwebengine` components.
  - For packaging and redistributing Slicer: build Qt using [qt-easy-build](https://github.com/jcfr/qt-easy-build#readme)
- Setting `CMAKE_OSX_DEPLOYMENT_TARGET` CMake variable specifies the minimum macOS version a generated installer may target.  So it should be equal to or less than the version of SDK you are building on. Note that the SDK version is set using `CMAKE_OSX_SYSROOT` CMake variable automatically initialized during CMake configuration.

### Mac OSX 10.9.4 (Mavericks)

- Make sure to install this update: http://support.apple.com/kb/DL1754

- Use CMake 3.12.2 - it is known to be working and is supported (if you want to use CMake already installed on your system, 2.8.12.2 is known to work on Mac OS X 10.9.5)

### Mac OSX >= 10.5 (Leopard)

- CMake >= 2.8.9
  - For Mac Os X >= 10.8 ([Mountain Lion](http://en.wikipedia.org/wiki/OS_X_Mountain_Lion)) and/or recent XCode >= 4.5.X - CMake >= 2.8.11 is required. See http://www.cmake.org/files/v2.8/cmake-2.8.11-Darwin64-universal.tar.gz

```
$ curl -O http://www.cmake.org/files/v2.8/cmake-2.8.11-Darwin64-universal.tar.gz
$ tar -xzvf cmake-2.8.11-Darwin64-universal.tar.gz --strip-components=1
```

```
$ CMake\ 2.8-11.app/Contents/bin/cmake --version
 cmake version 2.8.11
```

- Git >= 1.7.10
- Svn >= 1.7
- XCode (for the SDK libs)
  - After installing XCode, install XCode command line developer tools:
```
xcode-select --install
```
- XQuartz - For Mac Os X >= 10.8 ([Mountain Lion](http://en.wikipedia.org/wiki/OS_X_Mountain_Lion)) install XQuartz (http://xquartz.macosforge.org) to get X11 (no longer a default in OS X).
- Qt 4 >= 4.8.5. We recommend you install the following two packages:
  - Download and install [http://download.qt-project.org/official_releases/qt/4.8/4.8.6/qt-opensource-mac-4.8.6-1.dmg qt-opensource-mac-4.8.6-1.dmg]
  - Download and install [http://download.qt-project.org/official_releases/qt/4.8/4.8.6/qt-opensource-mac-4.8.6-1-debug-libs.dmg qt-opensource-mac-4.8.6-1-debug-libs.dmg]

### Mac OSX 10.11 (El Capitan)

XCode up to version 7 is known to work for Slicer compilation. XCode 8 breaks things on several levels for now.
Remember to install XCode command line tools (see above) if a reinstall for XCode is needed. 

The standard Qt4 installers fail on this version and the 4.8.6 source code won't build.  But [as described on the slicer-devel mailing list](http://slicer-devel.65872.n3.nabble.com/incompatible-qt-4-8-6-with-OS-X-El-Capitan-td4035551.html) it is possible to install the [homebrew version of qt4 which patches it to work on El Capitan](https://github.com/Homebrew/formula-patches/blob/master/qt/el-capitan.patch) (see below).

- Install the `OS`, `Xcode`, and `XQuartz` (see MacOSX 10.10 above)
- Install `Qt4` by running the following two commands:
```
brew install qt4
xcode-select --install
```
- TCL does not build correctly on El Capitan as of 2015-12-03, so when building Slicer turn `Slicer_USE_PYTHONQT_WITH_TCL` off.

### Mac OSX 10.12 (Sierra)

Similar to 10.11 (El Capitan), there are new issues with Qt4 (caused by Phonon).
The GitHub user Cartr [offered a patch to the brew team](https://github.com/Homebrew/homebrew-core/pull/5216), and even though it was not integrated (the homebrew team decided to stop patching their recipe to encourage people to use Qt5), he [turned his formula into a tap](https://github.com/cartr/homebrew-qt4) that can be installed (see below).

Follow instructions for 10.11 *(Installing Xcode, XQuartz, run without TCL)* but install **Qt4** like shown below instead:
```
brew install cartr/qt4/qt
xcode-select --install
```

Confirmed with Xcode: 

- Version 8.1 (8B62) and cmake version 3.4.20151021-g8fbc8e
- Version 8.3.3 and cmake 3.5.2

### Mac OSX 10.14 (Mojave)

(Associated discussion topic is https://discourse.slicer.org/t/building-on-mac-10-14-mojave/4554/21)

- Install Qt 5.11.2 using [Qt Online Installer for macOS](http://download.qt.io/official_releases/online_installers/qt-unified-mac-x64-online.dmg)
- Install XCode:
```
xcode-select --install
```
- Build qt from homebrew
```
brew install qt
ccmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 -DQt5_DIR=/usr/local//Cellar/qt/5.13.2/lib/cmake/Qt5 ~/slicer/latest/Slicer
```
- Explicitly set the SDK when running make
```
SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk make -j20
```

Note if you have build errors in dcmtk related to iconv symbols, you may need to uninstall the icu4c and dcmtk homebrew packages during the build process.  See [here](https://github.com/QIICR/dcmqi/issues/395) and [here](https://github.com/Slicer/Slicer/commit/6523a62d776e64f970c554978a3c3a8f26022db5).

## Checkout Slicer source files

Notes:
- While it is not enforced, we strongly recommend you to *avoid* the use of *spaces* for both the `source directory` and the `build directory`.
- Due to maximum path length limitations during build the build process, source and build folders must be located in a folder with very short total path length. This is expecially critical on Windows and MacOS. For example, `/sq5` has been confirmed to work on MacOS.

Check out the code using `git`:
- Clone the github repository</p>
```cd MyProjects
git clone git://github.com/Slicer/Slicer.git
```
The `Slicer``` directory is automatically created after cloning Slicer.
- Setup the development environment:
```cd Slicer
./Utilities/SetupForDevelopment.sh
```

## Configure and generate Slicer solution files

- Configure using the following commands. By default `CMAKE_BUILD_TYPE` is set to `Debug` (replace `/path/to/QtSDK` with the real path on your machine where QtSDK is located):
```
mkdir Slicer-SuperBuild-Debug
cd Slicer-SuperBuild-Debug
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DQt5_DIR:PATH=/path/to/Qt5.15.0/5.15.0/gcc_64/lib/cmake/Qt5 ../Slicer
```
- If `using Qt from the system`, do not forget to add the following CMake variable to your configuration command line: `-DSlicer_USE_SYSTEM_QT:BOOL=ON`
- Remarks:
  - Instead of `cmake`, you can use `ccmake` or `cmake-gui` to visually inspect and edit configure options.
  - Using top-level directory name like `Slicer-SuperBuild-Release` or `Slicer-SuperBuild-Debug` is recommended.
  - [Step-by-step debug instuctions](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Debug_Instructions)
  - Additional configuration options to customize the application are described [here](overview.md#Customized_builds).

- On Mac OS X 10.9 (Mavericks) / 10.10 (Yosemite) / 10.11 (El Capitan), also set the following variables (see [ discussion](http://massmail.spl.harvard.edu/public-archives/slicer-devel/2014/thread.html#16440)):
  - `Slicer_USE_PYTHONQT_WITH_TCL` -> `OFF`
  - `CMAKE_OSX_DEPLOYMENT_TARGET` -> `10.9` or `10.10` or `10.11`

### General information

Two projects are generated by either `cmake`, `ccmake` or `cmake-gui`. One of them is in the top-level bin directory `Slicer-SuperBuild` and the other one is in the subdirectory `Slicer-build`:
- `Slicer-SuperBuild` manages all the external dependencies of Slicer (VTK, ITK, Python, ...). To build Slicer for the first time, run make (or build the solution file in Visual Studio) in `Slicer-SuperBuild`, which will update and build the external libraries and if successful will then build the subproject Slicer-build.
- `Slicer-SuperBuild/Slicer-build` is the "traditional" build directory of Slicer.  After local changes in Slicer (or after an svn update on the source directory of Slicer), only running make (or building the solution file in Visual Studio) in `Slicer-SuperBuild/Slicer-build` is necessary (the external libraries are considered built and up to date).

*Warning:* An significant amount of disk space is required to compile Slicer in Debug mode (>10GB)

*Warning:* Some firewalls will block the git protocol. See more information and solution [here](../overview.html#firewall-is-blocking-git-protocol).

## Build Slicer

After configuration, start the build process in the `Slicer-SuperBuild` directory

- Start a terminal and type the following (you can replace 4 by the number of processor cores in the computer):
```
cd ~/Projects/Slicer-SuperBuild
make -j4
```

In case of file download hash mismatch error, you need to acquire the latest wget, and build cmake with OpenSSL turned on. For more information, see [here](http://slicer-devel.65872.n3.nabble.com/How-to-solve-wget-error-certificate-common-name-c-ssl-fastly-net-doesn-t-match-requested-host-name-p-td4031491.html) and [here](http://slicer-devel.65872.n3.nabble.com/Re-Hash-Error-td4031386.html).

When using the -j option, the build will continue past the source of the first error. If the build fails and you don't see what failed, rebuild without the -j option. Or, to speed up this process build first with the -j and -k options and then run plain make. The -k option will make the build keep going so that any code that can be compiled independent of the error will be completed and the second make will reach the error condition more efficiently.

## Run Slicer

Start a terminal and type the following:
```
Slicer-SuperBuild/Slicer-build/Slicer
```

## Test Slicer

After building, run the tests in the  `Slicer-SuperBuild/Slicer-build` directory.

Start a terminal and type the following (you can replace 4 by the number of processor cores in the computer):
```
cd ~/Projects/Slicer-SuperBuild/Slicer-build
ctest -j4
```

## Package Slicer

Start a terminal and type the following:
```
cd ~/Projects/Slicer-SuperBuild
cd Slicer-build
make package
```

## Common errors

See list of issues common to all operating systems on [Common errors](common_errors) page.

### CMake complains during configuration

CMake may not directly show what's wrong; try to look for log files of the form BUILD/CMakeFiles/*.log (where BUILD is your build directory) to glean further information.

### error: ‘class QList<QString>’ has no member named ‘reserve’

```
 /nfs/Users/blowekamp/QtSDK/Desktop/Qt/474/gcc/include/QtCore/qdatastream.h: In function ‘QDataStream& operator>>(QDataStream&, QList<T>&) [with T = QString]’:
 /nfs/Users/blowekamp/QtSDK/Desktop/Qt/474/gcc/include/QtCore/qstringlist.h:247:   instantiated from here
 /nfs/Users/blowekamp/QtSDK/Desktop/Qt/474/gcc/include/QtCore/qdatastream.h:246: error: ‘class QList<QString>’ has no member named ‘reserve’
```
You have multiple Qt versions installed on your machine. Try removing the Qt version installed on the system.

### libarchive.so: undefined reference to `SHA256_Update'

```
 Linking CXX executable ../../../../../bin/MRMLLogicCxxTests
 /home/benjaminlong/work/slicer/Slicer-SuperBuild-Debug/LibArchive-install/lib/libarchive.so: undefined reference to `SHA256_Update'
 /home/benjaminlong/work/slicer/Slicer-SuperBuild-Debug/LibArchive-install/lib/libarchive.so: undefined reference to `SHA256_Final'
 /home/benjaminlong/work/slicer/Slicer-SuperBuild-Debug/LibArchive-install/lib/libarchive.so: undefined reference to `SHA256_Init'
 /home/benjaminlong/work/slicer/Slicer-SuperBuild-Debug/LibArchive-install/lib/libarchive.so: undefined reference to `MD5_Init'
```

Solution:
```
 cd Slicer-SuperBuild
 rm -rf LibArchive-*
 make -j4
```

Details:
- http://na-mic.org/Mantis/view.php?id=1616
- http://viewvc.slicer.org/viewvc.cgi/Slicer4?view=revision&revision=18923
- http://viewvc.slicer.org/viewvc.cgi/Slicer4?view=revision&revision=18969

### ld: framework not found QtWebKit

```
Linking CXX shared library libPythonQt.dylib
ld: framework not found QtWebKit
clang: error: linker command failed with exit code 1 (use -v to see invocation)
make[8]: *** [libPythonQt.dylib] Error 1
```

See Slicer issue [2860](http://na-mic.org/Mantis/view.php?id=2860).

Workaround:
- See http://public.kitware.com/Bug/view.php?id=13765#c31824

Solution:
- Use a recent CMake. It includes patch [cc676c3a](http://cmake.org/gitweb?p=cmake.git;a=commitdiff;h=cc676c3a). Note also that the coming version of CMake 2.8.11 will work as expected.

### On MacOSX 10.8, CMake hangs forever

Issue: http://www.na-mic.org/Bug/view.php?id=2957

Solution: Use CMake build >= 2.8.10.20130220-g53b279. See http://www.cmake.org/files/dev/cmake-2.8.10.20130220-g53b279-Darwin-universal.tar.gz

Details:

CMake folks (Thanks Brad King :)) fixed an issue that was preventing the most recent nightly from being used to successfully compile Slicer. The fix has been tested and is known to work. Tomorrow nightly build of CMake and by extension the coming release of CMake 2.8.11 will work.

If you are curious about the details, check the commit message: http://cmake.org/gitweb?p=cmake.git;a=commitdiff;h=1df09e57

The associated CMake test have also been updated: http://cmake.org/gitweb?p=cmake.git;a=commitdiff;h=bff3d9ce

### On MacOSX 10.8/10.9 with XQuartz, 'X11/Xlib.h' file not found

Issue: http://www.na-mic.org/Bug/view.php?id=3405

Workaround: Build with -DSlicer_USE_PYTHONQT_WITH_TCL:BOOL=OFF

Tcl is required only for EMSegment module.

Details: See http://www.na-mic.org/Bug/view.php?id=3405

### 'QSslSocket' : is not a class or namespace name

This error message occurs if Slicer is configured to use SSL but Qt is built without SSL support.

Either set Slicer_USE_PYTHONQT_WITH_OPENSSL to OFF when configuring Slicer build in CMake, or build Qt with SSL support.

### error: Missing Qt module QTWEBKIT

```
 CMake Error at CMake/SlicerBlockFindQtAndCheckVersion.cmake:43 (message):
  error: Missing Qt module QTWEBKIT
 Call Stack (most recent call first):
  CMake/SlicerBlockFindQtAndCheckVersion.cmake:88 (__SlicerBlockFindQtAndCheckVersion_find_qt)
  CMakeLists.txt:607 (include)
```

Solution:
```
sudo apt-get -y install libqtwebkit-dev
```

Details: http://slicer-devel.65872.n3.nabble.com/Re-slicer-devel-Digest-Vol-143-Issue-39-td4037122.html

### error when starting Slicer: NameError: name 'getSlicerRCFileName' is not defined

```
./Slicer
Traceback (most recent call last):
  File "<string>", line 7, in <module>
  File "/home/fbudin/Devel/Slicer-SuperBuild-Release/Slicer-build/bin/Python/slicer/slicerqt.py", line 6, in <module>
    import vtk
  File "/home/fbudin/Devel/Slicer-SuperBuild-Release/VTKv7-build/Wrapping/Python/vtk/__init__.py", line 41, in <module>
    from .vtkCommonKit import *
  File "/home/fbudin/Devel/Slicer-SuperBuild-Release/VTKv7-build/Wrapping/Python/vtk/vtkCommonKit.py", line 9, in <module>
    from vtkCommonKitPython import *
ImportError: /home/fbudin/Devel/Slicer-SuperBuild-Release/VTKv7-build/lib/./libvtkCommonKitPython27D-7.1.so.1: undefined symbol: PyUnicodeUCS2_DecodeUTF8
Traceback (most recent call last):
  File "<string>", line 1, in <module>
NameError: name 'getSlicerRCFileName' is not defined
Number of registered modules: 138
error: [/home/fbudin/Devel/Slicer-SuperBuild-Release/Slicer-build/bin/./SlicerApp-real] exit abnormally - Report the problem.
```

Solution and details [here](http://na-mic.org/Mantis/view.php?id=3574)

### macOS: error while configuring PCRE: "cannot run C compiled program"

If the XCode command line tools are not properly set up on OS X, PCRE could fail to build in the Superbuild process with the errors like below:
```
configure: error: in `/Users/fedorov/local/Slicer4-Debug/PCRE-build':
configure: error: cannot run C compiled programs.
```

To install XCode command line tools, use the following command from the terminal:
```
xcode-select --install
```

### macOS: dyld: malformed mach-o: load commands size (...) > 32768

Path of source or build folder is too long. For example building Slicer in */User/somebody/projects/something/dev/slicer/slicer-qt5-rel* may fail with malformed mach-o error, while it succeeds in */sq5* folder. To resolve this error, move source and binary files into a folder with shorter full path and restart the build from scratch (the build tree is not relocatable).
