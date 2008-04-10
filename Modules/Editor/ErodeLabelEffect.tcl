
package require Itcl

#########################################################
#
if {0} { ;# comment

  ErodeLabelEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ErodeLabelEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ErodeLabelEffect] == "" } {

  itcl::class ErodeLabelEffect {

    inherit EffectSWidget

    variable _fillColor

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
itcl::body ErodeLabelEffect::constructor {sliceGUI} {
}

itcl::body ErodeLabelEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ErodeLabelEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }
}

itcl::body ErodeLabelEffect::apply {} {

}

itcl::body ErodeLabelEffect::buildOptions {} {

  tk_messageBox -message "in $this"

  # call superclass version of buildOptions
  chain

  # create the edit box and the color picker - note these aren't kwwidgets
  #  but helper classes that create kwwidgets in the given frame
  set _fillColor [::EditColor #auto]
  $_fillColor configure -frame [$this getOptionsFrame]
  $_fillColor create

  set o(smooth) [vtkKWCheckButtonWithLabel New]
  $o(smooth) SetParent [$this getOptionsFrame]
  $o(smooth) Create
  $o(smooth) SetLabelText "Smooth Model: "
  $o(smooth) SetBalloonHelpString "When smoothed, the model will look better, but some details of the label map will not be visible on the model.  When not smoothed you will see individual voxel boundaries in the model.  Smoothing here corresponds to Decimation of 0.25 and Smooting iterations of 10."
  pack [$o(smooth) GetWidgetName] \
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
  $o(help) SetHelpTitle "ErodeLabel"
  $o(help) SetHelpText "Use this tool build a model.  A subset of model building options is provided here.  Go to the Model Maker module to expose a range of parameters."
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
    $this errorDialog "Label map needed for ErodeLabeling"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body ErodeLabelEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  itcl::delete object $_fillColor

  foreach w "smooth help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

itcl::body ErodeLabelEffect::goToModelMaker { } {
  set toolbar [$::slicer3::ApplicationGUI GetApplicationToolbar]
  [$toolbar GetModuleChooseGUI] SelectModule "Model Maker"
  after idle ::EffectSWidget::RemoveAll
}
