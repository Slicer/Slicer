#
# DistanceTransformModel GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc DistanceTransformModelConstructor {this} {
}

proc DistanceTransformModelDestructor {this} {
}

proc DistanceTransformModelTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    run volumesSelect
    volumeOutputSelect thresholdSelect
    minFactor
    modelOutputSelect dataFrame 
  }

  foreach w $widgets {
    $::DistanceTransformModel($this,$w) SetParent ""
    $::DistanceTransformModel($this,$w) Delete
  }

  $::DistanceTransformModel($this,cast) Delete
  $::DistanceTransformModel($this,distanceTransform) Delete
  $::DistanceTransformModel($this,resample) Delete
  $::DistanceTransformModel($this,marchingCubes) Delete
  $::DistanceTransformModel($this,changeIn) Delete
  $::DistanceTransformModel($this,changeOut) Delete
  $::DistanceTransformModel($this,polyTransform) Delete
  $::DistanceTransformModel($this,polyTransformFilter) Delete

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceTransformModel"]
    [$this GetUIPanel] RemovePage "DistanceTransformModel"
  }

  unset ::DistanceTransformModel(singleton)

}

proc DistanceTransformModelBuildGUI {this} {

  if { [info exists ::DistanceTransformModel(singleton)] } {
    error "DistanceTransformModel singleton already created"
  }
  set ::DistanceTransformModel(singleton) $this

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
  [$this GetUIPanel] AddPage "DistanceTransformModel" "DistanceTransformModel" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceTransformModel"]

  #
  # help frame
  #
  set helptext "The DistanceTransformModel updates a model node based on the distance the boundary of a label map."
  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The Distance Transform module was contributed by Steve Pieper, Isomics, Inc., and uses the NumpyScript module created by Luca Antiga."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # DistanceTransformModel Volumes
  #
  set ::DistanceTransformModel($this,dataFrame) [vtkSlicerModuleCollapsibleFrame New]
  set frame $::DistanceTransformModel($this,dataFrame)
  $frame SetParent $pageWidget
  $frame Create
  $frame SetLabelText "Volumes"
  pack [$frame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::DistanceTransformModel($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  set select $::DistanceTransformModel($this,volumesSelect)
  $select SetParent [$frame GetFrame]
  $select Create
  $select SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $select SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $select UpdateMenu
  $select SetLabelText "Source Volume:"
  $select SetBalloonHelpString "The Source Volume to operate on"
  pack [$select GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceTransformModel($this,volumeOutputSelect) [vtkSlicerNodeSelectorWidget New]
  set outSelect $::DistanceTransformModel($this,volumeOutputSelect)
  $outSelect SetParent [$frame GetFrame]
  $outSelect Create
  $outSelect NewNodeEnabledOn
  $outSelect SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $outSelect SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $outSelect UpdateMenu
  $outSelect SetLabelText "Output Distance Transform Volume:"
  $outSelect SetBalloonHelpString "The target output volume"
  pack [$outSelect GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceTransformModel($this,modelOutputSelect) [vtkSlicerNodeSelectorWidget New]
  set outMSelect $::DistanceTransformModel($this,modelOutputSelect)
  $outMSelect SetParent [$frame GetFrame]
  $outMSelect Create
  $outMSelect NewNodeEnabledOn
  $outMSelect SetNodeClass "vtkMRMLModelNode" "" "" ""
  $outMSelect SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $outMSelect UpdateMenu
  $outMSelect SetLabelText "Output Model:"
  $outMSelect SetBalloonHelpString "The target output model"
  pack [$outMSelect GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceTransformModel($this,minFactor) [vtkKWThumbWheel New]
  set minThumb $::DistanceTransformModel($this,minFactor)
  $minThumb SetParent [$frame GetFrame]
  $minThumb PopupModeOff
  $minThumb Create
  $minThumb DisplayEntryAndLabelOnTopOn
  $minThumb DisplayEntryOn
  $minThumb DisplayLabelOn
  $minThumb SetResolution 0.01
  $minThumb SetMinimumValue 0.01
  $minThumb SetClampMinimumValue 1
  $minThumb SetValue 0.1
  [$minThumb GetLabel] SetText "Zoom Factor:"
  $minThumb SetBalloonHelpString "Amount to rescale the input volume before running the distance map and surface filters"
  pack [$minThumb GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceTransformModel($this,thresholdSelect) [vtkKWThumbWheel New]
  set thumb $::DistanceTransformModel($this,thresholdSelect)
  $thumb SetParent [$frame GetFrame]
  $thumb PopupModeOff
  $thumb Create
  $thumb DisplayEntryAndLabelOnTopOn
  $thumb DisplayEntryOn
  $thumb DisplayLabelOn
  $thumb SetResolution 0.1
  $thumb SetValue 0
  [$thumb GetLabel] SetText "Threshold:"
  $thumb SetBalloonHelpString "Value at which to generate the isosurface"
  pack [$thumb GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceTransformModel($this,run) [vtkKWPushButton New]
  set run $::DistanceTransformModel($this,run)
  $run SetParent [$frame GetFrame]
  $run Create
  $run SetText "Apply"
  $run SetBalloonHelpString "Apply algorithm."
  pack [$run GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  set ::DistanceTransformModel($this,cast) [vtkImageCast New]
  set ::DistanceTransformModel($this,distanceTransform) [vtkITKDistanceTransform New]
  set ::DistanceTransformModel($this,resample) [vtkImageResample New]
  set ::DistanceTransformModel($this,marchingCubes) [vtkMarchingCubes New]
  set ::DistanceTransformModel($this,changeIn) [vtkImageChangeInformation New]
  set ::DistanceTransformModel($this,changeOut) [vtkImageChangeInformation New]
  set ::DistanceTransformModel($this,polyTransform) [vtkTransform New]
  set ::DistanceTransformModel($this,polyTransformFilter) [vtkTransformPolyDataFilter New]

}

proc DistanceTransformModelAddGUIObservers {this} {
  $this AddObserverByNumber $::DistanceTransformModel($this,run) 10000 
  $this AddObserverByNumber $::DistanceTransformModel($this,thresholdSelect) 10000
  $this AddObserverByNumber $::DistanceTransformModel($this,minFactor) 10001
    
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc DistanceTransformModelRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
}

proc DistanceTransformModelRemoveLogicObservers {this} {
}

proc DistanceTransformModelRemoveMRMLNodeObservers {this} {
}

proc DistanceTransformModelProcessLogicEvents {this caller event} {
}

proc DistanceTransformModelProcessGUIEvents {this caller event} {
  
  if { $caller == $::DistanceTransformModel($this,run) } {
    switch $event {
      "10000" {
        DistanceTransformModelApply $this
      }
    }
  } 

  if { $caller == $::DistanceTransformModel($this,thresholdSelect) } {
    switch $event {
      "10000" {
        DistanceTransformModelApply $this
      }
    }
  } 

  if { $caller == $::DistanceTransformModel($this,minFactor) } {
    switch $event {
      "10001" {
        DistanceTransformModelApply $this
      }
    }
  } 

  DistanceTransformModelUpdateMRML $this
}

#
# Accessors to DistanceTransformModel state
#


# get the DistanceTransformModel parameter node, or create one if it doesn't exist
proc DistanceTransformModelCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "DistanceTransformModel"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the DistanceTransformModel parameter node, or create one if it doesn't exist
proc DistanceTransformModelGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "DistanceTransformModel" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    DistanceTransformModelCreateParameterNode
    set node [DistanceTransformModelGetParameterNode]
  }

  return $node
}


proc DistanceTransformModelGetLabel {} {
  set node [DistanceTransformModelGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc DistanceTransformModelSetLabel {index} {
  set node [DistanceTransformModelGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc DistanceTransformModelUpdateMRML {this} {
}

proc DistanceTransformModelProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc DistanceTransformModelEnter {this} {
}

proc DistanceTransformModelExit {this} {
}

proc DistanceTransformModelProgressEventCallback {filter} {

  set mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  set progressGauge [$mainWindow GetProgressGauge]
  set renderWidget [[$::slicer3::ApplicationGUI GetViewControlGUI] GetNavigationWidget]

  if { $filter == "" } {
    $mainWindow SetStatusText ""
    $progressGauge SetValue 0
    $renderWidget SetRendererGradientBackground 0
  } else {
    # TODO: this causes a tcl 'update' which re-triggers the module (possibly changing
    # values while it is executing!  Talk about evil...
    #$mainWindow SetStatusText [$filter GetClassName]
    #$progressGauge SetValue [expr 100 * [$filter GetProgress]]

    set progress [$filter GetProgress]
    set remaining [expr 1.0 - $progress]

    #$renderWidget SetRendererGradientBackground 1
    #$renderWidget SetRendererBackgroundColor $progress $progress $progress
    #$renderWidget SetRendererBackgroundColor2 $remaining $remaining $remaining
  }
}

proc DistanceTransformModelApply {this} {

  if { ![info exists ::DistanceTransformModel($this,processing)] } { 
    set ::DistanceTransformModel($this,processing) 0
  }

  if { $::DistanceTransformModel($this,processing) } {
    return
  }

  set volumeNode [$::DistanceTransformModel($this,volumesSelect) GetSelected]
  set ijkToRAS [vtkMatrix4x4 New]
  $volumeNode GetIJKToRASMatrix $ijkToRAS
  set outVolumeNode [$::DistanceTransformModel($this,volumeOutputSelect) GetSelected]
  set outModelNode [$::DistanceTransformModel($this,modelOutputSelect) GetSelected]
  set value [$::DistanceTransformModel($this,thresholdSelect) GetValue]
  set minFactor [$::DistanceTransformModel($this,minFactor) GetValue]
  set cast $::DistanceTransformModel($this,cast)
  set dt $::DistanceTransformModel($this,distanceTransform)
  set resample $::DistanceTransformModel($this,resample)
  set cubes $::DistanceTransformModel($this,marchingCubes)
  set changeIn $::DistanceTransformModel($this,changeIn)
  set changeOut $::DistanceTransformModel($this,changeOut)
  set polyTransformFilter $::DistanceTransformModel($this,polyTransformFilter)
  set polyTransform $::DistanceTransformModel($this,polyTransform)

  #
  # check that inputs are valid
  #
  set errorText ""
  if { $volumeNode == "" || [$volumeNode GetImageData] == "" } {
    set errorText "No input volume data..."
  }
  if { $outVolumeNode == "" } {
    set errorText "No output volume node..."
  }
  if { $outModelNode == "" } {
    set errorText "No output model node..."
  }

  if { $errorText != "" } {
    set dialog [vtkKWMessageDialog New]
    $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetStyleToMessage
    $dialog SetText $errorText
    $dialog Create
    $dialog Invoke
    $dialog Delete
    return
  }

  set ::DistanceTransformModel($this,processing) 1

  #
  # configure the pipeline
  #
  $changeIn SetInput [$volumeNode GetImageData]
  eval $changeIn SetOutputSpacing [$volumeNode GetSpacing]
  $cast SetInput [$changeIn GetOutput]
  $cast SetOutputScalarTypeToFloat
  $dt SetInput [$cast GetOutput]
  $dt SetSquaredDistance 0
  $dt SetUseImageSpacing 1
  $dt SetInsideIsPositive 0
  $changeOut SetInput [$dt GetOutput]
  $changeOut SetOutputSpacing 1 1 1
  $resample SetInput [$dt GetOutput]
  $resample SetAxisMagnificationFactor 0 $minFactor
  $resample SetAxisMagnificationFactor 1 $minFactor
  $resample SetAxisMagnificationFactor 2 $minFactor
  $cubes SetInput [$resample GetOutput]
  $cubes SetValue 0 $value
  $polyTransformFilter SetInput [$cubes GetOutput]
  $polyTransformFilter SetTransform $polyTransform
  set magFactor [expr 1.0 / $minFactor]
  $polyTransform Identity
  $polyTransform Concatenate $ijkToRAS
  foreach sp [$volumeNode GetSpacing] {
    lappend invSpacing [expr 1. / $sp]
  }
  eval $polyTransform Scale $invSpacing

  #
  # set up progress observers
  #
  set observerRecords ""
  set filters "$changeIn $resample $dt $changeOut $cubes"
  foreach filter $filters {
    set tag [$filter AddObserver ProgressEvent "DistanceTransformModelProgressEventCallback $filter"]
    lappend observerRecords "$filter $tag"
  }

  #
  # activate the pipeline
  #
  $polyTransformFilter Update

  # remove progress observers
  foreach record $observerRecords {
    foreach {filter tag} $record {
      $filter RemoveObserver $tag
    }
  }
  DistanceTransformModelProgressEventCallback ""

  #
  # create a mrml model display node if needed
  #
  if { [$outModelNode GetDisplayNode] == "" } {
    set modelDisplayNode [vtkMRMLModelDisplayNode New]
    $outModelNode SetScene $::slicer3::MRMLScene
    eval $modelDisplayNode SetColor .5 1 1
    $::slicer3::MRMLScene AddNode $modelDisplayNode
    $outModelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]
  }

  #
  # set the output into the MRML scene
  #
  $outModelNode SetAndObservePolyData [$polyTransformFilter GetOutput]

  $outVolumeNode SetAndObserveImageData [$changeOut GetOutput]
  $outVolumeNode SetIJKToRASMatrix $ijkToRAS
  $ijkToRAS Delete


  set ::DistanceTransformModel($this,processing) 0
}
