#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# lsext.tcl
#
# list extensions compatible with current build
#
# Usage:
#   lsext [server] [options]
#
# Initiated - sp - 2008-12-30
#


package require http

################################################################################
#
# simple command line argument parsing
#

proc Usage { {msg ""} } {
    
    set msg "$msg\nusage: extend \[options\] \[server\]..."
    set msg "$msg\n  \[server\] defaults to http://ext.slicer.org/ext"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   h --help : prints this message and exits"
    set msg "$msg\n   -b --build : build platform (required) (win32, darwin-x86, linux-x86, linux-x86_64...)"
    set msg "$msg\n   -d --date : build date (e.g. 2008-12-30)"
    set msg "$msg\n   --verbose : optional, print out lots of stuff, for debugging"
    puts stderr $msg
}

set ::LSEXT(server) "http://ext.slicer.org/ext"
set ::LSEXT(build) ""
set ::LSEXT(date) ""
set ::LSEXT(verbose) "false"

set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--build" -
        "-b" {
            incr i
            if { $i == $argc } {
                Usage "Missing build argument"
            } else {
                set ::LSEXT(build) [lindex $argv $i]
            }
        }
        "--date" -
        "-d" {
            incr i
            if { $i == $argc } {
                Usage "Missing date argument"
            } else {
                set ::LSEXT(date) [lindex $argv $i]
            }
        }
        "--verbose" {
            set ::LSEXT(verbose) "true"
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

if {$argc == 1 } {
  set ::LSEXT(server) $argv
} else {
  if { $argc != 0 } {
    Usage
    exit -1
  }
}

foreach arg {build date} {
  if { $::LSEXT($arg) == "" } {
    puts "Missing required $arg argument"
    exit -2
  }
}

if { $::LSEXT(verbose) } {
  puts [parray ::LSEXT]
}

#
# Fetch and parse the extensions from the server
# - for now, screen scrape since the format is predictable (and under our control)
#

set url $::LSEXT(server)/$::LSEXT(date)/$::LSEXT(build)/

if { $::LSEXT(verbose) } {
  puts "Reading from $url"
}

set token [::http::geturl $url]
set html [::http::data $token]

if { $::LSEXT(verbose) } {
  puts "Got back:\n\n$html"
}

set key ".zip\">"
set index [string first $key $html]
while { $index != -1 } {
  set index [expr $index + [string length $key]]
  set html [string range  $html $index end]
  set end [string first "</a>" $html]
  set name [string range  $html 0 [expr $end - 1]]
  puts $name
  set index [string first ".zip\">" $html]
}

