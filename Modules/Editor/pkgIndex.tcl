if {[catch {package require Itcl}]} { return }
package ifneeded Editor 3.0 [list 
  source [file join $dir Editor.tcl]
  source [file join $dir EditorNode.tcl]
  source [file join $dir EditorLogic.tcl]
  source [file join $dir EditorGUI.tcl]
]
