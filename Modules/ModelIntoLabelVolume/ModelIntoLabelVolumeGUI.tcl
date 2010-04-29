#
# ModelIntoLabelVolume GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc ModelIntoLabelVolumeConstructor {this} {
}

proc ModelIntoLabelVolumeDestructor {this} {
}

proc ModelIntoLabelVolumeTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    run modelsSelect
    volumesSelect labelValueEntry intensity subdivide labelVolumeName settingsFrame 
  }

  foreach w $widgets {
    $::ModelIntoLabelVolume($this,$w) SetParent ""
    $::ModelIntoLabelVolume($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "ModelIntoLabelVolume"]
    [$this GetUIPanel] RemovePage "ModelIntoLabelVolume"
  }

  unset ::ModelIntoLabelVolume(singleton)

}

proc ModelIntoLabelVolumeBuildGUI {this} {

  if { [info exists ::ModelIntoLabelVolume(singleton)] } {
    error "ModelIntoLabelVolume singleton already created"
  }
  set ::ModelIntoLabelVolume(singleton) $this

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


  $this SetCategory "Surface Models"
  [$this GetUIPanel] AddPage "ModelIntoLabelVolume" "ModelIntoLabelVolume" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "ModelIntoLabelVolume"]

  #
  # help frame
  #
  set helptext "The ModelIntoLabelVolume is a scripted module in Tcl. It intersects a model with a reference volume and creates an output label map volume with the model vertices labelled with the given label value. This is useful once tracts have been generated to turn them into a label volume. If the intensity option is checked, it will use the voxel values from the reference volume (you can change the output label volume to use the grey colour node). If the subdivide option is  set to 1, subdivide the input model into a new one with more vertices to get a more detailed output label map.<a>http://www.slicer.org/slicerWiki/index.php/Modules:Model_Into_Label_Volume_Documentation-3.6</a>"
  set abouttext "Contributed by Nicole Aucoin, SPL, BWH (Ron Kikinis, SPL, BWH). This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # ModelIntoLabelVolume Settings
  #
  set ::ModelIntoLabelVolume($this,settingsFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::ModelIntoLabelVolume($this,settingsFrame) SetParent $pageWidget
  $::ModelIntoLabelVolume($this,settingsFrame) Create
  $::ModelIntoLabelVolume($this,settingsFrame) SetLabelText "Settings"
  pack [$::ModelIntoLabelVolume($this,settingsFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

 set ::ModelIntoLabelVolume($this,modelsSelect) [vtkSlicerNodeSelectorWidget New]
    $::ModelIntoLabelVolume($this,modelsSelect) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
    $::ModelIntoLabelVolume($this,modelsSelect) Create
    $::ModelIntoLabelVolume($this,modelsSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ModelIntoLabelVolume($this,modelsSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ModelIntoLabelVolume($this,modelsSelect) SetNoneEnabled 1
    $::ModelIntoLabelVolume($this,modelsSelect) UpdateMenu
    $::ModelIntoLabelVolume($this,modelsSelect) SetLabelText "Model:"
    $::ModelIntoLabelVolume($this,modelsSelect) SetBalloonHelpString "The Model to use"
    pack [$::ModelIntoLabelVolume($this,modelsSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  set ::ModelIntoLabelVolume($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::ModelIntoLabelVolume($this,volumesSelect) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,volumesSelect) Create
  $::ModelIntoLabelVolume($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::ModelIntoLabelVolume($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::ModelIntoLabelVolume($this,volumesSelect) UpdateMenu
  $::ModelIntoLabelVolume($this,volumesSelect) SetLabelText "Reference Volume:"
  $::ModelIntoLabelVolume($this,volumesSelect) SetBalloonHelpString "The Reference Volume to intersect with"
  pack [$::ModelIntoLabelVolume($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  # label value to use in the new label volume

  set ::ModelIntoLabelVolume($this,labelValueEntry) [vtkKWEntryWithLabel New]
  $::ModelIntoLabelVolume($this,labelValueEntry) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,labelValueEntry) Create
  $::ModelIntoLabelVolume($this,labelValueEntry) SetLabelText "Label Value:"
  [ $::ModelIntoLabelVolume($this,labelValueEntry) GetWidget] SetValue 2
  $::ModelIntoLabelVolume($this,labelValueEntry) SetBalloonHelpString "The label value to insert in the label map output volume"
  pack [$::ModelIntoLabelVolume($this,labelValueEntry) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
  
  # name for the new label map
  set ::ModelIntoLabelVolume($this,labelVolumeName) [vtkKWEntryWithLabel New]
  $::ModelIntoLabelVolume($this,labelVolumeName) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,labelVolumeName) Create
  $::ModelIntoLabelVolume($this,labelVolumeName) SetLabelText "Label Volume Name:"
  [$::ModelIntoLabelVolume($this,labelVolumeName) GetWidget] SetValue "ModelInLabelMap"
  $::ModelIntoLabelVolume($this,labelVolumeName) SetBalloonHelpString "The name to use for the label map output volume"
  pack [$::ModelIntoLabelVolume($this,labelVolumeName) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  # or else use intensity from the reference volume
  set ::ModelIntoLabelVolume($this,intensity) [vtkKWCheckButton New]
    $::ModelIntoLabelVolume($this,intensity) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
    $::ModelIntoLabelVolume($this,intensity) Create
    $::ModelIntoLabelVolume($this,intensity) SetText "Use Intensity"
    $::ModelIntoLabelVolume($this,intensity) SetSelectedState 0
    $::ModelIntoLabelVolume($this,intensity) SetBalloonHelpString "Use the intensity value from the reference volume, overrides the label value and produces a more colourful output until you change the colour node on the label volume to grey"
    pack [$::ModelIntoLabelVolume($this,intensity) GetWidgetName] -side top -anchor w -padx 2 -pady 2 

  # subdivide the input model into a new one with more vertices?
  set ::ModelIntoLabelVolume($this,subdivide) [vtkKWCheckButton New]
    $::ModelIntoLabelVolume($this,subdivide) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
    $::ModelIntoLabelVolume($this,subdivide) Create
    $::ModelIntoLabelVolume($this,subdivide) SetText "Subdivide Model"
    $::ModelIntoLabelVolume($this,subdivide) SetSelectedState 0
    $::ModelIntoLabelVolume($this,subdivide) SetBalloonHelpString "Subdivide the input model using the vtkButterflySubdivisionFilter. WARNING: not fully tested on any but basic mrml model nodes. Copies the first display node over to the new model node. Useful to get a more filled in output label map if resampling the reference volume doesn't work."
    pack [$::ModelIntoLabelVolume($this,subdivide) GetWidgetName] -side top -anchor w -padx 2 -pady 2 

  # apply button
  set ::ModelIntoLabelVolume($this,run) [vtkKWPushButton New]
  $::ModelIntoLabelVolume($this,run) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,run) Create
  $::ModelIntoLabelVolume($this,run) SetText "Apply"
  $::ModelIntoLabelVolume($this,run) SetBalloonHelpString "Apply."
  pack [$::ModelIntoLabelVolume($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc ModelIntoLabelVolumeAddGUIObservers {this} {
  $this AddObserverByNumber $::ModelIntoLabelVolume($this,run) 10000 

  $this AddObserverByNumber $::ModelIntoLabelVolume($this,modelsSelect) 11000    

  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
    
}

proc ModelIntoLabelVolumeRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
  $this RemoveObserverByNumber $::ModelIntoLabelVolume($this,modelsSelect) 11000
  $this RemoveObserverByNumber $::ModelIntoLabelVolume($this,run) 10000 
}

proc ModelIntoLabelVolumeRemoveLogicObservers {this} {
}

proc ModelIntoLabelVolumeRemoveMRMLNodeObservers {this} {
}

proc ModelIntoLabelVolumeProcessLogicEvents {this caller event} {
}

proc ModelIntoLabelVolumeProcessGUIEvents {this caller event} {
  
  if { $caller == $::ModelIntoLabelVolume($this,modelsSelect) } {
        set mod [$::ModelIntoLabelVolume($this,modelsSelect) GetSelected]
        if { $mod != "" && [$mod GetPolyData] != ""} {
            # do something 
        }
    }

  if { $caller == $::ModelIntoLabelVolume($this,run) } {
    switch $event {
      "10000" {
        ModelIntoLabelVolumeApply $this
      }
    }
  } 

  ModelIntoLabelVolumeUpdateMRML $this
}

#
# Accessors to ModelIntoLabelVolume state
#


# get the ModelIntoLabelVolume parameter node, or create one if it doesn't exist
proc ModelIntoLabelVolumeCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "ModelIntoLabelVolume"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the ModelIntoLabelVolume parameter node, or create one if it doesn't exist
proc ModelIntoLabelVolumeGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "ModelIntoLabelVolume" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    ModelIntoLabelVolumeCreateParameterNode
    set node [ModelIntoLabelVolumeGetParameterNode]
  }

  return $node
}


proc ModelIntoLabelVolumeGetLabel {} {
  set node [ModelIntoLabelVolumeGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc ModelIntoLabelVolumeSetLabel {index} {
  set node [ModelIntoLabelVolumeGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc ModelIntoLabelVolumeUpdateMRML {this} {
}

proc ModelIntoLabelVolumeProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc ModelIntoLabelVolumeEnter {this} {
}

proc ModelIntoLabelVolumeExit {this} {
}

proc ModelIntoLabelVolumeApply {this} {

  set modelNode [$::ModelIntoLabelVolume($this,modelsSelect) GetSelected]
  set labelValue [[$::ModelIntoLabelVolume($this,labelValueEntry) GetWidget] GetValueAsInt]
  set labelVolumeName [[$::ModelIntoLabelVolume($this,labelVolumeName) GetWidget] GetValue]
  set volumeNode [$::ModelIntoLabelVolume($this,volumesSelect)  GetSelected]
  set intensityFlag [$::ModelIntoLabelVolume($this,intensity) GetSelectedState]
  set subdivideFlag [$::ModelIntoLabelVolume($this,subdivide) GetSelectedState]

  set matrixNode ""
 
  if {$modelNode == ""} {
      set msg "You must select a model first"
  } elseif {$volumeNode == ""} {
      set msg "You must select a reference volume first"
  } else {
      set msg ""
      # set msg "Working on model [$modelNode GetName], ref vol = [$volumeNode GetName], label value = $labelValue, label volume name = $labelVolumeName, intensityFlag = $intensityFlag, subdivideFlag = $subdivideFlag"
  }
  if {$msg != ""} {
      set dialog [vtkKWMessageDialog New]
      $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
      $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
      $dialog SetStyleToMessage
      $dialog SetText $msg
      $dialog Create
      $dialog Invoke
      $dialog Delete
      return
  }

  # subdivide the model node?
  if {$subdivideFlag} {
      set newModelNode [ModelIntoLabelVolumeLogicSubdivideModel [$modelNode GetName]] 
      if {$newModelNode == ""} {
          puts "ERROR subdividing model!"
          return
      } 
      set labelmapVolumeNode [ModelIntoLabelVolumeIntersect $newModelNode $volumeNode $labelValue $labelVolumeName $intensityFlag $matrixNode]
  } else {
      set labelmapVolumeNode [ModelIntoLabelVolumeIntersect $modelNode $volumeNode $labelValue $labelVolumeName $intensityFlag $matrixNode]
  }
  if {$labelmapVolumeNode == ""} {
      puts "ERROR: can't find a new label map volume with name '$labelVolumeName'"
      return
  }
  # set the new label map to be active
  [$::slicer3::ApplicationLogic GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
  [$::slicer3::ApplicationLogic GetSelectionNode] SetReferenceActiveLabelVolumeID [$labelmapVolumeNode GetID]
  $::slicer3::ApplicationLogic PropagateVolumeSelection 0
}
