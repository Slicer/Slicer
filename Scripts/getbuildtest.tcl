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
    set msg "$msg\n   -u --update : does a cvs/svn update on each lib"
    set msg "$msg\n   --version-patch : set the patch string for the build (used by installer)"
    set msg "$msg\n                   : default: version-patch is the current date"
    set msg "$msg\n   --tag : same as version-patch"
    set msg "$msg\n   --pack : run cpack after building (default: off)"
    set msg "$msg\n   --upload : set the upload string for the binary, used if pack is true"
    set msg "$msg\n            : snapshot (default), nightly, release"
    set msg "$msg\n   --doxy : just do an svn update on Slicer3 and run doxygen"
    set msg "$msg\n   --verbose : optional, print out lots of stuff, for debugging"
    puts stderr $msg
}

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
set ::GETBUILDTEST(buildList) ""

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
        "--release" {
            set ::GETBUILDTEST(release) "--release"
            set ::VTK_BUILD_TYPE "Release"
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

set cwd [pwd]
cd [file dirname [info script]]
cd ..
set ::Slicer3_HOME [pwd]
cd $cwd
if { $isWindows } {
  set ::Slicer3_HOME [file attributes $::Slicer3_HOME -shortname]
}
set ::Slicer3_LIB $::Slicer3_HOME/../Slicer3-lib
set ::Slicer3_BUILD $::Slicer3_HOME/../Slicer3-build
# use an environment variable so doxygen can use it
set ::env(Slicer3_DOC) $::Slicer3_HOME/../Slicer3-doc


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

if { $::GETBUILDTEST(doxy) && ![file exists $::env(Slicer3_DOC)] } {
    puts "Making documentation directory  $::env(Slicer3_DOC)"
    file mkdir $::env(Slicer3_DOC)
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
cd $::Slicer3_HOME/..
if { [file exists Slicer3] } {
    cd Slicer3
    runcmd svn switch $::Slicer3_TAG
} else {
    runcmd svn checkout $::Slicer3_TAG Slicer3
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
if { $::GETBUILDTEST(release) != "" } {
   append cmd " $::GETBUILDTEST(release)"
} 
if { $::GETBUILDTEST(update) != "" } {
   append cmd " $::GETBUILDTEST(update)"
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
set ::GETBUILDTEST(binary-filename) "Slicer3-3.0.$::GETBUILDTEST(version-patch)-$::env(BUILD)"
if {$::GETBUILDTEST(verbose)} {
    puts "CPack will use $::::GETBUILDTEST(binary-filename)"
}
# set the cpack generator to determine the binary file extension
if {$isLinux || $isDarwin} {
    set ::GETBUILDTEST(cpack-generator) "TGZ"
    set ::GETBUILDTEST(cpack-extension) ".tar.gz"
    # if wish to have .sh, use generator = STGZ and extension = .sh / currently disabled due to Ubuntu bug
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
cd $::Slicer3_BUILD
runcmd $::CMAKE \
        -G$::GENERATOR \
        -DMAKECOMMAND:STRING=$::MAKE \
        -DCMAKE_CXX_COMPILER:STRING=$COMPILER_PATH/$COMPILER \
        -DCMAKE_CXX_COMPILER_FULLPATH:FILEPATH=$COMPILER_PATH/$COMPILER \
        -DITK_DIR:FILEPATH=$ITK_BINARY_PATH \
        -DKWWidgets_DIR:FILEPATH=$Slicer3_LIB/KWWidgets-build \
        -DTEEM_DIR:FILEPATH=$Slicer3_LIB/teem-build \
        -DIGSTK_DIR:FILEPATH=$Slicer3_LIB/IGSTK-build \
        -DINCR_TCL_LIBRARY:FILEPATH=$::INCR_TCL_LIB \
        -DINCR_TK_LIBRARY:FILEPATH=$::INCR_TK_LIB \
        -DSlicer3_USE_PYTHON=OFF \
        -DPYTHON_INCLUDE_PATH:PATH=$::PYTHON_INCLUDE \
        -DPYTHON_LIBRARY:FILEPATH=$::PYTHON_LIB \
        -DSandBox_DIR:FILEPATH=$Slicer3_LIB/NAMICSandBox \
        -DCMAKE_BUILD_TYPE=$::VTK_BUILD_TYPE \
        -DSlicer3_VERSION_PATCH:STRING=$::GETBUILDTEST(version-patch) \
        -DCPACK_GENERATOR:STRING=$::GETBUILDTEST(cpack-generator) \
        -DCPACK_PACKAGE_FILE_NAME:STRING=$::GETBUILDTEST(binary-filename) \
        -DSlicer3_USE_IGSTK=$::IGSTK \
        -DSlicer3_USE_NAVITRACK=$::NAVITRACK \
        -DNAVITRACK_LIB_DIR:FILEPATH=$::NAVITRACK_LIB_DIR \
        -DNAVITRACK_INC_DIR:FILEPATH=$::NAVITRACK_INC_DIR \
        -DSLICERLIBCURL_DIR:FILEPATH=$Slicer3_LIB/cmcurl-build \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=$::GETBUILDTEST(cmake-verbose) \
        $Slicer3_HOME

if { $isWindows } {
    if { $MSVC6 } {
        eval runcmd $::MAKE Slicer3.dsw /MAKE $::GETBUILDTEST(test-type)
        if { $::GETBUILDTEST(pack) == "true" } {
            eval runcmd $::MAKE Slicer3.dsw /MAKE package
        }
    } else {
        # tell cmake explicitly what command line to run when doing the ctest builds
        set makeCmd "$::MAKE Slicer3.sln /build $::VTK_BUILD_TYPE /project ALL_BUILD"
        runcmd $::CMAKE -DMAKECOMMAND:STRING=$makeCmd $Slicer3_HOME

        if { $::GETBUILDTEST(test-type) == "" } {
            runcmd $::MAKE Slicer3.SLN /build $::VTK_BUILD_TYPE
        } else {
            # running ctest through visual studio is broken in cmake2.4, so run ctest directly
            runcmd $::CMAKE_PATH/bin/ctest -D $::GETBUILDTEST(test-type) -C $::VTK_BUILD_TYPE
        }

        if { $::GETBUILDTEST(pack) == "true" } {
            runcmd $::MAKE Slicer3.SLN /build $::VTK_BUILD_TYPE /project PACKAGE
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

    #else {
    #    if {$::GETBUILDTEST(verbose)} {
    #    puts "Not uploading $scpfile"
    #    }
    #}

}
