
package require Itcl

#########################################################
#
if {0} { ;# comment

  MakeModelEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             MakeModelEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class MakeModelEffect] == "" } {

  itcl::class MakeModelEffect {

    inherit EffectSWidget

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method apply {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method goToModelMaker {} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body MakeModelEffect::constructor {sliceGUI} {
}

itcl::body MakeModelEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body MakeModelEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }
}

itcl::body MakeModelEffect::apply {} {

  #
  # create a model using the command line module
  # based on the current editor parameters
  #

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
  set moduleNode [$::slicer3::MRMLScene CreateNodeByClass "vtkMRMLCommandLineModuleNode"]
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
    set outHierarchy [$::slicer3::MRMLScene CreateNodeByClass "vtkMRMLModelHierarchyNode"]
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

itcl::body MakeModelEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # go directly to model maker button
  #
  set o(goToModelMaker) [vtkNew vtkKWPushButton]
  $o(goToModelMaker) SetParent [$this getOptionsFrame]
  $o(goToModelMaker) Create
  $o(goToModelMaker) SetText "Go To Model Maker"
  $o(goToModelMaker) SetBalloonHelpString "The Model Maker interface contains a whole range of options for building sets of models and controlling the parameters."
  pack [$o(goToModelMaker) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # smooth
  #
  set o(smooth) [vtkKWCheckButtonWithLabel New]
  $o(smooth) SetParent [$this getOptionsFrame]
  $o(smooth) Create
  $o(smooth) SetLabelText "Smooth Model: "
  [$o(smooth) GetWidget] SetSelectedState 1
  $o(smooth) SetBalloonHelpString "When smoothed, the model will look better, but some details of the label map will not be visible on the model.  When not smoothed you will see individual voxel boundaries in the model.  Smoothing here corresponds to Decimation of 0.25 and Smooting iterations of 10."
  pack [$o(smooth) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # model name
  #
  set o(name) [vtkKWEntryWithLabel New]
  $o(name) SetParent [$this getOptionsFrame]
  $o(name) Create
  $o(name) SetLabelText "Name: "
  [$o(name) GetWidget] SetValue "Quick Model"
  $o(name) SetBalloonHelpString "Select the name for the newly created model."
  pack [$o(name) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # an apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Build a model for the current label value of the label map being edited in the Red slice window.  Model will be created in the background."
  pack [$o(apply) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 


  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel this effect."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # a help button
  #
  set o(help) [vtkNew vtkSlicerPopUpHelpWidget]
  $o(help) SetParent [$this getOptionsFrame]
  $o(help) Create
  $o(help) SetHelpTitle "MakeModel"
  $o(help) SetHelpText "Use this tool build a model.  A subset of model building options is provided here.  Go to the Model Maker module to expose a range of parameters."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(goToModelMaker) AddObserver AnyEvent "$this goToModelMaker"]
  lappend _observerRecords "$o(goToModelMaker) $tag"
  set tag [$o(apply) AddObserver AnyEvent "$this apply"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getOutputLabel] == "" } {
    $this errorDialog "Label map needed for MakeModeling"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body MakeModelEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "smooth name goToModelMaker help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

itcl::body MakeModelEffect::goToModelMaker { } {
  set toolbar [$::slicer3::ApplicationGUI GetApplicationToolbar]
  [$toolbar GetModuleChooseGUI] SelectModule "Model Maker"
  after idle ::EffectSWidget::RemoveAll
}
