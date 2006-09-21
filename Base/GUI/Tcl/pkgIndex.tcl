
package require Itcl

package ifneeded SlicerBaseGUITcl 3.0 [list 
  source [file join $dir SliceViewerInteractor.tcl]
  source [file join $dir SWidget.tcl]
  source [file join $dir SliceSWidget.tcl]
  source [file join $dir SeedSWidget.tcl]
  source [file join $dir PaintSWidget.tcl]
  source [file join $dir DrawSWidget.tcl]
  source [file join $dir CsysSWidget.tcl]
]
