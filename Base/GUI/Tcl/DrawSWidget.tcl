
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
    method processEvent { {caller ""} } {}
    method positionActors {} {}
    method apply {} {}
    method createPolyData {} {}
    method addPoint {r a s} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body DrawSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set o(xyPoints) [vtkNew vtkPoints]
  set o(rasPoints) [vtkNew vtkPoints]
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
  foreach event { LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent } {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "$this processEvent $sliceGUI"]
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "$this processEvent $node"]
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

if { 0 } {
  itcl::delete class DrawSWidget; source c:/pieper/bwh/slicer3/latest/slicer3/Base/GUI/Tcl/DrawSWidget.tcl; ::DrawSWidget::AddDraw

              itcl::body DrawSWidget::createPolyData {} {
                # make a single-polyline polydata
                set polyData [vtkNew vtkPolyData]
                # set lines [vtkCellArray New]

                $polyData SetPoints $o(xyPoints)
                # $polyData SetLines $lines
                #set polyLine [vtkPolyLine New]
                #$lines InsertNextCell $polyLine

                set lines [$polyData GetLines]
                set idArray [$lines GetData]
                $idArray InsertNextTuple1 0

                #$lines Delete
                #$polyLine Delete

                return $polyData
              }
}

itcl::body DrawSWidget::createPolyData {} {
  # make a single-polyline polydata

  set polyData [vtkNew vtkPolyData]
  $polyData SetPoints $o(xyPoints)

  set lines [vtkCellArray New]
  $polyData SetLines $lines
  set idArray [$lines GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0

  set polygons [vtkCellArray New]
  $polyData SetPolys $polygons
  set idArray [$polygons GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0

  $polygons Delete
  $lines Delete
  return $polyData
}

itcl::body DrawSWidget::addPoint {r a s} {

  set p [$o(rasPoints) InsertNextPoint $r $a $s]
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]
}

itcl::body DrawSWidget::apply {} {

  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  set p [$idArray GetTuple1 1]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]


  set idList [vtkIdList New]
  set size [$idArray GetNumberOfTuples]
  for {set i 1} {$i < $size} {incr i} {
    $idList InsertNextId [$idArray GetTuple1 $i]
  }
  [$o(polyData) GetPolys] InsertNextCell $idList
  $idList Delete

  set idList [vtkIdList New]
  set polygon [$o(polyData) GetCell 1]
  $polygon Triangulate $idList
  [$o(polyData) GetLines] InsertNextCell $idList
  $idList Delete


  $idArray Reset
  $idArray InsertNextTuple1 0

  puts $o(polyData)

  $this positionActors

  return

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
itcl::body DrawSWidget::positionActors { } {
  set rasToXY [vtkTransform New]
  $rasToXY SetMatrix [$_sliceNode GetXYToRAS]
  $rasToXY Inverse
  $o(xyPoints) Reset
  $rasToXY TransformPoints $o(rasPoints) $o(xyPoints)
  $rasToXY Delete
  $o(polyData) Modified
}

itcl::body DrawSWidget::processEvent { {caller ""} } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  if { $caller == $sliceGUI } {

    set grabID [$sliceGUI GetGrabID]
    if { ($grabID != "") && ($grabID != $this) } {
      return 
    }

    set event [$sliceGUI GetCurrentGUIEvent] 
    set ras [$this xyToRAS [$_interactor GetEventPosition]]

    switch $event {
      "LeftButtonPressEvent" {
        if { ! [$_interactor GetShiftKey] } {
          set _actionState "drawing"
          eval $this addPoint $ras
        } else {
          set _actionState ""
          $this apply
        }
        $sliceGUI SetGUICommandAbortFlag 1
        $sliceGUI SetGrabID $this
      }
      "MouseMoveEvent" {
        switch $_actionState {
          "drawing" {
            eval $this addPoint $ras
          }
          default {
            return
          }
        }
      }
      "LeftButtonReleaseEvent" {
        set _actionState ""
        $sliceGUI SetGrabID ""
        set _description ""
      }
      default {
        # other events...
      }
    }
  } else { 
    # events from the node... nothing particular to do
    # except the default update of the actors
  }

  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
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
