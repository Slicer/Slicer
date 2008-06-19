
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

set ::Slicer3_TAG "http://www.na-mic.org/svn/Slicer3/trunk"
set ::CMAKE_TAG "CMake-2-6"
set ::TEEM_TAG "Teem-1-9-0-patches"
set ::KWWidgets_TAG "Slicer-3-2"
set ::VTK_TAG "VTK-5-2"
set ::ITK_TAG ITK-3-6
set ::PYTHON_TAG "http://svn.python.org/projects/python/branches/release25-maint"
set ::SLICERLIBCURL_TAG "HEAD"

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
if { ![info exists ::VTK_BUILD_TYPE] } {
  # set a default if it hasn't already been specified
  set ::VTK_BUILD_TYPE "Debug" ;# options: Release, RelWithDebInfo, Debug
}
set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE 
set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
set ::KWWidgets_BUILD_DIR  $::Slicer3_LIB/KWWidgets-build
set ::KWWIDGETS_DIR  $::Slicer3_LIB/KWWidgets
set ::ITK_BINARY_PATH $::Slicer3_LIB/Insight-build
set ::TCL_BIN_DIR $::Slicer3_LIB/tcl-build/bin
set ::TCL_LIB_DIR $::Slicer3_LIB/tcl-build/lib
set ::TCL_INCLUDE_DIR $::Slicer3_LIB/tcl-build/include
set ::PYTHON_BIN_DIR $::Slicer3_LIB/python-build
set ::CMAKE_PATH $::Slicer3_LIB/CMake-build
set ::SOV_BINARY_DIR ""
set ::XVNC_EXECUTABLE " "
set ::IGSTK_DIR $::Slicer3_LIB/IGSTK-build 
set ::SLICERLIBCURL_SRC_DIR $::Slicer3_LIB/cmcurl
set ::SLICERLIBCURL_BUILD_DIR $::Slicer3_LIB/cmcurl-build


# Options for building IGT modules in Slicer
set ::IGSTK "OFF"
set ::NAVITRACK "OFF"

# The absolute path and directory containing the navitrack library,
# for instance on linux the libNaviTrack.so
# set ::NAVITRACK_LIB_DIR /home/hliu/projects/navitrack/NaviTrack-build
set ::NAVITRACK_LIB_DIR "" 

# The navitrack include directry, e.g.
# /home/hliu/projects/navitrack/NaviTrack/include
# set ::NAVITRACK_INC_DIR /home/hliu/projects/navitrack/NaviTrack/include
set ::NAVITRACK_INC_DIR "" 

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
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/libitcl3.2.dylib
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/libitk3.2.dylib
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/lib/libpython2.5.dylib
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include/python2.5
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitcl3.2.dylib
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.$shared_lib_ext
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/libIGSTK.$shared_lib_ext
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a

    }
    "Linux" {
        set ::VTK_BUILD_SUBDIR ""
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/libitcl3.2.so
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/libitk3.2.so
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/lib/libpython2.5.so
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include/python2.5
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitcl3.2.so
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.so
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/libIGSTK.$shared_lib_ext
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a

    }
    "Windows NT" {
    # Windows NT currently covers WinNT, Win2000, XP Home, XP Pro

        set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
        set ::TEEM_BIN_DIR  $::TEEM_BUILD_DIR/bin/$::VTK_BUILD_TYPE

        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh84.exe
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish84.exe
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/itclConfig.sh
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/itcl3.2/itcl32.lib
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/itk3.2/itk32.lib
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.2/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/BLT24.dll
        set ::TEEM_TEST_FILE $::TEEM_BIN_DIR/unu.exe
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python.exe
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/Libs/python25.lib
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include
        set ::VTK_TEST_FILE $::VTK_DIR/bin/$::VTK_BUILD_TYPE/vtk.exe
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/$::env(VTK_BUILD_SUBDIR)/KWWidgets.lib
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
        set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/$::VTK_BUILD_TYPE/ITKCommon.dll
        set ::IGSTK_TEST_FILE $::IGSTK_DIR/bin/$::VTK_BUILD_TYPE/IGSTK.lib
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
        set ::MAKE make
        set ::SERIAL_MAKE make
    }
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
        # - use the 64 bit version if available
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

        if { [file exists "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"] } {
            #set ::GENERATOR "Visual Studio 8 2005 Win64"
            set ::GENERATOR "Visual Studio 8 2005"   ;# do NOT use the 64 bit target
            set ::MAKE "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files (x86)/Microsoft Visual Studio 8/VC/bin"
        }
        #
        ## for Visual Studio 9
        if { [file exists "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"] } {
            set ::GENERATOR "Visual Studio 9 2008" 
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 9.0/VC/bin"
        
        }

        set ::COMPILER "cl"
        set ::SERIAL_MAKE $::MAKE
    }
}


