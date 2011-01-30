
#
# generic HideStep
#
proc HideStep {step} {
  upvar 0 ::WIZARD w

  [$w(wiz) GetWizardWidget] ClearPage
}

#
# step 1: pick the operation
#
proc ShowStep1 {step} { 
  upvar 0 ::WIZARD w

  set wiz_widget [$w(wiz) GetWizardWidget]
  set parent [$wiz_widget GetClientArea]

  if { ![info exists w(addVol)] } {
    set w(buttons) [vtkKWRadioButtonSet New]
    $w(buttons) SetParent $parent
    $w(buttons) Create

    set w(addVol) [vtkKWPushButton New]
    $w(addVol) SetParent $parent
    $w(addVol) Create
    $w(addVol) SetCommand $::slicer3::Application "Eval ::LoadVolume::ShowDialog"
    $w(addVol) SetText "Add Volume..."

    foreach vol {Fixed Moving} {
      set w(sel$vol) [vtkSlicerNodeSelectorWidget New]
      $w(sel$vol) SetParent $parent
      $w(sel$vol) SetMRMLScene $::slicer3::MRMLScene
      $w(sel$vol) Create
      $w(sel$vol) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
      $w(sel$vol) ChildClassesEnabledOff
      $w(sel$vol) UnconditionalUpdateMenu
      $w(sel$vol) SetLabelText "$vol Volume:"
    }
    $w(selFixed) SetBalloonHelpString "Pick the volume node to use as the reference for registration.  Transform will map moving image to this space.  This volume will become background in slice views"
    $w(selMoving) SetBalloonHelpString "Pick the volume node to use as the variable eference for registration.  Transform will map this volume into Fixed space.  This volume will become foreground in slice views"
  }

  pack [$w(addVol) GetWidgetName] -side top -expand y -fill none -anchor center
  foreach vol {Fixed Moving} {
    pack [$w(sel$vol) GetWidgetName] -side top -expand y -fill none -anchor center
  }
}

proc ValidateStep1 {step} {
  upvar 0 ::WIZARD w

  set wiz_widget [$w(wiz) GetWizardWidget]
  set workflow [$wiz_widget GetWizardWorkflow]

  set failed 0
  foreach vol {Fixed Moving} {
    if { [$w(sel$vol) GetSelected] == "" } {
      $wiz_widget SetErrorText "Must select $vol Volume" 
      $workflow PushInput [$w(step1) GetValidationFailedInput]
      set failed 1
    }
  }

  set fixed [$w(selFixed) GetSelected]
  set moving [$w(selMoving) GetSelected]
  if { !$failed && ($fixed == $moving) } {
    $wiz_widget SetErrorText "Fixed and Moving Volumes must differ" 
    $workflow PushInput [$w(step1) GetValidationFailedInput]
    set failed 1
  }

  if { !$failed } {
    $wiz_widget SetErrorText "" 
    $workflow PushInput [$w(step1) GetValidationSucceededInput]

    # make the source node the active background, and the label node the active label
    set selectionNode [$::slicer3::ApplicationLogic  GetSelectionNode]
    $selectionNode SetReferenceActiveVolumeID [$fixed GetID]
    $selectionNode SetReferenceSecondaryVolumeID [$moving GetID]
    $::slicer3::ApplicationLogic PropagateVolumeSelection

    set nnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]
    for {set n 0} {$n < $nnodes} {incr n} {
      set cnode [$::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLSliceCompositeNode"]
      $cnode SetForegroundOpacity 0.5
    }


    # TODO: set up transformations
  }
}


#
# step 2: TODO: fill this with registration-related code
#
proc ShowStep2 {step} { 
  upvar 0 ::WIZARD w

  set wiz_widget [$w(wiz) GetWizardWidget]
  set parent [$wiz_widget GetClientArea]

  if { ![info exists w(entry1)] } {
    set w(entry1) [vtkKWEntry New]
    $w(entry1) SetParent $parent
    $w(entry1) Create
    $w(entry1) SetRestrictValueToDouble
    $w(entry1) SetCommandTriggerToAnyChange
    $w(entry1) SetCommand ""
  }

  pack [$w(entry1) GetWidgetName] -side top -expand y -fill none -anchor center
}

proc RunRegistration {} {

  # TODO: convert this to something that calls registration

  #
  # find the Register
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
  set outHierarchy [[$::slicer3::MRMLScene GetNodesByClassByName "vtkMRMLModelHierarchyNode" "Editor Models"] GetItemAsObject 0]
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

proc ValidateStep2 {step} {
  upvar 0 ::WIZARD w

  set wiz_widget [$w(wiz) GetWizardWidget]
  set workflow [$wiz_widget GetWizardWorkflow]
}

#
# main wizard code
#
proc TestWizard {} {
  
  array unset ::WIZARD
  array set ::WIZARD ""
  upvar 0 ::WIZARD w

  set w(wiz) [vtkKWWizardDialog New]
  $w(wiz) SetTitle "Registration Helper"
  $w(wiz) SetApplication $::slicer3::Application
  $w(wiz) SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $w(wiz) SetDisplayPositionToMasterWindowCenter
  $w(wiz) ModalOff
  $w(wiz) Create

  set workflow [$w(wiz) GetWizardWorkflow]

  set w(step1) [vtkKWWizardStep New]
  $w(step1) SetName "Inputs"
  $w(step1) SetDescription "Select the Fixed and Moving input volumes."
  $w(step1) SetShowUserInterfaceCommand $::slicer3::Application "Eval ShowStep1 step1"
  $w(step1) SetValidateCommand $::slicer3::Application "Eval ValidateStep1 step1"
  $w(step1) SetHideUserInterfaceCommand $::slicer3::Application "Eval HideStep step1"

  set w(step2) [vtkKWWizardStep New]
  $w(step2) SetShowUserInterfaceCommand $::slicer3::Application "Eval ShowStep2 step2"
  $w(step2) SetValidateCommand $::slicer3::Application "Eval ValidateStep2 step2"
  $w(step2) SetHideUserInterfaceCommand $::slicer3::Application "Eval HideStep step2"

  set w(step3) [vtkKWWizardStep New]

  $workflow AddStep $w(step1)
  $workflow AddNextStep $w(step2)
  $workflow AddNextStep $w(step3)
  $workflow SetInitialStep $w(step1)
  $workflow SetFinishStep $w(step3)


  $w(wiz) Invoke

}
