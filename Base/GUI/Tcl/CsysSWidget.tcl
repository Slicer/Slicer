
package require Itcl

#########################################################
#
if {0} { ;# comment

  CsysSWidget a class for slicer coordinate system widget


# TODO : 

}
#
#########################################################
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

    variable _startXYPosition "0 0"
    variable _currentXYPosition "0 0"
    variable _radius 15

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method createGlyph { {polyData ""} } {}
    method positionActors {} {}
    method highlight {} {}
    method getTransform {} {}
    method updateTransform {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body CsysSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]

  foreach a {origin cursor} {
    set o($a,axes) [vtkNew vtkPolyData]
    $this createGlyph $o($a,axes)
    set o($a,mapper) [vtkNew vtkPolyDataMapper2D]
    set o($a,actor) [vtkNew vtkActor2D]
    $o($a,mapper) SetInput $o($a,axes)
    $o($a,actor) SetMapper $o($a,mapper)
    [$renderWidget GetRenderer] AddActor2D $o($a,actor)
    lappend _actors $o($a,actor)
    puts "o($a,actor) is $o($a,actor)"
  }

  set size [[$renderWidget GetRenderWindow]  GetSize]
  foreach {w h} $size {}
  foreach d {w h} c {cx cy} { set $c [expr [set $d] / 2.0] }

  $o(origin,actor) SetPosition $cx $cy

  # matrix to store the transform value at the start of the interaction
  set o(_startMatrix) [vtkNew vtkMatrix4x4]
  
  $this processEvent

  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  set events {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent EnterEvent LeaveEvent} 
  foreach event $events { 
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent"]
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent"]
}

