
package require Itcl

#########################################################
#
if {0} { ;# comment

  WandEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             WandEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class WandEffect] == "" } {

  itcl::class WandEffect {

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
itcl::body WandEffect::constructor {sliceGUI} {
}

itcl::body WandEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body WandEffect::processEvent { {caller ""} {event ""} } {

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
    "MouseMoveEvent" {
      $this preview
    }
    "EnterEvent" {
      $o(cursorActor) VisibilityOn
      if { [info exists o(wandActor)] } {
       $o(wandActor) VisibilityOn
      }
    }
    "LeaveEvent" {
      $o(cursorActor) VisibilityOff
      if { [info exists o(wandActor)] } {
       $o(wandActor) VisibilityOff
      }
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body WandEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  $this postApply
}

itcl::body WandEffect::preview {} {

  # 
  # get the event position to use as a seed
  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  #
  # create pipeline as needed
  if { ![info exists o(wandFilter)] } {
    set o(wandFilter) [vtkNew vtkITKWandImageFilter]
  }

  $o(wandFilter) SetInput [$this getInputBackground]
  $o(wandFilter) SetSeed $_layers(background,i) $_layers(background,j) $_layers(background,k) 
  set percent [$o(percentage) GetValue]
  $o(wandFilter) SetDynamicRangePercentage $percent

  # figure out which plane to use
  foreach {i0 j0 k0 l0} [$_layers(background,xyToIJK) MultiplyPoint $x $y 0 1] {}
  set x1 [expr $x + 1]; set y1 [expr $y + 1]
  foreach {i1 j1 k1 l1} [$_layers(background,xyToIJK) MultiplyPoint $x1 $y1 0 1] {}
  if { $i0 == $i1 } { $o(wandFilter) SetPlaneToJK }
  if { $j0 == $j1 } { $o(wandFilter) SetPlaneToIK }
  if { $k0 == $k1 } { $o(wandFilter) SetPlaneToIJ }

  $o(wandFilter) Update

  $_layers(label,node) SetAndObserveImageData [$o(wandFilter) GetOutput] 
}
  
itcl::body WandEffect::buildOptions {} {
  #
  # a slider to set the percentage of the dynamic range
  #
  set o(percentage) [vtkNew vtkKWScaleWithEntry]
  $o(percentage) SetParent [$this getOptionsFrame]
  $o(percentage) PopupModeOn
  $o(percentage) SetResolution 0.01
  $o(percentage) Create
  $o(percentage) SetRange 0.0 1.0
  $o(percentage) SetValue 0.1
  $o(percentage) SetLabelText "Dynamic range percentage"
  $o(percentage) SetBalloonHelpString "Set the percentage of the dynamic range to group with the seed (default 0.1)."
  pack [$o(percentage) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel wand without applying to label map."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
#  set tag [$o(percentage) AddObserver AnyEvent "after idle $this previewOptions"]
#  lappend _observerRecords "$o(percentage) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for wand"
    after idle ::EffectSWidget::RemoveAll
  }
}

itcl::body WandEffect::tearDownOptions { } {
  foreach w "percentage cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
