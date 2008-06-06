
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
    method processEvent {{caller ""} {event ""}} {}
    method preview {} {}
    method apply {} {}
    method positionCursor {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method previewOptions {} {}
    method setAnimationState { p } {}
    method applyOptions {} {}
    method setPaintThreshold {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body ThresholdEffect::constructor {sliceGUI} {
  set _scopeOptions "all visible"
}

itcl::body ThresholdEffect::destructor {} {
  # rely on superclass destructor
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ThresholdEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }

}

itcl::body ThresholdEffect::apply {} {

  if { [$this getInputBackground] == "" || [$this getInputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  set thresh [vtkImageThreshold New]
  $thresh SetInput [$this getInputBackground]
  eval $thresh ThresholdBetween $range
  $thresh SetInValue [EditorGetPaintLabel]
  $thresh SetOutValue 0
  $thresh SetOutput [$this getOutputLabel]
  $thresh SetOutputScalarType [[$this getInputLabel] GetScalarType]
  $this setProgressFilter $thresh "Threshold"
  $thresh Update
  $thresh Delete

  $this postApply
}


itcl::body ThresholdEffect::preview {} {

  if { [$this getInputBackground] == "" || [$this getInputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    return
  }

  #
  # make a lookup table where inside the threshold is opaque and colored
  # by the label color, while the background is transparent (black)
  # - apply the threshold operation to the currently visible background
  #   (output of the layer logic's vtkImageReslice instance)
  #

  set color [::EditorGetPaintColor $::Editor(singleton)]
  set o(lut) [vtkNew vtkLookupTable]
  $o(lut) SetRampToLinear
  $o(lut) SetNumberOfTableValues 2
  $o(lut) SetTableRange 0 1
  $o(lut) SetTableValue 0  0 0 0  0
  eval $o(lut) SetTableValue 1  $color
  set map [vtkImageMapToRGBA New]
  $map SetOutputFormatToRGBA
  $map SetLookupTable $o(lut)

  set thresh [vtkImageThreshold New]
  set logic [[$sliceGUI GetLogic] GetBackgroundLayer]
  $thresh SetInput [[$logic GetReslice] GetOutput]
  eval $thresh ThresholdBetween $range
  $thresh SetInValue 1
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToUnsignedChar
  $map SetInput [$thresh GetOutput]

  $map Update

  $o(cursorMapper) SetInput [$map GetOutput]
  $o(cursorActor) VisibilityOn

  $map Delete
  $thresh Delete

  [$sliceGUI GetSliceViewer] RequestRender
}

#
# p will be a floating point number 
#
itcl::body ThresholdEffect::setAnimationState { p } {

  if { [info exists o(lut)] } {
    set amt [expr 0.5 + 0.25 * (1 + cos(6.2831852 * ($p - floor($p)))) ]
    set color [::EditorGetPaintColor $::Editor(singleton)]
    eval $o(lut) SetTableValue 1 [lreplace $color 3 3 $amt]
  }
}

itcl::body ThresholdEffect::positionCursor {} {
  foreach actor $_cursorActors {
    $actor SetPosition 0 0
  }
}

itcl::body ThresholdEffect::buildOptions { } {

  chain
  
  #
  # a range setting for threshold values
  #
  set o(range) [vtkNew vtkKWRange]
  $o(range) SetParent [$this getOptionsFrame]
  $o(range) Create
  $o(range) SetLabelText "Range"
  $o(range) SetReliefToGroove
  $o(range) SetBalloonHelpString "Set the range of the background values that should be labeled."

  if { [$this getInputBackground] != "" } {
    set range [[$this getInputBackground] GetScalarRange]
    eval $o(range) SetWholeRange $range
    foreach {lo hi} $range {}
    set lo [expr $lo + (0.25 * ($hi - $lo))]
    $o(range) SetRange $lo $hi
  }

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
  set tag [$o(apply) AddObserver AnyEvent "$this applyOptions; after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(useForPainting) AddObserver AnyEvent "$this setPaintThreshold"]
  lappend _observerRecords "$o(useForPainting) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getInputLabel] == "" } {
    $this errorDialog "Need to have background and label layers to use threshold"
    after idle ::EffectSWidget::RemoveAll
  }
}

itcl::body ThresholdEffect::tearDownOptions { } {
  chain
  if { [info exists o(range)] } {
    foreach w "range apply useForPainting cancel" {
      if { [info exists o($w)] } {
        $o($w) SetParent ""
        pack forget [$o($w) GetWidgetName] 
      }
    }
  }
}

itcl::body ThresholdEffect::previewOptions { } {
  foreach te [itcl::find objects -class ThresholdEffect] {
    $te configure -range [$o(range) GetRange]
    $te animateCursor off
    $te preview
    $te animateCursor on
  }
}

itcl::body ThresholdEffect::applyOptions { } {
  $this previewOptions
  set te [lindex [itcl::find objects -class ThresholdEffect] 0]
  $te apply
}

itcl::body ThresholdEffect::setPaintThreshold {} {

  eval EditorSetPaintThreshold [$o(range) GetRange]
  EditorSetPaintThresholdState 1
}
