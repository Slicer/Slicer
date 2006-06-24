
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

proc SliceViewerHandleEvent {sliceGUI event} {

  set interactor [[$sliceGUI GetSliceViewer] GetRenderWindowInteractor]

  if { [lsearch "MouseMoveEvent ModifiedEvent" $event] == -1 } {
    puts -nonewline "got a $event for $sliceGUI, interactor $interactor at [$interactor GetEventPosition]"
    if { [$interactor GetControlKey] } {
      puts -nonewline " with control"
    }
    if { [$interactor GetShiftKey] } {
      puts -nonewline " with shift"
    }
    puts ""
  }

  switch $event {

    MouseMoveEvent {
    }
    RightButtonPressEvent {
    }
    RightButtonReleaseEvent {
    }
    LeftButtonPressEvent {
    }
    LeftButtonReleaseEvent {
    }
    MiddleButtonPressEvent {
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
      foreach {x y} $size {}
      if { $x < $y } { set min $x } else { set min $y }
      set sliceNode [[$sliceGUI GetLogic]  GetSliceNode]
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
