
package require Itcl

#########################################################
#
if {0} { ;# comment

  ImplicitRectangleEffect a class for slicer painting


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             ImplicitRectangleEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ImplicitRectangleEffect] == "" } {

  itcl::class ImplicitRectangleEffect {

    inherit Labeler

    constructor {sliceGUI} {Labeler::constructor $sliceGUI} {}
    destructor {}

    variable _startXYPosition "0 0"
    variable _currentXYPosition "0 0"
    variable _feedbackActors ""

    # methods

    # overridden virtuals
    method processEvent {{caller ""} {event ""}} {}
    method buildOptions {} {}
    method tearDownOptions {} {}

    # local methods
    method createGlyph { {polyData ""} } {}
    method updateGlyph { {polyData ""} } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------

itcl::body ImplicitRectangleEffect::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI

  # xyPoints is what the Labeler methods will use 
  # to rasterize
  set o(xyPoints) [vtkNew vtkPoints]
 
  # the actor is used to display the preview 
  # rubberband view of the region to label
  set o(rectangle) [vtkNew vtkPolyData]
  $this createGlyph $o(rectangle)
  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  set property [$o(actor) GetProperty]
  $property SetColor 1 1 0
  $property SetLineWidth 1
  $o(mapper) SetInput $o(rectangle)
  $o(actor) SetMapper $o(mapper)
  $o(actor) VisibilityOff
  [$_renderWidget GetRenderer] AddActor2D $o(actor)
  lappend _actors $o(actor)

  $this processEvent

  set _guiObserverTags ""
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  foreach event {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent EnterEvent LeaveEvent} {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent"]    }
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent"]
}

itcl::body ImplicitRectangleEffect::destructor {} {

  if { [info command $sliceGUI] != "" } {
    foreach tag $_guiObserverTags {
      $sliceGUI RemoveObserver $tag
    }
  }

  if { [info command $_sliceNode] != "" } {
    foreach tag $_nodeObserverTags {
      $_sliceNode RemoveObserver $tag
    }
  }

  if { [info command $_renderer] != "" } {
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
  }
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body ImplicitRectangleEffect::createGlyph { {polyData ""} } {

  # make a circle paint brush
  if { $polyData == "" } {
    set polyData [vtkNew vtkPolyData]
  }
  set points [vtkPoints New]
  set lines [vtkCellArray New]
  $polyData SetPoints $points
  $polyData SetLines $lines
  set prevPoint ""
  set firstPoint ""
  # TODO update positions
  foreach {x y} {0 0  0 0  0 0  0 0} {
    set p [$points InsertNextPoint $x $y 0]
    if { $prevPoint != "" } {
      set idList [vtkIdList New]
      $idList InsertNextId $prevPoint
      $idList InsertNextId $p
      $polyData InsertNextCell 3 $idList  ;# 3 is a VTK_LINE
      $idList Delete
    }
    set prevPoint $p
    if { $firstPoint == "" } {
      set firstPoint $p
    }
  }
  # make the last line in the polydata
  set idList [vtkIdList New]
  $idList InsertNextId $p
  $idList InsertNextId $firstPoint
  $polyData InsertNextCell 3 $idList  ;# 3 is a VTK_LINE
  $idList Delete

  $points Delete
  $lines Delete
  return $polyData
}

itcl::body ImplicitRectangleEffect::updateGlyph { {polyData ""} } {

  if { $polyData == "" } {
    # if called during construction...
    return
  }

  set points [$polyData GetPoints]

  foreach {xlo ylo} $_startXYPosition {}
  foreach {xhi yhi} $_currentXYPosition {}

  $points SetPoint 0 $xlo $ylo 0
  $points SetPoint 1 $xlo $yhi 0
  $points SetPoint 2 $xhi $yhi 0
  $points SetPoint 3 $xhi $ylo 0

}

itcl::body ImplicitRectangleEffect::processEvent { {caller ""} {event ""} } {

  # chain to superclass
  chain $caller $event

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  set grabID [$sliceGUI GetGrabID]
  if { ($grabID != "") && ($grabID != $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $event {
    "LeftButtonPressEvent" {
      set _actionState "dragging"
      set _startXYPosition [$_interactor GetEventPosition]
      set _currentXYPosition $_startXYPosition
      $this updateGlyph $o(rectangle)
      $sliceGUI SetGUICommandAbortFlag 1
      $sliceGUI SetGrabID $this
      [$_renderWidget GetRenderWindow] HideCursor
      set _description "Drag to define rectangular region to label"
    }
    "MouseMoveEvent" {
      switch $_actionState {
        "dragging" {
          $o(actor) VisibilityOn
          set _currentXYPosition [$_interactor GetEventPosition]
          $this updateGlyph $o(rectangle)
        }
        default {
          set _startXYPosition "0 0"
          set _currentXYPosition "0 0"
          $this updateGlyph $o(rectangle)
          $o(actor) VisibilityOff
        }
      }
    }
    "LeftButtonReleaseEvent" {
      [$_renderWidget GetRenderWindow] ShowCursor
      $this applyPolyMask $o(rectangle)
      set _actionState ""
      $sliceGUI SetGrabID ""
      set _description ""
    }
    "EnterEvent" {
      set _description "Ready to ImplicitRectangle!"
      $o(actor) VisibilityOn
    }
    "LeaveEvent" {
      set _description ""
      $o(actor) VisibilityOff
    }
  }

  [$sliceGUI GetSliceViewer] RequestRender
}

  
itcl::body ImplicitRectangleEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

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

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Label map needed for painting"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}


itcl::body ImplicitRectangleEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

