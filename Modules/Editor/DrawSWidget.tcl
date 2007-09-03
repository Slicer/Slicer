
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
    public variable thresholdPaint 0
    public variable thresholdMin 1
    public variable thresholdMax 1
    public variable drawOver 1
    public variable polygonDebugViewer 0

    variable _lastEventPoint ""
    variable _lastInsertSlice ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method positionActors {} {}
    method makeMaskImage {} {}
    method apply {} {}
    method createPolyData {} {}
    method resetPolyData {} {}
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
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  foreach event { LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent } {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI"]
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]
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

itcl::body DrawSWidget::resetPolyData {} {
  # return the polyline to initial state with no points
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0
  $o(xyPoints) Reset
  $o(rasPoints) Reset
  $lines SetNumberOfCells 0

  set _lastInsertSlice ""
}

itcl::body DrawSWidget::addPoint {r a s} {

  # store modify time so these points can be cleared if 
  # slice plane is moved
  set logic [$sliceGUI GetLogic]
  set _lastInsertSlice [$logic GetSliceOffset]

  set p [$o(rasPoints) InsertNextPoint $r $a $s]
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]
  $lines SetNumberOfCells 1
}

itcl::body DrawSWidget::makeMaskImage {} {

  #
  # use the slicer2-based vtkImageFillROI filter
  #

  #
  # Need to know the mapping from RAS into polygon space
  # so the painter can use this as a mask
  # - need the bounds in RAS space
  # - need to get an IJKToRAS for just the mask area
  # - directions are the XYToRAS for this slice
  # - origin is the lower left of the polygon bounds
  # - TODO: need to account for the boundary pixels
  #
  set maskIJKToRAS [vtkMatrix4x4 New]
  $maskIJKToRAS DeepCopy [$_sliceNode GetXYToRAS]
  $o(xyPoints) Modified
  set xyBounds [$o(xyPoints) GetBounds]
  foreach {xlo xhi ylo yhi zlo zhi} $xyBounds {}
  set xlo [expr $xlo - 1]
  set ylo [expr $ylo - 1]
  set originRAS [$this xyToRAS "$xlo $ylo"]
  $maskIJKToRAS SetElement 0 3  [lindex $originRAS 0]
  $maskIJKToRAS SetElement 1 3  [lindex $originRAS 1]
  $maskIJKToRAS SetElement 2 3  [lindex $originRAS 2]

  #
  # get a good size for the draw buffer 
  # - needs to include the full region of the polygon
  # - plus a little extra 
  #
  [$o(polyData) GetPoints] Modified
  set bounds [$o(polyData) GetBounds]
  foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
  # round to int and add extra pixel for both sides
  # -- TODO: figure out why we need to add buffer pixels on each 
  #    side for the width in order to end up with a single extra
  #    pixel in the rasterized image map.  Probably has to 
  #    do with how boundary conditions are handled in the filler
  set w [expr int($xhi - $xlo) + 32]
  set h [expr int($yhi - $ylo) + 32]

  set imageData [vtkImageData New]
  $imageData SetDimensions $w $h 1

  if { $_layers(label,image) != "" } {
    $imageData SetScalarType [$_layers(label,image) GetScalarType]
  }
  $imageData AllocateScalars

  #
  # move the points so the lower left corner of the 
  # bounding box is at 1, 1 (to avoid clipping)
  #
  set translate [vtkTransform New]
  $translate Translate [expr 2 + -1. * $xlo] [expr 1 + -1. * $ylo] 0
  set drawPoints [vtkPoints New]
  $drawPoints Reset
  $translate TransformPoints $o(xyPoints) $drawPoints
  $translate Delete
  $drawPoints Modified

  set fill [vtkImageFillROI New]
  $fill SetInput $imageData
  $fill SetValue 1
  $fill SetPoints $drawPoints
  [$fill GetOutput] Update

  set mask [vtkImageData New]
  $mask DeepCopy [$fill GetOutput]

  if { $polygonDebugViewer } {
    #
    # make a little preview window for debugging pleasure
    #
    catch "viewer Delete"
    catch "viewerImage Delete"
    vtkImageViewer viewer
    vtkImageData viewerImage
    viewerImage DeepCopy [$fill GetOutput]
    viewer SetInput viewerImage
    viewer SetColorWindow 2
    viewer SetColorLevel 1
    viewer Render
  }


  #
  # clean up our local class instances
  #
  $fill Delete
  $imageData Delete
  $drawPoints Delete

  return [list $maskIJKToRAS $mask]
}

    
itcl::body DrawSWidget::apply {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { $_layers(label,node) == "" } {
    # if there's no label, we can't draw
    return
  }

  # first, close the polyline back to the first point
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  set p [$idArray GetTuple1 1]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]

  # make sure the xyPoints are up to date
  $this positionActors

  set maskResult [$this makeMaskImage]
  foreach {maskIJKToRAS mask} $maskResult {}

  #
  # at this point, the mask vtkImageData contains a rasterized
  # version of the polygon and now needs to be added to the label
  # image
  #


  #
  # get the brush bounding box in ijk coordinates
  # - get the xy bounds
  # - transform to ijk
  # - clamp the bounds to the dimensions of the label image
  #

  [$o(polyData) GetPoints] Modified
  set bounds [$o(polyData) GetBounds]
  foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
  set xyToIJK [[$_layers(label,logic) GetXYToIJKTransform] GetMatrix]
  set tlIJK [$xyToIJK MultiplyPoint $xlo $yhi 0 1]
  set trIJK [$xyToIJK MultiplyPoint $xhi $yhi 0 1]
  set blIJK [$xyToIJK MultiplyPoint $xlo $ylo 0 1]
  set brIJK [$xyToIJK MultiplyPoint $xhi $ylo 0 1]

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
  # (use the maskToRAS calculated above)
  #
  set backgroundIJKToRAS [vtkMatrix4x4 New]
  $_layers(background,node) GetIJKToRASMatrix $backgroundIJKToRAS
  set labelIJKToRAS [vtkMatrix4x4 New]
  $_layers(label,node) GetIJKToRASMatrix $labelIJKToRAS

  #
  # set up the painter class and let 'r rip!
  #
  set painter [vtkImageSlicePaint New]
  $painter SetBackgroundImage $_layers(background,image)
  $painter SetBackgroundIJKToWorld $backgroundIJKToRAS
  $painter SetWorkingImage $_layers(label,image)
  $painter SetWorkingIJKToWorld $labelIJKToRAS
  $painter SetMaskImage $mask
  $painter SetMaskIJKToWorld $maskIJKToRAS
  $painter SetTopLeft $tl(i) $tl(j) $tl(k)
  $painter SetTopRight $tr(i) $tr(j) $tr(k)
  $painter SetBottomLeft $bl(i) $bl(j) $bl(k)
  $painter SetBottomRight $br(i) $br(j) $br(k)
  $painter SetPaintLabel $drawColor
  $painter SetPaintOver $drawOver
  $painter SetThresholdPaint $thresholdPaint
  $painter SetThresholdPaintRange $thresholdMin $thresholdMax

  $painter Paint

  $painter Delete
  $labelIJKToRAS Delete
  $backgroundIJKToRAS Delete
  $maskIJKToRAS Delete
  $mask Delete

  # TODO maybe just call $sliceGUI Render for faster update
  # and call this on mouse up - more important for paint
  $_layers(label,node) Modified

  #
  # remove the polygon from the draw buffer
  #
  after 100 "$this resetPolyData"

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

itcl::body DrawSWidget::processEvent { {caller ""} {event ""} } {

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
        eval $this addPoint $ras
        if { ! [$_interactor GetShiftKey] } {
          set _actionState "drawing"
        } else {
          set _actionState ""
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
# TODO - hot key to apply?
$this apply
        set _actionState ""
        $sliceGUI SetGrabID ""
        set _description ""
      }
      default {
        # other events...
      }
    }
  } 

  if { $caller == $_sliceNode } {
    # 
    # make sure all points are on the current slice plane
    # - if the SliceToRAS has been modified, then we're on a different plane
    #
    set logic [$sliceGUI GetLogic]
    set currentSlice [$logic GetSliceOffset]
    if { $_lastInsertSlice != $currentSlice } {
      $this resetPolyData
    }
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
