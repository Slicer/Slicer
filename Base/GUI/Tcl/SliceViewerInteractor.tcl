
package require Itcl


proc SliceViewerFindWidgets { sliceGUI } {
  # find the SliceSWidget for a given gui
  set sswidgets ""
  foreach ssw [itcl::find objects -class SliceSWidget] {
    if { [$ssw cget -sliceGUI] == $sliceGUI } {
      lappend sswidgets $ssw
    }
  }
  return $sswidgets
}

proc SliceViewerInitialize { sliceGUI } {
  if { [SliceViewerFindWidgets $sliceGUI] == "" } {
    SliceSWidget #auto $sliceGUI
  }
}


proc SliceViewerShutdown { sliceGUI } {
  # find the SliceSWidget that owns the sliceGUI and delete it
  foreach ssw [SliceViewerFindWidgets $sliceGUI] {
    itcl::delete object $ssw
  }
}

proc SliceViewerHandleEvent {sliceGUI event} {
  # initialize on first call 
  # -- allows clean shutdown because each vtkSlicerSliceGUI can shutdown in destructor
  # -- this is very light weight, so no problem calling every time

  SliceViewerInitialize $sliceGUI
}
