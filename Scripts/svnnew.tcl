#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"


#
# simple script to show what is new in the repository 
# compared to your current revision
#
set info [exec svn info]
set revIndex [lsearch $info "Revision:"]
set rev [lindex $info [expr $revIndex + 1]]
set log [exec svn log -r $rev:HEAD]
puts $log
