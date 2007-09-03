
package require Itcl

#########################################################
#
if {0} { ;# comment

  ChangeLabelEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ChangeLabelEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ChangeLabelEffect] == "" } {

  itcl::class ChangeLabelEffect {

    inherit EffectSWidget

    public variable inputLabel ""
    public variable outputLabel ""

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method preview {} {}
    method apply {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body ChangeLabelEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body ChangeLabelEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ChangeLabelEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }
}

itcl::body ChangeLabelEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  set in [[$o(colorIn) GetWidget] GetValue]
  set out [[$o(colorOut) GetWidget] GetValue]

  if { ![string is integer $in] || ![string is integer $out] } {
    $this errorDialog "Label values must be integers."
    return
  }

  set change [vtkImageLabelChange New]
  $change SetOutputLabel [EditorGetPaintLabel]
  $change SetInput [$this getInputLabel]
  $change SetOutput [$this getOutputLabel]
  $change SetInputLabel $in
  $change SetOutputLabel $out

  $this setProgressFilter $change "Change Label"
  [$this getOutputLabel] Update

  $change Delete

  $this postApply
}

  
itcl::body ChangeLabelEffect::buildOptions {} {

  #
  # a color button
  #
  set o(colorIn) [vtkNew vtkKWEntryWithLabel]
  $o(colorIn) SetParent [$this getOptionsFrame]
  $o(colorIn) Create
  $o(colorIn) SetLabelText "Input Color"
  $o(colorIn) SetBalloonHelpString "Set the color to replace."
  pack [$o(colorIn) GetWidgetName] \
    -anchor e -padx 2 -pady 2 

  set o(colorOut) [vtkNew vtkKWEntryWithLabel]
  $o(colorOut) SetParent [$this getOptionsFrame]
  $o(colorOut) Create
  $o(colorOut) SetLabelText "Output Color"
  [$o(colorOut) GetWidget] SetValue [EditorGetPaintLabel]
  $o(colorOut) SetBalloonHelpString "Set the new label value."
  pack [$o(colorOut) GetWidgetName] \
    -anchor e -padx 2 -pady 2 

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
  # a apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Change all instances of Input Color to Output Color in current label volume."
  pack [$o(apply) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(apply) AddObserver AnyEvent "$this apply"]
  lappend _observerRecords "$o(apply) $tag"

  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Need to have a label layer to use change label effect"
    after idle ::EffectSWidget::RemoveAll
  }
}

itcl::body ChangeLabelEffect::tearDownOptions { } {
  foreach w "colorIn colorOut apply cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
