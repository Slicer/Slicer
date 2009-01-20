#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# extend.tcl
#
# does an update and a clean build of slicer3 (including utilities and libs)
# then does a dashboard submission
#
# Usage:
#   extend [options] [target]
#
# Initiated - sp - 2006-05-11
#

################################################################################
#
# simple command line argument parsing
#

proc Usage { {msg ""} } {
    global SLICER
    
    set msg "$msg\nusage: extend \[options\] \[target\]..."
    set msg "$msg\n  \[target\] is determined automatically if not specified"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   h --help : prints this message and exits"
    set msg "$msg\n   -f --clean : delete lib and build directories first"
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   --relwithdebinfo : compile with optimization flags and debugging symbols"
    set msg "$msg\n   -u --update : does a cvs/svn update on each lib"
    set msg "$msg\n   --verbose : optional, print out lots of stuff, for debugging"
    puts stderr $msg
}

set ::EXTEND(clean) "false"
set ::EXTEND(update) ""
set ::EXTEND(release) ""
set ::EXTEND(verbose) "false"
set ::EXTEND(test-type) "Experimental"
set ::EXTEND(buildList) ""

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


set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--clean" -
        "-f" {
            set ::EXTEND(clean) "true"
        }
        "--update" -
        "-u" {
            set ::EXTEND(update) "--update"
        }
        "--release" {
            set ::EXTEND(release) "--release"
            set ::VTK_BUILD_TYPE "Release"
        }
        "--relwithdebinfo" {
            set ::EXTEND(release) "--relwithdebinfo"
            set ::VTK_BUILD_TYPE "RelWithDebInfo"
        }
        "-t" -
        "--test-type" {
            incr i
            if { $i == $argc } {
                Usage "Missing test-type argument"
            } else {
                set ::EXTEND(test-type) [lindex $argv $i]
            }
        }
        "--verbose" {
            set ::EXTEND(verbose) "true"
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
set ::EXTEND(buildList) $strippedargs


if { $::EXTEND(verbose) } {
  puts [parray ::EXTEND]
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
set ::Slicer3_EXT $::Slicer3_HOME/../Slicer3-ext

#######
#
# Note: the local vars file, Slicer3/slicer_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't 
#   be overwritten when this file is updated
#
set localvarsfile $Slicer3_HOME/slicer_variables.tcl
catch {set localvarsfile [file normalize $localvarsfile]}
if { [file exists $localvarsfile] } {
  if { $::EXTEND(verbose) } {
    puts "Sourcing $localvarsfile"
  }
  source $localvarsfile
} else {
  puts "stderr: $localvarsfile not found - use this file to set up your build"
  exit 1
}

if { $::EXTEND(verbose) } {
  puts "making with $::MAKE"
}


#
# Deletes Slicer3_EXT if clean option given
#
if { $::EXTEND(clean) } {
  if { $::EXTEND(verbose) } {
    puts "Deleting $::Slicer3_EXT..."
  }
  if { $isDarwin } {
    # tcl file delete is broken on Darwin, so use rm -rf instead
    runcmd rm -rf $::Slicer3_EXT
  } else {
    file delete -force $::Slicer3_EXT
  }
}
if { ![file exists $::Slicer3_EXT] } {
    file mkdir $::Slicer3_EXT
}

################################################################################
# discover the list of extensions

set ::EXTEND(s3extFiles) ""
set candidates [glob -nocomplain $::Slicer3_HOME/Extensions/*.s3ext]
foreach c $candidates {
  if { $::EXTEND(buildList) == "" } {
    lappend ::EXTEND(s3extFiles) $c
  } else {
    set name [file tail [file root $c]]
    if { [lsearch $::EXTEND(buildList) $name] != -1 } {
      lappend ::EXTEND(s3extFiles) $c
    }
  }
}

if { $::EXTEND(verbose) } {
  puts "Will build: $::EXTEND(s3extFiles)"
}


################################################################################
# helper
# - load the file into the named array
# - ignore lines that start with # and blank lines
# - first token on line is array key, rest of line is value

proc loadArray {fileName arrayName} {
  upvar $arrayName a
  set fp [open $fileName "r"]
  while { ![eof $fp] } {
    gets $fp line
    set line [string trim $line]
    if { $line != "" && [string index $line 0] != "#" } {
      set name [lindex $line 0]
      set a($name) [lrange $line 1 end]
    }
  }
  close $fp
}

################################################################################
#
# the actual build and test commands for each module
# - load the s3ext file parameters into array "ext"
# - checkout the source code
# - configure the project to point to Slicer3_BUILD
# - build the project
# - run the tests
# - run the install target
# - make the zip file
# - upload it 
#

foreach s3ext $::EXTEND(s3extFiles) {

  # collect params
  array unset ext
  set ext(name) [file root [file tail $s3ext]]
  set ext(scm) ""
  loadArray $s3ext ext
  set ext(date) [clock format [clock seconds] -format %Y-%m-%d]

  # make dirs
  foreach suffix {"" -build -install} {
    set dir $::Slicer3_EXT/$ext(name)$suffix
    if { ![file exists $dir] } {
      file mkdir $dir
    }
  }

  # check out code
  # - set array variable srcDir
  cd $::Slicer3_EXT/$ext(name)
  switch $ext(scm) {
    "cvs" {
      runcmd $::CVS -d $ext(cvsroot) co $ext(cvsmodule)
      set ext(srcDir) $::Slicer3_EXT/$ext(name)/$ext(cvsmodule)
    }
    "svn" {
      runcmd $::SVN co $ext(svnpath) $ext(name)
      set ext(srcDir) $::Slicer3_EXT/$ext(name)/$ext(name)
    }
    default {
      puts stderr "No source code control tool specified in $s3ext"
      set ext(srcDir) ""
    }
  }

  if { $ext(srcDir) == "" } {
    continue
  }

  # configure project and make
  cd $::Slicer3_EXT/$ext(name)-build
  runcmd $::CMAKE \
    -DSlicer3_DIR:PATH=$::Slicer3_BUILD \
    -DBUILD_AGAINST_SLICER3:BOOL=ON \
    -DCMAKE_INSTALL_PREFIX:PATH=$::Slicer3_EXT/$ext(name)-install \
    $ext(srcDir)

  # build the project
  cd $::Slicer3_EXT/$ext(name)-build
  if { $isWindows } {
    runcmd $::MAKE $ext(name).sln /build $::VTK_BUILD_TYPE /project ALL_BUILD
  } else {
    eval runcmd $::MAKE
  }

  # run the tests
  # - not all modules have tests, so allow make to fail gracefully with catch
  cd $::Slicer3_EXT/$ext(name)-build
  if { $::EXTEND(test-type) != "" } {
    if { $isWindows } {
      set ret [catch "runcmd $::MAKE $ext(name).sln /build $::VTK_BUILD_TYPE /project $::EXTEND(test-type)" res]
    } else {
      set ret [catch "eval runcmd $::MAKE $::EXTEND(test-type)" res]
    }
  }

  if { $ret } {
    puts stderr "ERROR building $::ext(name)"
    puts stderr $res
  }

  # run the install target
  cd $::Slicer3_EXT/$ext(name)-build
  if { $isWindows } {
    runcmd $::MAKE $ext(name).sln /build $::VTK_BUILD_TYPE /project INSTALL
  } else {
    eval runcmd $::MAKE install
  }

  # make the zip file
  # - TODO: first, write a config file that describes the build machine
  set dir $::Slicer3_EXT/$ext(name)-install/lib/Slicer3
  foreach dirType {Modules Plugins} {
    if { [file exists $dir/$dirType] } {
      set dir $dir/$dirType
      break
    }
  }
  cd $dir
  set ext(zipFileName) $dir/$ext(name)-$ext(date)-$::env(BUILD).zip 
  runcmd zip -r9 $ext(zipFileName) .

  # upload it
  # - read zip file into 'data' variable
  # - write it to a socket on the ext.slicer.org server
  #
  #runcmd curl --data-binary @$ext(zipFileName) http://ext.slicer.org/getfile.html?name=$ext(name)&build=$::env(BUILD)&date=$ext(date)
  set size [file size $ext(zipFileName)]
  set name [file tail $ext(zipFileName)]

  set fp [open $ext(zipFileName) "r"]
  fconfigure $fp -translation binary -encoding binary
  set data [read $fp]
  close $fp

  set sock [socket ext.slicer.org 8845]
  puts $sock "put $ext(date) $::env(BUILD) $name $size"
  fconfigure $sock -translation binary -encoding binary
  puts -nonewline $sock $data
  flush $sock
  close $sock
  
  puts "uploaded $ext(zipFileName) ($size bytes)"
}

