
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
    method processEvent {} {}
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

itcl::body WandEffect::processEvent { } {

  if { [$this preProcessEvent] } {
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

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { ![info exists o(ijkToXY)] } {

    set o(wandFilter) [vtkNew vtkITKWandImageFilter]

    set o(wandMapper) [vtkNew vtkImageMapper]
    set o(wandActor) [vtkNew vtkActor2D]
    $o(wandActor) SetMapper $o(wandMapper)
    set property [$o(wandActor) GetProperty]
    $property SetColor 1 1 0
    [$_renderWidget GetRenderer] AddActor2D $o(wandActor)
    lappend _actors $o(wandActor)
  }

  set $o(wandFilter) [vtkITKWandImageFilter New]
  $o(wandFilter) SetInput [$this getInputBackground]
  $o(wandFilter) SetSeed $_layers(background,i) $_layers(background,j) $_layers(background,k) 

  # figure out which plane to use
  foreach {i0 j0 k0 l0} [$_layers(background,xyToIJK) MultiplyPoint $x $y 0 1] {}
  set x1 [expr $x + 1]; set y1 [expr $y + 1]
  foreach {i1 j1 k1 l1} [$_layers(background,xyToIJK) MultiplyPoint $x1 $y1 0 1] {}
  if { $i0 == $i1 } { $o(wandFilter) SetPlaneToJK }
  if { $j0 == $j1 } { $o(wandFilter) SetPlaneToIK }
  if { $k0 == $k1 } { $o(wandFilter) SetPlaneToIJ }

  $o(wandFilter) Update
  set image [$o(wandFilter) GetOutput]

  $o(wandMapper) SetInput $image
}
  
itcl::body WandEffect::buildOptions {} {

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
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Threshold"
    after idle ::EffectSWidget::RemoveAll
  }
}

itcl::body WandEffect::tearDownOptions { } {
  foreach w "cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
