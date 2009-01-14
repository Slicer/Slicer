package require Itcl

#########################################################
#
if {0} { ;# comment

  ModelSWidget a class for slicer fiducials in 2D


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ModelSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ModelSWidget] == "" } {

  itcl::class ModelSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable modelID ""
    public variable movedCommand ""
    public variable movingCommand ""
    public variable opacity "0.5"
    public variable visibility "1"
    public variable text ""
    public variable textScale "1"

    variable _startOffset "0 0 0"
    variable _currentPosition "0 0 0"
    variable _modelNode ""
    variable _modelNodeObservation ""
    variable _modelDisplayNodeObservation ""
    variable _sliceCompositeNode ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method positionActors {} {}
    method pick {} {}
    method place {x y z} {}
    method setRASPosition {r a s} { $this place $x $y $z }
    method getRASPosition {} { return $_currentPosition }
    method getPickState {} { return $_pickState }
    method highlight {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body ModelSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set o(cutter) [vtkNew vtkCutter]
  set o(plane) [vtkNew vtkPlane]
  $o(cutter) SetCutFunction $o(plane)
  set o(cutTransform) [vtkNew vtkTransform]
  set o(cutTransformFilter) [vtkNew vtkTransformPolyDataFilter]
  $o(cutTransformFilter) SetInputConnection [$o(cutter) GetOutputPort]
  $o(cutTransformFilter) SetTransform $o(cutTransform)

  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInputConnection [$o(cutTransformFilter) GetOutputPort]
  $o(actor) SetMapper $o(mapper)
  set _renderer [$_renderWidget GetRenderer]
  $_renderer AddActor2D $o(actor)
  lappend _actors $o(actor)

  set o(textActor) [vtkNew vtkActor2D]
  set o(textMapper) [vtkNew vtkTextMapper]
  $o(textActor) SetMapper $o(textMapper)
  $_renderer AddActor2D $o(textActor)
  set textProperty [$o(textMapper) GetTextProperty]
  $textProperty ShadowOn
  lappend _actors $o(textActor)

  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"
  set _sliceCompositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
  $this configure -visibility [$_sliceCompositeNode GetSliceIntersectionVisibility]

  $this processEvent

  # observe the slice GUI for user input events
  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"
  foreach event {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent} {
    $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }

  # observe the slice node for direct manipulations of MRML
  set node [[$sliceGUI GetLogic] GetSliceNode]
  $::slicer3::Broker AddObservation $node DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $node AnyEvent "::SWidget::ProtectedCallback $this processEvent $node AnyEvent"

  # observe the composite node for slice plane visibility requests
  $::slicer3::Broker AddObservation $_sliceCompositeNode DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $_sliceCompositeNode AnyEvent "::SWidget::ProtectedCallback $this processEvent $_sliceCompositeNode AnyEvent"

}

itcl::body ModelSWidget::destructor {} {

  $o(cutTransformFilter) SetInput ""
  $o(cutTransformFilter) SetTransform ""

  if { [info command $_renderer] != "" } {
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
  }

}

#
# when told what model to observe...
#
itcl::configbody ModelSWidget::modelID {
  # find the model node
  set modelNode [$::slicer3::MRMLScene GetNodeByID $modelID]
  if { $modelNode == "" } {
    error "no node for id $modelID"
  }
  set displayNode [$modelNode GetDisplayNode]
  if { $displayNode == "" } {
    error "no display node for id $modelID"
  }

  # remove observation from old node and add to new node
  # then set input to pipeline
  if { $modelNode != $_modelNode } {
    if { $_modelNodeObservation != "" } {
      $::slicer3::Broker RemoveObservation $_modelNodeObservation
    }
    if { $_modelDisplayNodeObservation != "" } {
      $::slicer3::Broker RemoveObservation $_modelDisplayNodeObservation
    }
    set _modelNode $modelNode
    if { $_modelNode != "" } {
      $o(cutter) SetInput [$_modelNode GetPolyData]
      set _modelNodeObservation [$::slicer3::Broker AddObservation $_modelNode AnyEvent "::SWidget::ProtectedCallback $this processEvent $_modelNode AnyEvent"]
      if { $displayNode != "" } {
        set _modelDisplayNodeObservation [$::slicer3::Broker AddObservation $displayNode AnyEvent "::SWidget::ProtectedCallback $this processEvent $displayNode AnyEvent"]
      }
    }
  }

  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody ModelSWidget::opacity {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody ModelSWidget::visibility {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody ModelSWidget::text {
  $o(textMapper) SetInput $text
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody ModelSWidget::textScale {
  set textProperty [$o(textMapper) GetTextProperty]
  set fontSize [expr round(2.5 * $textScale)]
  $textProperty SetFontSize $fontSize
  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ModelSWidget::pick {} {

  foreach {x y z} [$this rasToXYZ $_currentPosition] {}
  foreach {wx wy} [$_interactor GetEventPosition] {}
  foreach {ex ey ez} [$this dcToXYZ $wx $wy] {}
  if { [expr abs($ex - $x) < 15] && [expr abs($ey - $y) < 15] } {
    set _pickState "over"
    set _startOffset [list [expr $x - $ex] [expr $y - $ey] [expr $z - $ez]]
  } else {
    set _pickState "outside"
  }
}

itcl::body ModelSWidget::place {x y z} {
  set _currentPosition "$x $y $z"
  $this positionActors
}

itcl::body ModelSWidget::positionActors { } {

  $o(actor) SetPosition 0 0
  return

  # determine the xyz location of the fiducial
  set xyzw [$this rasToXYZ $_currentPosition]
  foreach {x y z w} $xyzw {}
  $o(actor) SetPosition $x $y
  $o(textActor) SetPosition $x $y

  # determine which renderer based on z position
  set k [expr int($z + 0.5)]

  # remove the seed from the old renderer and add it to the new one
  if { [info command $_renderer] != ""} {
    $_renderer RemoveActor2D $o(actor)
    $_renderer RemoveActor2D $o(textActor)
  }

  if { $k >= 0 && $k < [$_renderWidget GetNumberOfRenderers] } {
    set _renderer [$_renderWidget GetNthRenderer $k]
    if { [info command $_renderer] != ""} {
      $_renderer AddActor2D $o(actor)
      $_renderer AddActor2D $o(textActor)
    }
  }
}

itcl::body ModelSWidget::highlight { } {

  set property [$o(actor) GetProperty]
  set textProperty [$o(textMapper) GetTextProperty]

  $o(actor) SetVisibility $visibility
  $o(textActor) SetVisibility $visibility

  #
  # TODO: Map Model name to color (hard coded for slice models, otherwise
  # extracted from the display node)
  #
  set color "0.5 0.5 0.5"
  set modelNode [$::slicer3::MRMLScene GetNodeByID $modelID]
  if { $modelNode != "" } {
    set displayNode [$modelNode GetDisplayNode]
    if { $displayNode != "" } {
      set color [$displayNode GetColor]
    }
  }

  eval $property SetColor $color
  eval $textProperty SetColor $color
  $property SetLineWidth 3
  $property SetOpacity $opacity
  $textProperty SetOpacity $opacity

  return

  set _description ""
  switch $_actionState {
    "dragging" {
      $property SetColor 0 1 0
      set _description "Move mouse with left button down to drag"
    }
    default {
      switch $_pickState {
        "over" {
          $property SetColor 1 1 0
          $property SetLineWidth 2
          set _description "Move mouse with left button down to drag"
        }
      }
    }
  }
}

itcl::body ModelSWidget::processEvent { {caller ""} {event ""} } {


  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  # control visibility based on ModelDisplayNode
  if { $_modelNode != "" } { 
    $o(cutter) SetInput [$_modelNode GetPolyData]
    set displayNode [$_modelNode GetDisplayNode]
    if { $caller == $displayNode } {
      $this configure -visibility [$displayNode GetSliceIntersectionVisibility]
    }
  }

  #
  # update the transform from world to screen space
  # for the extracted cut plane
  #
  $this queryLayers 0 0 0
  set rasToXY [vtkMatrix4x4 New]
  $rasToXY DeepCopy [$_sliceNode GetXYToRAS]
  $rasToXY Invert
  $o(cutTransform) SetMatrix $rasToXY
  $rasToXY Delete

  #
  # update the plane equation for the current slice cutting plane
  # - extract from the slice matrix
  # - normalize the normal
  #
  foreach row {0 1 2} {
    lappend normal [[$_sliceNode GetXYToRAS] GetElement $row 2]
    lappend origin [[$_sliceNode GetXYToRAS] GetElement $row 3]
  }
  set sum 0.
  foreach ele $normal {
    set sum [expr $sum + $ele * $ele]
  }
  set lenInv [expr 1./sqrt($sum)]
  foreach ele $normal {
    lappend norm [expr $ele * $lenInv]
  }
  eval $o(plane) SetNormal $norm
  eval $o(plane) SetOrigin $origin

  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionActors
    $o(cutter) Modified
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  if { $_actionState != "dragging" } {
    # only check pick if we haven't grabbed (avoid 'dropping' the widget
    # when the mouse moves quickly)
    $this pick
  }


  switch $_pickState {
    "outside" {
      # when mouse isn't over us, we don't do anything
      set _actionState ""
      $sliceGUI SetGrabID ""
    }
    "over" {
      # when mouse is over us, we pay attention to the
      # event and tell others not to look at it
      $sliceGUI SetGUICommandAbortFlag 1
      switch $event {
        "LeftButtonPressEvent" {
          set _actionState "dragging"
          $sliceGUI SetGrabID $this
        }
        "MouseMoveEvent" {
          switch $_actionState {
            "dragging" {
              foreach {wx wy} [$_interactor GetEventPosition] {}
              foreach {ex ey ez} [$this dcToXYZ $wx $wy] {}
              foreach {dx dy dz} $_startOffset {}
              set newxyz [list [expr $ex + $dx] [expr $ey + $dy] [expr $ez + $dz]]
              set _currentPosition [$this xyzToRAS $newxyz]
              eval $movingCommand
            }
          }
        }
        "LeftButtonReleaseEvent" {
          set _actionState ""
          $sliceGUI SetGrabID ""
          set _description ""
          eval $movedCommand
        }
      }
    }
  }

  $this highlight
  $this positionActors
  $o(cutter) Modified
  [$sliceGUI GetSliceViewer] RequestRender
}
