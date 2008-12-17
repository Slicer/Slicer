#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"

################################################################################
#
# test script to identify which (if any) of slicer's loadable modules is causing a leak
#
# - update the global modules list with all suspect modules
# - run this script from the Slicer3-build directory
#
################################################################################


#
# two global lists of slicer modules and scripted modules 
#
set ::modules { 
 "CLI" "Change Tracker" "EMSegment"  "Gradient Anisotropic Diffusion Filter"  "Label Statistics"  "Neuro Nav"  
 "OpenIGTLinkIF"  "QdecModule"  "QueryAtlas"  "Slicer Tractography Display"  
 "Slicer Tractography Fiducial Seeding"  "Volume Rendering"  "VolumeRenderingCuda" "Volumes"
 "OpenIGTLink IF" "Skeleton" "IA_FEMesh"
 }

set ::scriptedModules ""
foreach dir [glob -nocomplain lib/Slicer3/Modules/*] {
  if { [file exists $dir/pkgIndex.tcl] } {
    lappend ::scriptedModules $dir
  }
}

foreach f [glob -nocomplain lib/Slicer3/Plugins/*.py] {
#  lappend ::scriptedModules $f
}



#
# routine to run slicer with only the modules in the doNotIgnore list
# included
#
proc runSlicer { {doNotIgnore ""} } {

  puts "\n\n"

  # rename package files for any ignored scripted modules
  puts -nonewline "ignoring: "
  foreach sm $::scriptedModules {
    if { [file isdir $sm] } {
      if { [lsearch $sm $doNotIgnore] == -1 } {
        puts -nonewline " [file tail $sm]"
        file rename $sm/pkgIndex.tcl $sm/pkgIndexIgnore.tcl
      }
    } else {
        puts -nonewline "[file tail $sm]"
        file rename $sm [file root $sm].ignore
    }
  }
  puts "\n"
  
  set cmd "./Slicer3"

  # add command line flag for any ignored loadable modules
  foreach m $::modules {
    if { [lsearch $m $doNotIgnore] == -1 } {
      set cmd [concat $cmd "--ignore-module '$m'"]
    }
  }

  # exit slicer after the gui is built
  set cmd [concat $cmd "--exec exit"]

  puts "running: $cmd"

  set ret [catch "eval exec $cmd" res]

  puts ""
  if { $ret } {
    puts "****** Error when not ignoring $doNotIgnore"
    puts $res
  } else {
    puts "-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ NO ERRORS"
  }

  # restore ignored scripted modules
  puts -nonewline "restoring: "
  foreach sm $::scriptedModules {
    if { [file isdir $sm] } {
      if { [lsearch $sm $doNotIgnore] == -1 } {
        puts -nonewline " [file tail $sm]"
        file rename $sm/pkgIndexIgnore.tcl $sm/pkgIndex.tcl
      }
    } else {
        puts -nonewline " [file tail $sm]"
        file rename [file root $sm].ignore [file root $sm].py
    }
  }
  puts "\n\n"
  
  return $ret
}

# run with all modules turned off
puts "--------------------------------------------------------------------------------"
puts "ignoring all"
puts "--------------------------------------------------------------------------------"
runSlicer $::modules

puts "--------------------------------------------------------------------------------"
puts "ignoring none"
puts "--------------------------------------------------------------------------------"
runSlicer ""

# sequentially enable only one module at a time
foreach m $::modules {
  puts "--------------------------------------------------------------------------------"
  puts "not ignoring $m"
  puts "--------------------------------------------------------------------------------"
  set ::RESULTS($m) [runSlicer $m]
}

foreach m $::scriptedModules {
  puts "--------------------------------------------------------------------------------"
  puts "not ignoring $m"
  puts "--------------------------------------------------------------------------------"
  set ::RESULTS($m) [runSlicer $m]
}

puts "--------------------------------------------------------------------------------"
puts [parray ::RESULTS]
