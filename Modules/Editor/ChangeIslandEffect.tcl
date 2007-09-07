
package require Itcl

#########################################################
#
if {0} { ;# comment

  ChangeIslandEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ChangeIslandEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ChangeIslandEffect] == "" } {

  itcl::class ChangeIslandEffect {

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
itcl::body ChangeIslandEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body ChangeIslandEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ChangeIslandEffect::processEvent { {caller ""} {event ""} } {

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

itcl::body ChangeIslandEffect::apply {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  set conn [vtkImageConnectivity New]
  $conn SetFunctionToChangeIsland
  $conn SetSeed $_layers(label,i) $_layers(label,j) $_layers(label,k) 
  $conn SetOutputLabel [EditorGetPaintLabel]
  $conn SetInput [$this getInputLabel]
  $conn SetOutput [$this getOutputLabel]

  $this setProgressFilter $conn "Change Island"
  [$this getOutputLabel] Update

  $conn Delete

  $this postApply
}

  
itcl::body ChangeIslandEffect::buildOptions {} {

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

itcl::body ChangeIslandEffect::tearDownOptions { } {
  foreach w "cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
