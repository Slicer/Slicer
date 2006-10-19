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

  set widgets {
    nodeSelector volumesCreate volumeName volumesSelect
    volumesFrame paintThreshold paintOver paintDropper
    paintRadius paintRange paintEnable paintLabel
    paintFrame helpFrame rebuildButton
  }

  foreach w $widgets {
    $::Editor($this,$w) SetParent ""
    $::Editor($this,$w) Delete
  }

  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]
  [$this GetUIPanel] RemovePage "Editor"

}

proc EditorBuildGUI {this} {

  #
  # create and register the node class
  # - since this is a generic type of node, only do it if 
  #   it hasn't already been done by another module
  #

  set mrmlScene [[$this GetLogic] GetMRMLScene]
  set tag [$mrmlScene GetTagByClassName "vtkMRMLScriptedModuleNode"]
  if { $tag == "" } {
    set node [vtkMRMLScriptedModuleNode New]
    $mrmlScene RegisterNodeClass $node
    $node Delete
  }

  # TODO: make a node to store our parameters

  [$this GetUIPanel] AddPage "Editor" "Editor" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]

  #
  # help frame
  #
  #set ::Editor($this,helpFrame) [vtkKWFrameWithLabel New]
  set ::Editor($this,helpFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::Editor($this,helpFrame) SetParent $pageWidget
  $::Editor($this,helpFrame) Create
  $::Editor($this,helpFrame) CollapseFrame
  $::Editor($this,helpFrame) SetLabelText "Help"
  pack [$::Editor($this,helpFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::Editor($this,nodeSelector) [vtkSlicerNodeSelectorWidget New]
  $::Editor($this,nodeSelector) SetNodeClass "vtkMRMLScriptedModuleNode" "ModuleName" "Editor" "EditorParameter"
  $::Editor($this,nodeSelector) SetNewNodeEnabled 1
  $::Editor($this,nodeSelector) NoneEnabledOn
  $::Editor($this,nodeSelector) NewNodeEnabledOn
  $::Editor($this,nodeSelector) SetParent $pageWidget
  $::Editor($this,nodeSelector) Create
  $::Editor($this,nodeSelector) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::Editor($this,nodeSelector) UpdateMenu
  $::Editor($this,nodeSelector) SetLabelText "Editor Parameters"
  $::Editor($this,nodeSelector) SetBalloonHelpString "Select Editor parameters from current scene or create new ones"
  pack [$::Editor($this,nodeSelector) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  #
  # Editor Volumes
  #
  #set ::Editor($this,volumesFrame) [vtkKWFrameWithLabel New]
  set ::Editor($this,volumesFrame) [vtkSlicerModuleCollapsibleFrame New]
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

  set ::Editor($this,volumeName) [vtkKWEntryWithLabel New]
  $::Editor($this,volumeName) SetParent [$::Editor($this,volumesFrame) GetFrame]
  $::Editor($this,volumeName) Create
  $::Editor($this,volumeName) SetLabelText "Name for label map volume: "
  $::Editor($this,volumeName) SetBalloonHelpString \
    "Leave blank for automatic label name based on input name."
  pack [$::Editor($this,volumeName) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::Editor($this,volumesCreate) [vtkKWPushButton New]
  $::Editor($this,volumesCreate) SetParent [$::Editor($this,volumesFrame) GetFrame]
  $::Editor($this,volumesCreate) Create
  $::Editor($this,volumesCreate) SetText "Create Label Map"
  $::Editor($this,volumesCreate) SetBalloonHelpString "Create a new label map based on the source."
  pack [$::Editor($this,volumesCreate) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  #
  # Editor Paint
  #
  #set ::Editor($this,paintFrame) [vtkKWFrameWithLabel New]
  set ::Editor($this,paintFrame) [vtkSlicerModuleCollapsibleFrame New]
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

  set ::Editor($this,paintDropper) [vtkKWCheckButtonWithLabel New]
  $::Editor($this,paintDropper) SetParent [$::Editor($this,paintFrame) GetFrame]
  $::Editor($this,paintDropper) Create
  $::Editor($this,paintDropper) SetLabelText "Eye Dropper: "
  $::Editor($this,paintDropper) SetBalloonHelpString "Set the label number automatically by sampling the pixel location where the brush stroke starts."
  pack [$::Editor($this,paintDropper) GetWidgetName] \
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
  $this AddObserverByNumber [$::Editor($this,paintEnable) GetWidget] 10000 
  $this AddObserverByNumber $::Editor($this,paintLabel) 10001 
  $this AddObserverByNumber $::Editor($this,paintRange) 10001 
  $this AddObserverByNumber [$::Editor($this,paintThreshold) GetWidget] 10000 
  $this AddObserverByNumber [$::Editor($this,paintOver) GetWidget] 10000 
  $this AddObserverByNumber [$::Editor($this,paintDropper) GetWidget] 10000 
  $this AddObserverByNumber $::Editor($this,paintRadius) 10001 
}

proc EditorRemoveGUIObservers {this} {
}

proc EditorRemoveLogicObservers {this} {
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
  } elseif { $caller == [$::Editor($this,paintEnable) GetWidget] } {
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
  } elseif { $caller == $::Editor($this,paintRadius) } {
    switch $event {
      "10001" {
        ::PaintSWidget::ConfigureAll -radius [$::Editor($this,paintRadius) GetValue]
      }
    }
  } elseif { $caller == [$::Editor($this,paintOver) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -paintOver [[$::Editor($this,paintOver) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == [$::Editor($this,paintDropper) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -paintDropper [[$::Editor($this,paintDropper) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == [$::Editor($this,paintThreshold) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -thresholdPaint [[$::Editor($this,paintThreshold) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == $::Editor($this,paintRange) } {
    switch $event {
      "10001" {
        foreach {lo hi} [$::Editor($this,paintRange) GetRange] {}
        ::PaintSWidget::ConfigureAll -thresholdMin $lo -thresholdMax $hi
        puts "::PaintSWidget::ConfigureAll -thresholdMin $lo -thresholdMax $hi"
      }
    }
  }

  EditorUpdateMRML $this
}

proc EditorUpdateMRML {this} {
}

proc EditorProcessMRMLEvents {this caller event} {

if {0} {
   elseif { $caller == [$::Editor($this,paintEnable) GetWidget] } {
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
  } elseif { $caller == $::Editor($this,paintRadius) } {
    switch $event {
      "10001" {
        ::PaintSWidget::ConfigureAll -radius [$::Editor($this,paintRadius) GetValue]
      }
    }
  } elseif { $caller == [$::Editor($this,paintOver) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -paintOver [[$::Editor($this,paintOver) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == [$::Editor($this,paintDropper) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -paintDropper [[$::Editor($this,paintDropper) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == [$::Editor($this,paintThreshold) GetWidget] } {
    switch $event {
      "10000" {
        ::PaintSWidget::ConfigureAll -thresholdPaint [[$::Editor($this,paintThreshold) GetWidget] GetSelectedState]
      }
    }
  } elseif { $caller == $::Editor($this,paintRange) } {
    switch $event {
      "10001" {
        foreach {lo hi} [$::Editor($this,paintRange) GetRange] {}
        ::PaintSWidget::ConfigureAll -thresholdMin $lo -thresholdMax $hi
        puts "::PaintSWidget::ConfigureAll -thresholdMin $lo -thresholdMax $hi"
      }
    }
  }
}
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

  # create a display node
  set labelDisplayNode [vtkMRMLVolumeDisplayNode New]
  [[$this GetLogic] GetMRMLScene] AddNode $labelDisplayNode

  # create a volume node as copy of source volume
  set labelNode [vtkMRMLScalarVolumeNode New]
  $labelNode Copy $volumeNode
  $labelNode SetStorageNodeID ""
  $labelNode SetLabelMap 1
  # set the display node to have a label map lookup table
  $labelDisplayNode SetAndObserveColorNodeID "vtkMRMLColorNodeLabels"
  set name [[$::Editor($this,volumeName) GetWidget] GetValue]
  if { $name != "" } {
    $labelNode SetName $name
  } else {
    $labelNode SetName "[$volumeNode GetName]-label"
  }
  # Copy won't copy the ID 
  # $labelNode SetID ""  ;# clear ID so a new one is generated
  $labelNode SetAndObserveDisplayNodeID [$labelDisplayNode GetID]

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

  # add the label volume to the scene
  [[$this GetLogic] GetMRMLScene] AddNode $labelNode

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetActiveVolumeID [$volumeNode GetID]
  $selectionNode SetActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  $labelNode Delete

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval $::Editor($this,paintRange) SetWholeRange $range
  eval $::Editor($this,paintRange) SetRange $range

  # TODO: this is just so I can see the results for now
  #puts "Setting the label map to colour for the slices"
  EditorSetLabelColormap 
}

proc EditorSetRandomLabelColormap { {size 255} } {

  # TODO: once the mrml label map functionality is set up this will migrate
  # into the vtkSlicerVolumesDisplay -- the label map should be part of the DisplayNode
  # get a lut with:
  # [[[$::slicer3::ApplicationGUI GetMainSliceLogic0] GetLabelLayer] GetMapToColors] GetLookupTable
    foreach g {0 1 2} {
        puts "Setting logic $g label map to label colors..."
        [[[[$::slicer3::ApplicationGUI GetMainSliceLogic$g] GetLabelLayer] GetVolumeDisplayNode] GetColorNode] SetTypeToRandom
    }
}

proc EditorSetLabelColormap {} {
    foreach g {0 1 2} {
        puts "EditorGUI: Setting logic $g label map to label colors..."
        [[[[$::slicer3::ApplicationGUI GetMainSliceLogic$g] GetLabelLayer] GetVolumeDisplayNode] GetColorNode] SetTypeToLabels
    }
}

