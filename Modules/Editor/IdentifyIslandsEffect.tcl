
package require Itcl

#########################################################
#
if {0} { ;# comment

  IdentifyIslandsEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             IdentifyIslandsEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class IdentifyIslandsEffect] == "" } {

  itcl::class IdentifyIslandsEffect {

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
itcl::body IdentifyIslandsEffect::constructor {sliceGUI} {
  set _scopeOptions "all visible"
}

itcl::body IdentifyIslandsEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body IdentifyIslandsEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }
}

itcl::body IdentifyIslandsEffect::apply {} {

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Label map needed for Island operations"
    return
  }

  set castIn [vtkImageCast New]
  $castIn SetOutputScalarTypeToUnsignedLong
  $castIn SetInput [$this getInputLabel]

  set islands [vtkITKIslandMath New]
  $islands SetInput [$castIn GetOutput]

  set castOut [vtkImageCast New]
  $castOut SetOutputScalarTypeToShort
  $castOut SetInput [$islands GetOutput]
  $castOut SetOutput [$this getOutputLabel]

  $islands SetMinimumSize [[$o(minSize) GetWidget] GetValueAsInt]
  $islands SetFullyConnected [$o(fullyConnected) GetSelectedState]
  $this setProgressFilter $islands "Calculating Islands..."
  [$this getOutputLabel] Update
  set islandCount [$islands GetNumberOfIslands]
  set islandOrigCount [$islands GetOriginalNumberOfIslands]
  set ignoredIslands [expr $islandOrigCount - $islandCount]
  $this statusText "[$islands GetNumberOfIslands] islands created ($ignoredIslands ignored)"

  $castIn Delete
  $islands Delete
  $castOut Delete
  $this postApply

}

itcl::body IdentifyIslandsEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # iterations and label value
  #

  set o(minSize) [vtkKWEntryWithLabel New]
  $o(minSize) SetParent [$this getOptionsFrame]
  $o(minSize) Create
  [$o(minSize) GetWidget] SetRestrictValueToInteger
  [$o(minSize) GetWidget] SetValueAsInt 0
  $o(minSize) SetLabelText "Minimum Size: "
  $o(minSize) SetBalloonHelpString "Any islands smaller than this number of voxels will be ignored (label value will be 0)."
  pack [$o(minSize) GetWidgetName] -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  #
  # 4 or 8 neighbors
  #

  set o(fullyConnected) [vtkKWCheckButton New]
  $o(fullyConnected) SetParent [$this getOptionsFrame]
  $o(fullyConnected) Create
  $o(fullyConnected) SetText "Fully Connected"
  $o(fullyConnected) SetBalloonHelpString "When on, do not treat diagonally adjacent voxels as neighbors."
  $o(fullyConnected) SetSelectedState 0
  pack [$o(fullyConnected) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 -expand true

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
  $o(help) SetHelpTitle "IdentifyIslands"
  $o(help) SetHelpText "Use this tool to create a unique label value for each connected region in the current label map.  Connected regions are defined as groups of pixels which touch each other but are surrounded by zero valued voxels.  If FullyConnected is selected, then only voxels that share a face are counted as connected; if unselected, then voxels that touch at an edge or a corner are considered connected.\n\n Note: be aware that all non-zero label values labels values are considered equal by this filter and that the result will renumber the resulting islands in order of size."
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
    $this errorDialog "Label map needed for IdentifyIslandsing"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body IdentifyIslandsEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "minSize fullyConnected help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

