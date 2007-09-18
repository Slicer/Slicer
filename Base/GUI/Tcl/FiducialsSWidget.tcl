
#########################################################
#
if {0} { ;# comment

  FiducialsSWidget  - manages fiducial display for a slice view

# TODO : 

}
#
#########################################################
# ------------------------------------------------------------------
#                             FiducialsSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class FiducialsSWidget] == "" } {

  itcl::class FiducialsSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}
    
    # a list of seeds - the callback info includes the mapping to list and index
    variable _seedSWidgets ""
    variable _sceneObserverTags ""
    variable _fiducialListObserverTagPairs ""
    variable _jumpFiducialIndex 0

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method seedMovedCallback {seed fidListNode fidIndex} {}
    method seedMovingCallback {seed fidListNode fidIndex} {}
    method addFiducialListObserver {fidListNode} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body FiducialsSWidget::constructor {sliceGUI} {

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


itcl::body FiducialsSWidget::destructor {} {

  if { [info command $sliceGUI] != "" } {
    foreach tag $_guiObserverTags {
      $sliceGUI RemoveObserver $tag
    }
  }

  foreach pair $_fiducialListObserverTagPairs {
    foreach {fidListNode tag} $pair {}
    $fidListNode RemoveObserver $tag
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
# - for now, we need to always review fiducials lists in scene
# - create SeedSWidgets for any Fiducials that are close enough to slice
#

itcl::body FiducialsSWidget::processEvent { {caller ""} {event ""} } {

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
    switch $event {
      "KeyPressEvent" { 
        set key [$_interactor GetKeySym]
        if { [lsearch "grave quoteleft " $key] != -1 } {
          $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
          $sliceGUI SetGUICommandAbortFlag 1
          puts "fiducial eating $key"
          switch [$_interactor GetKeySym] {
            "grave" -
            "quoteleft" {
              # this is the 'backtick' key in the upper left of the 
              # keyboard - it seems to have different names on windows vs X
              #
              # increment the fiducial
              # index so we will cycle through.
              #
              set scene [$sliceGUI GetMRMLScene]
              set jumpRAS [::FiducialsSWidget::GetNthFiducialRAS $scene $_jumpFiducialIndex]
              # handle wrap around if needed
              if { $jumpRAS == "" } {
                set jumpRAS [::FiducialsSWidget::GetNthFiducialRAS $scene 0]
                set _jumpFiducialIndex 1
              } else {
                incr _jumpFiducialIndex
              }

              # now jump the slice(s)
              # - if the slice is linked, then jump all slices to it
              if { $jumpRAS != "" } {
                set node [[$sliceGUI GetLogic] GetSliceNode]
                eval $node JumpSlice $jumpRAS
                set compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
                if { [$compositeNode GetLinkedControl] || [$_interactor GetControlKey] } {
                  eval $node JumpAllSlices $jumpRAS
                }
              }
            }
          } else {
            # puts "fiducial ignoring $key"
          }
        }
      }
    }
  }

  #
  # first, remove the old seeds
  # - for reasons unknown as yet, deleting the object from within the 
  #   event handler causes a hang -- so set it up to be deleted later
  #
  foreach seed $_seedSWidgets {
    $seed place -10000 -10000 -10000
    after idle "::SWidget::ProtectedDelete ::FiducialsSWidget::$seed;"
    [$sliceGUI GetSliceViewer] RequestRender
  }
  set _seedSWidgets ""


  #
  # scene changed, so remove all the observers from the fiducial lists
  # - these will be recreated below to match the current scene
  #
  if { [$caller IsA "vtkMRMLScene"] } {
    foreach pair $_fiducialListObserverTagPairs {
      foreach {fidListNode tag} $pair {}
      after idle "$fidListNode RemoveObserver $tag"
    }
  }

  #
  # get the rasToSlice for the SliceNode - transforming the fiducial
  # by this matrix will let us easily check the distance from the slice plane
  #
  set node [[$sliceGUI GetLogic] GetSliceNode]
  set rasToSlice [vtkMatrix4x4 New]
  $rasToSlice DeepCopy [$node GetSliceToRAS]
  $rasToSlice Invert


  #
  # now, look through all the fiducial lists for fiducials that
  # are close enough to the current slice node; create seed widgets
  # for those and give them a "moved command" that will set the position of the fiducial
  #

  set scene [$sliceGUI GetMRMLScene]
  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLFiducialListNode"]

  for {set i 0} {$i < $nLists} {incr i} {
    set fidListNode [$scene GetNthNodeByClass $i "vtkMRMLFiducialListNode"]

    # add an observer on this fiducial list
    if { [$caller IsA "vtkMRMLScene"] } {
      after idle "$this addFiducialListObserver $fidListNode"
    }

    if { ![$fidListNode GetVisibility] } {
      continue
    }
    
    set glyphType [$fidListNode GetGlyphTypeAsString]
    set indexOf2D [string last "2D" $glyphType]
    if { $indexOf2D != -1 } {
      set glyphType [string range $glyphType 0 [incr indexOf2D -1]]
    }

    #
    # make the fiducial visible if within half a slicewidth of the slice
    # - place a seed widget and keep track for later deletion
    #
    set nFids [$fidListNode GetNumberOfFiducials]
    for {set f 0} {$f < $nFids} {incr f} {
      foreach {r a s} [$fidListNode GetNthFiducialXYZ $f] {}
      set xyz [$this rasToXYZ "$r $a $s"]
      foreach {x y z} $xyz {}
      if { $z >= -0.5 && $z < [expr 0.5+[lindex [$node GetDimensions] 2]-1]} {
        set seedSWidget [SeedSWidget #auto $sliceGUI]
        $seedSWidget place $r $a $s
        $seedSWidget configure -movedCommand "$this seedMovedCallback $seedSWidget $fidListNode $f"
        $seedSWidget configure -movingCommand "$this seedMovingCallback $seedSWidget $fidListNode $f"
        $seedSWidget configure -glyph $glyphType
        $seedSWidget configure -scale [$fidListNode GetSymbolScale]
        $seedSWidget configure -color [$fidListNode GetColor]
        $seedSWidget configure -selectedColor [$fidListNode GetSelectedColor]
        $seedSWidget configure -opacity [$fidListNode GetOpacity]
        $seedSWidget configure -text [$fidListNode GetNthFiducialLabelText $f]
        $seedSWidget configure -textScale [$fidListNode GetTextScale]
        if { [$fidListNode GetNthFiducialSelected $f] } {
          $seedSWidget configure -selected 1
        }
        lappend _seedSWidgets $seedSWidget
      }
    }
  }

  $rasToSlice Delete
}

itcl::body FiducialsSWidget::addFiducialListObserver {fidListNode} {
  set tag [$fidListNode AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $fidListNode"]
  lappend _fiducialListObserverTagPairs "$fidListNode $tag"
}

itcl::body FiducialsSWidget::seedMovedCallback {seed fidListNode fidIndex} {

  $::slicer3::MRMLScene SaveStateForUndo $fidListNode
  set ras [$seed getRASPosition]
  eval after idle "$fidListNode SetNthFiducialXYZ $fidIndex $ras"

  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
  set compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
  if { [$compositeNode GetLinkedControl] || [$_interactor GetControlKey] } {
    eval after idle $sliceNode JumpAllSlices $ras
  }
}

itcl::body FiducialsSWidget::seedMovingCallback {seed fidListNode fidIndex} {

  # nothing for right now - TODO: could try doing a jumpSlice 
  # while adjusting the fiducial, but the problem is that this 
  # class (FiducialsSWidget) re-creates the seeds on every event
  # from the fiducial list so the seed would be destroyed during
  # the event handler (which is bad).
  return 

}

#
# Use the Fiducials Logic to add a point to the active list
#
proc FiducialsSWidget::AddFiducial { r a s } {

  set fidLogic [$::slicer3::FiducialsGUI GetLogic]
  # the logic handles saving the state for undo
  set fidIndex [$fidLogic AddFiducialSelected $r $a $s 1]
  $::slicer3::MRMLScene Modified
}

#
# find the fiducial that is nth from the beginning in the 
# current set of lists in the scene 
#
proc FiducialsSWidget::GetNthFiducialRAS { scene n } {

  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLFiducialListNode"]

  set jumpRAS ""
  set count 0
  for {set i 0} {$i < $nLists} {incr i} {
    set fidListNode [$scene GetNthNodeByClass $i "vtkMRMLFiducialListNode"]
    set nFids [$fidListNode GetNumberOfFiducials]
    if { [expr $count + $nFids] > $n } {
      set index [expr $n - $count]
      set jumpRAS [$fidListNode GetNthFiducialXYZ $index]
      break
    } else {
      set count [expr $count + $nFids] 
    }
  }
  return $jumpRAS
}

