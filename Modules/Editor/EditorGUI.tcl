#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {

  set boxes [itcl::find objects -isa Box]
  foreach b $boxes {
    itcl::delete object $b
  }
}


# Note: not a method - this is invoked directly by the GUI
# - remove the GUI from the current Application
# - re-source the code (this file)
# - re-build the GUI
# Note: not a method - this is invoked directly by the GUI
proc EditorTearDownGUI {this} {

  # nodeSelector  ;# disabled for now
  set widgets {
    volumesCreate volumeName volumesSelect
    volumesFrame 
    optionsSpacer optionsFrame
    toolsActiveTool toolsEditFrame toolsColorFrame
    toolsFrame 
  }

  itcl::delete object $::Editor($this,editColor)
  itcl::delete object $::Editor($this,editBox)

  foreach w $widgets {
    $::Editor($this,$w) SetParent ""
    $::Editor($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]
    [$this GetUIPanel] RemovePage "Editor"
  }

}

proc EditorBuildGUI {this} {

  if { [info exists ::Editor(singleton)] } {
    error "editor singleton already created"
  }
  set ::Editor(singleton) $this

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


  [$this GetUIPanel] AddPage "Editor" "Editor" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]

  #
  # help frame
  #
  set helptext "The Editor allows label maps to be created and edited. The active label map will be modified by the Editor. This module is currently a prototype and will be under active development throughout 3DSlicer's Beta release."
  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext

  if { 0 } { 
    # leave out the node selector for now - we'll use one global node and save GUI space
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
  }

  #
  # Editor Volumes
  #
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
  # Tool Frame
  #
  set ::Editor($this,toolsFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::Editor($this,toolsFrame) SetParent $pageWidget
  $::Editor($this,toolsFrame) Create
  $::Editor($this,toolsFrame) SetLabelText "Tools"
  pack [$::Editor($this,toolsFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  set ::Editor($this,toolsEditFrame) [vtkKWFrame New]
  $::Editor($this,toolsEditFrame) SetParent [$::Editor($this,toolsFrame) GetFrame]
  $::Editor($this,toolsEditFrame) Create
  pack [$::Editor($this,toolsEditFrame) GetWidgetName] \
    -side right -anchor ne -padx 2 -pady 2

  set ::Editor($this,toolsColorFrame) [vtkKWFrame New]
  $::Editor($this,toolsColorFrame) SetParent [$::Editor($this,toolsFrame) GetFrame]
  $::Editor($this,toolsColorFrame) Create
  $::Editor($this,toolsColorFrame) SetBackgroundColor [expr 232/255.] [expr 230/255.] [expr 235/255.]
  pack [$::Editor($this,toolsColorFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2

  set ::Editor($this,toolsActiveTool) [vtkKWLabelWithLabel New]
  $::Editor($this,toolsActiveTool) SetParent [$::Editor($this,toolsFrame) GetFrame]
  $::Editor($this,toolsActiveTool) Create
  $::Editor($this,toolsActiveTool) SetBackgroundColor [expr 232/255.] [expr 230/255.] [expr 235/255.]
  [$::Editor($this,toolsActiveTool) GetWidget] SetBackgroundColor [expr 232/255.] [expr 230/255.] [expr 235/255.]
  [$::Editor($this,toolsActiveTool) GetLabel] SetBackgroundColor [expr 232/255.] [expr 230/255.] [expr 235/255.]
  $::Editor($this,toolsActiveTool) SetLabelText "Active Tool: "
  pack [$::Editor($this,toolsActiveTool) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2
  
  
  # create the edit box and the color picker - note these aren't kwwidgets
  #  but helper classes that create kwwidgets in the given frame
  set ::Editor($this,editColor) [::EditColor #auto]
  $::Editor($this,editColor) configure -frame $::Editor($this,toolsColorFrame)
  $::Editor($this,editColor) create
  set ::Editor($this,editBox) [::EditBox #auto]
  $::Editor($this,editBox) configure -frame $::Editor($this,toolsEditFrame)
  $::Editor($this,editBox) create

  #
  # Tool Options
  #
  set ::Editor($this,optionsFrame) [vtkKWFrame New]
  $::Editor($this,optionsFrame) SetParent [$::Editor($this,toolsFrame) GetFrame]
  $::Editor($this,optionsFrame) Create
  $::Editor($this,optionsFrame) SetBorderWidth 1
  $::Editor($this,optionsFrame) SetReliefToSolid
  pack [$::Editor($this,optionsFrame) GetWidgetName] \
    -side left -anchor nw -fill both -padx 2 -pady 2 

  # one pixel label to keep the option panel expanded
    # TODO: doesn't work as intended
  set ::Editor($this,optionsSpacer) [vtkKWLabel New]
  $::Editor($this,optionsSpacer) SetParent $::Editor($this,optionsFrame)
  $::Editor($this,optionsSpacer) Create
  pack [$::Editor($this,optionsSpacer) GetWidgetName] \
    -fill both -expand true 

}

proc EditorAddGUIObservers {this} {
  $this AddObserverByNumber $::Editor($this,volumesCreate) 10000 
    
# $this DebugOn
  if {[$this GetDebug]} {
    puts "Adding mrml observer to selection node, modified event"
  }
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc EditorRemoveGUIObservers {this} {
  if {[$this GetDebug]} {
    puts "Removing mrml observer on selection node, modified event"
  }
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
}

proc EditorRemoveLogicObservers {this} {
}

proc EditorRemoveMRMLNodeObservers {this} {
}

proc EditorProcessLogicEvents {this caller event} {
}

proc EditorProcessGUIEvents {this caller event} {
  
  if { $caller == $::Editor($this,volumesCreate) } {
    switch $event {
      "10000" {
        EditorCreateLabelVolume $this
      }
    }
  } 

  EditorUpdateMRML $this
}

#
# Accessors to editor state
#


# get the editor parameter node, or create one if it doesn't exist
proc EditorCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "Editor"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the editor parameter node, or create one if it doesn't exist
proc EditorGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "Editor" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    EditorCreateParameterNode
    set node [EditorGetParameterNode]
  }

  return $node
}

proc EditorSetActiveToolLabel {name} {
  [$::Editor($::Editor(singleton),toolsActiveTool) GetWidget] SetText $name
  [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText $name
}

proc EditorGetPaintLabel {} {
  set node [EditorGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc EditorSetPaintLabel {index} {
  set node [EditorGetParameterNode]
  $node SetParameter "label" $index
}

proc EditorGetPaintColor {this} {

  set sliceLogic [$::slicer3::ApplicationGUI GetMainSliceLogic0]
  if { $sliceLogic != "" } {
    set logic [$sliceLogic GetLabelLayer]
    if { $logic != "" } {
      set volumeDisplayNode [$logic GetVolumeDisplayNode]
      if { $volumeDisplayNode != "" } {
        set node [$volumeDisplayNode GetColorNode]
        set lut [$node GetLookupTable]
        set index [EditorGetPaintLabel]
        return [$lut GetTableValue $index]
      }
    }
  }
  return "0 0 0 0"
}

proc EditorGetPaintThreshold {} {
  set node [EditorGetParameterNode]
  return [list \
    [$node GetParameter "Labeler,paintThresholdMin"] \
    [$node GetParameter "Labeler,paintThresholdMax"]]
}

proc EditorSetPaintThreshold {min max} {
  set node [EditorGetParameterNode]
  $node SetParameter "Labeler,paintThresholdMin" $min
  $node SetParameter "Labeler,paintThresholdMax" $max
}

proc EditorGetPaintThresholdState {} {
  set node [EditorGetParameterNode]
  return [$node GetParameter "Labeler,paintThreshold"]
}

proc EditorSetPaintThresholdState {onOff} {
  set node [EditorGetParameterNode]
  $node SetParameter "Labeler,paintThreshold" $onOff
}

proc EditorGetOptionsFrame {this} {
  return $::Editor($this,optionsFrame)
}

proc EditorSelectModule {} {
  set toolbar [$::slicer3::ApplicationGUI GetApplicationToolbar]
  [$toolbar GetModuleChooseGUI] SelectModule "Editor"
}


#
# MRML Event processing
#

proc EditorUpdateMRML {this} {
}

proc EditorProcessMRMLEvents {this callerID event} {

    if { [$this GetDebug] } {
        puts "EditorProcessMRMLEvents: event = $event, callerID = $callerID"
    }
    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
    set selectionNode  [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
    # get the active label volume
    set labelVolume [[[$this GetLogic] GetMRMLScene] GetNodeByID [$selectionNode GetActiveLabelVolumeID]]
    if { $labelVolume == "" } {
        if { [$this GetDebug] } { puts "No labelvolume..." }
        return
    }
    # check it's display node colour node
    set displayNode [$labelVolume GetDisplayNode] 

    if { $caller == $selectionNode && $event == 31 } {
        if { [$this GetDebug] } {
            puts "...caller is selection node, with modified event"
        }
        if {[info exists ::Editor($this,observedNodeID)] && $::Editor($this,observedNodeID) != "" &&
        $::Editor($this,observedNodeID) != [$selectionNode GetActiveLabelVolumeID]} {
            # remove the observer on the old volume's display node
            if { [$this GetDebug] } {
                puts "Removing mrml observer on last node: $::Editor($this,observedNodeID) and $::Editor($this,observedEvent)"
            }
            $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetMRMLScene] GetNodeByID $::Editor($this,observedNodeID)] $::Editor($this,observedEvent)
        }
        

        # is it the one we're showing?
        if { $displayNode != "" } {
            if {0} {
                # deprecated, Editor(x,colorsColor) no longer used
                if { [$displayNode GetColorNodeID] != [[$::Editor($this,colorsColor) GetColorNode] GetID] } {
                    if { [$this GetDebug] } {
                        puts "Resetting the color node"
                    }
                    $::Editor($this,colorsColor) SetColorNode [$displayNode GetColorNode]
                }
            }
            # add an observer on the volume node for display modified events
            if { [$this GetDebug] } {
                puts "Adding display node observer on label volume [$labelVolume GetID]"
            }
            $this AddMRMLObserverByNumber $labelVolume 18000
            set ::Editor($this,observedNodeID) [$labelVolume GetID]
            set ::Editor($this,observedEvent) 18000
        } else {
            if { [$this GetDebug] } {
                puts "Not resetting the color node, resetting the observed node id,  not adding display node observers, display node is null"
            }
            set ::Editor($this,observedNodeID) ""
            set ::Editor($this,observedEvent) -1
        }
        return
    } 

    if { $caller == $labelVolume && $event == 18000 } {
        if { [$this GetDebug] } {
            puts "... caller is label volume, got a display modified event, display node = $displayNode"
        }
        if {0} {
            # deprecated, not using colorsColor
            if { $displayNode != "" && [$displayNode GetColorNodeID] != [[$::Editor($this,colorsColor) GetColorNode] GetID] } {
                if { [$this GetDebug] } {
                    puts "...resetting the color node"
                }
                $::Editor($this,colorsColor) SetColorNode [$displayNode GetColorNode]
            }
        }
        return
    }
}

proc EditorEnter {this} {
  if {[$this GetDebug]} {
    puts "EditorEnter: Adding mrml observer on selection node, modified event"
  }
  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode] 31
}

proc EditorExit {this} {
  if {[$this GetDebug]} {
    puts "EditorExit: Removing mrml observer on selection node modified event"
  }
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode] 31
}

# TODO: there might be a better place to put this for general use...  
proc EditorCreateLabelVolume {this} {

  set volumeNode [$::Editor($this,volumesSelect) GetSelected]
  if { $volumeNode == "" } {
    return;
  }

  set name [[$::Editor($this,volumeName) GetWidget] GetValue]
  if { $name == "" } {
    set name "[$volumeNode GetName]-label"
  }

  set scene [[$this GetLogic] GetMRMLScene]

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  $labelNode Delete

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval $::Editor($this,paintRange) SetWholeRange $range
  eval $::Editor($this,paintRange) SetRange $range
}
