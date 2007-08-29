if {[catch {package require Itcl}]} { return }
package ifneeded VolumeRendering 0.1 [list 
  source [file join $dir VolumeRendering.tcl]
  source [file join $dir VolumeRenderingNode.tcl]
  source [file join $dir VolumeRenderingLogic.tcl]
  source [file join $dir VolumeRenderingGUI.tcl]

]
