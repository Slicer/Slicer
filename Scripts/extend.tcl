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
# Initiated - sp - 2008-12-30
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
    set fileName [file tail $c]
    set name [file root $fileName]
    if { [lsearch $::EXTEND(buildList) $name] != -1 } {
      lappend ::EXTEND(s3extFiles) $c
    }
    foreach buildFile $::EXTEND(buildList) {
      if { [file normalize $buildFile] == [file normalize $c] } {
        lappend ::EXTEND(s3extFiles) $c
      }
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
# helper
# - uploads the named file to the extension server

proc upload {fileName} {

  set size [file size $fileName]
  set name [file tail $fileName]

  set fp [open $fileName "r"]
  fconfigure $fp -translation binary -encoding binary
  set data [read $fp]
  close $fp

  set sock [socket ext.slicer.org 8845]
  puts $sock "put $::ext(date) $::env(BUILD) $name $size"
  fconfigure $sock -translation binary -encoding binary
  puts -nonewline $sock $data
  flush $sock
  close $sock
  
  puts "uploaded $fileName ($size bytes)"
}

################################################################################
#
# before building, sort the ext files based on the dependencies they need
# - load the s3ext file parameters into array "ext"
# - build a dependency tree
# - re-export the list in the right order
#

# return 1 if ext1 depends on ext2
# return 0 if not
# return -1 if there is a dependency loop
proc checkDepends {ext1 ext2 dependArray} {
  upvar $dependArray depends
  set dependList $depends($ext1)
  set newDepends $depends($ext1)
  while { $newDepends != "" } {
    set dlist $newDepends
    set newDepends ""
    foreach d $dlist {
      if { $d == $ext1 } {
        return -1
      }
      if { [info exists depends($d)] } {
        set newDepends [concat $newDepends $depends($d)]
      }
    }
    set dependList [concat $dependList $newDepends]
  }
  if { [lsearch $dependList $ext2] != -1 } {
    return 1
  } else {
    return 0
  }
}

# first remove any extensions that depend on something we don't know about
set ::newExtFiles $::EXTEND(s3extFiles)
foreach s3ext $::EXTEND(s3extFiles) {
  array unset ext
  set ::ext(name) [file root [file tail $s3ext]]
  loadArray $s3ext ext
  if { ![info exists ::ext(depends)] } {
    set ::ext(depends) ""
  }
  set ::depends($s3ext) ""
  foreach dependency $::ext(depends) {
    set dependFile $::Slicer3_HOME/Extensions/$dependency.s3ext
    if { ![file exists $dependFile] && ![file exists $::Slicer3_BUILD/Modules/$dependency] } {
      puts stderr "$s3ext depends on non-existent extension $dependency - it will not be built"
      set index [lsearch $::newExtFiles $s3ext]
      set ::newExtFiles [lreplace $::newExtFiles $index $index]
    } else {
      lappend ::depends($s3ext) $dependFile
    }
  }
}

# now remove any circular dependencies
set ::EXTEND(s3extFiles) $::newExtFiles
set ::newExtFiles ""
foreach s3ext $::EXTEND(s3extFiles) {
  if { [checkDepends $s3ext $s3ext ::depends] == -1 } {
    puts stderr "$s3ext depends on itself! it will not be built"
  } else {
    lappend ::newExtFiles $s3ext
  }
}

# now sort the extensions so that dependencies are built first
set ::EXTEND(s3extFiles) $::newExtFiles
set rearranged 1
while { $rearranged } {
  set ::newExtFiles $::EXTEND(s3extFiles)
  foreach s3ext1 $::EXTEND(s3extFiles) {
    foreach s3ext2 $::EXTEND(s3extFiles) {
      if { [checkDepends $s3ext1 $s3ext2 ::depends] == 1 } {
        set ext1index [lsearch $newExtFiles $s3ext1] 
        set ext2index [lsearch $newExtFiles $s3ext2]
        if { $ext1index < $ext2index } {
          # here ext1 depends on ext2, but it is earlier in the list
          # - so move ext2 to the front of the list and start over
          set ::newExtFiles [lreplace $newExtFiles $ext2index $ext2index]
          set ::newExtFiles [concat $s3ext2 $::newExtFiles]
          puts "moved [file tail $s3ext2] to front because it is needed by [file tail $s3ext1]"
        }
      } else {
      }
    }
  }
  if { $::newExtFiles == $::EXTEND(s3extFiles) } {
    set rearranged 0
  } else {
    set ::EXTEND(s3extFiles) $::newExtFiles
  }
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
  set ::ext(name) [file root [file tail $s3ext]]
  set ::ext(scm) ""
  loadArray $s3ext ext

  set ::ext(date) [clock format [clock seconds] -format %Y-%m-%d]


  # make dirs, delete if asked for clean build
  foreach suffix {"" -build -install} {
    set dir $::Slicer3_EXT/$::ext(name)$suffix
    if { $::EXTEND(clean) && $suffix != "" } {
      puts "Deleting $dir..."
      file delete -force $dir
    }
    if { ![file exists $dir] } {
      file mkdir $dir
    }
  }

  # check out code
  # - set array variable srcDir
  cd $::Slicer3_EXT/$::ext(name)
  switch $::ext(scm) {
    "cvs" {
      runcmd $::CVS -d $::ext(cvsroot) co $::ext(cvsmodule)
      set ::ext(srcDir) $::Slicer3_EXT/$::ext(name)/$::ext(cvsmodule)
    }
    "svn" {
      runcmd $::SVN co $::ext(svnpath) $::ext(name)
      set ::ext(srcDir) $::Slicer3_EXT/$::ext(name)/$::ext(name)
    }
    default {
      puts stderr "No source code control tool specified in $s3ext"
      set ::ext(srcDir) ""
    }
  }

  if { $::ext(srcDir) == "" } {
    continue
  }

  if { $isWindows } {
    set make [file attributes $::MAKE -shortname]
    set makeCmd "$make $::ext(name).sln /build $::VTK_BUILD_TYPE /project ALL_BUILD"
  } else {
    set makeCmd $::MAKE
  }

  set dependPaths ""
  foreach dep $ext(depends) {
    if { [file exists $Slicer3_HOME/Modules/$dep] } {
      # this is a module that comes with slicer
      set dependPaths "$dependPaths -D${dep}_SOURCE_DIR=$Slicer3_HOME/Modules/$dep"
      set dependPaths "$dependPaths -D${dep}_BINARY_DIR=$Slicer3_BUILD/Modules/$dep"
    } else {
      set dependPaths "$dependPaths -D${dep}_SOURCE_DIR=$Slicer3_EXT/$dep/$dep"
      set dependPaths "$dependPaths -D${dep}_BINARY_DIR=$Slicer3_EXT/$dep-build"
    }
  }

  set extraLink ""
  if { $isDarwin } {
    set extraLink "$extraLink -DCMAKE_SHARED_LINKER_FLAGS:STRING=-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
    set extraLink "$extraLink -DCMAKE_EXE_LINKER_FLAGS=-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
  }

  # configure project and make
  cd $::Slicer3_EXT/$::ext(name)-build
  eval runcmd $::CMAKE \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
    -DSlicer3_DIR:PATH=$::Slicer3_BUILD \
    -DBUILD_AGAINST_SLICER3:BOOL=ON \
    -DMAKECOMMAND:STRING=$makeCmd \
    -DCMAKE_INSTALL_PREFIX:PATH=$::Slicer3_EXT/$::ext(name)-install \
    $dependPaths \
    $extraLink \
    $::ext(srcDir)

  # build the project
  cd $::Slicer3_EXT/$::ext(name)-build
  if { $isWindows } {
    runcmd "$::MAKE" $::ext(name).sln /build $::VTK_BUILD_TYPE /project ALL_BUILD
  } else {
    eval runcmd $::MAKE
  }

  # run the tests
  # - not all modules have tests, so allow make to fail gracefully with catch
  cd $::Slicer3_EXT/$::ext(name)-build
  if { $::EXTEND(test-type) != "" } {
    if { $isWindows } {
      set ret [catch "runcmd $::MAKE $::ext(name).sln /build $::VTK_BUILD_TYPE /project $::EXTEND(test-type)" res]
    } else {
      set ret [catch "eval runcmd $::MAKE $::EXTEND(test-type)" res]
    }
  }

  if { $ret } {
    puts stderr "ERROR building $::ext(name)"
    puts stderr $res
  }

  # run the install target
  cd $::Slicer3_EXT/$::ext(name)-build
  if { $isWindows } {
    runcmd $::MAKE $::ext(name).sln /build $::VTK_BUILD_TYPE /project INSTALL
  } else {
    eval runcmd $::MAKE install
  }

  # make the zip file
  # - TODO: first, write a config file that describes the build machine
  set dir $::Slicer3_EXT/$::ext(name)-install/lib/Slicer3
  foreach dirType {Modules Plugins} {
    if { [file exists $dir/$dirType] } {
      set dir $dir/$dirType
      break
    }
  }
  cd $dir
  set ::ext(zipFileName) $dir/$::ext(name)-$::ext(date)-$::env(BUILD).zip 
  runcmd zip -r9 $::ext(zipFileName) .

  # upload it
  # - read zip file into 'data' variable
  # - write it to a socket on the ext.slicer.org server
  #

  upload $::ext(zipFileName)  
  upload $s3ext
}

