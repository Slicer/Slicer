#########################################################
#
if {0} { ;# comment

  RegionsSWidget  - manages ROI display for a slice view

# TODO : 

}
#
#########################################################
# ------------------------------------------------------------------
#                             RegionsSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class RegionsSWidget] == "" } {

  itcl::class RegionsSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}
    
    # a list of seeds - the callback info includes the mapping to list and index
    variable _seedSWidgets ""
    variable _sceneObserverTags ""
    variable _roiListObserverTagPairs ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method seedMovedCallback {seed roiListNode roiIndex} {}
    method seedMovingCallback {seed roiListNode roiIndex} {}
    method addROIListObserver {roiListNode} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body RegionsSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI

  #
  # set up observers on sliceNode
  # - track them so they can be removed in the destructor
  #
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]


  set scene [$sliceGUI GetMRMLScene]
  lappend _sceneObserverTags [$scene AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _sceneObserverTags [$scene AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $scene"]


  set _guiObserverTags ""

  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]

  set events {  
    "KeyPressEvent" 
    }
  foreach event $events {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI"]    
  }

  $this processEvent $scene
}


itcl::body RegionsSWidget::destructor {} {

  if { [info command $sliceGUI] != "" } {
    foreach tag $_guiObserverTags {
      $sliceGUI RemoveObserver $tag
    }
  }

  foreach pair $_roiListObserverTagPairs {
    foreach {roiListNode tag} $pair {}
    $roiListNode RemoveObserver $tag
  }

  if { [info command $_sliceNode] != "" } {
    foreach tag $_nodeObserverTags {
      $_sliceNode RemoveObserver $tag
    }
  }

  if { [info command $sliceGUI] != "" } {
    set scene [$sliceGUI GetMRMLScene]
    if { [info command $scene] != "" } {
      foreach tag $_sceneObserverTags {
        $scene RemoveObserver $tag
      }
    }
  }
}



# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

#
# handle scene and slice node events
# - for now, we need to always review rois lists in scene
# - create SeedSWidgets for any Regions that are close enough to slice
#

