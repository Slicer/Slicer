
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
    public variable rgba ".5 .5 1 .3"  ;# grid color
    public variable cutoff "5"  ;# don't show grid if it's less than 'cutoff' screen pixels

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method updateGrid { } {}
    method resetGrid { } {}
    method addGridLine { startPoint endPoint } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body GridSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  # create grid display parts
  set o(gridPolyData) [vtkNew vtkPolyData]
  set o(gridLines) [vtkNew vtkCellArray]
  $o(gridPolyData) SetLines $o(gridLines)
  set o(gridPoints) [vtkNew vtkPoints]
  $o(gridPolyData) SetPoints $o(gridPoints)

  set o(gridMapper) [vtkNew vtkPolyDataMapper2D]
  set o(gridActor) [vtkNew vtkActor2D]
  $o(gridMapper) SetInput $o(gridPolyData)
  $o(gridActor) SetMapper $o(gridMapper)
  eval [$o(gridActor) GetProperty] SetColor [lrange $rgba 0 2]
  eval [$o(gridActor) GetProperty] SetOpacity [lindex $rgba 3]
  after idle [$_renderWidget GetRenderer] AddActor2D $o(gridActor)
  lappend _actors $o(gridActor)

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

  set node [[$sliceGUI GetLogic] GetSliceCompositeNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "after idle ::SWidget::ProtectedCallback $this processEvent $node"]

  after idle $this processEvent $node
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
itcl::body GridSWidget::processEvent { {caller ""} {event ""} } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    ::SWidget::ProtectedDelete $this
    return
  }

  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y
  set xyToRAS [$_sliceNode GetXYToRAS]
  set ras [$xyToRAS MultiplyPoint $x $y 0 1]
  foreach {r a s t} $ras {}

  set node [[$sliceGUI GetLogic] GetSliceNode]
  if { $caller == $node } {
    $this updateGrid
    return
  }

  set node [[$sliceGUI GetLogic] GetSliceCompositeNode]
  if { $caller == $node } {
    $this updateGrid
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  if { $caller == $sliceGUI } {

    switch $event {

      "MouseMoveEvent" {
        #
        # highlight the current grid cell
        #

        # update the actors...
      }
    }

    return
  }

}

