# C++ debugging on GNU/Linux systems

## GDB debug by attaching to running process (recommended)

1. Starting with Ubuntu 10.10, ptracing of non-child processes by non-root users as been disabled -ie. only a process which is a parent of another process can ptrace it for normal users. More details [here](https://askubuntu.com/questions/41629/after-upgrade-gdb-wont-attach-to-process).

    - You can temporarily disable this restriction by:
      ```bash
      $ echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
      ```
    - To permanently allow it to edit `/etc/sysctl.d/10-ptrace.conf` and change the line:
      ```txt
      kernel.yama.ptrace_scope = 1
      ```
      to read:
      ```txt
      kernel.yama.ptrace_scope = 0
      ```

2. Running Slicer with the following command line argument will allow you to easily obtain the associated PID:

    ```bash
    $ ./Slicer --attach-process
    ```

    This will bring up a window with the `PID` before loading any modules, which is also helpful for debugging the loading process.

3. Then, you can attach the process to `gdb` using the following command:

    ```bash
    $ gdb --pid $PIDABOVE
    ```

4. Finally type the following gdb command

    ```txt
    (gdb) continue
    ```

    If not using the `--attach-process`, the `PID` could be obtain using the following command:

    ```bash
    $ ps -Afww | grep SlicerApp-real
    ```

:::{tip}

How to print QString using GDB?

See <http://silmor.de/qtstuff.printqstring.php>.

:::

## GDB debug with launch arguments

The Slicer app launcher provides options to start other programs with the Slicer environment settings.

- `--launch <executable> [<parameters>]`: executes an arbitrary program. For example, `Slicer --launch /usr/bin/gnome-terminal` starts gnome-terminal (then run GDB directly on SlicerApp-real)
- `--gdb`: runs GDB then executes SlicerApp-real from within the debugger environment.

## GDB debug by following the forked process

```bash
gdb Slicer
```

gdb should warn you that there are no debug symbols for Slicer, which is true because Slicer is the launcher. Now we need to set gdb to follow the forked process SlicerApp-real and run the launcher:

```txt
(gdb) set follow-fork-mode child
(gdb) run
```
gdb will run Slicer and attach itself to the forked process SlicerApp-real

## GDB debug by using exec-wrapper

An alternative approach is to use a wrapper script to emulate the functionality of the app launcher. This will allow you to use gdb or a gdb-controlling program such as an IDE, in order to interactively debug directly from GDB without attaching.

The general idea of the wrapper is to set all of the appropriate environment variables as they would be set by the app launcher. From SlicerLauncherSettings:

- `[LibraryPath]` contents should be in `LD_LIBRARY_PATH`
- `[Paths]` contents should be in `PATH`
- `[EnvironmentVariables]` should each be set

Now, start gdb and do the following:

```txt
(gdb) set exec-wrapper ./WrapSlicer
(gdb) exec-file ./bin/SlicerQT-real
(gdb) run
```

Since VTK and ITK include many multithreaded filters, by default you will see lots of messages like the following from gdb during rendering and processing:

```txt
[New Thread 0x7fff8378f700 (LWP 20510)]
[Thread 0x7fff8b0aa700 (LWP 20506) exited]
```

These can be turned off with this command:

```txt
set print thread-events off
```

### Example wrapper script

To make a wrapper script, run

```bash
Slicer --launch `which gnome-terminal`
```

or

```bash
Slicer --gnome-terminal
```

and check the value of `LD_LIBRARY_PATH` (note, it doesn't seem to be set using xterm). Then check the other environment variables as listed in `[EnvironmentVariables]`.

See the examples below.

SlicerLaunchSettings.ini

```txt
[General]
launcherSplashImagePath=/cmn/git/Slicer4/Applications/SlicerQT/Resources/Images/SlicerSplashScreen.png
launcherSplashScreenHideDelayMs=3000
additionalLauncherHelpShortArgument=-h
additionalLauncherHelpLongArgument=--help
additionalLauncherNoSplashArguments=--no-splash,--help,--version,--home,--program-path,--no-main-window

[Application]
path=<APPLAUNCHER_DIR>/bin/./SlicerQT-real
arguments=

[ExtraApplicationToLaunch]

designer/shortArgument=
designer/help=Start Qt designer using Slicer plugins
designer/path=/usr/bin/designer-qt4
designer/arguments=

gnome-terminal/shortArgument=
gnome-terminal/help=Start gnome-terminal
gnome-terminal/path=/usr/bin/gnome-terminal
gnome-terminal/arguments=

xterm/shortArgument=
xterm/help=Start xterm
xterm/path=/usr/bin/xterm
xterm/arguments=

ddd/shortArgument=
ddd/help=Start ddd
ddd/path=/usr/bin/ddd
ddd/arguments=

gdb/shortArgument=
gdb/help=Start gdb
gdb/path=/usr/bin/gdb
gdb/arguments=


[LibraryPaths]
1\path=/cmn/git/Slicer4-sb/VTK-build/bin/.
2\path=/cmn/git/Slicer4-sb/CTK-build/CTK-build/bin/.
3\path=/usr/lib
4\path=/cmn/git/Slicer4-sb/ITKv3-build/bin/.
5\path=/cmn/git/Slicer4-sb/SlicerExecutionModel-build/ModuleDescriptionParser/bin/.
6\path=/cmn/git/Slicer4-sb/teem-build/bin/.
7\path=/cmn/git/Slicer4-sb/LibArchive-install/lib
8\path=<APPLAUNCHER_DIR>/bin/.
9\path=../lib/Slicer-4.0/qt-loadable-modules
10\path=<APPLAUNCHER_DIR>/lib/Slicer-4.0/cli-modules/.
11\path=<APPLAUNCHER_DIR>/lib/Slicer-4.0/qt-loadable-modules/.
12\path=/cmn/git/Slicer4-sb/tcl-build/lib
13\path=/cmn/git/Slicer4-sb/OpenIGTLink-build
14\path=/cmn/git/Slicer4-sb/OpenIGTLink-build/bin/.
15\path=/cmn/git/Slicer4-sb/CTK-build/PythonQt-build/.
16\path=/cmn/git/Slicer4-sb/python-build/lib
17\path=/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages/numpy/core
18\path=/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages/numpy/lib
size=18

[Paths]
1\path=<APPLAUNCHER_DIR>/bin/.
2\path=/cmn/git/Slicer4-sb/teem-build/bin/.
3\path=/usr/bin
4\path=<APPLAUNCHER_DIR>/lib/Slicer-4.0/cli-modules/.
5\path=/cmn/git/Slicer4-sb/tcl-build/bin
size=5

[EnvironmentVariables]
QT_PLUGIN_PATH=<APPLAUNCHER_DIR>/bin<PATHSEP>/cmn/git/Slicer4-sb/CTK-build/CTK-build/bin<PATHSEP>/usr/lib/qt4/plugins
SLICER_HOME=/cmn/git/Slicer4-sb/Slicer-build
PYTHONHOME=/cmn/git/Slicer4-sb/python-build
PYTHONPATH=<APPLAUNCHER_DIR>/bin<PATHSEP><APPLAUNCHER_DIR>/bin/Python<PATHSEP>/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages<PATHSEP><APPLAUNCHER_DIR>/lib/Slicer-4.0/qt-loadable-modules/.<PATHSEP><APPLAUNCHER_DIR>/lib/Slicer-4.0/qt-loadable-modules/Python
TCL_LIBRARY=/cmn/git/Slicer4-sb/tcl-build/lib/tcl8.4
TK_LIBRARY=/cmn/git/Slicer4-sb/tcl-build/lib/tk8.4
TCLLIBPATH=/cmn/git/Slicer4-sb/tcl-build/lib/itcl3.2 /cmn/git/Slicer4-sb/tcl-build/lib/itk3.2
```

It's easier to just check `LD_LIBRARY_PATH` using `--launch`. If this is somehow not available, the above settings translate directly to:

```
#!/bin/bash
BASE_DIR=/cmn/git/Slicer4-sb/
APPLAUNCHER_DIR=$BASE_DIR/Slicer-build

LD_PATHS="
/cmn/git/Slicer4-sb/VTK-build/bin/.
/cmn/git/Slicer4-sb/CTK-build/CTK-build/bin/.
/usr/lib
/cmn/git/Slicer4-sb/ITKv3-build/bin/.
/cmn/git/Slicer4-sb/SlicerExecutionModel-build/ModuleDescriptionParser/bin/.
/cmn/git/Slicer4-sb/teem-build/bin/.
/cmn/git/Slicer4-sb/LibArchive-install/lib
$APPLAUNCHER_DIR/bin/.
../lib/Slicer-4.0/qt-loadable-modules
$APPLAUNCHER_DIR/lib/Slicer-4.0/cli-modules/.
$APPLAUNCHER_DIR/lib/Slicer-4.0/qt-loadable-modules/.
/cmn/git/Slicer4-sb/tcl-build/lib
/cmn/git/Slicer4-sb/OpenIGTLink-build
/cmn/git/Slicer4-sb/OpenIGTLink-build/bin/.
/cmn/git/Slicer4-sb/CTK-build/PythonQt-build/.
/cmn/git/Slicer4-sb/python-build/lib
/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages/numpy/core
/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages/numpy/lib
"
for STR in $LD_PATHS; do LD_LIBRARY_PATH="${STR}:${LD_LIBRARY_PATH}"; done

QT_PLUGIN_PATH=$APPLAUNCHER_DIR/bin:/cmn/git/Slicer4-sb/CTK-build/CTK-build/bin:/usr/lib/qt4/plugins
SLICER_HOME=/cmn/git/Slicer4-sb/Slicer-build
PYTHONHOME=/cmn/git/Slicer4-sb/python-build
PYTHONPATH=$APPLAUNCHER_DIR:/bin:$APPLAUNCHER_DIR:/bin/Python:/cmn/git/Slicer4-sb/python-build/lib/python2.6/site-packages:$APPLAUNCHER_DIR/lib/Slicer-4.0/qt-loadable-modules/.:$APPLAUNCHER_DIR/lib/Slicer-4.0/qt-loadable-modules/Python
TCL_LIBRARY=/cmn/git/Slicer4-sb/tcl-build/lib/tcl8.4
TK_LIBRARY=/cmn/git/Slicer4-sb/tcl-build/lib/tk8.4
TCLLIBPATH=/cmn/git/Slicer4-sb/tcl-build/lib/itcl3.2:/cmn/git/Slicer4-sb/tcl-build/lib/itk3.2

export QTPLUGIN_PATH=$QT_PLUGIN_PATH
export SLICER_HOME=$SLICER_HOME
export PYTHONHOME=$PYTHONHOME
export PYTHONPATH=$PYTHONPATH
export TCL_LIBRARY=$TCL_LIBRARY
export TK_LIBRARY=$TK_LIBRARY
export TCLLIBPATH=$TCLLIBPATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH

exec "$@"
```

## Analyze a segmentation fault

In the build tree:

```bash
$ ulimit -c unlimited
$ ./Slicer
... make it crash
$ ./Slicer --gdb ./bin/SlicerApp-real
(gdb) core core
(gdb) backtrace
...
```

For an installed Slicer:

```bash
$ ulimit -c unlimited
$ ./Slicer
... make it crash
$ ./Slicer --launch bash
$ gdb ./bin/SlicerApp-real
(gdb) core core
(gdb) backtrace
...
```

````{note}

GDB requires Python. However, Python that is linked into GDB is not the same as Slicer's Python, which may cause issues. If GDB does not start because `_sysconfigdata__linux_x86_64-linux-gnu.py` file is missing then Slicer's sysconfigdata file must be copied to the expected filename. For example:

```bash
cd ~/Slicer-4.13.0-2021-09-10-linux-amd64/lib/Python/lib/python3.6
cp _sysconfigdata_m_linux2_.py _sysconfigdata__linux_x86_64-linux-gnu.py
```

````

### With systemd

In linux distros with systemd, coredumps are managed by the systemd daemon.
And stored, in a compressed format (.lz4), in

```txt
/var/lib/systemd/coredump/core.SlicerApp-real.xxxx.lz4
```

It can happen that even with ulimit -c unlimited, the coredump files are still truncated.
You can check latest coredumps, and the corresponding PID with:

```bash
coredumpctl list
Thu 2018-05-24 16:37:46 EDT   22544  1000  1000  11 missing   /usr/lib/firefox/firefox
Fri 2018-05-25 15:50:52 EDT   14721  1000  1000   6 truncated /path/Slicer-build/bin/SlicerApp-real
Mon 2018-05-28 11:35:43 EDT   17249  1000  1000   6 present   /path/Slicer-build/bin/SlicerApp-real
```

You can modify systemd coredump to increase the default max file size in `/etc/systemd/coredump.conf`:

```txt
[Coredump]
#Storage=external
#Compress=yes
ProcessSizeMax=8G
ExternalSizeMax=8G
JournalSizeMax=6G
#MaxUse=
#KeepFree=
```

After that change, make Slicer crash again, and the core file will be present, instead of truncated.

You can launch gdb with a coredump file with the command:

```bash
coredumpctl gdb $PID
```

If no $PID is provided, the latest coredump is used. PID can be retrieved using `coredumpctl list`.

To decompress the coredump file to use with other IDE or ddd, change the coredump.conf Compress option, or use:

```bash
coredumpctl dump $PID > /tmp/corefile
```

## Debugging using cross-platform IDEs

- [Debugging using CodeLite](codelitecpp.md)
- [Debugging using Qt Creator](qtcreatorcpp.md)
- [Debugging using Visual Studio Code](vscodecpp.md)
