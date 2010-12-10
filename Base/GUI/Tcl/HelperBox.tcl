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

  proc Recreate {} {
    foreach helper [itcl::find objects -class HelperBox] {
      set f [$helper cget -frame]
      itcl::delete object $helper
      set helper [HelperBox #auto]
      $helper configure -frame $f
      $helper create
    }
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

    variable _master "" ;# current master volume node 
    variable _merge "" ;# the current merge volume
    variable _masterWhenMergeWasSet "" ;# value of the master volume last time merge was selected (to detect changes)
    variable _colorBox ""

    variable _createMergeOptions ""

    variable _observations

    # methods
    method create {} {} ;# create the widget

    method createMerge { } {} ;# make a merge volume for the current selection
    method newMerge { } {} ;# prompt for a merge volume even if one exists
    method select {} {} ;# select the master volume node
    method rotateSlicesToImage {} {} ;# make sure all slices are aligned with Bb/Label orientation
    method mergeVolume {} {} ;# helper to get the merge volume for the current selected master
    method setMergeVolume { {mergeVolume ""} } {} ;# let the user explicitly set merge volume or with dialog
    method setMasterVolume { masterVolume } {} ;# programmatically set the master (invokes the gui callback indirectly
    method setVolumes { masterVolume mergeVolumes } {} ;# programmatically set the master and merge (invokes the gui callback indirectly
    method structureVolume { structureName } {} ;# get the volume for a given structure name

    method promptStructure {} {} ;# ask which label value to use
    method addStructure { {label ""} {options ""} } {} ;# add a new dependent volume
    method deleteStructures { {options "confirm"} } {} ;# delete all the structures
    method updateStructures {} {} ;# refresh the structure frame with currently loaded data
    method merge { {label all} } {} ;# merge the named or all structure labels into the master label
    method split {} {} ;# extract a label volume for each distinct label in the merge volume
    method edit { {label ""} } {} ;# select structure 'label' for editing
    method build {} {} ;# build merged model for all labels
    method colorSelectDialog {} {} ;# pop up a dialog with color table selection
    method labelSelectDialog {} {} ;# pop up a dialog to select a label map

    method processEvent { caller event } {}
    method getNodeByName { nodeName } {}

    # to support testing, accessor to instance variables
    method variables {} {
      set v(_master) $_master
      set v(_merge) $_merge
      set v(_masterWhenMergeWasSet) $_masterWhenMergeWasSet
      set v(_colorBox) $_colorBox
      return [array get v]
    }
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

  if { $_colorBox != "" } {
    itcl::delete object $_colorBox
  }
}

# create a merge volume for the current master even if one exists
itcl::body HelperBox::newMerge {} {
  set _createMergeOptions "new"
  $this colorSelectDialog  
}

# create a merge volume for the current master
itcl::body HelperBox::createMerge {} {
  if { $_master == "" } {
    # should never happen
    EditorErrorDialog "Cannot create merge volume without master"
    return
  }

  set masterName [$_master GetName]
  set mergeName $masterName-label
  if { [lsearch $_createMergeOptions "new"] == -1 } {
    set merge [$this mergeVolume]
  } else {
    set merge ""
  }
  set _createMergeOptions ""
  if { $merge == "" } {
    set volumesLogic [$::slicer3::VolumesGUI GetLogic]
    set merge [$volumesLogic CreateLabelVolume $::slicer3::MRMLScene $_master $mergeName]
    [$merge GetDisplayNode] SetAndObserveColorNodeID [[$o(colorSelector) GetSelected] GetID]
    $this setMergeVolume $merge
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
      $selectionNode SetReferenceActiveLabelVolumeID [$merge GetID]
      $::slicer3::ApplicationLogic  PropagateVolumeSelection 0
      set mergeText [$merge GetName]
      set _merge $merge
    }
  } else {
    # the master exists, but there is no merge volume yet 
    # bring up dialog to create a merge with a user-selected color node
    if { $_master != "" } {
      $this colorSelectDialog
    }
  }
  EffectSWidget::RotateToVolumePlanes ;# make sure slices are aligned to volumes
  [$o(mergeName) GetWidget] SetText $mergeText
  $this updateStructures
  # remove all active effects (so that when selected again slice rotation and snap to IJK will happen if needed
  ::EffectSWidget::RemoveAll
}

