#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# versioner.tcl
#
# adds a lib/Slicer3/SlicerVersion.txt file to the current build
#
# Usage: (no options)
#   versioner 
#
# Initiated - sp - 2009-03-24
#

################################################################################

# for subversion repositories (Sandbox)
if {[info exists ::env(SVN)]} {
    set ::SVN $::env(SVN)
} else {
    set ::SVN svn
}


################################################################################
# build the lib/Slicer3/Slicer3version.txt file
# - determine the location
# - determine the build variables
# - get the svn info
# - write the file
# 

set cwd [pwd]
cd [file dirname [info script]]
cd ..
set ::Slicer_HOME [pwd]

source $::Slicer_HOME/slicer_variables.tcl

set ::Slicer_BUILDDATE [clock format [clock seconds] -format %Y-%m-%d]

set ::ENV(LANG) "C"
set svninfo [split [exec svn info] "\n"]
array set svn ""
foreach line $svninfo {
  foreach {tag value} $line {
    if { $tag == "URL:" } {
      set svn(URL) $value
    }
    if { $tag == "Revision:" } {
      set svn(revision) $value
    }
  }
}
cd $cwd

set versionText ""
set versionText "${versionText}build $::env(BUILD)\n"
set versionText "${versionText}buildDate $::Slicer_BUILDDATE\n"
set versionText "${versionText}svnurl $svn(URL)\n"
set versionText "${versionText}svnrevision $svn(revision)\n"

set versionFile $::Slicer_BUILD/lib/Slicer3/SlicerVersion.txt
puts "Writing version information to $versionFile"
puts $versionText
set fp [open $versionFile "w"]
puts $fp $versionText
close $fp
