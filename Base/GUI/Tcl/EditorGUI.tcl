#
# Editor GUI Procs
#

proc EditorConstructor {this} {
}

proc EditorDestructor {this} {
}

proc EditorTearDownGUI {this} {

  # nodeSelector  ;# disabled for now
  set widgets {
      optionsSpacer optionsFrame
      toolsActiveTool toolsEditFrame toolsColorFrame
      enableCheckPoint
      toolsFrame volumesFrame 
  }

  itcl::delete object $::Editor($this,editColor)
  itcl::delete object $::Editor($this,editBox)
  itcl::delete object $::Editor($this,editHelper)

  # kill any remaining boxes (popup)
  set boxes [itcl::find objects -isa Box]
  foreach b $boxes {
    itcl::delete object $b
  }

  foreach w $widgets {
    $::Editor($this,$w) SetParent ""
    $::Editor($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "Editor"]
    [$this GetUIPanel] RemovePage "Editor"
  }

  unset ::Editor(singleton)

  EditorFreeCheckPointVolumes

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
  set helptext "The Editor allows label maps to be created and edited. The active label map will be modified by the Editor.  See <a>http://www.slicer.org/slicerWiki/index.php/Modules:Editor-Documentation-3.6</a>.\n\nThe Master Volume refers to the background grayscale volume to be edited (used, for example, when thresholding).  The Merge Volume refers to a volume that contains several different label values corresponding to different structures.\n\nBasic usage: selecting the Master and Merge Volume give access to the editor tools.  Each tool has a help icon to bring up a dialog with additional information.  Hover your mouse pointer over buttons and options to view Balloon Help (tool tips).  Use these to define the Label Map.\n\nAdvanced usage: open the Per-Structure Volumes tab to create independent Label Maps for each color you wish to edit.  Since many editor tools (such as threshold) will operate on the entire volume, you can use the Per-Structure Volumes feature to isolate these operations on a structure-by-structure basis.  Use the Split Merge Volume button to create a set of volumes with independent labels.  Use the Add Structure button to add a new volume.  Delete Structures will remove all the independent structure volumes.  Merge All will assemble the current structures into the Merge Volume.  Merge And Build will invoke the Model Maker module on the Merge Volume."

  set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # Editor Volumes
  #
  set ::Editor($this,volumesFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::Editor($this,volumesFrame) SetParent $pageWidget
  $::Editor($this,volumesFrame) Create
  $::Editor($this,volumesFrame) SetLabelText "Create & Select Label Maps"
  pack [$::Editor($this,volumesFrame) GetWidgetName] \
    -side top -anchor nw -fill x -expand false -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  # create the helper box - note this isn't a  kwwidget
  #  but a helper class that creates kwwidgets in the given frame
  set ::Editor($this,editHelper) [::HelperBox #auto]
  $::Editor($this,editHelper) configure -frame [$::Editor($this,volumesFrame) GetFrame]
  $::Editor($this,editHelper) create


  #
  # Tool Frame
  #
  set ::Editor($this,toolsFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::Editor($this,toolsFrame) SetParent $pageWidget
  $::Editor($this,toolsFrame) Create
  $::Editor($this,toolsFrame) SetLabelText "Edit Selected Label Map"
  pack [$::Editor($this,toolsFrame) GetWidgetName] \
    -side top -anchor nw -fill x -expand true -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

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
    -side left -anchor nw -fill both -expand true -padx 2 -pady 2 

  # one pixel label to keep the option panel expanded
    # TODO: doesn't work as intended
  set ::Editor($this,optionsSpacer) [vtkKWLabel New]
  $::Editor($this,optionsSpacer) SetParent $::Editor($this,optionsFrame)
  $::Editor($this,optionsSpacer) Create
  pack [$::Editor($this,optionsSpacer) GetWidgetName] \
    -fill both -expand true 

  # Enable check point check button
  set ::Editor($this,enableCheckPoint) [vtkKWCheckButton New]
  $::Editor($this,enableCheckPoint) SetParent $::Editor($this,toolsEditFrame)
  $::Editor($this,enableCheckPoint) Create
  $::Editor($this,enableCheckPoint) SetText "Undo/Redo"
  $::Editor($this,enableCheckPoint) SetBalloonHelpString "Volume Check Points allow you to undo and redo recent edits.\n\nNote: for large volumes, you may run out of system memory when this is enabled."
  set ::Editor(checkPointsEnabled) 1
  $::Editor($this,enableCheckPoint) SetSelectedState $::Editor(checkPointsEnabled)
  pack [$::Editor($this,enableCheckPoint) GetWidgetName] -side left

}

proc EditorAddGUIObservers {this} {
    $this AddObserverByNumber $::Editor($this,enableCheckPoint) 10000 

    if {[$this GetDebug]} {
        puts "Adding mrml observer to selection node, modified event"
    }
    $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
      [$this GetNumberForVTKEvent ModifiedEvent]
}

proc EditorRemoveGUIObservers {this} {
  $this RemoveObserverByNumber $::Editor($this,enableCheckPoint) 10000 
  
  if {[$this GetDebug]} {
    puts "Removing mrml observer on selection node, modified event"
  }
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
}

proc EditorRemoveLogicObservers {this} {
}

proc EditorRemoveMRMLNodeObservers {this} {
}

proc EditorProcessLogicEvents {this caller event} {
}

proc EditorProcessGUIEvents {this caller event} {
  
  if { $caller == $::Editor($this,enableCheckPoint) } {
    switch $event {
      "10000" {
        set onoff [$::Editor($this,enableCheckPoint) GetSelectedState]
        EditorSetCheckPointEnabled $onoff
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
  if { [info exists ::Editor($::Editor(singleton),toolsActiveTool)] } {
    [$::Editor($::Editor(singleton),toolsActiveTool) GetWidget] SetText $name
    [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText $name
  }
  set node [EditorGetParameterNode]
  $node SetParameter "tool" $name
}

proc EditorGetActiveToolLabel {} {
  set node [EditorGetParameterNode]
  if { [$node GetParameter "tool"] == "" } {
    $node SetParameter "tool" "Default"
  }
  return [$node GetParameter "tool"]
}


proc EditorGetPaintLabel {} {
  set node [EditorGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [expr int([$node GetParameter "label"])]
}

proc EditorSetPaintLabel {index} {
  set node [EditorGetParameterNode]
  set index [expr int($index)]
  $node SetParameter "label" $index
}

proc EditorToggleErasePaintLabel {} {
  # if in erase mode (label is 0), set to stored color
  # if in color, store current and set to 0
  if { [EditorGetPaintLabel] == 0 } {
    if { [info exists ::Editor(savedLabelValue)] } {
      EditorSetPaintLabel $::Editor(savedLabelValue)
    }
  } else {
    set ::Editor(savedLabelValue) [EditorGetPaintLabel]
    EditorSetPaintLabel 0
  }
}


proc EditorGetPaintColor { {this ""} } {

  if { [info exists ::slicer3::ApplicationLogic] } {
    set sliceLogic [$::slicer3::ApplicationLogic GetSliceLogicByLayoutLabel "Red"]
    if { $sliceLogic != "" } {
      set logic [$sliceLogic GetLabelLayer]
      if { $logic != "" } {
        set volumeDisplayNode [$logic GetVolumeDisplayNode]
        if { $volumeDisplayNode != "" } {
          set node [$volumeDisplayNode GetColorNode]
          set lut [$node GetLookupTable]
          set index [EditorGetPaintLabel]
          set index [expr int($index)]
          return [$lut GetTableValue $index]
        }
      }
    }
  }
  return "0 0 0 0"
}

proc EditorGetPaintName { {this ""} } {

  set sliceLogic [$::slicer3::ApplicationLogic GetSliceLogic "Red"]
  if { $sliceLogic != "" } {
    set logic [$sliceLogic GetLabelLayer]
    if { $logic != "" } {
      set volumeDisplayNode [$logic GetVolumeDisplayNode]
      if { $volumeDisplayNode != "" } {
        set node [$volumeDisplayNode GetColorNode]
        set index [EditorGetPaintLabel]
        set index [expr int($index)]
        return [$node GetColorName $index]
      }
    }
  }
  return "Unknown"
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

    if { $caller == $selectionNode && $event == [$this GetNumberForVTKEvent ModifiedEvent] } {
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
        return
    }
}

proc EditorEnter {this} {
  if {[$this GetDebug]} {
      puts "EditorEnter: Adding mrml observer on selection node, modified event"
  }

  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]

  
  set sliceLogic [$::slicer3::ApplicationLogic GetSliceLogic "Red"]
  set layerLogic [$sliceLogic GetBackgroundLayer]
  set masterNode [$layerLogic GetVolumeNode]
  set layerLogic [$sliceLogic GetLabelLayer]
  set mergeNode [$layerLogic GetVolumeNode]
  if { $masterNode != "" } {
    if { $mergeNode != "" } {
      $::Editor($this,editHelper) setVolumes $masterNode $mergeNode
    } else {
      if { [$masterNode GetClassName] == "vtkMRMLScalarVolumeNode" } {
        $::Editor($this,editHelper) setMasterVolume $masterNode
      }
    }
  }
}

proc EditorExit {this} {

  EffectSWidget::RemoveAll
  if {[$this GetDebug]} {
    puts "EditorExit: Removing mrml observer on selection node modified event"
  }
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode] \
    [$this GetNumberForVTKEvent ModifiedEvent]
  
  # delete the current effect - users were getting confused that the editor was still
  # active when the module wasn't visible
  after idle ::EffectSWidget::RemoveAll
}

proc EditorShowHideTools {showhide} {
  set this $::Editor(singleton)
  if { ![info exists ::Editor($this,toolsFrame)] } {
    return
  } 
  if { $showhide == "show" } {
    $::Editor($this,editColor) updateGUI [EditorGetPaintLabel]
    $::Editor($this,toolsFrame) ExpandFrame
  } else {
    $::Editor($this,toolsFrame) CollapseFrame
  }
}

# TODO: there might be a better place to put this for general use...  
proc EditorCreateLabelVolume {this} {

  if { $volumeNode == "" } {
    EditorErrorDialog "Select Scalar Source Volume for Label Map"
    return;
  }

  set name "[$volumeNode GetName]-label"

  set scene [[$this GetLogic] GetMRMLScene]
  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 0
  
  set id [ $labelNode GetID ]
  $labelNode Delete

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval ::Labler::SetPaintRange $range

  return $id
}

#
# store/restore a fixed number of volumes
# - for now, limit to 10 check point volumes total
# - keep two lists - check point and redo
# -- adding to nextCheckPoint invalidates redo list
# - each list entry is a pair of vtkImageData and mrml Node ID
#
proc EditorFreeVolumes {volumeList} {
  foreach volume $volumeList {
    array set checkPoint $volume
    foreach o "imageData stash" {
      if { [info command $checkPoint($o)] != "" } {
        $checkPoint($o) Delete
      }
    }
  }
}

# called by checkbox in GUI
proc EditorSetCheckPointEnabled {onoff} {
  if { !$onoff } {
    EditorFreeCheckPointVolumes
  }
  EditorUpdateCheckPointButtons
  set ::Editor(checkPointsEnabled) $onoff
}

# called at tear down time to free local instances
proc EditorFreeCheckPointVolumes {} {
  foreach list {previousCheckPointImages nextCheckPointImages} {
    if { [info exists ::Editor($list)] } {
      EditorFreeVolumes $::Editor($list)
      set ::Editor($list) ""
    }
  }
}

# enable or disable button state depending on existence of 
# volumes to restore
proc EditorUpdateCheckPointButtons {} {
  if { [info exists ::Editor(previousCheckPointImages)] } {
    if { $::Editor(previousCheckPointImages) != "" } {
      EditBox::SetButtonState PreviousCheckPoint ""
    } else {
      EditBox::SetButtonState PreviousCheckPoint Disabled
    }
  }
  if { [info exists ::Editor(nextCheckPointImages)] } {
    if { $::Editor(nextCheckPointImages) != "" } {
      EditBox::SetButtonState NextCheckPoint ""
    } else {
      EditBox::SetButtonState NextCheckPoint Disabled
    }
  }
  # issue a modified event on the parameter node so that any
  # code that is observing it will have a change to update the
  # enabled/disabled state of these buttons (slicer4)
  [EditorGetParameterNode] Modified
}

# called by editor effects
proc EditorStoreCheckPoint {node} {

  if { ![info exists ::Editor(checkPointsEnabled)] } {
    set ::Editor(checkPointsEnabled) 0
  }

  if { !$::Editor(checkPointsEnabled) } {
    return
  }

  EditorStoreCheckPointVolume [$node GetID] previousCheckPointImages

  # invalidate NextCheckPoint list
  EditorFreeVolumes $::Editor(nextCheckPointImages)

  EditorUpdateCheckPointButtons
}


# unsed internally to manage nodes
proc EditorStoreCheckPointVolume {nodeID listID} {
  
  # create lists if needed
  if { ![info exists ::Editor(previousCheckPointImages)] } {
    set ::Editor(previousCheckPointImages) ""
    set ::Editor(nextCheckPointImages) ""
    set ::Editor(numberOfCheckPoints) 10
  }

  # trim oldest previousCheckPoint image if needed
  if { [llength $::Editor($listID)] >= $::Editor(numberOfCheckPoints) } {
    array set disposeCheckPoint [lindex $::Editor($listID) 0]
    $disposeCheckPoint(imageData) Delete
    $disposeCheckPoint(stash) Delete
    set ::Editor($listID) [lrange $::Editor($listID) 1 end]
  }

  # add new 
  set checkPoint(nodeID) $nodeID
  set node [$::slicer3::MRMLScene GetNodeByID $nodeID]
  set checkPoint(imageData) [vtkImageData New]
  set checkPoint(stash) [vtkImageStash New]
  if { $node != "" } {
    $checkPoint(imageData) DeepCopy [$node GetImageData]
    $checkPoint(stash) SetStashImage $checkPoint(imageData)
    $checkPoint(stash) ThreadedStash
  } else {
    error "no node for $nodeID"
  }
  lappend ::Editor($listID) [array get checkPoint]
}

proc EditorRestoreData {restoreCheckPointArray} {
  # restore the volume data
  array set checkPoint $restoreCheckPointArray
  set node [$::slicer3::MRMLScene GetNodeByID $checkPoint(nodeID)]
  if { [info command $checkPoint(stash)] == "" } {
    # check point no longer exists
    return
  }
  if { $node != "" } {
    set tries 0
    while { $tries < 10 && [$checkPoint(stash) GetStashing] } {
      incr tries
      after 500
    }
    if { [$checkPoint(stash) GetStashing] } {
      EditorErrorDialog "Sorry - Cannot access stored checkpoint!"
      return
    }
    $checkPoint(stash) Unstash
    [$node GetImageData] DeepCopy $checkPoint(imageData)
    # now delete the stored checkpoint - it is the current image
    # label map and will get stashed again if draw on or restored over
    $checkPoint(stash) Delete
    $checkPoint(imageData) Delete
  } else {
    EditorErrorDialog "Sorry - no node for $checkPoint(nodeID)"
  }
  $node SetModifiedSinceRead 1
  $node Modified
}

# called by button presses or keystrokes
proc EditorPerformPreviousCheckPoint {} {
  if { ![info exists ::Editor(previousCheckPointImages)] || [llength $::Editor(previousCheckPointImages)] == 0 } {
    return
  }

  # get the volume to restore
  set restoreArray [lindex $::Editor(previousCheckPointImages) end]
  array set restore $restoreArray

  # save the current state as a redo point
  EditorStoreCheckPointVolume $restore(nodeID) nextCheckPointImages

  # now pop the next item on the previousCheckPoint stack
  set ::Editor(previousCheckPointImages) [lrange $::Editor(previousCheckPointImages) 0 end-1]

  EditorRestoreData $restoreArray
  EditorUpdateCheckPointButtons 
  EditorForceSliceRender $restore(nodeID)
}

# called by button presses or keystrokes
proc EditorPerformNextCheckPoint {} {
  if { ![info exists ::Editor(nextCheckPointImages)] || [llength $::Editor(nextCheckPointImages)] == 0 } {
    return
  }

  # get the volume to restore
  set restoreArray [lindex $::Editor(nextCheckPointImages) end]
  array set restore $restoreArray

  # save the current state as an previousCheckPoint Point
  EditorStoreCheckPointVolume $restore(nodeID) previousCheckPointImages

  # now pop the next item on the redo stack
  set ::Editor(nextCheckPointImages) [lrange $::Editor(nextCheckPointImages) 0 end-1]
  EditorRestoreData $restoreArray
  EditorUpdateCheckPointButtons 
  EditorForceSliceRender $restore(nodeID)
}


proc EditorForceSliceRender {nodeID} {
  # TODO: workaround for new pipeline in slicer4
  # - editing image data of the calling modified on the node
  #   does not pull the pipeline chain
  # - so we trick it by changing the image data first
  set node [$::slicer3::MRMLScene GetNodeByID $nodeID]
  $node SetModifiedSinceRead 1
  set workaround 1
  if { $workaround } {
    set tempImageData [vtkImageData New]
    set imageData [$node GetImageData]
    $node SetAndObserveImageData $tempImageData
    $node SetAndObserveImageData $imageData
    $tempImageData Delete
  } else {
    $node Modified
  }
}

#
# helper to display error
#
proc EditorErrorDialog {errorText} {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

#
# helper to display confirmation
#
proc EditorConfirmDialog {confirmText} {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToOkCancel
  $dialog SetText $confirmText
  $dialog Create
  set returnValue [$dialog Invoke]
  $dialog Delete
  return $returnValue
}

