
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

puts "Slicer3_HOME is $::Slicer3_HOME"

# Choose which library versions you want to compile against.  These
# shouldn't be changed between releases except for testing purposes.
# If you change them and Slicer breaks, you get to keep both pieces.
#
# When modifying these variables, make sure to make appropriate
# changes in the "Files to test if library has already been built"
# section below, or genlib will happily build the library again.

set ::Slicer3_TAG "http://svn.slicer.org/Slicer3/trunk"
set ::CMAKE_TAG "CMake-2-6"
set ::Teem_TAG http://teem.svn.sourceforge.net/svnroot/teem/teem/tags/1.10.0
set ::KWWidgets_TAG "Slicer-3-4"
set ::VTK_TAG "VTK-5-2"
set ::ITK_TAG ITK-3-10
set ::PYTHON_TAG "http://svn.python.org/projects/python/branches/release25-maint"
set ::BLAS_TAG http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/BLAS
set ::LAPACK_TAG http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/lapack-3.1.1
#set ::NUMPY_TAG "http://svn.scipy.org/svn/numpy/trunk"
set ::NUMPY_TAG "http://svn.scipy.org/svn/numpy/branches/1.2.x"
#set ::SCIPY_TAG "http://svn.scipy.org/svn/scipy/trunk"
set ::SCIPY_TAG "http://svn.scipy.org/svn/scipy/branches/0.7.x"
set ::BatchMake_TAG "BatchMake-1-0-8"
set ::SLICERLIBCURL_TAG "HEAD"
set ::OpenIGTLink_TAG "http://svn.na-mic.org/NAMICSandBox/branches/OpenIGTLink-1-0"
# set TCL_VERSION to "tcl" to get 8.4, otherwise use tcl85 get 8.5
set ::TCL_VERSION tcl
#set ::TCL_VERSION tcl85

# Set library, binary, etc. paths...

# if Slicer3_LIB and Slicer3_BUILD haven't been set, first check
# enviornment variables, then assume they are in the 'standard' places
# next to the source tree (as created by getbuildtest.tcl
if { ![info exists ::Slicer3_LIB] } {
    if { [info exists ::env(Slicer3_LIB)] } {
        set ::Slicer3_LIB $::env(Slicer3_LIB)
    } else {
        set wd [pwd]
        cd $::Slicer3_HOME/../Slicer3-lib
        set ::Slicer3_LIB [pwd]
        cd $wd
    }
}
if { ![info exists ::Slicer3_BUILD] } {
    if { [info exists ::env(Slicer3_BUILD)] } {
        set ::Slicer3_BUILD $::env(Slicer3_BUILD)
    } else {
        set wd [pwd]
        cd $::Slicer3_HOME/../Slicer3-build
        set ::Slicer3_BUILD [pwd]
        cd $wd
    }
}

set ::Teem_SRC_DIR  $::Slicer3_LIB/teem
set ::Teem_BUILD_DIR  $::Slicer3_LIB/teem-build
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
set ::OpenIGTLink_DIR $::Slicer3_LIB/OpenIGTLink-build 
set ::BatchMake_SRC_DIR $::Slicer3_LIB/BatchMake
set ::BatchMake_BUILD_DIR $::Slicer3_LIB/BatchMake-build
set ::SLICERLIBCURL_SRC_DIR $::Slicer3_LIB/cmcurl
set ::SLICERLIBCURL_BUILD_DIR $::Slicer3_LIB/cmcurl-build


# getbuildtest Option for build using system Python, should be "true" or "false"
set ::USE_SYSTEM_PYTHON "false"
# CMake option for Python, must be "OFF" on "ON", default is "ON"
set ::USE_PYTHON "ON"

# CMake option for numerical Python, only matters if Python is on
set ::USE_NUMPY "ON"
# getbuildtest option for SCIPY - also build support libraries (blas and lapack) needed for scipy
# - should be off except for experimentation (does not work on all plaftorms - requires fortran)
# - should be "true" or "false"
set ::USE_SCIPY "false"
# CMake Option for using OpenIGTLink library. Must be "OFF" or "ON", default is "OFF"
set ::USE_OPENIGTLINK "ON"
 

