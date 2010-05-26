#
# Note: this local vars file overrides sets the default environment for :
#   BuildScripts/genlib.tcl -- make all the support libs
#   BuildScripts/cmaker.tcl -- makes slicer code
#   launch.tcl -- sets up the runtime env and starts slicer
#   BuildScripts/tarup.tcl -- makes a tar.gz files with all the support files
#
# - use this file to set your local environment and then your change won't
#   be overwritten when those files are updated through CVS
#


## variables that are the same for all systems
set ::PACKAGE_DATA_ROOT ""
set ::LD_FLAGS ""
if {[info exists ::env(PACKAGE_HOME)]} {
    # already set by the launcher
    set ::PACKAGE_HOME $::env(PACKAGE_HOME)
} else {
    # if sourcing this into cmaker, PACKAGE_HOME may not be set
    # set the PACKAGE_HOME directory to the one in which this script resides
    set script [info script]
    catch {set script [file normalize $script]}
    set ::PACKAGE_HOME [file dirname [file dirname $script]]
    set cwd [pwd]
    set ::env(PACKAGE_HOME) $::PACKAGE_HOME
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
        set ::PACKAGE_HOME [file attributes $::PACKAGE_HOME -shortname]
        set ::env(PACKAGE_HOME) $::PACKAGE_HOME
    }
}

puts stderr "PACKAGE_HOME is $::PACKAGE_HOME"

# Choose which library versions you want to compile against.  These
# shouldn't be changed between releases except for testing purposes.
# If you change them and Slicer breaks, you get to keep both pieces.
#
# When modifying these variables, make sure to make appropriate
# changes in the "Files to test if library has already been built"
# section below, or genlib will happily build the library again.

set ::CMAKE_TAG "CMake-2-8-0"
#set ::ITK_CABLE_DATE "CVS"
### NOTE:  2010-04-16 corresponds to tagging 3.18 of ITK
set ::ITK_CABLE_DATE "2010-04-16"
#set ::KWWidgets_DATE "2010-03-27"
set ::KWWidgets_TAG "Slicer-3-6"
### NOTE 2010-03-27 corresponds to the tagging of VTK 5.6
#set ::VTK_DATE "2010-03-27"
set ::VTK_TAG "VTK-5-6"
set ::vtkinria3d_TAG "2010-03-27"
set ::VTK_VERSION "5.6"
set ::fltk_VERSION "1.3"

set ::BRAINS_TAG "https://www.psychiatry.uiowa.edu/svn/code/BRAINS/trunk"

if {[catch {exec which cmake} ::LOCAL_CMAKE] == 0} {
  set cmake_version [exec $::LOCAL_CMAKE --version]
  set patch_level [lindex $cmake_version 2]
} else {
  set cmake_version 0
  set patch_level 0
}
puts "|$cmake_version|$patch_level|"
if { [string compare $::BUILD_PACKAGE(cmake) "yes" ] != 0 &&
     [string compare $patch_level "2.8.0"] == 0 } {
    set ::CMAKE $::LOCAL_CMAKE
    set ::CMAKE_PATH [file dirname [ file dirname $::CMAKE ] ]
    set ::USE_SYSTEM_CMAKE "YES"
    puts "No need to build CMAKE, just use system version: $::CMAKE"
} else {
    set ::CMAKE_PATH $::PACKAGE_LIB/CMake-build
    set ::CMAKE $::CMAKE_PATH/bin/cmake
    set ::USE_SYSTEM_CMAKE "NO"
    puts "Need to build local version of cmake: $::CMAKE"
}


set ::VTK_DIR  $::PACKAGE_BUILD
set ::VTK_SRC_DIR $::PACKAGE_LIB/VTK
if { ![info exists ::VTK_BUILD_TYPE] } {
  # set a default if it hasn't already been specified
  set ::VTK_BUILD_TYPE "Debug" ;# options: Release, RelWithDebInfo, Debug
}

