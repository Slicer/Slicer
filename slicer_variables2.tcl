
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
set ::Slicer3_DATA_ROOT ""

if {[info exists ::env(Slicer3_HOME)]} {
    # already set by the launcher
    set ::Slicer3_HOME $::env(Slicer3_HOME)
} else {
    # if sourcing this into cmaker, Slicer3_HOME may not be set
    # set the Slicer3_HOME directory to the one in which this script resides
    set cwd [pwd]
    cd [file dirname [info script]]
    set ::Slicer3_HOME [pwd]
    set ::env(Slicer3_HOME) $::Slicer3_HOME
    cd $cwd
}

# set up variables for the OS Builds, to facilitate the move to solaris9
# - solaris can be solaris8 or solaris9
set ::SOLARIS "solaris8"
set ::LINUX "linux-x86"
set ::LINUX_64 "linux-x86_64"
set ::DARWIN "darwin-ppc"
set ::DARWIN_X86 "darwin-x86"
set ::WINDOWS "win32"

#
# set the default locations for the main components
#
switch $::tcl_platform(os) {
    "SunOS" { set ::env(BUILD) $::SOLARIS }
    "Linux" {
        if {$::tcl_platform(machine) == "x86_64"} {
            set ::env(BUILD) $::LINUX_64
        } else {
            set ::env(BUILD) $::LINUX
        }
    }
    "Darwin" {
        if {$::tcl_platform(machine) == "i386"} {
            set ::env(BUILD) $::DARWIN_X86
        } else {
            set ::env(BUILD) $::DARWIN
        }
    }
    default {
        set ::env(BUILD) $::WINDOWS
        set ::Slicer3_HOME [file attributes $::Slicer3_HOME -shortname]
        set ::env(Slicer3_HOME) $::Slicer3_HOME
    }
}

puts stderr "Slicer3_HOME is $::Slicer3_HOME"

# Choose which library versions you want to compile against.  These
# shouldn't be changed between releases except for testing purposes.
# If you change them and Slicer breaks, you get to keep both pieces.
#
# When modifying these variables, make sure to make appropriate
# changes in the "Files to test if library has already been built"
# section below, or genlib will happily build the library again.

#set ::Slicer3_TAG "http://www.na-mic.org/svn/Slicer3/branches/Slicer3.0-PreAHMBeta"
#set ::Slicer3_TAG "http://www.na-mic.org/svn/Slicer3/branches/testing/sign_integration/Slicer3"
set ::Slicer3_TAG "http://www.na-mic.org/svn/Slicer3/trunk"
set ::CMAKE_TAG "CMake-2-4-2"
#set ::TEEM_TAG "HEAD"
set ::TEEM_TAG "Teem-1-9-0-patches"
set ::KWWidgets_TAG "Slicer-3-0"
set ::VTK_TAG "VTK-5-0"
set ::ITK_TAG ITK-3-4
set ::TCL_TAG "core-8-4-6"
set ::TK_TAG "core-8-4-6"
set ::ITCL_TAG "itcl-3-2-1"
set ::IWIDGETS_TAG "iwidgets-4-0-1"
set ::BLT_TAG "blt24z"
set ::SANDBOX_TAG "http://svn.na-mic.org/svn/NAMICSandBox/branches/Slicer-2-6"
set ::NAVITRACK_TAG "https://ariser.uio.no/svn/navitrack/trunk"
set ::SIGN_TAG "https://ariser.uio.no/svn/sign/branches/stable-2.0/libs/SIGN"
set ::SIGN_APP_TAG "https://ariser.uio.no/svn/sign/branches/stable-2.0/Applications"
set ::PYTHON_TAG "http://svn.python.org/projects/python/branches/release25-maint"
set ::NUMPY_TAG "http://svn.scipy.org/svn/numpy/trunk"
set ::SCIPY_TAG "http://svn.scipy.org/svn/scipy/trunk"
set ::BatchMake_TAG "HEAD"
set ::SLICERLIBCURL_TAG "HEAD"
set ::USE_SIGN 0


# Set library, binary, etc. paths...

# if Slicer3_LIB and Slicer3_BUILD haven't been set,
# then assume they are in the 'standard' places next to the source tree
# (as created by getbuildtest.tcl
if { ![info exists ::Slicer3_LIB] } {
    set wd [pwd]
    cd $::Slicer3_HOME/../Slicer3-lib
    set ::Slicer3_LIB [pwd]
    cd $wd
}
if { ![info exists ::Slicer3_BUILD] } {
    set wd [pwd]
    cd $::Slicer3_HOME/../Slicer3-build
    set ::Slicer3_BUILD [pwd]
    cd $wd
}

