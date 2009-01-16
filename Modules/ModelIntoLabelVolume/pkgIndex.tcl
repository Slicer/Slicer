if {[catch {package require Itcl}]} { return }
package ifneeded ModelIntoLabelVolume 3.0 [list 
  source [file join $dir ModelIntoLabelVolume.tcl]
  source [file join $dir ModelIntoLabelVolumeNode.tcl]
  source [file join $dir ModelIntoLabelVolumeLogic.tcl]
  source [file join $dir ModelIntoLabelVolumeGUI.tcl]
]
