if {[catch {package require Itcl}]} { return }
package ifneeded DistanceMapModel 3.0 [list 
  source [file join $dir DistanceMapModel.tcl]
  source [file join $dir DistanceMapModelNode.tcl]
  source [file join $dir DistanceMapModelLogic.tcl]
  source [file join $dir DistanceMapModelGUI.tcl]
]
