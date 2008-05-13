
#########################################################
#
if {0} { ;# comment

  SlicePlaneSWidget  - manages PlaneWidget for slice 

# TODO : 

}
#
#########################################################


# ------------------------------------------------------------------
#                             SlicePlaneSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SlicePlaneSWidget] == "" } {

  itcl::class SlicePlaneSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}
    
    # methods
    method processEvent {{caller ""} {event ""}} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body SlicePlaneSWidget::constructor {sliceGUI} {

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
  $::slicer3::Broker AddObservation $scene AnyEvent "::SWidget::ProtectedCallback $this processEvent $scene AnyEvent"


  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"
  set events {  
    "KeyPressEvent" 
    }
  foreach event $events {
    $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }

  set o(plane) [vtkNew vtkImplicitPlaneWidget]
  $::slicer3::Broker AddObservation $o(plane) AnyEvent "::SWidget::ProtectedCallback $this processEvent $o(plane) AnyEvent"

  # TODO: generalize for multiple viewers
  set rwi [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindowInteractor]
  $o(plane) SetInteractor $rwi
  $o(plane) PlaceWidget -100 100 -100 100 -100 100
  $o(plane) On

  $this processEvent $scene

}


itcl::body SlicePlaneSWidget::destructor {} {
}



# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

#
# handle scene and slice node events
# - for now, we need to always review fiducials lists in scene
# - create SeedSWidgets for any SlicePlane that are close enough to slice
#

itcl::body SlicePlaneSWidget::processEvent { {caller ""} {event ""} } {

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

    switch $event {
      "KeyPressEvent" { 
        set key [$_interactor GetKeySym]
        set activeKeys "o"
        if { [lsearch $activeKeys $key] != -1 } {
          $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
          $sliceGUI SetGUICommandAbortFlag 1
          switch [$_interactor GetKeySym] {
            "o" {
              # toggle widget on off
            }
          }
        }
      }
    }
  }


  #
  # scene changed
  #
  if { [$caller IsA "vtkMRMLScene"] } {
  }


  set scene [$sliceGUI GetMRMLScene]
  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]

  if { $caller == $o(plane) } {
    set sliceToRAS [$sliceNode GetSliceToRAS]
    set p [vtkPlane New]
    $o(plane) GetPlane $p
    $sliceToRAS SetElement 0 3 [lindex [$p GetOrigin] 0]
    $sliceToRAS SetElement 1 3 [lindex [$p GetOrigin] 1]
    $sliceToRAS SetElement 2 3 [lindex [$p GetOrigin] 2]
    $p Delete
    $sliceNode UpdateMatrices
  }

  if { $caller == $sliceNode } {
    set sliceToRAS [$sliceNode GetSliceToRAS]
    $o(plane) SetOrigin \
      [$sliceToRAS GetElement 0 3] \
      [$sliceToRAS GetElement 1 3] \
      [$sliceToRAS GetElement 2 3]
  }

}

proc TestSlicePlaneSWidget {} {
  itcl::delete class SlicePlaneSWidget
  source $::SLICER_BUILD/../Slicer3/Base/GUI/Tcl/SlicePlaneSWidget.tcl

  set sliceGUI [lindex [vtkSlicerSliceGUI ListInstances] 0]
  SlicePlaneSWidget #auto $sliceGUI
}
