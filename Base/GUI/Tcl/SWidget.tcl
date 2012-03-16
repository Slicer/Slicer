
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
namespace eval SWidget set CALLBACK_LEVEL 0
namespace eval SWidget set VERBOSE_CALLBACKS 0
namespace eval SWidget set DEBUG_CALLBACKS 1
namespace eval SWidget set TIME_CALLBACKS 0
namespace eval SWidget set DISABLE_CALLBACKS 0
namespace eval SWidget set EXCLUDE "__instance_name_pattern__"
namespace eval SWidget {
  proc ProtectedCallback {instance args} {
    if { $::SWidget::DISABLE_CALLBACKS } {
      return
    }
    if { [string match $::SWidget::EXCLUDE $instance] ||
          [string match $::SWidget::EXCLUDE $args] } {
      return
    }
    if { $::SWidget::VERBOSE_CALLBACKS } {
      for {set sp 0} {$sp < $::SWidget::CALLBACK_LEVEL} {incr sp} {
        puts -nonewline " "
      }
      puts "callback for $instance with $args"
    }
    incr ::SWidget::CALLBACK_LEVEL
    if { [info command $instance] != "" } {
      if { $::SWidget::TIME_CALLBACKS } {
        puts -nonewline "($instance $args: "
        puts "[time "eval $instance $args" 1])"
      } else {
        if { $::SWidget::DEBUG_CALLBACKS } {
          eval $instance $args
        } else {
          if { [catch "eval $instance $args" res] } {
            catch "puts $res"
            catch "puts $::errorInfo"
            if { [string match "*bad alloc*" $res] } {
              ::bgerror $res
            }
          }
        }
      }
    }
    incr ::SWidget::CALLBACK_LEVEL -1
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

    public variable enabled "true"  ;# if not enabled, subclasses should not process events

    public variable forceAnnotationsOff 1  ;# don't show annotations at all

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

    # flag to indicate that there is an update pending (in after idle)
    variable _updatePending 0
    # annotation update requested
    variable _annotationTaskID ""

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
    method getTensorPixel {node i j k} {}
    method setPixel {image i j k value} {}
    method setPixelBlock {image i j k size value} {}
    method requestUpdate {} {}
    method processUpdate {} {}
    method requestDelayedAnnotation {} {}
    method processDelayedAnnotation {} {}
    method cancelDelayedAnnotation {} {}
    method getInAnySliceSWidget {} {}

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

    # for debugging
    method log {msg} {
      set fp [open "swidget.log" "a"]
      puts $fp "$this: $msg"
      close $fp
    }
  }
}

#
# called on 'widget configure -sliceGUI gui'
# - caches handles to useful objects
#
itcl::configbody SWidget::sliceGUI {
  set _renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
    
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

  if { $windoww == 0 || $windowh == 0 } {
    # degenerate case, return gracefully
    return "0 0 0"
  }

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

  # determine which renderer based on z position
  # - ignore if z is not define (as when there is just one slice)
  if { [catch "expr $z" res] } {
    # puts "bad z: $res"
    set lightboxK 0
  } else {
    set lightboxK [expr int($z + 0.5)]
  }
      
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
    } elseif { $lightboxK < 0 || $lightboxK >= [$_renderWidget GetNumberOfRenderers] } {
      # RAS position in not a visible slice, don't report pixel information
      set _layers($layer,image) ""
      set _layers($layer,xyToIJK) ""
      foreach v {i j k} { 
        set _layers($layer,$v) "-"
      }
      set _layers($layer,pixel) "Slice not shown"
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
      if { [$_layers($layer,node) GetClassName] == "vtkMRMLDiffusionTensorVolumeNode" } {
        set _layers($layer,pixel) [$this getTensorPixel $_layers($layer,node) \
                        $_layers($layer,i) $_layers($layer,j) $_layers($layer,k)]
      } else {
        set _layers($layer,pixel) [$this getPixel $_layers($layer,image) \
                        $_layers($layer,i) $_layers($layer,j) $_layers($layer,k)]
      }
    }
  }
}

namespace eval SWidget set getTensorPixelWarning 0

