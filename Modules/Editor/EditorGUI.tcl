#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {
}

# Note: not a method - this is invoked directly by the GUI
proc EditorReload { {this ""} } {

  if { $this == "" } {
    set tag [lindex [array names ::Editor] 0]
    regsub -all "," $tag " " tag
    set this [lindex $tag 0]
  }
  EditorRemoveGUIObservers $this
  EditorTearDownGUI $this
  foreach n [array names ::Editor ${this}*] {
    unset ::Editor($n)
  }
  # TODO: figure this out from the CMakeCache and only offer the 
  # reload button if the source files exist
  source c:/pieper/bwh/slicer3/latest/Slicer3/Modules/Editor/EditorGUI.tcl
  EditorBuildGUI $this
  EditorAddGUIObservers $this

}

# Note: not a method - this is invoked directly by the GUI
proc EditorTearDownGUI {this} {

  $::Editor($this,volumesCreate) Delete
  $::Editor($this,volumesSelect) Delete
  $::Editor($this,volumesFrame) Delete
  $::Editor($this,paintFrame) Delete
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
  # Editor Volumes
  #
  set ::Editor($this,volumesFrame) [vtkKWFrameWithLabel New]
  $::Editor($this,volumesFrame) SetParent $pageWidget
  $::Editor($this,volumesFrame) Create
  $::Editor($this,volumesFrame) SetLabelText "Volumes"
  pack [$::Editor($this,volumesFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::Editor($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::Editor($this,volumesSelect) SetParent [$::Editor($this,volumesFrame) GetFrame]
  $::Editor($this,volumesSelect) Create
  $::Editor($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::Editor($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::Editor($this,volumesSelect) UpdateMenu
  $::Editor($this,volumesSelect) SetLabelText "Source Volume:"
  $::Editor($this,volumesSelect) SetBalloonHelpString "The Source Volume will define the dimensions and directions for the new label map"
  pack [$::Editor($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::Editor($this,volumesCreate) [vtkKWPushButton New]
  $::Editor($this,volumesCreate) SetParent [$::Editor($this,volumesFrame) GetFrame]
  $::Editor($this,volumesCreate) Create
  $::Editor($this,volumesCreate) SetText "Create"
  pack [$::Editor($this,volumesCreate) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  #
  # Editor Paint
  #
  set ::Editor($this,paintFrame) [vtkKWFrameWithLabel New]
  $::Editor($this,paintFrame) SetParent $pageWidget
  $::Editor($this,paintFrame) Create
  $::Editor($this,paintFrame) SetLabelText "Paint"
  pack [$::Editor($this,paintFrame) GetWidgetName] \
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
}

proc EditorAddGUIObservers {this} {
  $this AddObserverByNumber $::Editor($this,rebuildButton) 10000 
  $this AddObserverByNumber $::Editor($this,volumesCreate) 10000 
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
  } elseif { $caller == $::Editor($this,volumesCreate) } {
    switch $event {
      "10000" {
        EditorCreateLabelVolume $this
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

# TODO: there might be a better place to put this for general use...  
proc EditorCreateLabelVolume {this} {

  set volumeNode [$::Editor($this,volumesSelect) GetSelected]
  if { $volumeNode == "" } {
    return;
  }

  set labelNode [vtkMRMLScalarVolumeNode New]
  $labelNode Copy $volumeNode
  $labelNode SetLabelMap 1
  $labelNode SetName "[$volumeNode GetName]-label"
  $labelNode SetID ""  ;# clear ID so a new one is generated

  set imageData [vtkImageData New]
  $imageData ShallowCopy [$volumeNode GetImageData]
  $imageData AllocateScalars
  $labelNode SetAndObserveImageData $imageData
  $imageData Delete

  [[$this GetLogic] GetMRMLScene] AddNode $labelNode

  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  $labelNode Delete

}


