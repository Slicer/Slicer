
package require Itcl

#########################################################
#
if {0} { ;# comment

  EffectSWidget a superclass for editor effects


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             EffectSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class EffectSWidget] == "" } {

  itcl::class EffectSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable scope "all"

    variable _startPosition "0 0 0"
    variable _currentPosition "0 0 0"
    variable _cursorActors ""

    # methods
    method processEvent {} {}
    method preProcessEvent {} {}
    method positionCursor {} {}
    method createCursor {} {}
    method preview {} {}
    method apply {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body EffectSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  $this createCursor

  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"

  $this processEvent

  set _guiObserverTags ""
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  foreach event "LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent EnterEvent LeaveEvent" {
    lappend _guiObserverTags \
              [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent"]
  }
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent"]
}

itcl::body EffectSWidget::destructor {} {

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
    foreach a $_cursorActors {
      $_renderer RemoveActor2D $a
    }
  }
}

itcl::configbody EffectSWidget::scope {
  if { [lsearch "all visible" $scope] == -1 } {
    set scope "all"
    error "invalid scope for EffectSWidget $this"
  }
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body EffectSWidget::createCursor {} {

  set o(cursorMapper) [vtkNew vtkImageMapper]
  set o(cursorActor) [vtkNew vtkActor2D]
  $o(cursorActor) SetMapper $o(cursorMapper)
  $o(cursorMapper) SetColorWindow 255
  $o(cursorMapper) SetColorLevel 128

  set reader [vtkPNGReader New]
  #TODO: configure icons into build directory...
  $reader SetFileName c:/tmp/DrawFreePolylineROI.png
  $reader Update
  $o(cursorMapper) SetInput [$reader GetOutput]
  $reader Delete
  lappend _cursorActors $o(cursorActor)

  foreach actor $_cursorActors {
    [$_renderWidget GetRenderer] AddActor2D $o(cursorActor)
  }
}

itcl::body EffectSWidget::positionCursor {} {
  set xyzw [$this rasToXY $_currentPosition]
  foreach {x y z w} $xyzw {}
  set x [expr $x + 16]
  set y [expr $y - 32]
  foreach actor $_cursorActors {
    eval $actor SetPosition $x $y
  }
}

#
# returns 1 if the event is 'swallowed' by
# the superclass, otherwise returns 0 and the
# subclass should do normal processing
#
itcl::body EffectSWidget::preProcessEvent { } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return 1
  }

  set grabID [$sliceGUI GetGrabID]
  if { ($grabID != "") && ($grabID != $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionActors
    [$sliceGUI GetSliceViewer] RequestRender
    return 1
  }

  return 0
}

itcl::body EffectSWidget::processEvent { } {
  # to be overridden by subclass
  # - should include call to superclass preProcessEvent
  #   to handle 'friendly' interaction with other SWidgets

  [$this superclass] preProcessEvent

}

itcl::body EffectSWidget::apply {} {
  # to be overridden by subclass
}

itcl::body EffectSWidget::preview {} {
  # to be overridden by subclass
}

proc EffectSWidget::AddEffect {} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      EffectSWidget #auto [$sw cget -sliceGUI]
    }
  }
}

proc EffectSWidget::RemoveEffect {} {
  foreach pw [itcl::find objects -class EffectSWidget] {
    itcl::delete object $pw
  }
}

proc EffectSWidget::ToggleEffect {} {
  if { [itcl::find objects -class EffectSWidget] == "" } {
    EffectSWidget::AddEffect
  } else {
    EffectSWidget::RemoveEffect
  }
}

proc EffectSWidget::ConfigureAll { args } {
  foreach pw [itcl::find objects -class EffectSWidget] {
    eval $pw configure $args
  }
}