set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
set ::env(VTK_BUILD_TYPE) $::VTK_BUILD_TYPE
set ::KWWidgets_BUILD_DIR  $::PACKAGE_LIB/KWWidgets-build
set ::KWWIDGETS_DIR  $::PACKAGE_BUILD/lib/KWWidgets
set ::ITK_BINARY_PATH $::PACKAGE_BUILD/lib/InsightToolkit
set ::VTK_BINARY_PATH $::PACKAGE_BUILD/lib/vtk-$::VTK_VERSION
set ::TCL_BIN_DIR $::PACKAGE_BUILD/bin
set ::TCL_LIB_DIR $::PACKAGE_BUILD/lib
set ::TCL_INCLUDE_DIR $::PACKAGE_BUILD/include
set ::PYTHON_BIN_DIR $::PACKAGE_BUILD/bin
set ::SOV_BINARY_DIR ""
set ::XVNC_EXECUTABLE " "
set ::vtkinria3d_BUILD_DIR $::PACKAGE_LIB/vtkinria3d-build
set ::vtkinria3d_BINARY_DIR $::PACKAGE_BUILD/lib
set ::FFTW_LIB_DIR $::PACKAGE_BUILD/lib
set ::FFTW_INCLUDE_DIR $::PACKAGE_BUILD/include

switch $::tcl_platform(os) {
    "SunOS" -
    "Linux" {
        set shared_lib_ext "so"
        set static_lib_ext "a"
    }
    "Darwin" {
        set shared_lib_ext "dylib"
        set static_lib_ext "a"
    }
    "Windows NT" {
        set shared_lib_ext "dll"
        set static_lib_ext "lib"
    }
}

#
# getbuildtest.tcl gets the command line variable '--shared' to indicate
# that you want to build shared. But when genlib.tcl gets run, it doesn't get
# run in the same tcl image, and doesn't get the tcl variables set in getbuildtest.tcl.
# and yet we need them in genlib.tcl, so getbuildtest.tcl adds this one to the library
#
if { [ info exists env(BUILD_SHARED_LIBS) ] } {
  set ::BUILD_SHARED_LIBS $env(BUILD_SHARED_LIBS)
} else {
  set ::BUILD_SHARED_LIBS "OFF" # no good default for this.
}
if { [ info exists env(ADD_WRAPITK) ] } {
    set ::ADD_WRAPITK $env(ADD_WRAPITK)
} else {
    set ::ADD_WRAPITK "OFF"
}
if { [ string compare $::BUILD_SHARED_LIBS "OFF" ] == 0 } {
  set shared_lib_ext "a"
  set ::TCL_SHARED_FLAGS "--disable-shared"
} else {
    set ::TCL_SHARED_FLAGS "--enable-shared"
}

# TODO: identify files for each platform
set ::VTK_BUILD_SUBDIR ""

switch $::tcl_platform(os) {
    "SunOS" -
    "Darwin" -
    "Linux" {
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh8.5
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish8.5
        set ::VTK_TEST_FILE $::PACKAGE_BUILD/bin/vtk
        set ::KWWidgets_TEST_FILE $::PACKAGE_BUILD/lib/KWWidgets/libKWWidgets.$shared_lib_ext
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/libtcl8.5.$shared_lib_ext
        set ::VTK_TK_LIB $::TCL_LIB_DIR/libtk8.5.$shared_lib_ext
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh8.5
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/libITKCommon.$shared_lib_ext
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::vtkinria3d_TEST_FILE $::PACKAGE_BUILD/lib/libvtkRenderingAddOn.$shared_lib_ext
        set ::fltk_TEST_FILE $::PACKAGE_BUILD/lib/libfltk.$shared_lib_ext
        set ::FFTW_TEST_FILE $::PACKAGE_BUILD/lib/libfftw3f.$shared_lib_ext
        set ::QT_TEST_FILE $::PACKAGE_BUILD/bin/qmake
    }
    "Windows NT" {
    # Windows NT currently covers WinNT, Win2000, XP Home, XP Pro
        set ::VTK_BUILD_SUBDIR $::VTK_BUILD_TYPE
        set ::env(VTK_BUILD_SUBDIR) $::VTK_BUILD_SUBDIR
        set ::TCL_TEST_FILE $::TCL_BIN_DIR/tclsh84.exe
        set ::TK_TEST_FILE  $::TCL_BIN_DIR/wish84.exe
        set ::VTK_TEST_FILE $::VTK_DIR/bin/$::VTK_BUILD_TYPE/vtk.exe
        set ::KWWidgets_TEST_FILE $::KWWidgets_BUILD_DIR/bin/$::env(VTK_BUILD_SUBDIR)/KWWidgets.lib
        set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
        set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
        set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe
        set ::ITK_TEST_FILE $::ITK_BINARY_PATH/bin/$::VTK_BUILD_TYPE/ITKCommon.dll
        set ::vtkinria3d_TEST_FILE $::vtkinria3d_BUILD_DIR/bin
        set ::fltk_TEST_FILE $::PACKAGE_BUILD/lib/libfltk.$shared_lib_ext
    }
    default {
        puts stderr "Could not match platform \"$::tcl_platform(os)\"."
        exit
    }
}

