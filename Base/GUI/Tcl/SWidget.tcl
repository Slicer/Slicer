
package require Itcl

package provide SlicerBaseGUITcl 3.0

# 
# utility to only delete an instance if it hasn't already been deleted
# (this is useful in event handling)
#
namespace eval SWidget {
  proc ProtectedDelete {instance} {
    if { [info command $instance] != "" } {
      itcl::delete object $instance
    }
  }
}

# 
# utility to run method only if instance hasn't already been deleted
# (this is useful in event handling)
#
namespace eval SWidget {
  proc ProtectedCallback {instance args} {
    if { [info command $instance] != "" } {
      eval $instance $args
    }
  }
}

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
    method rasToXYZ {rasPoint} {}
    method xyToRAS {xyPoint} {}
    method xyzToRAS {xyPoint} {}
    method dcToXYZ { x y } {}
    method queryLayers { x y {z 0} } {}
    method getLayers {} {return [array get _layers]}
    method getObjects {} {return [array get o]}
    method processEvent {{caller ""} {event ""}} {}
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
        catch "$object Delete"
      }
      set _vtkObjects ""
    }

    # generic useful routine
    method superclass {} {
      return [lindex [$this info heritage] 1]
    }

    # interact with the status line on the main window
    method statusText {msg} {
      [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText $msg
    }
  }
}

#
# called on 'widget configure -sliceGUI gui'
# - caches handles to useful objects
#
itcl::configbody SWidget::sliceGUI {
  set _renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
    
  #set numberOfRenderers

  #set _renderer [$_renderWidget GetRenderer]
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

# return x y z for a give r a s
itcl::body SWidget::rasToXYZ { rasPoint } {
  set rasToXY [vtkMatrix4x4 New]
  $rasToXY DeepCopy [$_sliceNode GetXYToRAS]
  $rasToXY Invert
  set xyzw [eval $rasToXY MultiplyPoint $rasPoint 1]
  $rasToXY Delete
  return [lrange $xyzw 0 2]
}

# return r a s for a given x y
itcl::body SWidget::xyToRAS { xyPoint } {
  set rast [eval [$_sliceNode GetXYToRAS] MultiplyPoint $xyPoint 0 1]
  return [lrange $rast 0 2]
}

# return r a s for a given x y z
itcl::body SWidget::xyzToRAS { xyPoint } {
  set rast [eval [$_sliceNode GetXYToRAS] MultiplyPoint $xyPoint 1]
  return [lrange $rast 0 2]
}

# return xyz for a given device coordinate (x y).  THe device coordinate (xy)
# is a position in the window, the returned xyz is a position in a viewport, 
# with z corresonding to a slice in the slice volume
itcl::body SWidget::dcToXYZ { wx wy } {
  set wx [expr int($wx)]
  set wy [expr int($wy)]

  foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}
  set numRows [$_sliceNode GetLayoutGridRows]
  set numCols [$_sliceNode GetLayoutGridColumns]

  set tx [expr $wx / double($windoww)]
  set ty [expr ($windowh - $wy) / double($windowh)]

  set z [expr (floor($ty*$numRows)*$numCols + floor($tx*$numCols))]
  
  set pokedRenderer [$_interactor FindPokedRenderer $wx $wy]
  foreach {rox roy} [$pokedRenderer GetOrigin] {}
  set x [expr $wx - $rox]
  set y [expr $wy - $roy]

  return "$x $y $z"
}

itcl::body SWidget::queryLayers { x y {z 0} } {
  # 
  # get the logic, node, image, ijk coords, and pixel for each layer
  # - store these in a layers array for easy access
  #
  foreach layer {background foreground label} {
    set _layers($layer,logic) [[$sliceGUI GetLogic]  Get[string totitle $layer]Layer]
    set _layers($layer,node) [$_layers($layer,logic) GetVolumeNode]
    if { $_layers($layer,node) == "" } {
      set _layers($layer,image) ""
      set _layers($layer,xyToIJK) ""
      foreach v {i j k} { 
        set _layers($layer,$v) 0
      }
      set _layers($layer,pixel) "None"
    } else {
      set _layers($layer,image) [$_layers($layer,node) GetImageData]
      set _layers($layer,xyToIJK) [[$_layers($layer,logic) GetXYToIJKTransform] GetMatrix]
      foreach {i j k l} [$_layers($layer,xyToIJK) MultiplyPoint $x $y $z 1] {}
      foreach v {i j k} { ;# cast to integer
        if { ![string is double [set $v]] } {
          set _layers($layer,$v) 0
        } else {
          set _layers($layer,$v) [expr int(round([set $v]))]
        }
      }
      set _layers($layer,pixel) [$this getPixel $_layers($layer,image) \
                      $_layers($layer,i) $_layers($layer,j) $_layers($layer,k)]
    }
  }
}

itcl::body SWidget::getPixel { image i j k } {
  if { $image == "" } { return "" }
  foreach index "i j k" dimension [$image GetDimensions] {
    set ind [set $index]
    if { $ind < 0 || $ind >= $dimension } {return "Out of Frame"}
  }
  set n [$image GetNumberOfScalarComponents]

  if { 0 } {
    ### BUG IN vtkImageData GetScalarComponentAsDouble ??? 
    for {set c 0} {$c < $n} {incr c} {
      lappend pixel [$image GetScalarComponentAsDouble $i $j $k $c]
    }
  } else {
    # directly access the scalars to get pixel value
    # - need to compensate because the increments already include the pixel size
    set scalars [[$image GetPointData] GetScalars]
    foreach "w h d" [$image GetDimensions] {}
    set sliceSize [expr $w * $h]
    set idx [expr $i + $j*$w + $k*$sliceSize]
    if { $scalars != "" && $n <= 4 } {
      set pixel [$scalars GetTuple$n $idx]
    } else {
      set pixel "Unknown"
    }
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

#
# TODO: this little helper reloads the swidget functionality
#
proc sssss {} {
  itcl::delete class SWidget

  set dir $::env(SLICER_HOME)/../Slicer3/Base/GUI/Tcl
  source $dir/SWidget.tcl
  foreach sw [glob $dir/*SWidget.tcl] {
    source $sw
  }
}
