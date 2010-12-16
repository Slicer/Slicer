#!/bin/sh
echo "This script is deprecated, and will probably not work any more."
exit 1
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
# Packages: cmake, tcl, itcl, iwidgets, blt, ITK, VTK, teem
# 
# Usage:
#   genlib [options] [target]
#
# run genlib from the $::PACKAGE_NAME directory next to where you want the packages to be built
# E.g. if you run /home/pieper/$::PACKAGE_NAME/BuildScripts/genlib.tcl it will create
# /home/pieper/Slicer3-lib
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
#
# check to see if need to build a package
# returns 1 if need to build, 0 else
# if { [BuildThis  ""] == 1 } {
proc BuildThis { testFile packageName } {
#    catch { runcmd ls -l $testFile }
    if {![file exists $testFile] || $::GENLIB(update) || [lsearch $::GENLIB(buildList) $packageName] != -1} {
        puts "Building $packageName (testFile = $testFile, update = $::GENLIB(update), buildlist = $::GENLIB(buildList) )"
        return 1
    } else {
        puts "Skipping $packageName"
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
    set msg "$msg\n  \[target\] is the the PACKAGE_LIB directory"
    set msg "$msg\n             and is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   --help : prints this message and exits"
    set msg "$msg\n   --clean : delete the target first"
    set msg "$msg\n   --update : do a cvs update even if there's an existing build"
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   --nobuild : only download and/or update, but don't build"
    set msg "$msg\n   optional space separated list of packages to build (lower case)"
    set msg "$msg\n   --build-cmake : build cmake"
    set msg "$msg\n   --build-tcl : build tcl"
    set msg "$msg\n   --build-tk : build tk"
    set msg "$msg\n   --build-vtk : build vtk"
    set msg "$msg\n   --build-kwwidgets : build kwwidgets"
    set msg "$msg\n   --build-itk : build itk"
    set msg "$msg\n   --build-vtkinria3d : build vtkinria3d"
    set msg "$msg\n   --build-fltk : build fltk"
    set msg "$msg\n   --build-fftw : build fftw"
    puts stderr $msg
}

set GENLIB(clean) "false"
set GENLIB(update) "false"
set GENLIB(buildit) "true"
set ::GENLIB(buildList) ""

set isRelease 0
set strippedargs ""
set argc [llength $argv]

set BUILD_PACKAGE(cmake) "no"
set BUILD_PACKAGE(tcl) "no"
set BUILD_PACKAGE(tk) "no"
set BUILD_PACKAGE(vtk) "no"
set BUILD_PACKAGE(kwwidgets) "no"
set BUILD_PACKAGE(itk) "no"
set BUILD_PACKAGE(vtkinria3d) "no"
set BUILD_PACKAGE(fltk) "no"
set BUILD_PACKAGE(fftw) "no"
set BUILD_PACKAGE(Qt) "no"
set BUILD_PACKAGE(SystemQt) "no"

set FFTW_CMAKE_FLAGS ""
set USE_CARBON "OFF"
set USE_COCOA "OFF"
set USE_X "ON"

set ::CMAKE_BUILD_TYPE "DEBUG"
puts "ARGV=${argv}"
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--build-cmake" {
            set BUILD_PACKAGE(cmake) "yes"
        }
        "--build-tcl" {
            set BUILD_PACKAGE(tcl) "yes"
        }
        "--build-tk" {
            set BUILD_PACKAGE(tk) "yes"
        }
        "--build-vtk" {
            set BUILD_PACKAGE(vtk) "yes"
        }
        "--build-kwwidgets" {
            set BUILD_PACKAGE(kwwidgets) "yes"
        }
        "--build-itk" {
            set BUILD_PACKAGE(itk) "yes"
        }
        "--build-fftw" {
            set BUILD_PACKAGE(fftw) "yes"
        }
        "--build-vtkinria3d" {
            set BUILD_PACKAGE(vtkinria3d) "yes"
        }
        "--build-fltk" {
            set BUILD_PACKAGE(fltk) "yes"
        }
        "--build-Qt" {
            set BUILD_PACKAGE(Qt) "yes"
        }
        "--system-Qt" {
            set BUILD_PACKAGE(SystemQt) "yes"
            puts "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   --system-Qt= $::BUILD_PACKAGE(SystemQt)"
        }
        "--UseCarbon" {
            set USE_CARBON "ON"
            set USE_COCOA "OFF"
            set USE_X "OFF"
# USE_X needed for KWWIDGETS and VTK_TCL_WRAP
        }
        "--UseCocoa" {
            set USE_CARBON "OFF"
            set USE_COCOA   "ON"
            set USE_X "OFF"
        }
        "--clean" -
        "-f" {
            set ::GENLIB(clean) "true"
        }
        "--update" -
        "-u" {
            set ::GENLIB(update) "true"
        }
        "--release" {
            set isRelease 1
            set ::CMAKE_BUILD_TYPE "Release"
        }
        "--nobuild" {
            set ::GENLIB(buildit) "false"
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

set ::PACKAGE_LIB ""
if {$argc > 2 } {
    Usage
    exit 1
    # the stripped args list now has the PACKAGE_LIB first and then the list of packages to build
    set ::GENLIB(buildList) [lrange $strippedargs 1 end]
    set strippedargs [lindex $strippedargs 0]
# puts "Got the list of package to build: '$::GENLIB(buildList)' , stripped args = $strippedargs"
} 
set ::PACKAGE_LIB [lindex $argv 0]
set ::PACKAGE_BUILD [lindex $argv 1]




################################################################################
# First, set up the directory
# - determine the location
# - determine the build
# 

# hack to work around lack of normalize option in older tcl
# set PACKAGE_HOME [file dirname [file dirname [file normalize [info script]]]]
set script [info script]
catch {set script [file normalize $script]}
set ::PACKAGE_HOME [file dirname [file dirname $script]]
set cwd [pwd]
cd $cwd

#######
#
# Note: the local vars file, $PACKAGE_HOME/BuildScripts/package_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't
#   be overwritten when this file is updated
#
set localvarsfile $PACKAGE_HOME/BuildScripts/package_variables.tcl
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
    set ::CMAKE_BUILD_TYPE "Release"
    set ::env(CMAKE_BUILD_TYPE) $::CMAKE_BUILD_TYPE
    if ($isWindows) {
        set ::VTK_BUILD_SUBDIR "Release"
    } else {
        set ::VTK_BUILD_SUBDIR ""
    }
    puts "Overriding package_variables.tcl; CMAKE_BUILD_TYPE is '$::env(CMAKE_BUILD_TYPE)', VTK_BUILD_SUBDIR is '$::VTK_BUILD_SUBDIR'"
    set ::VTK_DEBUG_LEAKS "OFF"

}

