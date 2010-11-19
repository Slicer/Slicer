
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
    variable _storedSeedSWidgets ""
    variable _fiducialListObservervations ""
    variable _timeOfLastKeyEvent 0

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method seedMovedCallback {seed fidListNode fidIndex} {}
    method seedMovingCallback {seed fidListNode fidIndex} {}
    method addFiducialListObserver {fidListNode} {}
    method processUpdate {} {} ;# override superclass 

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
  $::slicer3::Broker AddObservation $node DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $node AnyEvent "::SWidget::ProtectedCallback $this processEvent $node AnyEvent"


  set scene [$sliceGUI GetMRMLScene]
  $::slicer3::Broker AddObservation $scene DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $scene AnyEvent "::SWidget::ProtectedCallback $this processEvent $scene"


  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"
  set events {  
    "KeyPressEvent" 
    }
  foreach event $events {
   $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }

  $this processEvent $scene
}


itcl::body FiducialsSWidget::destructor {} {

  foreach seed [concat $_seedSWidgets $_storedSeedSWidgets] {
    ::SWidget::ProtectedDelete ::FiducialsSWidget::$seed
  }
  set _seedSWidgets ""
  set _storedSeedSWidgets ""

  foreach obs $_fiducialListObservervations {
    $slicer3::Broker RemoveObservation $obs
  }
  set _fiducialListObservervations ""
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

  if { $enabled != "true" } {
    return
  }

  if { [info command $caller] == ""} {
      return
  }

  if { [info command $sliceGUI] == "" || [$sliceGUI GetLogic] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    ::SWidget::ProtectedDelete $this
    return
  }

  if { $caller == $sliceGUI } {

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
              # this is the 'backtick' ` key in the upper left of the 
              # keyboard - it seems to have different names on windows vs X
              #
              # - if the slice is linked, then jump all slices to it
              set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
              set compositeNode [[$sliceGUI GetLogic] GetSliceCompositeNode]
              if { [$compositeNode GetLinkedControl] } {
                set jumpMode "all"
              } else {
                set jumpMode "one"
              }
              if { [$_interactor GetControlKey] } {
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
              # first check for key repeats (don't allow more than
              # one fiducial per $fiducialDelay # of seconds)
              # also ignore control-p (brings up python interactor)
              #
              set fiducialDelay 0
              set now [clock seconds]
              if { [expr $now - $_timeOfLastKeyEvent] >= $fiducialDelay && ![$_interactor GetControlKey] } {
                set _timeOfLastKeyEvent $now
                #
                # get the event position and make it relative to a renderer/viewport
                #
                foreach {windowx windowy} [$_interactor GetEventPosition] {}
                foreach {lastwindowx lastwindowy} [$_interactor GetLastEventPosition] {}
                foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}

                if { $windowx < $windoww && $windowy < $windowh } {
                  # only add fiducial if event came from inside render window
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
    }
  }


  #
  # scene changed, so remove all the observers from the fiducial lists
  # - these will be recreated below to match the current scene
  #
  if { [$caller IsA "vtkMRMLScene"] } {
    foreach obs $_fiducialListObservervations {
      $slicer3::Broker RemoveObservation $obs
    }
    set _fiducialListObservervations ""
  }

  $this requestUpdate
}

itcl::body FiducialsSWidget::processUpdate {} {

  chain

  #
  # first - disable old seeds
  # - put them in a list for reuse
  #
  foreach seed $_seedSWidgets {
    $seed place -10000 -10000 -10000
    lappend _storedSeedSWidgets $seed
    $seed configure -visibility 0
  }
  set _seedSWidgets ""


  #
  # now, look through all the fiducial lists for fiducials that
  # are close enough to the current slice node; create seed widgets
  # for those and give them a "moved command" that will set the position of the fiducial
  #
  set scene [$sliceGUI GetMRMLScene]
  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLFiducialListNode"]
  set node [[$sliceGUI GetLogic] GetSliceNode]

  if { $node != "" && $nLists > 0 } {

    #
    # get the rasToSlice for the SliceNode - transforming the fiducial
    # by this matrix will let us easily check the distance from the slice plane
    #
    set rasToSlice [vtkMatrix4x4 New]
    $rasToSlice DeepCopy [$node GetSliceToRAS]
    $rasToSlice Invert

    set rasToRAS [vtkMatrix4x4 New]

    for {set i 0} {$i < $nLists} {incr i} {
      set fidListNode [$scene GetNthNodeByClass $i "vtkMRMLFiducialListNode"]

      # add an observer on this fiducial list (won't re-add existing observers)
      $this addFiducialListObserver $fidListNode

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

        # id we are within 0.5mm of the slice
        if { $z >= -0.5 && $z < [expr 0.5+[lindex [$node GetDimensions] 2]-1]} {

          # get a stored seed widget or create a new one
          if { [llength $_storedSeedSWidgets] > 0 } {
            set seedSWidget [lindex $_storedSeedSWidgets 0]
            set _storedSeedSWidgets [lrange $_storedSeedSWidgets 1 end]
            $seedSWidget configure -visibility 1
          } else {
            set seedSWidget [SeedSWidget #auto $sliceGUI]
          }
          lappend _seedSWidgets $seedSWidget

          $seedSWidget place $r $a $s
          $seedSWidget configure -movedCommand "$this seedMovedCallback $seedSWidget $fidListNode $f"
          $seedSWidget configure -movingCommand "$this seedMovingCallback $seedSWidget $fidListNode $f"
          $seedSWidget configure -glyph $glyphType
          $seedSWidget configure -scale [expr 2. * [$fidListNode GetSymbolScale]]
          $seedSWidget configure -color [$fidListNode GetColor]
          $seedSWidget configure -selectedColor [$fidListNode GetSelectedColor]
          $seedSWidget configure -opacity [$fidListNode GetOpacity]
          $seedSWidget configure -text [$fidListNode GetNthFiducialLabelText $f]
          $seedSWidget configure -textScale [$fidListNode GetTextScale]
          $seedSWidget configure -inactive [$fidListNode GetLocked]
          $seedSWidget configure -selected [$fidListNode GetNthFiducialSelected $f]
          $seedSWidget configure -visibility [$fidListNode GetNthFiducialVisibility $f]
          $seedSWidget processEvent
        }
      }
    }

    $rasToRAS Delete
    $rasToSlice Delete
  }
}

itcl::body FiducialsSWidget::addFiducialListObserver {fidListNode} {
  if { [info command $fidListNode] != "" } {
    foreach obs $_fiducialListObservervations {
      if { [$obs GetSubject] == $fidListNode } {
        return ;# the observer already exists
      }
    }
    # no observer, so add one
    set obs [$::slicer3::Broker AddObservation $fidListNode AnyEvent \
        "::SWidget::ProtectedCallback $this processEvent $fidListNode"]
    lappend _fiducialListObservervations $obs
  }
}

itcl::body FiducialsSWidget::seedMovedCallback {seed fidListNode fidIndex} {

  $::slicer3::MRMLScene SaveStateForUndo $fidListNode
  set ras [$seed getRASPosition]
  eval after idle "$fidListNode SetNthFiducialXYZWorld $fidIndex $ras"

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

    if { [info exists ::slicer3::FiducialsGUI] } {
        set fidLogic [$::slicer3::FiducialsGUI GetLogic]
        # the logic handles saving the state for undo
        set fidIndex [$fidLogic AddFiducialPicked $r $a $s 1]
        $::slicer3::MRMLScene Modified
    } else {
        puts "AddFiducial: No Fiducials GUI found."
    }
}

#
# find the fiducial that is nth from the beginning in the 
# current set of lists in the scene 
# - uses namespace global index to keep track of next index
# - increments/decrements after using the index so we start by jumping to 0th fid
#
proc FiducialsSWidget::JumpAllToNextFiducial { {direction 1} } {
  set sliceNode [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLSliceNode"]
  ::FiducialsSWidget::JumpToNextFiducial $sliceNode "all" $direction
}

proc FiducialsSWidget::JumpToNextFiducial { sliceNode {jumpMode "all"} {direction 1} } {

  # get the RAS location to jump to
  set scene [$sliceNode GetScene]
  set jumpIndex $::FiducialsSWidget::jumpFiducialIndex
  set jumpRAS [::FiducialsSWidget::GetNthFiducialRAS $scene $jumpIndex]

  # increment the fiducial index so we will cycle through.
  # handle wrap around if needed for next time
  set nFids [::FiducialsSWidget::GetNumberOfFiducials $scene]
  incr jumpIndex $direction
  if { $jumpIndex >= $nFids } {
    set jumpIndex 0
  }
  if { $jumpIndex < 0 } {
    set jumpIndex [expr $nFids - 1]
  }
  set ::FiducialsSWidget::jumpFiducialIndex $jumpIndex


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
# Get the total number of fiducials in the scene
#
proc FiducialsSWidget::GetNumberOfFiducials { scene } {

  set nLists [$scene GetNumberOfNodesByClass "vtkMRMLFiducialListNode"]

  set count 0
  for {set i 0} {$i < $nLists} {incr i} {
    set fidListNode [$scene GetNthNodeByClass $i "vtkMRMLFiducialListNode"]
    set nFids [$fidListNode GetNumberOfFiducials]
    set count [expr $count + $nFids]
  }
  return $count
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

