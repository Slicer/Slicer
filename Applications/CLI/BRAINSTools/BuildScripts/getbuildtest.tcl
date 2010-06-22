#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# getbuildtest.tcl
#
# does an update and a clean build of slicer3 (including utilities and libs)
# then does a dashboard submission
#
# Usage:
#   getbuildtest [options] [target]
#
# Initiated - sp - 2006-05-11
#

################################################################################
#
# simple command line argument parsing
#


set script [info script]
catch {set script [file normalize $script]}
#
# the following is one of those ugly expressions, so
# It assumes the script we're running is in ${PACKAGE_NAME}/buildscripts
# so we normalize the name to the full path, take the directory part, then the directory
# part of THAT (i.e. ../.. ) and then the 'tail' -- the last path component -- gives
# the name of the package
set PACKAGE_NAME [file tail [file dirname [file dirname [file normalize [info script ]]]]]
proc Usage { {msg ""} } {
    global SLICER

    set msg "$msg\nusage: getbuildtest \[options\] \[target\]"
    set msg "$msg\n  \[target\] is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   h --help : prints this message and exits"
    set msg "$msg\n   -f --clean : delete lib and build directories first"
    set msg "$msg\n   -t --test-type : CTest test target (default: Experimental)"
    set msg "$msg\n   -a --abi : Build Parameter Name (default: DEBUG or DEBUG_64 on the mac)"
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   -u --update : does a cvs/svn update on each lib"
    set msg "$msg\n   --version-patch : set the patch string for the build (used by installer)"
    set msg "$msg\n                   : default: version-patch is the current date"
    set msg "$msg\n   --tag : same as version-patch"
    set msg "$msg\n   --pack : run cpack after building (default: off)"
    set msg "$msg\n   --upload : set the upload string for the binary, used if pack is true"
    set msg "$msg\n            : snapshot (default), nightly, release"
    set msg "$msg\n   --doxy : just do an svn update and run doxygen"
    set msg "$msg\n   --verbose : optional, print out lots of stuff, for debugging"
    set msg "$msg\n   --rpm : optional, specify CPack RPM generator for packaging"
    set msg "$msg\n   --deb : optional, specify CPack DEB generator for packaging"
    puts stderr $msg
}

set ::GETBUILDTEST(skipupdate) "false"
set ::GETBUILDTEST(clean) "false"
set ::GETBUILDTEST(update) ""
set ::GETBUILDTEST(release) ""
set ::GETBUILDTEST(test-type) "Experimental"
set ::GETBUILDTEST(version-patch) ""
set ::GETBUILDTEST(pack) "false"
set ::GETBUILDTEST(upload) "false"
set ::GETBUILDTEST(uploadFlag) "nightly"
set ::GETBUILDTEST(doxy) "false"
set ::GETBUILDTEST(verbose) "false"
###VAM
if { $::tcl_platform(os) == "Darwin" } {
  set ::GETBUILDTEST(abi) "DEBUG_64"
} else {
  set ::GETBUILDTEST(abi) "DEBUG"
}

set ::GETBUILDTEST(buildList) ""
set ::GETBUILDTEST(cpack-generator) ""
set ::GETBUILDTEST(installPrefix) ""
set ::BUILD_SHARED_LIBS "OFF"
set ::IGNORE_EXTERNALS ""
set ::GETBUILDTEST(bin) "bin"
set ::GETBUILDTEST(lib) "lib"
set ::ADD_WRAPITK "OFF"
set ::USE_DEBUG_IMAGE_VIEWER "OFF"
set strippedargs ""

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
set BUILD_PACKAGE(UseCarbon) "no"
set BUILD_PACKAGE(UseCocoa) "no"

