
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

    # methods
    method processEvent { caller } {}
    method seedMovedCallback {seed fidListNode fidIndex} {}
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
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "$this processEvent $node"]

  set scene [$sliceGUI GetMRMLScene]
  lappend _sceneObserverTags [$scene AddObserver DeleteEvent "itcl::delete object $this"]
  lappend _sceneObserverTags [$scene AddObserver AnyEvent "$this processEvent $scene"]

  $this processEvent $scene
}


itcl::body FiducialsSWidget::destructor {} {

  foreach pair $_fiducialListObserverTagPairs {
    foreach {fidListNode tag} $pair {}
    $fidListNode RemoveObserver $tag
  }

  if { [info command $_sliceNode] != "" } {
    foreach tag $_nodeObserverTags {
      $_sliceNode RemoveObserver $tag
    }
  }

  set scene [$sliceGUI GetMRMLScene]
  if { [info command $scene] != "" } {
    foreach tag $_sceneObserverTags {
      $scene RemoveObserver $tag
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

itcl::body FiducialsSWidget::processEvent { caller } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  #
  # first, remove the old seeds
  # - for reasons unknown as yet, deleting the object from within the 
  #   event handler causes a hang -- so set it up to be deleted later
  #
  foreach seed $_seedSWidgets {
    $seed place -10000 -10000 -10000
    after idle "itcl::delete object ::FiducialsSWidget::$seed;"
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
    # make the fiducial visible if within 1mm of the slice
    # - place a seed widget and keep track for later deletion
    #
    set nFids [$fidListNode GetNumberOfFiducials]
    for {set f 0} {$f < $nFids} {incr f} {
      foreach {r a s} [$fidListNode GetNthFiducialXYZ $f] {}
      set slice [eval $rasToSlice MultiplyPoint $r $a $s 1]
      set z [lindex $slice 2]
      if { [expr abs($z)] <= 1 } {
        set seedSWidget [SeedSWidget #auto $sliceGUI]
        $seedSWidget place $r $a $s
        $seedSWidget configure -movedCommand "$this seedMovedCallback $seedSWidget $fidListNode $f"
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
  set tag [$fidListNode AddObserver AnyEvent "$this processEvent $fidListNode"]
  lappend _fiducialListObserverTagPairs "$fidListNode $tag"
}

itcl::body FiducialsSWidget::seedMovedCallback {seed fidListNode fidIndex} {

  $::slicer3::MRMLScene SaveStateForUndo $fidListNode
  set ras [$seed getRASPosition]
  eval after idle "$fidListNode SetNthFiducialXYZ $fidIndex $ras"
}

#
# Use the Fiducials Logic to add a point to the active list
#
proc FiducialsSWidget::AddFiducial { r a s } {

  set fidLogic [$::slicer3::FiducialsGUI GetLogic]
  # the logic handles saving the state for undo
  set fidIndex [$fidLogic AddFiducial $r $a $s]
  $::slicer3::MRMLScene Modified
}

