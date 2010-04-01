package require Itcl

#########################################################
#
if {0} { ;# comment

  HelperBox is a wrapper around a set of kwwidgets and other
  structures to manage the slicer3 segmentation helper box.  

# TODO : 

  for testing, create from tcl console:

catch "itcl::delete class HelperBox"; source ../Slicer3/Modules/Editor/HelperBox.tcl; ::HelperBox::ShowDialog; array set o [::Box::helperBox0 objects]
}
#
#########################################################

#
# namespace procs
#

namespace eval HelperBox {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog {} {
    ::Box::ShowDialog HelperBox
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class HelperBox] == "" } {

  itcl::class HelperBox {

    inherit Box

    destructor {}

    variable col ;# array of column indices (for easy access)

    variable _master ;# master volume node 
    variable _observations ;# master volume node 

    # methods
    method create {} {} ;# create the widget

    method createMerge {} {} ;# make a merge volume for the current selection
    method select {} {} ;# select the master volume node
    method mergeVolume {} {} ;# helper to get the merge volume for the current selected master
    method structureVolume { structureName } {} ;# get the volume for a given structure name

    method promptStructure {} {} ;# ask which label value to use
    method addStructure { {label ""} } {} ;# add a new dependent volume
    method deleteStructures {} {} ;# delete all the structures
    method updateStructures {} {} ;# refresh the structure frame with currently loaded data
    method merge { {label all} } {} ;# merge the named or all structure labels into the master label
    method edit { {label ""} } {} ;# select structure 'label' for editing
    method roi {label} {} ;# create a subvolume ROI for 'label'
    method build { {label all} } {} ;# build a model for one or all labels
    method colorSelectDialog {} {} ;# pop up a dialog with color table selection

    method processEvent { caller event } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# - rely on superclass
# ------------------------------------------------------------------

itcl::body HelperBox::destructor {} {

  set broker $::slicer3::Broker
  foreach ob $_observations {
    $broker RemoveObservation $ob
  }
}

# create a merge volume for the current master
itcl::body HelperBox::createMerge { } {
  if { $_master == "" } {
    # should never happen
    EditorErrorDialog "Cannot create merge volume without master"
    return
  }

  set masterName [$_master GetName]
  set mergeName $masterName-label
  set merge [$this mergeVolume]
  if { $merge == "" } {
    set volumesLogic [$::slicer3::VolumesGUI GetLogic]
    set merge [$volumesLogic CreateLabelVolume $::slicer3::MRMLScene $_master $mergeName]
    [$merge GetDisplayNode] SetAndObserveColorNodeID [[$o(colorSelector) GetSelected] GetID]
  }
  after idle $this select
}

# select master volume - load merge volume if one with the correct name exists
itcl::body HelperBox::select { } {

  set _master [$o(masterSelector) GetSelected]
  set merge [$this mergeVolume]
  set mergeText "None"
  if { $merge != "" } {
    if { [$merge GetClassName] != "vtkMRMLScalarVolumeNode" || ![$merge GetLabelMap] } {
      EditorErrorDialog "Error: merge label volume $mergeName is not a label volume"
    } else {
      # make the source node the active background, and the label node the active label
      set selectionNode [$::slicer3::ApplicationLogic  GetSelectionNode]
      $selectionNode SetReferenceActiveVolumeID [$_master GetID]
      $selectionNode SetReferenceSecondaryVolumeID [$merge GetID]
      $::slicer3::ApplicationLogic  PropagateVolumeSelection 0
      set mergeText [$merge GetName]
    }
  } else {
    # bring up dialog if picked a new master
    if { $_master != "" } {
      $this colorSelectDialog
    }
  }
  [$o(mergeName) GetWidget] SetText $mergeText
  $this updateStructures
}

# select merge volume 
itcl::body HelperBox::mergeVolume {} {
  if { $_master == "" } {
    return ""
  }
  set masterName [$_master GetName]
  set mergeName $masterName-label
  set mergeCandidates [$::slicer3::MRMLScene GetNodesByName $mergeName]
  
  if { [$mergeCandidates GetNumberOfItems] > 1 } {
    EditorErrorDialog "Warning: more than one merge label volume for master $masterName"
  }

  return [$mergeCandidates GetItemAsObject 0]
}

# select structure volume 
itcl::body HelperBox::structureVolume { structureName } {
  if { $_master == "" } {
    return ""
  }
  set masterName [$_master GetName]
  set structureVolumeName $masterName-$structureName-label
  set structureVolumeCandidates [$::slicer3::MRMLScene GetNodesByName $structureVolumeName]
  
  if { [$structureVolumeCandidates GetNumberOfItems] > 1 } {
    EditorErrorDialog "Warning: more than one structure label volume for master $masterName structure $structureName"
  }

  return [$structureVolumeCandidates GetItemAsObject 0]
}

# ask user which label to create
itcl::body HelperBox::promptStructure {} {

  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 
  set colorNode [[$merge GetDisplayNode] GetColorNode]

  if { $colorNode == "" } {
    EditorErrorDialog "No color node selected"
    return
  }

  set _colorBox [ColorBox #auto]
  $_colorBox configure -colorNode $colorNode
  $_colorBox configure -selectCommand "$this addStructure %d; after idle itcl::delete object [namespace current]::$_colorBox"
  $_colorBox create
}


# create the segmentation helper box
itcl::body HelperBox::addStructure { {label ""} } {

  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 

  if { $label == "" } {
    set label [$this promptStructure]
    return
  }

  set colorNode [[$merge GetDisplayNode] GetColorNode]
  set labelName [$colorNode GetColorName $label]
  set structureName [$_master GetName]-$labelName-label

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set struct [$volumesLogic CreateLabelVolume $::slicer3::MRMLScene $_master $structureName]
  [$struct GetDisplayNode] SetAndObserveColorNodeID [$colorNode GetID]

  $this updateStructures
  $this edit $label
}

# delete all the structures
itcl::body HelperBox::deleteStructures {} {

  #
  # iterate through structures and delete them
  #
  set w [$o(structures) GetWidget]
  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 

  set rows [$w GetNumberOfRows]

  if { ![EditorConfirmDialog "Delete $rows structure volume(s)?"] } {
    return
  }

  $slicer3::MRMLScene SaveStateForUndo

  for {set row 0} {$row < $rows} {incr row} {
    set structureName [$w GetCellText $row $col(Name)]
    set structureVolume [$this structureVolume $structureName]
    if { $row == [expr $rows -1] } {
      $slicer3::MRMLScene RemoveNode $structureVolume
    } else {
      $slicer3::MRMLScene RemoveNodeNoNotify $structureVolume
    }
  }
  $this updateStructures
}

# merge the named or all structure labels into the master label
itcl::body HelperBox::merge { {label all} } {

  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 

  set w [$o(structures) GetWidget]

  # check that structures are all the same size as the merge volume
  set dims [[$merge GetImageData] GetDimensions]
  set rows [$w GetNumberOfRows]
  for {set row 0} {$row < $rows} {incr row} {
    set structureName [$w GetCellText $row $col(Name)]
    set structureVolume [$this structureVolume $structureName]
    if { [[$structureVolume GetImageData] GetDimensions] != $dims } {
      set mergeName [$merge GetName]
      EditorErrorDialog "Merge Aborted: Volume $structureVolume does not have the same dimensions as the target merge volume.  Use the Resample Scalar/Vector/DWI module to resample.  Use $mergeName as the Reference Volume and select Nearest Neighbor (nn) Interpolation Type."
      return
    }
  }

  # check that user really wants to merge
  set rows [$w GetNumberOfRows]
  for {set row 0} {$row < $rows} {incr row} {
    set structureName [$w GetCellText $row $col(Name)]
    set structureVolume [$this structureVolume $structureName]
    if { [[$structureVolume GetImageData] GetMTime] < [[$merge GetImageData] GetMTime] } {
      set mergeName [$merge GetName]
      EditorErrorDialog "Note: Merge volume has been modified more recently than structure volumes.\nCreating backup copy as $mergeName-backup"
      set volumesLogic [$::slicer3::VolumesGUI GetLogic]
      $volumesLogic CloneVolume $::slicer3::MRMLScene $merge $mergeName-backup
      break
    }
  }


  #
  # find the Image Label Combine
  # - call Enter to be sure GUI has been built
  #
  set combiner [vtkImageLabelCombine New]
  #

  #
  # iterate through structures merging into merge volume
  #
  for {set row 0} {$row < $rows} {incr row} {
    set structureName [$w GetCellText $row $col(Name)]
    set structureVolume [$this structureVolume $structureName]

    if { $row == 0 } {
      # first row, just copy into merge volume
      [$merge GetImageData] DeepCopy [$structureVolume GetImageData]
      continue
    }

    $combiner SetInput1 [$merge GetImageData]
    $combiner SetInput2 [$structureVolume GetImageData]
    $this statusText "Merging $structureName..."
    $combiner Update
    [$merge GetImageData] DeepCopy [$combiner GetOutput]

  }

  # mark all volumes as modified so we will be able to tell if the 
  # merged volume gets edited after these
  set rows [$w GetNumberOfRows]
  for {set row 0} {$row < $rows} {incr row} {
    set structureName [$w GetCellText $row $col(Name)]
    set structureVolume [$this structureVolume $structureName]
    [$structureVolume GetImageData] Modified
  }

  #
  # clean up our references
  #
  $combiner Delete

  set selectionNode [$::slicer3::ApplicationLogic  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$_master GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$merge GetID]
  $::slicer3::ApplicationLogic  PropagateVolumeSelection 0

  $this statusText "Finished merging."

}

itcl::body HelperBox::build { {label all} } {

  #
  # get the image data for the label layer
  #
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  set layerLogic [$sliceLogic GetLabelLayer]
  set volumeNode [$layerLogic GetVolumeNode]
  if { $volumeNode == "" } {
    errorDialog "Cannot make model - no volume node for $layerLogic in $sliceLogic."
    return
  }

  #
  # find the Model Maker
  # - call Enter to be sure GUI has been built
  #
  set modelMaker ""
  foreach gui [vtkCommandLineModuleGUI ListInstances] {
    if { [$gui GetGUIName] == "Model Maker" } {
      set modelMaker $gui
    }
  }

  if { $modelMaker == "" } {
    errorDialog "Cannot make model: no Model Maker Module found."
  }

  $modelMaker Enter

  #
  # set up the model maker node
  #
  set moduleNode [vtkMRMLCommandLineModuleNode New]
  $::slicer3::MRMLScene AddNode $moduleNode
  $moduleNode SetName "Editor Make Model"
  $moduleNode SetModuleDescription "Model Maker"

  set name [[$o(name) GetWidget] GetValue]
  if { $name == "" } {
    $moduleNode SetParameterAsString "Name" "Quick Model"
  } else {
    $moduleNode SetParameterAsString "Name" $name
  }
  $moduleNode SetParameterAsString "FilterType" "Sinc"
  $moduleNode SetParameterAsBool "GenerateAll" "0"
  $moduleNode SetParameterAsString "Labels" [EditorGetPaintLabel]
  $moduleNode SetParameterAsBool "JointSmooth" 1
  $moduleNode SetParameterAsBool "SplitNormals" 1
  $moduleNode SetParameterAsBool "PointNormals" 1
  $moduleNode SetParameterAsBool "SkipUnNamed" 1
  $moduleNode SetParameterAsInt "Start" -1
  $moduleNode SetParameterAsInt "End" -1
  if { [[$o(smooth) GetWidget] GetSelectedState] } {
    $moduleNode SetParameterAsDouble "Decimate" 0.25
    $moduleNode SetParameterAsDouble "Smooth" 10
  } else {
    $moduleNode SetParameterAsDouble "Decimate" 0
    $moduleNode SetParameterAsDouble "Smooth" 0
  }

  $moduleNode SetParameterAsString "InputVolume" [$volumeNode GetID]

  #
  # output 
  # - make a new hierarchy node if needed
  #
  set numNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLModelHierarchyNode"]
  set outHierarchy ""
  for {set n 0} {$n < $numNodes} {incr n} {
    set node [$::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLModelHierarchyNode"]
    if { [$node GetName] == "Editor Models" } {
      set outHierarchy $node
    }
  }

  if { $outHierarchy == "" } {
    set outHierarchy [vtkMRMLModelHierarchyNode New]
    $outHierarchy SetScene $::slicer3::MRMLScene
    $outHierarchy SetName "Editor Models"
    $::slicer3::MRMLScene AddNode $outHierarchy
  }

  $moduleNode SetParameterAsString "ModelSceneFile" [$outHierarchy GetID]


  # 
  # run the task (in the background)
  # - use the GUI to provide progress feedback
  # - use the GUI's Logic to invoke the task
  # - model will show up when the processing is finished
  #
  $modelMaker SetCommandLineModuleNode $moduleNode
  [$modelMaker GetLogic] SetCommandLineModuleNode $moduleNode
  $modelMaker SetCommandLineModuleNode $moduleNode
  [$modelMaker GetLogic] Apply $moduleNode

  $this statusText "Model Making Started..."

  #
  # clean up our references
  #
  $moduleNode Delete
  $outHierarchy Delete
  $modelMaker Enter

}

# select the picked label for editing
itcl::body HelperBox::edit { {label ""} } {

  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 
  set colorNode [[$merge GetDisplayNode] GetColorNode]

  if { $label == "" } {
    set row [[$o(structures) GetWidget] GetIndexOfFirstSelectedRow]
    set label [[$o(structures) GetWidget] GetCellText $row $col(Number)]
  }
  if { $label == "" } {
    # still no label means listbox is not correctly configured, must quit
    return
  }

  set structureName [$colorNode GetColorName $label]
  set structureVolume [$this structureVolume $structureName]

  # make the master node the active background, and the structure label node the active label
  set selectionNode [$::slicer3::ApplicationLogic  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$_master GetID]
  $selectionNode SetReferenceSecondaryVolumeID [$merge GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$structureVolume GetID]
  $::slicer3::ApplicationLogic  PropagateVolumeSelection 0

  EditorSetPaintLabel $label

}

# re-build the Structures frame
itcl::body HelperBox::updateStructures {} {

  $o(masterSelector) UpdateMenu

  set w [$o(structures) GetWidget]
  $w DeleteAllRows

  # no merge volume exists, disable everything
  set merge [$this mergeVolume]
  if { $merge == "" } {
    $o(addStructureButton) SetStateToDisabled
    $o(deleteStructuresButton) SetStateToDisabled
    $o(mergeButton) SetStateToDisabled
    $o(mergeAndBuildbutton) SetStateToDisabled
    return
  }

  # we have a merge volume, proceed to fill out the widget in normal state
  $o(addStructureButton) SetStateToNormal
  $o(deleteStructuresButton) SetStateToNormal
  $o(mergeButton) SetStateToNormal
  $o(mergeAndBuildbutton) SetStateToNormal

  set colorNode [[$merge GetDisplayNode] GetColorNode]
  set lut [$colorNode GetLookupTable]

  set masterName [$_master GetName]
  $::slicer3::MRMLScene InitTraversal
  set vNode [$::slicer3::MRMLScene GetNextNodeByClass "vtkMRMLScalarVolumeNode"]
  while { $vNode != "" } {
    set vName [$vNode GetName]
    if { [string match "$masterName-*-label" $vName] } {
      # figure out what name it is
      set start [expr 1+[string length $masterName]]
      set endOff [string length "-label"]
      set structureName [string range $vName $start end-$endOff]
      set structureIndex [$colorNode GetColorIndexByName $structureName]
      set structureColor [lrange [$lut GetTableValue $structureIndex] 0 2]
      
      # add it to the listbox
      set row [$w GetNumberOfRows]
      $w InsertCellText $row $col(Number) "$structureIndex"
      eval $w SetCellBackgroundColor $row $col(Color) $structureColor
      $w InsertCellText $row $col(Name) $structureName
      $w InsertCellText $row $col(LabelName) $vName
    }
    set vNode [$::slicer3::MRMLScene GetNextNodeByClass "vtkMRMLScalarVolumeNode"]
  }
}

# create the segmentation helper box
itcl::body HelperBox::create { } {

  if { $frame != "" } {
    set parent $frame
  } else {

    #
    # make the toplevel 
    #
    set o(toplevel) [vtkNew vtkKWTopLevel]
    $o(toplevel) SetApplication $::slicer3::Application
    $o(toplevel) SetTitle "Segmentation Helper Box"
    $o(toplevel) Create

    set parent $o(toplevel)

  }


  #
  # Master Frame
  # 
  #set o(masterFrame) [vtkNew vtkKWFrameWithLabel]
  set o(masterFrame) [vtkNew vtkKWFrame]
  $o(masterFrame) SetParent $parent
  $o(masterFrame) Create
  #$o(masterFrame) SetLabelText "Master Volume Properties"
  pack [$o(masterFrame) GetWidgetName] -side top -anchor nw -fill x -padx 2 -pady 2
  #set masterFrame [$o(masterFrame) GetFrame]
  set masterFrame $o(masterFrame)

  # the master volume

  set o(masterSelector) [vtkNew vtkSlicerNodeSelectorWidget]
  $o(masterSelector) SetParent $masterFrame
  $o(masterSelector) Create
  $o(masterSelector) SetNodeClass "vtkMRMLScalarVolumeNode" "LabelMap" "0" ""
  $o(masterSelector) NewNodeEnabledOff
  $o(masterSelector) NoneEnabledOn
  $o(masterSelector) DefaultEnabledOff
  $o(masterSelector) ShowHiddenOff
  $o(masterSelector) ChildClassesEnabledOff
  $o(masterSelector) SetMRMLScene $::slicer3::MRMLScene
  $o(masterSelector) UpdateMenu
  $o(masterSelector) SetLabelText "Master Volume:"
  $o(masterSelector) SetBalloonHelpString "Pick the master structural volume to define the segmentation"
  pack [$o(masterSelector) GetWidgetName] -side top -fill x

  # master label name

  set o(mergeName) [vtkNew vtkKWLabelWithLabel]
  $o(mergeName) SetParent $masterFrame
  $o(mergeName) Create
  $o(mergeName) SetLabelText "Merge Volume:"
  $o(mergeName) SetBalloonHelpString "Composite label map containing the merged structures (be aware that merge operations will overwrite any edits applied to this volume)"
  [$o(mergeName) GetWidget] SetText "None"
  pack [$o(mergeName) GetWidgetName] -side top -fill x

  #
  # Structures Frame
  # 
  #set o(structuresFrame) [vtkNew vtkKWFrameWithLabel]
  set o(structuresFrame) [vtkNew vtkKWFrame]
  $o(structuresFrame) SetParent $parent
  $o(structuresFrame) Create
  #$o(structuresFrame) SetLabelText "Structures"
  pack [$o(structuresFrame) GetWidgetName] -side top -anchor nw -fill both -expand true -padx 2 -pady 2
  #set structuresFrame [$o(structuresFrame) GetFrame]
  set structuresFrame $o(structuresFrame)

  # buttons frame 

  set o(structureButtonsFrame) [vtkNew vtkKWFrame]
  $o(structureButtonsFrame) SetParent $structuresFrame
  $o(structureButtonsFrame) Create
  pack [$o(structureButtonsFrame) GetWidgetName] -side top -anchor sw -fill x -padx 2 -pady 2

  # add button

  set o(addStructureButton) [vtkNew vtkKWPushButton]
  $o(addStructureButton) SetParent $o(structureButtonsFrame)
  $o(addStructureButton) Create
  $o(addStructureButton) SetText "Add Structure Volume"
  $o(addStructureButton) SetBalloonHelpString "Add a label volume for a structure to edit"
  pack [$o(addStructureButton) GetWidgetName] -side left

  # delete structures button

  set o(deleteStructuresButton) [vtkNew vtkKWPushButton]
  $o(deleteStructuresButton) SetParent $o(structureButtonsFrame)
  $o(deleteStructuresButton) Create
  $o(deleteStructuresButton) SetText "Delete Structure Volumes"
  $o(deleteStructuresButton) SetBalloonHelpString "Delete all the structure volumes from the scene"
  pack [$o(deleteStructuresButton) GetWidgetName] -side left

  # structures listbox

  set o(structures) [vtkNew vtkKWMultiColumnListWithScrollbars]
  $o(structures) SetParent $structuresFrame
  $o(structures) Create
  $o(structures) SetHeight 2
  $o(structures) SetHorizontalScrollbarVisibility 1
  set w [$o(structures) GetWidget]
  $w SetSelectionTypeToCell
  $w MovableRowsOff
  $w MovableColumnsOn
  $w SetPotentialCellColorsChangedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
  $w SetColumnSortedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

  foreach column {Number Color Name LabelName} text {Number Color Name "Label Volume"} width {7 6 15 30} {
    set col($column) [$w AddColumn $column]
    $w ColumnEditableOff $col($column)
    $w SetColumnWidth $col($column) $width
  }
  pack [$o(structures) GetWidgetName] -side top -anchor ne -padx 2 -pady 2 -expand true -fill both

  # all buttons frame 

  set o(allButtonsFrame) [vtkNew vtkKWFrame]
  $o(allButtonsFrame) SetParent $structuresFrame
  $o(allButtonsFrame) Create
  pack [$o(allButtonsFrame) GetWidgetName] -side bottom -anchor sw -fill x -padx 2 -pady 2

  # merge button

  set o(mergeButton) [vtkNew vtkKWPushButton]
  $o(mergeButton) SetParent $o(allButtonsFrame)
  $o(mergeButton) Create
  $o(mergeButton) SetText "Merge All"
  $o(mergeButton) SetBalloonHelpString "Merge all structures into Merge Volume"
  pack [$o(mergeButton) GetWidgetName] -side left

  # merge and build button

  set o(mergeAndBuildbutton) [vtkNew vtkKWPushButton]
  $o(mergeAndBuildbutton) SetParent $o(allButtonsFrame)
  $o(mergeAndBuildbutton) Create
  $o(mergeAndBuildbutton) SetText "Merge And Build"
  $o(mergeAndBuildbutton) SetBalloonHelpString "Merge all structures into Merge Volume and build models from all structures"
  pack [$o(mergeAndBuildbutton) GetWidgetName] -side left


  #
  # add observations
  #
  set broker $::slicer3::Broker

  # observations on kwwidgets are deleted by the broker automatically when
  # this class destructs, but observations of the scene must be explicitly 
  # removed in the destuctor

  # node selected event
  $broker AddObservation $o(masterSelector) 11000 "$this select"
  # invoked event
  $broker AddObservation $o(addStructureButton) 10000 "$this addStructure"
  $broker AddObservation $o(deleteStructuresButton) 10000 "$this deleteStructures"
  # selection changed event
  $broker AddObservation [$o(structures) GetWidget] 10000 "$this edit"
  # invoked event
  $broker AddObservation $o(mergeButton) 10000 "$this merge"
  $broker AddObservation $o(mergeAndBuildbutton) 10000 "$this merge; $this build"

  # new scene, node added or removed events
  lappend _observations [$broker AddObservation $::slicer3::MRMLScene 66000 "$this updateStructures"]
  lappend _observations [$broker AddObservation $::slicer3::MRMLScene 66001 "$this updateStructures"]
  lappend _observations [$broker AddObservation $::slicer3::MRMLScene 66002 "$this updateStructures"]

  # so buttons will initially be disabled
  set _master ""
  $this updateStructures


  if { $frame != "" } {
    # nothing, calling code will pack the frame
  } else {
    $o(toplevel) Display
  }
}

# color table dialog
itcl::body HelperBox::colorSelectDialog { } {

  if { ![info exists o(colorSelectTopLevel)] } {
    set o(colorSelectTopLevel) [vtkNew vtkKWTopLevel]
    $o(colorSelectTopLevel) SetApplication $::slicer3::Application
    $o(colorSelectTopLevel) ModalOn
    $o(colorSelectTopLevel) Create
    $o(colorSelectTopLevel) SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $o(colorSelectTopLevel) HideDecorationOff
    $o(colorSelectTopLevel) Withdraw
    $o(colorSelectTopLevel) SetBorderWidth 2
    $o(colorSelectTopLevel) SetReliefToGroove

    set topFrame [vtkNew vtkKWFrame]
    $topFrame SetParent $o(colorSelectTopLevel)
    $topFrame Create
    pack [$topFrame GetWidgetName] -side top -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(promptLabel) [vtkNew vtkKWLabel]
    $o(promptLabel) SetParent $topFrame
    $o(promptLabel) Create
    pack [$o(promptLabel) GetWidgetName] -side top -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(colorSelector) [vtkNew vtkSlicerNodeSelectorWidget]
    $o(colorSelector) SetParent $topFrame
    $o(colorSelector) Create
    $o(colorSelector) SetNodeClass "vtkMRMLColorNode" "" "" ""
    $o(colorSelector) AddExcludedChildClass "vtkMRMLDiffusionTensorDisplayPropertiesNode"
    $o(colorSelector) NewNodeEnabledOff
    $o(colorSelector) NoneEnabledOff
    $o(colorSelector) DefaultEnabledOn
    $o(colorSelector) ShowHiddenOn
    $o(colorSelector) ChildClassesEnabledOn
    $o(colorSelector) SetMRMLScene $::slicer3::MRMLScene
    $o(colorSelector) UpdateMenu
    $o(colorSelector) SetLabelText "Color Table:"
    $o(colorSelector) SetBalloonHelpString "Pick the table of structures you wish to edit"
    pack [$o(colorSelector) GetWidgetName] -side top -fill x -expand true


    set buttonFrame [vtkNew vtkKWFrame]
    $buttonFrame SetParent $topFrame
    $buttonFrame Create
    pack [$buttonFrame GetWidgetName] -side left -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(colorDialogApply) [vtkNew vtkKWPushButton]
    $o(colorDialogApply) SetParent $buttonFrame
    $o(colorDialogApply) Create
    $o(colorDialogApply) SetText Apply
    set o(colorDialogCancel) [vtkNew vtkKWPushButton]
    $o(colorDialogCancel) SetParent $buttonFrame
    $o(colorDialogCancel) Create
    $o(colorDialogCancel) SetText Cancel
    pack [$o(colorDialogCancel) GetWidgetName] [$o(colorDialogApply) GetWidgetName] -side left -padx 4 -anchor c 

    # invoked event
    set broker $::slicer3::Broker
    $broker AddObservation $o(colorDialogApply) 10000 "$this createMerge; $o(colorSelectTopLevel) Withdraw"
    $broker AddObservation $o(colorDialogCancel) 10000 "$o(colorSelectTopLevel) Withdraw"
  }

  $o(promptLabel) SetText "Create a merge label map for selected master volume [$_master GetName].\nNew volume will be [$_master GetName]-label.\nSelected the color table node will be used for segmentation labels."

  $o(colorSelectTopLevel) DeIconify
  $o(colorSelectTopLevel) Raise
}