set ::CMAKE_BUILD_TYPE "DEBUG"
set ::BUILD_DASHBOARD "no"
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
  "--installPrefix" {
      incr i
      if { $i == $argc } {
                Usage "Missing installPrefix pathname"
            } else {
                set ::GETBUILDTEST(installPrefix) [lindex $argv $i ]
            }
  }
       "--skipupdate" {
           set ::GETBUILDTEST(skipupdate) "true"
       }
       "--nightly" {
           set ::BUILD_DASHBOARD "yes"
           set ::DASHBOARD_TYPE "Nightly"
       }
       "--Experimental" {
           set ::BUILD_DASHBOARD "yes"
           set ::DASHBOARD_TYPE "Experimental"
       }
        "--package-name" {
            incr i
            if { $i == $argc } {
              Usage "Missing package-name argument"
            } else {
              set PACKAGE_NAME [lindex $argv $i ]
            }
        }
        "--clean" -
        "-f" {
            set ::GETBUILDTEST(clean) "true"
        }
        "--update" -
        "-u" {
            set ::GETBUILDTEST(update) "--update"
        }
        "--ignore-externals" {
            set ::IGNORE_EXTERNALS "--ignore-externals"
        }
        "--release" {
            set ::GETBUILDTEST(release) "--release"
            set ::CMAKE_BUILD_TYPE "Release"
        }
  "--shared" {
     set ::BUILD_SHARED_LIBS "ON"
  }
        "-t" -
        "--test-type" {
            incr i
            if { $i == $argc } {
                Usage "Missing test-type argument"
            } else {
                set ::GETBUILDTEST(test-type) [lindex $argv $i]
            }
        }
        "-a" -
        "--abi" {
            incr i
            if { $i == $argc } {
                Usage "Missing abi argument"
                exit -1;
            } else {
                set ::GETBUILDTEST(abi) [lindex $argv $i]
                puts "Setting --abi to  $::GETBUILDTEST(abi)"
                if [ string match "*FAST*" $::GETBUILDTEST(abi) ] {
                  puts "Setting to Release Mode"
                  set ::GETBUILDTEST(release) "--release"
                  set ::CMAKE_BUILD_TYPE "Release"
                } else {
                  puts "Setting to Debug Mode"
                  set ::GETBUILDTEST(release) ""
                  set ::CMAKE_BUILD_TYPE "Debug"
                }
            }
        }
        "--tag" -
        "--version-patch" {
            incr i
            if { $i == $argc } {
                Usage "Missing version-patch argument"
            } else {
                set ::GETBUILDTEST(version-patch) [lindex $argv $i]
            }
        }
        "--pack" {
                set ::GETBUILDTEST(pack) "true"
        }
        "--upload" {
            set ::GETBUILDTEST(upload) "true"
            incr i
            if {$i == $argc} {
                # uses default value
            } else {
                # peek at the next arg to see if we should use it...
                set arg [lindex $argv $i]
                if { [string match "--*" $arg] } {
                  # next arg is another -- flag, so don't use it as the
                  # upload flag...
                  incr i -1
                } else {
                  set ::GETBUILDTEST(uploadFlag) [lindex $argv $i]
                }
            }
        }
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
        "--build-fftw" {
            set BUILD_PACKAGE(fftw) "yes"
        }
        "--build-itk" {
            set BUILD_PACKAGE(itk) "yes"
        }
        "--build-vtkinria3d" {
            set BUILD_PACKAGE(vtkinria3d) "yes"
        }
        "--build-Qt" {
            set BUILD_PACKAGE(Qt) "yes"
        }
        "--system-Qt" {
            set BUILD_PACKAGE(SystemQt) "yes"
            puts "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   --system-Qt= $::BUILD_PACKAGE(SystemQt)"
        }
        "--UseCocoa" {
            set BUILD_PACKAGE(UseCocoa) "yes"
        }
        "--UseCarbon" {
            set BUILD_PACKAGE(UseCarbon) "yes"
        }
        "--doxy" {
            set ::GETBUILDTEST(doxy) "true"
        }
        "--verbose" {
            set ::GETBUILDTEST(verbose) "true"
        }
        "--rpm" {
            set ::GETBUILDTEST(cpack-generator) "RPM"
        }
        "--deb" {
            set ::GETBUILDTEST(cpack-generator) "DEB"
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

if {$argc > 1 } {
    Usage
    exit 1
}
puts "getbuildtest: setting build list to $strippedargs"
set ::GETBUILDTEST(buildList) $strippedargs



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


#initialize platform variables
foreach v { isSolaris isWindows isDarwin isLinux } { set $v 0 }
switch $tcl_platform(os) {
    "SunOS" { set isSolaris 1 }
    "Linux" { set isLinux 1 }
    "Darwin" { set isDarwin 1 }
    default { set isWindows 1 }
}

################################################################################
# First, set up the directory
# - determine the location
# - determine the build
#

set ::PACKAGE_HOME [file dirname [file dirname $script]]
set cwd [pwd]

if { [ string match $::PACKAGE_HOME $cwd ] } {
    puts "Error, do not build in the source directory $cwd"
    puts "This script is meant to run in the directory that contains"
    puts "the source directory, and will build in the BRAINS-COMPILE subdirectory"
    puts "adjacent to the source directory"
    puts "Try these commands: cd .. ; tclsh $::PACKAGE_HOME/BuildScripts/getbuildtest.tcl"
    exit
}

if { $isWindows } {
  set ::PACKAGE_HOME [file attributes $::PACKAGE_HOME -shortname]
}

puts "$::PACKAGE_HOME $::PACKAGE_NAME"
set ::PACKAGE_BUILD_TYPE $::GETBUILDTEST(abi)
puts "Using --abi $::GETBUILDTEST(abi)"
set env(PACKAGE_BUILD_TYPE) $::PACKAGE_BUILD_TYPE

if { [string match $tcl_platform(os) "Windows NT"] } {
  set ::PACKAGE_LIB [file dirname $::PACKAGE_HOME]/BRAINS-COMPILE/Win32/${::PACKAGE_BUILD_TYPE}-lib
  set ::PACKAGE_BUILD [file dirname $::PACKAGE_HOME]/BRAINS-COMPILE/Win32/${::PACKAGE_BUILD_TYPE}-build
} else {
  set ::PACKAGE_LIB [file dirname $::PACKAGE_HOME]/BRAINS-COMPILE/$tcl_platform(os)/${::PACKAGE_BUILD_TYPE}-lib
  set ::PACKAGE_BUILD [file dirname $::PACKAGE_HOME]/BRAINS-COMPILE/$tcl_platform(os)/${::PACKAGE_BUILD_TYPE}-build
}
if {[string match $::GETBUILDTEST(installPrefix) ""]} {
    set ::GETBUILDTEST(installPrefix) $::PACKAGE_BUILD
}

# use an environment variable so doxygen can use it
set ::env(PACKAGE_DOC) $::PACKAGE_HOME/../$::PACKAGE_NAME-doc
#
# these flags added to main CMake
set EXTRA_CMAKE_FLAGS ""
set FFTW_CMAKE_FLAGS ""

# only for QT builds
#
# this needs to be BEFORE sourcing package_variables.tcl
# in the particular case of BRAINS3 building because otherwise
# package_variable will screw up with respect to shared libraries,
# unless --shared is also given on command if
set package_found 0
if { ${PACKAGE_NAME} == "BRAINSTracerQT" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainstracer/BRAINSTracerQT"
set ::BUILD_SHARED_LIBS "ON"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(tcl) "yes"
set BUILD_PACKAGE(tk) "no"
set BUILD_PACKAGE(vtk) "yes"
set BUILD_PACKAGE(itk) "yes"
set BUILD_PACKAGE(vtkinria3d) "no"
#
# I think the default should be SystemQT, so
# later on, the Qt stuff in package_variables.tcl will
# test if Qt is installed and exit with a helpful message
# if Qt isn't installed.
# this test means -- if nothing is specified on the command line,
# assume SystemQt.
if { $BUILD_PACKAGE(SystemQt) == "no" && \
     $BUILD_PACKAGE(Qt) == "no" } {
        set BUILD_PACKAGE(SystemQt) "yes"
}

if { $isDarwin } {
  set BUILD_PACKAGE(UseCarbon) "no"
  set BUILD_PACKAGE(UseCocoa)   "yes"
}
set QT_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=$::PACKAGE_BUILD/bin/qmake"
set package_found 1
}

if { ${PACKAGE_NAME} == "BRAINSTracer" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainstracer"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(tcl) "yes"
set BUILD_PACKAGE(tk) "yes"
set BUILD_PACKAGE(vtk) "yes"
set BUILD_PACKAGE(kwwidgets) "yes"
set BUILD_PACKAGE(itk) "yes"
set BUILD_PACKAGE(vtkinria3d) "yes"
set package_found 1
}
if { ${PACKAGE_NAME} == "GTRACT" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainstracer"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(tcl) "yes"
set BUILD_PACKAGE(tk) "yes"
set BUILD_PACKAGE(vtk) "yes"
set BUILD_PACKAGE(kwwidgets) "yes"
set BUILD_PACKAGE(itk) "yes"
set BUILD_PACKAGE(vtkinria3d) "yes"
set package_found 1
}
if { ${PACKAGE_NAME} == "BRAINS3" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brains/BRAINS/trunk"
set ::GETBUILDTEST(bin) "bin"
set ::GETBUILDTEST(lib) "lib"
set ::BUILD_SHARED_LIBS "ON"
set ::USE_DEBUG_IMAGE_VIEWER "ON"
set ::ADD_WRAPITK "ON"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(tcl) "yes"
set BUILD_PACKAGE(tk) "yes"
set BUILD_PACKAGE(vtk) "yes"
set BUILD_PACKAGE(kwwidgets) "yes"
set BUILD_PACKAGE(itk) "yes"
set BUILD_PACKAGE(vtkinria3d) "no"
set package_found 1
}
if { ${PACKAGE_NAME} == "BRAINSFit" || ${PACKAGE_NAME} == "multimodereg" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/multimodereg"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set ::BUILD_SHARED_LIBS "OFF"
set package_found 1
}
if { ${PACKAGE_NAME} == "BRAINSCut" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainscut"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set package_found 1
}

if { ${PACKAGE_NAME} == "BRAINSDemonWarp" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainsdemonwarp"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set ::BUILD_SHARED_LIBS "OFF"
set package_found 1
}

if { ${PACKAGE_NAME} == "BRAINSConstellationDetector" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainscdetector"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set package_found 1
}

if { ${PACKAGE_NAME} == "BRAINSROIAuto" } {
set ::PACKAGE_TAG "https://www.nitrc.org/svn/brainsroiauto"
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set package_found 1
}

if { ${PACKAGE_NAME} == "iccdefRegistration_New" } {
set ::PACKAGE_TAG "https://www.psychiatry.uiowa.edu/svn/code/BRAINS/branches/zhao_iccdef"
set BUILD_PACKAGE(cmake) "yes"
#set BUILD_PACKAGE(tcl) "yes"
#set BUILD_PACKAGE(tk) "yes"
#set BUILD_PACKAGE(vtk) "yes"
#set BUILD_PACKAGE(kwwidgets) "yes"
set BUILD_PACKAGE(fftw) "yes"
set BUILD_PACKAGE(itk) "yes"
set FFTW_CMAKE_FLAGS "-DFFTW_DIR:PATH=$::PACKAGE_BUILD -DFFTW_LIB:PATH=$::PACKAGE_BUILD/lib"
set package_found 1
}

if { ${PACKAGE_NAME} == "SGIProgsNew" } {
  if { $isDarwin } {
    if { [string match "*64" $::GETBUILDTEST(abi) ] } {
      puts "Can't build SGIProgsNew 64-bit because of FLTK/Quartz limitations ABI=$::GETBUILDTEST(abi)"
      exit 1
    }
  }
set BUILD_PACKAGE(cmake) "yes"
set BUILD_PACKAGE(itk) "yes"
set BUILD_PACKAGE(fltk) "yes"
set EXTRA_CMAKE_FLAGS "-DUSE_GUI:BOOL=ON"
set package_found 1
}

if { $package_found != 1 } {
puts "${PACKAGE_NAME} is not a recognizeable package for this script"
exit 1
}
# need to pass this variable in the environment, so that
# when package_variables.tcl gets sourced in genlib.tcl, this
# variable gets set properly
set env(BUILD_SHARED_LIBS) $::BUILD_SHARED_LIBS
set env(ADD_WRAPITK) $::ADD_WRAPITK


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

puts "making with $::MAKE"

## Set up the directory structure for the out of source build.
if { ![file exists $PACKAGE_LIB] } {
    file mkdir $PACKAGE_LIB
}
if { ![file exists ${PACKAGE_BUILD}/bin ] } {
    file mkdir ${PACKAGE_BUILD}/bin
}
#
# write a script that sets up paths properly
set pathscript "${::PACKAGE_BUILD}/bin/setpaths.sh"
set pathfile [ open $pathscript w ]
set ldpath "${::PACKAGE_BUILD}/lib"
if { $BUILD_PACKAGE(itk) == "yes" } {
set ldpath "${ldpath}:${::PACKAGE_BUILD}/lib/InsightToolkit"
}
if { $BUILD_PACKAGE(vtk) == "yes" } {
set ldpath "${ldpath}:${::PACKAGE_BUILD}/lib/vtk-${VTK_VERSION}"
}
if { $BUILD_PACKAGE(kwwidgets) == "yes" } {
set ldpath "${ldpath}:${::PACKAGE_BUILD}/lib/KWWidgets"
}
if { $isDarwin } {
  set ldpathvar "DYLD_LIBRARY_PATH"
} else {
  set ldpathvar "LD_LIBRARY_PATH"
}
puts $pathfile "#!/bin/sh"
puts $pathfile "${ldpathvar}=\"${ldpath}\""
puts $pathfile "export ${ldpathvar}"
puts $pathfile {if [ $# -gt 0 ] ; then}
puts $pathfile "\techo \$${ldpathvar}\nfi"
close $pathfile

#
# Deletes both PACKAGE_LIB and PACKAGE_BUILD if clean option given
#
# tcl file delete is broken on Darwin, so use rm -rf instead
if { $::GETBUILDTEST(clean) } {
    puts "Deleting slicer lib files..."
    if { $isDarwin } {
        runcmd rm -rf $PACKAGE_LIB
        runcmd rm -rf $PACKAGE_BUILD
        if { [file exists $PACKAGE_LIB/tcl/isPatched] } {
            runcmd rm $PACKAGE_LIB/tcl/isPatched
        }

        if { [file exists $PACKAGE_LIB/tcl/isPatchedBLT] } {
            runcmd rm $PACKAGE_LIB/tcl/isPatchedBLT
        }
    } else {
        file delete -force $PACKAGE_LIB
        file delete -force $PACKAGE_BUILD
    }
}

if { $::GETBUILDTEST(doxy) && ![file exists $::env(PACKAGE_DOC)] } {
    puts "Making documentation directory  $::env(PACKAGE_DOC)"
    file mkdir $::env(PACKAGE_DOC)
}


################################################################################
#
# the actual build and test commands
# - checkout the source code
# - make the prerequisite libs
# - cmake and build the program
# - run the tests
# - make a package
#


# svn checkout (does an update if it already exists)
cd $::PACKAGE_HOME/..
puts "$::PACKAGE_HOME $::PACKAGE_NAME"
if { [file exists $::PACKAGE_NAME] } {
    cd $::PACKAGE_NAME
    if { ! $::GETBUILDTEST(skipupdate) } {
            runcmd svn update $::IGNORE_EXTERNALS
        }
  } else {
    eval runcmd svn checkout $::IGNORE_EXTERNALS $::PACKAGE_TAG $::PACKAGE_NAME
  }

if { $::GETBUILDTEST(doxy) } {
    # just run doxygen and exit
    puts "Creating documenation files in $::env(PACKAGE_DOC)"
    set cmd "doxygen $::PACKAGE_HOME/Doxyfile"
    eval runcmd $cmd
    return
}


# build the lib with options
cd $::PACKAGE_HOME
set cmd "sh ./BuildScripts/genlib.tcl $PACKAGE_LIB $::PACKAGE_BUILD"
if { $BUILD_PACKAGE(cmake) == "yes" } {
    append cmd " --build-cmake"
}
if { $BUILD_PACKAGE(tcl) == "yes" } {
    append cmd " --build-tcl"
}
if { $BUILD_PACKAGE(tk) == "yes" } {
    append cmd " --build-tk"
}
if { $BUILD_PACKAGE(vtk) == "yes" } {
    append cmd " --build-vtk"
}
if { $BUILD_PACKAGE(kwwidgets) == "yes" } {
    append cmd " --build-kwwidgets"
}
if { $BUILD_PACKAGE(itk) == "yes" } {
    append cmd " --build-itk"
}
if { $BUILD_PACKAGE(fftw) == "yes" } {
    append cmd " --build-fftw"
}
if { $BUILD_PACKAGE(vtkinria3d) == "yes" } {
    append cmd " --build-vtkinria3d"
}
if { $::GETBUILDTEST(release) != "" } {
   append cmd " $::GETBUILDTEST(release)"
}
if { $::GETBUILDTEST(update) != "" } {
   append cmd " $::GETBUILDTEST(update)"
}
if { $BUILD_PACKAGE(fltk) == "yes" } {
    append cmd " --build-fltk"
}
if { $BUILD_PACKAGE(Qt) == "yes" } {
    append cmd " --build-Qt"
}
if { $BUILD_PACKAGE(SystemQt) == "yes" } {
    append cmd " --system-Qt"
}
if { $BUILD_PACKAGE(UseCarbon) == "yes" } {
    append cmd " --UseCarbon"
}
if { $BUILD_PACKAGE(UseCocoa) == "yes" } {
    append cmd " --UseCocoa"
}
if { $::GETBUILDTEST(buildList) != "" } {
    # puts "Passing $::GETBUILDTEST(buildList) to genlib"
    append cmd " $::GETBUILDTEST(buildList)"
}
eval runcmd $cmd

if { $::GETBUILDTEST(version-patch) == "" } {
  # TODO: add build type (win32, etc) here...
  set ::GETBUILDTEST(version-patch) [clock format [clock seconds] -format %Y-%m-%d]
}

# set the binary filename root
set ::GETBUILDTEST(binary-filename) "$::PACKAGE_NAME-3.3-alpha-$::GETBUILDTEST(version-patch)-$::env(BUILD)"
if {$::GETBUILDTEST(verbose)} {
    puts "CPack will use $::::GETBUILDTEST(binary-filename)"
}

# set the cpack generator to determine the binary file extension
if {$isLinux || $isDarwin} {
    if { $::GETBUILDTEST(cpack-generator) == "" } {
        # default generator is TGZ"
        set ::GETBUILDTEST(cpack-generator) "TGZ"
        set ::GETBUILDTEST(cpack-extension) ".tar.gz"
        # if wish to have .sh, use generator = STGZ and extension = .sh / currently disabled due to Ubuntu bug
    }
    if {$::GETBUILDTEST(cpack-generator) == "RPM" || $::GETBUILDTEST(cpack-generator) == "DEB"} {
        # RPMs cannot have dashes in the version names, so we use underscores instead
        set ::GETBUILDTEST(version-patch) [clock format [clock seconds] -format %Y_%m_%d]

        if { $::GETBUILDTEST(cpack-generator) == "RPM" } {
            set ::GETBUILDTEST(cpack-extension) ".rpm"
        }
        if { $::GETBUILDTEST(cpack-generator) == "DEB" } {
            set ::GETBUILDTEST(cpack-extension) ".deb"
        }
    }
}


if {$isWindows} {
    set ::GETBUILDTEST(cpack-generator) "NSIS"
    set ::GETBUILDTEST(cpack-extension) ".exe"
}

# once dmg packaging is done
if {0 && $isDarwin} {
   set ::GETBUILDTEST(cpack-generator) "OSXX11"
   set ::GETBUILDTEST(cpack-extension) ".dmg"
}

# make verbose makefiles?
if {$::GETBUILDTEST(verbose)} {
   set ::GETBUILDTEST(cmake-verbose) "ON"
} else {
   set ::GETBUILDTEST(cmake-verbose) "OFF"
}

puts "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#
# this is really annoying but ...
# If you build with shared libraries, GenerateCLP also gets built shared.  But then
# since we're running it in place, it doesn't know where to load some libraries.
# so we have to augment LD_LIBRARY_PATH before we can build, so GenerateCLP doesn't fail.
if { $::BUILD_SHARED_LIBS == "ON"} {
  set libpaths ""
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/bin"
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/lib"
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/lib/InsightToolkit"
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/lib/InsightToolkit/WrapITK/lib"
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/lib/vtk-${VTK_VERSION}"
  set libpaths "${libpaths}:${::PACKAGE_BUILD}/lib/KWWidgets"
  puts "LIBPATHS = ${libpaths}"
  if { $isDarwin }  {
    if { [ info exists env(DYLD_LIBRARY_PATH] } {
      set env(DYLD_LIBRARY_PATH) "${libpaths}:${env(DYLD_LIBRARY_PATH)}"
    } else {
      set env(DYLD_LIBRARY_PATH) "${libpaths}"
    }
  } else {
    if { [ info exists env(LD_LIBRARY_PATH] } {
      set env(LD_LIBRARY_PATH) "${libpaths}:${env(LD_LIBRARY_PATH)}:${libpaths}"
    } else {
      set env(LD_LIBRARY_PATH) "${libpaths}"
    }
  }
}
puts "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

# build ${PACKAGE_NAME} version 3
file mkdir  $::PACKAGE_BUILD/${PACKAGE_NAME}-build
 cd $::PACKAGE_BUILD/${PACKAGE_NAME}-build
 if { ! [file exists $::PACKAGE_BUILD/${PACKAGE_NAME}-build/src/brainsConfigure.h ] } {
         eval runcmd $::CMAKE \
             $::CMAKE_COMPILE_SETTINGS  \
             $::OPENGL_COMPILE_SETTINGS \
             $QT_CMAKE_FLAGS \
             -DCMAKE_BUILD_TYPE=$::CMAKE_BUILD_TYPE \
             -DMAKECOMMAND:STRING=$::MAKE \
             -DITK_DIR:FILEPATH=${::ITK_BINARY_PATH} \
             -DKWWidgets_DIR:FILEPATH=$::KWWIDGETS_DIR \
             -DVTK_DIR:FILEPATH=${::VTK_BINARY_PATH} \
             -DCMAKE_INSTALL_PREFIX:PATH=$::GETBUILDTEST(installPrefix) \
             -DCPACK_GENERATOR:STRING=$::GETBUILDTEST(cpack-generator) \
             -DCPACK_PACKAGE_FILE_NAME:STRING=$::GETBUILDTEST(binary-filename) \
             -DUSE_OLD_BUILD:BOOL=OFF \
             -DBRAINS_BUILD_TESTING:BOOL=ON \
             -DBUILD_TESTING:BOOL=ON \
             -DBRAINS_BINARY_DIR:PATH=$::PACKAGE_BUILD \
             -DDISABLE_ITK_TESTING:BOOL=ON \
             -DDISABLE_ALT_DICOM_FILTERS:BOOL=ON \
             -DUSE_GUI:BOOL=OFF \
             -DENABLE_TEST_IN_GUI_MODE:BOOL=OFF \
             -DUSE_BRAINS_BETA:BOOL=ON \
             -DUSE_BRAINS_ALPHA:BOOL=ON \
             -DCMAKE_SKIP_RPATH:BOOL=ON \
             -DUSE_PYTHON:BOOL=OFF \
             -DUSE_TCL:BOOL=ON \
             -DWrapITK_DIR:PATH=$::PACKAGE_LIB/Insight-build/Wrapping/WrapITK \
             -DINSTALL_DEVEL_FILES:BOOL=ON \
             -DBUILD_SHARED_LIBS:BOOL=$::BUILD_SHARED_LIBS \
             -DTCL_INCLUDE_PATH:PATH=$::PACKAGE_LIB/tcl-build/include \
             -DTCL_LIBRARY:FILEPATH=$::TCL_LIB_DIR/libtcl8.5.$shared_lib_ext \
             -DTK_INCLUDE_PATH:PATH=$::PACKAGE_LIB/tcl-build/include \
             -DTK_LIBRARY:FILEPATH=$::TCL_LIB_DIR/libtk8.5.$shared_lib_ext \
             -DvtkINRIA3D_DIR:PATH=$::vtkinria3d_BINARY_DIR \
             -DNITRC_Standalone:BOOL=ON \
             -DUSE_DEBUG_IMAGE_VIEWER:BOOL=$::USE_DEBUG_IMAGE_VIEWER \
             -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=$::PACKAGE_BUILD/$::GETBUILDTEST(bin) \
             -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=$::PACKAGE_BUILD/$::GETBUILDTEST(lib) \
             -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=$::PACKAGE_BUILD/$::GETBUILDTEST(lib) \
             -DCTEST_NEW_FORMAT:BOOL=ON \
             $EXTRA_CMAKE_FLAGS \
             $FFTW_CMAKE_FLAGS \
             $::PACKAGE_HOME/../${PACKAGE_NAME}
#         -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=$::PACKAGE_BUILD/${PACKAGE_NAME}-build/$::GETBUILDTEST(bin) \
#         -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=$::PACKAGE_BUILD/${PACKAGE_NAME}-build/$::GETBUILDTEST(lib) \
     }
if { $isWindows } {
    if { $MSVC6 } {
        eval runcmd $::MAKE ${PACKAGE_NAME}.dsw /MAKE $::GETBUILDTEST(test-type)
        if { $::GETBUILDTEST(pack) == "true" } {
            eval runcmd $::MAKE ${PACKAGE_NAME}.dsw /MAKE package
        }
    } else {
        # tell cmake explicitly what command line to run when doing the ctest builds
        set makeCmd "$::MAKE ${PACKAGE_NAME}.sln /build $::VTK_BUILD_TYPE /project ALL_BUILD"
        runcmd $::CMAKE -DMAKECOMMAND:STRING=$makeCmd $${PACKAGE_NAME}_HOME

        if { $::GETBUILDTEST(test-type) == "" } {
            runcmd $::MAKE ${PACKAGE_NAME}.SLN /build $::CMAKE_BUILD_TYPE
        } else {
            # running ctest through visual studio is broken in cmake2.4, so run ctest directly
            runcmd $::CMAKE_PATH/bin/ctest -D $::GETBUILDTEST(test-type) -C $::CMAKE_BUILD_TYPE
        }

        if { $::GETBUILDTEST(pack) == "true" } {
            runcmd $::MAKE ${PACKAGE_NAME}.SLN /build $::CMAKE_BUILD_TYPE /project PACKAGE
        }
    }
} else {
#    set buildReturn [catch "eval runcmd $::MAKE $::GETBUILDTEST(test-type)"]
#    set buildReturn [catch "eval runcmd $::MAKE"]
    if { $::BUILD_DASHBOARD == "yes" } {
       puts "-------------------BUILD_DASHBOARD---------------------"
#       if { $::PACKAGE_NAME != "SGIProgsNew" && $::PACKAGE_NAME != "BRAINSTracerQT" } {
#         set generateclpReturn [catch "eval runcmd $::MAKE -C $::PACKAGE_BUILD/${PACKAGE_NAME}-build/SlicerExecutionModel"]
#       }
       set cmd "runcmd ctest -V -D ${DASHBOARD_TYPE}Start -D ${DASHBOARD_TYPE}Configure -D ${DASHBOARD_TYPE}Build -D ${DASHBOARD_TYPE}Test"
       # Only run valgrind on one dedicated machine.
       if {[string compare $env(PACKAGE_BUILD_TYPE) "DEBUG_64" ] == 0 } {
         if {[string compare $::FULLHOSTNAME "pandora.psychiatry.uiowa.edu"] == 0} {
           set cmd "${cmd} -D ${DASHBOARD_TYPE}MemCheck"
         }
       }
       if {[string compare $env(PACKAGE_BUILD_TYPE) "PROFILE" ] == 0 } {
         set cmd "${cmd} -D ${DASHBOARD_TYPE}Coverage"
       }
       set cmd "${cmd} -D ${DASHBOARD_TYPE}Submit"
       set buildReturn [catch "runcmd $::MAKE"]
       if { ${PACKAGE_NAME} == "BRAINS3" } {
         cd src-build
       }
       set buildReturn [ catch "eval $cmd" ]
       if { ${PACKAGE_NAME} == "BRAINS3" } {
         cd ..
       }

    } else {
      #
      # make GenerateCLP first of all
#       if { $::PACKAGE_NAME != "SGIProgsNew" && $::PACKAGE_NAME != "BRAINSTracerQT" } {
#    set generateclpReturn [catch "eval runcmd $::MAKE -C $::PACKAGE_BUILD/${PACKAGE_NAME}-build/SlicerExecutionModel"]
#        }
        set buildReturn [catch "eval runcmd $::SERIAL_MAKE"]
    }

    if { $::GETBUILDTEST(pack) == "true" } {
        set buildReturn [catch "eval runcmd $::MAKE package"]
    }

    puts "\nResults: "
    puts "build of \"$::GETBUILDTEST(test-type)\" [if $buildReturn "concat failed" "concat succeeded"]"
    if { $::GETBUILDTEST(pack) == "true" } {
        puts "package [if $packageReturn "concat failed" "concat succeeded"]"
    }
}


# upload

if {$::GETBUILDTEST(upload) == "true"} {
    set scpfile "${::GETBUILDTEST(binary-filename)}${::GETBUILDTEST(cpack-extension)}"
    set namic_path "/clients/$::PACKAGE_NAME/WWW/Downloads"
    if {$::GETBUILDTEST(pack) == "true" &&
        [file exists $::PACKAGE_BUILD/$scpfile] &&
        $::GETBUILDTEST(upload) == "true"} {
        puts "About to do a $::GETBUILDTEST(uploadFlag) upload with $scpfile"
    }

    switch $::GETBUILDTEST(uploadFlag) {
        "nightly" {
            # reset the file name - take out the date
            #set ex ".${::GETBUILDTEST(version-patch)}"
            #regsub $ex $scpfile "" scpNightlyFile
            #set scpfile $scpNightlyFile
            set scpdest "${namic_path}/Nightly"
        }
        "snapshot" {
            set scpdest "${namic_path}/Snapshots/$::env(BUILD)"
        }
        "release" {
            set scpdest "${namic_url}/Release/$::env(BUILD)"
        }
        default {
            puts "Invalid ::GETBUILDTEST(uploadFlag) \"$::GETBUILDTEST(uploadFlag)\", setting scpdest to nightly value"
            set scpdest "${namic_path}/Nightly"
        }
    }

    puts " -- upload $scpfile to $scpdest"
    set curlcmd ""
    switch $::tcl_platform(os) {
        "SunOS" -
        "Linux" {
            set scpcmd "/usr/bin/scp $scpfile hayes@na-mic1.bwh.harvard.edu:$scpdest"
        }
        "Darwin" {
            set scpcmd "/usr/bin/scp $scpfile hayes@na-mic1.bwh.harvard.edu:$scpdest"
        }
        default {
            set scpcmd "scp $scpfile hayes@na-mic1.bwh.harvard.edu:$scpdest"
        }
    }

    set scpReturn [catch "eval runcmd [split $scpcmd]"]
    if {$scpReturn} {
        puts "Upload failed..."
    } else {
        puts "See http://www.na-mic.org/Slicer/Download, in the $::GETBUILDTEST(uploadFlag) directory, for the uploaded file."
    }

    #else {
    #    if {$::GETBUILDTEST(verbose)} {
    #    puts "Not uploading $scpfile"
    #    }
    #}

}
