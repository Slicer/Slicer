
package require Itcl

#########################################################
#
if {0} { ;# comment

  DrawSWidget a class for slicer drawing


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             DrawSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class DrawSWidget] == "" } {

  itcl::class DrawSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable drawColor 1
    public variable thresholdDraw 0
    public variable thresholdMin 1
    public variable thresholdMax 1
    public variable drawOver 1

    # methods
    method processEvent {} {}
    method positionActors {} {}
    method apply {} {}
    method createPolyData {} {}
    method addPoint {x y} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body DrawSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set o(polyData) [$this createPolyData]
  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInput $o(polyData)
  $o(actor) SetMapper $o(mapper)
  set property [$o(actor) GetProperty]
  $property SetColor 1 1 0
  $property SetLineWidth 1
  [$_renderWidget GetRenderer] AddActor2D $o(actor)
  lappend _actors $o(actor)

  $this processEvent

  set _guiObserverTags ""
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _guiObserverTags [$sliceGUI AddObserver AnyEvent "$this processEvent"]
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "$this processEvent"]
}

itcl::body DrawSWidget::destructor {} {

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

itcl::body DrawSWidget::createPolyData {} {
  # make a single-polyline polydata
  set polyData [vtkNew vtkPolyData]
  set points [vtkPoints New]
  set lines [vtkCellArray New]

  $polyData SetPoints $points
  $polyData SetLines $lines
  set polyLine [vtkPolyLine New]
  $lines InsertNextCell $polyLine

  $points Delete
  $lines Delete
  $polyLine Delete
  return $polyData
}

if { 0 } {
  itcl::delete class DrawSWidget; source c:/pieper/bwh/slicer3/latest/slicer3/Base/GUI/Tcl/DrawSWidget.tcl; ::DrawSWidget::AddDraw
}

itcl::body DrawSWidget::addPoint {x y} {

  set points [$o(polyData) GetPoints]
  set p [$points InsertNextPoint $x $y 0]
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]
  $o(polyData) Modified
}

itcl::body DrawSWidget::positionActors { } {

  return
  # TODO: update the polygon position 
  set xyzw [$this rasToXY $_currentPosition]
  eval $o(actor) SetPosition [lrange $xyzw 0 1]
}

