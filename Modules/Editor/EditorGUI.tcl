#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {
}

proc EditorBuildGUI {this} {

  tk_messageBox -message "Okay, let's get to work!"

  # TODO: create and register the node class
  # TODO: make a node to store our parameters

  [$this GetUIPanel] AddPage "Editor" "Editor" ""

  #
  # help frame
  #
  set ::Editor($this,helpFrame) [vtkKWFrameWithLabel New]
  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]
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

}

proc EditorAddGUIObservers {this} {
}

proc EditorRemoveGUIObservers {this} {
}

proc EditorRemoveMRMLNodeObservers {this} {
}

proc EditorProcessLogicEvents {this caller event} {
}

proc EditorProcessGUIEvents {this caller event} {
}

proc EditorProcessMRMLEvents {this caller event} {
}

proc EditorEnter {this} {
}

proc EditorExit {this} {
}
