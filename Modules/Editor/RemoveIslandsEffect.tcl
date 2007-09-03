
package require Itcl

#########################################################
#
if {0} { ;# comment

  RemoveIslandsEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             RemoveIslandsEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class RemoveIslandsEffect] == "" } {

  itcl::class RemoveIslandsEffect {

    inherit EffectSWidget

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
itcl::body RemoveIslandsEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body RemoveIslandsEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body RemoveIslandsEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

  switch $event {
    "LeftButtonPressEvent" {
      $this apply
      $sliceGUI SetGUICommandAbortFlag 1
    }
    "EnterEvent" {
      $o(cursorActor) VisibilityOn
    }
    "LeaveEvent" {
      $o(cursorActor) VisibilityOff
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body RemoveIslandsEffect::apply {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  set conn [vtkImageConnectivity New]
  $conn SetFunctionToRemoveIslands
  $conn SetSeed $_layers(label,i) $_layers(label,j) $_layers(label,k) 
  $conn SetInput [$this getInputLabel]
  $conn SetOutput [$this getOutputLabel]
  $this setProgressFilter $conn "Remove Islands"
  [$this getOutputLabel] Update
  $conn Delete

  $this postApply
}

  
itcl::body RemoveIslandsEffect::buildOptions {} {

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
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Need to have a label layer to use island effect"
    after idle ::EffectSWidget::RemoveAll
  }
}

itcl::body RemoveIslandsEffect::tearDownOptions { } {
  foreach w "cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
