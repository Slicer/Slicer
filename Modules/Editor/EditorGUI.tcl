#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {
}

# Note: not a method - this is invoked directly by the GUI
proc EditorReload {this} {

  EditorRemoveGUIObservers $this
  EditorTearDownGUI $this
  # TODO: figure this out from the CMakeCache and only offer the 
  # reload button if the source files exist
  source c:/pieper/bwh/slicer3/latest/Slicer3/Modules/Editor/EditorGUI.tcl
  EditorBuildGUI $this
}

# Note: not a method - this is invoked directly by the GUI
proc EditorTearDownGUI {this} {

  $::Editor($this,optionsFrame) Delete
  $::Editor($this,helpFrame) Delete
  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]
  [$this GetUIPanel] RemovePage "Editor"

}

proc EditorBuildGUI {this} {

  # TODO: create and register the node class
  # TODO: make a node to store our parameters

  [$this GetUIPanel] AddPage "Editor" "Editor" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]

  #
  # help frame
  #
  set ::Editor($this,helpFrame) [vtkKWFrameWithLabel New]
  $::Editor($this,helpFrame) SetParent $pageWidget
  $::Editor($this,helpFrame) Create
  $::Editor($this,helpFrame) CollapseFrame
  $::Editor($this,helpFrame) SetLabelText "Help"
  pack [$::Editor($this,helpFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  #
  # Editor Options
  #
  set ::Editor($this,optionsFrame) [vtkKWFrameWithLabel New]
  $::Editor($this,optionsFrame) SetParent $pageWidget
  $::Editor($this,optionsFrame) Create
  $::Editor($this,optionsFrame) SetLabelText "Options"
  pack [$::Editor($this,optionsFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  #
  # Rebuild Button
  #
  set ::Editor($this,rebuildButton) [vtkKWPushButton New]
  $::Editor($this,rebuildButton) SetParent $pageWidget
  $::Editor($this,rebuildButton) Create
  $::Editor($this,rebuildButton) SetText "Reload"
  pack [$::Editor($this,rebuildButton) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  EditorAddGUIObservers $this
}

proc EditorAddGUIObservers {this} {
  $this AddObserverByNumber $::Editor($this,rebuildButton) 10000 
}

proc EditorRemoveGUIObservers {this} {
}

proc EditorRemoveMRMLNodeObservers {this} {
}

proc EditorProcessLogicEvents {this caller event} {
}

proc EditorProcessGUIEvents {this caller event} {

  if { $caller == $::Editor($this,rebuildButton) } {
    switch $event {
      "10000" {
        EditorReload $this
      }
    }
  }
}

proc EditorProcessMRMLEvents {this caller event} {
}

proc EditorEnter {this} {
}

proc EditorExit {this} {
}
