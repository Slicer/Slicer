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
#

if {[info exists ::env(CVS)]} {
    set ::CVS "{$::env(CVS)}"
} else {
    set ::CVS cvs
}

# for subversion repositories (Sandbox)
if {[info exists ::env(SVN)]} {
    set ::SVN $::env(SVN)
} else {
    set ::SVN svn
}

################################################################################
#
# check to see if need to build a package
# returns 1 if need to build, 0 else
# if { [BuildThis  ""] == 1 } {
proc BuildThis { testFile packageName } {
    if {![file exists $testFile] || $::GENLIB(update) || [lsearch $::GENLIB(buildList) $packageName] != -1} {
        # puts "Building $packageName (testFile = $testFile, update = $::GENLIB(update), buildlist = $::GENLIB(buildList) )"
        return 1
    } else {
        # puts "Skipping $packageName"
        return 0
    }
}
#

################################################################################
#
# simple command line argument parsing
#

proc Usage { {msg ""} } {
    global SLICER
    
    set msg "$msg\nusage: genlib \[options\] \[target\]"
    set msg "$msg\n  \[target\] is the the SLICER_LIB directory"
    set msg "$msg\n             and is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   --help : prints this message and exits"
    set msg "$msg\n   --clean : delete the target first"
    set msg "$msg\n   --update : do a cvs update even if there's an existing build"
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   optional space separated list of packages to build (lower case)"
    puts stderr $msg
}

set GENLIB(clean) "false"
set GENLIB(update) "false"
set ::GENLIB(buildList) ""
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
            set ::VTK_BUILD_TYPE "Release"
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
puts "Stripped args = $argv"

set ::SLICER_LIB ""
if {$argc > 1 } {
  #Usage
  #exit 1
    # the stripped args list now has the SLICER_LIB first and then the list of packages to build
    set ::GENLIB(buildList) [lrange $strippedargs 1 end]
    set strippedargs [lindex $strippedargs 0]
# puts "Got the list of package to build: '$::GENLIB(buildList)' , stripped args = $strippedargs"
} 
set ::SLICER_LIB $strippedargs


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
# set SLICER_HOME [file dirname [file dirname [file normalize [info script]]]]
set cwd [pwd]
cd [file dirname [info script]]
cd ..
set SLICER_HOME [pwd]
cd $cwd

if { $::SLICER_LIB == "" } {
  set ::SLICER_LIB [file dirname $::SLICER_HOME]/Slicer3-lib
  puts "SLICER_LIB is $::SLICER_LIB"
}

#######
#
# Note: the local vars file, slicer2/slicer_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't 
#   be overwritten when this file is updated
#
set localvarsfile $SLICER_HOME/slicer_variables.tcl
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
        runcmd rm -rf $SLICER_LIB
        if { [file exists $SLICER_LIB/tcl/isPatched] } {
            runcmd rm $SLICER_LIB/tcl/isPatched
        }

        if { [file exists $SLICER_LIB/tcl/isPatchedBLT] } {
            runcmd rm $SLICER_LIB/tcl/isPatchedBLT
        }
    } else {
        file delete -force $SLICER_LIB
    }
}

if { ![file exists $SLICER_LIB] } {
    file mkdir $SLICER_LIB
}

################################################################################
# Get and unzip Slicer Lib file if Windows
#

if {$isWindows} {
    if {![file exists $::CMAKE]} {
        cd $SLICER_LIB
        runcmd curl -k -O http://www.na-mic.org/Slicer/Download/External/Slicer3-lib_win32.zip
        runcmd unzip ./Slicer3-lib_win32.zip
    }
}

################################################################################
# If is Darwin, don't use cvs compression to get around bug in cvs 1.12.13
#

if {$isDarwin} {
    set CVS_CO_FLAGS "-q"  
} else {
    set CVS_CO_FLAGS "-q -z3"    
}


################################################################################
# Get and build CMake
#

# set in slicer_vars
if { [BuildThis $::CMAKE "cmake"] == 1 } {
    file mkdir $::CMAKE_PATH
    cd $SLICER_LIB


    if {$isWindows} {
      if { ! $::GENLIB(update) } {
        puts stderr "Slicer3-lib_win32.zip did not download and unzip CMAKE correctly."
        exit
      }
    } else {
        runcmd $::CVS -d :pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake login
        eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@www.cmake.org:/cvsroot/CMake checkout -r $::CMAKE_TAG CMake"

        cd $::CMAKE_PATH
        if { $isSolaris } {
            # make sure to pick up curses.h in /local/os/include
            runcmd $SLICER_LIB/CMake/bootstrap --init=$SLICER_HOME/Scripts/spl.cmake.init
        } else {
            runcmd $SLICER_LIB/CMake/bootstrap
        } 
        eval runcmd $::MAKE
    }
}


