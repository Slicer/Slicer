
package require Itcl

#########################################################
#
if {0} { ;# comment

  Labeler is an abstract superclass for slicer Editor effects that
  use an outliner or applies a label map to a slice

  SWidget
  ^
  EffectSWidget
  ^
  Labeler
  ^^^
  DrawEffect
  PaintEffect
  LevelTracingEffect
  ...



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

    public variable paintThreshold 0
    public variable paintThresholdMin 1
    public variable paintThresholdMax 1
    public variable paintOver 1
    public variable polygonDebugViewer 0

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method makeMaskImage {polyData} {}
    method applyPolyMask {polyData} {}
    method applyImageMask { maskIJKToRAS mask bounds } {}
    method undoLastApply {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method setMRMLDefaults {} {}
    method updateMRMLFromGUI {} {}
    method updateGUIFromMRML {} {}
    method rotateSliceToImage {} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body Labeler::constructor {sliceGUI} {
  set o(painter) [vtkNew vtkImageSlicePaint]
  $this rotateSliceToImage
}

itcl::body Labeler::destructor {} {
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body Labeler::processEvent { {caller ""} {event ""} } {
  
  # don't chain this method - superclass method is meant 
  # to be overridden and not chained.

  if { $caller != "" && [$caller IsA "vtkKWWidget"] } {
    $this updateMRMLFromGUI
  }

  if { $caller != "" && [$caller IsA "vtkMRMLNode"] } {
    $this updateGUIFromMRML
  }

}


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
  [$polyData GetPoints] Modified
  set bounds [$polyData GetBounds]
  foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
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
  $translate TransformPoints [$polyData GetPoints] $drawPoints
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

    
#
# rasterize a polyData (closed list of points) 
# into the label map layer
# - points are specified in current XY space
#
itcl::body Labeler::applyPolyMask {polyData} {

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

  [$polyData GetPoints] Modified
  set bounds [$polyData GetBounds]

  $this applyImageMask $maskIJKToRAS $mask $bounds

}


#
# apply a pre-rasterized image to the current label layer
# - maskIJKToRAS tells the mapping from image pixels to RAS
# - mask is a vtkImageData
# - bounds are the xy extents of the mask (zlo and zhi ignored)
#
itcl::body Labeler::applyImageMask { maskIJKToRAS mask bounds } {

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

  foreach {xlo xhi ylo yhi zlo zhi} $bounds {}
  set xyToIJK [[$_layers(label,logic) GetXYToIJKTransform] GetMatrix]
  set tlIJK [$xyToIJK MultiplyPoint $xlo $yhi 0 1]
  set trIJK [$xyToIJK MultiplyPoint $xhi $yhi 0 1]
  set blIJK [$xyToIJK MultiplyPoint $xlo $ylo 0 1]
  set brIJK [$xyToIJK MultiplyPoint $xhi $ylo 0 1]

  # do the clamping
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
  # create an exract image for undo if it doesn't exist yet
  #
  if { ![info exists o(extractImage)] } {
    set o(extractImage) [vtkNew vtkImageData]
  }

  #
  # set up the painter class and let 'r rip!
  #
  $o(painter) SetBackgroundImage [$this getInputBackground]
  $o(painter) SetBackgroundIJKToWorld $backgroundIJKToRAS
  $o(painter) SetWorkingImage [$this getInputLabel]
  $o(painter) SetWorkingIJKToWorld $labelIJKToRAS
  $o(painter) SetMaskImage $mask
  $o(painter) SetExtractImage $o(extractImage)
  $o(painter) SetReplaceImage ""
  $o(painter) SetMaskIJKToWorld $maskIJKToRAS
  $o(painter) SetTopLeft $tl(i) $tl(j) $tl(k)
  $o(painter) SetTopRight $tr(i) $tr(j) $tr(k)
  $o(painter) SetBottomLeft $bl(i) $bl(j) $bl(k)
  $o(painter) SetBottomRight $br(i) $br(j) $br(k)
  $o(painter) SetPaintLabel [EditorGetPaintLabel]
  $o(painter) SetPaintOver $paintOver
  $o(painter) SetThresholdPaint $paintThreshold
  $o(painter) SetThresholdPaintRange $paintThresholdMin $paintThresholdMax

  $o(painter) Paint

  $labelIJKToRAS Delete
  $backgroundIJKToRAS Delete
  $maskIJKToRAS Delete
  $mask Delete

  # TODO maybe just call $sliceGUI Render for faster update
  # and call this on mouse up - more important for paint
  $_layers(label,node) Modified

  return
}

# use the 'save under' information from last paint apply
# to restore the original value of the working volume
# - be careful can only call this after when the painter class
#   is valid (e.g. after an apply but before changing any of the volumes)
#   it should be crash-proof in any case, but may generated warnings
# - if extract image doesn't exists, failes silently
itcl::body Labeler::undoLastApply { } {
  if { [info exists o(extractImage)] } {
    $o(painter) SetReplaceImage $o(extractImage)
    $o(painter) Paint
  }
}


itcl::body Labeler::buildOptions { } {

  $this setMRMLDefaults

  set o(paintOver) [vtkKWCheckButtonWithLabel New]
  $o(paintOver) SetParent [$this getOptionsFrame]
  $o(paintOver) Create
  $o(paintOver) SetLabelText "Paint Over: "
  $o(paintOver) SetBalloonHelpString "Allow effect to overwrite non-zero labels."
  pack [$o(paintOver) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  set o(paintThreshold) [vtkKWCheckButtonWithLabel New]
  $o(paintThreshold) SetParent [$this getOptionsFrame]
  $o(paintThreshold) Create
  $o(paintThreshold) SetLabelText "Threshold Painting: "
  $o(paintThreshold) SetBalloonHelpString "Enable/Disable threshold mode for labeling."
  pack [$o(paintThreshold) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 


  set o(paintRange) [vtkKWRange New]
  $o(paintRange) SetParent [$this getOptionsFrame]
  $o(paintRange) Create
  $o(paintRange) SetLabelText "Threshold"
  $o(paintRange) SetReliefToGroove
  $o(paintRange) SetBalloonHelpString "In threshold mode, the label will only be set if the background value is within this range."
  pack [$o(paintRange) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  if { [$this getInputBackground] != "" } {
    set range [[$this getInputBackground] GetScalarRange]
    eval $o(paintRange) SetWholeRange $range
  }

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [[$o(paintOver) GetWidget] AddObserver AnyEvent "$this processEvent $o(paintOver)"]
  lappend _observerRecords "$o(paintOver) $tag"
  set tag [[$o(paintThreshold) GetWidget] AddObserver AnyEvent "$this processEvent $o(paintThreshold)"]
  lappend _observerRecords "$o(paintThreshold) $tag"
  set tag [$o(paintRange) AddObserver AnyEvent "$this processEvent $o(paintRange)"]
  lappend _observerRecords "$o(paintRange) $tag"

}

itcl::body Labeler::tearDownOptions { } {
  if { [info exists o(paintOver)] } {
    foreach w "paintOver paintThreshold paintRange" {
      if { [info exists o($w)] } {
        $o($w) SetParent ""
        pack forget [$o($w) GetWidgetName] 
      }
    }
  }
}

itcl::body Labeler::updateMRMLFromGUI { } {
  #
  # set the node to the current value of the GUI
  # - this will be saved/restored with the scene
  # - all instances of the effect are observing the node,
  #   so changes will propogate automatically
  #
  chain
  set node [EditorGetParameterNode]
  $this configure -paintOver [[$o(paintOver) GetWidget] GetSelectedState]
  $node SetParameter "Labeler,paintOver" $paintOver
  $this configure -paintThreshold [[$o(paintThreshold) GetWidget] GetSelectedState]
  $node SetParameter "Labeler,paintThreshold" $paintThreshold
  foreach {Min Max} [$o(paintRange) GetRange] {}
  $this configure -paintThresholdMin $Min
  $node SetParameter "Labeler,paintThresholdMin" $Min
  $this configure -paintThresholdMax $Max
  $node SetParameter "Labeler,paintThresholdMax" $Max
}

itcl::body Labeler::setMRMLDefaults { } {
  chain
  set node [EditorGetParameterNode]
  foreach {param default} {
    paintOver 1
    paintThreshold 0
    paintThresholdMin 0
    paintThresholdMax 1000
  } {
    set pvalue [$node GetParameter Labeler,$param] 
    if { $pvalue == "" } {
      $node SetParameter Labeler,$param $default
    } 
  }
}

itcl::body Labeler::updateGUIFromMRML { } {

  #
  # get the gui to the parameter from the node
  #
  chain

  set node [EditorGetParameterNode]

  # set the GUI and effect parameters to match node
  # (only if this is the instance that "owns" the GUI
  set paintOver [$node GetParameter "Labeler,paintOver"]
  $this configure -paintOver $paintOver
  if { [info exists o(paintOver)] } {
    if { [[$o(paintOver) GetWidget] GetSelectedState] != $paintOver } {
      [$o(paintOver) GetWidget] SetSelectedState $paintOver
    }
  }
  set paintThreshold [$node GetParameter "Labeler,paintThreshold"]
  $this configure -paintThreshold $paintThreshold
  if { [info exists o(paintThreshold)] } {
    if { [[$o(paintThreshold) GetWidget] GetSelectedState] != $paintThreshold } {
      [$o(paintThreshold) GetWidget] SetSelectedState $paintThreshold
    }
  }

  set paintThresholdMin [$node GetParameter "Labeler,paintThresholdMin"]
  $this configure -paintThresholdMin $paintThresholdMin
  set paintThresholdMax [$node GetParameter "Labeler,paintThresholdMax"]
  $this configure -paintThresholdMax $paintThresholdMax
  if { [info exists o(paintRange)] } {
    if { [$o(paintRange) GetRange] != "$paintThresholdMin $paintThresholdMax" } {
      $o(paintRange) SetRange $paintThresholdMin $paintThresholdMax
    }
    $o(paintRange) SetEnabled $paintThreshold
  }
}

#
# rotateSliceToImage:
# - adjusts the slice node to align with the 
#   native space of the image data so that paint
#   operations can happen cleanly between image
#   space and screen space
#
itcl::body Labeler::rotateSliceToImage { } {

  $this queryLayers 0 0

  set ijkToRAS [vtkMatrix4x4 New]
  set rotation [vtkMatrix4x4 New]
  $rotation Identity

  set volumeNode $_layers(background,node)
  $volumeNode GetIJKToRASMatrix $ijkToRAS

  set M(init) 1

  for {set col 0} {$col < 3} {incr col} {
    set M($col,mag) 0
    for {set row 0} {$row < 3} {incr row} {
      set M($row,$col) [$ijkToRAS GetElement $row $col]
      set M($col,mag) [expr $M($row,$col) * $M($row,$col) + $M($col,mag)]
    }
    set M($col,mag) [expr sqrt($M($col,mag))]
    set M($col,mag_inv) [expr 1. / $M($col,mag)]
    for {set row 0} {$row < 3} {incr row} {
      set M(norm,$row,$col) [expr $M($col,mag_inv) * $M($row,$col)]
      $rotation SetElement $row $col $M(norm,$row,$col)
    }
  }

  if { $_sliceNode == "" } { 
    tk_messageBox -message "no slice node yet..."
  }
  set sliceToRAS [$_sliceNode GetSliceToRAS]

  puts [parray M]

  switch [$_sliceNode GetOrientationString] {
    "Axial" {
      $sliceToRAS SetElement 0 0 [expr  1.0 * $M(norm,0,0)]
      $sliceToRAS SetElement 1 0 [expr  1.0 * $M(norm,1,0)]
      $sliceToRAS SetElement 2 0 [expr  1.0 * $M(norm,2,0)]

      $sliceToRAS SetElement 0 1 [expr -1.0 * $M(norm,0,1)]
      $sliceToRAS SetElement 1 1 [expr -1.0 * $M(norm,1,1)]
      $sliceToRAS SetElement 2 1 [expr -1.0 * $M(norm,2,1)]

      $sliceToRAS SetElement 0 2 [expr  1.0 * $M(norm,0,2)]
      $sliceToRAS SetElement 1 2 [expr  1.0 * $M(norm,1,2)]
      $sliceToRAS SetElement 2 2 [expr  1.0 * $M(norm,2,2)]
    }
    "Sagittal" {
      $sliceToRAS SetElement 0 0 [expr  1.0 * $M(norm,0,1)]
      $sliceToRAS SetElement 1 0 [expr  1.0 * $M(norm,1,1)]
      $sliceToRAS SetElement 2 0 [expr  1.0 * $M(norm,2,1)]

      $sliceToRAS SetElement 0 1 [expr  1.0 * $M(norm,0,2)]
      $sliceToRAS SetElement 1 1 [expr  1.0 * $M(norm,1,2)]
      $sliceToRAS SetElement 2 1 [expr  1.0 * $M(norm,2,2)]

      $sliceToRAS SetElement 0 2 [expr  1.0 * $M(norm,0,0)]
      $sliceToRAS SetElement 1 2 [expr  1.0 * $M(norm,1,0)]
      $sliceToRAS SetElement 2 2 [expr  1.0 * $M(norm,2,0)]
    }
    "Coronal" {
      $sliceToRAS SetElement 0 0 [expr  1.0 * $M(norm,0,0)]
      $sliceToRAS SetElement 1 0 [expr  1.0 * $M(norm,1,0)]
      $sliceToRAS SetElement 2 0 [expr  1.0 * $M(norm,2,0)]

      $sliceToRAS SetElement 0 1 [expr  1.0 * $M(norm,0,2)]
      $sliceToRAS SetElement 1 1 [expr  1.0 * $M(norm,1,2)]
      $sliceToRAS SetElement 2 1 [expr  1.0 * $M(norm,2,2)]

      $sliceToRAS SetElement 0 2 [expr -1.0 * $M(norm,0,1)]
      $sliceToRAS SetElement 1 2 [expr -1.0 * $M(norm,1,1)]
      $sliceToRAS SetElement 2 2 [expr -1.0 * $M(norm,2,1)]
    }
  }

  after idle $_sliceNode Modified
}

namespace eval Labler {
  proc SetPaintRange {min max} {
    set node [EditorGetParameterNode]
    $node SetParameter Labeler,paintThresholdMin $min
    $node SetParameter Labeler,paintThresholdMax $max
  }
}
