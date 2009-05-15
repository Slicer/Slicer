if {[catch {package require Itcl}]} { return }
package ifneeded FastMarchingSegmentation 3.0 [list 
  source [file join $dir FastMarchingSegmentation.tcl]
  source [file join $dir FastMarchingSegmentationNode.tcl]
  source [file join $dir FastMarchingSegmentationLogic.tcl]
  source [file join $dir FastMarchingSegmentationGUI.tcl]
]
