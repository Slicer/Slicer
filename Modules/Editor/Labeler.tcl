
package require Itcl

#########################################################
#
if {0} { ;# comment

  Labeler is an abstract superclass for slicer Editor effects that
  use an outliner to draw

  SWidget
  ^
  EffectSWidget
  ^
  Labeler
  ^^^
  DrawEffect
  PaintEffect
  LevelTracingEffect



# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             Labeler
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class Labeler] == "" } {

  itcl::class Labeler {

    inherit EffectSWidget

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    public variable thresholdPaint 0
    public variable thresholdMin 1
    public variable thresholdMax 1
    public variable drawOver 1
    public variable polygonDebugViewer 0

    variable _lastEventPoint ""
    variable _lastInsertSlice ""

    # methods
    method makeMaskImage {polyData} {}
    method applyMaskImage {polyData} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body Labeler::constructor {sliceGUI} {
}

itcl::body Labeler::destructor {} {
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body Labeler::makeMaskImage {polyData} {

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
  [$polyData GetPoints] Modified
  set bounds [$polyData GetBounds]
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

    
itcl::body Labeler::applyMaskImage {polyData} {

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  if { $_layers(label,node) == "" } {
    # if there's no label, we can't draw
    return
  }

  # first, close the polyline back to the first point
  set lines [$polyData GetLines]
  set idArray [$lines GetData]
  set p [$idArray GetTuple1 1]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]

  set maskResult [$this makeMaskImage $polyData]
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

  [$polyData GetPoints] Modified
  set bounds [$polyData GetBounds]
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
  $painter SetBackgroundImage [$this getInputBackground]
  $painter SetBackgroundIJKToWorld $backgroundIJKToRAS
  $painter SetWorkingImage [$this getInputLabel]
  $painter SetWorkingIJKToWorld $labelIJKToRAS
  $painter SetMaskImage $mask
  $painter SetMaskIJKToWorld $maskIJKToRAS
  $painter SetTopLeft $tl(i) $tl(j) $tl(k)
  $painter SetTopRight $tr(i) $tr(j) $tr(k)
  $painter SetBottomLeft $bl(i) $bl(j) $bl(k)
  $painter SetBottomRight $br(i) $br(j) $br(k)
  $painter SetPaintLabel [EditorGetPaintLabel]
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

  return
}
