
set ::SliceViewerEvents {
    MouseMoveEvent RightButtonPressEvent RightButtonReleaseEvent LeftButtonPressEvent
    LeftButtonReleaseEvent MiddleButtonPressEvent MiddleButtonReleaseEvent MouseWheelForwardEvent
    MouseWheelBackwardEvent ExposeEvent ConfigureEvent EnterEvent LeaveEvent
    TimerEvent KeyPressEvent KeyReleaseEvent CharEvent ExitEvent
  }

proc SliceViewerAddObservers {sliceViewer} {

  #set interactor [[$sliceViewer GetRenderWidget] GetInteractor]
  set interactor [$sliceViewer GetRenderWindowInteractor]

  foreach event $::SliceViewerEvents {
    $interactor AddObserver $event "SliceViewerHandleEvent $interactor $event"
  }
}

proc SliceViewerRemoveObservers {sliceViewer} {

  #set interactor [[$sliceViewer GetRenderWidget] GetInteractor]
  set interactor [$sliceViewer GetRenderWindowInteractor]

  foreach event $::SliceViewerEvents {
    $interactor RemoveObserver $event "SliceViewerHandleEvent $interactor $event"
  }
}

proc SliceViewerHandleEvent {interactor event} {

  puts "got a $event for $interactor at [$interactor GetEventPosition]"

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

