
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
    method setPaintThreshold {} {}
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

  $this configure -scope "all"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  set thresh [vtkImageThreshold New]
  $thresh SetInput [$this getInputBackground]
  eval $thresh ThresholdBetween $range
  $thresh SetInValue [EditorGetPaintLabel $::Editor(singleton)]
  $thresh SetOutValue 0
  $thresh SetOutput [$this getOutputLabel]
  $thresh Update
  $thresh Delete

  $this postApply
}


itcl::body ThresholdEffect::preview {} {

  $this configure -scope "visible"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  #
  # make a lookup table where inside the threshold is opaque and colored
  # by the label color, while the background is transparent (black)
  #

  set color [::EditorGetPaintColor $::Editor(singleton)]
  set lut [vtkLookupTable New]
  $lut SetRampToLinear
  $lut SetNumberOfTableValues 2
  $lut SetTableRange 0 1
  $lut SetTableValue 0  0 0 0  0
  eval $lut SetTableValue 1  $color
  set map [vtkImageMapToRGBA New]
  $map SetOutputFormatToRGBA
  $map SetLookupTable $lut

  set thresh [vtkImageThreshold New]
  #TODO: this background has already been windowed - so range is not correct
  $thresh SetInput [$this getInputBackground]
  eval $thresh ThresholdBetween $range
  $thresh SetInValue 1
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToUnsignedChar
  $map SetInput [$thresh GetOutput]

  $map Update

  $o(cursorMapper) SetInput [$map GetOutput]
  $o(cursorActor) VisibilityOn

  $map Delete
  $lut Delete
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
  $o(range) SetParent [$this getOptionsFrame]
  $o(range) Create
  $o(range) SetLabelText "Min/Max for Threshold Paint"
  $o(range) SetWholeRange 0 2000
  $o(range) SetReliefToGroove
  $o(range) SetBalloonHelpString "Set the range of the background values that should be labeled."

  set range [[$this getInputBackground] GetScalarRange]
  eval $o(range) SetWholeRange $range
  foreach {lo hi} $range {}
  set lo [expr $lo + (0.5 * ($hi - $lo))]
  $o(range) SetRange $lo $hi

  pack [$o(range) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 


  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel threshold without applying to label map."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # use for painting button
  #
  set o(useForPainting) [vtkNew vtkKWPushButton]
  $o(useForPainting) SetParent [$this getOptionsFrame]
  $o(useForPainting) Create
  $o(useForPainting) SetText "Use For Paint"
  $o(useForPainting) SetBalloonHelpString "Transfer the current threshold settings to be used for Paint and Draw operations."
  pack [$o(useForPainting) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # an apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Apply current threshold settings to the label map."
  pack [$o(apply) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(range) AddObserver AnyEvent "after idle $this previewOptions"]
  lappend _observerRecords "$o(range) $tag"
  set tag [$o(apply) AddObserver AnyEvent "$this applyOptions"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(useForPainting) AddObserver AnyEvent "$this setPaintThreshold"]
  lappend _observerRecords "$o(useForPainting) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"
}

itcl::body ThresholdEffect::tearDownOptions { } {
  if { [info exists o(range)] } {
    foreach w "range apply useForPainting cancel" {
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
  $this previewOptions
  foreach te [itcl::find objects -class ThresholdEffect] {
    $te apply
  }
}

itcl::body ThresholdEffect::setPaintThreshold {} {

  set editor $::Editor(singleton)
  eval EditorSetPaintThreshold $editor [$o(range) GetRange]
  EditorSetPaintThresholdState $editor 1
}
