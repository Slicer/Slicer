
#########################################################
#
if {0} { ;# comment

  GridSWidget  - manages slice plane interactions

# TODO : 

}
#
#########################################################
# ------------------------------------------------------------------
#                             GridSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class GridSWidget] == "" } {

  itcl::class GridSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable layer "label"  ;# which slice layer to show the grid for
    public variable rgba ".5 .5 1 .5"  ;# grid color
    public variable cutoff "3"  ;# don't show grid if it's less than 'cutoff' screen pixels

    # methods
    method processEvent {} {}
    method updateGrid { delta } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body GridSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  # create grid display parts
  set o(gridPolyData) [vtkNew vtkPolyData]
  set o(gridMapper) [vtkNew vtkPolyDataMapper2D]
  set o(gridActor) [vtkNew vtkActor2D]
  $o(gridMapper) SetInput [$o(gridIcon) GetOutput]
  $o(gridActor) SetMapper $o(gridMapper)
  [$o(gridActor) GetProperty] SetColor 1 1 0
  $o(gridActor) VisibilityOff
  $o(gridActor) SetPosition 3 3
  [$_renderWidget GetRenderer] AddActor2D $o(gridActor)
  lappend _actors $o(gridActor)

  $this processEvent

  #
  # set up observers on sliceGUI and on sliceNode
  # - track them so they can be removed in the destructor
  #
  set _guiObserverTags ""

  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "itcl::delete object $this"]

  set events {  "MouseMoveEvent" "UserEvent" }
  foreach event $events {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "$this processEvent"]    
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "$this processEvent"]
}


itcl::body GridSWidget::destructor {} {

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

#
# handle interactor events
#
itcl::body GridSWidget::processEvent { } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y
  set xyToRAS [$_sliceNode GetXYToRAS]
  set ras [$xyToRAS MultiplyPoint $x $y 0 1]
  foreach {r a s t} $ras {}

  switch $event {

    "MouseMoveEvent" {
      #
      # Mouse move behavior governed by _actionState mode
      # - first update the annotation
      # - then handle modifying the view
      #

      # update the actors...
    }

  }
}

