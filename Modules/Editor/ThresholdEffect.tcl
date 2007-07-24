
package require Itcl

#########################################################
#
if {0} { ;# comment

  ThresholdEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ThresholdEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ThresholdEffect] == "" } {

  itcl::class ThresholdEffect {

    inherit EffectSWidget

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    public variable range ""
    public variable label "1"

    # methods
    method processEvent {} {}
    method preview {} {}
    method apply {} {}
    method positionCursor {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method previewOptions {} {}
    method applyOptions {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body ThresholdEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body ThresholdEffect::destructor {} {
  $this tearDownOptions
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ThresholdEffect::processEvent { } {

  if { [$this preProcessEvent] } {
    # superclass processed the event, so we don't
    return
  }

  # TODO: there needs be a way to preview the threshold
  # on a per-slice basis... this should probably be done
  # by grabbing the background out of the logic layer

}

itcl::body ThresholdEffect::apply {} {

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  set conn [vtkImageThreshold New]
  $conn SetFunctionToThreshold
  $conn SetSeed $_layers(label,i) $_layers(label,j) $_layers(label,k) 
  $conn SetInput [$this getInputLabel]
  $conn SetOutput [$this getOutputLabel]
  [$this getOutputLabel] Update
  $conn Delete

  $this postApply
}


itcl::body ThresholdEffect::preview {} {

  $this configure -scope "visible"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  set thresh [vtkImageThreshold New]
  $thresh SetInput [$this getInputBackground]
  #TODO: this has already been windowed - so range is not correct
  eval $thresh ThresholdBetween $range
  $thresh SetInValue 255
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToShort
  $thresh Update
  $o(cursorMapper) SetInput [$thresh GetOutput]
  $o(cursorActor) VisibilityOn
  $thresh Delete

  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body ThresholdEffect::positionCursor {} {
  foreach actor $_cursorActors {
    $actor SetPosition 0 0
  }
}

itcl::body ThresholdEffect::buildOptions { } {
  
  #
  # a range setting for threshold values
  #
  set o(range) [vtkNew vtkKWRange]
  $o(range) SetParent [$this getOptionFrame]
  $o(range) Create
  $o(range) SetLabelText "Min/Max for Threshold Paint"
  $o(range) SetWholeRange 0 2000
  $o(range) SetRange 50 2000
  $o(range) SetReliefToGroove
  $o(range) SetBalloonHelpString "Set the range of the background values that should be labeled."

  set range [[$this getInputBackground] GetScalarRange]
  eval $o(range) SetWholeRange $range
  eval $o(range) SetRange $range

  pack [$o(range) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # an apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Apply current threshold settings to the label map."
  pack [$o(apply) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(range) AddObserver AnyEvent "after idle $this previewOptions"]
  lappend _observerRecords "$o(range) $tag"
  set tag [$o(apply) AddObserver AnyEvent "$this applyOptions"]
  lappend _observerRecords "$o(apply) $tag"
}

itcl::body ThresholdEffect::tearDownOptions { } {
  if { [info exists o(range)] } {
    foreach w "range apply" {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

itcl::body ThresholdEffect::previewOptions { } {
  foreach te [itcl::find objects -class ThresholdEffect] {
    $te configure -range [$o(range) GetRange]
    $te preview
  }
}

itcl::body ThresholdEffect::applyOptions { } {
  puts "got apply callback"
  $this previewOptions
  foreach te [itcl::find objects -class ThresholdEffect] {
    $te apply
  }
}

