
package require Itcl

#########################################################
#
if {0} { ;# comment

  VolumeDisplaySWidget a class for slicer control window/level of
  volumes displayed in the associated slice viewer


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             VolumeDisplaySWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class VolumeDisplaySWidget] == "" } {

  itcl::class VolumeDisplaySWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    variable _startWindow 0
    variable _startLevel 0
    variable _startPosition "0 0"
    variable _currentPosition "0 0"

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method positionActors {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body VolumeDisplaySWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"

  $this processEvent

  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"
  foreach event {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent} {
    $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }
}

itcl::body VolumeDisplaySWidget::destructor {} {

  if { [info command $_renderer] != "" } {
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
  }
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body VolumeDisplaySWidget::positionActors { } {
  # nothing for now...
}

itcl::body VolumeDisplaySWidget::processEvent { {caller ""} {event ""} } {

  if { $enabled != "true" } {
    return
  }

  if { [info command $sliceGUI] == "" || [$sliceGUI GetLogic] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  # do nothing if other widget has grabbed the events
  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    return 
  }

  # if you don't have a volume to operate on, do nothing
  $this queryLayers 0 0
  if { $_layers(background,node) == "" } {
    return
  }
  set displayNode [$_layers(background,node) GetDisplayNode]
  if { $displayNode  == "" } {
    return
  }

  # map x, y offset of drag to a change in the window/level of the background volume
  switch $event {
    "LeftButtonPressEvent" {
      if { [$sliceGUI GetActiveLeftButtonTool] == "" } {
        set interactionNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLInteractionNode]
        if { $interactionNode != "" } {
          set mode [$interactionNode GetCurrentInteractionMode]
          set modeString [$interactionNode GetInteractionModeAsString $mode]
          if { $modeString != "ViewTransform" } {
            return
          }
          set _actionState "dragging"
          $sliceGUI SetGrabID $this
          set _startPosition [$_interactor GetEventPosition]
          set _startWindow [$displayNode GetWindow]
          set _startLevel [$displayNode GetLevel]
          $displayNode SetAutoWindowLevel 0
          $::slicer3::MRMLScene SaveStateForUndo $displayNode
          $this statusText "Drag to adjust Window/Level for [$_layers(background,node) GetName]"

          # don't update annotations while effect is active
          $this configure -forceAnnotationsOff 1
        } else {
          set _actionState ""
        }
      }
    }
    "MouseMoveEvent" {
      switch $_actionState {
        "dragging" {
          $this requestDelayedAnnotation 
          set _currentPosition [$_interactor GetEventPosition]
          foreach {startx starty} $_startPosition {currx curry} $_currentPosition {}
          set offx [expr $currx - $startx]
          set offy [expr $curry - $starty]
          set imageData [$_layers(background,node) GetImageData]
          set range [$imageData GetScalarRange]
          foreach {rangelo rangehi} $range {}
          # each pixel should be 1/500 of the scalar range
          set gain [expr ($rangehi - $rangelo) / 500.]
          set window [expr $_startWindow + ($gain * $offx)]
          set level [expr $_startLevel + ($gain * $offy)]
          if { $window < 0 } { set window 0 }
          $displayNode SetDisableModifiedEvent 1
          $displayNode SetWindow $window
          $displayNode SetLevel $level
          $displayNode SetDisableModifiedEvent 0
          $displayNode InvokePendingModifiedEvent
          [$sliceGUI GetSliceViewer] RequestRender
          set window [format %.1f $window]
          set level [format %.1f $level]
          $this statusText "Window/Level: $window/$level for [$_layers(background,node) GetName]"
        }
      }
    }
    "LeftButtonReleaseEvent" {
      # TODO: this is a hack - make up for the GUI not observing mrml by explicitly
      # telling it to update from the changes we made to the mrml node
      if { [info exists ::slicer3::VolumesGUI] } {
        set widget [$::slicer3::VolumesGUI GetVolumeDisplayWidget]
        if { $widget != "" } {
          [$::slicer3::VolumesGUI GetVolumeDisplayWidget] UpdateWidgetFromMRML
        }
      }
      # don't update annotations while effect is active
      $this configure -forceAnnotationsOff 1
      set _actionState ""
      $sliceGUI SetGrabID ""
      set _description ""
      $this statusText ""
    }
  }

}