itcl::body HelperBox::rotateSlicesToImage {} {
}


itcl::body HelperBox::setVolumes { masterVolume mergeVolume } {
  # set both volumes at the same time - trick the callback into 
  # thinking that the merge volume is already set so it won't prompt for a new one
  set _merge $mergeVolume
  set _masterWhenMergeWasSet $masterVolume
  $this setMasterVolume $masterVolume
}

# select merge volume 
itcl::body HelperBox::setMasterVolume { masterVolume } {
  $o(masterSelector) SetSelected $masterVolume
  $this select
}

# select merge volume 
itcl::body HelperBox::setMergeVolume { {mergeVolume ""} } {
  if { $_master == "" } {
    return ""
  }
  if { $mergeVolume != "" } {
    set _merge $mergeVolume
    if { [info exists o(labelMapSelector)] } {
      $o(labelMapSelector) SetSelected $_merge
    }
  } else {
    if { [info exists o(labelMapSelector)] } {
      set _merge [$o(labelMapSelector) GetSelected]
    }
  }
  set _masterWhenMergeWasSet $_master
  $this select
}

# select merge volume 
itcl::body HelperBox::mergeVolume {} {
  if { $_master == "" || [info command $_master] == "" } {
    return ""
  }

  # if we already have a merge and the master hasn't changed, use it
  if { $_merge != "" && $_master == $_masterWhenMergeWasSet } {
    if { [$::slicer3::MRMLScene GetNodeByID [$_merge GetID]] != "" } {
      return $_merge
    } 
  }

  set _merge ""
  set _masterWhenMergeWasSemasterWhenMergeWasSet ""

  # otherwise pick the merge based on the master name
  # - either return the merge volume or empty string
  set masterName [$_master GetName]
  set mergeName $masterName-label
  set _merge [$this getNodeByName $mergeName]
  return $_merge
}

