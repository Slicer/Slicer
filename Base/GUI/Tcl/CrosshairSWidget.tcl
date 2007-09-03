
#########################################################
#
if {0} { ;# comment

  CrosshairSWidget  - displays a crosshair on a sliceGUI

# TODO : 

}
#
#########################################################
# ------------------------------------------------------------------
#                             CrosshairSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class CrosshairSWidget] == "" } {

  itcl::class CrosshairSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    #public variable rgba ".5 .9 .5 .6"  ;# crosshair color
    public variable rgba "1.0 0.8 0.1 .6"  ;# crosshair color

    variable _compositeNodeObserverTags
    variable _compositeNode

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method updateCrosshair { } {}
    method resetCrosshair { } {}
    method addCrosshairLine { startPoint endPoint } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body CrosshairSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  # create crosshair display parts
  set o(crosshairPolyData) [vtkNew vtkPolyData]
  set o(crosshairLines) [vtkNew vtkCellArray]
  $o(crosshairPolyData) SetLines $o(crosshairLines)
  set o(crosshairPoints) [vtkNew vtkPoints]
  $o(crosshairPolyData) SetPoints $o(crosshairPoints)

  set o(crosshairMapper) [vtkNew vtkPolyDataMapper2D]
  set o(crosshairActor) [vtkNew vtkActor2D]
  $o(crosshairMapper) SetInput $o(crosshairPolyData)
  $o(crosshairActor) SetMapper $o(crosshairMapper)
  eval [$o(crosshairActor) GetProperty] SetColor [lrange $rgba 0 2]
  eval [$o(crosshairActor) GetProperty] SetOpacity [lindex $rgba 3]
  [$_renderWidget GetRenderer] AddActor2D $o(crosshairActor)
  lappend _actors $o(crosshairActor)

  #
  # set up observers on sliceGUI and on sliceNode
  # - track them so they can be removed in the destructor
  #
  set _guiObserverTags ""

  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]

  set events {  "MouseMoveEvent" "UserEvent" }
  foreach event $events {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI"]    
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]

  set _compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
  lappend _compositeNodeObserverTags [$_compositeNode AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _compositeNodeObserverTags [$_compositeNode AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $_compositeNode"]

  $this updateCrosshair
}


itcl::body CrosshairSWidget::destructor {} {

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

  if { [info command $_compositeNode] != "" } {
    foreach tag $_compositeNodeObserverTags {
      $_compositeNode RemoveObserver $tag
    }
  }

  if { [info command $_renderer] != "" } {
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
  }
}

itcl::configbody CrosshairSWidget::rgba {
  eval [$o(crosshairActor) GetProperty] SetColor [lrange $rgba 0 2]
  eval [$o(crosshairActor) GetProperty] SetOpacity [lindex $rgba 3]
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

#
# handle interactor events
#
itcl::body CrosshairSWidget::processEvent { {caller ""} {event ""} } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    ::SWidget::ProtectedDelete $this
    return
  }

  if { $caller == $_sliceNode || $caller == $_compositeNode } {
    $this updateCrosshair
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  if { $caller == $sliceGUI } {

    switch $event {

      "MouseMoveEvent" {
        #
        # TODO: what do we want to do on mouse move?
        #

        # update the actors...
      }
    }

    return
  }

}

itcl::body CrosshairSWidget::resetCrosshair { } {

  set idArray [$o(crosshairLines) GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0
  $o(crosshairPoints) Reset
  $o(crosshairLines) SetNumberOfCells 0

}

itcl::body CrosshairSWidget::addCrosshairLine { startPoint endPoint } {

  set startPoint [lrange $startPoint 0 2]
  set endPoint [lrange $endPoint 0 2]
  set startIndex [eval $o(crosshairPoints) InsertNextPoint $startPoint]
  set endIndex [eval $o(crosshairPoints) InsertNextPoint $endPoint]

  set cellCount [$o(crosshairLines) GetNumberOfCells]
  set idArray [$o(crosshairLines) GetData]
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $startIndex
  $idArray InsertNextTuple1 $endIndex
  $o(crosshairLines) SetNumberOfCells [expr $cellCount + 1]

}


#
# make the crosshair object
#
itcl::body CrosshairSWidget::updateCrosshair { } {

  $this resetCrosshair

  set logic [$sliceGUI GetLogic]
  set sliceCompositeNode [$logic GetSliceCompositeNode]

  set tkwindow [$_renderWidget  GetWidgetName]
  set w [winfo width $tkwindow]
  set h [winfo height $tkwindow]

  set halfW [expr $w / 2.0]
  set halfWminus [expr $halfW - 5]
  set halfWplus [expr $halfW + 5]
  set halfH [expr $h / 2.0]
  set halfHminus [expr $halfH - 5]
  set halfHplus [expr $halfH + 5]

  switch [$sliceCompositeNode GetCrosshairMode] {
    "0" {
      # do nothing
    }
    "1" {
      # show basic
      $this addCrosshairLine "$halfW 0 0 0" "$halfW $halfHminus 0 0"
      $this addCrosshairLine "$halfW $halfHplus 0 0" "$halfW $h 0 0"
      $this addCrosshairLine "0 $halfH 0 0" "$halfWminus $halfH 0 0"
      $this addCrosshairLine "$halfWplus $halfH 0 0" "$w $halfH 0 0"
    }
    "2" {
      # show intersection
      $this addCrosshairLine "$halfW 0 0 0" "$halfW $h 0 0"
      $this addCrosshairLine "0 $halfH 0 0" "$w $halfH 0 0"
    }
    "3" {
      # show hashmarks
      # TODO: add hashmarks
      $this addCrosshairLine "$halfW 0 0 0" "$halfW $halfHminus 0 0"
      $this addCrosshairLine "$halfW $halfHplus 0 0" "$halfW $h 0 0"
      $this addCrosshairLine "0 $halfH 0 0" "$halfWminus $halfH 0 0"
      $this addCrosshairLine "$halfWplus $halfH 0 0" "$w $halfH 0 0"
    }
    "4" {
      # show all
      # TODO: add hashmarks
      # show intersection
      $this addCrosshairLine "$halfW 0 0 0" "$halfW $h 0 0"
      $this addCrosshairLine "0 $halfH 0 0" "$w $halfH 0 0"
    }
  }

  [$sliceGUI GetSliceViewer] RequestRender
}

proc CrosshairSWidget::AddCrosshair {} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      CrosshairSWidget #auto [$sw cget -sliceGUI]
    }
  }
}

proc CrosshairSWidget::RemoveCrosshair {} {
  foreach pw [itcl::find objects -class CrosshairSWidget] {
    itcl::delete object $pw
  }
}

proc CrosshairSWidget::ToggleCrosshair {} {
  if { [itcl::find objects -class CrosshairSWidget] == "" } {
    CrosshairSWidget::AddCrosshair
  } else {
    CrosshairSWidget::RemoveCrosshair
  }
}