# System dependent variables
## Initialize them
    set ::CMAKE_SHARED_LINKER_FLAGS ""
    set ::CMAKE_MODULE_LINKER_FLAGS ""
    set ::CMAKE_EXE_LINKER_FLAGS ""

#HACK:  NOTE THIS ONLY WORKS AT UI
if { [ catch { set vgrind [ exec which valgrind ] } ]  == 0 } {
  set ::VALGRIND_EXEC "$vgrind"
} else {
  # absolute paths kind of useless, but ...
  set ::VALGRIND_EXEC "/raid0/homes/hjohnson/local/bin/valgrind"
  # set ::VALGRIND_EXEC ""
}
puts "VALGRIND_EXEC = $::VALGRIND_EXEC"


set ::VALGRIND_COMMANDS_FLAGS "--suppressions=$::PACKAGE_HOME/BuildScripts/ValgrindSuppression.supp --verbose --num-callers=40 --leak-check=full --leak-resolution=high --show-reachable=yes"

set COMMON_WARNING_FLAGS " -Wall -W -Wcast-qual -Wwrite-strings "
# set COMMON_WARNING_FLAGS " -Wall -W -Wshadow -Wcast-qual -Wwrite-strings "

switch $::tcl_platform(os) {
    "SunOS" {
        set ::VTKSLICERBASE_BUILD_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/local/os/bin"
        set ::C_COMPILER "gcc"
        set ::COMPILER "g++"
        set ::MAKE "gmake"
        set ::SERIAL_MAKE "gmake"

        switch $env(PACKAGE_BUILD_TYPE) {
        "DEBUG" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-UNDEBUG -g -UITKIO_DEPRECATED_METADATA_ORIENTATION"
              set ::CXXFLAGS "-UNDEBUG -g -UITKIO_DEPRECATED_METADATA_ORIENTATION"
    set ::OPENGL_INCLUDE_DIR "/usr/include"
    set ::OPENGL_gl_LIBRARY "/usr/lib/libGL.$shared_lib_ext"
    set ::OPENGL_glu_LIBRARY "/usr/lib/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
           }
        "DUMMY" {
        }
        default {
          puts "ERROR Invalid abi."
          exit -1;
        }
        }
    }
    "Linux" {
        set ::VTKSLICERBASE_BUILD_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.so
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.so
        set ::GENERATOR "KDevelop3"
        set numCPUs [lindex [exec grep processor /proc/cpuinfo | wc] 0]
        set ::MAKE "make -j [expr $numCPUs ]"
        set ::SERIAL_MAKE "make"

        set ::COMPILER_PATH "/usr/bin"
        if { [catch {exec gcc4} ] } {
            set ::C_COMPILER "gcc"
            set ::COMPILER "g++"
        } else {
            set ::C_COMPILER "gcc4"
            set ::COMPILER "g++4"
        }
        set ::CC "$COMPILER_PATH/$C_COMPILER"
        set ::CXX "$COMPILER_PATH/$COMPILER"
        switch $env(PACKAGE_BUILD_TYPE) {
        "DEBUG" {
              set ::CFLAGS   "-m32 -UNDEBUG -g -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-m32 -UNDEBUG -g -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY "/usr/lib/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "PROFILE_64" {
              # NOTE:  on RHEL4 gcov supports only the 3.4 compiler properly.
              set ::C_COMPILER "gcc"
              set ::COMPILER "g++"
              set ::COMPILER_PATH "/usr/bin"
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-UNDEBUG -fprofile-arcs -ftest-coverage -pg -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -fprofile-arcs -ftest-coverage -pg -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "PROFILE" {
              # NOTE:  on RHEL4 gcov supports only the 3.4 compiler properly.
              set ::C_COMPILER "gcc"
              set ::COMPILER "g++"
              set ::COMPILER_PATH "/usr/bin"
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-m32 -UNDEBUG -fprofile-arcs -ftest-coverage -pg -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-m32 -UNDEBUG -fprofile-arcs -ftest-coverage -pg -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY "/usr/lib/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
              ## Should be found be default with proper compiler flags ::env(LDFLAGS) "-lgcov"
          }
        "FAST" {
              set ::CFLAGS   "-m32 -DNDEBUG -O2 -msse -mmmx -msse2 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-m32 -DNDEBUG -O2 -msse -mmmx -msse2 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "FAST_64" {
              set ::CFLAGS   "-DNDEBUG -O2 -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-DNDEBUG -O2 -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "FASTO3_64" {
              set ::CFLAGS   "-DNDEBUG -O3 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-DNDEBUG -O3 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "DEBUG_64" {
              set ::CFLAGS   "-UNDEBUG -g -Wall -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -g -Wall -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        "DEBUG2_64" {
              set ::CFLAGS   "-UNDEBUG -g -Wall -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -g -Wall -msse -mmmx -msse2 -m64 -UITKIO_DEPRECATED_METADATA_ORIENTATION ${COMMON_WARNING_FLAGS}"
              set ::OPENGL_INCLUDE_DIR "/usr/include"
              set ::OPENGL_gl_LIBRARY  "/usr/lib64/libGL.$shared_lib_ext"
              set ::OPENGL_glu_LIBRARY "/usr/lib64/libGLU.$shared_lib_ext"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
          }
        default {
          puts "ERROR Invalid abi."
          exit -1;
        }
        }
    }
    "Darwin" {
        set ::VTKSLICERBASE_BUILD_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBase.dylib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/vtkSlicerBaseTCL.dylib
        set ::GENERATOR "Unix Makefiles"
        set ::COMPILER_PATH "/usr/bin"
        set VECTOR_OPT "-O2"
        if { [ catch { exec gcc-4.2 --version } ] == 0 } {
            set ::C_COMPILER "gcc-4.2"
            set ::COMPILER "g++-4.2"
            set VECTOR_OPT "-O3 -fast -ftree-vectorize"
        } elseif { [ catch { exec gcc-4.0 --version } ] == 0 } {
            set ::C_COMPILER "gcc-4.0"
            set ::COMPILER "g++-4.0"
        } else {
            set ::C_COMPILER "gcc"
            set ::COMPILER "g++"
        }
        puts "Using $::COMPILER & $::C_COMPILER"
        set numCPUs [exec sysctl -n hw.ncpu ]
        set ::MAKE "make -j [expr $numCPUs ]"
#        set ::MAKE "make"
        set ::SERIAL_MAKE "make"
        set ::LD_FLAGS "-framework corefoundation"
        set ::env(LDFLAGS) $::LD_FLAGS

        set OSX_VERSION  [ lindex [ split [exec uname -r ] "." ] 0 ]
        switch $OSX_VERSION {
          "8" {
            set ::env(LDFLAGS) ""
          }
          "9" {
            ## OPEN GL under Leopard requires this link line in order to resolve the GL libraries correctly.
            ## set ::env(LDFLAGS) "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
            set ::CMAKE_SHARED_LINKER_FLAGS "${LD_FLAGS} -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib"
            set ::CMAKE_MODULE_LINKER_FLAGS "${LD_FLAGS} -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib"
            set ::CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS} -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/libGL.dylib"
          }
        }
        # puts "*************************** USE_CARBON = ${USE_CARBON} *********************"
        if { $::BUILD_PACKAGE(Qt) == "yes" || $BUILD_PACKAGE(SystemQt) } {
          set ::OPENGL_INCLUDE_DIR "/System/Library/Frameworks/OpenGL.framework"
          set ::OPENGL_gl_LIBRARY "/System/Library/Frameworks/OpenGL.framework"
          set ::OPENGL_glu_LIBRARY "/System/Library/Frameworks/AGL.framework"
        } else {
          set ::OPENGL_INCLUDE_DIR "/usr/X11R6/include"
          set ::OPENGL_gl_LIBRARY  "-framework OpenGL;/usr/X11R6/lib/libGL.dylib"
          set ::OPENGL_glu_LIBRARY "-framework OpenGL;/usr/X11R6/lib/libGLU.dylib"
        #   puts "OPENGL SETTINGS $::OPENGL_INCLUDE_DIR $::OPENGL_gl_LIBRARY $::OPENGL_glu_LIBRARY"
        }
        switch $env(PACKAGE_BUILD_TYPE) {
        "DEBUG" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-UNDEBUG -g -Wall -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -g -Wall -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
        }
        "DEBUG_64" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-UNDEBUG -g -Wall -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m64 -arch x86_64 -Wconversion -mmacosx-version-min=10.4 ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -g -Wall -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m64 -arch x86_64 -Wconversion -mmacosx-version-min=10.4 ${COMMON_WARNING_FLAGS}"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
        }
        "FAST" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-DNDEBUG -O2 -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-DNDEBUG -O2 -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
        }
        "FAST_64" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-DNDEBUG ${VECTOR_OPT} -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m64 -arch x86_64 -Wconversion -mmacosx-version-min=10.4 ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-DNDEBUG ${VECTOR_OPT} -msse -mmmx -msse2 -msse3 -UITKIO_DEPRECATED_METADATA_ORIENTATION -m64 -arch x86_64 -Wconversion -mmacosx-version-min=10.4 ${COMMON_WARNING_FLAGS}"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
        }
        "PROFILE" {
              set ::CC "$COMPILER_PATH/$C_COMPILER"
              set ::CXX "$COMPILER_PATH/$COMPILER"
              set ::CFLAGS   "-UNDEBUG -fprofile-arcs -ftest-coverage -pg -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::FORTRAN_FLAGS "$::CFLAGS"
              set ::CXXFLAGS "-UNDEBUG -fprofile-arcs -ftest-coverage -pg -UITKIO_DEPRECATED_METADATA_ORIENTATION -m32 -arch i386 ${COMMON_WARNING_FLAGS}"
              set ::env(CC) $::CC
              set ::env(CXX) $::CXX
              ##  Some of the iwidgets, itcl, itk does not compile with these flags due to poor build tools used.
              set ::env(CFLAGS) $::CFLAGS
              set ::env(CXXFLAGS) $::CXXFLAGS
        }
        default {
          puts "ERROR Invalid abi."
          exit -1;
        }
        }
    }
    default {
        # different windows machines say different things, so assume
        # that if it doesn't match above it must be windows
        # (VC7 is Visual C++ 7.0, also known as the .NET version)


        set ::VTKSLICERBASE_BUILD_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBase.lib
        set ::VTKSLICERBASE_BUILD_TCL_LIB $::PACKAGE_HOME/Base/builds/$::env(BUILD)/bin/$::VTK_BUILD_TYPE/vtkSlicerBaseTCL.lib


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

set ::FULLHOSTNAME [info hostname]
set ::COMPILER_HOSTNAME [regsub "\\.psychiatry\\.uiowa\\.edu" ${FULLHOSTNAME} ""]

    set ::CMAKE_COMPILE_SETTINGS [list "-G$GENERATOR" "-DCMAKE_CXX_COMPILER:FILEPATH=$::CXX" \
"-DCMAKE_CXX_FLAGS:STRING=$::CXXFLAGS" "-DCMAKE_CXX_FLAGS_RELEASE:STRING=$::CXXFLAGS" \
"-DCMAKE_CXX_FLAGS_DEBUG:STRING=$::CXXFLAGS" "-DCMAKE_C_COMPILER:FILEPATH=$::CC" \
"-DCMAKE_C_FLAGS:STRING=$::CFLAGS" \
"-DCMAKE_Fortran_FLAGS:STRING=$::CFLAGS" \
"-DDART_TESTING_TIMEOUT:STRING=600" \
"-DMEMORYCHECK_COMMAND_OPTIONS:STRING=$::VALGRIND_COMMANDS_FLAGS" \
"-DMEMORYCHECK_COMMAND:PATH=$::VALGRIND_EXEC" \
"-DCMAKE_SHARED_LINKER_FLAGS:STRING=$::CMAKE_SHARED_LINKER_FLAGS" \
"-DCMAKE_EXE_LINKER_FLAGS:STRING=$::CMAKE_EXE_LINKER_FLAGS" \
"-DCMAKE_MODULE_LINKER_FLAGS:STRING=$::CMAKE_MODULE_LINKER_FLAGS" \
"-DSITE:STRING=$::COMPILER_HOSTNAME.uiowa" \
"-DBUILDNAME:STRING=$::tcl_platform(os).$env(PACKAGE_BUILD_TYPE)"]

     set ::OPENGL_COMPILE_SETTINGS [ list "-DOPENGL_gl_LIBRARY:FILEPATH=$::OPENGL_gl_LIBRARY" \
"-DOPENGL_glu_LIBRARY:FILEPATH=$::OPENGL_glu_LIBRARY"  \
"-DOPENGL_INCLUDE_DIR:PATH=$::OPENGL_INCLUDE_DIR" ];

set QT_CMAKE_FLAGS ""
puts "build_packag(Qt)=${::BUILD_PACKAGE(Qt)} build_package(SystemQt)=${::BUILD_PACKAGE(SystemQt)}"
if { $::BUILD_PACKAGE(Qt) == "yes" || $BUILD_PACKAGE(SystemQt) == "yes" } {
set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DVTK_USE_QT:BOOL=ON"
set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DVTK_USE_QVTK:BOOL=ON"
set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DVTK_USE_GUISUPPORT:BOOL=ON"
set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DDESIRED_QT_VERSION:STRING=4"
set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DVTK_USE_QVTK_QTOPENGL:BOOL=ON"
    if { $BUILD_PACKAGE(SystemQt) } {
        if { [ catch { set QT_QMAKE_EXECUTABLE [ exec which qmake ] } ] } {
            puts "Qt is required but not installed on this system"
            puts "Add --build-Qt to the getbuildtest.tcl command line"
            puts "or install Qt -- see http://qt.nokia.com"
            exit 1
        }

        set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
        set QTDIR [ lindex [ split [ lindex [ exec qmake -query ] 4 ] : ] 1 ]
        set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DQT_SEARCH_PATH:FILEPATH=${QTDIR}"
    } else {
        set QTDIR $::PACKAGE_BUILD
        set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DQT_SEARCH_PATH:FILEPATH=${QTDIR}"
        set QT_CMAKE_FLAGS "$QT_CMAKE_FLAGS -DQT_QMAKE_EXECUTABLE:FILEPATH=${QTDIR}/bin/qmake"
    }
}
puts "in package_variables QT_CMAKE_FLAGS=%%${QT_CMAKE_FLAGS}%%"

## puts $::CMAKE_COMPILE_SETTINGS
##     puts $::OPENGL_COMPILE_SETTINGS