itcl::body RegionsSWidget::processEvent { {caller ""} {event ""} } {

    if { [info command $caller] == ""} {
        return
    }

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    ::SWidget::ProtectedDelete $this
    return
  }

  if { $caller == $sliceGUI } {
    set event [$sliceGUI GetCurrentGUIEvent] 
    set capture 1
    switch $event {
      "KeyPressEvent" {
        switch [$_interactor GetKeySym] {
          "grave" -
          "quoteleft" {
          }
          default {
            set capture 0
          }
        }
      }
    }
    if { $capture } {
      $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
      $sliceGUI SetGUICommandAbortFlag 1
    }
  }

  #
  # first, remove the old seeds
  # - for reasons unknown as yet, deleting the object from within the 
  #   event handler causes a hang -- so set it up to be deleted later
  #
  foreach seed $_seedSWidgets {
    $seed place -10000 -10000 -10000
    after idle "::SWidget::ProtectedDelete ::RegionsSWidget::$seed;"
    [$sliceGUI GetSliceViewer] RequestRender
  }
  set _seedSWidgets ""


  #
  # scene changed, so remove all the observers from the roi lists
  # - these will be recreated below to match the current scene
  #
  if { [$caller IsA "vtkMRMLScene"] } {
    foreach pair $_roiListObserverTagPairs {
      foreach {roiListNode tag} $pair {}
      after idle "$roiListNode RemoveObserver $tag"
    }
  }

  #
  # get the rasToSlice for the SliceNode - transforming the roi
  # by this matrix will let us easily check the distance from the slice plane
  #
  set node [[$sliceGUI GetLogic] GetSliceNode]
  set rasToSlice [vtkMatrix4x4 New]
  $rasToSlice DeepCopy [$node GetSliceToRAS]
  $rasToSlice Invert


  #
  # now, look through all the roi lists for rois that
  # are close enough to the current slice node; create seed widgets
  # for those and give them a "moved command" that will set the position of the roi
  #

  set scene [$sliceGUI GetMRMLScene]
  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLROIListNode"]

  for {set i 0} {$i < $nLists} {incr i} {
    set roiListNode [$scene GetNthNodeByClass $i "vtkMRMLROIListNode"]

    # add an observer on this roi list
    if { [$caller IsA "vtkMRMLScene"] } {
      after idle "$this addROIListObserver $roiListNode"
    }

    if { ![$roiListNode GetVisibility] } {
      continue
    }
    
    #
    # make the roi visible if within 1mm of the slice
    # - place a seed widget and keep track for later deletion
    #
    set nROIs [$roiListNode GetNumberOfROIs]
    for {set f 0} {$f < $nROIs} {incr f} {
      foreach {r a s} [$roiListNode GetNthROIXYZ $f] {}
      set slice [eval $rasToSlice MultiplyPoint $r $a $s 1]
      set z [lindex $slice 2]
      if { [expr abs($z)] <= 1000 } {

        set seedSWidget [SeedSWidget #auto $sliceGUI]
        $seedSWidget place $r $a $s
        $seedSWidget configure -movedCommand "$this seedMovedCallback $seedSWidget $roiListNode $f"
        $seedSWidget configure -movingCommand "$this seedMovingCallback $seedSWidget $roiListNode $f"
        $seedSWidget configure -glyph "Cross"
        $seedSWidget configure -scale 15
        $seedSWidget configure -color [$roiListNode GetColor]
        $seedSWidget configure -selectedColor [$roiListNode GetSelectedColor]
        $seedSWidget configure -opacity [$roiListNode GetOpacity]
        $seedSWidget configure -text [$roiListNode GetNthROILabelText $f]
        $seedSWidget configure -textScale [$roiListNode GetTextScale]
        if { [$roiListNode GetNthROISelected $f] } {
          $seedSWidget configure -selected 1
        }
        lappend _seedSWidgets $seedSWidget

        if { 0 } {
          foreach vert [::RegionsSWidget::GetROIRASVertices $roiListNode $f] {
            set seedSWidget [SeedSWidget #auto $sliceGUI]
            eval $seedSWidget place $vert
            $seedSWidget configure -movedCommand "$this seedMovedCallback $seedSWidget $roiListNode $f"
            $seedSWidget configure -movingCommand "$this seedMovingCallback $seedSWidget $roiListNode $f"
            $seedSWidget configure -glyph "Circle"
            $seedSWidget configure -scale 10
            $seedSWidget configure -color [$roiListNode GetColor]
            $seedSWidget configure -selectedColor [$roiListNode GetSelectedColor]
            $seedSWidget configure -opacity [$roiListNode GetOpacity]
            if { [$roiListNode GetNthROISelected $f] } {
              $seedSWidget configure -selected 1
            }
            lappend _seedSWidgets $seedSWidget
          }
        }

      }
    }
  }

  $rasToSlice Delete
}

itcl::body RegionsSWidget::addROIListObserver {roiListNode} {
  set tag [$roiListNode AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $roiListNode"]
  lappend _roiListObserverTagPairs "$roiListNode $tag"
}

itcl::body RegionsSWidget::seedMovedCallback {seed roiListNode roiIndex} {

  $::slicer3::MRMLScene SaveStateForUndo $roiListNode
  set ras [$seed getRASPosition]
  eval after idle "$roiListNode SetNthROIXYZ $roiIndex $ras"

  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
  set compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
  if { [$compositeNode GetLinkedControl] || [$_interactor GetControlKey] } {
    eval after idle $sliceNode JumpAllSlices $ras
  }
}

itcl::body RegionsSWidget::seedMovingCallback {seed roiListNode roiIndex} {

  # nothing for right now - TODO: could try doing a jumpSlice 
  # while adjusting the roi, but the problem is that this 
  # class (RegionsSWidget) re-creates the seeds on every event
  # from the roi list so the seed would be destroyed during
  # the event handler (which is bad).
  return 

}

#
# Use the Regions Logic to add a point to the active list
#
proc RegionsSWidget::AddROI { r a s } {

  set roiLogic [$::slicer3::RegionsGUI GetLogic]
  # the logic handles saving the state for undo
  set roiIndex [$roiLogic AddROISelected $r $a $s 1]
  $::slicer3::MRMLScene Modified
}


#
# find the roi that is nth from the beginning in the 
# current set of lists in the scene 
#
proc RegionsSWidget::GetNthROIRAS { scene n } {

  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLROIListNode"]

  set jumpRAS ""
  set count 0
  for {set i 0} {$i < $nLists} {incr i} {
    set roiListNode [$scene GetNthNodeByClass $i "vtkMRMLROIListNode"]
    set nROIs [$roiListNode GetNumberOfROIs]
    if { [expr $count + $nROIs] > $n } {
      set index [expr $n - $count]
      set jumpRAS [$roiListNode GetNthROIXYZ $index]
      break
    } else {
      set count [expr $count + $nROIs] 
    }
  }
  return $jumpRAS
}

#
# find the corner vertices in RAS of the current ROI
#
proc RegionsSWidget::GetROIRASVertices { roiListNode n } {

  foreach {r a s} [$roiListNode GetNthROIXYZ $n] {}
  foreach {dr da ds} [$roiListNode GetNthROIRadiusXYZ $n] {}

  foreach ss "[expr $s - $ds] [expr $s + $ds]" {
    foreach aa "[expr $a - $da] [expr $a + $da]" {
      foreach rr "[expr $r - $dr] [expr $r + $dr]" {
        lappend verts "$rr $aa $ss"
      }
    }
  }
  return $verts
}