itcl::body GridSWidget::resetGrid { } {

  set idArray [$o(gridLines) GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0
  $o(gridPoints) Reset
  $o(gridLines) SetNumberOfCells 0

}

itcl::body GridSWidget::addGridLine { startPoint endPoint } {

  set startPoint [lrange $startPoint 0 2]
  set endPoint [lrange $endPoint 0 2]
  set startIndex [eval $o(gridPoints) InsertNextPoint $startPoint]
  set endIndex [eval $o(gridPoints) InsertNextPoint $endPoint]

  set cellCount [$o(gridLines) GetNumberOfCells]
  set idArray [$o(gridLines) GetData]
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $startIndex
  $idArray InsertNextTuple1 $endIndex
  $o(gridLines) SetNumberOfCells [expr $cellCount + 1]

}


#
# make the grid object
#
itcl::body GridSWidget::updateGrid { } {

  $this resetGrid

  if { ![$_sliceCompositeNode GetLabelGrid] || [$_sliceCompositeNode GetLabelVolumeID] == "" } {
    $o(gridActor) SetVisibility 0
    [$sliceGUI GetSliceViewer] RequestRender
    return
  }

  #
  # check the size cutoff
  # - map a single pixel from IJK to XY and check the size
  # - if the grid won't be visible, don't show it.
  # - also don't so if image data doesn't exist
  #
  set ijkToXY [vtkMatrix4x4 New]
  $ijkToXY DeepCopy [[$_layers($layer,logic) GetXYToIJKTransform] GetMatrix]
  $ijkToXY SetElement 0 3  0
  $ijkToXY SetElement 1 3  0
  $ijkToXY SetElement 2 3  0
  $ijkToXY Invert
  foreach {x y z w} [$ijkToXY MultiplyPoint 1 1 1 0] {}
  if { [expr abs($x)] < $cutoff && [expr abs($y)] < $cutoff || $_layers($layer,image) == "" } {
    $o(gridActor) SetVisibility 0
    $ijkToXY Delete
    return
  } else {
    if { [$o(gridActor) GetVisibility] } {
      # put the actor at the top of the display list
      [$_renderWidget GetRenderer] RemoveActor2D $o(gridActor)
      [$_renderWidget GetRenderer] AddActor2D $o(gridActor)
    }
    $o(gridActor) SetVisibility 1
  }


  #
  # determine which slice plane to display 
  # - since this is an orthogonal projection, all slices will look the same,
  #   so once we know which to draw, we only need to draw a single one
  # - choose the two indices that have the largest change with respect to XY
  # - make two sets of lines, one along the rows and one along the columns
  #
  set xyToIJK [vtkMatrix4x4 New]
  $xyToIJK DeepCopy [[$_layers($layer,logic) GetXYToIJKTransform] GetMatrix]
  $xyToIJK SetElement 0 3  0
  $xyToIJK SetElement 1 3  0
  $xyToIJK SetElement 2 3  0
  foreach {i j k l} [$xyToIJK MultiplyPoint 1 1 0 0] {}
  foreach v {i j k l} { set $v [expr abs([set $v])] }

  if { $i < $j && $i < $k } { set rowAxis 1; set colAxis 2 }
  if { $j < $i && $j < $k } { set rowAxis 0; set colAxis 2 }
  if { $k < $i && $k < $j } { set rowAxis 0; set colAxis 1 }

  set dims [$_layers($layer,image) GetDimensions]
  set rowDims [lindex $dims $rowAxis]
  set colDims [lindex $dims $colAxis]

  $ijkToXY DeepCopy [[$_layers($layer,logic) GetXYToIJKTransform] GetMatrix]
  $ijkToXY Invert

  set startPoint "-0.5 -0.5 -0.5 1"
  set endPoint "-0.5 -0.5 -0.5 1"
  set endPoint [lreplace $endPoint $colAxis $colAxis [expr $colDims - 0.5]]

  for {set row -0.5} {$row <= $rowDims} {set row [expr $row + 1.0]} {
    set startPoint [lreplace $startPoint $rowAxis $rowAxis $row]
    set endPoint [lreplace $endPoint $rowAxis $rowAxis $row]
    set xyStartPoint [eval $ijkToXY MultiplyPoint $startPoint]
    set xyEndPoint [eval $ijkToXY MultiplyPoint $endPoint]
    $this addGridLine $xyStartPoint $xyEndPoint
  }

  set startPoint "-0.5 -0.5 -0.5 1"
  set endPoint "-0.5 -0.5 -0.5 1"
  set endPoint [lreplace $endPoint $rowAxis $rowAxis [expr $rowDims - 0.5]]

  for {set col -0.5} {$col <= $colDims} {set col [expr $col + 1.0]} {
    set startPoint [lreplace $startPoint $colAxis $colAxis $col]
    set endPoint [lreplace $endPoint $colAxis $colAxis $col]
    set xyStartPoint [eval $ijkToXY MultiplyPoint $startPoint]
    set xyEndPoint [eval $ijkToXY MultiplyPoint $endPoint]
    $this addGridLine $xyStartPoint $xyEndPoint
  }

  $ijkToXY Delete
  $xyToIJK Delete

  [$sliceGUI GetSliceViewer] RequestRender
}

proc GridSWidget::AddGrid {} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      GridSWidget #auto [$sw cget -sliceGUI]
    }
  }
}

proc GridSWidget::RemoveGrid {} {
  foreach pw [itcl::find objects -class GridSWidget] {
    itcl::delete object $pw
  }
}

proc GridSWidget::ToggleGrid {} {
  if { [itcl::find objects -class GridSWidget] == "" } {
    GridSWidget::AddGrid
  } else {
    GridSWidget::RemoveGrid
  }
}
