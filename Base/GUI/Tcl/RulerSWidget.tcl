package require Itcl

#########################################################
#
if {0} { ;# comment

  RulerSWidget a class for slicer rulers in 2D


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             RulerSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class RulerSWidget] == "" } {

  itcl::class RulerSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable rulerID ""

    variable _rulerNode ""
    variable _rulerNodeObservation ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method updateMRMLFromWidget {} {}
    method updateWidgetFromMRML {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body RulerSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set o(lineWidget) [vtkNew vtkLineWidget2]
  $o(lineWidget) SetInteractor [$_renderWidget GetRenderWindowInteractor]
  [$o(lineWidget) GetRepresentation] PlaceWidget 0 1 0 1 0 1
  $o(lineWidget) On

  # observe the slice GUI for user input events
  # TODO: no mouse events until we start interacting with the slice nodes
  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"

  # observe the slice node for direct manipulations of MRML
  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
  $::slicer3::Broker AddObservation $sliceNode DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $sliceNode AnyEvent "::SWidget::ProtectedCallback $this processEvent $sliceNode AnyEvent"

  # observe the widget for interaction
  $::slicer3::Broker AddObservation $o(lineWidget) StartInteractionEvent "::SWidget::ProtectedCallback $this processEvent $o(lineWidget) StartInteractionEvent"
  $::slicer3::Broker AddObservation $o(lineWidget) EndInteractionEvent "::SWidget::ProtectedCallback $this processEvent $o(lineWidget) EndInteractionEvent"
  $::slicer3::Broker AddObservation $o(lineWidget) InteractionEvent "::SWidget::ProtectedCallback $this processEvent $o(lineWidget) InteractionEvent"

}

itcl::body RulerSWidget::destructor {} {
  $o(lineWidget) Off
}

#
# when told what ruler to observe...
#
itcl::configbody RulerSWidget::rulerID {
  # find the ruler node
  set rulerNode [$::slicer3::MRMLScene GetNodeByID $rulerID]
  if { $rulerNode == "" } {
    #error "no node for id $rulerID"
    return
  }

  # remove observation from old node and add to new node
  # then set input to pipeline
  if { $rulerNode != $_rulerNode } {
    if { $_rulerNodeObservation != "" } {
      $::slicer3::Broker RemoveObservation $_rulerNodeObservation
    }
    set _rulerNode $rulerNode
    if { $_rulerNode != "" } {
      set _rulerNodeObservation [$::slicer3::Broker AddObservation $_rulerNode AnyEvent "::SWidget::ProtectedCallback $this processEvent $_rulerNode AnyEvent"]
    }
  }

  $this updateWidgetFromMRML
  [$sliceGUI GetSliceViewer] RequestRender
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body RulerSWidget::updateWidgetFromMRML { } {

  if { $_rulerNode == "" } {
    return
  }

  #
  # set the two endpoints in screen space
  #

  $this queryLayers 0 0 0
  set lineRep [$o(lineWidget) GetRepresentation]
  eval $lineRep SetPoint1DisplayPosition [$this rasToXYZ [$_rulerNode GetPosition1]]
  eval $lineRep SetPoint2DisplayPosition [$this rasToXYZ [$_rulerNode GetPosition2]]

  #
  # set up the colours
  #
  eval [[$lineRep GetPoint1Representation] GetProperty] SetColor [$_rulerNode GetPointColour]
  # for now, both ends get coloured the same, so only set the first one as that's all that can be set via the gui, this will change with vtk 5.6
#  eval [[$lineRep GetPoint2Representation] GetProperty] SetColor [$_rulerNode GetPoint2Colour]
  eval [$lineRep GetLineProperty] SetColor [$_rulerNode GetLineColour]
}


itcl::body RulerSWidget::updateMRMLFromWidget { } {

  if { $_rulerNode == "" } {
    return
  }

  #
  # set the two endpoints in screen space
  #
  $this queryLayers 0 0 0
  set lineRep [$o(lineWidget) GetRepresentation]
  $_rulerNode DisableModifiedEventOn
  $lineRep GetPoint1DisplayPosition 0 0 0
  $lineRep GetPoint2DisplayPosition 0 0 0
  eval $_rulerNode SetPosition1 [$this xyzToRAS [$lineRep GetPoint1DisplayPosition]]
  eval $_rulerNode SetPosition2 [$this xyzToRAS [$lineRep GetPoint2DisplayPosition]]
  $_rulerNode DisableModifiedEventOff
  $_rulerNode InvokePendingModifiedEvent
}


itcl::body RulerSWidget::processEvent { {caller ""} {event ""} } {

  if { [info command $sliceGUI] == "" || [$sliceGUI GetLogic] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this updateWidgetFromMRML
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  $sliceGUI SetGrabID ""

  set sliceNode [[$sliceGUI GetLogic] GetSliceNode]
  if { $caller == $_rulerNode || $caller == $sliceNode } {
    $this updateWidgetFromMRML
    [$sliceGUI GetSliceViewer] RequestRender
  }

  if { $caller == $o(lineWidget) } {
    if { 1 || $event == "StartInteractionEvent" } {
          if { $event == "StartInteractionEvent" } {
              #--- now adjust the interactionMode.
              #--- This implementation of mouse modes turns on
              #--- 'pick' mode when a ruler endpoint is picked.
              set interactionNode [ $::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLInteractionNode ]
              if { $interactionNode != "" } {
                  set mode [$interactionNode GetCurrentInteractionMode]
                  set modeString [$interactionNode GetInteractionModeAsString $mode]
                  set pickPersistence [ $interactionNode GetPickModePersistence]
                  if { $pickPersistence == 0 } {
                      $interactionNode SetLastInteractionMode $mode
                  }
                  $interactionNode SetCurrentInteractionMode [ $interactionNode GetInteractionModeByString "PickManipulate" ]
              }
          }
        $this updateMRMLFromWidget
    }
      [$sliceGUI GetSliceViewer] RequestRender
  }



  if { $caller == $o(lineWidget) } {
    if { 1 || $event == "EndInteractionEvent" } {
        if { $event == "EndInteractionEvent" } {
            #--- now adjust the interactionMode.        
            set interactionNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLInteractionNode]
            # Reset interaction mode to default viewtransform
            # mode if user has not selected a persistent pick or place.
            if { $interactionNode != "" } {
                if { [$interactionNode GetCurrentInteractionMode] != [$interactionNode GetInteractionModeByString "ViewTransform"] } {
                    set pickPersistence [ $interactionNode GetPickModePersistence]
                    set placePersistence [ $interactionNode GetPlaceModePersistence ]
                    if { $pickPersistence == 0 && $placePersistence == 0 } {
                        $interactionNode SetCurrentInteractionMode [ $interactionNode GetInteractionModeByString "ViewTransform" ]
                        $interactionNode SetPickModePersistence 0
                        $interactionNode SetPlaceModePersistence 0
                    }
                }
            }
        }
        $this updateMRMLFromWidget
    }
      [$sliceGUI GetSliceViewer] RequestRender
  }


}