# select structure volume 
itcl::body HelperBox::structureVolume { structureName } {
  if { $_master == "" } {
    return ""
  }
  set masterName [$_master GetName]
  set structureVolumeName $masterName-$structureName-label
  return [$this getNodeByName $structureVolumeName]
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

  if { $_colorBox == "" } {
    set _colorBox [ColorBox #auto]
    $_colorBox configure -colorNode $colorNode
    $_colorBox create
    $_colorBox configure -selectCommand "$this addStructure %d"
  } else {
    $_colorBox configure -colorNode $colorNode
    $_colorBox show
  }
}


# create the segmentation helper box
itcl::body HelperBox::addStructure { {label ""} {options ""} } {

  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 

  if { $label == "" } {
    # if no label given, prompt the user.  The selectCommand of the colorBox will
    # then re-invoke this method with the label value set and we will continue
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
  if { [lsearch $options "noEdit"] == -1 } {
    $this edit $label
  }
}

# delete all the structures
itcl::body HelperBox::deleteStructures { {options "confirm"} } {

  #
  # iterate through structures and delete them
  #
  set w [$o(structures) GetWidget]
  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 

  set rows [$w GetNumberOfRows]

  if { [lsearch $options "yes"] != -1 && ![EditorConfirmDialog "Delete $rows structure volume(s)?"] } {
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

# split the merge volume into individual strutures
itcl::body HelperBox::split {} {

  $this statusText "Splitting..."
  set merge [$this mergeVolume]
  if { $merge == "" } {
    return
  } 
  set colorNode [[$merge GetDisplayNode] GetColorNode]

  set w [$o(structures) GetWidget]

  set accum [vtkImageAccumulate New]
  $accum SetInput [$merge GetImageData]
  $accum Update
  set lo [expr int([lindex [$accum GetMin] 0])]
  set hi [expr int([lindex [$accum GetMax] 0])]
  $accum Delete

  set thresholder [vtkImageThreshold New]
  for {set i $lo} {$i <= $hi} {incr i} {
    $this statusText "Splitting label $i..."
    $thresholder SetInput [$merge GetImageData]
    $thresholder SetInValue $i
    $thresholder SetOutValue 0
    $thresholder ReplaceInOn
    $thresholder ReplaceOutOn
    $thresholder ThresholdBetween $i $i
    $thresholder SetOutputScalarType [[$merge GetImageData] GetScalarType]
    $thresholder Update
    if { [[$thresholder GetOutput] GetScalarRange] != " 0.0 0.0" } {
      set labelName [$colorNode GetColorName [expr int($i)]]
      $this statusText "Creating structure volume $labelName..."
      set structureVolume [$this structureVolume $labelName]
      if { $structureVolume == "" } {
        addStructure $i "noEdit"
      }
      set structureVolume [$this structureVolume $labelName]
      [$structureVolume GetImageData] DeepCopy [$thresholder GetOutput]
      $structureVolume Modified
    }
  }
  $thresholder Delete

  $this statusText "Finished splitting."
}

itcl::body HelperBox::build {} {

  #
  # get the image data for the label layer
  #

  $this statusText "Building..."
  set merge [$this mergeVolume]
  if { $merge == "" } {
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

  $moduleNode SetParameterAsString "FilterType" "Sinc"
  $moduleNode SetParameterAsBool "GenerateAll" "1"
  $moduleNode SetParameterAsBool "JointSmoothing" 1
  $moduleNode SetParameterAsBool "SplitNormals" 1
  $moduleNode SetParameterAsBool "PointNormals" 1
  $moduleNode SetParameterAsBool "SkipUnNamed" 1
  $moduleNode SetParameterAsInt "Start" -1
  $moduleNode SetParameterAsInt "End" -1
  $moduleNode SetParameterAsDouble "Decimate" 0.25
  $moduleNode SetParameterAsDouble "Smooth" 10

  $moduleNode SetParameterAsString "InputVolume" [$merge GetID]

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

  if { $outHierarchy != "" && [[$o(replaceModels) GetWidget] GetSelectedState] } {
    # user wants to delete any existing models, so take down hierarchy
    for {set n [expr $numNodes - 1]} {$n >= 0} {incr n -1} {
      set node [$::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLModelHierarchyNode"]
      if { [$node GetParentNodeID] == [$outHierarchy GetID] } {
        $::slicer3::MRMLScene RemoveNode [$node GetModelNode]
        $::slicer3::MRMLScene RemoveNode $node
      }
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
  if { $structureVolume != "" } {
    $selectionNode SetReferenceActiveLabelVolumeID [$structureVolume GetID]
  }
  $::slicer3::ApplicationLogic  PropagateVolumeSelection 0

  EditorSetPaintLabel $label

}

# re-build the Structures frame
itcl::body HelperBox::updateStructures {} {

  $o(masterSelector) UpdateMenu

  if { $_master == "" } {
    $o(setMergeButton) SetStateToDisabled
  } else {
    $o(setMergeButton) SetStateToNormal
  }

  set w [$o(structures) GetWidget]
  $w DeleteAllRows

  # no merge volume exists, disable everything
  set merge [$this mergeVolume]
  if { $merge == "" } {
    $o(addStructureButton) SetStateToDisabled
    $o(deleteStructuresButton) SetStateToDisabled
    $o(mergeButton) SetStateToDisabled
    $o(splitButton) SetStateToDisabled
    $o(mergeAndBuildbutton) SetStateToDisabled
    EditorShowHideTools "hide"
    return
  }

  # we have a merge volume, proceed to fill out the widget in normal state
  $o(addStructureButton) SetStateToNormal
  $o(deleteStructuresButton) SetStateToNormal
  $o(mergeButton) SetStateToNormal
  $o(splitButton) SetStateToNormal
  $o(mergeAndBuildbutton) SetStateToNormal

  set colorNode [[$merge GetDisplayNode] GetColorNode]
  set lut [$colorNode GetLookupTable]

  set masterName [$_master GetName]
  $::slicer3::MRMLScene InitTraversal
  set vNode [$::slicer3::MRMLScene GetNextNodeByClass "vtkMRMLScalarVolumeNode"]
  while { $vNode != "" } {
    set vName [$vNode GetName]
    if { [string match "$masterName-*-label" $vName]  ||
         [string match "$masterName-*-label\[0-9\]*" $vName] } {
      # figure out what name it is
      # - account for the fact that sometimes a number will be added to the end of the name
      set start [expr 1+[string length $masterName]]
      set end [expr [string last "-label" $vName] - 1]
      set structureName [string range $vName $start $end]
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
  
  # show the tools if a structure has been selected
  EditorShowHideTools "show"
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
  set o(masterFrame) [vtkNew vtkKWFrame]
  $o(masterFrame) SetParent $parent
  $o(masterFrame) Create
  pack [$o(masterFrame) GetWidgetName] -side top -anchor nw -fill x -padx 2 -pady 2
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
  $o(masterSelector) SetBalloonHelpString "Pick the master structural volume to define the segmentation.  A label volume with the with \"-label\" appended to the name will be created if it doesn't already exist."
  pack [$o(masterSelector) GetWidgetName] -side top -fill x

  # master label name

  set o(mergeFrame) [vtkNew vtkKWFrame]
  $o(mergeFrame) SetParent $masterFrame
  $o(mergeFrame) Create
  pack [$o(mergeFrame) GetWidgetName] -side top -anchor nw -fill x -padx 2 -pady 2

  set o(mergeName) [vtkNew vtkKWLabelWithLabel]
  $o(mergeName) SetParent $o(mergeFrame)
  $o(mergeName) Create
  $o(mergeName) SetLabelText "Merge Volume:"
  $o(mergeName) SetBalloonHelpString "Composite label map containing the merged structures (be aware that merge operations will overwrite any edits applied to this volume)"
  [$o(mergeName) GetWidget] SetText "None"
  pack [$o(mergeName) GetWidgetName] -side left -fill x -expand false

  set o(setMergeButton) [vtkNew vtkKWPushButton]
  $o(setMergeButton) SetParent $o(mergeFrame)
  $o(setMergeButton) Create
  $o(setMergeButton) SetText "Set "
  $o(setMergeButton) SetBalloonHelpString "Set the merge volume to use with this master."
  pack [$o(setMergeButton) GetWidgetName] -side right


  #
  # Structures Frame
  # 

  set o(structuresFrame) [vtkNew vtkKWFrameWithLabel]
  $o(structuresFrame) SetParent $parent
  $o(structuresFrame) Create
  $o(structuresFrame) SetLabelText "Per-Structure Volumes"
  $o(structuresFrame) CollapseFrame
  pack [$o(structuresFrame) GetWidgetName] -side top -anchor nw -fill both -expand false -padx 2 -pady 2
  set structuresFrame [$o(structuresFrame) GetFrame]

  # buttons frame 

  set o(structureButtonsFrame) [vtkNew vtkKWFrame]
  $o(structureButtonsFrame) SetParent $structuresFrame
  $o(structureButtonsFrame) Create
  pack [$o(structureButtonsFrame) GetWidgetName] -side top -anchor sw -fill x -padx 2 -pady 2

  # add button

  set o(addStructureButton) [vtkNew vtkKWPushButton]
  $o(addStructureButton) SetParent $o(structureButtonsFrame)
  $o(addStructureButton) Create
  $o(addStructureButton) SetText "Add Structure"
  $o(addStructureButton) SetBalloonHelpString "Add a label volume for a structure to edit"
  pack [$o(addStructureButton) GetWidgetName] -side left

  # split button

  set o(splitButton) [vtkNew vtkKWPushButton]
  $o(splitButton) SetParent $o(structureButtonsFrame)
  $o(splitButton) Create
  $o(splitButton) SetText "Split Merge Volume"
  $o(splitButton) SetBalloonHelpString "Split distinct labels from merge volume into new volumes"
  pack [$o(splitButton) GetWidgetName] -side left

  # structures listbox

  set o(structures) [vtkNew vtkKWMultiColumnListWithScrollbars]
  $o(structures) SetParent $structuresFrame
  $o(structures) Create
  [$o(structures) GetWidget] SetHeight 4
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
  pack [$o(structures) GetWidgetName] -side top -anchor ne -padx 2 -pady 2 -expand false -fill x

  # all buttons frame 

  set o(allButtonsFrame) [vtkNew vtkKWFrame]
  $o(allButtonsFrame) SetParent $structuresFrame
  $o(allButtonsFrame) Create
  pack [$o(allButtonsFrame) GetWidgetName] -side bottom -anchor sw -fill x -padx 2 -pady 2


  # delete structures button

  set o(deleteStructuresButton) [vtkNew vtkKWPushButton]
  $o(deleteStructuresButton) SetParent $o(allButtonsFrame)
  $o(deleteStructuresButton) Create
  $o(deleteStructuresButton) SetText "Delete Structures"
  $o(deleteStructuresButton) SetBalloonHelpString "Delete all the structure volumes from the scene.\n\nNote: to delete individual structure volumes, use the Data Module."
  pack [$o(deleteStructuresButton) GetWidgetName] -side left

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

  # options frame

  set o(optionsFrame) [vtkNew vtkKWFrame]
  $o(optionsFrame) SetParent $structuresFrame
  $o(optionsFrame) Create
  pack [$o(optionsFrame) GetWidgetName] -side bottom -anchor sw -fill x -padx 2 -pady 2

  # replace models button

  set o(replaceModels) [vtkNew vtkKWCheckButtonWithLabel]
  $o(replaceModels) SetParent $o(optionsFrame)
  $o(replaceModels) Create
  $o(replaceModels) SetLabelText "Replace Models"
  $o(replaceModels) SetBalloonHelpString "Replace any existing models when building"
  [$o(replaceModels) GetWidget] SetSelectedState 1
  pack [$o(replaceModels) GetWidgetName] -side left


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
  $broker AddObservation $o(splitButton) 10000 "$this split"
  $broker AddObservation $o(mergeButton) 10000 "$this merge"
  $broker AddObservation $o(mergeAndBuildbutton) 10000 "$this merge; $this build"
  $broker AddObservation $o(setMergeButton) 10000 "$this labelSelectDialog"

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
    $o(colorSelectTopLevel) SetDisplayPositionToPointer
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

    # pick the default editor LUT for the user
    set defaultID [[$::slicer3::ColorGUI GetLogic] GetDefaultEditorColorNodeID]
    set defaultNode [$::slicer3::MRMLScene GetNodeByID $defaultID]
    if {$defaultNode != ""} {
      $o(colorSelector) SetSelected $defaultNode
    }

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

  $o(promptLabel) SetText "Create a merge label map for selected master volume [$_master GetName].\nNew volume will be [$_master GetName]-label.\nSelect the color table node will be used for segmentation labels."

  $o(colorSelectTopLevel) DeIconify
  $o(colorSelectTopLevel) Raise
}


# Label Select Dialog - prompt to pick a label map
itcl::body HelperBox::labelSelectDialog { } {

  if { ![info exists o(labelSelectTopLevel)] } {
    set o(labelSelectTopLevel) [vtkNew vtkKWTopLevel]
    $o(labelSelectTopLevel) SetApplication $::slicer3::Application
    $o(labelSelectTopLevel) ModalOn
    $o(labelSelectTopLevel) Create
    $o(labelSelectTopLevel) SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $o(labelSelectTopLevel) SetDisplayPositionToPointer
    $o(labelSelectTopLevel) HideDecorationOff
    $o(labelSelectTopLevel) Withdraw
    $o(labelSelectTopLevel) SetBorderWidth 2
    $o(labelSelectTopLevel) SetReliefToGroove

    set topFrame [vtkNew vtkKWFrame]
    $topFrame SetParent $o(labelSelectTopLevel)
    $topFrame Create
    pack [$topFrame GetWidgetName] -side top -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(labelPromptLabel) [vtkNew vtkKWLabel]
    $o(labelPromptLabel) SetParent $topFrame
    $o(labelPromptLabel) Create
    pack [$o(labelPromptLabel) GetWidgetName] -side top -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(labelMapSelector) [vtkNew vtkSlicerNodeSelectorWidget]
    $o(labelMapSelector) SetParent $topFrame
    $o(labelMapSelector) Create
    $o(labelMapSelector) SetNodeClass "vtkMRMLScalarVolumeNode" "LabelMap" "1" ""
    $o(labelMapSelector) NewNodeEnabledOff
    $o(labelMapSelector) NoneEnabledOff
    $o(labelMapSelector) DefaultEnabledOn
    $o(labelMapSelector) ShowHiddenOn
    $o(labelMapSelector) ChildClassesEnabledOff
    $o(labelMapSelector) SetMRMLScene $::slicer3::MRMLScene
    $o(labelMapSelector) UpdateMenu
    $o(labelMapSelector) SetLabelText "Label Map:"
    $o(labelMapSelector) SetBalloonHelpString "Pick the label map to edit."
    pack [$o(labelMapSelector) GetWidgetName] -side top -fill x -expand true

    set buttonFrame [vtkNew vtkKWFrame]
    $buttonFrame SetParent $topFrame
    $buttonFrame Create
    pack [$buttonFrame GetWidgetName] -side left -anchor w -padx 2 -pady 2 -fill both -expand true

    set o(labelDialogNew) [vtkNew vtkKWPushButton]
    $o(labelDialogNew) SetParent $buttonFrame
    $o(labelDialogNew) Create
    $o(labelDialogNew) SetText "Create New..."
    set o(labelDialogApply) [vtkNew vtkKWPushButton]
    $o(labelDialogApply) SetParent $buttonFrame
    $o(labelDialogApply) Create
    $o(labelDialogApply) SetText Apply
    set o(labelDialogCancel) [vtkNew vtkKWPushButton]
    $o(labelDialogCancel) SetParent $buttonFrame
    $o(labelDialogCancel) Create
    $o(labelDialogCancel) SetText Cancel
    pack [$o(labelDialogCancel) GetWidgetName] [$o(labelDialogApply) GetWidgetName] -side left -padx 4 -anchor c 
    pack [$o(labelDialogNew) GetWidgetName] -side right -padx 4 -anchor c 

    # invoked event
    set broker $::slicer3::Broker
    $broker AddObservation $o(labelDialogNew) 10000 "$this newMerge; $o(labelSelectTopLevel) Withdraw"
    $broker AddObservation $o(labelDialogApply) 10000 "$this setMergeVolume; $o(labelSelectTopLevel) Withdraw"
    $broker AddObservation $o(labelDialogCancel) 10000 "$o(labelSelectTopLevel) Withdraw"
  }

  $o(labelPromptLabel) SetText "Select existing label map volume to edit."

  $o(labelSelectTopLevel) DeIconify
  $o(labelSelectTopLevel) Raise
}

# get the first MRML node that has the given name
# - use a regular expression to match names post-pended with numbers
itcl::body HelperBox::getNodeByName { name } {

  $::slicer3::MRMLScene InitTraversal
  while { [set node [$::slicer3::MRMLScene GetNextNode]] != "" } {
    set ret [catch "$node GetName" nodeName]
    if { !$ret && ($name == $nodeName || [string match $name\[0-9\]* $nodeName]) } {
      return $node
    }
  }
  return ""
}