switch $::tcl_platform(os) {
    "SunOS" {
        set shared_lib_ext "so"
    }
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
    "SunOS" {
        set ::VTK_BUILD_SUBDIR ""
        set ::Teem_BIN_DIR  $::Teem_BUILD_DIR/bin

        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/libitcl3.2.so
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/libitk3.2.so

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        if { $::USE_SYSTEM_PYTHON } {
          error "need to define system python path for $::tcl_platform(os)"
        }
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/lib/libpython2.5.so
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include/python2.5
        set ::NETLIB_TEST_FILE $::Slicer3_LIB/netlib-build/BLAS-build/libblas.a
        set ::NUMPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/numpy/core/numeric.pyc
        set ::SCIPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/scipy/version.pyc
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitcl3.2.so
        set ::Teem_TEST_FILE $::Teem_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.so
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::OPENIGTLINK_TEST_FILE $::OpenIGTLink_DIR/bin/libOpenIGTLink.$shared_lib_ext
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/BatchMake
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.$shared_lib_ext
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24

        # We need a workaround here, because gcc does not pick up libraries in /lib
        # on Solaris. 
        # The bug, and it's resolution can be found here:
        # http://bugs.opensolaris.org/view_bug.do?bug_id=6619485
        set ::env(LDFLAGS) -L/lib
    }
    "Darwin" {
        set ::VTK_BUILD_SUBDIR ""
        set ::Teem_BIN_DIR  $::Teem_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/libitcl3.2.dylib
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/libitk3.2.dylib
        if { $::USE_SYSTEM_PYTHON } {
          set ::PYTHON_BIN_DIR /usr
        }
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/lib/libpython2.5.dylib
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include/python2.5
        set ::NETLIB_TEST_FILE $::Slicer3_LIB/netlib-build/BLAS-build/libblas.a
        set ::NUMPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/numpy/core/numeric.pyc
        set ::SCIPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/scipy/version.pyc
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitcl3.2.dylib
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        set ::Teem_TEST_FILE $::Teem_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.$shared_lib_ext
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::OPENIGTLINK_TEST_FILE $::OpenIGTLink_DIR/bin/libOpenIGTLink.$shared_lib_ext
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/BatchMake
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a

    }
    "Linux" {
        set ::VTK_BUILD_SUBDIR ""
        set ::Teem_BIN_DIR  $::Teem_BUILD_DIR/bin

        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.4
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/libitcl3.2.so
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/libitk3.2.so
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.1/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/bltwish24
        if { $::USE_SYSTEM_PYTHON } {
          error "need to define system python path for $::tcl_platform(os)"
        }
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/lib/libpython2.5.so
        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include/python2.5
        set ::NETLIB_TEST_FILE $::Slicer3_LIB/netlib-build/BLAS-build/libblas.a
        set ::NUMPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/numpy/core/numeric.pyc
        set ::SCIPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/scipy/version.pyc
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.4
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/libitcl3.2.so
        set ::Teem_TEST_FILE $::Teem_BIN_DIR/unu
        set ::VTK_TEST_FILE $::VTK_DIR/bin/vtk
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/libKWWidgets.so
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.4.$shared_lib_ext 
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.4.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.4
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/libITKCommon.$shared_lib_ext
        set ::TK_EVENT_PATCH $::Slicer3_HOME/tkEventPatch.diff
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::OPENIGTLINK_TEST_FILE $::OpenIGTLink_DIR/bin/libOpenIGTLink.$shared_lib_ext
        set ::BatchMake_TEST_FILE $::BatchMake_BUILD_DIR/bin/BatchMake
        set ::SLICERLIBCURL_TEST_FILE $::SLICERLIBCURL_BUILD_DIR/bin/libslicerlibcurl.a

    }
    "Windows NT" {
    # Windows NT currently covers WinNT, Win2000, XP Home, XP Pro

        set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
        set ::Teem_BIN_DIR  $::Teem_BUILD_DIR/bin/$::VTK_BUILD_TYPE

        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh84.exe
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish84.exe
        set ::ITCL_TEST_FILE $::TCL_LIB_DIR/itclConfig.sh
        set ::INCR_TCL_LIB $::TCL_LIB_DIR/lib/itcl3.2/itcl32.lib
        set ::INCR_TK_LIB $::TCL_LIB_DIR/lib/itk3.2/itk32.lib
        set ::IWIDGETS_TEST_FILE $::TCL_LIB_DIR/iwidgets4.0.2/iwidgets.tcl
        set ::BLT_TEST_FILE $::TCL_BIN_DIR/BLT24.dll
        set ::Teem_TEST_FILE $::Teem_BIN_DIR/unu.exe
        if { $::USE_SYSTEM_PYTHON } {
          error "need to define system python path for $::tcl_platform(os)"
        }
        #set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/bin/python.exe
        #set ::PYTHON_LIB $::PYTHON_BIN_DIR/Libs/python25.lib
        set ::PYTHON_TEST_FILE $::PYTHON_BIN_DIR/PCbuild/python.exe
        set ::PYTHON_LIB $::PYTHON_BIN_DIR/PCbuild/python25.lib

        set ::PYTHON_INCLUDE $::PYTHON_BIN_DIR/include

        set ::NETLIB_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/numpy/core/numeric.pyc
        set ::NUMPY_TEST_FILE $::PYTHON_BIN_DIR/lib/site-packages/numpy/core/numeric.pyc
        set ::SCIPY_TEST_FILE $::PYTHON_BIN_DIR/lib/python2.5/site-packages/scipy/version.pyc
        set ::VTK_TEST_FILE $::VTK_DIR/bin/$::VTK_BUILD_TYPE/vtk.exe
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/$::env(VTK_BUILD_SUBDIR)/KWWidgets.lib
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
        set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/$::VTK_BUILD_TYPE/ITKCommon.dll
        set ::OPENIGTLINK_TEST_FILE $::OpenIGTLink_DIR/bin/$::VTK_BUILD_TYPE/OpenIGTLink.lib
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
        set ::COMPILER_PATH "/usr/sfw/bin"
        set ::COMPILER "g++"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set numCPUs [lindex [exec /usr/sbin/psrinfo | grep on-line | wc -l | tr -d ''] 0]
        set ::MAKE "gmake -j[expr $numCPUs]"        
        set ::SERIAL_MAKE "gmake"
    }
    "Linux" {
        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles" 
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++"
        set ::FORTRAN_COMPILER "gfortran"
        set ::CMAKE $::CMAKE_PATH/bin/cmake
        set numCPUs [lindex [exec grep processor /proc/cpuinfo | wc] 0]
        set ::MAKE "make -j [expr $numCPUs]"
        set ::SERIAL_MAKE "make"
    }
    "Darwin" {
        set ::VTKSLICERBASE_BUILD_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.dylib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::Slicer3_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.dylib
        set ::GENERATOR "Unix Makefiles" 
        set ::COMPILER_PATH "/usr/bin"
        set ::COMPILER "g++"
        set ::FORTRAN_COMPILER "gfortran"
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
            set ::MSSDK_PATH "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\PlatformSDK"
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
            set ::MSSDK_PATH "C:\Program Files\Microsoft Platform SDK"
        }


        if { [file exists "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 8 2005" 
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 8/VC/bin"
            set ::MSSDK_PATH "C:\Program Files\Microsoft Platform SDK"
        }

        if { [file exists "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"] } {
            #set ::GENERATOR "Visual Studio 8 2005 Win64"
            set ::GENERATOR "Visual Studio 8 2005"   ;# do NOT use the 64 bit target
            set ::MAKE "c:/Program Files (x86)/Microsoft Visual Studio 8/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files (x86)/Microsoft Visual Studio 8/VC/bin"
            error "*****\n MSSDK_PATH value not known for this compiler - \nLook at settings for other Visual Studio 2005 configuration for possible locations of the SDK include and library files.  \ncontact slicer-devel@bwh.harvard.edu for more instructions or to report correct path for this compiler."
        }
        #
        ## for Visual Studio 9
        if { [file exists "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"] } {
            set ::GENERATOR "Visual Studio 9 2008" 
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/VCExpress.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 9.0/VC/bin"
            set ::MSSDK_PATH "c:/Program Files/Microsoft SDKs/Windows/v6.0A"
        }

        if { [file exists "c:/Program Files (x86)/Microsoft Visual Studio 9.0/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 9 2008"
            set ::MAKE "c:/Program Files (x86)/Microsoft Visual Studio 9.0/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/bin"
            set ::MSSDK_PATH "c:/Program Files/Microsoft SDKs/Windows/v6.0A"
        }


        if { [file exists "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/devenv.exe"] } {
            set ::GENERATOR "Visual Studio 9 2008"
            set ::MAKE "c:/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/devenv.exe"
            set ::COMPILER_PATH "c:/Program Files/Microsoft Visual Studio 9.0/VC/bin"
            set ::MSSDK_PATH "c:/Program Files/Microsoft SDKs/Windows/v6.0A"
        }

        set ::COMPILER "cl"
        set ::SERIAL_MAKE $::MAKE
    }
}



