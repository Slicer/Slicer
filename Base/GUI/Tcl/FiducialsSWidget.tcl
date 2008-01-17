
#########################################################
#
if {0} { ;# comment

  FiducialsSWidget  - manages fiducial display for a slice view

# TODO : 

}
#
#########################################################


namespace eval FiducialsSWidget set jumpFiducialIndex 0

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
    if { [info command $fidListNode] != "" } {
      $fidListNode RemoveObserver $tag
    }
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
        set activeKeys "grave quoteleft BackSpace Delete p"
        if { [lsearch $activeKeys $key] != -1 } {
          $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
          $sliceGUI SetGUICommandAbortFlag 1
          switch [$_interactor GetKeySym] {
            "grave" -
            "quoteleft" {
              # this is the 'backtick' key in the upper left of the 
              # keyboard - it seems to have different names on windows vs X
              #
              # - if the slice is linked, then jump all slices to it
              set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
              set compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
              if { [$compositeNode GetLinkedControl] || [$_interactor GetControlKey] } {
                set jumpMode "all"
              } else {
                set jumpMode "one"
              }
              if { [$_interactor GetShiftKey] } {
                set direction -1
              } else {
                set direction 1
              }
              ::FiducialsSWidget::JumpToNextFiducial $sliceNode $jumpMode $direction
            }
            "BackSpace" -
            "Delete" {
              # delete the fiducial if you are over it
              foreach seed $_seedSWidgets {
                if { [$seed getPickState] == "over" } {
                  set cmd [$seed cget -movedCommand]
                  foreach {fid tag seed fidListNode fidIndex} $cmd {}
                  $fidListNode RemoveFiducial $fidIndex
                  return
                }
              }
            }
            "p" {
              # add a fiducial to the current list

              #
              # get the event position and make it relative to a renderer/viewport
              #
              foreach {windowx windowy} [$_interactor GetEventPosition] {}
              foreach {lastwindowx lastwindowy} [$_interactor GetLastEventPosition] {}
              foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}

              set pokedRenderer [$_interactor FindPokedRenderer $windowx $windowy]
              set renderer0 [$_renderWidget GetRenderer]

              foreach {x y z} [$this dcToXYZ $windowx $windowy] {}
              $this queryLayers $x $y $z
              set xyToRAS [$_sliceNode GetXYToRAS]
              set ras [$xyToRAS MultiplyPoint $x $y $z 1]

              foreach {r a s t} $ras {}
              FiducialsSWidget::AddFiducial $r $a $s
            }
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
      if { [info command $fidListNode] != "" } {
        after idle "::SWidget::ProtectedCallback $fidListNode RemoveObserver $tag"
      } 
    }
  }


  #
  # now, look through all the fiducial lists for fiducials that
  # are close enough to the current slice node; create seed widgets
  # for those and give them a "moved command" that will set the position of the fiducial
  #
  set scene [$sliceGUI GetMRMLScene]
  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLFiducialListNode"]

  if { $nLists > 0 } {

    #
    # get the rasToSlice for the SliceNode - transforming the fiducial
    # by this matrix will let us easily check the distance from the slice plane
    #
    set node [[$sliceGUI GetLogic] GetSliceNode]
    set rasToSlice [vtkMatrix4x4 New]
    $rasToSlice DeepCopy [$node GetSliceToRAS]
    $rasToSlice Invert

    set rasToRAS [vtkMatrix4x4 New]

    for {set i 0} {$i < $nLists} {incr i} {
      set fidListNode [$scene GetNthNodeByClass $i "vtkMRMLFiducialListNode"]

      # add an observer on this fiducial list
      if { [$caller IsA "vtkMRMLScene"] } {
        $this addFiducialListObserver $fidListNode
      }

      if { ![$fidListNode GetVisibility] } {
        continue
      }
      
      $rasToRAS Identity
      set transformNode [$::slicer3::MRMLScene GetNodeByID [$fidListNode GetTransformNodeID]]
      if { $transformNode != "" } {
        $transformNode GetMatrixTransformToWorld $rasToRAS
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
        foreach {r a s t} [$rasToRAS MultiplyPoint $r $a $s 1] {}
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

    $rasToRAS Delete
    $rasToSlice Delete
  }
}

itcl::body FiducialsSWidget::addFiducialListObserver {fidListNode} {
  if { [info command $fidListNode] != "" } {
    set tag [$fidListNode AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $fidListNode"]
    lappend _fiducialListObserverTagPairs "$fidListNode $tag"
  }
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

  # make sure everything gets updated before adding another fiducial
  update
}

#
# find the fiducial that is nth from the beginning in the 
# current set of lists in the scene 
# - uses namespace global index to keep track of next index
#
proc FiducialsSWidget::JumpAllToNextFiducial { {direction 1} } {
  set sliceNode [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLSliceNode"]
  ::FiducialsSWidget::JumpToNextFiducial $sliceNode "all" $direction
}

proc FiducialsSWidget::JumpToNextFiducial { sliceNode {jumpMode "all"} {direction 1} } {

  # increment the fiducial
  # index so we will cycle through.
  #
  set scene [$sliceNode GetScene]
  set jumpIndex $::FiducialsSWidget::jumpFiducialIndex
  set jumpRAS [::FiducialsSWidget::GetNthFiducialRAS $scene $jumpIndex]

  # handle wrap around if needed
  # - note: doesn't wrap around from beginning to end, just end to beginning
  if { $jumpRAS == "" } {
    set jumpRAS [::FiducialsSWidget::GetNthFiducialRAS $scene 0]
    set ::FiducialsSWidget::jumpFiducialIndex 1
  } else {
    incr ::FiducialsSWidget::jumpFiducialIndex $direction
    if { $::FiducialsSWidget::jumpFiducialIndex < 0 } {
      set ::FiducialsSWidget::jumpFiducialIndex 0
    }
  }

  # now jump the slice(s)
  # - if the slice is linked, then jump all slices to it
  if { $jumpRAS != "" } {
    eval $sliceNode JumpSlice $jumpRAS
    if { $jumpMode == "all" } {
      eval $sliceNode JumpAllSlices $jumpRAS
    }
  }
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

