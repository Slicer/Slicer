#
# DistanceMapModel GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc DistanceMapModelConstructor {this} {
}

proc DistanceMapModelDestructor {this} {
}

proc DistanceMapModelTearDownGUI {this} {
  $::DistanceMapModel($this,resample) Delete
  $::DistanceMapModel($this,euclideanDistance) Delete
  $::DistanceMapModel($this,marchingCubes) Delete
    $::DistanceMapModel($this,transform) Delete

  # nodeSelector  ;# disabled for now
  set widgets {
    run volumesSelect
    modelOutputSelect distanceScale volumesFrame 
  }

  foreach w $widgets {
    $::DistanceMapModel($this,$w) SetParent ""
    $::DistanceMapModel($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceMapModel"]
    [$this GetUIPanel] RemovePage "DistanceMapModel"
  }

  unset ::DistanceMapModel(singleton)

}

proc DistanceMapModelBuildGUI {this} {

  if { [info exists ::DistanceMapModel(singleton)] } {
    error "DistanceMapModel singleton already created"
  }
  set ::DistanceMapModel(singleton) $this

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


  $this SetCategory "Demonstration"
  [$this GetUIPanel] AddPage "DistanceMapModel" "DistanceMapModel" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceMapModel"]

  set ::DistanceMapModel($this,resample) [vtkImageResample New]
  set ::DistanceMapModel($this,euclideanDistance) [vtkITKDistanceTransform New]
  set ::DistanceMapModel($this,marchingCubes) [vtkMarchingCubes New]
  set ::DistanceMapModel($this,transform) [vtkTransformPolyDataFilter New]

  $::DistanceMapModel($this,marchingCubes) ComputeNormalsOff 
  $::DistanceMapModel($this,marchingCubes) ComputeGradientsOff
  $::DistanceMapModel($this,marchingCubes) ComputeScalarsOff
  $::DistanceMapModel($this,euclideanDistance) SetSquaredDistance 0
  $::DistanceMapModel($this,euclideanDistance) SetInsideIsPositive 0
  $::DistanceMapModel($this,euclideanDistance) SetUseImageSpacing 1


  #
  # help frame
  #
  set helptext "The DistanceMapModel shows the framework for creating a scripted module in Tcl."
  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # DistanceMapModel Volumes
  #
  set ::DistanceMapModel($this,volumesFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::DistanceMapModel($this,volumesFrame) SetParent $pageWidget
  $::DistanceMapModel($this,volumesFrame) Create
  $::DistanceMapModel($this,volumesFrame) SetLabelText "Volumes"
  pack [$::DistanceMapModel($this,volumesFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::DistanceMapModel($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::DistanceMapModel($this,volumesSelect) SetParent [$::DistanceMapModel($this,volumesFrame) GetFrame]
  $::DistanceMapModel($this,volumesSelect) Create
  $::DistanceMapModel($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::DistanceMapModel($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::DistanceMapModel($this,volumesSelect) UpdateMenu
  $::DistanceMapModel($this,volumesSelect) SetLabelText "Source Label Volume:"
  $::DistanceMapModel($this,volumesSelect) SetBalloonHelpString "The Source Label Volume to operate on"
  pack [$::DistanceMapModel($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapModel($this,modelOutputSelect) [vtkSlicerNodeSelectorWidget New]
  $::DistanceMapModel($this,modelOutputSelect) SetParent [$::DistanceMapModel($this,volumesFrame) GetFrame]
  $::DistanceMapModel($this,modelOutputSelect) Create
  $::DistanceMapModel($this,modelOutputSelect) NewNodeEnabledOn
  $::DistanceMapModel($this,modelOutputSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
  $::DistanceMapModel($this,modelOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::DistanceMapModel($this,modelOutputSelect) SetNewNodeEnabled 1
  $::DistanceMapModel($this,modelOutputSelect) UpdateMenu
  $::DistanceMapModel($this,modelOutputSelect) SetLabelText "Output Model:"
  $::DistanceMapModel($this,modelOutputSelect) SetBalloonHelpString "The target output model"
  pack [$::DistanceMapModel($this,modelOutputSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  set ::DistanceMapModel($this,resampleScale) [vtkKWScaleWithLabel New]
  $::DistanceMapModel($this,resampleScale) SetParent [$::DistanceMapModel($this,volumesFrame) GetFrame]
  $::DistanceMapModel($this,resampleScale) Create
  $::DistanceMapModel($this,resampleScale) SetLabelText "Downsample Factor"
  [$::DistanceMapModel($this,resampleScale) GetWidget] SetRange 1 10
  [$::DistanceMapModel($this,resampleScale) GetWidget] SetResolution 1
  [$::DistanceMapModel($this,resampleScale) GetWidget] SetValue 2
  $::DistanceMapModel($this,resampleScale) SetBalloonHelpString "Downsampling of input volume"
  pack [$::DistanceMapModel($this,resampleScale) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapModel($this,distanceScale) [vtkKWScaleWithLabel New]
  $::DistanceMapModel($this,distanceScale) SetParent [$::DistanceMapModel($this,volumesFrame) GetFrame]
  $::DistanceMapModel($this,distanceScale) Create
  $::DistanceMapModel($this,distanceScale) SetLabelText "Distance"
  [$::DistanceMapModel($this,distanceScale) GetWidget] SetRange -20 100
  [$::DistanceMapModel($this,distanceScale) GetWidget] SetResolution 1
  [$::DistanceMapModel($this,distanceScale) GetWidget] SetValue 5
  $::DistanceMapModel($this,distanceScale) SetBalloonHelpString "Distance form the label map"
  pack [$::DistanceMapModel($this,distanceScale) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapModel($this,run) [vtkKWPushButton New]
  $::DistanceMapModel($this,run) SetParent [$::DistanceMapModel($this,volumesFrame) GetFrame]
  $::DistanceMapModel($this,run) Create
  $::DistanceMapModel($this,run) SetText "Apply"
  $::DistanceMapModel($this,run) SetBalloonHelpString "Apply algorithm."
  pack [$::DistanceMapModel($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc DistanceMapModelAddGUIObservers {this} {
  $this AddObserverByNumber $::DistanceMapModel($this,run) 10000 
    
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc DistanceMapModelRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
}

proc DistanceMapModelRemoveLogicObservers {this} {
}

proc DistanceMapModelRemoveMRMLNodeObservers {this} {
}

proc DistanceMapModelProcessLogicEvents {this caller event} {
}

proc DistanceMapModelProcessGUIEvents {this caller event} {
  
  if { $caller == $::DistanceMapModel($this,run) } {
    switch $event {
      "10000" {
        DistanceMapModelApply $this
      }
    }
  } 

  DistanceMapModelUpdateMRML $this
}

#
# Accessors to DistanceMapModel state
#


# get the DistanceMapModel parameter node, or create one if it doesn't exist
proc DistanceMapModelCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "DistanceMapModel"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the DistanceMapModel parameter node, or create one if it doesn't exist
proc DistanceMapModelGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "DistanceMapModel" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    DistanceMapModelCreateParameterNode
    set node [DistanceMapModelGetParameterNode]
  }

  return $node
}


#
# MRML Event processing
#

proc DistanceMapModelUpdateMRML {this} {
}

proc DistanceMapModelProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc DistanceMapModelEnter {this} {
}

proc DistanceMapModelExit {this} {
}

proc DistanceMapModelApply {this} {

  set vol [$::DistanceMapModel($this,volumesSelect) GetSelected]
  set fid [$::DistanceMapModel($this,modelOutputSelect) GetSelected]
  if { $vol == "" || $fid == "" } {
        return
    }
  set img [vtkImageData New]
  set spacing [$vol GetSpacing]

  set mat [vtkMatrix4x4 New]
  $vol GetRASToIJKMatrix $mat
  $mat Invert

  set trans [vtkTransform New]
  $trans Identity
  $trans PreMultiply
  $trans Concatenate $mat
  $trans Scale [expr 1.0/[lindex $spacing 0]] [expr 1.0/[lindex $spacing 1]] [expr 1.0/[lindex $spacing 2]]
  
  $::DistanceMapModel($this,transform) SetTransform $trans

  set dist [[$::DistanceMapModel($this,distanceScale) GetWidget] GetValue]
  #set dist [expr $dist * $dist]

  set resamp [[$::DistanceMapModel($this,resampleScale) GetWidget] GetValue]
  set resamp [expr 1.0/$resamp]

  $img DeepCopy [$vol GetImageData]
  $img SetSpacing [lindex $spacing 0] [lindex $spacing 1] [lindex $spacing 2]

  $::DistanceMapModel($this,resample) SetInput $img
  $::DistanceMapModel($this,resample) SetAxisMagnificationFactor 0 $resamp
  $::DistanceMapModel($this,resample) SetAxisMagnificationFactor 1 $resamp
  $::DistanceMapModel($this,resample) SetAxisMagnificationFactor 2 $resamp
  $::DistanceMapModel($this,resample) Update
  
  $::DistanceMapModel($this,euclideanDistance) SetInput [$::DistanceMapModel($this,resample) GetOutput]
  $::DistanceMapModel($this,euclideanDistance) Update

  $::DistanceMapModel($this,marchingCubes) SetInput [$::DistanceMapModel($this,euclideanDistance) GetOutput]
  $::DistanceMapModel($this,marchingCubes) SetValue 0 $dist
  $::DistanceMapModel($this,marchingCubes) Update

  $::DistanceMapModel($this,transform) SetInput [$::DistanceMapModel($this,marchingCubes) GetOutput]
  $::DistanceMapModel($this,transform) Update

  set poly [$::DistanceMapModel($this,transform) GetOutput]
  $fid SetAndObservePolyData $poly

  if {[$fid GetDisplayNode] == ""} {
      set dnode [vtkMRMLModelDisplayNode New]
      $::slicer3::MRMLScene AddNode $dnode
      $fid SetAndObserveDisplayNodeID [$dnode GetID]
  }
  $img Delete
  $mat Delete
  $trans Delete
}