set ::TEEM_SRC_DIR  $::Slicer3_LIB/teem
set ::TEEM_BUILD_DIR  $::Slicer3_LIB/teem-build
set ::VTK_DIR  $::Slicer3_LIB/VTK-build
set ::VTK_SRC_DIR $::Slicer3_LIB/VTK
set ::CMAKE_CXX_FLAGS_DEBUG "-g"
if { ![info exists ::VTK_BUILD_TYPE] } {
  # set a default if it hasn't already been specified
  set ::VTK_BUILD_TYPE "Debug" ;# options: Release, RelWithDebInfo, Debug
}
set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE 
set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
set ::KWWidgets_BUILD_DIR  $::Slicer3_LIB/KWWidgets-build
set ::KWWIDGETS_DIR  $::Slicer3_LIB/KWWidgets
set ::ITK_BINARY_PATH $::Slicer3_LIB/Insight-build
set ::SANDBOX_BIN_DIR $::Slicer3_LIB/NAMICSandBox-build/bin
set ::TCL_BIN_DIR $::Slicer3_LIB/tcl-build/bin
set ::TCL_LIB_DIR $::Slicer3_LIB/tcl-build/lib
set ::TCL_INCLUDE_DIR $::Slicer3_LIB/tcl-build/include
set ::CMAKE_PATH $::Slicer3_LIB/CMake-build
set ::SOV_BINARY_DIR ""
set ::XVNC_EXECUTABLE " "
set ::IGSTK_DIR $::Slicer3_LIB/IGSTK-build
set ::NaviTrack_DIR $::Slicer3_LIB/NaviTrack-build
set ::dcmtk_DIR $::Slicer3_LIB/dcmtk-build
set ::BatchMake_SRC_DIR $::Slicer3_LIB/BatchMake
set ::BatchMake_BUILD_DIR $::Slicer3_LIB/BatchMake-build
set ::SLICERLIBCURL_SRC_DIR $::Slicer3_LIB/cmcurl
set ::SLICERLIBCURL_BUILD_DIR $::Slicer3_LIB/cmcurl-build



# Options for building IGT modules in Slicer
set ::IGSTK "OFF"
set ::OPENTRACKER "OFF"
# If the value of OPENTRACKER is "ON", you need to set the following variables:

# ON:  opentracker 1.3
# OFF: opentracker 2.0
set ::OT_VERSION "ON"

# OT_LIB_DIR: the absolute path contains the OpenTracker library,
# e.g. libOpenTrakcer.so
# set ::OT_LIB_DIR /home/hliu/projects/splot-build/opentracker/lib/.libs/
set ::OT_LIB_DIR ""