################################################################################
# Get and build tcl, tk, itcl, widgets
#

# on windows, tcl won't build right, as can't configure, so save commands have to run
if { [BuildThis $::TCL_TEST_FILE "tcl"] == 1 } {

    if {$isWindows} {
      if { ! $::GENLIB(update) } {
        puts stderr "Slicer3-lib_win32.zip did not download and unzip Tcl correctly."
        exit
      }
    }

    file mkdir $SLICER_LIB/tcl
    cd $SLICER_LIB/tcl

    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer checkout -r $::TCL_TAG tcl"

    if {$isWindows} {
        # can't do windows
    } else {
        cd $SLICER_LIB/tcl/tcl/unix

        runcmd ./configure --prefix=$SLICER_LIB/tcl-build
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
    }
}

if { [BuildThis $::TK_TEST_FILE "tk"] == 1 } {
    cd $SLICER_LIB/tcl

    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer checkout -r $::TK_TAG tk"

    if {$isDarwin} {
        if { ![file exists $SLICER_LIB/tcl/isPatched] } {
                puts "Patching..."
                runcmd curl -k -O https://share.spl.harvard.edu/share/birn/public/software/External/Patches/tkEventPatch.diff
                runcmd cp tkEventPatch.diff $SLICER_LIB/tcl/tk/generic 
                cd $SLICER_LIB/tcl/tk/generic
                runcmd patch -i tkEventPatch.diff

                # create a file to make sure tkEvent.c isn't patched twice
                runcmd touch $SLICER_LIB/tcl/isPatched
                file delete $SLICER_LIB/tcl/tk/generic/tkEventPatch.diff
        } else {
            puts "tkEvent.c already patched."
        }
    }

    if {$isWindows} {
        # can't do windows
    } else {
        cd $SLICER_LIB/tcl/tk/unix

        runcmd ./configure --with-tcl=$SLICER_LIB/tcl-build/lib --prefix=$SLICER_LIB/tcl-build
        eval runcmd $::MAKE
        eval runcmd $::MAKE install

        file copy -force $SLICER_LIB/tcl/tk/generic/default.h $SLICER_LIB/tcl-build/include
        file copy -force $SLICER_LIB/tcl/tk/unix/tkUnixDefault.h $SLICER_LIB/tcl-build/include
    }
}

