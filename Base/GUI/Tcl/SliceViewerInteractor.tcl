
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

# create a slice widget for the sliceGUI
# - check that there isn't already a widget for this sliceGUI
# - check that logic has been created
# - check that node has been assigned
proc SliceViewerInitialize { sliceGUI } {
  if { [SliceViewerFindWidgets $sliceGUI] != "" } {
    return
  }
  set logic [$sliceGUI GetLogic]
  if { $logic == "" } {
    return
  }
  set node [$logic GetSliceNode]
  if { $node == "" } {
    return
  }
  # new widget is needed, so allocate it
  SliceSWidget #auto $sliceGUI
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
