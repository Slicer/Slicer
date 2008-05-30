#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# genlib.tcl
#
# generate the Lib directory with the needed components for slicer
# to build
#
# Steps:
# - pull code from anonymous cvs
# - configure (or cmake) with needed options
# - build for this platform
#
# Packages: cmake, tcl, itcl, ITK, VTK, blt, teem, NA-MIC sandbox
#
# Usage:
#   genlib [options] [target]
#
# run genlib from the slicer directory where you want the packages to be build
# E.g. if you run /home/pieper/slicer2/Scripts/genlib.tcl on a redhat7.3
# machine it will create /home/pieper/slicer2/Lib/redhat7.3
#
# - sp - 2004-06-20
# - modified for svn mirrors sp - 2007-05-22
#

# for subversion repositories (Sandbox)
if {[info exists ::env(SVN)]} {
    set ::SVN $::env(SVN)
} else {
    set ::SVN svn
}


################################################################################
#
# simple command line argument parsing
#

proc Usage { {msg ""} } {
    global SLICER

    set msg "$msg\nusage: genlib \[options\] \[target\]"
    set msg "$msg\n  \[target\] is the Slicer3_LIB directory"
    set msg "$msg\n             and is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   --help : prints this message and exits"
    set msg "$msg\n   --clean : delete the target first"
    set msg "$msg\n   --update : do a cvs update even if there's an existing build"
    set msg "$msg\n   --release : compile with optimization flags"
    puts stderr $msg
}

set GENLIB(clean) "false"
set GENLIB(update) "false"
set isRelease 0
set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--clean" -
        "-f" {
            set GENLIB(clean) "true"
        }
        "--update" -
        "-u" {
            set GENLIB(update) "true"
        }
        "--release" {
            set isRelease 1
        }
        "--help" -
        "-h" {
            Usage
            exit 1
        }
        "-*" {
            Usage "unknown option $a\n"
            exit 1
        }
        default {
            lappend strippedargs $a
        }
    }
}
set argv $strippedargs
set argc [llength $argv]

set ::Slicer3_LIB ""
if {$argc > 1 } {
  Usage
  exit 1
} else {
  set ::Slicer3_LIB $argv
}


################################################################################
#
# Utilities:

proc runcmd {args} {
    global isWindows
    puts "running: $args"

    # print the results line by line to provide feedback during long builds
    # interleaves the results of stdout and stderr, except on Windows
    if { $isWindows } {
        # Windows does not provide native support for cat
        set fp [open "| $args" "r"]
    } else {
        set fp [open "| $args |& cat" "r"]
    }
    while { ![eof $fp] } {
        gets $fp line
        puts $line
    }
    set ret [catch "close $fp" res]
    if { $ret } {
        puts stderr $res
        if { $isWindows } {
            # Does not work on Windows
        } else {
            error $ret
        }
    }
}


################################################################################
# First, set up the directory
# - determine the location
# - determine the build
#

# hack to work around lack of normalize option in older tcl
# set Slicer3_HOME [file dirname [file dirname [file normalize [info script]]]]
set cwd [pwd]
cd [file dirname [info script]]
cd ..
set Slicer3_HOME [pwd]
cd $cwd

if { $::Slicer3_LIB == "" } {
  set ::Slicer3_LIB [file dirname $::Slicer3_HOME]/Slicer3-lib
  puts "Slicer3_LIB is $::Slicer3_LIB"
}

#######
#
# Note: the local vars file, slicer2/slicer_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't
#   be overwritten when this file is updated
#
set localvarsfile $Slicer3_HOME/slicer_variables2.tcl
catch {set localvarsfile [file normalize $localvarsfile]}
if { [file exists $localvarsfile] } {
    puts "Sourcing $localvarsfile"
    source $localvarsfile
} else {
    puts "stderr: $localvarsfile not found - use this file to set up your build"
    exit 1
}

