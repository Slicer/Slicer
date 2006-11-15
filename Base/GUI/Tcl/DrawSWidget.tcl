
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
    public variable drawMethod "fillROI"

    variable _lastEventPoint ""

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
  foreach event { LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent RightButtonPressEvent RightButtonReleaseEvent} {
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
  $lines SetNumberOfCells 1
}

    
itcl::body DrawSWidget::apply {} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  # first, close the polyline back to the first point
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  set p [$idArray GetTuple1 1]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]

  # make sure the xyPoints are up to date
  $this positionActors


  # switch on the public configuration variable
  switch $drawMethod {
    "fillROI" {

      #
      # use the slicer2 vtkImageFillROI filter
      #

      #
      # get a good size for the draw buffer 
      # - needs to include the full region of the polygon
      # - plus a little extra 
      #
      [$o(polyData) GetPoints] Modified
      set bounds [$o(polyData) GetBounds]
      foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
      # round to int and add extra pixel for both sides
      # -- TODO: figure out why we need to add two pixels on each 
      #    side for the width in order to end up with a single extra
      #    pixel in the rasterized image map.  Probably has to 
      #    do with how boundary conditions are handled in the filler
      set w [expr int($xhi - $xlo) + 4]
      set h [expr int($yhi - $ylo) + 2]

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

      #
      # remove the polygon from the draw buffer
      #
      $idArray Reset
      $idArray InsertNextTuple1 0
      $o(xyPoints) Reset
      $o(rasPoints) Reset
      $lines SetNumberOfCells 0

      #
      # clean up our local class instances
      #
      $fill Delete
      $imageData Delete
      $drawPoints Delete
    }

    "stencil" {

      #
      # use the extrusion/stencil approach from VTK/Examples/GUI/Tcl/ImageTracerWidget.tcl
      # - extrude the polygon into a 'skirt' along the z axis
      # - create a stencil from this polydata
      # - make an image data with 1 inside the polygon, 0 elsewhere
      #

      set extrude [vtkLinearExtrusionFilter New]
      $extrude SetInput $o(polyData)
      $extrude SetScaleFactor 1
      $extrude SetExtrusionTypeToVectorExtrusion
      $extrude SetVector 0 0 1

      set dataToStencil [vtkPolyDataToImageStencil New]
      $dataToStencil SetInputConnection [$extrude GetOutputPort]

      #
      # get a good size for the draw buffer 
      # - needs to include the full region
      #
      [$o(polyData) GetPoints] Modified
      set bounds [$o(polyData) GetBounds]
      foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
      set factor 2
      set w [expr $factor * (int($xhi - $xlo) + 2)]
      set h [expr $factor * (int($yhi - $ylo) + 2)]

      set imageData [vtkImageData New]
      $imageData SetSpacing [expr 1. / $factor] [expr 1. / $factor] 1
      $imageData SetDimensions $w $h 1
      $imageData SetOrigin [expr $xlo - 1.0001] [expr $ylo - 1.0001] 0
      if { $_layers(label,image) != "" } {
        $imageData SetScalarType [$_layers(label,image) GetScalarType]
      }
      $imageData AllocateScalars

      set oneImageData [vtkImageData New]
      
      set threshold [vtkImageThreshold New]
      $threshold SetInput $imageData
      $threshold SetOutValue 1
      $threshold SetInValue 1
      $threshold ThresholdBetween 1 -1
      $threshold SetOutput $oneImageData
      $oneImageData Update

      set stencil [vtkImageStencil New]
      $stencil SetInput $oneImageData
      $stencil SetStencil [$dataToStencil GetOutput]
      $stencil ReverseStencilOff
      $stencil SetBackgroundValue 0

      #
      # reverse stencil meaning if needed
      # - check if corners are 1, and if so re-execute the inverse
      # - just a heuristic, but pretty reliable
      #
      [$stencil GetOutput] Update
      set ll [[$stencil GetOutput] GetScalarComponentAsFloat 0 0 0 0]
      set ur [[$stencil GetOutput] GetScalarComponentAsFloat [expr $w-1] [expr $h-1] 0 0]
      if { $ll == 1 && $ur == 1 } {
        $stencil ReverseStencilOn
      }
      [$stencil GetOutput] Update

      #
      # make a little preview window for debugging pleasure
      #
      catch "viewer Delete"
      catch "viewerImage Delete"
      vtkImageViewer viewer
      vtkImageData viewerImage
      viewerImage DeepCopy [$stencil GetOutput]
      viewer SetInput viewerImage
      viewer SetColorWindow 2
      viewer SetColorLevel 1
      viewer Render

      #
      # remove the polygon from the draw buffer
      #
      $idArray Reset
      $idArray InsertNextTuple1 0
      $o(xyPoints) Reset
      $o(rasPoints) Reset
      $lines SetNumberOfCells 0

      #
      # clean up our local class instances
      #
      $imageData Delete
      $oneImageData Delete
      $threshold Delete
      $extrude Delete
      $dataToStencil Delete
    }
  }

  return;

  #
  # at this point 
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
        eval $this addPoint $ras
        if { ! [$_interactor GetShiftKey] } {
          set _actionState "drawing"
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
      "RightButtonPressEvent" {
        puts "got right down"
        set _lastEventPoint [$_interactor GetEventPosition]
      }
      "RightButtonReleaseEvent" {
        puts "got right up"
        puts "$_lastEventPoint == [$_interactor GetEventPosition]"
        if { $_lastEventPoint == [$_interactor GetEventPosition] } {
          eval $this addPoint $ras
          set _actionState ""
          $this apply
        }
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
