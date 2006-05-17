
#
# Note: this local vars file overrides sets the default environment for :
#   Scripts/genlib.tcl -- make all the support libs
#   Scripts/cmaker.tcl -- makes slicer code
#   launch.tcl -- sets up the runtime env and starts slicer
#   Scripts/tarup.tcl -- makes a tar.gz files with all the support files
#
# - use this file to set your local environment and then your change won't 
#   be overwritten when those files are updated through CVS
#


## variables that are the same for all systems
set ::SLICER_DATA_ROOT ""

if {[info exists ::env(SLICER_HOME)]} {
    # already set by the launcher
    set ::SLICER_HOME $::env(SLICER_HOME)
} else {
    # if sourcing this into cmaker, SLICER_HOME may not be set
    # set the SLICER_HOME directory to the one in which this script resides
    set cwd [pwd]
    cd [file dirname [info script]]
    set ::SLICER_HOME [pwd]
    cd $cwd
}

# set up variables for the OS Builds, to facilitate the move to solaris9
# - solaris can be solaris8 or solaris9
set solaris "solaris8"
set linux "linux-x86"
set linux_64 "linux-x86_64"
set darwin "darwin-ppc"
set windows "win32"
#
# set the default locations for the main components
#
switch $::tcl_platform(os) {
    "SunOS" { set ::env(BUILD) $solaris }
    "Linux" {           
        if {$::tcl_platform(machine) == "x86_64"} {
            set ::env(BUILD) $linux_64 
        } else {
            set ::env(BUILD) $linux
        }
    }       
    "Darwin" { set ::env(BUILD) $darwin }
    default { 
        set ::env(BUILD) $windows 
        set ::SLICER_HOME [file attributes $::SLICER_HOME -shortname]
        set ::env(SLICER_HOME) $::SLICER_HOME
    }
}

puts stderr "SLICER_HOME is $::SLICER_HOME"

# Choose which library versions you want to compile against.  These
# shouldn't be changed between releases except for testing purposes.
# If you change them and Slicer breaks, you get to keep both pieces.
#
# When modifying these variables, make sure to make appropriate
# changes in the "Files to test if library has already been built"
# section below, or genlib will happily build the library again.

set ::CMAKE_TAG "CMake-2-4-1"
set ::TEEM_TAG "Teem-1-9-0-patches"
set ::KWWidgets_TAG "HEAD"
set ::VTK_TAG "VTK-5-0"
set ::ITK_TAG "Slicer-2-6"
set ::TCL_TAG "core-8-4-6"
set ::TK_TAG "core-8-4-6"
set ::ITCL_TAG "itcl-3-2-1"
set ::IWIDGETS_TAG "iwidgets-4-0-1"
set ::BLT_TAG "blt24z"
set ::SANDBOX_TAG "http://svn.na-mic.org:8000/svn/NAMICSandBox/branches/Slicer-2-6"

# Set library, binary, etc. paths...

#set ::SLICER_LIB $::SLICER_HOME/Lib/$::env(BUILD) ;# as used in slicer2
#                                                  ;# now can be set outide 
#                                                  ;# of this file

# if SLICER_LIB and SLICER_BUILD haven't been set, 
# then assume they are in the 'standard' places next to the source tree
# (as created by getbuildtest.tcl
if { ![info exists ::SLICER_LIB] } {
    set wd [pwd]
    cd $SLICER_HOME/../Slicer3-lib
    set SLICER_LIB [pwd]
    cd $wd
}
if { ![info exists ::SLICER_BUILD] } {
    set wd [pwd]
    cd $SLICER_HOME/../Slicer3-build
    set SLICER_BUILD [pwd]
    cd $wd
}

set ::TEEM_SRC_DIR  $::SLICER_LIB/teem
set ::TEEM_BUILD_DIR  $::SLICER_LIB/teem-build
set ::VTK_DIR  $::SLICER_LIB/VTK-build
set ::VTK_SRC_DIR $::SLICER_LIB/VTK
set ::VTK_BUILD_TYPE ""
set ::VTK_BUILD_SUBDIR ""
set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
set ::KWWidgets_BUILD_DIR  $::SLICER_LIB/KWWidgets-build
set ::ITK_BINARY_PATH $::SLICER_LIB/Insight-build
set ::SANDBOX_BIN_DIR $::SLICER_LIB/NAMICSandBox-build/bin
set ::TCL_BIN_DIR $::SLICER_LIB/tcl-build/bin
set ::TCL_LIB_DIR $::SLICER_LIB/tcl-build/lib
set ::TCL_INCLUDE_DIR $::SLICER_LIB/tcl-build/include
set ::CMAKE_PATH $::SLICER_LIB/CMake-build
set ::SOV_BINARY_DIR ""
set ::XVNC_EXECUTABLE " "


