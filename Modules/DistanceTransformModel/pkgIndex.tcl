if {[catch {package require Itcl}]} { return }
package ifneeded DistanceTransformModel 3.0 [list 
  source [file join $dir DistanceTransformModel.tcl]
  source [file join $dir DistanceTransformModelNode.tcl]
  source [file join $dir DistanceTransformModelLogic.tcl]
  source [file join $dir DistanceTransformModelGUI.tcl]
]