itcl::body CsysSWidget::destructor {} {

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

  set renderer [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetRenderer]
  foreach a $_actors {
    $renderer RemoveActor2D $a
  }
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body CsysSWidget::createGlyph { {polyData ""} } {
  # make a circle 
  if { $polyData == "" } {
    set polyData [vtkNew vtkPolyData]
  }
  set points [vtkPoints New]
  set lines [vtkCellArray New]
  $polyData SetPoints $points
  $polyData SetLines $lines
  set PI 3.1415926
  set TWOPI [expr $PI * 2]
  set PIoverFOUR [expr $PI / 4]
  set prevPoint ""
  set firstPoint ""
  for { set angle 0 } { $angle <= $TWOPI } { set angle [expr $angle + $PIoverFOUR] } {
    set x [expr $_radius * cos($angle)]
    set y [expr $_radius * sin($angle)]
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
  # make the last line in the circle
  set idList [vtkIdList New]
  $idList InsertNextId $p
  $idList InsertNextId $firstPoint
  $polyData InsertNextCell 3 $idList  ;# 3 is a VTK_LINE
  $idList Delete

  # make the axes
  set r2 [expr $_radius * 2.0]
  set p0 [$points InsertNextPoint -$r2 0 0]
  set p1 [$points InsertNextPoint  $r2 0 0]
  set p2 [$points InsertNextPoint  0 -$r2 0]
  set p3 [$points InsertNextPoint  0  $r2 0]
  set idList [vtkIdList New]
  $idList InsertNextId $p0
  $idList InsertNextId $p1
  $polyData InsertNextCell 3 $idList  ;# 3 is a VTK_LINE
  $idList Delete
  set idList [vtkIdList New]
  $idList InsertNextId $p2
  $idList InsertNextId $p3
  $polyData InsertNextCell 3 $idList  ;# 3 is a VTK_LINE
  $idList Delete

  $points Delete
  $lines Delete
  return $polyData
}

itcl::body CsysSWidget::getTransform {} {

  eval $this queryLayers $_currentXYPosition

  set node ""
  foreach layer {foreground background} {
    if { $_layers($layer,node) != "" } {
      set node $_layers($layer,node) 
      break
    }
  }
  
  if { $node == "" } { 
    return ""
  }

  # TODO: the scene should probably be a ivar of the SWidget
  return [$::slicer3::MRMLScene GetNodeByID [$node GetTransformNodeID]]
}


itcl::body CsysSWidget::updateTransform {} {

  set transformNode [$this getTransform]

  if { $transformNode == "" } {
    return
  }

  set startRAS [$this xyToRAS $_startXYPosition]
  set currentRAS [$this xyToRAS $_currentXYPosition]

  switch $_actionState {
    "translating" {
      foreach d {dr da ds} s $startRAS c $currentRAS {
        set $d [expr $c - $s]
      }

      set matrix [$transformNode GetMatrixTransformToParent]
      foreach d {dr da ds} i {0 1 2} {
        set start [$o(_startMatrix) GetElement $i 3]
        $matrix SetElement $i 3 [expr $start + [set $d]]
      }
      $transformNode Modified
    }
    "rotating" {
      set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
      set size [[$renderWidget GetRenderWindow]  GetSize]
      foreach {w h} $size {}
      foreach d {w h} c {cx cy} { set $c [expr [set $d] / 2.0] }
      set centerRAS [$this xyToRAS "$cx $cy"]
      set minusCenterRAS [CsysSWidget::scale -1 $centerRAS]

      set math [vtkMath New]

      # get the normalized vectors from the center to the start point and to the current point
      foreach d {vorigR vorigA vorigS} s $startRAS c $centerRAS { 
        set $d [expr $s - $c] 
      }
      set origLength [$math Norm $vorigR $vorigA $vorigS]
      foreach d {vorigR vorigA vorigS} { 
        set $d [expr [set $d] / $origLength] 
      }

      foreach d {vcurrR vcurrA vcurrS} cc $currentRAS c $centerRAS { 
        set $d [expr $cc - $c] 
      }
      set currLength [$math Norm $vcurrR $vcurrA $vcurrS]
      foreach d {vcurrR vcurrA vcurrS} { 
        set $d [expr [set $d] / $currLength] 
      }

      set dot [CsysSWidget::dot $vorigR $vorigA $vorigS  $vcurrR $vcurrA $vcurrS]
      set toTheRight [CsysSWidget::cross $vorigR $vorigA $vorigS  $vcurrR $vcurrA $vcurrS]

      set length [eval $math Norm $toTheRight]
      set angleW [expr asin( $length ) * [$math RadiansToDegrees]]

      if { $dot < 0 } {
        set angleW [expr 180. - $angleW]
      }

      foreach v $toTheRight n {normRightX normRightY normRightZ} {
        set $n [expr $v / $length]
      }

      set rotation [vtkTransform New]
      eval $rotation Translate $centerRAS
      $rotation RotateWXYZ $angleW $normRightX $normRightY $normRightZ
      eval $rotation Translate $minusCenterRAS

      set matrix [$transformNode GetMatrixTransformToParent] 
      $o(_startMatrix) Multiply4x4 [$rotation GetMatrix] $o(_startMatrix) $matrix
      $matrix Modified
      $transformNode Modified

      $math Delete
      $rotation Delete
    }
  }

}

# TODO: this should really be accessible from vtkMath
proc CsysSWidget::scale {sfactor v {v1 ""} {v2 ""} } {

  if { [llength $v] == 1 } {
    set v [list $v $v1 $v2]
  }
  return [list [expr $sfactor * [lindex $v 0]] \
               [expr $sfactor * [lindex $v 1]] \
               [expr $sfactor * [lindex $v 2]] ]
}

# TODO: this should really be accessible from vtkMath
proc CsysSWidget::cross {x0 x1 x2  y0 y1 y2} {

  set Zx [expr $x1 * $y2 - $x2 * $y1]
  set Zy [expr $x2 * $y0 - $x0 * $y2]
  set Zz [expr $x0 * $y1 - $x1 * $y0];
  return "$Zx $Zy $Zz"

}

# TODO: this should really be accessible from vtkMath
proc CsysSWidget::dot {x0 x1 x2  y0 y1 y2} {

  return [expr $x0 * $y0 + $x1 * $y1 + $x2 * $y2]

}


itcl::body CsysSWidget::positionActors { } {

  eval $o(cursor,actor) SetPosition $_currentXYPosition
}

itcl::body CsysSWidget::highlight { } {

  # for the origin
  switch $_actionState {
    "active" - "translating" - "rotating" {
      set property [$o(origin,actor) GetProperty]
      $property SetColor 1 1 0
    }
    default {
      set property [$o(origin,actor) GetProperty]
      $property SetColor .65 .65 0
    }
  }

  # for the cursor
  switch $_actionState {
    "translating" - "rotating" {
      set property [$o(cursor,actor) GetProperty]
      $property SetColor 1 0 0
      $property SetOpacity 0.5
      $o(cursor,actor) VisibilityOn
    }
    default {
      $o(cursor,actor) VisibilityOff
    }
  }

}

itcl::body CsysSWidget::processEvent { {caller ""} {event ""} } {


  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    return ;# some other widget wants these events
  }

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set interactor [$renderWidget GetRenderWindowInteractor]
  set _currentXYPosition [$interactor GetEventPosition]
  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $event {
    "EnterEvent" {
      set transformNode [$this getTransform]
      if { $transformNode != "" } {
        set _actionState "active"
        set _description "Left mouse to translate, Control-left mouse to rotate"
      }
    }
    "LeaveEvent" {
      set _actionState ""
      set _description ""
    }
    "LeftButtonPressEvent" {
      $sliceGUI SetGrabID $this
      set transformNode [$this getTransform]
      if { $transformNode != "" } {
        $::slicer3::MRMLScene SaveStateForUndo $transformNode
        $o(_startMatrix) DeepCopy [$transformNode GetMatrixTransformToParent]
        set _startXYPosition $_currentXYPosition
        if { [$_interactor GetControlKey] } {
          set _actionState "rotating"
        } else {
          set _actionState "translating"
        }
      }
    }
    "LeftButtonReleaseEvent" {
      set _actionState "active"
      $sliceGUI SetGrabID ""
    }
    "MouseMoveEvent" {
      switch $_actionState {
        "translating" - "rotating" {
          $this updateTransform
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

proc CsysSWidgetRemove {} {

  foreach csys [itcl::find objects -class CsysSWidget] {
    set sliceGUI [$csys cget -sliceGUI]
    itcl::delete object $csys
    [$sliceGUI GetSliceViewer] RequestRender
  }
}

proc CsysSWidgetAdd {} {

  CsysSWidgetRemove

  set n [[$::slicer3::SlicesGUI GetSliceGUICollection] GetNumberOfItems]
  for {set i 0} {$i < $n} {incr i} {
    set sliceGUI [$::slicer3::SlicesGUI GetSliceGUI $i]
    CsysSWidget #auto $sliceGUI 
  }
}
