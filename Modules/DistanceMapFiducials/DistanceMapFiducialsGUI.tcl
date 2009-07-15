#
# DistanceMapFiducials GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc DistanceMapFiducialsConstructor {this} {
}

proc DistanceMapFiducialsDestructor {this} {
}

proc DistanceMapFiducialsTearDownGUI {this} {
  $::DistanceMapFiducials($this,resample) Delete
  $::DistanceMapFiducials($this,euclideanDistance) Delete
  $::DistanceMapFiducials($this,marchingCubes) Delete
    $::DistanceMapFiducials($this,transform) Delete

  # nodeSelector  ;# disabled for now
  set widgets {
    run volumesSelect
    fiducialOutputSelect distanceScale volumesFrame 
  }

  foreach w $widgets {
    $::DistanceMapFiducials($this,$w) SetParent ""
    $::DistanceMapFiducials($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceMapFiducials"]
    [$this GetUIPanel] RemovePage "DistanceMapFiducials"
  }

  unset ::DistanceMapFiducials(singleton)

}

proc DistanceMapFiducialsBuildGUI {this} {

  if { [info exists ::DistanceMapFiducials(singleton)] } {
    error "DistanceMapFiducials singleton already created"
  }
  set ::DistanceMapFiducials(singleton) $this

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
  [$this GetUIPanel] AddPage "DistanceMapFiducials" "DistanceMapFiducials" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "DistanceMapFiducials"]

  set ::DistanceMapFiducials($this,resample) [vtkImageResample New]
  set ::DistanceMapFiducials($this,euclideanDistance) [vtkImageEuclideanDistance New]
  set ::DistanceMapFiducials($this,marchingCubes) [vtkMarchingCubes New]
  set ::DistanceMapFiducials($this,transform) [vtkTransformPolyDataFilter New]

  $::DistanceMapFiducials($this,marchingCubes) ComputeNormalsOff 
  $::DistanceMapFiducials($this,marchingCubes) ComputeGradientsOff
  $::DistanceMapFiducials($this,marchingCubes) ComputeScalarsOff
  $::DistanceMapFiducials($this,euclideanDistance) SetAlgorithmToSaito


  #
  # help frame
  #
  set helptext "The DistanceMapFiducials shows the framework for creating a scripted module in Tcl."
  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # DistanceMapFiducials Volumes
  #
  set ::DistanceMapFiducials($this,volumesFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::DistanceMapFiducials($this,volumesFrame) SetParent $pageWidget
  $::DistanceMapFiducials($this,volumesFrame) Create
  $::DistanceMapFiducials($this,volumesFrame) SetLabelText "Volumes"
  pack [$::DistanceMapFiducials($this,volumesFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::DistanceMapFiducials($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::DistanceMapFiducials($this,volumesSelect) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,volumesSelect) Create
  $::DistanceMapFiducials($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::DistanceMapFiducials($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::DistanceMapFiducials($this,volumesSelect) UpdateMenu
  $::DistanceMapFiducials($this,volumesSelect) SetLabelText "Source Volume:"
  $::DistanceMapFiducials($this,volumesSelect) SetBalloonHelpString "The Source Volume to operate on"
  pack [$::DistanceMapFiducials($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapFiducials($this,fiducialOutputSelect) [vtkSlicerNodeSelectorWidget New]
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,fiducialOutputSelect) Create
  $::DistanceMapFiducials($this,fiducialOutputSelect) NewNodeEnabledOn
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetNodeClass "vtkMRMLFiducialListNode" "" "" ""
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetNewNodeEnabled 1
  $::DistanceMapFiducials($this,fiducialOutputSelect) UpdateMenu
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetLabelText "Output Fiducial List:"
  $::DistanceMapFiducials($this,fiducialOutputSelect) SetBalloonHelpString "The target output fiducial list"
  pack [$::DistanceMapFiducials($this,fiducialOutputSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  set ::DistanceMapFiducials($this,resampleScale) [vtkKWScaleWithLabel New]
  $::DistanceMapFiducials($this,resampleScale) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,resampleScale) Create
  $::DistanceMapFiducials($this,resampleScale) SetLabelText "Downsample Factor"
  [$::DistanceMapFiducials($this,resampleScale) GetWidget] SetRange 1 10
  [$::DistanceMapFiducials($this,resampleScale) GetWidget] SetResolution 1
  [$::DistanceMapFiducials($this,resampleScale) GetWidget] SetValue 2
  $::DistanceMapFiducials($this,resampleScale) SetBalloonHelpString "Downsampling of input volume"
  pack [$::DistanceMapFiducials($this,resampleScale) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapFiducials($this,distanceScale) [vtkKWScaleWithLabel New]
  $::DistanceMapFiducials($this,distanceScale) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,distanceScale) Create
  $::DistanceMapFiducials($this,distanceScale) SetLabelText "Distance"
  [$::DistanceMapFiducials($this,distanceScale) GetWidget] SetRange 0 100
  [$::DistanceMapFiducials($this,distanceScale) GetWidget] SetResolution 0.5
  [$::DistanceMapFiducials($this,distanceScale) GetWidget] SetValue 5
  $::DistanceMapFiducials($this,distanceScale) SetBalloonHelpString "Distance form the label map"
  pack [$::DistanceMapFiducials($this,distanceScale) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapFiducials($this,run) [vtkKWPushButton New]
  $::DistanceMapFiducials($this,run) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,run) Create
  $::DistanceMapFiducials($this,run) SetText "Apply"
  $::DistanceMapFiducials($this,run) SetBalloonHelpString "Apply algorithm."
  pack [$::DistanceMapFiducials($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc DistanceMapFiducialsAddGUIObservers {this} {
  $this AddObserverByNumber $::DistanceMapFiducials($this,run) 10000 
    
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
    
}

proc DistanceMapFiducialsRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
}

proc DistanceMapFiducialsRemoveLogicObservers {this} {
}

proc DistanceMapFiducialsRemoveMRMLNodeObservers {this} {
}

proc DistanceMapFiducialsProcessLogicEvents {this caller event} {
}

proc DistanceMapFiducialsProcessGUIEvents {this caller event} {
  
  if { $caller == $::DistanceMapFiducials($this,run) } {
    switch $event {
      "10000" {
        DistanceMapFiducialsApply $this
      }
    }
  } 

  DistanceMapFiducialsUpdateMRML $this
}

#
# Accessors to DistanceMapFiducials state
#


# get the DistanceMapFiducials parameter node, or create one if it doesn't exist
proc DistanceMapFiducialsCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "DistanceMapFiducials"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the DistanceMapFiducials parameter node, or create one if it doesn't exist
proc DistanceMapFiducialsGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "DistanceMapFiducials" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    DistanceMapFiducialsCreateParameterNode
    set node [DistanceMapFiducialsGetParameterNode]
  }

  return $node
}


#
# MRML Event processing
#

proc DistanceMapFiducialsUpdateMRML {this} {
}

proc DistanceMapFiducialsProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc DistanceMapFiducialsEnter {this} {
}

proc DistanceMapFiducialsExit {this} {
}

proc DistanceMapFiducialsApply {this} {

  set vol [$::DistanceMapFiducials($this,volumesSelect) GetSelected]
  set fid [$::DistanceMapFiducials($this,fiducialOutputSelect) GetSelected]
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
  
  $::DistanceMapFiducials($this,transform) SetTransform $trans

  set dist [[$::DistanceMapFiducials($this,distanceScale) GetWidget] GetValue]
  set dist [expr $dist * $dist]

  set resamp [[$::DistanceMapFiducials($this,resampleScale) GetWidget] GetValue]
  set resamp [expr 1.0/$resamp]

  $img DeepCopy [$vol GetImageData]
  $img SetSpacing [lindex $spacing 0] [lindex $spacing 1] [lindex $spacing 2]

  $::DistanceMapFiducials($this,resample) SetInput $img
  $::DistanceMapFiducials($this,resample) SetAxisMagnificationFactor 0 $resamp
  $::DistanceMapFiducials($this,resample) SetAxisMagnificationFactor 1 $resamp
  $::DistanceMapFiducials($this,resample) SetAxisMagnificationFactor 2 $resamp
  $::DistanceMapFiducials($this,resample) Update
  
  $::DistanceMapFiducials($this,euclideanDistance) SetInput [$::DistanceMapFiducials($this,resample) GetOutput]
  $::DistanceMapFiducials($this,euclideanDistance) SetInitialize 1
  $::DistanceMapFiducials($this,euclideanDistance) SetAlgorithmToSaitoCached
  $::DistanceMapFiducials($this,euclideanDistance) Update
  $::DistanceMapFiducials($this,marchingCubes) SetInput [$::DistanceMapFiducials($this,euclideanDistance) GetOutput]
  $::DistanceMapFiducials($this,marchingCubes) SetValue 0 $dist
  $::DistanceMapFiducials($this,marchingCubes) Update

  $::DistanceMapFiducials($this,transform) SetInput [$::DistanceMapFiducials($this,marchingCubes) GetOutput]
  $::DistanceMapFiducials($this,transform) Update

  $fid SetDisableModifiedEvent 1
  $fid RemoveAllFiducials

  set poly [$::DistanceMapFiducials($this,transform) GetOutput]
  set nvert [$poly GetNumberOfPoints]
  for {set i 0} {$i < $nvert} {incr i} {
    set xyz [$poly GetPoint $i]
    $fid AddFiducialWithXYZ [lindex $xyz 0] [lindex $xyz 1] [lindex $xyz 2] 0
    if {$i == [expr $nvert - 2]} {
      $fid SetDisableModifiedEvent 0
    }
  }
  $img Delete
  $mat Delete
  $trans Delete
}