itcl::body SWidget::getTensorPixel { node i j k } {

  # TODO: in slicer4 this code is not needed
  return

  if { ![info exists o(dtiMath)] } {
    if { [info command vtkDiffusionTensorMathematics] == "" } {
      if { !$::SWidget::getTensorPixelWarning } {
        puts "$this: TODO: in slicer4 vtkDiffusionTensorMathematics class is not available - need to figure out why..."
        set ::SWidget::getTensorPixelWarning 1
      }
      return
    }
    set o(dtiMath) [vtkNew vtkDiffusionTensorMathematics]
    set o(dtiPixelImage) [vtkNew vtkImageData]
    set o(dtiPixelTensors) [vtkNew vtkDoubleArray]
    $o(dtiPixelImage) SetDimensions 1 1 1
    $o(dtiPixelImage) AllocateScalars
    $o(dtiPixelTensors) SetNumberOfComponents 9
    $o(dtiPixelTensors) SetNumberOfTuples 1 
    $o(dtiPixelTensors) Allocate 9 0
    [$o(dtiPixelImage) GetPointData] SetTensors $o(dtiPixelTensors)
    $o(dtiMath) SetInput $o(dtiPixelImage)
  }

  set tensorImage [$node GetImageData]
  if { $tensorImage == "" } {
    return "No Image Data"
  }
  set tensors [[$tensorImage GetPointData] GetTensors]
  if { $tensors == "" } {
    return "No Tensors"
  }
  set displayNode [$node GetDisplayNode]
  if { $displayNode == "" } {
    return "No Display Node"
  }

  foreach "w h d" [$tensorImage GetDimensions] {}
  set sliceSize [expr $w * $h]
  set volSize [expr $d * $sliceSize]
  set idx [expr $i + $j*$w + $k*$sliceSize]

  if { $idx < 0 || $idx >= $volSize } {
    return "Out of Frame"
  }

  set tensor [$tensors GetTuple9 $idx]
  eval $o(dtiPixelTensors) SetTuple9  0 $tensor

  $o(dtiPixelImage) Modified
  $o(dtiMath) SetOperation [$displayNode GetScalarInvariant]
  $o(dtiMath) Update
  set outImage [$o(dtiMath) GetOutput]
  set numComps [$outImage GetNumberOfScalarComponents]
  set pixel ""
  if { $numComps == 1 } {
    lappend pixel [$outImage GetScalarComponentAsDouble 0 0 0 0]
  }
  lappend pixel [$o(dtiMath) GetOperationAsAbbreviatedString]
  return $pixel
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
    if { $scalars == "" } {
      set scalars [[$image GetPointData] GetTensors]
    }
    foreach "w h d" [$image GetDimensions] {}
    set sliceSize [expr $w * $h]
    set idx [expr $i + $j*$w + $k*$sliceSize]
    if { $scalars != "" && $n <= 3 } {
      set pixel [$scalars GetTuple$n $idx]
    } else {
      if { $scalars != "" } {
        set pixel "$n Components"
      } else {
        set pixel "Unknown"
      }
    }
  }
  if { [string is double $pixel] && $pixel == [expr int($pixel)] } {
    set pixel [expr int($pixel)]
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

itcl::body SWidget::requestUpdate {} {
  if { $_updatePending } {
    return
  }
  set _updatePending 1
  after idle $this processUpdate
}

itcl::body SWidget::processUpdate {} {
  set _updatePending 0
}

itcl::body SWidget::requestDelayedAnnotation { } {
    $this cancelDelayedAnnotation
    set _annotationTaskID [after 300 ::SWidget::ProtectedCallback $this processDelayedAnnotation]
}

itcl::body SWidget::processDelayedAnnotation { } {
    if { [$this getInAnySliceSWidget] && !$forceAnnotationsOff } {
      $_renderWidget CornerAnnotationVisibilityOn
      [$sliceGUI GetSliceViewer] RequestRender
    } else {
      $_renderWidget CornerAnnotationVisibilityOff
      [$sliceGUI GetSliceViewer] RequestRender
    }
}

itcl::body SWidget::cancelDelayedAnnotation { } {
    $_renderWidget CornerAnnotationVisibilityOff
    if {$_annotationTaskID != ""} {
        after cancel $_annotationTaskID
        set _annotationTaskID ""
        [$sliceGUI GetSliceViewer] RequestRender
    }
}

itcl::body SWidget::getInAnySliceSWidget { } {
    if { [itcl::find class ::SliceSWidget] == "::SliceSWidget"} {
      set swidgets [itcl::find objects -class ::SliceSWidget]
      foreach sw $swidgets {
          if { [$sw getInWidget] } {
            return 1
          }
      }
    }
    return 0
}

#
# TODO: this little helper reloads the swidget functionality
#
proc sssss {} {
  itcl::delete class SWidget

  set dir $::env(SLICER_HOME)/$::env(SLICER_SHARE_DIR)/Tcl
  source $dir/SWidget.tcl
  foreach sw [glob $dir/*SWidget.tcl] {
    source $sw
  }
}
