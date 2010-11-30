
#########################################################
#
if {0} { ;# comment

  CrosshairSWidget  - displays a crosshair on a sliceGUI

# TODO : 
#   1. Get the crosshair to show up initially
#   2. If changing slices, does the crosshair need to change position in the other views
#   3. 
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
    #public variable rgba "1.0 0.8 0.1 .6"  ;# crosshair color
    public variable rgba "1.0 0.8 0.1 1.0"  ;# crosshair color

    variable _crosshairNode
    variable _annotationTaskID ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method updateCrosshair { visible } {}
    method resetCrosshair { } {}
    method addCrosshairLine { startPoint endPoint } {}

    method pick { } {}
    method highlight { } {}
    method unhighlight { } {}

    # set the position of the crosshair actor (does not modify the crosshair node)
    # returns whether the crosshair is visible on in the viewer
    method setPosition { r a s } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body CrosshairSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI

  # cache the color of the viewer
  set rgba "[lindex [[$sliceGUI GetSliceViewer] GetHighlightColor] 0] [lindex [[$sliceGUI GetSliceViewer] GetHighlightColor] 1] [lindex [[$sliceGUI GetSliceViewer] GetHighlightColor] 2] [lindex $rgba 3]"
 
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
  $o(crosshairActor) VisibilityOff
  eval [$o(crosshairActor) GetProperty] SetColor [lrange $rgba 0 2]
  eval [$o(crosshairActor) GetProperty] SetOpacity [lindex $rgba 3]
  [$_renderWidget GetRenderer] AddActor2D $o(crosshairActor)
  lappend _actors $o(crosshairActor)

  # create a highlight representation
  set o(crosshairHighlightPolyData) [vtkNew vtkPolyData]
  set o(crosshairHighlightLines) [vtkNew vtkCellArray]
  $o(crosshairHighlightPolyData) SetLines $o(crosshairHighlightLines)
  set o(crosshairHighlightVerts) [vtkNew vtkCellArray]
  $o(crosshairHighlightPolyData) SetVerts $o(crosshairHighlightVerts)
  set o(crosshairHighlightPoints) [vtkNew vtkPoints]
  $o(crosshairHighlightPolyData) SetPoints $o(crosshairHighlightPoints)

  set o(crosshairHighlightMapper) [vtkNew vtkPolyDataMapper2D]
  set o(crosshairHighlightActor) [vtkNew vtkActor2D]
  $o(crosshairHighlightMapper) SetInput $o(crosshairHighlightPolyData)
  $o(crosshairHighlightActor) SetMapper $o(crosshairHighlightMapper)
  $o(crosshairHighlightActor) VisibilityOff
  eval [$o(crosshairHighlightActor) GetProperty] SetColor [lrange $rgba 0 2]
  eval [$o(crosshairHighlightActor) GetProperty] SetOpacity [lindex $rgba 3]
  [$_renderWidget GetRenderer] AddActor2D $o(crosshairHighlightActor)
  lappend _actors $o(crosshairHighlightActor)

  # set the initial position to the middle of the window
  set tkwindow [$_renderWidget  GetWidgetName]
  set w2 [expr [winfo width $tkwindow] / 2]
  set h2 [expr [winfo height $tkwindow] / 2]
  $o(crosshairActor) SetPosition $w2 $h2
  $o(crosshairHighlightActor) SetPosition $w2 $h2

  # 
  # set the crosshair node and add observers to the crosshair
  # (for now, always talk to the singleton "default")
  #
  set numberOfCrosshairs [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLCrosshairNode"]
  for {set xi 0} { $xi < $numberOfCrosshairs} { [incr xi] } {
      set xnode [$::slicer3::MRMLScene GetNthNodeByClass $xi vtkMRMLCrosshairNode]
      if { [$xnode GetCrosshairName] == "default" } {
          set _crosshairNode $xnode
          $::slicer3::Broker AddObservation $_crosshairNode ModifiedEvent "::SWidget::ProtectedCallback $this processEvent $_crosshairNode ModifiedEvent"          
          break
      }
  }


  #
  # set up observers on sliceGUI and on sliceNode
  #

  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"

  set events {  "MouseMoveEvent" "ConfigureEvent" "LeaveEvent" "LeftButtonPressEvent" "LeftButtonReleaseEvent"}
  foreach event $events {
    $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  $::slicer3::Broker AddObservation $node DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $node AnyEvent "::SWidget::ProtectedCallback $this processEvent $node AnyEvent"

  # watch for a scene close event so that we can get the state of the
  # crosshair back in sync with the SliceCompositeNode.  Scene close
  # events do not call modified when resetting the SliceCompositeNodes
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene 66003 "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene SceneClosedEvent"

  $this updateCrosshair 1
}


itcl::body CrosshairSWidget::destructor {} {

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

  if { [info command $sliceGUI] == "" || [$sliceGUI GetLogic] == "" } {
      # the sliceGUI was deleted behind our back, so we need to 
      # self destruct
      ::SWidget::ProtectedDelete $this
      return
  }

  if { [$_crosshairNode GetCrosshairMode] == 0 } {
      # don't bother to calculate anything if we aren't visible anyway...
      $o(crosshairActor) VisibilityOff
      $o(crosshairHighlightActor) VisibilityOff
      [$sliceGUI GetSliceViewer] RequestRender
      return
  }

  if { $caller == $_sliceNode } {
      # Slice node changed. How we respond depends who initiated the change.  
      #
      # 1. Slice node changed because because of a slider or a restore. Here we may 
      #     need to recalculate the position of the crosshair in RAS space.
      # 2. Slice node changed because it was "jumped" by another slice. Here the change
      #     of the node does not change the crosshair at all (would the actor need to be 
      #     moved?)
      
      # Check to see if any Crosshairs are in the "dragging" state 
      set itclobjects [itcl::find objects -class CrosshairSWidget]
      set drg false
      foreach cw $itclobjects {
          set s [$cw cget -state]
          if { $s == "dragging" } {
              set drg true
          }
      }

      if { $drg == "false" } {
          eval $this setPosition [$_crosshairNode GetCrosshairRAS]
          if {1} {
            # SP - don't think this code is needed
            # JM - this code is needed for the case the case where the slice node
            #    is changing but through a slider or through a snapshot.  We need
            #    this code to move the crosshair to the current slice.
            #

            # puts "[clock seconds] Slice node change $_sliceNode"

            # No crosshairs are being dragged. So the slice node is changing through some
            # other mechanism. Move the crosshair RAS to keep it on the slice.

            # convert current (previous) RAS position to xyz
            foreach {x y z} [$this rasToXYZ [$_crosshairNode GetCrosshairRAS]] {}
        
            # convert that xyz to an RAS (won't work for lightbox!!!)
            foreach {r a s} [$this xyToRAS "$x $y"] {}
            
            # set the new crosshair position
            $_crosshairNode SetCrosshairRAS $r $a $s
          }
          return
      } else {
          # Some crosshair was being dragged and changed the slice node. Do nothing.
          return
     }
  }

  if { $caller == $_crosshairNode } {

      # position crosshair actor
      foreach {r a s} [$_crosshairNode GetCrosshairRAS] {}
      set visible [$this setPosition $r $a $s]
      
      # update the design and properties of the crosshair
      $this updateCrosshair $visible

#      if { [$_crosshairNode GetNavigation] == 0 } {
#          puts "call is crosshair node $this [expr rand()]"
#          [$sliceGUI GetSliceViewer] RequestRender
#      }

      return
  }


  if { $caller == $::slicer3::MRMLScene && $event == "SceneClosedEvent" } {
      $this updateCrosshair 1
      return
  }

  if { [$_crosshairNode GetCrosshairMode] == 0 } {
      # not using a crosshair, eject
      return
  }

  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
      # some other widget wants these events, eject
      return 
  }

  if { $_actionState != "dragging" } {
      # only check pick if we haven't grabbed (avoid 'dropping' the widget
      # when the mouse moves quickly)
      $this pick 
  }

  if { $caller == $sliceGUI } {
      $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again

      # handle events that do not depend on the pickstate
      switch $event {
          "ConfigureEvent" {
              # need to rebuild the crosshairs that span the whole window and reposition to 
              # the same RAS position
              set visible [$this setPosition [$_crosshairNode GetCrosshairRAS]]

              $this updateCrosshair $visible
              return
          }
          "LeaveEvent" {
              if { [$_crosshairNode GetNavigation] != 1 } {
                  # not navigating, move crosshair to middle
                  set renderer0 [$_renderWidget GetRenderer]
                  foreach {w h} [$renderer0 GetSize] {}

                  set xyz "[expr $w / 2] [expr $h / 2] 0"
                  set ras [$this xyzToRAS $xyz]
                  foreach {r a s} $ras {}

                  # update crosshair position (observers move the crosshair actors)
                  $_crosshairNode SetCrosshairRAS $r $a $s
              }
              return
          }
      }
      
      # handle events that do depend on the pickstate
      switch $_pickState {
          "" {
             # Not in a picking mode -> cursor mode not navigation mode
             switch $event {
                 "MouseMoveEvent" {
                     # get the event position and convert to RAS
                     foreach {windowx windowy} [$_interactor GetEventPosition] {}
                     set xyz [$this dcToXYZ $windowx $windowy]
                     
                     set ras [$this xyzToRAS $xyz]
                     foreach {r a s} $ras {}
                              
                     # update crosshair position (observers move the crosshair actors)
                     $_crosshairNode SetCrosshairRAS $r $a $s
                 }
             }
          }
          "outside" {
              set _actionState ""
              set state $_actionState
              $sliceGUI SetGrabID ""
              $this unhighlight
              return
          }
          "near" {
              set _actionState ""
              set state $_actionState
              $sliceGUI SetGrabID ""
              $this highlight
              return
          }
          "over" - "horizontal" - "vertical"  {
              # when mouse is over us, we pay attention to the
              # event and tell others not to look at it
              $sliceGUI SetGUICommandAbortFlag 1
              $this highlight
              $sliceGUI SetGrabID $this

              switch $event {

                  "LeftButtonPressEvent" {
                      $this requestDelayedAnnotation 
                      set _actionState "dragging"
                      set state $_actionState
                      return
                  }

                  "LeftButtonReleaseEvent" {
                      set _actionState ""
                      set state $_actionState
                      return
                  }

                  "MouseMoveEvent" {
                      $this requestDelayedAnnotation 
                      if { $_pickState == "over" } {
                          if { $_actionState == "dragging" } {
                              # get the event position and convert to RAS
                              foreach {windowx windowy} [$_interactor GetEventPosition] {}
                              set xyz [$this dcToXYZ $windowx $windowy]
                              foreach {x y z} $xyz {}
                              set k [expr int($z + 0.5)]
                              if { $k < 0 || $k >= [lindex [$_sliceNode GetDimensions] 2] } {                   
                                  return
                              }

                              set ras [$this xyzToRAS $xyz]
                              foreach {r a s} $ras {}
                              
                              # jump the other slices to the crosshair (move elsewhere?)
                              set itclobjects [itcl::find objects -class CrosshairSWidget]
                              
                              foreach cw $itclobjects {
                                  if {1 || $cw != $this} {
                                      set tlogic [[$cw cget -sliceGUI] GetLogic]
                                      set tlink [[$tlogic GetSliceCompositeNode] GetLinkedControl]
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl 0
                                      [$tlogic GetSliceNode] JumpSliceByOffsetting $k $r $a $s
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl $tlink
                                  }
                              }

                              # update crosshair position (observers move the crosshair actors)
                              # (need to Jump the slices before moving crosshair so the visibility 
                              # of the crosshair is mainpulated correctly)
                              $_crosshairNode SetCrosshairRAS $r $a $s
                          }
                      } elseif { $_pickState == "vertical" } {
                          if { $_actionState == "dragging" } {
                              # get the event position in xyz
                              foreach {windowx windowy} [$_interactor GetEventPosition] {}
                              foreach {x y z} [$this dcToXYZ $windowx $windowy] {}
                              set k [expr int($z + 0.5)]
                              if { $k < 0 || $k >= [lindex [$_sliceNode GetDimensions] 2] } {                   
                                  return
                              }

                              # convert current (previous) RAS position to xyz
                              foreach {ox oy oz} [$this rasToXYZ [$_crosshairNode GetCrosshairRAS]] {}
                              # create RAS (use $z and not $oz so crosshair can switch viewports)
                              foreach {r a s} [$this xyzToRAS "$x $oy $z"] {}
                              
                              # jump the other slices to the crosshair (move elsewhere?)
                              set itclobjects [itcl::find objects -class CrosshairSWidget]
                              
                              foreach cw $itclobjects {
                                  if {1 || $cw != $this} {
                                      set tlogic [[$cw cget -sliceGUI] GetLogic]
                                      set tlink [[$tlogic GetSliceCompositeNode] GetLinkedControl]
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl 0
                                      [$tlogic GetSliceNode] JumpSliceByOffsetting $k $r $a $s
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl $tlink
                                  }
                              }

                              # update crosshair position (observers move the crosshair actors)
                              # (need to Jump the slices before moving crosshair so the visibility 
                              # of the crosshair is mainpulated correctly)
                              $_crosshairNode SetCrosshairRAS $r $a $s
                          }
                      } elseif { $_pickState == "horizontal" } {
                          if { $_actionState == "dragging" } {
                              # get the event position in xyz
                              foreach {windowx windowy} [$_interactor GetEventPosition] {}
                              foreach {x y z} [$this dcToXYZ $windowx $windowy] {}
                              set k [expr int($z + 0.5)]
                              if { $k < 0 || $k >= [lindex [$_sliceNode GetDimensions] 2] } {                   
                                  return
                              }

                              # convert current (previous) RAS position to xyz
                              foreach {ox oy oz} [$this rasToXYZ [$_crosshairNode GetCrosshairRAS]] {}
                              # create RAS (use $z and not $oz so crosshair can switch viewports)
                              foreach {r a s} [$this xyzToRAS "$ox $y $z"] {}
                              
                              # jump the other slices to the crosshair (move elsewhere?)
                              set itclobjects [itcl::find objects -class CrosshairSWidget]
                              
                              foreach cw $itclobjects {
                                  if {1 || $cw != $this} {
                                      set tlogic [[$cw cget -sliceGUI] GetLogic]
                                      set tlink [[$tlogic GetSliceCompositeNode] GetLinkedControl]
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl 0
                                      [$tlogic GetSliceNode] JumpSliceByOffsetting $k $r $a $s
                                      [$tlogic GetSliceCompositeNode] SetLinkedControl $tlink
                                  }
                              }

                              # update crosshair position (observers move the crosshair actors)
                              # (need to Jump the slices before moving crosshair so the visibility 
                              # of the crosshair is mainpulated correctly)
                              $_crosshairNode SetCrosshairRAS $r $a $s
                          }
                      }
                      return
                  }
              }
          }
      }
  }
}

itcl::body CrosshairSWidget::resetCrosshair { } {

  set idArray [$o(crosshairLines) GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0
  $o(crosshairPoints) Reset
  $o(crosshairLines) SetNumberOfCells 0

  set hidArray [$o(crosshairHighlightLines) GetData]
  $hidArray Reset
  $hidArray InsertNextTuple1 0
  set cidArray [$o(crosshairHighlightVerts) GetData]
  $cidArray Reset
  $cidArray InsertNextTuple1 0
  $o(crosshairHighlightPoints) Reset
  $o(crosshairHighlightLines) SetNumberOfCells 0
  $o(crosshairHighlightVerts) SetNumberOfCells 0

#  $o(crosshairActor) VisibilityOff
#  $o(crosshairHighlightActor) VisibilityOff

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
itcl::body CrosshairSWidget::updateCrosshair { visible } {

  $this resetCrosshair

  set tkwindow [$_renderWidget  GetWidgetName]
  set w [winfo width $tkwindow]
  set h [winfo height $tkwindow]

  set negW [expr -1.0*$w]
  set negWminus -5
  set negWminus2 -10
  set posWplus 5
  set posWplus2 10
  set posW $w

  set negH [expr -1.0*$h]
  set negHminus -5
  set negHminus2 -10
  set posHplus 5
  set posHplus2 10
  set posH $h



  switch [$_crosshairNode GetCrosshairMode] {
    "0" {
      # do nothing
    }
    "1" {
      # show basic
      $this addCrosshairLine "0 $negH 0 0" "0 $negHminus 0 0"
      $this addCrosshairLine "0 $posHplus 0 0" "0 $posH 0 0"
      $this addCrosshairLine "$negW 0 0 0" "$negWminus 0 0 0"
      $this addCrosshairLine "$posWplus 0 0 0" "$posW 0 0 0"
    }
    "2" {
      # show intersection
      $this addCrosshairLine "$negW 0 0 0" "$posW 0 0 0"
      $this addCrosshairLine "0 $negH 0 0" "0 $posH 0 0"
    }
    "3" {
      # show hashmarks
      # TODO: add hashmarks (in cm?)
      $this addCrosshairLine "0 $negH 0 0" "0 $negHminus 0 0"
      $this addCrosshairLine "0 $posHplus 0 0" "0 $posH 0 0"
      $this addCrosshairLine "$negW 0 0 0" "$negWminus 0 0 0"
      $this addCrosshairLine "$posWplus 0 0 0" "$posW 0 0 0"
    }
    "4" {
      # show all
      # TODO: add hashmarks (in cm?)
      # show intersection
      $this addCrosshairLine "$negW 0 0 0" "$posW 0 0 0"
      $this addCrosshairLine "0 $negH 0 0" "0 $posH 0 0"
    }
    "5" {
      # small open cross
      $this addCrosshairLine "0 $negHminus2 0 0" "0 $negHminus 0 0"
      $this addCrosshairLine "0 $posHplus 0 0" "0 $posHplus2 0 0"
      $this addCrosshairLine "$negWminus2 0 0 0" "$negWminus 0 0 0"
      $this addCrosshairLine "$posWplus 0 0 0" "$posWplus2 0 0 0"
    }
    "6" {
      # small closed cross (intersection)
      $this addCrosshairLine "0 $negHminus2 0 0" "0 $posHplus2 0 0"
      $this addCrosshairLine "$negWminus2 0 0 0" "$posWplus2 0 0 0"
    }
  }

  if { $visible && [$_crosshairNode GetCrosshairMode] != 0 } {
      #puts "[$_sliceNode GetSingletonTag] is now visible"
      $o(crosshairActor) VisibilityOn
  } else {
      #puts "[$_sliceNode GetSingletonTag] is now hidden"
      $o(crosshairActor) VisibilityOff
      $o(crosshairHighlightActor) VisibilityOff
  }

  if { [$_crosshairNode GetCrosshairThickness] == 1 } {
    [$o(crosshairActor) GetProperty] SetLineWidth 1
  } elseif  { [$_crosshairNode GetCrosshairThickness] == 2 } {
    [$o(crosshairActor) GetProperty] SetLineWidth 3
  } elseif  { [$_crosshairNode GetCrosshairThickness] == 3 } {
    [$o(crosshairActor) GetProperty] SetLineWidth 5
  } 

  # build the highlight actor (is this too subtle?, do we box all the views?)
  #

  # center dot (need to offset by half a pixel, why?
  set cindex [$o(crosshairHighlightPoints) InsertNextPoint -0.5 -0.5 0]

  # box for highlight
  set llindex [$o(crosshairHighlightPoints) InsertNextPoint -5 -5 0]
  set lrindex [$o(crosshairHighlightPoints) InsertNextPoint 5 -5 0]
  set urindex [$o(crosshairHighlightPoints) InsertNextPoint 5 5 0]
  set ulindex [$o(crosshairHighlightPoints) InsertNextPoint -5 5 0]

  set cellCount [$o(crosshairHighlightLines) GetNumberOfCells]
  set idArray [$o(crosshairHighlightLines) GetData]
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $llindex
  $idArray InsertNextTuple1 $lrindex
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $lrindex
  $idArray InsertNextTuple1 $urindex
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $urindex
  $idArray InsertNextTuple1 $ulindex
  $idArray InsertNextTuple1 2
  $idArray InsertNextTuple1 $ulindex
  $idArray InsertNextTuple1 $llindex
  $o(crosshairLines) SetNumberOfCells [expr $cellCount + 4]

  set ccellCount [$o(crosshairHighlightVerts) GetNumberOfCells]
  set cidArray [$o(crosshairHighlightVerts) GetData]
  $cidArray InsertNextTuple1 1
  $cidArray InsertNextTuple1 $cindex
  $o(crosshairHighlightVerts) SetNumberOfCells [expr $ccellCount + 1]

  #puts "updateCrosshair  $this [expr rand()]"
  [$sliceGUI GetSliceViewer] RequestRender
}


itcl::body CrosshairSWidget::setPosition { r a s } {

  set visible 0

  $this queryLayers 0 0
  foreach {x y z } [rasToXYZ "$r $a $s"] {}

  # determine which renderer based on z position
  # - ignore if z is not define (as when there is just one slice)
  if { [catch {expr $z}] } {
    set k -1
  } else {
    set k [expr int($z + 0.5)]
  }

  if { $k >= 0 && $k < [$_renderWidget GetNumberOfRenderers] } {
    if { [info command $_renderer] != "" && $_renderer != [$_renderWidget GetNthRenderer $k] } {
      # switching viewports
      $_renderer RemoveActor2D $o(crosshairActor)
      $_renderer RemoveActor2D $o(crosshairHighlightActor)
    }

    # get the new viewport
    set _renderer [$_renderWidget GetNthRenderer $k]

    # add the actor, actor is only added if it is not already in the viewport
    if { [info command $_renderer] != "" } {
        $_renderer AddActor2D $o(crosshairActor)
        $_renderer AddActor2D $o(crosshairHighlightActor)
    }
  }
  
  # position the actor 
  set oldPosition [$o(crosshairActor) GetPosition]
  if { [lindex $oldPosition 0] != $x || [lindex $oldPosition 1] != $y } {
    $o(crosshairActor) SetPosition $x $y
    $o(crosshairHighlightActor) SetPosition $x $y
  } 

  if { $k >= 0 && $k < [$_renderWidget GetNumberOfRenderers] } {
      #puts "[$_sliceNode GetSingletonTag] should be visible"
      set visible 1
  } else {
      #puts "[$_sliceNode GetSingletonTag] should be hidden"
  }

  return $visible
}

itcl::body CrosshairSWidget::pick {} {

   if { [$_crosshairNode GetNavigation] == 1 } {
     # crosshair position in xyz
     foreach {cx cy cz} [$this rasToXYZ [$_crosshairNode GetCrosshairRAS]] {}
    
      # event position
      foreach {windowx windowy} [$_interactor GetEventPosition] {}
      set xyz [$this dcToXYZ $windowx $windowy]
      foreach {x y z} $xyz {}

      # check if within a few pixels
      set tol 5
      if { [expr abs($x - $cx) < $tol] && [expr abs($y - $cy) < $tol] } {
        set _pickState "over"
      } elseif { [expr abs($x - $cx) < $tol] } {
        set _pickState "vertical"
      } elseif { [expr abs($y - $cy) < $tol] } {
        set _pickState "horizontal"
      } elseif { [expr abs($x - $cx) < 2*$tol] || [expr abs($y - $cy) < 2*$tol] } {
        set _pickState "near"
      } else {
        set _pickState "outside"
      }
   } else {
       set _pickState ""
   }
}

itcl::body CrosshairSWidget::highlight { } {
    $o(crosshairHighlightActor) VisibilityOn
}

itcl::body CrosshairSWidget::unhighlight { } {
    $o(crosshairHighlightActor) VisibilityOff
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

