
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
    method updateWidgetFromNode {sliceNode planeWidget} {}
    method updateNodeFromWidget {sliceNode planeWidget} {}
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
  set rwi [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindowInteractor]

  # Update the size of the render window interactor to match that of the render window
  set rw [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow]
  $rwi UpdateSize [lindex [$rw GetSize] 0] [lindex [$rw GetSize] 1]
  # the problem is that it gets reset, so there's another call in the visibility toggle

  $o(plane) SetInteractor $rwi
  $o(plane) SetDrawPlane 0
  $o(plane) PlaceWidget -100 100 -100 100 -100 100

  $::slicer3::Broker AddObservation $o(plane) AnyEvent "::SWidget::ProtectedCallback $this processEvent $o(plane) AnyEvent"

}


itcl::body SlicePlaneSWidget::destructor {} {
}



# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

#
# handle scene and slice node events
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

  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]

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
              $this updateWidgetFromNode $sliceNode $o(plane)
              set visible [$sliceNode GetWidgetVisible]
              $sliceNode SetWidgetVisible [expr !$visible]
              if { [expr !$visible] } {
                  set rwi [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindowInteractor]
                  # Update the size of the render window interactor to match that of the render window
                  set rw [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow]
                  if {[$rw GetSize] != [$rwi GetSize]} { 
                      # puts "Updating interactor size after making [$sliceNode GetName] plane widget visible, rw size = [$rw GetSize], rwi size is currently = [$rwi GetSize]"
                      $rwi UpdateSize [lindex [$rw GetSize] 0] [lindex [$rw GetSize] 1]
                  }
              }
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

  #
  # widget manipulated, so update the slice node to match
  #
  if { $caller == $o(plane) } {
    $this updateNodeFromWidget $sliceNode $o(plane)
  }

  #
  # slice node modified, so update the widget to match
  #
  if { $caller == $sliceNode } {
    if { [$sliceNode GetWidgetVisible] } {
      $o(plane) On
    } else {
      $o(plane) Off
    }
    $this updateWidgetFromNode $sliceNode $o(plane)
  }

}

itcl::body SlicePlaneSWidget::updateNodeFromWidget {sliceNode planeWidget} {

  set sliceToRAS [$sliceNode GetSliceToRAS]

  # zero out current translation
  $sliceToRAS SetElement 0 3  0
  $sliceToRAS SetElement 1 3  0
  $sliceToRAS SetElement 2 3  0

  #
  # rotate so slice normal matches widget normal
  #
  set sliceNormal [list \
    [$sliceToRAS GetElement 0 2] \
    [$sliceToRAS GetElement 1 2] \
    [$sliceToRAS GetElement 2 2] ]
  set widgetNormal [$o(plane) GetNormal]

  set radiansToDegrees 57.2957795131
  set xyz [SlicePlaneSWidget::Cross $sliceNormal $widgetNormal]
  set w [expr  1. * $radiansToDegrees * acos([SlicePlaneSWidget::Dot $sliceNormal $widgetNormal])]

  set transform [vtkTransform New]
  $transform PostMultiply
  $transform SetMatrix $sliceToRAS
  eval $transform RotateWXYZ $w $xyz
  $transform GetMatrix $sliceToRAS
  $transform Delete

  # insert widget translation
  $sliceToRAS SetElement 0 3 [lindex [$o(plane) GetOrigin] 0]
  $sliceToRAS SetElement 1 3 [lindex [$o(plane) GetOrigin] 1]
  $sliceToRAS SetElement 2 3 [lindex [$o(plane) GetOrigin] 2]

  $sliceNode UpdateMatrices
}

itcl::body SlicePlaneSWidget::updateWidgetFromNode {sliceNode planeWidget} {

  set sliceToRAS [$sliceNode GetSliceToRAS]
  $planeWidget SetOrigin \
    [$sliceToRAS GetElement 0 3] \
    [$sliceToRAS GetElement 1 3] \
    [$sliceToRAS GetElement 2 3]
  $planeWidget SetNormal \
    [$sliceToRAS GetElement 0 2] \
    [$sliceToRAS GetElement 1 2] \
    [$sliceToRAS GetElement 2 2]
}

#-------------------------------------------------------------------------------
#- Helper procs

proc SlicePlaneSWidget::Cross { x y } {
  foreach {x0 x1 x2} $x {}
  foreach {y0 y1 y2} $y {}
  set z0 [expr $x1*$y2 - $x2*$y1]
  set z1 [expr $x2*$y0 - $x0*$y2]
  set z2 [expr $x0*$y1 - $x1*$y0]
  return [list $z0 $z1 $z2]
}

proc SlicePlaneSWidget::Dot { x y } {
  set sum 0.0
  foreach xx $x yy $y {
    set sum [expr $sum + $xx*$yy]
  }
  return $sum
}

proc TestSlicePlaneSWidget {} {
  itcl::delete class SlicePlaneSWidget
  source $::Slicer3_HOME/lib/Slicer3/SlicerBaseGUI/Tcl/SlicePlaneSWidget.tcl

  set sliceGUI [lindex [vtkSlicerSliceGUI ListInstances] 0]
  SlicePlaneSWidget #auto $sliceGUI
}
