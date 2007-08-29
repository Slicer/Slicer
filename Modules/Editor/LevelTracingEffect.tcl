
package require Itcl

#########################################################
#
if {0} { ;# comment

  LevelTracingEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             LevelTracingEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class LevelTracingEffect] == "" } {

  itcl::class LevelTracingEffect {

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
itcl::body LevelTracingEffect::constructor {sliceGUI} {
}

itcl::body LevelTracingEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body LevelTracingEffect::processEvent { } {

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
      if { [info exists o(tracingActor)] } {
       $o(tracingActor) VisibilityOn
      }
    }
    "LeaveEvent" {
      $o(cursorActor) VisibilityOff
      if { [info exists o(tracingActor)] } {
       $o(tracingActor) VisibilityOff
      }
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body LevelTracingEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  $this postApply
}

itcl::body LevelTracingEffect::preview {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { ![info exists o(ijkToXY)] } {
    set o(tracingFilter) [vtkNew vtkITKLevelTracingImageFilter]
    set o(ijkToXY) [vtkNew vtkTransform]
    $o(ijkToXY) Inverse
    set o(tracingTransformFilter) [vtkNew vtkTransformPolyDataFilter]
    set o(tracingPolyData) [vtkNew vtkPolyData]
    set o(tracingMapper) [vtkNew vtkPolyDataMapper2D]
    set o(tracingActor) [vtkNew vtkActor2D]
    $o(tracingActor) SetMapper $o(tracingMapper)
    $o(tracingTransformFilter) SetInput $o(tracingPolyData)
    $o(tracingTransformFilter) SetTransform $o(ijkToXY)
    $o(tracingMapper) SetInput [$o(tracingTransformFilter) GetOutput]
    [$_renderWidget GetRenderer] AddActor2D $o(tracingActor)
    lappend _actors $o(tracingActor)
  }

  set $o(tracingFilter) [vtkITKLevelTracingImageFilter New]
  $o(tracingFilter) SetInput [$this getInputBackground]
  $o(tracingFilter) SetOutput $o(tracingPolyData)
  $o(tracingFilter) SetSeed $_layers(background,i) $_layers(background,j) $_layers(background,k) 

  # figure out which plane to use
  foreach {i0 j0 k0 l0} [$_layers(background,xyToIJK) MultiplyPoint $x $y 0 1] {}
  set x1 [expr $x + 1]; set y1 [expr $y + 1]
  foreach {i1 j1 k1 l1} [$_layers(background,xyToIJK) MultiplyPoint $x1 $y1 0 1] {}
  if { $i0 == $i1 } { $o(tracingFilter) SetPlaneToJK; puts jk }
  if { $j0 == $j1 } { $o(tracingFilter) SetPlaneToIK; puts ik }
  if { $k0 == $k1 } { $o(tracingFilter) SetPlaneToIJ; puts ij }

  $o(ijkToXY) SetMatrix $_layers(background,xyToIJK)
  $o(tracingTransformFilter) Update


  puts "-------- ijk points "

  set points [$o(tracingPolyData) GetPoints]
  set pts [$o(tracingPolyData) GetNumberOfPoints]
  for {set p 0} {$p < $pts} {incr p} {
    set pt [$points GetPoint $p]
    puts -nonewline "$pt   "
  }
  puts "\n"


  puts "-------- xy points "

  set points [[$o(tracingTransformFilter) GetOutput] GetPoints]
  set pts [[$o(tracingTransformFilter) GetOutput] GetNumberOfPoints]
  for {set p 0} {$p < $pts} {incr p} {
    set pt [$points GetPoint $p]
    puts -nonewline "$pt   "
  }
  puts "\n\n"


}
  
itcl::body LevelTracingEffect::buildOptions {} {

  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel level tracing without applying to label map."
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

itcl::body LevelTracingEffect::tearDownOptions { } {
  foreach w "cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
