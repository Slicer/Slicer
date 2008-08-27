if {[catch {package require Itcl}]} { return }
package ifneeded DistanceMapFiducials 3.0 [list 
  source [file join $dir DistanceMapFiducials.tcl]
  source [file join $dir DistanceMapFiducialsNode.tcl]
  source [file join $dir DistanceMapFiducialsLogic.tcl]
  source [file join $dir DistanceMapFiducialsGUI.tcl]
]