itcl::body DrawSWidget::processEvent { } {

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
    $this positionActors
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $event {
    "LeftButtonPressEvent" {
      foreach {x y} [$_interactor GetEventPosition] {}
      $this addPoint $x $y
      set _actionState "drawing" ;# rubber band line
      $sliceGUI SetGUICommandAbortFlag 1
      $sliceGUI SetGrabID $this
    }
    "MouseMoveEvent" {
      switch $_actionState {
        "drawing" {
          foreach {x y} [$_interactor GetEventPosition] {}
          $this addPoint $x $y
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

  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body DrawSWidget::apply {} {

  #
  # draw with a brush that is circular in XY space 
  # (could be streched or rotate when transformed to IJK)
  # - make sure to hit ever pixel in IJK space 
  # - apply the threshold if selected
  #

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { $_layers(label,node) == "" } {
    # if there's no label, we can't draw
    return
  }

  #
  # get the brush bounding box in ijk coordinates
  # - get the xy bounds
  # - transform to ijk
  # - clamp the bounds to the dimensions of the label image
  #
  set bounds [[$o(brush) GetPoints] GetBounds]
  set left [expr $x + [lindex $bounds 0]]
  set right [expr $x + [lindex $bounds 1]]
  set bottom [expr $y + [lindex $bounds 2]]
  set top [expr $y + [lindex $bounds 3]]

  set xyToIJK [[$_layers(label,logic) GetXYToIJKTransform] GetMatrix]
  set tlIJK [$xyToIJK MultiplyPoint $left $top 0 1]
  set trIJK [$xyToIJK MultiplyPoint $right $top 0 1]
  set blIJK [$xyToIJK MultiplyPoint $left $bottom 0 1]
  set brIJK [$xyToIJK MultiplyPoint $right $bottom 0 1]

  set dims [$_layers(label,image) GetDimensions]
  foreach v {i j k} c [lrange $tlIJK 0 2] d $dims {
    set tl($v) [expr int(round($c))]
    if { $tl($v) < 0 } { set tl($v) 0 }
    if { $tl($v) >= $d } { set tl($v) [expr $d - 1] }
  }
  foreach v {i j k} c [lrange $trIJK 0 2] d $dims {
    set tr($v) [expr int(round($c))]
    if { $tr($v) < 0 } { set tr($v) 0 }
    if { $tr($v) >= $d } { set tr($v) [expr $d - 1] }
  }
  foreach v {i j k} c [lrange $blIJK 0 2] d $dims {
    set bl($v) [expr int(round($c))]
    if { $bl($v) < 0 } { set bl($v) 0 }
    if { $bl($v) >= $d } { set bl($v) [expr $d - 1] }
  }
  foreach v {i j k} c [lrange $brIJK 0 2] d $dims {
    set br($v) [expr int(round($c))]
    if { $br($v) < 0 } { set br($v) 0 }
    if { $br($v) >= $d } { set br($v) [expr $d - 1] }
  }


  #
  # get the ijk to ras matrices 
  #
  set backgroundIJKToRAS [vtkMatrix4x4 New]
  $_layers(background,node) GetIJKToRASMatrix $backgroundIJKToRAS
  set labelIJKToRAS [vtkMatrix4x4 New]
  $_layers(label,node) GetIJKToRASMatrix $labelIJKToRAS

  #
  # get the brush center and radius in World
  # - assume uniform scaling between XY and RAS
  set xyToRAS [$_sliceNode GetXYToRAS]
  set brushCenter [lrange [$xyToRAS MultiplyPoint $x $y 0 1] 0 2]
  set worldScale [lrange [$xyToRAS MultiplyPoint 1 1 1 0] 0 2]
  set scale 1
  foreach c $worldScale {
    if { $c != 1 } { 
      set scale $c
      break
    }
  }
  set brushRadius [expr $scale * $radius]

  #
  # set up the drawer class and let 'r rip!
  #
  set extractImage [vtkImageData New]
  set drawer [vtkImageSliceDraw New]
  $drawer SetBackgroundImage $_layers(background,image)
  $drawer SetBackgroundIJKToWorld $backgroundIJKToRAS
  $drawer SetWorkingImage $_layers(label,image)
  $drawer SetWorkingIJKToWorld $labelIJKToRAS
  $drawer SetTopLeft $tl(i) $tl(j) $tl(k)
  $drawer SetTopRight $tr(i) $tr(j) $tr(k)
  $drawer SetBottomLeft $bl(i) $bl(j) $bl(k)
  $drawer SetBottomRight $br(i) $br(j) $br(k)
  eval $drawer SetBrushCenter $brushCenter
  $drawer SetBrushRadius $brushRadius
  $drawer SetDrawLabel $drawColor
  $drawer SetDrawOver $drawOver
  $drawer SetThresholdDraw $thresholdDraw
  $drawer SetThresholdDrawRange $thresholdMin $thresholdMax

  $drawer Draw

  $drawer Delete
  $labelIJKToRAS Delete
  $backgroundIJKToRAS Delete

  $_layers(label,node) Modified


  return
}

proc DrawSWidget::AddDraw {} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      DrawSWidget #auto [$sw cget -sliceGUI]
    }
  }
}

proc DrawSWidget::RemoveDraw {} {
  foreach pw [itcl::find objects -class DrawSWidget] {
    itcl::delete object $pw
  }
}

proc DrawSWidget::ToggleDraw {} {
  if { [itcl::find objects -class DrawSWidget] == "" } {
    DrawSWidget::AddDraw
  } else {
    DrawSWidget::RemoveDraw
  }
}

proc DrawSWidget::ConfigureAll { args } {
  foreach pw [itcl::find objects -class DrawSWidget] {
    eval $pw configure $args
  }
}
