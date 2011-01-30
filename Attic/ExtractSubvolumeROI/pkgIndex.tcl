if {[catch {package require Itcl}]} { return }
package ifneeded ExtractSubvolumeROI 3.0 [list 
  source [file join $dir ExtractSubvolumeROI.tcl]
  source [file join $dir ExtractSubvolumeROINode.tcl]
  source [file join $dir ExtractSubvolumeROILogic.tcl]
  source [file join $dir ExtractSubvolumeROIGUI.tcl]
]
