if {[catch {package require Itcl}]} { return }
package ifneeded CropVolume 3.0 [list 
  source [file join $dir CropVolume.tcl]
  source [file join $dir CropVolumeNode.tcl]
  source [file join $dir CropVolumeLogic.tcl]
  source [file join $dir CropVolumeGUI.tcl]
]