# tcl file delete is broken on Darwin, so use rm -rf instead
if { $GENLIB(clean) } {
    puts "Deleting lib files..."
    if { $isDarwin } {
        runcmd rm -rf $PACKAGE_LIB
        if { [file exists $PACKAGE_LIB/tcl/isPatched] } {
            runcmd rm $PACKAGE_LIB/tcl/isPatched
        }

    } else {
        file delete -force $PACKAGE_LIB
    }
}

if { ![file exists $PACKAGE_LIB] } {
    file mkdir $PACKAGE_LIB
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
puts "+++ DEBUG BUILD_PACKAGE(cmake) = $BUILD_PACKAGE(cmake) USE_SYSTEM_CMAKE = $::USE_SYSTEM_CMAKE"
set buildthis [BuildThis $::CMAKE "cmake" ]
puts "*** DEBUG puts Buildthis $buildthis "

if { [string compare $BUILD_PACKAGE(cmake) "yes" ] == 0 } {
if { [string compare $::USE_SYSTEM_CMAKE "NO" ] == 0 } {
if { [BuildThis $::CMAKE "cmake"] == 1 } {
    file mkdir $::CMAKE_PATH
    cd $PACKAGE_LIB
    if { ! [ file executable $::CMAKE ] } {
    if {$isWindows} {
      runcmd $::SVN co http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/CMake-build CMake-build
    } else {
       runcmd $::CVS -d :pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake login
       eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@www.cmake.org:/cvsroot/CMake checkout -r $::CMAKE_TAG CMake"
        if {$::GENLIB(buildit)} {
          cd $::CMAKE_PATH
          if { $isSolaris } {
              # make sure to pick up curses.h in /local/os/include
              runcmd $PACKAGE_LIB/CMake/bootstrap --init=$PACKAGE_HOME/BuildScripts/spl.cmake.init
          } else {
              ## CMAKE IS TERRIBLY SLOW WHEN BUILT IN DEBUG MODE
              set TEMP_CFLAGS $env(CFLAGS);
              set TEMP_CXXFLAGS $env(CXXFLAGS);
              unset env(CFLAGS);
              unset env(CXXFLAGS);
              set env(CXXFLAGS) "-O2"
              set env(CFLAGS) $env(CXXFLAGS)
              runcmd $PACKAGE_LIB/CMake/bootstrap
              set env(CFLAGS) $TEMP_CFLAGS;
              set env(CXXFLAGS) $TEMP_CXXFLAGS;
          }
          eval runcmd $::MAKE
       }
    }
  }
}
}
}


################################################################################
# Get and build tcl, tk
#
#

if { $isDarwin } {
set SAVECFLAGS $::env(CFLAGS)
set SAVECXXFLAGS $::env(CXXFLAGS)
set ::env(CFLAGS) "$::env(CFLAGS) -framework corefoundation"
set ::env(CXXFLAGS) "$::env(CXXFLAGS) -framework corefoundation"
}

# on windows, tcl won't build right, as can't configure, so save commands have to run
if { [string compare $BUILD_PACKAGE(tcl) "yes" ] == 0 } {
if { [BuildThis $::TCL_TEST_FILE "tcl"] == 1 } {

    if {$isWindows} {
      runcmd $::SVN co http://s)vn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build tcl-build
    }

    file mkdir $PACKAGE_LIB/tcl
    cd $PACKAGE_LIB/tcl

#    runcmd $::SVN co http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tcl tcl
    eval "runcmd $::CVS -d:pserver:anonymous:@tcl.cvs.sourceforge.net:/cvsroot/tcl login"
    if { ! [ file exists tcl ] } {
            eval "runcmd $::CVS -z3 -d:pserver:anonymous:@tcl.cvs.sourceforge.net:/cvsroot/tcl co -r core-8-5-7 -P tcl"
    }
    if {$::GENLIB(buildit)} {
      if {$isWindows} {
          # can't do windows
      } else {
          cd $PACKAGE_LIB/tcl/tcl/unix
          eval runcmd ./configure --prefix=$::PACKAGE_BUILD $::TCL_SHARED_FLAGS
          eval runcmd $::MAKE
          eval runcmd $::MAKE install
    if { $isDarwin } {
      catch { file attributes \
    $::PACKAGE_BUILD/lib/libtcl8.5.$shared_lib_ext -permissions 00755 }
          }
      }
    }
}
}

if { [string compare $BUILD_PACKAGE(tk) "yes" ] == 0 } {
if { [BuildThis $::TK_TEST_FILE "tk"] == 1 } {
    cd $PACKAGE_LIB/tcl

#    runcmd $::SVN co http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tk tk
    eval "runcmd $::CVS -d:pserver:anonymous:@tktoolkit.cvs.sourceforge.net:/cvsroot/tktoolkit login"
    if { ! [ file exists tk ] } {
      eval "runcmd $::CVS -z3 -d:pserver:anonymous:@tcl.cvs.sourceforge.net:/cvsroot/tktoolkit co -r core-8-5-7 -P tk"
    }
    if {$::GENLIB(buildit)} {
      if {$isWindows} {
         # ignore, already downloaded with tcl
      } else {
         cd $PACKAGE_LIB/tcl/tk/unix
         if { $isDarwin } {
                  eval runcmd ./configure --with-tcl=$PACKAGE_BUILD/lib --prefix=$PACKAGE_BUILD --disable-corefoundation --x-libraries=/usr/X11R6/lib --x-includes=/usr/X11R6/include --with-x $::TCL_SHARED_FLAGS
               } else {
                  eval runcmd ./configure --with-tcl=$PACKAGE_BUILD/lib --prefix=$PACKAGE_BUILD $::TCL_SHARED_FLAGS
               }
         eval runcmd $::MAKE
         eval runcmd $::MAKE install
         set tklibs [ glob $::PACKAGE_BUILD/lib/libtk* ]
#         catch { file copy -force $tklibs $::PACKAGE_BUILD/bin }
         #eval runcmd cp $tklibs $::PACKAGE_BUILD/bin
         
         file copy -force $PACKAGE_LIB/tcl/tk/generic/default.h $PACKAGE_BUILD/include
         file copy -force $PACKAGE_LIB/tcl/tk/unix/tkUnixDefault.h $PACKAGE_BUILD/include
      }
   }
}
}

################################################################################
# build fftw
#
if { [string compare $BUILD_PACKAGE(fftw) "yes" ] == 0 } {
  if { [BuildThis $::FFTW_TEST_FILE "fftw"] == 1 } {
   set ::PACKAGE_HOME [file dirname [file dirname $script]]
   set PACKAGE_NAME [file tail [file dirname [file dirname [file normalize [info script ]]]]]
    file mkdir $PACKAGE_LIB/fftw
   #get FFTW toolkit and unzip                                                                 
    cd $PACKAGE_LIB/fftw                                                                       
    eval runcmd ls                                                                             
    eval runcmd gunzip -c $PACKAGE_HOME/fftw-3.2.2.tar.gz | tar -xvf -;                        
    cd $PACKAGE_LIB/fftw/fftw-3.2.2                                                            
                                                                                                 
  if {$::GENLIB(buildit)} {                                                                    
      if {$isWindows} {                                                                        
         # ignore, already downloaded with tcl                                                 
      } else {                                                                                 
        # file mkdir $PACKAGE_LIB/fftw-build                                                   
                                                                                                  
          eval runcmd ./configure --prefix $PACKAGE_BUILD --enable-float --enable-threads      
          eval runcmd $::MAKE                                                                  
          eval runcmd $::MAKE install                                                          
          eval runcmd ./configure --prefix $PACKAGE_BUILD  --enable-threads                    
          eval runcmd $::MAKE                                                                  
          eval runcmd $::MAKE install                                                          
         set FFTW_CMAKE_FLAGS "-DFFTW_INCLUDE_PATH=$::PACKAGE_BUILD -DFFTWD_LIB:PATH=$::PACKAGE_BUILD/lib -DFFTWF_LIB:PATH=$::PACKAGE_BUILD/lib -DUSE_FFTWF:BOOL=ON DUSE_FFTWD:BOOL=OFF -DUSE_SCSL:BOOL=OFF"
      }                                                                                        
   }                                                                                           
 }                                                                                              
}

if { [string compare $BUILD_PACKAGE(Qt) "yes" ] == 0 } {
if { [BuildThis $::QT_TEST_FILE "Qt"] == 1 } {
   set QT_SRC_DIR qt-everywhere-opensource-src-4.6.2
   set QT_TAR ${QT_SRC_DIR}.tar.gz
        cd $PACKAGE_LIB
        if { ! [ file exists $QT_TAR ] } {
           puts "${QT_TAR} doesn't exist"
           eval "runcmd wget -nd http://get.qt.nokia.com/qt/source/${QT_TAR}"
           eval "runcmd tar xzf ${QT_TAR}"
        } else {
           if { ! [ file exists ${QT_SRC_DIR} ] } {
             eval "runcmd tar xzf ${QT_TAR}"
           }
        }
        file mkdir qt-build
        cd qt-build
        eval "runcmd touch .licenseAccepted"
        eval "runcmd touch LICENSE.LGPL"
        set configcmd "runcmd sh ../${QT_SRC_DIR}/configure --prefix=$PACKAGE_BUILD"
        set configcmd "$configcmd -confirm-license -opensource"
        if { ! [ file exists config.status ] } {
                eval $configcmd
        }
        runcmd make
        runcmd make install
}

}

################################################################################
# Get and build vtk
#

if { $BUILD_PACKAGE(vtk) == "yes"  } {
if { [BuildThis $::VTK_TEST_FILE "vtk"] == 1 } {

    cd $PACKAGE_LIB

    eval "runcmd $::CVS -d :pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK login"
   #eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@public.kitware.com:/cvsroot/VTK checkout -D $::VTK_DATE VTK"
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anonymous@public.kitware.com:/cvsroot/VTK checkout -r $::VTK_TAG VTK"

    # Andy's temporary hack to get around wrong permissions in VTK cvs repository
    # catch statement is to make file attributes work with RH 7.3
    if { !$isWindows } {
        catch "file attributes $PACKAGE_LIB/VTK/VTKConfig.cmake.in -permissions a+rw"
    }
    if {$::GENLIB(buildit)} {

      file mkdir $PACKAGE_LIB/VTK-build
      cd $PACKAGE_LIB/VTK-build

      set USE_VTK_ANSI_STDLIB ""
      if { $isWindows } {
        if {$MSVC6} {
            set USE_VTK_ANSI_STDLIB "-DVTK_USE_ANSI_STDLIB:BOOL=ON"
        }
      }

      # if configure has run once, don't do it again, make will re-run it if need be.
          if { ! [ file exists $PACKAGE_LIB/VTK-build/CMakeCache.txt ] } {
      #
      # Note - the two banches are identical down to the line starting -DOPENGL...
      # -- the text needs to be duplicated to avoid quoting problems with paths that have spaces
      #
                  if { $isLinux && $::tcl_platform(machine) == "x86_64" } {
                          eval runcmd $::CMAKE \
                              $::CMAKE_COMPILE_SETTINGS  \
                              $::OPENGL_COMPILE_SETTINGS \
                              -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                              -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                              -DVTK_USE_CARBON:BOOL=OFF \
                              -DVTK_USE_COCOA:BOOL=OFF \
                              -DVTK_USE_X:BOOL=ON \
                              -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                              -DCMAKE_SKIP_RPATH:BOOL=ON \
                              -DBUILD_TESTING:BOOL=OFF \
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
                              $QT_CMAKE_FLAGS \
                              $USE_VTK_ANSI_STDLIB \
                              -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
                              -DVTK_USE_64BIT_IDS:BOOL=ON \
                              ../VTK
                      } elseif { $isDarwin } {
        puts "QT_CMAKE_FLAGS=%%${QT_CMAKE_FLAGS}%%"
                          puts "CMAKE_COMPILER_SETTINGS=%%${::CMAKE_COMPILE_SETTINGS}%%"
                          puts "OPENGL_COMPILE_SETTINGS=%%${::OPENGL_COMPILE_SETTINGS}%%"
                          eval runcmd $::CMAKE \
                              $::CMAKE_COMPILE_SETTINGS  \
                              $::OPENGL_COMPILE_SETTINGS \
                              -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                              -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                              -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                              -DCMAKE_SKIP_RPATH:BOOL=OFF \
                              -DBUILD_TESTING:BOOL=OFF \
                              -DVTK_USE_CARBON:BOOL=$USE_CARBON \
                              -DVTK_USE_COCOA:BOOL=$USE_COCOA \
                              -DVTK_USE_TK:BOOL=${BUILD_PACKAGE(tk)} \
                              -DVTK_USE_X:BOOL=$USE_X \
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
                              $QT_CMAKE_FLAGS \
                              $USE_VTK_ANSI_STDLIB \
                              ../VTK
                      } else {
                          eval runcmd $::CMAKE \
                              $::CMAKE_COMPILE_SETTINGS \
                              -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                              -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                              -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                              -DCMAKE_SKIP_RPATH:BOOL=ON \
                              -DBUILD_TESTING:BOOL=OFF \
                              -DVTK_WRAP_TCL:BOOL=ON \
                              -DVTK_USE_HYBRID:BOOL=ON \
                              -DVTK_USE_PATENTED:BOOL=ON \
                              -DVTK_USE_PARALLEL:BOOL=ON \
                              -DVTK_USE_CARBON:BOOL=OFF \
                              -DVTK_USE_COCOA:BOOL=OFF \
                              -DVTK_USE_X:BOOL=ON \
                              -DVTK_DEBUG_LEAKS:BOOL=$::VTK_DEBUG_LEAKS \
                              -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                              -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                              -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
                              -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
                              -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
                              $QT_CMAKE_FLAGS \
                              $USE_VTK_ANSI_STDLIB \
                              ../VTK
                      }
              }
      if { $isWindows } {
        if { $MSVC6 } {
            runcmd $::MAKE VTK.dsw /MAKE "ALL_BUILD - $::CMAKE_BUILD_TYPE"
        } else {
            runcmd $::MAKE VTK.SLN /build  $::CMAKE_BUILD_TYPE
        }
      } else {
        # this is really annoying but ...
        # If you build with shared libraries, GenerateCLP also gets built shared.  But then
        # since we're running it in place, it doesn't know where to load some libraries.
        # so we have to augment LD_LIBRARY_PATH before we can build, so GenerateCLP doesn't fail.
        if { $::BUILD_SHARED_LIBS == "ON"} {
          set libpaths ""
          if { $isDarwin }  {
            if { [ info exists env(DYLD_LIBRARY_PATH] } {
              set env(DYLD_LIBRARY_PATH) "$PACKAGE_LIB/VTK-build/bin:${env(DYLD_LIBRARY_PATH)}"
            } else {
              set env(DYLD_LIBRARY_PATH) "$PACKAGE_LIB/VTK-build/bin"
            }
            puts "DYLD_LIBRARY_PATH=${env(DYLD_LIBRARY_PATH)}"
          } else {
            if { [ info exists env(LD_LIBRARY_PATH] } {
              set env(LD_LIBRARY_PATH) "$PACKAGE_LIB/VTK-build/bin:${env(LD_LIBRARY_PATH)}"
            } else {
              set env(LD_LIBRARY_PATH) "$PACKAGE_LIB/VTK-build/bin"
            }
            puts "LD_LIBRARY_PATH=${env(LD_LIBRARY_PATH)}"
          }
        }
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
      }
  }
}
}
if { $isDarwin } {
  set ::env(CFLAGS) "${SAVECFLAGS}"
  set ::env(CXXFLAGS) "${SAVECXXFLAGS}"
}


################################################################################
# Get and build kwwidgets
#

if { [string compare $BUILD_PACKAGE(kwwidgets) "yes" ] == 0 } {
if { [BuildThis $::KWWidgets_TEST_FILE "kwwidgets"] == 1 } {
    cd $PACKAGE_LIB

    runcmd $::CVS -d :pserver:anoncvs:@www.kwwidgets.org:/cvsroot/KWWidgets login
#    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets checkout -r $::KWWidgets_TAG KWWidgets"
#    runcmd cvs -d :pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets co KWWidgets
    eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets checkout -r $::KWWidgets_TAG KWWidgets"
#eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets checkout -D $::KWWidgets_DATE KWWidgets"

    if {$::GENLIB(buildit)} {
      file mkdir $PACKAGE_LIB/KWWidgets-build
      cd $PACKAGE_LIB/KWWidgets-build


      if { ! [ file exists $PACKAGE_LIB/KWWidgets-build/CMakeCache.txt ] } {
              eval runcmd $::CMAKE \
                  $::CMAKE_COMPILE_SETTINGS  \
                  -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                  -DVTK_DIR:PATH=$PACKAGE_BUILD/lib/vtk-$VTK_VERSION \
                  -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                  -DCMAKE_SKIP_RPATH:BOOL=ON \
                  -DBUILD_EXAMPLES:BOOL=ON \
                  -DKWWidgets_BUILD_EXAMPLES:BOOL=ON \
                  -DBUILD_TESTING:BOOL=ON \
                  -DKWWidgets_BUILD_TESTING:BOOL=ON \
                  -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                  ../KWWidgets
          }
    if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE KWWidgets.dsw /MAKE "ALL_BUILD - $::CMAKE_BUILD_TYPE"
        } else {
            runcmd $::MAKE KWWidgets.SLN /build  $::CMAKE_BUILD_TYPE
        }
      } else {
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
      }
  }
}
}

################################################################################
# Get and build itk
#

if { [string compare $BUILD_PACKAGE(itk) "yes" ] == 0 } {
if { [BuildThis $::ITK_TEST_FILE "itk"] == 1 } {
    cd $PACKAGE_LIB

#    eval runcmd $::SVN co http://svn.slicer.org/$::PACKAGE_NAME/$::PACKAGE_NAME/Slicer3-lib-mirrors/trunk/Insight Insight
## Checkout CableSwig
## Use a date tag where ITK and CableSwig can be compiled together.
# Moved up to package_variables.tcl    set ::ITK_CABLE_DATE "2008-03-09"
#    eval runcmd $::CVS update -dAP -D $::ITK_CABLE_DATE
    runcmd $::CVS -d :pserver:anoncvs:@www.vtk.org:/cvsroot/Insight login
    if { [ string compare $::ITK_CABLE_DATE "CVS" ] == 0} {
       eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.vtk.org:/cvsroot/Insight co -dAP Insight"
    } else {
       eval "runcmd $::CVS $CVS_CO_FLAGS -d :pserver:anoncvs@www.vtk.org:/cvsroot/Insight checkout -D $::ITK_CABLE_DATE Insight"
    }
    puts "ADD_WRAPITK = $::ADD_WRAPITK"
    if { [string compare $::ADD_WRAPITK "ON" ] == 0 } {

        cd $::PACKAGE_LIB/Insight/Utilities
#         set patchfile "$::PACKAGE_HOME/SlicerPatches/wrap_itkWindowedSincInterpolateImageFunction.cmake"
#         set plib "$::PACKAGE_LIB/Insight/Wrapping/WrapITK/Modules/Interpolators"
#         puts "PATCHFILE = $patchfile PACKAGE_LIB = $plib"
#         if { [ file exists  $plib ] == 1 } {
#             if { [ file exists $patchfile ] } {
#             puts "***** copying $patchfile"
#             runcmd cp $patchfile $plib
#             }
#         }
    if { [ string compare $::ITK_CABLE_DATE "CVS" ] == 0} {
            eval "runcmd $::CVS -d :pserver:anonymous@public.kitware.com:/cvsroot/CableSwig co -dAP  CableSwig"
        } else {
            eval "runcmd $::CVS -d :pserver:anonymous@public.kitware.com:/cvsroot/CableSwig co -D $::ITK_CABLE_DATE  CableSwig"
        }
    }
    cd $::PACKAGE_LIB

    if {$::GENLIB(buildit)} {
      file mkdir $PACKAGE_LIB/Insight-build
      cd $PACKAGE_LIB/Insight-build

      if { ! [ file exists $PACKAGE_LIB/Insight-build/CMakeCache.txt ] } {
              if {$isDarwin} {
                      eval runcmd $::CMAKE \
                          $::CMAKE_COMPILE_SETTINGS  \
                          -DBUILD_EXAMPLES:BOOL=OFF \
                          -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                          -DBUILD_TESTING:BOOL=OFF \
                          -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                          -DCMAKE_SKIP_RPATH:BOOL=OFF \
                          -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON \
                          -DITK_USE_REVIEW:BOOL=ON \
                          -DITK_USE_REVIEW_STATISTICS:BOOL=ON \
                          -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON \
                          -DITK_USE_ORIENTED_IMAGE_DIRECTION:BOOL=ON \
                          -DITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE:BOOL=ON \
                          -DITK_USE_PORTABLE_ROUND:BOOL=ON \
                          -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON \
                          { -DWRAP_ITK_DIMS:STRING=2\;3 } \
                          -DUSE_WRAP_ITK:BOOL=$::ADD_WRAPITK \
                          -DINSTALL_WRAP_ITK_COMPATIBILITY:BOOL=OFF \
                          -DWRAP_ITK_JAVA:BOOL=OFF \
                          -DWRAP_ITK_PYTHON:BOOL=OFF \
                          -DWRAP_ITK_TCL:BOOL=$::ADD_WRAPITK  \
                          -DWRAP_complex_float:BOOL=OFF \
                          -DWRAP_covariant_vector_float:BOOL=OFF \
                          -DWRAP_float:BOOL=ON \
                          -DWRAP_rgb_signed_short:BOOL=OFF \
                          -DWRAP_rgb_unsigned_char:BOOL=OFF \
                          -DWRAP_rgb_unsigned_short:BOOL=OFF \
                          -DWRAP_signed_short:BOOL=ON \
                          -DWRAP_unsigned_char:BOOL=ON \
                          -DWRAP_unsigned_short:BOOL=OFF \
                          -DWRAP_vector_float:BOOL=ON \
                          -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                          -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                          -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
                          -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
                          -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
                          -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                          $FFTW_CMAKE_FLAGS \
                          ../Insight
                  } else {
                      eval runcmd $::CMAKE \
                          $::CMAKE_COMPILE_SETTINGS \
                          -DBUILD_EXAMPLES:BOOL=OFF \
                          -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
                          -DBUILD_TESTING:BOOL=OFF \
                          -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                          -DCMAKE_SKIP_RPATH:BOOL=ON \
                          -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON \
                          -DITK_USE_REVIEW:BOOL=ON \
                          -DITK_USE_REVIEW_STATISTICS:BOOL=ON \
                          -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON \
                          -DITK_USE_ORIENTED_IMAGE_DIRECTION:BOOL=ON \
                          -DITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE:BOOL=ON \
                          -DITK_USE_PORTABLE_ROUND:BOOL=ON \
                          -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON \
                          { -DWRAP_ITK_DIMS:STRING=2\;3 } \
                          -DUSE_WRAP_ITK:BOOL=$::ADD_WRAPITK \
                          -DINSTALL_WRAP_ITK_COMPATIBILITY:BOOL=OFF \
                          -DWRAP_ITK_JAVA:BOOL=OFF \
                          -DWRAP_ITK_PYTHON:BOOL=OFF \
                          -DWRAP_ITK_TCL:BOOL=$::ADD_WRAPITK  \
                          -DWRAP_complex_float:BOOL=OFF \
                          -DWRAP_covariant_vector_float:BOOL=OFF \
                          -DWRAP_float:BOOL=ON \
                          -DWRAP_rgb_signed_short:BOOL=OFF \
                          -DWRAP_rgb_unsigned_char:BOOL=OFF \
                          -DWRAP_rgb_unsigned_short:BOOL=OFF \
                          -DWRAP_signed_short:BOOL=ON \
                          -DWRAP_unsigned_char:BOOL=ON \
                          -DWRAP_unsigned_short:BOOL=OFF \
                          -DWRAP_vector_float:BOOL=ON \
                          -DTCL_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                          -DTK_INCLUDE_PATH:PATH=$TCL_INCLUDE_DIR \
                          -DTCL_LIBRARY:FILEPATH=$::VTK_TCL_LIB \
                          -DTK_LIBRARY:FILEPATH=$::VTK_TK_LIB \
                          -DTCL_TCLSH:FILEPATH=$::VTK_TCLSH \
                          -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                          $FFTW_CMAKE_FLAGS \
                          ../Insight
                  }
          }
      if {$isWindows} {
        if { $MSVC6 } {
            runcmd $::MAKE ITK.dsw /MAKE "ALL_BUILD - $::CMAKE_BUILD_TYPE"
        } else {
            runcmd $::MAKE ITK.SLN /build  $::CMAKE_BUILD_TYPE
        }
      } else {
        eval runcmd $::MAKE 
        eval runcmd $::MAKE install
    }
    puts "Patching ITK..."

    set fp1 [open "$PACKAGE_LIB/Insight-build/Utilities/nifti/niftilib/cmake_install.cmake" r]
    set fp2 [open "$PACKAGE_LIB/Insight-build/Utilities/nifti/znzlib/cmake_install.cmake" r]
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

    set fw1 [open "$PACKAGE_LIB/Insight-build/Utilities/nifti/niftilib/cmake_install.cmake" w]
    set fw2 [open "$PACKAGE_LIB/Insight-build/Utilities/nifti/znzlib/cmake_install.cmake" w]

    puts -nonewline $fw1 $data1
#    puts "data1out is $data1"
    puts -nonewline $fw2 $data2
#    puts "data2out is $data2"
 
    close $fw1
    close $fw2
  }
}
}


##set inriaSiteStatus "Down"
set inriaSiteStatus "Up"
if { [string compare $BUILD_PACKAGE(vtkinria3d) "yes" ] == 0 } {
#
# get and build vtkinria3d
if { ![file exists $::vtkinria3d_TEST_FILE] || $::GENLIB(update) } {
    cd $PACKAGE_LIB

#    catch { eval runcmd $::SVN co svn://scm.gforge.inria.fr/svn/vtkinria3d -r "\\{$::vtkinria3d_TAG\\}" }
    catch { eval runcmd $::SVN co svn://scm.gforge.inria.fr/svn/vtkinria3d/trunk -r "\\{$::vtkinria3d_TAG\\}" vtkinria3d }
    file mkdir $PACKAGE_LIB/vtkinria3d-build
    cd $PACKAGE_LIB/vtkinria3d-build

    if { $isWindows } {
      set ::GETBUILDTEST(cpack-generator) "NSIS"
    } else {
      set ::GETBUILDTEST(cpack-generator) "STGZ"
    }

    if {$isWindows} {
      set ::env(PATH) "$::env(PATH):$::PACKAGE_BUILD/lib/InsightToolkit:$::PACKAGE_BUILD/lib/vtk"
    }
    if { $isDarwin } {
      if { [info exists ::env(DYLD_LIBRARY_PATH)] } {
        set ::env(DYLD_LIBRARY_PATH) "$::env(DYLD_LIBRARY_PATH):$::PACKAGE_BUILD/lib/InsightToolkit:$::PACKAGE_BUILD/lib/vtk-5.1"
      } else {
        set ::env(DYLD_LIBRARY_PATH) "$::PACKAGE_BUILD/lib/InsightToolkit:$::PACKAGE_BUILD/lib/vtk"
      }
    }
    if { $isLinux } {
      if { [info exists ::env(LD_LIBRARY_PATH)] } {
        set ::env(LD_LIBRARY_PATH) "$::env(LD_LIBRARY_PATH):$::PACKAGE_BUILD/lib/InsightToolkit:$::PACKAGE_BUILD/lib/vtk-5.1"
      } else {
        set ::env(LD_LIBRARY_PATH) "$::PACKAGE_BUILD/lib/InsightToolkit:$::PACKAGE_BUILD/lib/vtk"
      }
    }

        if { ! [ file exists $PACKAGE_LIB/vtkinria-build/CMakeCache.txt ] } {
                eval runcmd $::CMAKE \
                    $::CMAKE_COMPILE_SETTINGS  \
                    $::OPENGL_COMPILE_SETTINGS \
                    -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                    -DCMAKE_BUILD_TYPE:STRING=$::CMAKE_BUILD_TYPE \
                    -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
                    -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD \
                    -DITK_DIR:FILEPATH=$::ITK_BINARY_PATH \
                    -DKWWidgets_DIR:FILEPATH=$::KWWIDGETS_DIR \
                    -DVTK_DIR:PATH=$PACKAGE_BUILD/lib/vtk-$VTK_VERSION \
                    -DUSE_KWWidgets:BOOL=ON \
                    -DUSE_ITK:BOOL=ON \
                    -DCMAKE_BUILD_TYPE=$::CMAKE_BUILD_TYPE \
                    ../vtkinria3d
            }
#        -DCPACK_GENERATOR:STRING=$::GETBUILDTEST(cpack-generator) \
#        -DCPACK_PACKAGE_FILE_NAME:STRING=$::GETBUILDTEST(binary-filename) \

    if {$isWindows} {
        if { $MSVC6 } {
            eval runcmd $::MAKE vtkinria3d.dsw /MAKE "ALL_BUILD - $::CMAKE_BUILD_TYPE"
        } else {
            eval runcmd $::MAKE vtkinria3d.SLN /build  $::CMAKE_BUILD_TYPE
        }
    } else {
        eval runcmd $::MAKE
        eval runcmd $::MAKE install
    }
}
}

if { [string compare $BUILD_PACKAGE(fltk) "yes" ] == 0 } {
if { ![file exists $::fltk_TEST_FILE] || $::GENLIB(update) } {
   cd $PACKAGE_LIB
   catch { eval runcmd \
       $::SVN co http://svn.easysw.com/public/fltk/fltk/branches/branch-${::fltk_VERSION}/ fltk-${::fltk_VERSION} } 
   file mkdir $PACKAGE_LIB/fltk-build
   cd $PACKAGE_LIB/fltk-build
   if { $isDarwin } {
     set fltk_platform "-DFLTK_QUARTZ:BOOL=ON"
   } else {
     set fltk_platform ""
   }
   runcmd $::CMAKE -DCMAKE_INSTALL_PREFIX:PATH=$::PACKAGE_BUILD ${fltk_platform} \
       $PACKAGE_LIB/fltk-${::fltk_VERSION} 
   eval runcmd $::MAKE
   eval runcmd $::MAKE install
}        

}
if {! $::GENLIB(buildit)} {
 exit 0
}

# Are all the test files present and accounted for?  If not, return error code
set file_missing 0
if { $BUILD_PACKAGE(cmake) == "yes" && ![file exists $::CMAKE] } {
    set file_missing 1 
    puts "CMake test file $::CMAKE not found."
}
if { $BUILD_PACKAGE(tcl) == "yes" &&  ![file exists $::TCL_TEST_FILE] } {
    set file_missing 1 
    puts "Tcl test file $::TCL_TEST_FILE not found."
}
if { $BUILD_PACKAGE(tk) == "yes" &&  ![file exists $::TK_TEST_FILE] } {
    set file_missing 1 
    puts "Tk test file $::TK_TEST_FILE not found."
}
if { $BUILD_PACKAGE(vtk) == "yes" &&  ![file exists $::VTK_TEST_FILE] } {
    set file_missing 1 
    puts "VTK test file $::VTK_TEST_FILE not found."
}
if { $BUILD_PACKAGE(itk) == "yes" &&  ![file exists $::ITK_TEST_FILE] } {
    set file_missing 1 
    puts "ITK test file $::ITK_TEST_FILE not found."
}
if { $BUILD_PACKAGE(vtkinria3d) == "yes" &&  ![file exists $::vtkinria3d_TEST_FILE] } {
    set file_missing 1 
    puts "vtkinria3d test file $::vtkinria3d_TEST_FILE not found."
}
if { $BUILD_PACKAGE(fftw) == "yes" &&  ![file exists $::FFTW_TEST_FILE] } {                    
    set file_missing 1                                                                         
    puts "fftw test file $::FFTW_TEST_FILE not found."                                         
}
if { $file_missing == 1 } {
    puts "Not all packages compiled; check errors and run genlib.tcl again."
    exit 1 
} else { 
    puts "All packages compiled."
    exit 0 
}
