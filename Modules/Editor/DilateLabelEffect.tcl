
package require Itcl

#########################################################
#
if {0} { ;# comment

  DilateLabelEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             DilateLabelEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class DilateLabelEffect] == "" } {

  itcl::class DilateLabelEffect {

    inherit EffectSWidget

    constructor {sliceGUI} {Labeler::constructor $sliceGUI} {}
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
itcl::body DilateLabelEffect::constructor {sliceGUI} {
}

itcl::body DilateLabelEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body DilateLabelEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }
}

itcl::body DilateLabelEffect::apply {} {

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Label map needed for Dilate"
    return
  }


  set erode [vtkImageErode New]
  $erode SetInput [$this getInputLabel]
  $erode SetOutput [$this getOutputLabel]

  $erode SetBackground [EditorGetPaintLabel]
  $erode SetForeground [[$o(fill) GetWidget] GetValue]
  set neighborhood [set ::[$o(fourNeighbors) GetVariableName]]
  $erode SetNeighborTo$neighborhood

  set iterations [[$o(iterations) GetWidget] GetValue]
  for {set i 0} {$i < $iterations} {incr i} {
    $this setProgressFilter $erode "Dilate ($i)"
    $erode Update
  }

  $erode SetOutput [$this getOutputLabel]

  $erode Delete

  $this postApply

}

itcl::body DilateLabelEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # iterations and label value
  #

  set o(fill) [vtkKWEntryWithLabel New]
  $o(fill) SetParent [$this getOptionsFrame]
  $o(fill) Create
  [$o(fill) GetWidget] SetRestrictValueToInteger
  [$o(fill) GetWidget] SetValueAsInt 0
  $o(fill) SetLabelText "Background Label: "
  $o(fill) SetBalloonHelpString "Dilated pixels will replace this value."
  pack [$o(fill) GetWidgetName] -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  set o(iterations) [vtkKWEntryWithLabel New]
  $o(iterations) SetParent [$this getOptionsFrame]
  $o(iterations) Create
  [$o(iterations) GetWidget] SetRestrictValueToInteger
  [$o(iterations) GetWidget] SetValueAsInt 1
  $o(iterations) SetLabelText "Iterations: "
  $o(iterations) SetBalloonHelpString "Number of times to apply selected operation."
  # TODO: support iterations
  #pack [$o(iterations) GetWidgetName] -side top -anchor e -fill x -padx 2 -pady 2 


  #
  # 4 or 8 neighbors
  #

  set o(eightNeighbors) [vtkKWRadioButton New]
  $o(eightNeighbors) SetParent [$this getOptionsFrame]
  $o(eightNeighbors) Create
  $o(eightNeighbors) SetValueAsInt 8
  $o(eightNeighbors) SetText "Eight Neighbors"
  $o(eightNeighbors) SetBalloonHelpString "Treat diagonally adjacent voxels as neighbors."
  pack [$o(eightNeighbors) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  set o(fourNeighbors) [vtkKWRadioButton New]
  $o(fourNeighbors) SetParent [$this getOptionsFrame]
  $o(fourNeighbors) Create
  $o(fourNeighbors) SetValueAsInt 4
  $o(fourNeighbors) SetText "Four Neighbors"
  $o(fourNeighbors) SetBalloonHelpString "Do not treat diagonally adjacent voxels as neighbors."
  pack [$o(fourNeighbors) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  $o(fourNeighbors) SetSelectedState 1
  $o(eightNeighbors) SetVariableName [$o(fourNeighbors) GetVariableName] 

  #
  # erode or erode then dilate
  #

  set o(erode) [vtkKWRadioButton New]
  $o(erode) SetParent [$this getOptionsFrame]
  $o(erode) Create
  $o(erode) SetValue "Dilate"
  $o(erode) SetText "Dilate"
  $o(erode) SetBalloonHelpString "Treat diagonally adjacent voxels as neighbors."
  # TODO: support erode and dilate
  #pack [$o(erode) GetWidgetName] -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  set o(erodeDilate) [vtkKWRadioButton New]
  $o(erodeDilate) SetParent [$this getOptionsFrame]
  $o(erodeDilate) Create
  $o(erodeDilate) SetValue "Dilate&Erode"
  $o(erodeDilate) SetText "Dilate&Erode"
  $o(erodeDilate) SetBalloonHelpString "Do not treat diagonally adjacent voxels as neighbors."
  # TODO: support erode and dilate
  #pack [$o(erodeDilate) GetWidgetName] -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  $o(erode) SetSelectedState 1
  $o(erodeDilate) SetVariableName [$o(erode) GetVariableName] 

  #
  # an apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Run the selected operation."
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
  $o(help) SetHelpTitle "DilateLabel"
  $o(help) SetHelpText "Use this tool to remove pixels from the boundary of the current label."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(apply) AddObserver AnyEvent "$this apply"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getOutputLabel] == "" } {
    $this errorDialog "Label map needed for DilateLabeling"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body DilateLabelEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "fill iterations fourNeighbors eightNeighbors erode erodeDilate help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