if { [BuildThis $::ITCL_TEST_FILE "itcl"] == 1 } {
    cd $SLICER_LIB/tcl

    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer checkout -r $::ITCL_TAG incrTcl"

    cd $SLICER_LIB/tcl/incrTcl

    exec chmod +x ../incrTcl/configure 

    if {$isWindows} {
        # can't do windows
    } else {
        if { $isDarwin } {
            exec cp ../incrTcl/itcl/configure ../incrTcl/itcl/configure.orig
            exec sed -e "s/\\*\\.c | \\*\\.o | \\*\\.obj) ;;/\\*\\.c | \\*\\.o | \\*\\.obj | \\*\\.dSYM | \\*\\.gnoc ) ;;/" ../incrTcl/itcl/configure.orig > ../incrTcl/itcl/configure 
        }
        runcmd ../incrTcl/configure --with-tcl=$SLICER_LIB/tcl-build/lib --with-tk=$SLICER_LIB/tcl-build/lib --prefix=$SLICER_LIB/tcl-build
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

if { [BuildThis $::IWIDGETS_TEST_FILE "iwidgets"] == 1 } {
    cd $SLICER_LIB/tcl

    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer checkout -r $::IWIDGETS_TAG iwidgets"


    if {$isWindows} {
        # can't do windows
    } else {
        cd $SLICER_LIB/tcl/iwidgets
        runcmd ../iwidgets/configure --with-tcl=$SLICER_LIB/tcl-build/lib --with-tk=$SLICER_LIB/tcl-build/lib --with-itcl=$SLICER_LIB/tcl/incrTcl --prefix=$SLICER_LIB/tcl-build
        # make all doesn't do anything... 
        # iwidgets won't compile in parallel (with -j flag)
        eval runcmd $::SERIAL_MAKE all
        eval runcmd $::SERIAL_MAKE install
    }
}


################################################################################
# Get and build blt
#

if { [BuildThis $::BLT_TEST_FILE "blt"] == 1 } {
    cd $SLICER_LIB/tcl
    
    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer co -r $::BLT_TAG blt"

    if { $isWindows } {
        # can't do Windows
    } elseif { $isDarwin } {
        if { ![file exists $SLICER_LIB/tcl/isPatchedBLT] } {
            puts "Patching..."
            runcmd curl -k -O https://share.spl.harvard.edu/share/birn/public/software/External/Patches/bltpatch
            cd $SLICER_LIB/tcl/blt
            runcmd patch -p2 < ../bltpatch
            
            # create a file to make sure BLT isn't patched twice
            runcmd touch $SLICER_LIB/tcl/isPatchedBLT
            file delete $SLICER_LIB/tcl/bltpatch
        } else {
            puts "BLT already patched."
        }

        cd $SLICER_LIB/tcl/blt
        runcmd ./configure --with-tcl=$SLICER_LIB/tcl/tcl/unix --with-tk=$SLICER_LIB/tcl-build --prefix=$SLICER_LIB/tcl-build --enable-shared --x-includes=/usr/X11R6/include --with-cflags=-fno-common
        
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
    } else {
        cd $SLICER_LIB/tcl/blt
        runcmd ./configure --with-tcl=$SLICER_LIB/tcl/tcl/unix --with-tk=$SLICER_LIB/tcl-build --prefix=$SLICER_LIB/tcl-build 
        eval runcmd $::SERIAL_MAKE
        eval runcmd $::SERIAL_MAKE install
    }
}


################################################################################
# Get and build vtk
#

if { [BuildThis $::VTK_TEST_FILE "vtk"] == 1 } {
    cd $SLICER_LIB

    runcmd $::CVS -d :pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@public.kitware.com:/cvsroot/VTK checkout -r $::VTK_TAG VTK"

    # Andy's temporary hack to get around wrong permissions in VTK cvs repository
    # catch statement is to make file attributes work with RH 7.3
    if { !$isWindows } {
        catch "file attributes $SLICER_LIB/VTK/VTKConfig.cmake.in -permissions a+rw"
    }

    file mkdir $SLICER_LIB/VTK-build
    cd $SLICER_LIB/VTK-build

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
            -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
            -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
            -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
            -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
            $USE_VTK_ANSI_STDLIB \
            -DOPENGL_INCLUDE_DIR:PATH=/usr/include \
            -DOPENGL_gl_LIBRARY:FILEPATH=/usr/lib64/libGL.so \
            -DOPENGL_glu_LIBRARY:FILEPATH=/usr/lib64/libGLU.so \
            -DX11_X11_LIB:FILEPATH=/usr/X11R6/lib64/libX11.a \
            -DX11_Xext_LIB:FILEPATH=/usr/X11R6/lib64/libXext.a \
            -DCMAKE_MODULE_LINKER_FLAGS:STRING=-L/usr/X11R6/lib64 \
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
            -DVTK_USE_64BIT_IDS:BOOL=ON \
            ../VTK
    } elseif { $isDarwin } {
        set OpenGLString "-framework OpenGL;/usr/X11R6/lib/libGL.dylib"
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_SKIP_RPATH:BOOL=OFF \
            -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
            -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
            -DCMAKE_SHARED_LINKER_FLAGS:STRING="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
            -DCMAKE_EXE_LINKER_FLAGS="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
            -DBUILD_TESTING:BOOL=OFF \
            -DVTK_USE_CARBON:BOOL=OFF \
            -DVTK_USE_X:BOOL=ON \
            -DVTK_WRAP_TCL:BOOL=ON \
            -DVTK_USE_HYBRID:BOOL=ON \
            -DVTK_USE_PATENTED:BOOL=ON \
            -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
            -DOPENGL_INCLUDE_DIR:PATH=/usr/X11R6/include \
            -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
            -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
            -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
            -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
            -DOPENGL_gl_LIBRARY:STRING=$OpenGLString \
            $USE_VTK_ANSI_STDLIB \
            ../VTK
    } else {
        runcmd $::CMAKE \
            -G$GENERATOR \
            -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
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

if { [BuildThis $::KWWidgets_TEST_FILE "kwwidgets"] == 1 } {
    cd $SLICER_LIB

    runcmd $::CVS -d :pserver:anoncvs:@www.kwwidgets.org:/cvsroot/KWWidgets login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets checkout -r $::KWWidgets_TAG KWWidgets"

    file mkdir $SLICER_LIB/KWWidgets-build
    cd $SLICER_LIB/KWWidgets-build



    runcmd $::CMAKE \
        -G$GENERATOR \
        -DVTK_DIR:PATH=$SLICER_LIB/VTK-build \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DBUILD_EXAMPLES:BOOL=ON \
        -DBUILD_TESTING:BOOL=ON \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        ../KWWidgets

    if { $isDarwin } {
      runcmd $::CMAKE \
          -DCMAKE_SHARED_LINKER_FLAGS:STRING="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
          -DCMAKE_EXE_LINKER_FLAGS="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
          ../KWWidgets
    }

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

if { [BuildThis $::ITK_TEST_FILE "itk"] == 1 } {
    cd $SLICER_LIB

    runcmd $::CVS -d :pserver:anoncvs:@www.vtk.org:/cvsroot/Insight login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.vtk.org:/cvsroot/Insight checkout -r $::ITK_TAG Insight"

    file mkdir $SLICER_LIB/Insight-build
    cd $SLICER_LIB/Insight-build


    if {$isDarwin} {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=OFF \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
        ../Insight
    } else {
    runcmd $::CMAKE \
        -G$GENERATOR \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DCMAKE_BUILD_TYPE:STRING=$::VTK_BUILD_TYPE \
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

    puts "Patching ITK..."

    set fp1 [open "$SLICER_LIB/Insight-build/Utilities/nifti/niftilib/cmake_install.cmake" r]
    set fp2 [open "$SLICER_LIB/Insight-build/Utilities/nifti/znzlib/cmake_install.cmake" r]
    set data1 [read $fp1]
#    puts "data1 is $data1"
    set data2 [read $fp2]
#    puts "data2 is $data2"

    close $fp1
    close $fp2

    regsub -all /usr/local/lib $data1 \${CMAKE_INSTALL_PREFIX}/lib data1
    regsub -all /usr/local/include $data1 \${CMAKE_INSTALL_PREFIX}/include data1
    regsub -all /usr/local/lib $data2 \${CMAKE_INSTALL_PREFIX}/lib data2
    regsub -all /usr/local/include $data2 \${CMAKE_INSTALL_PREFIX}/include data2

    set fw1 [open "$SLICER_LIB/Insight-build/Utilities/nifti/niftilib/cmake_install.cmake" w]
    set fw2 [open "$SLICER_LIB/Insight-build/Utilities/nifti/znzlib/cmake_install.cmake" w]

    puts -nonewline $fw1 $data1
#    puts "data1out is $data1"
    puts -nonewline $fw2 $data2
#    puts "data2out is $data2"
 
    close $fw1
    close $fw2
}


################################################################################
# Get and build teem
# -- relies on VTK's png and zlib
#

if { [BuildThis $::TEEM_TEST_FILE "teem"] == 1 } {
    cd $SLICER_LIB

    runcmd $::CVS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer login 
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous:bwhspl@cvs.spl.harvard.edu:/projects/cvs/slicer checkout -r $::TEEM_TAG teem"

    file mkdir $SLICER_LIB/teem-build
    cd $SLICER_LIB/teem-build

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
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        $C_FLAGS \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF \
        -DTEEM_ZLIB:BOOL=ON \
        -DTEEM_PNG:BOOL=ON \
        -DTEEM_VTK_MANGLE:BOOL=ON \
        -DTEEM_VTK_TOOLKITS_IPATH:FILEPATH=$::SLICER_LIB/VTK-build \
        -DZLIB_INCLUDE_DIR:PATH=$::SLICER_LIB/VTK/Utilities/vtkzlib \
        -DTEEM_ZLIB_DLLCONF_IPATH:PATH=$::SLICER_LIB/VTK-build/Utilities \
        -DZLIB_LIBRARY:FILEPATH=$::SLICER_LIB/VTK-build/bin/$::VTK_BUILD_SUBDIR/$zlib \
        -DPNG_PNG_INCLUDE_DIR:PATH=$::SLICER_LIB/VTK/Utilities/vtkpng \
        -DTEEM_PNG_DLLCONF_IPATH:PATH=$::SLICER_LIB/VTK-build/Utilities \
        -DPNG_LIBRARY:FILEPATH=$::SLICER_LIB/VTK-build/bin/$::VTK_BUILD_SUBDIR/$png \
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

if { [BuildThis $::SANDBOX_TEST_FILE "sandbox"] == 1 && [BuildThis $::ALT_SANDBOX_TEST_FILE "sandbox"] == 1 } {
    cd $SLICER_LIB

    runcmd $::SVN checkout $::SANDBOX_TAG NAMICSandBox 

    file mkdir $SLICER_LIB/NAMICSandBox-build
    cd $SLICER_LIB/NAMICSandBox-build

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
            #cd $SLICER_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation
            #runcmd $::MAKE SlicerClustering.SLN /build  $::VTK_BUILD_TYPE

            # Building within the subdirectory works
            cd $SLICER_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation/Code
            runcmd $::MAKE SlicerClustering.vcproj /build  $::VTK_BUILD_TYPE
            cd $SLICER_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation/Code
            runcmd $::MAKE SlicerClustering.vcproj /build  $::VTK_BUILD_TYPE
            # However then it doesn't pick up this needed library
            cd $SLICER_LIB/NAMICSandBox-build/SpectralClustering
            runcmd $::MAKE SpectralClustering.SLN /build  $::VTK_BUILD_TYPE
            # this one is independent
            # TODO Distributions broken with ITK 3.0 f2c
            #cd $SLICER_LIB/NAMICSandBox-build/Distributions
            #runcmd $::MAKE Distributions.SLN /build  $::VTK_BUILD_TYPE
            # this one is independent
            cd $SLICER_LIB/NAMICSandBox-build/MGHImageIOConverter
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
        cd $SLICER_LIB/NAMICSandBox-build/SlicerTractClusteringImplementation   
        eval runcmd $::MAKE 
        # TODO Distributions broken with ITK 3.0 f2c
        #cd $SLICER_LIB/NAMICSandBox-build/Distributions
        #eval runcmd $::MAKE
        cd $SLICER_LIB/NAMICSandBox-build/MGHImageIOConverter
        eval runcmd $::MAKE
        cd $SLICER_LIB/NAMICSandBox-build
    }
}


################################################################################
# Get and build igstk 
#

if { [BuildThis $::IGSTK_TEST_FILE "igstk"] == 1 } {
    cd $SLICER_LIB

    runcmd $::CVS -d:pserver:anonymous:igstk@public.kitware.com:/cvsroot/IGSTK login
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@public.kitware.com:/cvsroot/IGSTK co -r IGSTK-2-0 IGSTK"

    file mkdir $SLICER_LIB/IGSTK-build
    cd $SLICER_LIB/IGSTK-build


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
        ../IGSTK
    }

    if { $isDarwin } {
      runcmd $::CMAKE \
          -DCMAKE_SHARED_LINKER_FLAGS:STRING="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
          -DCMAKE_EXE_LINKER_FLAGS="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib" \
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
# Get and build SLICERLIBCURL (slicerlibcurl)
#
#

if { [BuildThis $::SLICERLIBCURL_TEST_FILE "libcurl"] == 1 } {
    cd $::SLICER_LIB

    runcmd $::SVN co http://www.na-mic.org/svn/Slicer3-lib-mirrors/trunk/cmcurl cmcurl

    file mkdir $::SLICER_LIB/cmcurl-build
    cd $::SLICER_LIB/cmcurl-build

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
if { ![file exists $::SANDBOX_TEST_FILE] && ![file exists $::ALT_SANDBOX_TEST_FILE] } { 
    if {$isLinux} { 
    puts "Sandbox test file $::SANDBOX_TEST_FILE or $::ALT_SANDBOX_TEST_FILE not found." 
    } else { 
    puts "Sandbox test file $::SANDBOX_TEST_FILE not found." 
    }
}

# check for both regular and alternate sandbox file for linux builds
if { ![file exists $::CMAKE] || \
         ![file exists $::TEEM_TEST_FILE] || \
         ![file exists $::SLICERLIBCURL_TEST_FILE] || \
         ![file exists $::TCL_TEST_FILE] || \
         ![file exists $::TK_TEST_FILE] || \
         ![file exists $::ITCL_TEST_FILE] || \
         ![file exists $::IWIDGETS_TEST_FILE] || \
         ![file exists $::BLT_TEST_FILE] || \
         ![file exists $::VTK_TEST_FILE] || \
         ![file exists $::ITK_TEST_FILE] || \
         ![file exists $::SANDBOX_TEST_FILE] } {
    if { ![file exists $::ALT_SANDBOX_TEST_FILE] } {
    puts "Not all packages compiled; check errors and run genlib.tcl again."
    exit 1 
    }
} else { 
    puts "All packages compiled."
    exit 0 
}
