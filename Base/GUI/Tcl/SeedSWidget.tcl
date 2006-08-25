
package require Itcl

#########################################################
#
if {0} { ;# comment

  SeedSWidget a class for slicer fiducials in 2D


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             SeedSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SeedSWidget] == "" } {

  itcl::class SeedSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    variable _startPosition "0 0 0"
    variable _currentPosition "0 0 0"

    # methods
    method processEvent {} {}
    method positionActors {} {}
    method pick {} {}
    method place {x y z} {}
    method highlight {} {}
    method createGlyph {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body SeedSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
 
  set o(cross) [$this createGlyph]
  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInput $o(cross)
  $o(actor) SetMapper $o(mapper)
  [$renderWidget GetRenderer] AddActor2D $o(actor)
  lappend _actors $o(actor)

  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"

  $this processEvent

  set _guiObserverTags ""
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "itcl::delete object $this"]
  foreach event {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent} {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "$this processEvent"]    }
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "$this processEvent"]
}

itcl::body SeedSWidget::destructor {} {

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

itcl::body SeedSWidget::createGlyph {} {
  # make a star shaped array of lines around the center
  set polyData [vtkNew vtkPolyData]
  set points [vtkPoints New]
  set lines [vtkCellArray New]
  $polyData SetPoints $points
  $polyData SetLines $lines
  set PI 3.1415926
  set TWOPI [expr $PI * 2]
  set PIoverFOUR [expr $PI / 4]
  for { set angle 0 } { $angle <= $TWOPI } { set angle [expr $angle + $PIoverFOUR] } {
    set x [expr $_glyphScale * 0.3 * cos($angle)]
    set y [expr $_glyphScale * 0.3 * sin($angle)]
    set p0 [$points InsertNextPoint $x $y 0]
    set x [expr $_glyphScale * cos($angle)]
    set y [expr $_glyphScale * sin($angle)]
    set p1 [$points InsertNextPoint $x $y 0]
    set idList [vtkIdList New]
    $idList InsertNextId $p0
    $idList InsertNextId $p1
    $polyData InsertNextCell 3 $idList
    $idList Delete
  }
  $points Delete
  $lines Delete
  return $polyData
}

itcl::body SeedSWidget::pick {} {

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set interactor [$renderWidget GetRenderWindowInteractor]

  foreach {x y} [$this rasToXY $_currentPosition] {}
  foreach {ex ey} [$interactor GetEventPosition] {}
  if { [expr abs($ex - $x) < 15] && [expr abs($ey - $y) < 15] } {
    set _pickState "over"
  } else {
    set _pickState "outside"
  }
}

itcl::body SeedSWidget::place {x y z} {
  set _currentPosition "$x $y $z"
  $this positionActors
}

itcl::body SeedSWidget::positionActors { } {

  set xyzw [$this rasToXY $_currentPosition]
  eval $o(actor) SetPosition [lrange $xyzw 0 1]
}

itcl::body SeedSWidget::highlight { } {

  set property [$o(actor) GetProperty]
  $property SetColor 1 0 0
  $property SetLineWidth 1
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
        }
      }
    }
  }
}

itcl::body SeedSWidget::processEvent { } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionActors
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  if { $_actionState != "dragging" } {
    # only check pick if we haven't grabbed (avoid 'dropping' the widget
    # when the mouse moves quickly)
    $this pick
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

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
              set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]
            }
            default {
            }
          }
        }
        "LeftButtonReleaseEvent" {
          set _actionState ""
          $sliceGUI SetGrabID ""
          set _description ""
        }
      }
    }
  }

  $this highlight
  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

proc SeedSWidget::ManyWidgetTest { sliceGUI } {

  set s 0
  for { set r -95. } { $r <= 95. } { set r [expr $r + 20] } {
    puts [time {
      for { set a -95. } { $a <= 95. } { set a [expr $a + 20] } {
        set seedSWidget [SeedSWidget #auto $sliceGUI]
        $seedSWidget place $r $a $s
      }
    }]
  }
}

