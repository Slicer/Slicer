#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# test script to identify which (if any) of slicer's loadable modules is causing a leak
#
# - update the global modules list with all suspect modules
# - run this script from the Slicer-build directory
#
################################################################################

# for easy cut and paste...
set typical_command {
  ../Slicer3/Modules/Testing/LeakTest.tcl 2>&1 | tee /tmp/leakout
}

#
# global list of slicer modules
#

switch $tcl_platform(os) {
  "Darwin" {set ::EXT "dylib"}
  "Linux" {set ::EXT "so"}
  default {set ::EXT "dll"}
}

set ::dynamicModules ""
foreach candidate [glob -nocomplain lib/Slicer3/Modules/*] {
  if { [file exists $candidate/pkgIndex.tcl] } {
    lappend ::dynamicModules $candidate
  }
  if { [string match "*.$::EXT" $candidate] } {
    lappend ::dynamicModules $candidate
  }
}


foreach f [glob -nocomplain lib/Slicer3/Plugins/*.py] {
#  lappend ::dynamicModules $f
}

puts "evaluating:"
puts $::dynamicModules


#
# routine to run slicer with only the modules in the doNotIgnore list
# included
#
proc runSlicer { {doNotIgnore ""} {target ""} } {

  set target $doNotIgnore
  # these libs are in the Modules directory even though Slicer3.cxx links to them (bad...)
  # todo - this doesn't actully support windows dll naming convention
  lappend doNotIgnore "lib/Slicer3/Modules/libVolumes.$::EXT"
  lappend doNotIgnore "lib/Slicer3/Modules/libCommandLineModule.$::EXT"
  lappend doNotIgnore "lib/Slicer3/Modules/libScriptedModule.$::EXT"
  lappend doNotIgnore "lib/Slicer3/Modules/libSlicerDaemon.$::EXT"
  lappend doNotIgnore "lib/Slicer3/Modules/libSlicerTractographyDisplay.$::EXT"
  lappend doNotIgnore "lib/Slicer3/Modules/libSlicerTractographyFiducialSeeding.$::EXT"
  puts "\n\n"
  puts "do not ignore $doNotIgnore"
  puts "\n\n"
  puts "testing $target"


  # rename package files for any ignored scripted modules
  puts -nonewline "ignoring: "
  foreach sm $::dynamicModules {
    if { [lsearch $doNotIgnore $sm] == -1 } {
      if { [file isdir $sm] } {
          puts -nonewline " [file tail $sm]"
          file rename -force $sm/pkgIndex.tcl $sm/pkgIndexIgnore.tcl
      } else {
          puts -nonewline " [file tail $sm] "
          file rename -force $sm ${sm}.ignore
      }
    }
  }
  puts "\n"
  
  set cmd "./Slicer3"

  # exit slicer after the gui is built
  set cmd [concat $cmd "--exec exit"]

  puts "running: $cmd"

  set ret [catch "eval exec $cmd" res]

  puts ""
  if { $ret } {
    puts "****** Error when not ignoring $target"
    lappend ::RESULTS(failers) $target"
  } else {
    puts "-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ NO ERRORS"
  }
  if { [string match "*vtkDebugLeaks*" $res] } {
    puts "%%%%%%%%% Leaks when not ignoring $target"
    lappend ::RESULTS(leakers) $target"
  } else {
    puts "-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ NO LEAKS"
  }
  puts $res

  # restore ignored scripted modules
  puts -nonewline "restoring: "
  foreach sm $::dynamicModules {
    if { [lsearch $doNotIgnore $sm] == -1 } {
      if { [file isdir $sm] } {
          puts -nonewline " [file tail $sm]"
          file rename -force $sm/pkgIndexIgnore.tcl $sm/pkgIndex.tcl
      } else {
          puts -nonewline " [file tail $sm]"
          file rename -force ${sm}.ignore $sm
      }
    }
  }
  puts "\n\n"
  
  return $ret
}

# run with all modules turned off
puts "--------------------------------------------------------------------------------"
puts "ignoring all"
puts "--------------------------------------------------------------------------------"
runSlicer "" "no modules"

puts "--------------------------------------------------------------------------------"
puts "ignoring none"
puts "--------------------------------------------------------------------------------"
runSlicer $::dynamicModules "all modules"

# sequentially enable only one module at a time

foreach m $::dynamicModules {
  puts "--------------------------------------------------------------------------------"
  puts "not ignoring $m"
  puts "--------------------------------------------------------------------------------"
  set ::RESULTS($m) [runSlicer $m $m]
}

puts "--------------------------------------------------------------------------------"
puts [parray ::RESULTS]
