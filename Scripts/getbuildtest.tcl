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

proc Usage { {msg ""} } {
    global SLICER
    
    set msg "$msg\nusage: getbuildtest \[options\] \[target\]"
    set msg "$msg\n  \[target\] is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   h --help : prints this message and exits"
    set msg "$msg\n   -f --clean : delete lib and build directories first"
    set msg "$msg\n   -t --test-type : CTest test target (default: Experimental)"
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   --relwithdebinfo : compile with optimization flags and debugging symbols"
    set msg "$msg\n   -u --update : does a cvs/svn update on each lib"
    set msg "$msg\n   --no-slicer-update : don't update slicer source (does not effect libs)"
    set msg "$msg\n   --build-dir : override default build directory"
    set msg "$msg\n   --doc-dir : override default documentation directory"
    set msg "$msg\n   --version-patch : set the patch string for the build (used by installer)"
    set msg "$msg\n                   : default: version-patch is the current date"
    set msg "$msg\n   --tag : same as version-patch"
    set msg "$msg\n   --pack : run cpack after building (default: off)"
    set msg "$msg\n   --upload : set the upload string for the binary, used if pack is true"
    set msg "$msg\n            : snapshot (default), nightly, release"
    set msg "$msg\n   --doxy : just do an svn update on Slicer3 and run doxygen"
    set msg "$msg\n   --verbose : optional, print out lots of stuff, for debugging"
    set msg "$msg\n   --rpm : optional, specify CPack RPM generator for packaging"
    set msg "$msg\n   --deb : optional, specify CPack DEB generator for packaging"
    set msg "$msg\n   -e --extend : optional, build external modules using the extend script"
    set msg "$msg\n   -32 -64 : Set if we want to build Slicer 32 or 64 bits" 
    set msg "$msg\n            : The default on Solaris is the current bitness of the underlying kernel (isainfo -b)"
    set msg "$msg\n            : The default on Linux is the current bitness of the underlying kernel"
    set msg "$msg\n            : 32 bits on other platforms"
    set msg "$msg\n   --gcc --suncc : Set the desired compiler for the build process"
    set msg "$msg\n            : The default is gcc/g++"
    puts stderr $msg
}

set ::GETBUILDTEST(clean) "false"
set ::GETBUILDTEST(update) ""
set ::GETBUILDTEST(no-slicer-update) ""
set ::GETBUILDTEST(build-dir) ""
set ::GETBUILDTEST(doc-dir) ""
set ::GETBUILDTEST(release) ""
set ::GETBUILDTEST(test-type) "Experimental"
set ::GETBUILDTEST(version-patch) ""
set ::GETBUILDTEST(pack) "false"
set ::GETBUILDTEST(upload) "false"
set ::GETBUILDTEST(uploadFlag) "nightly"
set ::GETBUILDTEST(doxy) "false"
set ::GETBUILDTEST(verbose) "false"
set ::GETBUILDTEST(buildList) ""
set ::GETBUILDTEST(cpack-generator) ""
set ::GETBUILDTEST(rpm-spec) ""
set ::GETBUILDTEST(extend) "false"
set ::GETBUILDTEST(compiler) ""
set ::GETBUILDTEST(bitness) "32"
switch $::tcl_platform(os) {
    "SunOS" { 
        set isainfo [exec isainfo -b]
        set ::GETBUILDTEST(bitness) "$isainfo"
        set ::GETBUILDTEST(compiler) "gcc"
    }
    "Linux" {           
        if {$::tcl_platform(machine) == "x86_64"} {
            set ::GETBUILDTEST(bitness) 64
        }
    }
}
 
