
set ::SliceViewerEvents {
    MouseMoveEvent RightButtonPressEvent RightButtonReleaseEvent LeftButtonPressEvent
    LeftButtonReleaseEvent MiddleButtonPressEvent MiddleButtonReleaseEvent MouseWheelForwardEvent
    MouseWheelBackwardEvent ExposeEvent ConfigureEvent EnterEvent LeaveEvent
    TimerEvent KeyPressEvent KeyReleaseEvent CharEvent ExitEvent
  }

proc SliceViewerAddObservers {sliceViewer} {

  return; # turn off this event manager for now

  set interactor [$sliceViewer GetRenderWindowInteractor]

  foreach event $::SliceViewerEvents {
    $interactor AddObserver $event "SliceViewerHandleEvent $interactor $event"
  }
}

proc SliceViewerRemoveObservers {sliceViewer} {

  return; # turn off this event manager for now

  set interactor [$sliceViewer GetRenderWindowInteractor]

  foreach event $::SliceViewerEvents {
    $interactor RemoveObserver $event "SliceViewerHandleEvent $interactor $event"
  }
}

set ::SliceViewerMode ""
set ::SliceViewerStorageXYToRAS [vtkMatrix4x4 New]
set ::SliceViewerStorageSliceToRAS [vtkMatrix4x4 New]
set ::SliceViewerScratchMatrix [vtkMatrix4x4 New]

proc SliceViewerHandleEvent {sliceGUI event} {

  set interactor [[$sliceGUI GetSliceViewer] GetRenderWindowInteractor]
  set sliceNode [[$sliceGUI GetLogic]  GetSliceNode]
  set sliceCompositeNode [[$sliceGUI GetLogic]  GetSliceCompositeNode]
  set background [[$sliceGUI GetLogic]  GetBackgroundLayer]
  set backgroundNode [$background GetVolumeNode]
  set backgroundImage [$backgroundNode GetImageData]
  
  foreach {x y} [$interactor GetEventPosition] {}

  set xyToRAS [$sliceNode GetXYToRAS]
  set ras [$xyToRAS MultiplyPoint $x $y 0 1]

  set xyToIJK [$background GetXYToIJKTransform]
  foreach {i j k} [$xyToIJK TransformVector $x $y 0] {}
  foreach v {i j k} { ;# cast to integer
    set $v [expr int([set $v])]
  }

  #set pixel [$backgroundImage GetScalarComponentAsDouble $i $j $k 0]

  set ignoreEvents "MouseMoveEvent ModifiedEvent TimerEvent RenderEvent"

  if { [lsearch $ignoreEvents $event] == -1 } {
    puts -nonewline "got a $event for $sliceGUI, interactor $interactor at xy $x $y, ras $ras"
    if { [$interactor GetControlKey] } {
      puts -nonewline " with control"
    }
    if { [$interactor GetShiftKey] } {
      puts -nonewline " with shift"
    }
    puts ""
  }

  if { [string match "*PressEvent" $event] } {
    set ::SliceViewerStartRAS $ras 
    set ::SliceViewerStartXY [$interactor GetEventPosition]
  }

  if { [string match "*ReleaseEvent" $event] ||
        [string match "LeaveEvent" $event] } {
    set ::SliceViewerMode ""
  }


  switch $event {

    MouseMoveEvent {
      # puts "background pixel at $i $j $k is $pixel"
      switch $::SliceViewerMode {
        Translate {
          set currentRAS [$::SliceViewerStorageXYToRAS MultiplyPoint $x $y 0 1]
          foreach d {dr da ds} start $::SliceViewerStartRAS current $currentRAS {
            set $d [expr $current - $start]
          }
          $::SliceViewerScratchMatrix DeepCopy $::SliceViewerStorageSliceToRAS
          foreach d {dr da ds} i {0 1 2} {
            set v [$::SliceViewerScratchMatrix GetElement $i 3]
            $::SliceViewerScratchMatrix SetElement $i 3 [expr $v - [set $d]]
          }
          [$sliceNode GetSliceToRAS] DeepCopy $::SliceViewerScratchMatrix
          $sliceNode UpdateMatrices
        }
        Zoom {
          set deltay [expr $y - [lindex $::SliceViewerStartXY 1]]
          set h [lindex [[[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetRenderWindow]  GetSize] 1]
          set percent [expr ($h + $deltay) / (1.0 * $h)]
          set newFOV ""
          foreach f $::SliceViewerStorageFieldOfView {
            lappend newFOV [expr $f * $percent]
          }
          eval $sliceNode SetFieldOfView $newFOV
          $sliceNode UpdateMatrices
        }
      }
    }
    RightButtonPressEvent {
      set ::SliceViewerMode Zoom
      set ::SliceViewerStorageFieldOfView [$sliceNode GetFieldOfView]
      $::slicer3::MRMLScene SaveStateForUndo $sliceNode
    }
    RightButtonReleaseEvent {
    }
    LeftButtonPressEvent {
    }
    LeftButtonReleaseEvent {
    }
    MiddleButtonPressEvent {
      set ::SliceViewerMode Translate
      $::SliceViewerStorageXYToRAS DeepCopy [$sliceNode GetXYToRAS]
      $::SliceViewerStorageSliceToRAS DeepCopy [$sliceNode GetSliceToRAS]
      $::slicer3::MRMLScene SaveStateForUndo $sliceNode
    }
    MiddleButtonReleaseEvent {
    }
    MouseWheelForwardEvent {
    }
    MouseWheelBackwardEvent {
    }
    ExposeEvent {
    }
    ConfigureEvent {
      set size [[[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetRenderWindow]  GetSize]
      foreach {w h} $size {}
      if { $w < $h } { set min $w } else { set min $h }
      set oldDim [$sliceNode GetDimensions]
      $sliceNode SetDimensions $min $min [lindex $oldDim 2]
      puts "[$sliceNode GetDimensions]"
    }
    EnterEvent {
    }
    LeaveEvent {
    }
    TimerEvent {
    }
    KeyPressEvent {
    }
    KeyReleaseEvent {
    }
    CharEvent {
      puts -nonewline "char event [$interactor GetKeyCode]"
      if { [$interactor GetControlKey] } {
        puts -nonewline " with control"
      }
      if { [$interactor GetShiftKey] } {
        puts -nonewline " with shift"
      }
    }
    ExitEvent {
    }

  }

}