# Files to test if library has already been built by genlib.tcl.

switch $::tcl_platform(os) {
    "SunOS" -
    "Linux" {
        set shared_lib_ext "so"
    }
    "Darwin" {
        set shared_lib_ext "dylib"
    }
    "Windows NT" {
        set shared_lib_ext "dll"
    }
}

# TODO: identify files for each platform

switch $::tcl_platform(os) {
    "SunOS" -
    "Darwin" {
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitclstub3.2.a
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $KWWidgets_BUILD_DIR/bin/libKWWidgets.$shared_lib_ext
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::SLICER_HOME/tkEventPatch.diff
        set ::BLT_PATCH $::SLICER_HOME/blt-patch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
    }
    "Linux" {
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitclstub3.2.a
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $KWWidgets_BUILD_DIR/bin/libKWWidgets.so
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.so
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::SLICER_HOME/tkEventPatch.diff
        set ::BLT_PATCH $::SLICER_HOME/blt-patch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
    }
    "Windows NT" {
    # Windows NT currently covers WinNT, Win2000, XP Home, XP Pro

        #
        ### Set your peferred build type: 
        #
        #set ::VTK_BUILD_TYPE RelWithDebInfo ;# good if you have the full (expensive) compiler
        #set ::VTK_BUILD_TYPE Release  ;# faster, but no debugging
        set ::VTK_BUILD_TYPE Debug  ;# a good default
        set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin/$::VTK_BUILD_TYPE

        set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh84.exe
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish84.exe
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/itcl3.2/itcl32.dll
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.2/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/BLT24.dll
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu.exe
        set ::VTK_TEST_FILE $::VTK_DIR/bin/$::VTK_BUILD_TYPE/vtk.exe
        set ::KWWidgets_TEST_FILE $KWWidgets_BUILD_DIR/bin/$::env(VTK_BUILD_SUBDIR)/KWWidgets.lib
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/$::VTK_BUILD_TYPE/SlicerClustering.lib
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/$::VTK_BUILD_TYPE/SlicerClustering.lib
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
        set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/$::VTK_BUILD_TYPE/ITKCommon.dll

    }
    default {
        puts stderr "Could not match platform \"$::tcl_platform(os)\"."
        exit
    }
}

# System dependent variables

switch $::tcl_platform(os) {
    "SunOS" {
        set ::VTKSLICERBASE_BUILD_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/local/os/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set ::MAKE "gmake"
        set ::SERIAL_MAKE "gmake"
    }
    "Linux" {
        set ::VTKSLICERBASE_BUILD_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles" 
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set ::MAKE "make -j 8"
        set ::SERIAL_MAKE "make"
    }
    "Darwin" {
        set ::VTKSLICERBASE_BUILD_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.dylib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.dylib
        set ::GENERATOR "Unix Makefiles" 
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++-3.3"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set ::MAKE make
        set ::SERIAL_MAKE make
    }
    default {
        # different windows machines say different things, so assume
        # that if it doesn't match above it must be windows
        # (VC7 is Visual C++ 7.0, also known as the .NET version)


        set ::VTKSLICERBASE_BUILD_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBase.lib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::SLICER_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBaseTCL.lib

        set ::CMAKE $::CMAKE_PATH/bin/cmake.exe

        set MSVC6 0
        #
        ## match this to the version of the compiler you have:
        #
        
        ## for Visual Studio 6:
        #set ::GENERATOR "Visual Studio 6" 
        #set ::MAKE "msdev"
        #set ::COMPILER_PATH ""
        #set MSVC6 1

        if {[info exists ::env(MSVC6)]} {
            set ::MSVC6 $::env(MSVC6)
        } else {
        }

        ## for Visual Studio 7:
        if {[info exists ::env(GENERATOR)]} {
            set ::GENERATOR $::env(GENERATOR)
        } else {
            set ::GENERATOR "Visual Studio 7" 
        }

        if {[info exists ::env(MAKE)]} {
            set ::MAKE $::env(MAKE)
        } else {
            set ::MAKE "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET/Common7/IDE/devenv"
        }

        if {[info exists ::env(COMPILER_PATH)]} {
            set ::COMPILER_PATH $::env(COMPILER_PATH)
        } else {
            set ::COMPILER_PATH "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET/Common7/Vc7/bin"
        }

        #
        ## for Visual Studio 7.1:
        # - automatically use newest if available
        #
        if { [file exists "c:/Program Files/Microsoft Visual Studio .NET 2003/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 7 .NET 2003" 
            set ::MAKE "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET 2003/Common7/IDE/devenv"
            set ::COMPILER_PATH "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET 2003/Vc7/bin"
        }

        set ::COMPILER "cl"
        set ::SERIAL_MAKE $::MAKE

    }
}


