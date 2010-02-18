#!/bin/sh
# start tclsh \
  exec tclsh "$0" "$@"

#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: slicerextd.tcl,v $
#   Date:      $Date: 2006/01/30 20:47:46 $
#   Version:   $Revision: 1.10 $
# 
#===============================================================================
# FILE:        slicerextd.tcl
# PROCEDURES:  
#==========================================================================auto=

#
# daemon to listen for extension (module) uploads
#
# - start with 'nohup ./slicerextd.tcl &'
#
#

set __comment {

}

set ::SLICEREXTD(dir) /home/pieper/slicer-extensions
#
# returns a listening socket on given port or uses default
#
proc slicerextd_start { {port 8845} } {

    set ::SLICEREXTD(port) $port
    set ret [ catch {set ::SLICEREXTD(serversock) [socket -server slicerextd_sock_cb $port]} res]

    if { $ret } {
        puts "Warning: could not start slicer daemon at default port (probably another daemon already running on this machine)."
        return
    }

    set ::SLICEREXTD(running) 1
}

#
# shuts down the socket
# - frees the tcl helper if it exists
#
proc slicerextd_stop { } {

    if { ![info exists ::SLICEREXTD(serversock)] } {
        return
    }

    close $sock

    set ::SLICEREXTD(running) 0
}

#
# accepts new connections
#
proc slicerextd_sock_cb { sock addr port } {

  set ::SLICEREXTD(addr) $addr
  fileevent $sock readable "slicerextd_sock_fileevent $sock"
}

#
# handles input on the connection
#
proc slicerextd_sock_fileevent {sock} {
    
    if { [eof $sock] } {
        close $sock
        return
    }
    
    gets $sock line
    
    switch -glob $line {
        "ls*" {
            puts -nonewline $sock [exec find $::SLICEREXTD(dir)]
            puts $sock ""
            flush $sock
        }
        "get*" {
            if { [llength $line] < 2 } {
                puts $sock "get error: missing filename"
                flush $sock
                return
            }
            set fileName [lindex $line 1]
            
            fconfigure $sock -translation binary

            set fp [open $::SLICEREXTD(dir)/$fileName "r"]
            set data [read $fp]
            close $fp
            puts -nonewline $sock $data
            fconfigure $sock -translation auto
            flush $sock
        } 
        "put*" {
            #
            # read a file from the socket and store it based on the parameters
            # date, build, name
            # - use file tail to avoid getting ../../trash
            # - the protocol is a convention that matches the 
            #   upload proc in Scripts/extend.tcl
            #

            if { [llength $line] < 5 } {
                puts $sock "put error: need svnsubpath, rev-build, name, and size"
                flush $sock
                return
            }
            set subpath [file tail [lindex $line 1]]
            set revbuild [file tail [lindex $line 2]]
            set name [file tail [lindex $line 3]]
            set size [file tail [lindex $line 4]]
            
            fconfigure $sock -translation binary
            puts "reading data ($size bytes) from $::SLICEREXTD(addr) for $name..."
            set data [read $sock $size]
            fconfigure $sock -translation auto
            puts "succeeded in reading $name."

            set pathSoFar ""
            foreach subdir [file split $subpath] {
              set pathSoFar $pathSoFar/$subdir
              set dir $::SLICEREXTD(dir)/$pathSoFar
              if { ![file exists $dir] } {
                file mkdir $dir
              }
            }

            set dir $dir/$revbuild
            if { ![file exists $dir] } {
              file mkdir $dir
            }
            set fp [open $dir/$name "w"]
            fconfigure $fp -translation binary
            puts "saving..."
            puts -nonewline $fp $data
            close $fp
            puts "done"

        }
        default {
            puts $sock "unknown command $line"
            flush $sock
        }
    }
}

slicerextd_start

vwait $::SLICEREXTD(running)
