#
# ScriptedModuleExample GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc ScriptedModuleExampleConstructor {this} {
}

proc ScriptedModuleExampleDestructor {this} {
}

proc ScriptedModuleExampleTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    run volumesSelect
    volumesOutputSelect volumesFrame 
  }

  foreach w $widgets {
    $::ScriptedModuleExample($this,$w) SetParent ""
    $::ScriptedModuleExample($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "ScriptedModuleExample"]
    [$this GetUIPanel] RemovePage "ScriptedModuleExample"
  }

  unset ::ScriptedModuleExample(singleton)

}

proc ScriptedModuleExampleBuildGUI {this} {

  if { [info exists ::ScriptedModuleExample(singleton)] } {
    error "ScriptedModuleExample singleton already created"
  }
  set ::ScriptedModuleExample(singleton) $this

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


  $this SetCategory "Developer Tools"
  [$this GetUIPanel] AddPage "ScriptedModuleExample" "ScriptedModuleExample" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "ScriptedModuleExample"]

  #
  # help frame
  #
  set helptext "The ScriptedModuleExample shows the framework for creating a scripted module in Tcl."
  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # ScriptedModuleExample Volumes
  #
  set ::ScriptedModuleExample($this,volumesFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::ScriptedModuleExample($this,volumesFrame) SetParent $pageWidget
  $::ScriptedModuleExample($this,volumesFrame) Create
  $::ScriptedModuleExample($this,volumesFrame) SetLabelText "Volumes"
  pack [$::ScriptedModuleExample($this,volumesFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::ScriptedModuleExample($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::ScriptedModuleExample($this,volumesSelect) SetParent [$::ScriptedModuleExample($this,volumesFrame) GetFrame]
  $::ScriptedModuleExample($this,volumesSelect) Create
  $::ScriptedModuleExample($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::ScriptedModuleExample($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::ScriptedModuleExample($this,volumesSelect) UpdateMenu
  $::ScriptedModuleExample($this,volumesSelect) SetLabelText "Source Volume:"
  $::ScriptedModuleExample($this,volumesSelect) SetBalloonHelpString "The Source Volume to operate on"
  pack [$::ScriptedModuleExample($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::ScriptedModuleExample($this,volumesOutputSelect) [vtkSlicerNodeSelectorWidget New]
  $::ScriptedModuleExample($this,volumesOutputSelect) SetParent [$::ScriptedModuleExample($this,volumesFrame) GetFrame]
  $::ScriptedModuleExample($this,volumesOutputSelect) Create
  $::ScriptedModuleExample($this,volumesOutputSelect) NewNodeEnabledOn
  $::ScriptedModuleExample($this,volumesOutputSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::ScriptedModuleExample($this,volumesOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::ScriptedModuleExample($this,volumesOutputSelect) UpdateMenu
  $::ScriptedModuleExample($this,volumesOutputSelect) SetLabelText "Output Volume:"
  $::ScriptedModuleExample($this,volumesOutputSelect) SetBalloonHelpString "The target output volume"
  pack [$::ScriptedModuleExample($this,volumesOutputSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::ScriptedModuleExample($this,run) [vtkKWPushButton New]
  $::ScriptedModuleExample($this,run) SetParent [$::ScriptedModuleExample($this,volumesFrame) GetFrame]
  $::ScriptedModuleExample($this,run) Create
  $::ScriptedModuleExample($this,run) SetText "Apply"
  $::ScriptedModuleExample($this,run) SetBalloonHelpString "Apply algorithm."
  pack [$::ScriptedModuleExample($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc ScriptedModuleExampleAddGUIObservers {this} {
  $this AddObserverByNumber $::ScriptedModuleExample($this,run) 10000 
    
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
    
}

proc ScriptedModuleExampleRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
}

proc ScriptedModuleExampleRemoveLogicObservers {this} {
}

proc ScriptedModuleExampleRemoveMRMLNodeObservers {this} {
}

proc ScriptedModuleExampleProcessLogicEvents {this caller event} {
}

proc ScriptedModuleExampleProcessGUIEvents {this caller event} {
  
  if { $caller == $::ScriptedModuleExample($this,run) } {
    switch $event {
      "10000" {
        ScriptedModuleExampleApply $this
      }
    }
  } 

  ScriptedModuleExampleUpdateMRML $this
}

#
# Accessors to ScriptedModuleExample state
#


# get the ScriptedModuleExample parameter node, or create one if it doesn't exist
proc ScriptedModuleExampleCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "ScriptedModuleExample"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the ScriptedModuleExample parameter node, or create one if it doesn't exist
proc ScriptedModuleExampleGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "ScriptedModuleExample" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    ScriptedModuleExampleCreateParameterNode
    set node [ScriptedModuleExampleGetParameterNode]
  }

  return $node
}


proc ScriptedModuleExampleGetLabel {} {
  set node [ScriptedModuleExampleGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc ScriptedModuleExampleSetLabel {index} {
  set node [ScriptedModuleExampleGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc ScriptedModuleExampleUpdateMRML {this} {
}

proc ScriptedModuleExampleProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc ScriptedModuleExampleEnter {this} {
}

proc ScriptedModuleExampleExit {this} {
}

proc ScriptedModuleExampleApply {this} {

  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText "Perform action here..."
  $dialog Create
  $dialog Invoke
  $dialog Delete
}