#initialize platform variables
switch $tcl_platform(os) {
    "SunOS" {
        set isSolaris 1
        set isWindows 0
        set isDarwin 0
        set isLinux 0
    }
    "Linux" {
        set isSolaris 0
        set isWindows 0
        set isDarwin 0
        set isLinux 1
    }
    "Darwin" {
        set isSolaris 0
        set isWindows 0
        set isDarwin 1
        set isLinux 0
    }
    default {
        set isSolaris 0
        set isWindows 1
        set isDarwin 0
        set isLinux 0
    }
}

set ::VTK_DEBUG_LEAKS "ON"
if ($isRelease) {
    set ::VTK_BUILD_TYPE "Release"
    set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
    if ($isWindows) {
        set ::VTK_BUILD_SUBDIR "Release"
    } else {
        set ::VTK_BUILD_SUBDIR ""
    }
    puts "Overriding slicer_variables.tcl; VTK_BUILD_TYPE is '$::env(VTK_BUILD_TYPE)', VTK_BUILD_SUBDIR is '$::VTK_BUILD_SUBDIR'"
    set ::VTK_DEBUG_LEAKS "OFF"

}

# tcl file delete is broken on Darwin, so use rm -rf instead
if { $GENLIB(clean) } {
    puts "Deleting slicer lib files..."
    if { $isDarwin } {
        runcmd rm -rf $::Slicer3_LIB
        if { [file exists $::Slicer3_LIB/tcl/isPatched] } {
            runcmd rm $::Slicer3_LIB/tcl/isPatched
        }

        if { [file exists $::Slicer3_LIB/tcl/isPatchedBLT] } {
            runcmd rm $::Slicer3_LIB/tcl/isPatchedBLT
        }
    } else {
        file delete -force $::Slicer3_LIB
    }
}

if { ![file exists $::Slicer3_LIB] } {
    file mkdir $::Slicer3_LIB
}


################################################################################
# Get and build CMake
#

