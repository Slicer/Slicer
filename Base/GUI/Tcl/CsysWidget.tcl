
package require Itcl

#########################################################
#
if {0} { ;# comment

  a class for collecting information about a slicer widget 
  including it's vtk class instances and it's interaction
  state

# TODO : 

}
#
#########################################################


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SWidget] == "" } {

  itcl::class SWidget {

    constructor {} {
      # make a unique name associated with this object
      set _name [namespace tail $this]
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable state ""  ;# the interaction state of the SWidget
    public variable description ""  ;# a status string describing the current state
    public variable sliceGUI ""  ;# the sliceGUI on which the SWidget lives

    variable _name ""
    variable _vtkObjects ""
    variable _pickState "outside"
    variable _actionState ""
    variable _glyphScale 10

    variable o ;# array of the objects for this widget, for convenient cleanup
    variable _actors "" ;# list of actors for removing from the renderer
    variable _guiObserverTag ;# save so destructor can remove observer
    variable _nodeObserverTag ;# save so destructor can remove observer

    # methods
    method getObjects {} {return [array get o]}
    method processEvent {} {}
    method pick {} {}
    method highlight {} {}
    method place {x y z} {}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this SWidget
    method vtkDelete {} {
      foreach object $_vtkObjects {
        $object Delete
      }
      set _vtkObjects ""
    }

  }
}

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
    method rasToXY {rasPoint} {}
    method xyToRAS {xyPoint} {}
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
  set _guiObserverTag [$sliceGUI AddObserver AnyEvent "$this processEvent"]
  set node [[$sliceGUI GetLogic] GetSliceNode]
  set _nodeObserverTag [$node AddObserver ModifiedEvent "$this processEvent"]
}

itcl::body SeedSWidget::destructor {} {

  $sliceGUI RemoveObserver $_guiObserverTag
  set node [[$sliceGUI GetLogic] GetSliceNode]
  $node RemoveObserver $_nodeObserverTag

  set renderer [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetRenderer]
  foreach a $_actors {
    $renderer RemoveActor2D $a
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


  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    return ;# some other widget wants these events
  }
  if { $grabID != $this } {
    # only check pick if we haven't grabbed (avoid 'dropping' the widget
    # when the mouse moves quickly)
    $this pick
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $_pickState {
    "outside" {
      set _actionState ""
    }
    "over" {
      $sliceGUI SetGUICommandAbortFlag 1
      switch $event {
        "LeftButtonPressEvent" {
          set _actionState "dragging"
          $sliceGUI SetGrabID $this
        }
        "MouseMoveEvent" {
          switch $_actionState {
            "dragging" {
              set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
              set interactor [$renderWidget GetRenderWindowInteractor]
              set _currentPosition [$this xyToRAS [$interactor GetEventPosition]]
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


# return x y for a give r a s
itcl::body SeedSWidget::rasToXY { rasPoint } {
  set node [[$sliceGUI GetLogic] GetSliceNode]
  set rasToXY [vtkMatrix4x4 New]
  $rasToXY DeepCopy [$node GetXYToRAS]
  $rasToXY Invert
  set xyzw [eval $rasToXY MultiplyPoint $rasPoint 1]
  $rasToXY Delete
  return [lrange $xyzw 0 1]
}

# return r a s for a given x y
itcl::body SeedSWidget::xyToRAS { xyPoint } {
  set node [[$sliceGUI GetLogic] GetSliceNode]
  set rast [eval [$node GetXYToRAS] MultiplyPoint $xyPoint 0 1]
  return [lrange $rast 0 2]
}



# ------------------------------------------------------------------
#                             CsysSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class CsysSWidget] == "" } {

  itcl::class CsysSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    variable _startPosition "0 0 0"
    variable _currentPosition "0 0 0"

    # methods
    method processEvent {} {}
    method positionActors {} {}
    method pick {} {}
    method highlight {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body CsysSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
 
  set o(sphere) [vtkNew vtkSphereSource]
  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInput [$o(sphere) GetOutput]
  $o(actor) SetMapper $o(mapper)
  [$renderWidget GetRenderer] AddActor2D $o(actor)
  lappend _actors $o(actor)

  set size [[$renderWidget GetRenderWindow]  GetSize]
  foreach {w h} $size {}
  foreach d {w h} c {cx cy} { set $c [expr [set $d] / 2.0] }

  set _startPosition "$cx $cy 0"
  set _currentPosition "$cx $cy 0"

  $o(sphere) SetRadius 5
  
  $this processEvent
  set _guiObserverTag [$sliceGUI AddObserver AnyEvent "$this processEvent"]
}

itcl::body CsysSWidget::destructor {} {

  $sliceGUI RemoveObserver $_guiObserverTag

  set renderer [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetRenderer]
  foreach a $_actors {
    $renderer RemoveActor2D $a
  }
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body CsysSWidget::pick {} {

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set interactor [$renderWidget GetRenderWindowInteractor]

  foreach {cx cy cz} $_currentPosition {}
  foreach {ex ey} [$interactor GetEventPosition] {}
  if { [expr abs($ex - $cx) < 15] && [expr abs($ey - $cy) < 15] } {
    set _pickState "over"
  } else {
    set _pickState "outside"
  }

}

itcl::body CsysSWidget::positionActors { } {
  eval $o(sphere) SetCenter $_currentPosition
}

itcl::body CsysSWidget::highlight { } {

  set property [$o(actor) GetProperty]
  $property SetColor 1 1 1
  switch $_actionState {
    "dragging" {
      $property SetColor 0 1 0
    }
    default {
      switch $_pickState {
        "over" {
          $property SetColor 0 1 1
        }
      }
    }
  }
}

itcl::body CsysSWidget::processEvent { } {


  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    return ;# some other widget wants these events
  }
  if { $grabID != $this } {
    # only check pick if we haven't grabbed (avoid 'dropping' the widget
    # when the mouse moves quickly)
    $this pick
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $_pickState {
    "outside" {
      set _actionState ""
      set _description ""
    }
    "over" {
      $sliceGUI SetGUICommandAbortFlag 1
      switch $event {
        "LeftButtonPressEvent" {
          set _actionState "dragging"
          set _description "Move mouse with left button down to drag"
          $sliceGUI SetGrabID $this
        }
        "MouseMoveEvent" {
          switch $_actionState {
            "dragging" {
              set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
              set interactor [$renderWidget GetRenderWindowInteractor]
              set _currentPosition "[$interactor GetEventPosition] 0"
            }
            default {
              set _description "Press left mouse button to begin dragging"
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





# ------------------------------------------------------------------
#    slicer interface -- create and remove the widgets
# ------------------------------------------------------------------

proc CsysWidgetRemove {} {

  foreach csys [itcl::find objects -class CsysSWidget] {
    set sliceGUI [$csys cget -sliceGUI]
    itcl::delete object $csys
    [$sliceGUI GetSliceViewer] RequestRender
  }
}

proc CsysWidgetAdd {} {

  CsysWidgetRemove

  set n [[$::slicer3::SlicesGUI GetSliceGUICollection] GetNumberOfItems]
  for {set i 0} {$i < $n} {incr i} {
    set sliceGUI [$::slicer3::SlicesGUI GetSliceGUI $i]
    CsysSWidget #auto $sliceGUI 
  }
}

