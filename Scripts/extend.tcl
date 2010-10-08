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
    set msg "$msg\n   --release : compile with optimization flags"
    set msg "$msg\n   --relwithdebinfo : compile with optimization flags and debugging symbols"
    set msg "$msg\n   -u --update : does a cvs/svn update on each lib"
    set msg "$msg\n   --quiet : turns off debugging messages"
    set msg "$msg\n   --no-extension-update : disables svn checkout for the extension"
    set msg "$msg\n   --upload : upload the script to the extension server"
    puts stderr $msg
}

set ::EXTEND(update) ""
set ::EXTEND(release) ""
set ::EXTEND(verbose) "true"
set ::EXTEND(test-type) ""
set ::EXTEND(buildList) ""
set ::EXTEND(no-extension-update) ""
set ::EXTEND(upload) "false"

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
        "--quiet" {
            set ::EXTEND(verbose) "false"
        }
        "--no-extension-update" {
            set ::EXTEND(no-extension-update) "true"
        }
        "--upload" {
            set ::EXTEND(upload) "true"
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


# only print if verbose flag is set
proc vputs {s} {
  if { $::EXTEND(verbose) } {
    puts $s
  }
}


################################################################################
#
# Utilities:

proc runcmd {args} {
    puts "running: $args"

    # print the results line by line to provide feedback during long builds
    # interleaves the results of stdout and stderr, except on Windows
    if { $::isWindows } {
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
        if { $::isWindows } {
            # Does not work on Windows
        } else {
            if { $res != "child killed: write on pipe with no readers" } {
              error $ret
            }
        }
    } 
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



#initialize platform variables
foreach v { ::isSolaris ::isWindows ::isDarwin ::isLinux } { set $v 0 }
switch $tcl_platform(os) {
    "SunOS" { set ::isSolaris 1 }
    "Linux" { set ::isLinux 1 }
    "Darwin" { set ::isDarwin 1 }
    default { set ::isWindows 1 }
}

################################################################################
# First, set up the directory
# - determine the location
# - determine the build
# 

set cwd [pwd]
cd [file dirname [info script]]
cd ..
set ::Slicer_HOME [pwd]
cd $cwd
if { $::isWindows } {
  set ::Slicer_HOME [file attributes $::Slicer_HOME -shortname]
}

set ::Slicer_LIB $::Slicer_HOME/../Slicer3-lib
set ::Slicer_BUILD $::Slicer_HOME/../Slicer-build
set ::Slicer_EXT $::Slicer_HOME/../Slicer3-ext

#######
#
# Note: the local vars file, Slicer3/slicer_variables.tcl, overrides the default values in this script
# - use it to set your local environment and then your change won't 
#   be overwritten when this file is updated
#
set localvarsfile $::Slicer_HOME/slicer_variables.tcl
catch {set localvarsfile [file normalize $localvarsfile]}
if { [file exists $localvarsfile] } {
  vputs "Sourcing $localvarsfile"
  source $localvarsfile
} else {
  puts "stderr: $localvarsfile not found - use this file to set up your build"
  exit 1
}

if { $::isWindows } {
  set ::MAKE [file attributes $::MAKE -shortname]
}
vputs "making with $::MAKE"

# get the slicer version information
loadArray $::Slicer_BUILD/lib/Slicer3/SlicerVersion.txt slicerVersion
set len [string length "http://svn.slicer.org/Slicer3/"]
set ::EXTEND(slicerSVNSubpath) [string range $slicerVersion(svnurl) $len end]
set ::EXTEND(slicerSVNRevision) $slicerVersion(svnrevision)


# Fill the Slicer3-ext/Extensions directory with .s3ext files for this build.
# They come from two places:
# - the Extensions subdirectory in the Slicer3 source tree
# - the svn repository branch that corresponds to the version currently
#   being built (*)
#
# (*) why? By storing the extensions in distinct svn directory we can add
# .s3ext files to the release branch without incrementing the svn revision
# number of the Slicer3 source code.  This means that when we make a release version
# of slicer available it can be uniquely identified by it's svn revision number
# which defines the exact ABI of slicer, it's libs, and all the other libs
# since the exact build is specified by the files under svn control.  The extensions 
# layer on top of that.

if { ![file exists $::Slicer_EXT] } {
  file mkdir $::Slicer_EXT
}
cd $::Slicer_EXT
if { ![file exists Extensions] } {
  file mkdir Extensions
}

# try to check out the extension that corresponds to our build branch
if { [string match "branches*" $::EXTEND(slicerSVNSubpath)] } {
  set len [string length "branches/"]
  set branch [string range $::EXTEND(slicerSVNSubpath) $len end]
  set svncmd "$::SVN checkout http://svn.slicer.org/Slicer3/branches/Extensions/$branch Extensions-checkout"
  set ret [catch "runcmd $svncmd" res]
  if { $ret } {
    vputs "svn command failed:\n$res"
  } else {
  }
}

# get the files from the checked out Extensions branch
set extFiles [glob -nocomplain $::Slicer_EXT/Extensions-checkout/*.s3ext]
foreach f $extFiles {
  file copy -force $f $::Slicer_EXT/Extensions
}

# get the files from the Slicer3 source tree
set extFiles [glob -nocomplain $::Slicer_HOME/Extensions/*.s3ext]
foreach f $extFiles {
  file copy -force $f $::Slicer_EXT/Extensions
}


################################################################################
# discover the list of extensions
#
# the buildList from the command line can be:
# - the exact file name of an s3ext file (absolute or relative path)
# - the <name> such that Extensions/<name>.s3ext exists
#

set ::EXTEND(s3extFiles) ""
set candidates [glob -nocomplain $::Slicer_EXT/Extensions/*.s3ext]
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

vputs "Will build: $::EXTEND(s3extFiles)"


################################################################################
# helper
# - uploads the named file to the extension server

proc upload {fileName} {

  vputs "Uploading $fileName to ext.slicer.org port 8845..."
  flush stdout

  set size [file size $fileName]
  set name [file tail $fileName]

  set fp [open $fileName "r"]
  fconfigure $fp -translation binary -encoding binary
  set data [read $fp]
  close $fp

  set sock [socket ext.slicer.org 8845]
  puts $sock "put $::EXTEND(slicerSVNSubpath) $::EXTEND(slicerSVNRevision)-$::env(BUILD) $name $size"
  fconfigure $sock -translation binary -encoding binary
  puts -nonewline $sock $data
  flush $sock
  close $sock
  
  vputs "uploaded $fileName ($size bytes)"
  flush stdout
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
    set dependFile $::Slicer_EXT/Extensions/$dependency.s3ext
    if { ![file exists $dependFile] && ![file exists $::Slicer_BUILD/Modules/$dependency] } {
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
          vputs "moved [file tail $s3ext2] to front because it is needed by [file tail $s3ext1]"
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
# - configure the project to point to ::Slicer_BUILD
# - build the project
# - run the tests
# - run the install target
# - make the zip file
# - upload it 
#

proc buildExtension {s3ext} {

  # collect params
  array unset ::ext
  array set ::ext ""
  set ::ext(name) [file root [file tail $s3ext]]
  set ::ext(scm) ""
  set ::ext(depends) ""
  loadArray $s3ext ::ext
  if { ![info exists ::ext(cmakeproject)] } {
    set ::ext(cmakeproject) $::ext(name)
  }

  set ::ext(date) [clock format [clock seconds] -format %Y-%m-%d]


  # make dirs, delete if asked for clean build
  foreach suffix {"" -build -install} {
    set dir $::Slicer_EXT/$::ext(name)$suffix
    if { $suffix != "" } {
      vputs "Deleting $dir..."
      file delete -force $dir
    }
    if { ![file exists $dir] } {
      file mkdir $dir
    }
  }

  # check out code
  # - set array variable srcDir
  cd $::Slicer_EXT/$::ext(name)
  switch $::ext(scm) {
    "cvs" {
      if { $::EXTEND(no-extension-update) == "" } {
        runcmd $::CVS -d $::ext(cvsroot) co $::ext(cvsmodule)
      }
      set ::ext(srcDir) $::Slicer_EXT/$::ext(name)/$::ext(cvsmodule)
    }
    "svn" {
      set svncmd "yes t | $::SVN co"
      if { [info exists ::ext(svnusername)] } {
        set svncmd "$svncmd --username $::ext(svnusername)"
      }
      if { [info exists ::ext(svnpassword)] } {
        set svncmd "$svncmd --password $::ext(svnpassword)"
      }
      set svncmd "$svncmd $::ext(svnpath) $::ext(name)"
      if { $::EXTEND(no-extension-update) == ""} {
        eval runcmd $svncmd
      }
      set ::ext(srcDir) $::Slicer_EXT/$::ext(name)/$::ext(name)
    }
    default {
      puts stderr "No source code control tool specified in $s3ext"
      set ::ext(srcDir) ""
    }
  }

  if { $::ext(srcDir) == "" } {
    return
  }

  if { $::isWindows } {
    set makeCmd "$::MAKE $::ext(cmakeproject).sln /out buildlog-ctest.txt /build $::VTK_BUILD_TYPE /project ALL_BUILD"
  } else {
    set makeCmd $::MAKE
  }

  set dependPaths ""
  foreach dep $::ext(depends) {
    if { [file exists $::Slicer_HOME/Modules/$dep] } {
      # this is a module that comes with slicer
      set dependPaths "$dependPaths -D${dep}_SOURCE_DIR=$::Slicer_HOME/Modules/$dep"
      set dependPaths "$dependPaths -D${dep}_BINARY_DIR=$::Slicer_BUILD/Modules/$dep"
    } else {
      set dependPaths "$dependPaths -D${dep}_SOURCE_DIR=$::Slicer_EXT/$dep/$dep"
      set dependPaths "$dependPaths -D${dep}_BINARY_DIR=$::Slicer_EXT/$dep-build"
    }
  }

  set extraLink ""
  if { $::isDarwin } {
    set extraLink "$extraLink -DCMAKE_SHARED_LINKER_FLAGS:STRING=-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
    set extraLink "$extraLink -DCMAKE_EXE_LINKER_FLAGS=-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
  }

  # configure project and make
  cd $::Slicer_EXT/$::ext(name)-build
  set cmakeCmd [list $::CMAKE \
    -G$::GENERATOR \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=$::EXTEND(verbose) \
    -DSlicer_DIR:PATH=$::Slicer_BUILD \
    -DBUILD_AGAINST_SLICER3:BOOL=ON \
    -DMAKECOMMAND:STRING=$makeCmd \
    -DCMAKE_INSTALL_PREFIX:PATH=$::Slicer_EXT/$::ext(name)-install]
  if { $::EXTEND(release) != "" } {
    lappend cmakeCmd -DCMAKE_BUILD_TYPE:STRING=Release
  } else {
    lappend cmakeCmd -DCMAKE_BUILD_TYPE:STRING=Debug
  }
  foreach dep $dependPaths {
    lappend cmakeCmd $dep
  }
  foreach link $extraLink {
    lappend cmakeCmd $link
  }
  lappend cmakeCmd $::ext(srcDir)
  eval runcmd $cmakeCmd

  # build the project
  cd $::Slicer_EXT/$::ext(name)-build
  if { $::isWindows } {
    runcmd "$::MAKE" $::ext(cmakeproject).sln /out buildlog-allbuild.txt /build $::VTK_BUILD_TYPE /project ALL_BUILD
  } else {
    eval runcmd $::MAKE
  }

  # run the tests
  # - not all modules have tests, so allow make to fail gracefully with catch
  cd $::Slicer_EXT/$::ext(name)-build
  set ret 0
  if { $::EXTEND(test-type) != "" } {
    if { $::isWindows } {
      # don't run testing on windows - if target doesn't exist, a dialog will come up and operation will hang
      #set ret [catch "runcmd $::MAKE $::ext(cmakeproject).sln /out buildlog-test.txt /build $::VTK_BUILD_TYPE /project $::EXTEND(test-type)" res]
      set ret 0
    } else {
      set ret [catch "eval runcmd $::MAKE $::EXTEND(test-type)" res]
    }
  }

  if { $ret } {
    puts stderr "ERROR building $::ext(name)"
    puts stderr $res
  }

  # run the install target
  cd $::Slicer_EXT/$::ext(name)-build
  if { $::isWindows } {
    runcmd $::MAKE $::ext(cmakeproject).sln /out buildlog-install.txt /build $::VTK_BUILD_TYPE /project INSTALL
  } else {
    eval runcmd $::MAKE install
  }

  # extract the extension revision number
  switch $::ext(scm) {
    "cvs" {
      # TODO: look at file modification dates
      set ::ext(revision) cvs$::ext(date)
    }
    "svn" {
      set cwd [pwd]
      cd $::ext(srcDir)
      set svninfo [split [exec svn info] "\n"]
      array set svn ""
      foreach line $svninfo {
        foreach {tag value} $line {
          if { $tag == "Revision:" } {
            set ::ext(revision) svn$value
          }
        }
      }
      cd $cwd
    }
  }

  # make the zip file
  # - TODO: first, write a config file that describes the build machine
  set dir $::Slicer_EXT/$::ext(name)-install/lib/Slicer3
  foreach dirType {Modules Plugins} {
    if { [file exists $dir/$dirType] } {
      set dir $dir/$dirType
      break
    }
  }
  cd $dir
  set ::ext(zipFileName) $dir/$::ext(name)-$::ext(revision)-$::ext(date)-$::env(BUILD).zip 
  runcmd zip -r9 $::ext(zipFileName) .

  # upload it
  # - read zip file into 'data' variable
  # - write it to a socket on the ext.slicer.org server
  #
  if { $::EXTEND(upload) } {
    upload $::ext(zipFileName)  
    upload $s3ext
  }
}


set ::EXTEND(BUILT) ""
set ::EXTEND(FAILED) ""

foreach s3ext $::EXTEND(s3extFiles) {
  vputs "----------------------------------------"
  vputs "----------------------------------------"
  vputs "building $s3ext"
  set ret [catch "buildExtension $s3ext" res]
  if { $ret } {
    vputs "********************"
    vputs "Failed to build $s3ext"
    vputs "error code is: $ret"
    vputs "error result is:\n$res"
    vputs "errorInfo is:\n$::errorInfo"
    lappend ::EXTEND(FAILED) $s3ext
  } else {
    lappend ::EXTEND(BUILT) $s3ext
  }
}

vputs "********************\n"
vputs "BUILT:"
foreach built $::EXTEND(BUILT) {
  vputs "  $built"
}

if { [llength $::EXTEND(FAILED)] != 0 } {
  vputs "FAILED:"
  foreach failed $::EXTEND(FAILED) {
    vputs "  $failed"
  }
}

if { [llength $::EXTEND(s3extFiles)] != 0 } {
  vputs "\n[format %3.1f [expr 100 * (1.*[llength $::EXTEND(BUILT)] / [llength $::EXTEND(s3extFiles)])]]% succeeded" 
} else {
  vputs "\nNo .s3ext files found."
}
