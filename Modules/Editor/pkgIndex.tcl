if {[catch {package require Itcl}]} { return }
package ifneeded Editor 3.0 [list 
  source [file join $dir Editor.tcl]
  source [file join $dir EditorNode.tcl]
  source [file join $dir EditorLogic.tcl]
  source [file join $dir EditorGUI.tcl]
  source [file join $dir EditBox.tcl]
  source [file join $dir PaintSWidget.tcl]
  source [file join $dir DrawSWidget.tcl]
  source [file join $dir EffectSWidget.tcl]
  source [file join $dir SaveIslandEffect.tcl]
  source [file join $dir ThresholdEffect.tcl]
]