set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--clean" -
        "-f" {
            set ::GETBUILDTEST(clean) "true"
        }
        "--update" -
        "-u" {
            set ::GETBUILDTEST(update) "--update"
        }
        "--no-slicer-update" {
            set ::GETBUILDTEST(no-slicer-update) "--no-slicer-update"
        }
        "--release" {
            set ::GETBUILDTEST(release) "--release"
            set ::VTK_BUILD_TYPE "Release"
        }
        "--relwithdebinfo" {
            set ::GETBUILDTEST(release) "--relwithdebinfo"
            set ::VTK_BUILD_TYPE "RelWithDebInfo"
        }
        "--build-dir" {
            incr i
            if { $i == $argc } {
                Usage "Missing build-dir argument"
            } else {
                set ::GETBUILDTEST(build-dir) [lindex $argv $i]
            }
        }
        "--doc-dir" {
            incr i
            if { $i == $argc } {
                Usage "Missing doc-dir argument"
            } else {
                set ::GETBUILDTEST(doc-dir) [lindex $argv $i]
            }
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
        "--extend" -
        "-e" {
            set ::GETBUILDTEST(extend) "true"
        }
        "-64" {
            set ::GETBUILDTEST(bitness) "64"
        }
        "-32" {
            set ::GETBUILDTEST(bitness) "32"
        }
        "--suncc" {
          set ::GETBUILDTEST(compiler) "suncc"
        }
        "--gcc" {
            set ::GETBUILDTEST(compiler) "gcc"
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
#    Usage
#    exit 1
}
# puts "getbuildtest: setting build list to $strippedargs"
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

set cwd [pwd]
cd [file dirname [info script]]
cd ..
set ::Slicer3_HOME [pwd]
cd $cwd
if { $isWindows } {
  set ::Slicer3_HOME [file attributes $::Slicer3_HOME -shortname]
}
if { $::GETBUILDTEST(build-dir) == ""} {
        # use an enviornment variables so slicer-variables.tcl can see them
        set ::env(Slicer3_LIB) $::Slicer3_HOME/../Slicer3-lib
        set ::env(Slicer3_BUILD) $::Slicer3_HOME/../Slicer3-build
} else {
        # use an enviornment variables so slicer-variables.tcl can see them
        set ::env(Slicer3_LIB) $::GETBUILDTEST(build-dir)/Slicer3-lib
        set ::env(Slicer3_BUILD) $::GETBUILDTEST(build-dir)/Slicer3-build
}

if { $::GETBUILDTEST(doc-dir) == ""} {
    # check if there's an environment variable
    if { [info exists ::env(Slicer3_DOC)] == 1 } {
        set ::GETBUILDTEST(doc-dir) $::env(Slicer3_DOC)
    } else {
        # set an default value
        set ::GETBUILDTEST(doc-dir)  $::Slicer3_HOME/../Slicer3-doc
    }
}
# set an environment variable so doxygen can use it
set ::env(Slicer3_DOC) $::GETBUILDTEST(doc-dir)


set Slicer3_LIB $::env(Slicer3_LIB) 
set Slicer3_BUILD $::env(Slicer3_BUILD) 

#######
#
# Note: the local vars file, slicer2/slicer_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't 
#   be overwritten when this file is updated
#
set localvarsfile $Slicer3_HOME/slicer_variables.tcl
catch {set localvarsfile [file normalize $localvarsfile]}
if { [file exists $localvarsfile] } {
    puts "Sourcing $localvarsfile"
    source $localvarsfile
} else {
    puts "stderr: $localvarsfile not found - use this file to set up your build"
    exit 1
}

puts "making with $::MAKE"


#
# Deletes both Slicer3_LIB and Slicer3_BUILD if clean option given
#
# tcl file delete is broken on Darwin, so use rm -rf instead
if { $::GETBUILDTEST(clean) } {
    puts "Deleting slicer lib files..."
    if { $isDarwin } {
        runcmd rm -rf $Slicer3_LIB
        runcmd rm -rf $Slicer3_BUILD
        if { [file exists $Slicer3_LIB/tcl/isPatched] } {
            runcmd rm $Slicer3_LIB/tcl/isPatched
        }

        if { [file exists $Slicer3_LIB/tcl/isPatchedBLT] } {
            runcmd rm $Slicer3_LIB/tcl/isPatchedBLT
        }
    } else {
        file delete -force $Slicer3_LIB
        file delete -force $Slicer3_BUILD
    }
}

if { ![file exists $Slicer3_LIB] } {
    file mkdir $Slicer3_LIB
}

if { ![file exists $Slicer3_BUILD] } {
    file mkdir $Slicer3_BUILD
}

if { $::GETBUILDTEST(doxy) } {
    if {[file exists $::env(Slicer3_DOC)] } {
        # force removal of the old dir
        puts "Clearing out old documentation directory $::env(Slicer3_DOC)"
        file delete -force  $::env(Slicer3_DOC)
    }
    if {![file exists $::env(Slicer3_DOC)] } {
        puts "Making documentation directory  $::env(Slicer3_DOC)"
        file mkdir $::env(Slicer3_DOC)
    }
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
if { $::GETBUILDTEST(test-type) != "Continuous" && $::GETBUILDTEST(no-slicer-update) == ""} {
  cd $::Slicer3_HOME/..
  if { [file exists Slicer3] } {
    cd Slicer3
    runcmd svn switch $::Slicer3_TAG
  } else {
    runcmd svn checkout $::Slicer3_TAG Slicer3
  }
} else {
  puts "Skipping update of Slicer3 (if a continuous test is select then Slicer3 source will be updated when the test starts)."
}

if { $::GETBUILDTEST(doxy) } {
    # just run doxygen and exit
    puts "Creating documenation files in $::env(Slicer3_DOC)"
    set cmd "doxygen $::Slicer3_HOME/Doxyfile"
    eval runcmd $cmd
    return
}


# build the lib with options
cd $::Slicer3_HOME
set cmd "sh ./Scripts/genlib.tcl $Slicer3_LIB"

if { $::GETBUILDTEST(compiler) != "" } {
  append cmd " --$::GETBUILDTEST(compiler)"
}
append cmd " -$::GETBUILDTEST(bitness)"

if { $::GETBUILDTEST(release) != "" } {
   append cmd " $::GETBUILDTEST(release)"
} 
if { $::GETBUILDTEST(update) != "" } {
   append cmd " $::GETBUILDTEST(update)"
} 
if { $::GETBUILDTEST(test-type) != "" } {
   append cmd " --test-type $::GETBUILDTEST(test-type)"
} 
if { $::GETBUILDTEST(buildList) != "" } {
    # puts "Passing $::GETBUILDTEST(buildList) to genlib"
    append cmd " $::GETBUILDTEST(buildList)"
}

set retval [catch "eval runcmd $cmd"]
if {$retval == 1} {
  puts "ERROR: failed to run getbuildtest: $cmd"
  return
}

if { $::GETBUILDTEST(version-patch) == "" } {
  # TODO: add build type (win32, etc) here...
  set ::GETBUILDTEST(version-patch) [clock format [clock seconds] -format %Y-%m-%d]
}

# set the binary filename root
set ::GETBUILDTEST(binary-filename) "Slicer3-3.5-alpha-$::GETBUILDTEST(version-patch)-$::env(BUILD)"
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
        set ::GETBUILDTEST(rpm-spec) "%define __spec_install_post /bin/true"

        if { $::GETBUILDTEST(cpack-generator) == "RPM" } {
            set ::GETBUILDTEST(cpack-extension) ".rpm"
        }
        if { $::GETBUILDTEST(cpack-generator) == "DEB" } {
            set ::GETBUILDTEST(cpack-extension) ".deb"
        }
    } 
}
if {$isSolaris} {
    set ::GETBUILDTEST(cpack-generator) "TGZ"
    set ::GETBUILDTEST(cpack-extension) ".tar.gz"
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

# build the slicer
# - first run cmake
# - create the Slicer3Version.txt file
# - then run plaftorm specific build command

if { $::USE_SYSTEM_PYTHON } {
  set ::Slicer3_USE_SYSTEM_PYTHON ON
} else {
  set ::Slicer3_USE_SYSTEM_PYTHON OFF
}

cd $::Slicer3_BUILD
runcmd $::CMAKE \
        -G$::GENERATOR \
        -DMAKECOMMAND:STRING=$::MAKE \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        -DKWWidgets_DIR:FILEPATH=$Slicer3_LIB/KWWidgets-build \
        -DOpenCV_DIR:FILEPATH=$Slicer3_LIB/OpenCV-build \
        -DTeem_DIR:FILEPATH=$Slicer3_LIB/teem-build \
        -DOpenIGTLink_DIR:FILEPATH=$Slicer3_LIB/OpenIGTLink-build \
        -DBatchMake_DIR:FILEPATH=$Slicer3_LIB/BatchMake-build \
        -DSlicer3_USE_BatchMake=ON \
        -DINCR_TCL_LIBRARY:FILEPATH=$::INCR_TCL_LIB \
        -DINCR_TK_LIBRARY:FILEPATH=$::INCR_TK_LIB \
        -DSlicer3_USE_PYTHON=$::USE_PYTHON \
        -DSlicer3_USE_SYSTEM_PYTHON=$::Slicer3_USE_SYSTEM_PYTHON \
        -DSlicer3_USE_NUMPY=$::USE_NUMPY \
        -DSlicer3_USE_OPENIGTLINK=$::USE_OPENIGTLINK \
        -DSlicer3_USE_OPENCV=$::USE_OPENCV \
        -DPYTHON_INCLUDE_PATH:PATH=$::PYTHON_INCLUDE \
        -DPYTHON_LIBRARY:FILEPATH=$::PYTHON_LIB \
        -DSandBox_DIR:FILEPATH=$Slicer3_LIB/NAMICSandBox \
        -DCMAKE_BUILD_TYPE=$::VTK_BUILD_TYPE \
        -DSlicer3_VERSION_PATCH:STRING=$::GETBUILDTEST(version-patch) \
        -DCPACK_GENERATOR:STRING=$::GETBUILDTEST(cpack-generator) \
        -DCPACK_PACKAGE_FILE_NAME:STRING=$::GETBUILDTEST(binary-filename) \
        -DCPACK_RPM_SPEC_MORE_DEFINE=$::GETBUILDTEST(rpm-spec) \
        -DSLICERLIBCURL_DIR:FILEPATH=$Slicer3_LIB/cmcurl-build \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=$::GETBUILDTEST(cmake-verbose) \
        -DLAUNCHER_REPORT_VTK_ERRORS=$::LAUNCHER_REPORT_VTK_ERRORS \
        $Slicer3_HOME


#
# run the versioner script to create the Slicer3Version.txt file
# that tells what slicer3 build these newly built extensions 
# are compatibile with
#

set cmd "sh $::Slicer3_HOME/Scripts/versioner.tcl"
set retval [catch "eval runcmd $cmd" res]
if {$retval == 1} {
  puts "ERROR: failed to run versioner script: $cmd"
  puts "$res"
  return
}

#
# now do the actual build
#

if { $isWindows } {

    # TODO: this needs to be touched when upgrading python versions
    # Here we put a copy of the python dll into a spot we know will be in
    # the runtime path for GenerateCLP (which depends on python).
    # We can't put this in the system path, since developer studio ignores
    # the environment variables (or if you pass it the /UseEnv flag it
    # can't find it's own executables).
    if { $::USE_PYTHON == "ON" } {
      if { ![file exists bin] } { file mkdir bin }
      if { ![file exists bin/$::VTK_BUILD_TYPE] } { file mkdir bin/$::VTK_BUILD_TYPE }
      if { ![file exists bin/$::VTK_BUILD_TYPE/python26.dll] } { 
        file copy $::Slicer3_LIB/python-build/PCbuild/python26.dll bin/$::VTK_BUILD_TYPE 
      }
    }

    if { $MSVC6 } {
        eval runcmd $::MAKE Slicer3.dsw /MAKE $::GETBUILDTEST(test-type)
        if { $::GETBUILDTEST(pack) == "true" } {
            eval runcmd $::MAKE Slicer3.dsw /MAKE package
        }
    } else {
        # tell cmake explicitly what command line to run when doing the ctest builds
        set makeCmd "$::MAKE Slicer3.sln /out buildlog.txt /build $::VTK_BUILD_TYPE /project ALL_BUILD"
        runcmd $::CMAKE -DCTEST_TEST_TIMEOUT=180 -DMAKECOMMAND:STRING=$makeCmd $Slicer3_HOME 

        if { $::GETBUILDTEST(test-type) == "" } {
            runcmd $::MAKE Slicer3.SLN /out buildlog.txt /build $::VTK_BUILD_TYPE
        } else {
            # running ctest through visual studio is broken in cmake2.4, so run ctest directly
            runcmd $::CMAKE_PATH/bin/ctest -D $::GETBUILDTEST(test-type) -C $::VTK_BUILD_TYPE
        }

        if { $::GETBUILDTEST(pack) == "true" } {
            runcmd $::MAKE Slicer3.SLN /out packlog.txt /build $::VTK_BUILD_TYPE /project PACKAGE
        }
    }
} else {
    set buildReturn [catch "eval runcmd $::MAKE $::GETBUILDTEST(test-type)"]
    if { $::GETBUILDTEST(pack) == "true" } {
        set packageReturn [catch "eval runcmd $::MAKE package"]
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
    set namic_path "/clients/Slicer3/WWW/Downloads"
    if {$::GETBUILDTEST(pack) == "true" &&  
        [file exists $::Slicer3_BUILD/$scpfile] && 
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
}


#
# build slicer extensions if requested on the command line
#

if { $::GETBUILDTEST(extend) == "true" } {
  # build the slicer3 extensions
  cd $::Slicer3_HOME
  set cmd "sh ./Scripts/extend.tcl $::GETBUILDTEST(test-type) $::GETBUILDTEST(release)"
  eval runcmd $cmd
}

