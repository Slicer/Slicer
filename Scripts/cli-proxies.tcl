#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# cli-proxies.tcl
#
# creates proxy scripts for each CLI module in a directory
# so they will execute more quickly when being queried for
# command line arguments
#
# Usage: 
#   cli-proxy <directory>
#
# Initiated - sp - 2009-10-14
#

################################################################################
#
# simple command line argument parsing
#

proc Usage { {msg ""} } {
    
    set msg "$msg\nusage: cli-proxy \[options\] <dir>"
    set msg "$msg\n  <dir> is the the directory to operate on."
    set msg "$msg\n             Typically Slicer-build/lib/Slicer3/Plugins"
    set msg "$msg\n  \[options\] is one of the following:"
    set msg "$msg\n   --slicer <arg>: slicer launcher to use (default ./Slicer3"
    set msg "$msg\n   --tcl : makes tcl proxy scripts (default)"
    set msg "$msg\n   --sh : makes sh proxy scripts (not supported)"
    set msg "$msg\n   --bat : makes sh proxy scripts (not supported)"
    set msg "$msg\n   -v --verbose : prints status as you go (default)"
    set msg "$msg\n   -q --quite : prints status as you go (default)"
    set msg "$msg\n   --help : prints this message and exits"
    puts stderr $msg
}

proc vputs {msg} {
  if { $::CLIPROXY(verbose) } {
    puts $msg
  }
}

set ::CLIPROXY(scriptType) "tcl"
set ::CLIPROXY(verbose) 1
set ::CLIPROXY(slicer) "./Slicer3"

set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "--tcl" {
            set ::CLIPROXY(scriptType) "tcl"
        }
        "--sh" {
            set ::CLIPROXY(scriptType) "sh"
        }
        "--bat" {
            set ::CLIPROXY(scriptType) "bat"
        }
        "-s" -
        "--slicer" {
            incr i
            if { $i == $argc } {
                Usage "Missing slicer argument"
            } else {
                set ::CLIPROXY(slicer) [lindex $argv $i]
            }
        }
        "--verbose" -
        "-v" {
          set ::CLIPROXY(verbose) 1
        }
        "--quiet" -
        "-z" {
          set ::CLIPROXY(verbose) 0
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

if { $argc != 1 } {
  Usage "No directory specified"
  exit 1
}
set ::CLIPROXY(dir) $argv

vputs ""
vputs "will generate $::CLIPROXY(scriptType) scripts for executables in $::CLIPROXY(dir)"
vputs ""
set ::CLIPROXY(realdir) $::CLIPROXY(dir)/Real-Executables
vputs "making new subdirectory for original CLI executables: $::CLIPROXY(realdir)"
file mkdir $::CLIPROXY(realdir)


set ignoreExts {".dylib" ".so" ".dll" ".py" ".pyc" ".tcl" ".m" ".sh"}

foreach candidate [glob $::CLIPROXY(dir)/*] {
  vputs "\n\nchecking $candidate"
  if { [file isdirectory $candidate] } {
    vputs "skipping directory"
    continue
  }
  if { [lsearch $ignoreExts [file extension $candidate]] == -1 } {
    vputs "*********************"
    set exename [file tail $candidate]
    vputs "trying to execute $candidate ($exename)"
    set ret [catch "exec $::CLIPROXY(slicer) --launch $candidate --xml" xml]
    if { $ret == 0 } {
      vputs "moving $exename"
      file rename -force $candidate $::CLIPROXY(realdir)/$exename
      switch $::CLIPROXY(scriptType) {
        "tcl" {
          set proxyName $candidate-proxy.tcl
          set fp [open "$proxyName" "w"]
          puts $fp "#!/bin/sh"
          puts $fp "# the next line restarts using tclsh \\"
          puts $fp "    exec tclsh \"\$0\" \"\$@\""
          puts $fp ""
          puts $fp "set xml \{$xml\}"
          puts $fp "if \{ \[lindex \$argv 0\] == \"--xml\" \} \{ "
          puts $fp "  puts \$xml"
          puts $fp "\} else \{"
          puts $fp "  set ret \[catch \"exec $::CLIPROXY(realdir)/$exename \$argv\" res\]"
          puts $fp "  puts \$res"
          puts $fp "  return $ret"
          puts $fp "\}"
          close $fp
          file attributes "$proxyName" -permissions ugo+x 
          vputs "created $proxyName"
        }
        default {
          puts "unsupported script type"
          exit -1
        }
      }
      vputs "*********************"
    } else {
      vputs "Could not execute $::CLIPROXY(slicer) --launch $candidate --xml"
    }
  } else {
    vputs "ignoring $candidate because of file extension"
  }
}
