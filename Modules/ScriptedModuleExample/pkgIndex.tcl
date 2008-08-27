if {[catch {package require Itcl}]} { return }
package ifneeded ScriptedModuleExample 3.0 [list 
  source [file join $dir ScriptedModuleExample.tcl]
  source [file join $dir ScriptedModuleExampleNode.tcl]
  source [file join $dir ScriptedModuleExampleLogic.tcl]
  source [file join $dir ScriptedModuleExampleGUI.tcl]
]
