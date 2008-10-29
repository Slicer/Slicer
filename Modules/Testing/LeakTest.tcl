
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

set ::modules { 
 "EMSegment"  "Gradient Anisotropic Diffusion Filter"  "Label Statistics"  "Neuro Nav"  
 "OpenIGTLink"  "QdecModule"  "QueryAtlas"  "Slicer Tractography Display"  
 "Slicer Tractography Fiducial Seeding"  "Tumor Growth"  "Volume Rendering"  "Volumes"
 }



proc runSlicer { {doNotIgnore ""} } {

  set cmd "./Slicer3"
  
  foreach m $::modules {
    if { [lsearch $m $doNotIgnore] == -1 } {
      set cmd [concat $cmd "--ignore-module \"$m\""]
    }
  }
  set cmd [concat $cmd "--exec exit"]
  puts "running: $cmd"

  set ret [catch "eval exec $cmd" res]

  puts $res
}


foreach m $::modules {
  puts "ignoring $m"
  runSlicer $m
}
