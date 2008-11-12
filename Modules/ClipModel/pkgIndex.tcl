if {[catch {package require Itcl}]} { return }
package ifneeded ClipModel 3.0 [list 
  source [file join $dir ClipModel.tcl]
  source [file join $dir ClipModelNode.tcl]
  source [file join $dir ClipModelLogic.tcl]
  source [file join $dir ClipModelGUI.tcl]
]
