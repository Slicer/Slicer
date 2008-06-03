
package require Itcl

#########################################################
#
if {0} { ;# comment

  SaveIslandEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             SaveIslandEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SaveIslandEffect] == "" } {

  itcl::class SaveIslandEffect {

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
itcl::body SaveIslandEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body SaveIslandEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body SaveIslandEffect::processEvent { {caller ""} {event ""} } {

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

itcl::body SaveIslandEffect::apply {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { [$this getInputLabel] == "" || [$this getInputLabel] == "" } {
    $this flashCursor 3
    return
  }

  set conn [vtkImageConnectivity New]
  $conn SetFunctionToSaveIsland
  $conn SetSeed $_layers(label,i) $_layers(label,j) $_layers(label,k) 
  $conn SetInput [$this getInputLabel]
  $conn SetOutput [$this getOutputLabel]
  $this setProgressFilter $conn "Save Island"
  [$this getOutputLabel] Update
  $conn Delete

  $this postApply
}

  
itcl::body SaveIslandEffect::buildOptions {} {

  #
  # a help button
  #
  set o(help) [vtkNew vtkSlicerPopUpHelpWidget]
  $o(help) SetParent [$this getOptionsFrame]
  $o(help) Create
  $o(help) SetHelpTitle "Save Islands"
  $o(help) SetHelpText "Click on an island you want to keep.  All voxels not connected to the island are set to zero."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

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

itcl::body SaveIslandEffect::tearDownOptions { } {
  foreach w "help cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
