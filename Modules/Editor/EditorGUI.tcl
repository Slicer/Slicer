#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {
}

# Note: not a method - this is invoked directly by the GUI
# - remove the GUI from the current Application
# - re-source the code (this file)
# - re-build the GUI
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
  $::Editor($this,paintThreshold) Delete
  $::Editor($this,paintOver) Delete
  $::Editor($this,paintRadius) Delete
  $::Editor($this,paintRange) Delete
  $::Editor($this,paintEnable) Delete
  $::Editor($this,paintLabel) Delete
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
  $::Editor($this,volumesCreate) SetText "Create Label Map"
  $::Editor($this,volumesCreate) SetBalloonHelpString "Create a new label map based on the source."
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

  set ::Editor($this,paintEnable) [vtkKWCheckButtonWithLabel New]
  $::Editor($this,paintEnable) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintEnable) Create
  $::Editor($this,paintEnable) SetLabelText "Enable Painting: "
  pack [$::Editor($this,paintEnable) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintRadius) [vtkKWThumbWheel New]
  $::Editor($this,paintRadius) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintRadius) PopupModeOn
  $::Editor($this,paintRadius) Create
  $::Editor($this,paintRadius) DisplayEntryAndLabelOnTopOn
  $::Editor($this,paintRadius) DisplayEntryOn
  $::Editor($this,paintRadius) DisplayLabelOn
  [$::Editor($this,paintRadius) GetLabel] SetText "Radius: "
  $::Editor($this,paintRadius) SetBalloonHelpString "Set the radius of the paint brush in screen space pixels"
  pack [$::Editor($this,paintRadius) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintLabel) [vtkKWThumbWheel New]
  $::Editor($this,paintLabel) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintLabel) PopupModeOn
  $::Editor($this,paintLabel) Create
  $::Editor($this,paintLabel) DisplayEntryAndLabelOnTopOn
  $::Editor($this,paintLabel) DisplayEntryOn
  $::Editor($this,paintLabel) DisplayLabelOn
  [$::Editor($this,paintLabel) GetLabel] SetText "Label: "
  pack [$::Editor($this,paintLabel) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintOver) [vtkKWCheckButtonWithLabel New]
  $::Editor($this,paintOver) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintOver) Create
  $::Editor($this,paintOver) SetLabelText "Paint Over: "
  $::Editor($this,paintOver) SetBalloonHelpString "Allow brush to paint over non-zero labels."
  pack [$::Editor($this,paintOver) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintOver) [vtkKWCheckButtonWithLabel New]
  $::Editor($this,paintOver) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintOver) Create
  $::Editor($this,paintOver) SetLabelText "Eye Dropper: "
  $::Editor($this,paintOver) SetBalloonHelpString "Set the label number automatically by sampling the pixel location where the brush stroke starts."
  pack [$::Editor($this,paintOver) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintThreshold) [vtkKWCheckButtonWithLabel New]
  $::Editor($this,paintThreshold) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintThreshold) Create
  $::Editor($this,paintThreshold) SetLabelText "Threshold Painting: "
  $::Editor($this,paintThreshold) SetBalloonHelpString "Enable/Disable threshold mode for painting."
  pack [$::Editor($this,paintThreshold) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set ::Editor($this,paintRange) [vtkKWRange New]
  $::Editor($this,paintRange) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintRange) Create
  $::Editor($this,paintRange) SetLabelText "Min/Max for Threshold Paint: "
  $::Editor($this,paintRange) SetWholeRange 0 2000
  $::Editor($this,paintRange) SetRange 50 2000
  $::Editor($this,paintRange) SetReliefToGroove
  $::Editor($this,paintRange) SetBalloonHelpString "In threshold mode, the label will only be set if the background value is within this range."
  pack [$::Editor($this,paintRange) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 
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
  $this AddObserverByNumber $::Editor($this,paintEnable) 10000 
  $this AddObserverByNumber $::Editor($this,paintLabel) 10001 
  $this AddObserverByNumber $::Editor($this,paintRange) 10001 
  $this AddObserverByNumber $::Editor($this,paintThreshold) 10000 
  $this AddObserverByNumber $::Editor($this,paintOver) 10000 
  $this AddObserverByNumber $::Editor($this,paintRadius) 10001 
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
  } elseif { $caller == $::Editor($this,paintEnable) } {
    switch $event {
      "10000" {
        ::PaintSWidget::TogglePaint
      }
    }
  } elseif { $caller == $::Editor($this,paintLabel) } {
    switch $event {
      "10001" {
        ::PaintSWidget::ConfigureAll -paintColor [$::Editor($this,paintLabel) GetValue]
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

  # make an image data of the same size and shape as the input volume,
  # but filled with zeros
  set thresh [vtkImageThreshold New]
  $thresh ReplaceInOn
  $thresh ReplaceOutOn
  $thresh SetInValue 0
  $thresh SetOutValue 0
  $thresh SetInput [$volumeNode GetImageData]
  [$thresh GetOutput] Update
  $labelNode SetAndObserveImageData [$thresh GetOutput]
  $thresh Delete

  [[$this GetLogic] GetMRMLScene] AddNode $labelNode

  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  $labelNode Delete

}