# OT_INC: the directory contains "OpenTracker.h" (opentracker 1.3) or
#         "OpenTracker/OpenTracker.h" (opentracker 2.0)
# set ::OT_INC_DIR /home/hliu/projects/splot-build/opentracker/src
set ::OT_INC_DIR ""


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
        set ::VTK_BUILD_SUBDIR ""
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitclstub3.2.a
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::PYTHON_TEST_FILE $::Slicer3_LIB/python-build/bin/python
        set ::MATPLOTLIB_TEST_FILE $::Slicer3_LIB/python-build/bin/bar
        set ::NUMPY_TEST_FILE $::Slicer3_LIB/python-build/lib/python2.5/site-packages/numpy/lib/_compiled_base.$shared_lib_ext
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.$shared_lib_ext
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::BLT_PATCH $::Slicer3_HOME/blt-patch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/libIGSTK.$shared_lib_ext
        set ::NaviTrack_TEST_FILE $::NaviTrack_DIR/libNaviTrack.$shared_lib_ext
        set ::dcmtk_TEST_FILE $::dcmtk_DIR/dcmnet/libsrc/libdcmnet.a
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/BatchMake
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a
    }
    "Linux" {
        set ::VTK_BUILD_SUBDIR ""
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitclstub3.2.a
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::PYTHON_TEST_FILE $::Slicer3_LIB/python-build/bin/python
        set ::MATPLOTLIB_TEST_FILE $::Slicer3_LIB/python-build/bin/bar
        set ::NUMPY_TEST_FILE $::Slicer3_LIB/python-build/lib/python2.5/site-packages/numpy/lib/_compiled_base.so
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.so
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.so
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/libSlicerClustering.a
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::BLT_PATCH $::Slicer3_HOME/blt-patch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/libIGSTK.so
        set ::NaviTrack_TEST_FILE $::NaviTrack_DIR/libNaviTrack.$shared_lib_ext
        set ::dcmtk_TEST_FILE $::dcmtk_DIR/dcmnet/libsrc/libdcmnet.a
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/BatchMake
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a

    }
    "Windows NT" {
    # Windows NT currently covers WinNT, Win2000, XP Home, XP Pro

        set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
        set ::CMAKE_CXX_FLAGS_DEBUG ""
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin/$::VTK_BUILD_TYPE

        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh84.exe
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish84.exe
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/itcl3.2/itcl32.dll
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.2/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/BLT24.dll
        set ::PYTHON_TEST_FILE $::Slicer3_LIB/python-build/bin/python
        set ::MATPLOTLIB_TEST_FILE $::Slicer3_LIB/python-build/bin/bar
        set ::NUMPY_TEST_FILE $::Slicer3_LIB/python-build/lib/python2.5/site-packages/numpy/lib/_compiled_base.dll
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu.exe
        set ::VTK_TEST_FILE $::VTK_DIR/bin/$::VTK_BUILD_TYPE/vtk.exe
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/$::env(VTK_BUILD_SUBDIR)/KWWidgets.lib
        set ::SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/$::VTK_BUILD_TYPE/SlicerClustering.lib
        set ::ALT_SANDBOX_TEST_FILE $::SANDBOX_BIN_DIR/$::VTK_BUILD_TYPE/SlicerClustering.lib
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
        set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/$::VTK_BUILD_TYPE/ITKCommon.dll
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/$::VTK_BUILD_TYPE/IGSTK.lib
        set ::NaviTrack_TEST_FILE $::NaviTrack_DIR/$::VTK_BUILD_TYPE//libNaviTrack.lib
        set ::dcmtk_TEST_FILE $::dcmtk_DIR/bin/$::VTK_BUILD_TYPE/libdcmdata.lib
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/$::VTK_BUILD_TYPE/BatchMake.lib
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/$::VTK_BUILD_TYPE/slicerlibcurl.lib

    }
    default {
        puts stderr "Could not match platform \"$::tcl_platform(os)\"."
        exit
    }
}

# System dependent variables

switch $::tcl_platform(os) {
    "SunOS" {
        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/local/os/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set ::MAKE "gmake"
        set ::SERIAL_MAKE "gmake"
    }
    "Linux" {
        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set numCPUs [lindex [exec grep processor /proc/cpuinfo | wc] 0]
        set ::MAKE "make -j [expr $numCPUs * 2]"
        set ::SERIAL_MAKE "make"
    }
    "Darwin" {
        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.dylib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.dylib
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set numCPUs [exec sysctl -n hw.ncpu ]
        set ::MAKE "make -j [expr $numCPUs * 2]"
        set ::SERIAL_MAKE "make"}
    default {
        # different windows machines say different things, so assume
        # that if it doesn't match above it must be windows
        # (VC7 is Visual C++ 7.0, also known as the .NET version)


        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBase.lib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBaseTCL.lib

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
        # - automatically use newer if available
        #
        if { [file exists "c:/Program Files/Microsoft Visual Studio .NET 2003/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 7 .NET 2003"
            set ::MAKE "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET 2003/Common7/IDE/devenv"
            set ::COMPILER_PATH "c:/Program\ Files/Microsoft\ Visual\ Studio\ .NET 2003/Vc7/bin"
        }

        #
        ## for Visual Studio 8
        # - automatically use newest if available
        # - use full if available, otherwise express
        #
        if { [file exists "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/VCExpress.exe"] } {
            set ::GENERATOR "Visual Studio 8 2005"
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/VCExpress.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 8/VC/bin"
        }


        if { [file exists "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 8 2005"
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 8/VC/bin"
        }
        #
        ## for Visual Studio 9
        if { [file exists "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"] } {
            set ::GENERATOR "Visual Studio 9 2008" 
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 9.0/VC/bin"
        
        }

        if { [file exists "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 8 2005" 
            set ::MAKE "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files (x86)/Microsoft Visual Studio 8/VC/bin"
        }


        set ::COMPILER "cl"
        set ::SERIAL_MAKE $::MAKE
    }
}