# set in slicer_vars
if { ![file exists $::CMAKE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB
    file mkdir $::CMAKE_PATH

    if {$isWindows} {
      runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Binaries/Windows/CMake-build CMake-build
    } else {
        runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/CMake CMake

        cd $::CMAKE_PATH
        runcmd $::Slicer3_LIB/CMake/bootstrap
        eval runcmd $::MAKE
    }
}


################################################################################
# Get and build tcl, tk, itcl, widgets
#

# on windows, tcl won't build right, as can't configure
if { ![file exists $::TCL_TEST_FILE] || $::GENLIB(update) } {

    if {$isWindows} {
      runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build tcl-build
    }

    file mkdir $::Slicer3_LIB/tcl
    cd $::Slicer3_LIB/tcl

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/tcl tcl

    if {$isWindows} {
        # nothing to do for windows
    } else {
        cd $::Slicer3_LIB/tcl/tcl/unix
        if { $isDarwin } {
            runcmd ./configure --prefix=$::Slicer3_LIB/tcl-build --disable-corefoundation
        } else {
            runcmd ./configure --prefix=$::Slicer3_LIB/tcl-build
        }
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
    }
}

if { ![file exists $::TK_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB/tcl

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/tk tk

    if {$isWindows} {
        # don't need to do windows
    } else {
        cd $::Slicer3_LIB/tcl/tk/unix

        runcmd ./configure --with-tcl=$::Slicer3_LIB/tcl-build/lib --prefix=$::Slicer3_LIB/tcl-build --disable-corefoundation
        eval runcmd $::MAKE
        eval runcmd $::MAKE install

        file copy -force $::Slicer3_LIB/tcl/tk/generic/default.h $::Slicer3_LIB/tcl-build/include
        file copy -force $::Slicer3_LIB/tcl/tk/unix/tkUnixDefault.h $::Slicer3_LIB/tcl-build/include
    }
}

if { ![file exists $::ITCL_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB/tcl


    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/incrTcl incrTcl

    if {$isWindows} {
        # can't do windows
    } else {
        cd $::Slicer3_LIB/tcl/incrTcl
        if {$isDarwin} {
            exec cp ../incrTcl/itcl/configure ../incrTcl/itcl/configure.orig
            exec sed -e "s/\\*\\.c | \\*\\.o | \\*\\.obj) ;;/\\*\\.c | \\*\\.o | \\*\\.obj | \\*\\.dSYM) ;;/" ../incrTcl/itcl/configure.orig > ../incrTcl/itcl/configure
        }
        exec chmod +x ../incrTcl/configure
        runcmd ../incrTcl/configure --with-tcl=$::Slicer3_LIB/tcl-build/lib --with-tk=$::Slicer3_LIB/tcl-build/lib --prefix=$::Slicer3_LIB/tcl-build
        if { $isDarwin } {
            # need to run ranlib separately on lib for Darwin
            # file is created and ranlib is needed inside make all
            catch "eval runcmd $::MAKE all"
            runcmd ranlib ../incrTcl/itcl/libitclstub3.2.a
        }
        eval runcmd $::MAKE all
        eval runcmd $::SERIAL_MAKE install
    }
}

if { ![file exists $::IWIDGETS_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB/tcl

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/iwidgets iwidgets

    if {$isWindows} {
        # can't do windows
    } else {
        cd $::Slicer3_LIB/tcl/iwidgets
        runcmd ../iwidgets/configure --with-tcl=$::Slicer3_LIB/tcl-build/lib --with-tk=$::Slicer3_LIB/tcl-build/lib --with-itcl=$::Slicer3_LIB/tcl/incrTcl --prefix=$::Slicer3_LIB/tcl-build
        # make all doesn't do anything...
        # iwidgets won't compile in parallel (with -j flag)
        eval runcmd $::SERIAL_MAKE all
        eval runcmd $::SERIAL_MAKE install
    }
}


################################################################################
# Get and build blt
#

if { !$isDarwin  && (![file exists $::BLT_TEST_FILE] || $::GENLIB(update)) } {
    cd $::Slicer3_LIB/tcl

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/tcl/blt blt

    if { $isWindows } {
        # can't do Windows
    } elseif { $isDarwin } {

        cd $::Slicer3_LIB/tcl/blt
        runcmd ./configure --with-tcl=$::Slicer3_LIB/tcl/tcl/unix --with-tk=$::Slicer3_LIB/tcl-build --prefix=$::Slicer3_LIB/tcl-build --enable-shared --x-includes=/usr/X11R6/include --with-cflags=-fno-common

        eval runcmd $::SERIAL_MAKE
        eval runcmd $::SERIAL_MAKE install
    } else {
        cd $::Slicer3_LIB/tcl/blt
        runcmd ./configure --with-tcl=$::Slicer3_LIB/tcl/tcl/unix --with-tk=$::Slicer3_LIB/tcl-build --prefix=$::Slicer3_LIB/tcl-build
        eval runcmd $::SERIAL_MAKE
        eval runcmd $::SERIAL_MAKE install
    }
}

################################################################################
# Get and build python
#

if {  ![file exists $::PYTHON_TEST_FILE] || $::GENLIB(update) } {

    file mkdir $::Slicer3_LIB/python
    file mkdir $::Slicer3_LIB/python-build
    cd $::Slicer3_LIB/python

    runcmd $::SVN co $::PYTHON_TAG

    if { $isWindows } {
        # can't do Windows
    } else {
        cd $Slicer3_LIB/python/release25-maint

        set ::env(LDFLAGS) -L$Slicer3_LIB/tcl-build/lib
        set ::env(CPPFLAGS) -I$Slicer3_LIB/tcl-build/include

        runcmd ./configure --prefix=$Slicer3_LIB/python-build --with-tcl=$Slicer3_LIB/tcl-build --enable-shared
        eval runcmd $::MAKE
        puts [catch "eval runcmd $::SERIAL_MAKE install" res] ;# try twice - it probably fails first time...
        if { $isDarwin } {
            # Special Slicer hack to build and install the .dylib
            file mkdir $::Slicer3_LIB/python-build/lib/
            file delete -force $::Slicer3_LIB/python-build/lib/libpython2.5.dylib
            set fid [open environhack.c w]
            puts $fid "char **environ=0;"
            close $fid
            runcmd gcc -c -o environhack.o environhack.c
            runcmd libtool -o $::Slicer3_LIB/python-build/lib/libpython2.5.dylib -dynamic  \
                -all_load libpython2.5.a environhack.o -single_module \
                -install_name $::Slicer3_LIB/python-build/lib/libpython2.5.dylib \
                -compatibility_version 2.5 \
                -current_version 2.5 -lSystem -lSystemStubs

        }
    }
}

################################################################################
# Get and build numpy and scipy
#

if { ( ![file exists $::NUMPY_TEST_FILE] || $::GENLIB(update) ) } {

    set ::env(PYTHONHOME)        $::Slicer3_LIB/python-build
    cd $::Slicer3_LIB/python

    # do numpy

    runcmd $::SVN co $::NUMPY_TAG numpy

    if { $isWindows } {
        # can't do Windows
    } else {
        if { $isDarwin } {
            if { [info exists ::env(DYLD_LIBRARY_PATH)] } {
              set ::env(DYLD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib:$::env(DYLD_LIBRARY_PATH)
            } else {
              set ::env(DYLD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib
            }
        } else {
            if { [info exists ::env(LD_LIBRARY_PATH)] } {
                set ::env(LD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib:$::env(LD_LIBRARY_PATH)
            } else {
                set ::env(LD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib
            }
        }
        cd $::Slicer3_LIB/python/numpy
        set ::env(ATLAS) None
        set ::env(BLAS) None
        set ::env(LAPACK) None
        runcmd $::Slicer3_LIB/python-build/bin/python ./setup.py install

        # do scipy

        if { 0 } {
          # TODO: need to have a way to build the blas library...
          cd $::Slicer3_LIB/python
          runcmd $::SVN co $::SCIPY_TAG scipy
          
          cd $::Slicer3_LIB/python/scipy
          set ::env(ATLAS) None
          set ::env(BLAS) None
          set ::env(LAPACK) None
          # skip scipy for now until we find a BLAS to link against
          # runcmd $::Slicer3_LIB/python-build/bin/python ./setup.py install
        }
    }


}

################################################################################
# Get and build matplotlib
#
if { 0 && (![file exists $::MATPLOTLIB_TEST_FILE] || $::GENLIB(update)) } {

    set ::env(PYTHONHOME)        $Slicer3_LIB/python-build
    cd $Slicer3_LIB/python

    # do freetype

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/freetype2 freetype2

    if { $isWindows } {
        # can't do Windows
    } else {
        cd $Slicer3_LIB/python/freetype2
        runcmd ./configure --prefix=$Slicer3_LIB/python-build
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
    }

    # Now matplotlib
    cd $Slicer3_LIB/python
    runcmd $::SVN co http://matplotlib.svn.sourceforge.net/svnroot/matplotlib/trunk/matplotlib matplotlib
    if { $isWindows } {
        # Can't do Windows yet
    } else {
        cd $Slicer3_LIB/python/matplotlib
        set ::env(FREETYPEDIR) $Slicer3_LIB/python-build
        runcmd $Slicer3_LIB/python-build/bin/python ./setup.py build
    }
}
        


################################################################################
# Get and build vtk
#

if { ![file exists $::VTK_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/VTK VTK

    file mkdir $::Slicer3_LIB/VTK-build
    cd $::Slicer3_LIB/VTK-build

    set USE_VTK_ANSI_STDLIB ""
    if { $isWindows } {
        if {$MSVC6} {
            set USE_VTK_ANSI_STDLIB "-DVTK_USE_ANSI_STDLIB:BOOL=ON"
        }
    }

    #
    # Note - the two banches are identical down to the line starting -DOPENGL...
    # -- the text needs to be duplicated to avoid quoting problems with paths that have spaces
    #
    if { $isLinux && $::tcl_platform(machine) == "x86_64" } {
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_SKIP_RPATH:BOOL=ON \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DBUILD_TESTING:BOOL=OFF \
            -DVTK_USE_CARBON:BOOL=OFF \
            -DVTK_USE_X:BOOL=ON \
            -DVTK_WRAP_TCL:BOOL=ON \
            -DVTK_USE_HYBRID:BOOL=ON \
            -DVTK_USE_PATENTED:BOOL=ON \
            -DVTK_USE_PARALLEL:BOOL=ON \
            -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
            -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
            -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
            -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
            $USE_VTK_ANSI_STDLIB \
            -DOPENGL_INCLUDE_DIR:PATH=/usr/include \
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
            -DVTK_USE_64BIT_IDS:BOOL=ON \
            ../VTK
    } elseif { $isDarwin } {
        set OpenGLString "-framework OpenGL;/usr/X11R6/lib/libGL.dylib"
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_SKIP_RPATH:BOOL=OFF \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DBUILD_TESTING:BOOL=OFF \
            -DVTK_USE_CARBON:BOOL=OFF \
            -DVTK_USE_X:BOOL=ON \
            -DVTK_WRAP_TCL:BOOL=ON \
            -DVTK_USE_HYBRID:BOOL=ON \
            -DVTK_USE_PATENTED:BOOL=ON \
            -DVTK_USE_PARALLEL:BOOL=ON \
            -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
            -DOPENGL_INCLUDE_DIR:PATH=/usr/X11R6/include \
            -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
            -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
            -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
            -DOPENGL_gl_LIBRARY:STRING=$OpenGLString \
            "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
            "-DCMAKE_EXE_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
            $USE_VTK_ANSI_STDLIB \
            ../VTK
    } else {
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_SKIP_RPATH:BOOL=ON \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DBUILD_TESTING:BOOL=OFF \
            -DVTK_USE_CARBON:BOOL=OFF \
            -DVTK_USE_X:BOOL=ON \
            -DVTK_WRAP_TCL:BOOL=ON \
            -DVTK_USE_HYBRID:BOOL=ON \
            -DVTK_USE_PATENTED:BOOL=ON \
            -DVTK_USE_PARALLEL:BOOL=ON \
            -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
            -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
            -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
            -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
            $USE_VTK_ANSI_STDLIB \
            ../VTK
    }

    if { $isWindows } {
        if { $MSVC6 } {
            runcmd $::MAKE VTK.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE VTK.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}

################################################################################
# Get and build kwwidgets
#

if { ![file exists $::KWWidgets_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/KWWidgets KWWidgets

    file mkdir $::Slicer3_LIB/KWWidgets-build
    cd $::Slicer3_LIB/KWWidgets-build


    if {$isDarwin} {
        set SharedLink "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib "
        set LinkFlags "-DCMAKE_EXE_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib "
    } else {
        set SharedLink ""
        set LinkFlags ""
    }

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DVTK_DIR:PATH=$::Slicer3_LIB/VTK-build \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DKWWidgets_BUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DKWWidgets_BUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        $SharedLink $LinkFlags\
        ../KWWidgets

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE KWWidgets.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE KWWidgets.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}

################################################################################
# Get and build itk
#

if { ![file exists $::ITK_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/Insight Insight

    file mkdir $::Slicer3_LIB/Insight-build
    cd $::Slicer3_LIB/Insight-build


    if {$isDarwin} {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DITK_USE_REVIEW:BOOL=ON \
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=OFF \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        ../Insight
    } else {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DITK_USE_REVIEW:BOOL=ON \
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        ../Insight
    }

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE ITK.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE ITK.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}




################################################################################
# Get and build teem
# -- relies on VTK's png and zlib
#

if { ![file exists $::TEEM_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/teem teem

    file mkdir $::Slicer3_LIB/teem-build
    cd $::Slicer3_LIB/teem-build

    if { $isDarwin } {
        set C_FLAGS -DCMAKE_C_FLAGS:STRING=-fno-common \
    } else {
        set C_FLAGS ""
    }

    switch $::tcl_platform(os) {
        "SunOS" -
        "Linux" {
            set zlib "libvtkzlib.so"
            set png "libvtkpng.so"
        }
        "Darwin" {
            set zlib "libvtkzlib.dylib"
            set png "libvtkpng.dylib"
        }
        "Windows NT" {
            set zlib "vtkzlib.lib"
            set png "vtkpng.lib"
        }
    }

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        $C_FLAGS \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF \
        -DTEEM_ZLIB:BOOL=ON \
        -DTEEM_PNG:BOOL=ON \
        -DBUILD_VTK_CAPATIBLE_TEEM:BOOL=ON \
        -DTEEM_VTK_TOOLKITS_IPATH:FILEPATH=$::Slicer3_LIB/VTK-build \
        -DZLIB_INCLUDE_DIR:PATH=$::Slicer3_LIB/VTK/Utilities \
        -DTEEM_ZLIB_DLLCONF_IPATH:PATH=$::Slicer3_LIB/VTK-build/Utilities \
        -DZLIB_LIBRARY:FILEPATH=$::Slicer3_LIB/VTK-build/bin/$::VTK_BUILD_SUBDIR/$zlib \
        -DPNG_PNG_INCLUDE_DIR:PATH=$::Slicer3_LIB/VTK/Utilities/vtkpng \
        -DTEEM_PNG_DLLCONF_IPATH:PATH=$::Slicer3_LIB/VTK-build/Utilities \
        -DPNG_LIBRARY:FILEPATH=$::Slicer3_LIB/VTK-build/bin/$::VTK_BUILD_SUBDIR/$png \
        -DTEEM_SUBLIBRARIES:BOOL=TRUE \
        ../teem

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE teem.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE teem.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}



################################################################################
# Get and build the sandbox

# NOTE: disabled for now - nothing in the sandbox we actually need...
if { 0 && ( ![file exists $::SANDBOX_TEST_FILE] && ![file exists $::ALT_SANDBOX_TEST_FILE] || $::GENLIB(update) ) } {
    cd $::Slicer3_LIB

    runcmd $::SVN checkout $::SANDBOX_TAG NAMICSandBox

    file mkdir $::Slicer3_LIB/NAMICSandBox-build
    cd $::Slicer3_LIB/NAMICSandBox-build

    if { $isLinux && $::tcl_platform(machine) == "x86_64" } {
        # to build correctly, 64 bit linux requires shared libs for the sandbox
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_SKIP_RPATH:BOOL=ON \
            -DBUILD_EXAMPLES:BOOL=OFF \
            -DBUILD_TESTING:BOOL=OFF \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
            -DVTK_DIR:PATH=$VTK_DIR \
            -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
            -DOPENGL_glu_LIBRARY:FILEPATH=\" \" \
            ../NAMICSandBox
    } else {
        # windows and mac require static libs for the sandbox
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DBUILD_SHARED_LIBS:BOOL=OFF \
            -DCMAKE_SKIP_RPATH:BOOL=ON \
            -DBUILD_EXAMPLES:BOOL=OFF \
            -DBUILD_TESTING:BOOL=OFF \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
            -DVTK_DIR:PATH=$VTK_DIR \
            -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
            -DOPENGL_glu_LIBRARY:FILEPATH=\" \" \
            ../NAMICSandBox
    }

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE NAMICSandBox.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            #runcmd $::MAKE NAMICSandBox.SLN /build  $::VTK_BUILD_TYPE

            # These two lines fail on windows because the .sln file has a problem.
            # Perhaps this is a cmake issue.
            #cd $::Slicer3_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation
            #runcmd $::MAKE SlicerClustering.SLN /build  $::VTK_BUILD_TYPE

            # Building within the subdirectory works
            cd $::Slicer3_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation/Code
            runcmd $::MAKE SlicerClustering.vcproj /build  $::VTK_BUILD_TYPE
            cd $::Slicer3_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation/Code
            runcmd $::MAKE SlicerClustering.vcproj /build  $::VTK_BUILD_TYPE
            # However then it doesn't pick up this needed library
            cd $::Slicer3_LIB/NAMICSandBox-build/SpectralClustering
            runcmd $::MAKE SpectralClustering.SLN /build  $::VTK_BUILD_TYPE
            # this one is independent
            # TODO Distributions broken with ITK 3.0 f2c
            #cd $::Slicer3_LIB/NAMICSandBox-build/Distributions
            #runcmd $::MAKE Distributions.SLN /build  $::VTK_BUILD_TYPE
            # this one is independent
            cd $::Slicer3_LIB/NAMICSandBox-build/MGHImageIOConverter
            runcmd $::MAKE MGHImageIOConverter.SLN /build $::VTK_BUILD_TYPE
        }
    } else {

        # Just build the two libraries we need, not the rest of the sandbox.
        # This line builds the SlicerClustering library.
        # It also causes the SpectralClustering lib to build,
        # since SlicerClustering depends on it.
        # Later in the slicer Module build process,
        # vtkDTMRI links to SlicerClustering.
        # At some point in the future, the classes in these libraries
        # will become part of ITK and this will no longer be needed.
        cd $::Slicer3_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation
        eval runcmd $::MAKE
        # TODO Distributions broken with ITK 3.0 f2c
        #cd $::Slicer3_LIB/NAMICSandBox-build/Distributions
        #eval runcmd $::MAKE
        cd $::Slicer3_LIB/NAMICSandBox-build/MGHImageIOConverter
        eval runcmd $::MAKE
        cd $::Slicer3_LIB/NAMICSandBox-build
    }
}


################################################################################
# Get and build igstk
#

if { ![file exists $::IGSTK_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/IGSTK IGSTK

    file mkdir $::Slicer3_LIB/IGSTK-build
    cd $::Slicer3_LIB/IGSTK-build


    if {$isDarwin} {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DVTK_DIR:PATH=$VTK_DIR \
        -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=OFF \
        -DIGSTK_BUILD_EXAMPLES:BOOL=OFF \
        -DIGSTK_BUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
        "-DCMAKE_EXE_LINKER_FLAGS:STRING=-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
        ../IGSTK
    } else {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DVTK_DIR:PATH=$VTK_DIR \
        -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DIGSTK_BUILD_EXAMPLES:BOOL=OFF \
        -DIGSTK_BUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        ../IGSTK
    }

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE IGSTK.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE IGSTK.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {

        # Running this cmake again will populate those CMake variables
        # in IGSTK/CMakeLists.txt marked as MARK_AS_ADVANCED with their
        # default values. For instance, IGSTK_SERIAL_PORT_0, IGSTK_SERIAL_PORT_1,
        # IGSTK_SERIAL_PORT_2, ......
        eval runcmd $::CMAKE ../IGSTK

        eval runcmd $::MAKE

    }
}

################################################################################
# Get and build NaviTrack
#
#

if { 0 && (![file exists $::NaviTrack_TEST_FILE] || $::GENLIB(update)) } {
    cd $::Slicer3_LIB

    runcmd echo t | $::SVN co https://ariser.uio.no/svn/navitrack/trunk NaviTrack

    file mkdir $::Slicer3_LIB/NaviTrack-build
    cd $::Slicer3_LIB/NaviTrack-build

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        ../NaviTrack


    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE NaviTrack.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE NaviTrack.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}



################################################################################
# Get and build dcmtk
#
#

if { $::USE_SIGN && (![file exists $::dcmtk_TEST_FILE] || $::GENLIB(update)) } {
    cd $::Slicer3_LIB

    runcmd echo t | $::SVN --username ivs --password ivs co https://ariser.uio.no/svn/sign/trunk/libs/dcmtk-3.5.4 dcmtk

    file mkdir $::Slicer3_LIB/dcmtk-build
    cd $::Slicer3_LIB/dcmtk-build

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF \
        -DINSTALL_LIBDIR:PATH=$::Slicer3_LIB/dcmtk-build/bin \
        ../dcmtk


    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE dcmtk.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE dcmtk.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}


################################################################################
# Get and build BatchMake
#
#

if { ![file exists $::BatchMake_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/BatchMake BatchMake

    file mkdir $::Slicer3_LIB/BatchMake-build
    cd $::Slicer3_LIB/BatchMake-build

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DUSE_FLTK:BOOL=OFF \
        -DDASHBOARD_SUPPORT:BOOL=ON \
        -DGRID_SUPPORT:BOOL=ON \
        -DUSE_SPLASHSCREEN:BOOL=OFF \
        -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        ../BatchMake

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE BatchMake.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE BatchMake.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}




################################################################################
# Get and build SLICERLIBCURL (slicerlibcurl)
#
#

if { ![file exists $::SLICERLIBCURL_TEST_FILE] || $::GENLIB(update) } {
    cd $::Slicer3_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/cmcurl cmcurl

    file mkdir $::Slicer3_LIB/cmcurl-build
    cd $::Slicer3_LIB/cmcurl-build

    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CMAKE_CXX_FLAGS_DEBUG \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        ../cmcurl

    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE SLICERLIBCURL.dsw /MAKE "ALL_BUILD - $::VTK_BUILD_TYPE"
        } else {
            runcmd $::MAKE SLICERLIBCURL.SLN /build  $::VTK_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
    }
}



# Are all the test files present and accounted for?  If not, return error code

if { ![file exists $::CMAKE] } {
    puts "CMake test file $::CMAKE not found."
}
if { ![file exists $::TEEM_TEST_FILE] } {
    puts "Teem test file $::TEEM_TEST_FILE not found."
}
if { ![file exists $::IGSTK_TEST_FILE] } {
    puts "IGSTK test file $::IGSTK_TEST_FILE not found."
}
if { ![file exists $::BatchMake_TEST_FILE] } {
    puts "BatchMake test file $::BatchMake_TEST_FILE not found."
}
if { ![file exists $::SLICERLIBCURL_TEST_FILE] } {
    puts "SLICERLIBCURL test file $::SLICERLIBCURL_TEST_FILE not found."
}
if { ![file exists $::TCL_TEST_FILE] } {
    puts "Tcl test file $::TCL_TEST_FILE not found."
}
if { ![file exists $::TK_TEST_FILE] } {
    puts "Tk test file $::TK_TEST_FILE not found."
}
if { ![file exists $::ITCL_TEST_FILE] } {
    puts "incrTcl test file $::ITCL_TEST_FILE not found."
}
if { ![file exists $::IWIDGETS_TEST_FILE] } {
    puts "iwidgets test file $::IWIDGETS_TEST_FILE not found."
}
if { ![file exists $::BLT_TEST_FILE] } {
    puts "BLT test file $::BLT_TEST_FILE not found."
}
if { ![file exists $::VTK_TEST_FILE] } {
    puts "VTK test file $::VTK_TEST_FILE not found."
}
if { ![file exists $::ITK_TEST_FILE] } {
    puts "ITK test file $::ITK_TEST_FILE not found."
}
if { 0 &&  ![file exists $::SANDBOX_TEST_FILE] && ![file exists $::ALT_SANDBOX_TEST_FILE] } {
    if {$isLinux} {
    puts "Sandbox test file $::SANDBOX_TEST_FILE or $::ALT_SANDBOX_TEST_FILE not found."
    } else {
    puts "Sandbox test file $::SANDBOX_TEST_FILE not found."
    }
}

# check for both regular and alternate sandbox file for linux builds
if { ![file exists $::CMAKE] || \
         ![file exists $::TEEM_TEST_FILE] || \
         ![file exists $::BatchMake_TEST_FILE] || \
         ![file exists $::SLICERLIBCURL_TEST_FILE] || \
         ![file exists $::TCL_TEST_FILE] || \
         ![file exists $::TK_TEST_FILE] || \
         ![file exists $::ITCL_TEST_FILE] || \
         ![file exists $::IWIDGETS_TEST_FILE] || \
         (0 && ![file exists $::BLT_TEST_FILE]) || \
         ![file exists $::VTK_TEST_FILE] || \
         ![file exists $::ITK_TEST_FILE]  } {
    puts "Not all packages compiled; check errors and run genlib.tcl again."
    exit 1
} else {
    puts "All packages compiled."
    exit 0
}
