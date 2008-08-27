#
# DistanceMapFiducials GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc DistanceMapFiducialsConstructor {this} {
}

proc DistanceMapFiducialsDestructor {this} {
}

proc DistanceMapFiducialsTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    run volumesSelect
    volumesOutputSelect volumesFrame 
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

  set ::DistanceMapFiducials($this,volumesOutputSelect) [vtkSlicerNodeSelectorWidget New]
  $::DistanceMapFiducials($this,volumesOutputSelect) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,volumesOutputSelect) Create
  $::DistanceMapFiducials($this,volumesOutputSelect) NewNodeEnabledOn
  $::DistanceMapFiducials($this,volumesOutputSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::DistanceMapFiducials($this,volumesOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::DistanceMapFiducials($this,volumesOutputSelect) UpdateMenu
  $::DistanceMapFiducials($this,volumesOutputSelect) SetLabelText "Output Volume:"
  $::DistanceMapFiducials($this,volumesOutputSelect) SetBalloonHelpString "The target output volume"
  pack [$::DistanceMapFiducials($this,volumesOutputSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::DistanceMapFiducials($this,run) [vtkKWPushButton New]
  $::DistanceMapFiducials($this,run) SetParent [$::DistanceMapFiducials($this,volumesFrame) GetFrame]
  $::DistanceMapFiducials($this,run) Create
  $::DistanceMapFiducials($this,run) SetText "Apply"
  $::DistanceMapFiducials($this,run) SetBalloonHelpString "Apply algorithm."
  pack [$::DistanceMapFiducials($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc DistanceMapFiducialsAddGUIObservers {this} {
  $this AddObserverByNumber $::DistanceMapFiducials($this,run) 10000 
    
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc DistanceMapFiducialsRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
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


proc DistanceMapFiducialsGetLabel {} {
  set node [DistanceMapFiducialsGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc DistanceMapFiducialsSetLabel {index} {
  set node [DistanceMapFiducialsGetParameterNode]
  $node SetParameter "label" $index
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

  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText "Perform action here..."
  $dialog Create
  $dialog Invoke
  $dialog Delete
}
