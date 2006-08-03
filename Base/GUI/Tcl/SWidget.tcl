
package provide SlicerBaseGUITcl 3.0

#########################################################
#
if {0} { ;# comment

  SWidget a class for collecting information about a slicer widget 
  including it's vtk class instances and it's interaction
  state.

  Parent class of other SWidgets

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
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable state ""  ;# the interaction state of the SWidget
    public variable description ""  ;# a status string describing the current state
    public variable sliceGUI ""  ;# the sliceGUI on which the SWidget lives

    variable _vtkObjects ""
    variable _pickState "outside"
    variable _actionState ""
    variable _glyphScale 10

    variable o ;# array of the objects for this widget, for convenient access
    variable _actors "" ;# list of actors for removing from the renderer
    variable _guiObserverTags ;# save so destructor can remove observer
    variable _nodeObserverTags ;# save so destructor can remove observer

    # parts of the sliceGUI saved for easy access
    variable _renderWidget ""
    variable _renderer ""
    variable _interactor ""
    variable _annotation ""
    variable _sliceNode ""
    variable _sliceCompositeNode ""
    variable _layers

    # methods
    method rasToXY {rasPoint} {}
    method xyToRAS {xyPoint} {}
    method queryLayers { x y } {}
    method getLayers {} {return [array get _layers]}
    method getObjects {} {return [array get o]}
    method processEvent {} {}
    method pick {} {}
    method highlight {} {}
    method place {x y z} {}
    method getPixel {image i j k} {}
    method setPixel {image i j k value} {}
    method setPixelBlock {image i j k size value} {}

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

#
# called on 'widget configure -sliceGUI gui'
# - caches handles to useful objects
#
itcl::configbody SWidget::sliceGUI {
  set _renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set _renderer [$_renderWidget GetRenderer]
  set _interactor [$_renderWidget GetRenderWindowInteractor]
  set _annotation [$_renderWidget GetCornerAnnotation]
  set _sliceNode [[$sliceGUI GetLogic] GetSliceNode]
  set _sliceCompositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
}

# return x y for a give r a s
itcl::body SWidget::rasToXY { rasPoint } {
  set rasToXY [vtkMatrix4x4 New]
  $rasToXY DeepCopy [$_sliceNode GetXYToRAS]
  $rasToXY Invert
  set xyzw [eval $rasToXY MultiplyPoint $rasPoint 1]
  $rasToXY Delete
  return [lrange $xyzw 0 1]
}

# return r a s for a given x y
itcl::body SWidget::xyToRAS { xyPoint } {
  set rast [eval [$_sliceNode GetXYToRAS] MultiplyPoint $xyPoint 0 1]
  return [lrange $rast 0 2]
}

itcl::body SWidget::queryLayers { x y } {
  # 
  # get the logic, node, image, ijk coords, and pixel for each layer
  # - store these in a layers array for easy access
  #
  foreach layer {background foreground label} {
    set _layers($layer,logic) [[$sliceGUI GetLogic]  Get[string totitle $layer]Layer]
    set _layers($layer,node) [$_layers($layer,logic) GetVolumeNode]
    set _layers($layer,image) [$_layers($layer,node) GetImageData]

    set _layers($layer,xyToIJK) [[$_layers($layer,logic) GetXYToIJKTransform] GetMatrix]
    foreach {i j k l} [$_layers($layer,xyToIJK) MultiplyPoint $x $y 0 1] {}
    foreach v {i j k} { ;# cast to integer
      set _layers($layer,$v) [expr int(round([set $v]))]
    }
    set _layers($layer,ijk) "i j k"
    set _layers($layer,pixel) [$this getPixel $_layers($layer,image) \
                    $_layers($layer,i) $_layers($layer,j) $_layers($layer,k)]
  }
}

itcl::body SWidget::getPixel { image i j k } {
  if { $image == "" } { return "" }
  foreach index "i j k" dimension [$image GetDimensions] {
    set ind [set $index]
    if { $ind < 0 || $ind >= $dimension } {return "Unknown"}
  }
  set n [$image GetNumberOfScalarComponents]
  for {set c 0} {$c < $n} {incr c} {
    lappend pixel [$image GetScalarComponentAsDouble $i $j $k $c]
  }
  return $pixel
}

itcl::body SWidget::setPixelBlock { image i j k size value } {
  for {set ii [expr $i - $size]} {$ii <= [expr $i + $size]} {incr ii} {
    for {set jj [expr $j - $size]} {$jj <= [expr $j + $size]} {incr jj} {
      for {set kk [expr $k - $size]} {$kk <= [expr $k + $size]} {incr kk} {
        SliceViewerSetPixel $image $ii $jj $kk $value
      }
    }
  }
}

itcl::body SWidget::setPixel { image i j k value } {
  foreach index "i j k" dimension [$image GetDimensions] {
    set ind [set $index]
    if { $ind < 0 || $ind >= $dimension } {return -1}
  }
  set n [$image GetNumberOfScalarComponents]
  for {set c 0} {$c < $n} {incr c} {
    set v [lindex $value $c]
    if { $v != "" } { 
      $image SetScalarComponentFromDouble $i $j $k $c $v
      $image Modified
    }
  }
  return 0
}
