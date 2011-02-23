
#########################################################
#
if {0} { ;# comment

  SliceSWidget  - manages slice plane interactions

# TODO : 

}
#
#########################################################
# ------------------------------------------------------------------
#                             SliceSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SliceSWidget] == "" } {

  itcl::class SliceSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable sliceStep 1  ;# the size of the slice increment/decrement
    public variable calculateAnnotations 1  ;# include annotation calculation (turned off for slicer4)
    #
    # These are widgets that track the state of a node in the MRML Scene
    # - for each of these we update things each time a node or scene is added
    #
    public variable swidgetTypes {
      { ModelSWidget -modelID vtkMRMLModelNode }
      { RulerSWidget -rulerID vtkMRMLMeasurementsRulerNode }
    }

    variable _actionStartRAS "0 0 0"
    variable _actionStartXY "0 0"
    variable _actionStartViewportOrigin "0 0"
    variable _actionStartWindowXY "0 0"
    variable _actionStartFOV "250 250 250"
    variable _actionLink 0
    variable _actionStartOrientation ""
    variable _actionModifier ""
    variable _swidgets ""
    variable _inWidget 0
    variable _lastLabelOpacity 1.0
    variable _lastForegroundOpacity 1.0

    # methods
    method updateSWidgets {} {}
    method resizeSliceNode {} {}
    method processEvent {{caller ""} {event ""}} {}
    method updateAnnotation {r a s} {}
    method updateAnnotations {r a s} {}
    method updateStatusAnnotation {r a s} {}
    method incrementSlice {} {}
    method decrementSlice {} {}
    method moveSlice { delta } {}
    method jumpSlice { r a s } {}
    method jumpOtherSlices { r a s } {}
    method getLinkedSliceLogics {} {}
    method getLinkedSliceGUIs {} {}
    method addSliceModelSWidgets {} {}
    method isCompareViewer {} {}
    method isCompareViewMode {} {}
    method getSliceSWidgetForGUI { gui } {}
    method getInWidget {} {}
    method startTranslate { x y  windowx windowy  rox roy  ras } {}
    method endTranslate {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body SliceSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI

  # create matrices to store transform state
  set o(storeXYToRAS) [$this vtkNew vtkMatrix4x4]
  set o(storeSliceToRAS) [$this vtkNew vtkMatrix4x4]
  set o(scratchMatrix) [$this vtkNew vtkMatrix4x4]

  # set the annotation property
  # but have the actual text be off by default
  [$_annotation GetTextProperty] SetColor 1 1 1
  [$_annotation GetTextProperty] SetShadow 1
  $_renderWidget CornerAnnotationVisibilityOff

  # TODO:
  # create text actors for L/R, I/S, P/A

  # setup the layers structure
  foreach layer {background foreground label} {
      set _layers($layer,logic) [[$sliceGUI GetLogic]  Get[string totitle $layer]Layer]
      set _layers($layer,node) [$_layers($layer,logic) GetVolumeNode]
      if { $_layers($layer,node) == "" } {
          set _layers($layer,image) ""
          set _layers($layer,xyToIJK) ""
          foreach v {i j k} { 
              set _layers($layer,$v) 0
          }
          set _layers($layer,pixel) "None"
      }
  }

  $this processEvent

  #
  # set up observers on sliceGUI and on sliceNode
  #

  $::slicer3::Broker AddObservation $sliceGUI DeleteEvent "::SWidget::ProtectedDelete $this"

  # events to catch (currently not catching KeyReleaseEvent, CharEvent, UserEvent)
  set events {  "MouseMoveEvent" "RightButtonPressEvent" "RightButtonReleaseEvent"
    "LeftButtonPressEvent" "LeftButtonReleaseEvent" "MiddleButtonPressEvent"
    "MiddleButtonReleaseEvent" "MouseWheelForwardEvent" "MouseWheelBackwardEvent"
    "ExposeEvent" "ConfigureEvent" "EnterEvent" "LeaveEvent"
    "TimerEvent" "KeyPressEvent" "ExitEvent" }
  foreach event $events {
    $::slicer3::Broker AddObservation $sliceGUI $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI $event"
  }

  # BUG: expose events are not passed through the vtk/kw layer so we need to explicitly handle them here
  [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetVTKWidget] AddBinding "<Expose>" "::SWidget::ProtectedCallback $this processEvent $sliceGUI ExposeEvent"

  set node [[$sliceGUI GetLogic] GetSliceNode]
  $::slicer3::Broker AddObservation $node DeleteEvent "::SWidget::ProtectedDelete $this"
  $::slicer3::Broker AddObservation $node AnyEvent "::SWidget::ProtectedCallback $this processEvent $node AnyEvent"

  # observe NodeAdded events from the MRML scene to create
  # model intersection displays as needed
  set NodeAddedEvent 66000
  set NodeRemovedEvent 66001
  set SceneClosedEvent 66003
  set SceneAboutToBeClosedEvent 66004
  set SceneImportedEvent 66011
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene $NodeAddedEvent "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene NodeAddedEvent"
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene $NodeRemovedEvent "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene NodeRemovedEvent"
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene $SceneClosedEvent "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene SceneClosedEvent"
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene $SceneAboutToBeClosedEvent "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene SceneAboutToBeClosedEvent"
  $::slicer3::Broker AddObservation $::slicer3::MRMLScene $SceneImportedEvent "::SWidget::ProtectedCallback $this processEvent $::slicer3::MRMLScene SceneImportedEvent"

  # put the other widgets last the events in this widget get natural
  # priority over the same event to a child widget

  lappend _swidgets [FiducialsSWidget #auto $sliceGUI]
  set gridSWidget [GridSWidget #auto $sliceGUI]
  $gridSWidget configure -layer "label"
  lappend _swidgets $gridSWidget
  lappend _swidgets [CrosshairSWidget #auto $sliceGUI]
  lappend _swidgets [VolumeDisplaySWidget #auto $sliceGUI]
  lappend _swidgets [SlicePlaneSWidget #auto $sliceGUI]

  #lappend _swidgets [RegionsSWidget #auto $sliceGUI] ;# not used
}


itcl::body SliceSWidget::destructor {} {

  foreach sw $_swidgets {
    if { [info command $sw] != "" } {
      itcl::delete object $sw
    }
  }

  $_renderWidget RemoveAllRenderers
}



# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

#
# create a slice widget for every node that does
# not already have one.  Also delete old ones.
#
itcl::body SliceSWidget::updateSWidgets {} {

  # this part is generic, based on the types configured for this class to manage
  foreach swidgetType $swidgetTypes {
    foreach {swidgetClass configVar nodeClass} $swidgetType {}
    array set sws ""
    # build look up table for current nodes
    set swidgets [itcl::find objects -class $swidgetClass]
    foreach sw $swidgets {
      set id [$sw cget $configVar]
      set gui [$sw cget -sliceGUI]
      set sws($gui,$id) $sw 
    }

    # loop through nodes and create any needed new widgets
    # - special case: skip the slice model that corresponds to our slice GUI
    # - also keep track of used ones so we know which ones to delete later
    set usedSWidgets ""
    set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass $nodeClass]
    for {set n 0} {$n < $nNodes} {incr n} {
      set node [$::slicer3::MRMLScene GetNthNodeByClass $n $nodeClass]
      set id [$node GetID]
      set layoutName [$_sliceNode GetName]
      set name [$node GetName]
      if { ![string match ${layoutName}* $name] } {
        if { ![info exists sws($sliceGUI,$id)] } {
          set swidget [$swidgetClass #auto $sliceGUI]
          $swidget configure $configVar $id
          lappend _swidgets $swidget
        } else {
          lappend usedSWidgets $sws($sliceGUI,$id)
        }
      }
    }

    # delete any of the widgets that no longer correspond to a node that is in the scene
    # - only the ones which correspond to 'our' sliceGUI instance
    foreach sw $swidgets {
      if { [lsearch $usedSWidgets $sw] == -1 } {
        if { [$sw cget -sliceGUI] == $sliceGUI } {
          set _swidgets [lremove -all $_swidgets $sw]
          itcl::delete object $sw
        }
      }
    }
  }
}

#
# make sure the size of the slice matches the window size of the widget
#
itcl::body SliceSWidget::resizeSliceNode {} {
  set epsilon 1.0e-6

  if { $_layers(background,node) != "" } {
    set logic [$sliceGUI GetLogic]
    set sliceSpacing [lindex [$logic GetLowestVolumeSliceSpacing] 2]
    if { [catch "expr $sliceSpacing"] } {
      set sliceSpacing 1.0
    }
    $this configure -sliceStep $sliceSpacing
  }

  foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}
  foreach {windowx windowy} [$_interactor GetEventPosition] {}
  # We should really use the pokedrenderer's size for these calculations.
  # However, viewerports in the LightBox can differ in size by a pixel.  So 
  # set the image size based on the size of renderer zero.
  #
  ###set pokedRenderer [$_interactor FindPokedRenderer $windowx $windowy]
  set pokedRenderer [$_renderWidget GetRenderer]  
  foreach {w h} [$pokedRenderer GetSize] {}


  foreach {nodeW nodeH nodeD} [$_sliceNode GetDimensions] {}
  foreach {nodefovx nodefovy nodefovz} [$_sliceNode GetFieldOfView] {}
  if { [catch "expr $nodefovx"] } {
    set nodefovx 1.0
  }
  if { [catch "expr $nodefovy"] } {
    set nodefovy 1.0
  }
  if { [catch "expr $nodefovz"] } {
    set nodefovz 1.0
  }

  if { $windoww < 1 || $windowh < 1 ||
       $w < 1 || $h < 1 || $nodeW < 1 || $nodeH < 1 ||
       $nodefovx == 0. || $nodefovx == 0.} {
    #puts "ignoring bogus resize"
  } else {
    set scaling0 [expr $w / (1. * $nodeW)]
    set scaling1 [expr $h / (1. * $nodeH)]

    set sMagnitude0 $scaling0
    if { $sMagnitude0 < 1.0 } {
       set sMagnitude0 [expr 1. / $sMagnitude0]
    }

    set sMagnitude1 $scaling1
    if { $sMagnitude1 < 1.0 } {
       set sMagnitude1 [expr 1. / $sMagnitude1]
    }

    if {$sMagnitude0 < $sMagnitude1} {
       # keep x fov the same, adjust y
       set fovx $nodefovx
       set fovy [expr $nodefovy * $scaling1 / $scaling0]
       set fovz [expr $sliceStep * $nodeD]
    } else {
       # keep y fov the same, adjust x
       set fovx [expr $nodefovx * $scaling0 / $scaling1]
       set fovy $nodefovy
       set fovz [expr $sliceStep * $nodeD]
    }

    set windowAspect [expr $h / (1. * $w)]
    set planeAspect [expr $fovy / (1. * $fovx)]
    if { [expr $windowAspect != $planeAspect] } {
      set fovx [expr $fovy / $windowAspect]
    }

    if { $fovx == $nodefovx && $fovy == $nodefovy && $fovz == $nodefovz &&
          $w == $nodeW && $h == $nodeH && [expr abs($sliceStep - ($nodefovz / (1. * $nodeD)))] < $epsilon} {
      return
    }
    set disabled [$_sliceNode GetDisableModifiedEvent]
    $_sliceNode DisableModifiedEventOn
    $_sliceNode SetDimensions $w $h $nodeD
    $_sliceNode SetFieldOfView $fovx $fovy $fovz
    $_sliceNode SetDisableModifiedEvent $disabled
    if { $disabled == 0 } {
        $_sliceNode InvokePendingModifiedEvent
        $_sliceNode DisableModifiedEventOff
    }
  }
}

#
# handle interactor events
#
itcl::body SliceSWidget::processEvent { {caller ""} {event ""} } {

  if { $enabled != "true" } {
    return
  }

  if { [info command $sliceGUI] == "" || [$sliceGUI GetLogic] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    ::SWidget::ProtectedDelete $this
    return
  }

  if { [info command $_sliceNode] == "" } {
    # somehow our slice node is corrupted - we need to bail out
    return
  }

  if { [$::slicer3::MRMLScene GetIsClosing] ||
        [$::slicer3::MRMLScene GetIsConnecting] ||
        [$::slicer3::MRMLScene GetIsImporting] ||
        [$::slicer3::MRMLScene GetIsRestoring] ||
        [$::slicer3::MRMLScene GetIsUpdating] } {
    return
  }

  # MRML Scene update probably means we need to create a new model intersection SWidget
  if { $caller == $::slicer3::MRMLScene && 
       ((($event == "NodeAddedEvent" || $event == "NodeRemovedEvent") && ![$::slicer3::MRMLScene GetIsUpdating]) || 
        $event == "SceneClosedEvent" || $event == "SceneAboutToBeClosedEvent" ||
        $event == "SceneImportedEvent") } {
    $this updateSWidgets
  }

  #
  # get the current event info
  # - the event type
  # - check to see if another widget is grabbing
  #   (we'll process some events even if we don't have grab)
  # - the actual x y location of the event
  # - fill the layers info with local info
  # - get the RAS space location of the event
  #
  if { $event != "ConfigureEvent" } {
    set tkwindow [$_renderWidget  GetWidgetName]
    if { $tkwindow != "" } {
      $_interactor UpdateSize [winfo width $tkwindow] [winfo height $tkwindow]
    }
  }


  # To support the LightBox, the event locations sometimes need to be
  # relative to a renderer (or viewport or pane of the lightbox).
  # Currently, these are relative to the viewport of the "start" action. 
  # We may need to change this in some cases to be relative to an "active" 
  # viewport.


  #
  # get the event position and make it relative to a renderer/viewport
  #
  foreach {windowx windowy} [$_interactor GetEventPosition] {}
  foreach {lastwindowx lastwindowy} [$_interactor GetLastEventPosition] {}
  foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}

  set pokedRenderer [$_interactor FindPokedRenderer $windowx $windowy]
  set renderer0 [$_renderWidget GetRenderer]
  
  foreach {x y z} [$this dcToXYZ $windowx $windowy] {}  

  if { $x < 0 } { 
    set x 0
  }
  if { $y < 0 } { 
    set y 0
  }
  # We should really use the pokedrenderer's size for these calculations.
  # However, viewports in the LightBox can differ in size by a pixel.  So 
  # set the image size based on the size of renderer zero.
  #
  ###foreach {w h} [$pokedRenderer GetSize] {}
  foreach {w h} [$renderer0 GetSize] {}
  foreach {rox roy} [$pokedRenderer GetOrigin] {}

  set xyToRAS [$_sliceNode GetXYToRAS]
  set ras [$xyToRAS MultiplyPoint $x $y $z 1]

  foreach {r a s t} $ras {}

  #
  # check that the window size is correct for the node and resize if needed
  #
  if { $caller != $_sliceNode } {
    $this resizeSliceNode
  }

  if { $calculateAnnotations && $_actionState == "" } {

    # TODO: the annotations seem to be very time consuming to calculate
    # but most of the time they will not be displayed - instead we should
    # move the calculation into the DelayedAnnotation method

    #
    # cancel any scheduled annotations
    #
    if { [itcl::find class ::SliceSWidget] == "::SliceSWidget"} {
        set swidgets [itcl::find objects -class ::SliceSWidget]
        foreach sw $swidgets {
            $sw cancelDelayedAnnotation
        }
    }


    #
    # update the annotations even if they aren't currently visible
    # - this way the values will be correct when the corner annotation
    #   are eventually made visible
    #
    set annotationsUpdated false
    set link [$_sliceCompositeNode GetLinkedControl]
    if { $link == 1 && [$this isCompareViewMode] == 1 && 
          ([$this isCompareViewer] == 1 || [$_sliceNode GetSingletonTag] == "Red") } {
        $this updateAnnotations $r $a $s
        set annotationsUpdated true
    } else {
        $this updateAnnotation $r $a $s
        set annotationsUpdated true
    }
  } else {
    $_renderWidget CornerAnnotationVisibilityOff
  }

  if { $forceAnnotationsOff } {
    $_renderWidget CornerAnnotationVisibilityOff
  }

  #
  # if another widget has the grab, let this go unless
  # it is a focus event, in which case we want to update
  # our display icon
  #
  set grabID [$sliceGUI GetGrabID]
  if { ($grabID != "" && $grabID != $this) } {
    if { ![string match "Focus*Event" $event] } {
      if { [info command $grabID] != $grabID } {
        # the widget with the grab doesn't exist any more (probably deleted while grabbing)
        # reset the grabID and continue
        $sliceGUI SetGrabID ""
      } else {
        return ;# some other widget wants these events
      }
    }
  }

  if { $_actionState == "Translate" && $_actionModifier == "Shift" } {
    # need to check if the user has stopped doing a shift-drag
    if { (![$_interactor GetShiftKey] && ![$_interactor GetControlKey]) || $event == "LeftButtonReleaseEvent" } {
      $this endTranslate
      set _actionModifier ""
    }
  }

  switch $event {

    "MouseMoveEvent" {
      #
      # Mouse move behavior governed by _actionState mode
      # - handle modifying the view
      #
      if { [$_interactor GetShiftKey] && $_actionState != "Translate" } {
        $this jumpOtherSlices $r $a $s
        # need to render to show the annotation
        [$sliceGUI GetSliceViewer] RequestRender
      } else {
        switch $_actionState {
          "Translate" {
            #
            # Translate
            # TODO: move calculation to vtkMRMLSliceLogic
            $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
            set tx [expr $windowx - [lindex $_actionStartViewportOrigin 0]]
            set ty [expr $windowy - [lindex $_actionStartViewportOrigin 1]]
            
            set currentRAS [$o(storeXYToRAS) MultiplyPoint $tx $ty 0 1]
            foreach d {dr da ds} start $_actionStartRAS current $currentRAS {
              set $d [expr $current - $start]
            }
            $o(scratchMatrix) DeepCopy $o(storeSliceToRAS)
            foreach d {dr da ds} i {0 1 2} {
              set v [$o(scratchMatrix) GetElement $i 3]
              $o(scratchMatrix) SetElement $i 3 [expr $v - [set $d]]
            }

            [$_sliceNode GetSliceToRAS] DeepCopy $o(scratchMatrix)
            $_sliceNode UpdateMatrices

            $sliceGUI SetGUICommandAbortFlag 1
            $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
          }
          "Zoom" {
            #
            # Zoom
            # TODO: move calculation to vtkMRMLSliceLogic
            $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
            set deltay [expr $windowy - [lindex $_actionStartWindowXY 1]]

            set percent [expr ($windowh + $deltay) / (1.0 * $windowh)]

            # the factor operation is so 'z' isn't changed and the 
            # slider can still move through the full range
            if { $percent > 0. } {
              set newFOV ""
              foreach f $_actionStartFOV factor "$percent $percent 1" {
                lappend newFOV [expr $f * $factor]
              }

              foreach {nfx nfy nfz} $newFOV {}
              $_sliceNode SetFieldOfView $nfx $nfy $nfz
            }
            $sliceGUI SetGUICommandAbortFlag 1
          }
          "Rotate" {
            #
            # Rotate
            # TODO: move calculation to vtkMRMLSliceLogic
            $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
            set dx [expr $windowx - $lastwindowx]
            set dy [expr $windowy - $lastwindowy]

            set dazimuth   [expr 20.0 / $w]
            set delevation [expr 20.0 / $h]

            set rx [expr $dx * $dazimuth * 10.0]
            set ry [expr $dy * $delevation * 10.0]

            set tfm [$this vtkNew vtkTransform]
            $tfm PreMultiply
            $tfm Identity
            $tfm SetMatrix [$_sliceNode GetSliceToRAS] 
            $tfm RotateX $ry
            $tfm RotateY $rx
            
            [$_sliceNode GetSliceToRAS] DeepCopy [$tfm GetMatrix]
            $_sliceNode UpdateMatrices

            $tfm Delete

            $sliceGUI SetGUICommandAbortFlag 1
           }
          default {
            # need to render to show the annotation
            [$sliceGUI GetSliceViewer] RequestRender
          }
        }
      }
    }

    "RightButtonPressEvent" {

      $::slicer3::MRMLScene SaveStateForUndo $_sliceNode

      $this requestDelayedAnnotation
      if { [$_interactor GetControlKey] } {
        set _actionState "Rotate"
      } else {
        set _actionState "Zoom"
      }
      set _actionStartXY "$x $y"
      set _actionStartWindowXY "$windowx $windowy"
      set _actionStartViewportOrigin "$rox $roy"
      set _actionStartRAS $ras
      set _actionStartOrientation [$_sliceNode GetOrientationString]
      set _actionLink [$_sliceCompositeNode GetLinkedControl]
      $_sliceCompositeNode SetLinkedControl 0

      $sliceGUI SetGrabID $this
      $sliceGUI SetGUICommandAbortFlag 1
      set _actionStartFOV [$_sliceNode GetFieldOfView]
    }
    "RightButtonReleaseEvent" { 
      $_sliceCompositeNode SetLinkedControl $_actionLink
      if { $_actionState == "Zoom" } {
         # set the field of view on each slice node. note that
         # the Red viewer and Compare viewers may have different
         # aspect ratios, so set the field of views in x the same
         foreach {nfx nfy nfz} [$_sliceNode GetFieldOfView] {}
         set sliceLogics [$this getLinkedSliceLogics]
         foreach logic $sliceLogics {
            set snode [$logic GetSliceNode]
            foreach {fx fy fz} [$snode GetFieldOfView] {}
            # new prescribed x fov, aspect corrected y fov, orig z fov
            $snode SetFieldOfView $nfx [expr $nfx*$fy/$fx] $fz
         }
     } elseif { $_actionState == "Rotate" } {
         set sliceLogics [$this getLinkedSliceLogics]
         # set the SliceToRAS on each slice node
         foreach logic $sliceLogics {
           set snode [$logic GetSliceNode]
           [$snode GetSliceToRAS] DeepCopy [$_sliceNode GetSliceToRAS]
           $snode UpdateMatrices
         }
     }

      $this requestDelayedAnnotation
      set _actionState ""
      set _actionStartOrientation ""
      $sliceGUI SetGrabID ""
      $sliceGUI SetGUICommandAbortFlag 1
    }
    "LeftButtonPressEvent" {
      if { [info command SeedSWidget] != "" } {
        set interactionNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLInteractionNode]
        set modeString ""
        if { $interactionNode != "" } {
          set mode [$interactionNode GetCurrentInteractionMode]
          set modeString [$interactionNode GetInteractionModeAsString $mode]
        }
        if { $modeString == "Place" } {
          # AND PLACE FIDUCIAL.
          # don't place a fiducial in slicer4 - let this mode by handled by the Annotations module
          # FiducialsSWidget::AddFiducial $r $a $s
        } else {
          if { [$_interactor GetShiftKey] || [$_interactor GetControlKey] } {
            # shift-left-button is alias for middle mouse button to support 
            # machines with no middle mouse button (e.g. macs)
            # - shift+mouse move will also do jump all slices if no left button
            #   is pressed.  This is unfortunate, but we use shift to be compatible
            #   with the 3D viewer convention for pan
            # - control key is also a translate modifier to make it easier
            #   to avoid the jump slice issue.
            $this startTranslate $x $y  $windowx $windowy  $rox $roy  $ras
            set _actionModifier "Shift"
          }
        }
      }
    }
    "LeftButtonReleaseEvent" { 
        $sliceGUI SetGUICommandAbortFlag 1
        if { [$sliceGUI GetGrabID] == $this } {
            $sliceGUI SetGrabID ""
        }
        # RESET MOUSE MODE BACK TO
        # TRANSFORM, UNLESS USER HAS
        # SELECTED A PERSISTENT PICK OR
        # PLACE MODE.
        set interactionNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLInteractionNode]
        if { $interactionNode != "" } {
            set pickPersistence [ $interactionNode GetPickModePersistence ]
            set placePersistence [ $interactionNode GetPlaceModePersistence ]
            if { $pickPersistence == 0 && $placePersistence == 0 } {
                set mode [ $interactionNode GetInteractionModeByString "ViewTransform" ]
                $interactionNode SetCurrentInteractionMode $mode
            }
        }
    }
    "MiddleButtonPressEvent" {
        $this startTranslate $x $y  $windowx $windowy  $rox $roy  $ras
    }
    "MiddleButtonReleaseEvent" { 
        $this endTranslate
    }
    "MouseWheelForwardEvent" { 
      $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
      $this incrementSlice 
    }
    "MouseWheelBackwardEvent" {
      $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
      $this decrementSlice 
    }
    "ExposeEvent" { 
      [$sliceGUI GetSliceViewer] RequestRender
    }
    "ConfigureEvent" {
      $this resizeSliceNode
    }
    "EnterEvent" { 
      #puts "Enter"
      set _inWidget 1
      $_renderWidget CornerAnnotationVisibilityOn
      [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText "Middle Button: Pan; Right Button: Zoom"

      set sliceGUIs [$this getLinkedSliceGUIs]
      # find the slice spacing to use
      set thisSliceSpacing [[$sliceGUI GetLogic] GetLowestVolumeSliceSpacing]
      foreach gui $sliceGUIs {
          set snode [$gui GetSliceNode]
          if { $_sliceNode != $snode } {
              set sliceSpacing [[$gui GetLogic] GetLowestVolumeSliceSpacing]
              if { [lindex $sliceSpacing 2] < [lindex $thisSliceSpacing 2] } {
                  set thisSliceSpacing $sliceSpacing
              }
          }
      }
      # disable modifieds
      foreach gui $sliceGUIs {
          set snode [$gui GetSliceNode]
          $snode DisableModifiedEventOn
      }
      # modify each slice node
      #puts "Current node $_sliceNode Current gui $sliceGUI Current SliceWidget $this"
      if { $sliceGUIs != $sliceGUI } {
        # only adjust the spacing mode if there is something 
        # linked to us
        $_sliceNode SetSliceSpacingModeToAutomatic
        foreach gui $sliceGUIs {
            set snode [$gui GetSliceNode]
            
            # prescribe spacing for all guis
            eval $snode SetPrescribedSliceSpacing $thisSliceSpacing
            $snode SetSliceSpacingModeToPrescribed

            # then tell them to reslice
            set that [$this getSliceSWidgetForGUI $gui]
            $that resizeSliceNode
        }
      }
      # enable modifieds, don't invoke the pending events, 
      # request a render on each
      foreach gui $sliceGUIs {
          set snode [$gui GetSliceNode]
          #$snode InvokePendingModifiedEvent
          $snode DisableModifiedEventOff
          [$gui GetSliceViewer] RequestRender
      }
    }
    "LeaveEvent" { 
      #puts "Leave"
      set _inWidget 0

      set sliceGUIs [$this getLinkedSliceGUIs]
      # cancel annotation requests before doing anything else
      foreach gui $sliceGUIs {
        set sw [$this getSliceSWidgetForGUI $gui]
        $sw cancelDelayedAnnotation
      }
      # now turn do whatever else is needed
      foreach gui $sliceGUIs {
        [[$gui GetSliceViewer] GetRenderWidget] CornerAnnotationVisibilityOff
        set snode [$gui GetSliceNode]
        if { [$this isCompareViewMode] == 1 } {
           # for now, don't switch back to automatic (maybe when we switch 
           # out of compare view mode turn automatic back on)
           # $snode SetSliceSpacingModeToAutomatic
        }
        [$gui GetSliceViewer] RequestRender
      }
      [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText ""
    }
    "TimerEvent" { }
    "KeyPressEvent" { 
      set key [$_interactor GetKeySym]
      if { [lsearch "v V r b f g G T space c e s S Up Down Left Right" $key] != -1 } {
        $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
        $sliceGUI SetGUICommandAbortFlag 1
        switch [$_interactor GetKeySym] {
          "v" {
            $_sliceNode SetSliceVisible [expr ![$_sliceNode GetSliceVisible]]
          }
          "V" {
           # toggle all slices visibility
           puts "Toggling all slices visibility not implemented yet"
          }
          "r" {
            # figure out the new field of view for the current slice
            # node, then set that field of view on the linked slice
            # nodes
            set logic [$sliceGUI GetLogic]
            $logic FitSliceToBackground $w $h
            $_sliceNode UpdateMatrices
            
            foreach {nfx nfy nfz} [$_sliceNode GetFieldOfView] {}

            # get the linked logics (including self)
            set sliceLogics [$this getLinkedSliceLogics]
            # save state for undo
            
            # can't call FitToBackground on linked nodes since they
            # may have different aspect rations
            foreach logic $sliceLogics {
              set snode [$logic GetSliceNode]
              foreach {fx fy fz} [$snode GetFieldOfView] {}
              # new prescribed x fov, aspect corrected y fov, orig z fov
              $snode SetFieldOfView $nfx [expr $nfx*$fy/$fx] $fz
            }
          }
          "g" {
            # toggle the label opacity via the slice compoiste node
              set opacity [$_sliceCompositeNode GetLabelOpacity]
              if {$opacity != 0.0} {
                  set _lastLabelOpacity $opacity
              }
              if { $opacity == 0.0 } {
                  $_sliceCompositeNode SetLabelOpacity $_lastLabelOpacity
              } else {
                  $_sliceCompositeNode SetLabelOpacity 0.0
              }
          }
          "G" {
              puts "Toggling label opacity for all slices not implemented yet"
          }
          "T" {
            # toggle opacity of foreground overlay       
            set opacity [$_sliceCompositeNode GetForegroundOpacity]
            if {$opacity != 0.0} {
                set _lastForegroundOpacity $opacity
            }
            if { $opacity == 0.0 } {
                $_sliceCompositeNode SetForegroundOpacity $_lastForegroundOpacity
            } else {
                $_sliceCompositeNode SetForegroundOpacity 0.0
            }
          }
          "b" - "Left" - "Down" {
            $this decrementSlice
          }
          "f" - "Right" - "Up" {
            $this incrementSlice
          }
          "space" {
            ::Box::ShowDialog EditBox
          }
          "c" {
            ::Box::ShowDialog ColorBox
          }
          "e" {
            EditorToggleErasePaintLabel
          }
          "s" {
            # set the active slice. if in compare view put this slice in 
            # the background of the red viewer
            set k [expr int($z + 0.5)]
            if { $k >= 0 && $k < [lindex [$_sliceNode GetDimensions] 2] } {
              $_sliceNode SetActiveSlice $k
              if { [$this isCompareViewer] == 1 } {
                # set the volume on the red viewer
                set redGUI [$::slicer3::ApplicationGUI GetMainSliceGUI Red]
                set redLogic [$redGUI GetLogic]
                set redNode [$redGUI GetSliceNode]
                set redCNode [$redLogic GetSliceCompositeNode]

                $redCNode SetReferenceBackgroundVolumeID [$_sliceCompositeNode GetBackgroundVolumeID]
                
                # set the slice offset on the red viewer (translate to 
                # active slice)
                # offset to first slice in viewer
                set offset [[$sliceGUI GetLogic] GetSliceOffset]
                # offset to the active slice
                set offset2 [expr $offset + [$_sliceNode GetActiveSlice] * [lindex [[$sliceGUI GetLogic] GetLowestVolumeSliceSpacing] 2] ]
                # offset needed for the active slice in red viewer
                set offset3 [expr $offset2 - [$redNode GetActiveSlice] * [lindex [[$redGUI GetLogic] GetLowestVolumeSliceSpacing] 2] ]
                $redLogic SetSliceOffset $offset3

                # force the red viewer to recalculate its matrices
                set that [$this getSliceSWidgetForGUI $redGUI]
                $that resizeSliceNode
              }
            }
          }
          "S" {
            # set the active slice. if in compare view put this slice in 
            # the foreground of the red viewer
            set k [expr int($z + 0.5)]
            if { $k >= 0 && $k < [lindex [$_sliceNode GetDimensions] 2] } {
              $_sliceNode SetActiveSlice $k
              if { [$this isCompareViewer] == 1 } {
                # set the volume on the red viewer
                set redGUI [$::slicer3::ApplicationGUI GetMainSliceGUI Red]
                set redLogic [$redGUI GetLogic]
                set redNode [$redGUI GetSliceNode]
                set redCNode [$redLogic GetSliceCompositeNode]

                $redCNode SetReferenceForegroundVolumeID [$_sliceCompositeNode GetBackgroundVolumeID]
                
                # set the slice offset on the red viewer (translate to 
                # active slice)
                # offset to first slice in viewer
                set offset [[$sliceGUI GetLogic] GetSliceOffset]
                # offset to the active slice
                set offset2 [expr $offset + [$_sliceNode GetActiveSlice] * [lindex [[$sliceGUI GetLogic] GetLowestVolumeSliceSpacing] 2] ]
                # offset needed for the active slice in red viewer
                set offset3 [expr $offset2 - [$redNode GetActiveSlice] * [lindex [[$redGUI GetLogic] GetLowestVolumeSliceSpacing] 2] ]
                $redLogic SetSliceOffset $offset3

                # force the red viewer to recalculate its matrices
                set that [$this getSliceSWidgetForGUI $redGUI]
                $that resizeSliceNode
              }
            }
          }
          default {
            set capture 0
          }
        }
      } else {
        # puts "slice ignoring $key"
      }
    }
    "KeyReleaseEvent" { 
    }
    "FocusInEvent" {
      #puts "focusin. prescribing spacing"
      #need to do the same as EnterEvent?
    }
    "FocusOutEvent" {
      #puts "focusout. automatic spacing"
      #need to do the same as EnterEvent?
    }
    "ExitEvent" { }
  }
}


itcl::body SliceSWidget::updateAnnotations {r a s} {

  $this updateStatusAnnotation $r $a $s
    
  foreach {x y z} [$this rasToXYZ "$r $a $s"] {}
  $this queryLayers $x $y $z

  set ssgui [[$::slicer3::ApplicationGUI GetApplication] GetModuleGUIByName "Slices"]
  if { $ssgui == "" } {
    return
  }
  set numsgui [$ssgui GetNumberOfSliceGUI]
  for { set i 0 } { $i < $numsgui } { incr i } {
    if { $i == 0} {
      set sgui [$ssgui GetFirstSliceGUI]
      set lname [$ssgui GetFirstSliceGUILayoutName]
    } else {
      set sgui [$ssgui GetNextSliceGUI $lname]
      set lname [$ssgui GetNextSliceGUILayoutName $lname]
    }
    set sNode [$sgui GetSliceNode]
    set logic [$sgui GetLogic]
    set sliceCompositeNode [$logic GetSliceCompositeNode]

    if { $sliceCompositeNode == "" } {
      # need a composite node to be able to do anything
      return
    }

    if { $lname != "Red" && [string first "Compare" $lname] != 0 } {
      continue
    } 
    
    set laybelText ""
    set voxelText ""
    set ijkText ""
    set xyText ""
    set rasText ""
    set foregroundname "None"
    set backgroundname "None"
    set labelname "None"
    set reformation ""

    # find the SliceSWidget for sgui
    set sw [$this getSliceSWidgetForGUI $sgui]
    if { $sw != "" } {
      foreach {x y z} [$sw rasToXYZ "$r $a $s"] {}
      $sw queryLayers $x $y $z

      array set slayers [$sw getLayers]

      set colorName ""
      if {[info command $slayers(label,node)] != "" && \
            $slayers(label,node) != "" && \
            $slayers(label,pixel) != "" && \
            $slayers(label,pixel) != "Unknown" && \
            $slayers(label,pixel) != "Out of Frame" && \
            $slayers(label,pixel) != "Slice not shown"} {
        set labelDisplayNode [$slayers(label,node) GetDisplayNode]
        if {$labelDisplayNode != "" && [$labelDisplayNode GetColorNodeID] != ""} {
          set colorNode [$labelDisplayNode GetColorNode]
          if {$colorNode != ""} {
            if {[string is integer $slayers(label,pixel)]} {
              set colorName [$colorNode GetColorName $slayers(label,pixel)]
            }
          }
        }
      }
      set labelText "Lb: $slayers(label,pixel) $colorName"
      set voxelText "Fg: $slayers(foreground,pixel)\nBg: $slayers(background,pixel)"
      if { [string is integer $slayers(background,i)] } {
          set ijkText [format "Bg I: %d\nBg J: %d\nBg K: %d" \
                     $slayers(background,i) $slayers(background,j) $slayers(background,k)]
      } else {
          set ijkText ""
      }
      set xyText "X: $x\nY:$y"
      set rasText [format "R: %.1f\nA: %.1f\nS: %.1f" $r $a $s]

      if {[info command $slayers(label,node)] != ""
          && $slayers(label,node) != ""} {
          set labelname [$slayers(label,node) GetName]
      }

      if {[info command $slayers(foreground,node)] != ""
          && $slayers(foreground,node) != ""} {
          set foregroundname [$slayers(foreground,node) GetName]
      }

      if {[info command $slayers(background,node)] != ""
          && $slayers(background,node) != ""} {
          set backgroundname [$slayers(background,node) GetName]
      } 

    }

    set formattedSpacing [format "%.3g" [lindex [$logic GetLowestVolumeSliceSpacing] 2]]
    set spacingText "Sp: ${formattedSpacing}mm"
    if { [$sNode GetSliceSpacingMode] == 1 } {
      set spacingText "(Sp: ${formattedSpacing}mm)"
    }


    set spaceText0 ""
    set spaceText1 ""
    switch [$sliceCompositeNode GetAnnotationSpace] {
      "0" {set spaceText0 $xyText}
      "1" {set spaceText0 $ijkText}
      "2" {set spaceText0 $rasText}
      "3" {set spaceText0 $rasText; set spaceText1 $ijkText}
    }
    
   set reformation [$_sliceNode GetOrientationString]
      

    set annotation [[[$sgui GetSliceViewer] GetRenderWidget] GetCornerAnnotation] 
    switch [$sliceCompositeNode GetAnnotationMode] {
      "0" {
        $annotation SetText 0 ""
        $annotation SetText 1 ""
        $annotation SetText 2 ""
        $annotation SetText 3 ""
      }
      "1" {
        $annotation SetText 0 "${labelText}\n${voxelText}"
        $annotation SetText 1 $spaceText0
        $annotation SetText 2 $spaceText1
        $annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
      }
      "2" {
        $annotation SetText 0 "${labelText}"
        $annotation SetText 1 ""
        $annotation SetText 2 ""
        $annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
      }
      "3" {
        $annotation SetText 0 "${labelText}\n${voxelText}"
        $annotation SetText 1 ""
        $annotation SetText 2 ""
        $annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
      }
    }
    
    # jvm - request a render so the annotations on other viewers update
    $sw requestDelayedAnnotation
  }
}

itcl::body SliceSWidget::updateAnnotation {r a s} {

  $this updateStatusAnnotation $r $a $s

  foreach {x y z} [$this rasToXYZ "$r $a $s"] {}
  $this queryLayers $x $y $z

  set logic [$sliceGUI GetLogic]
  set sliceCompositeNode [$logic GetSliceCompositeNode]

  if { $sliceCompositeNode == "" } {
    # need a composite node to be able to do anything
    return
  }


  set foregroundname "None"
  set backgroundname "None"
  set labelname "None"

  set reformation [$_sliceNode GetOrientationString]
  
  # get the display node for the label volume, extract the name of the colour used to represent the label pixel
  set colorName ""
  if {[info command $_layers(label,node)] != "" && \
      $_layers(label,node) != "" && \
      $_layers(label,pixel) != "" && \
      $_layers(label,pixel) != "Unknown" && \
      $_layers(label,pixel) != "Out of Frame" && \
      $_layers(label,pixel) != "Slice not shown"} {
      set labelDisplayNode [$_layers(label,node) GetDisplayNode]
      if {$labelDisplayNode != "" && [$labelDisplayNode GetColorNodeID] != ""} {
          set colorNode [$labelDisplayNode GetColorNode]
          if {$colorNode != ""} {
              if {[string is integer $_layers(label,pixel)]} {
                  set colorName [$colorNode GetColorName $_layers(label,pixel)]
              }
          }
      }
  }

  if {[info command $_layers(label,node)] != ""
      && $_layers(label,node) != ""} {
      set labelname "Lb: [$_layers(label,node) GetName]"
  }

  if {[info command $_layers(foreground,node)] != ""
      && $_layers(foreground,node) != ""} {
      set foregroundname "Fg: [$_layers(foreground,node) GetName]"
  }

  if {[info command $_layers(background,node)] != ""
      && $_layers(background,node) != ""} {
      set backgroundname "Bg: [$_layers(background,node) GetName]"
  } 

  set labelText "Lb: $_layers(label,pixel) $colorName"

  set pixelValue $_layers(foreground,pixel)
  if { $pixelValue != "" && [string is double $pixelValue] } {
      set fgvoxelText [format "Fg: %.1f" $pixelValue]
  } else {
      if { $pixelValue == "None"} {
          set fgvoxelText ""
      } else {
          set fgvoxelText "Fg: $pixelValue,"
      }
  }
  set pixelValue $_layers(background,pixel)
  if { $pixelValue != "" && [string is double $pixelValue] } {
      set bgvoxelText [format "Bg: %.1f" $pixelValue]
  } else {
      if { $pixelValue == "None"} {
          set bgvoxelText ""
      } else {
          set bgvoxelText "Bg: $pixelValue,"
      }
  }

  set voxelText "$fgvoxelText\n$bgvoxelText"


  if { [string is integer $_layers(background,i)] } {
      set ijkText [format "Bg I: %d\nBg J: %d\nBg K: %d" \
                       $_layers(background,i) $_layers(background,j) $_layers(background,k)]
  } else {
      set ijkText ""
  }

  set xyText "X: $x\nY:$y"
  set rasText [format "R: %.1f\nA: %.1f\nS: %.1f" $r $a $s]

  set formattedSpacing [format "%.3g" [lindex [[$sliceGUI GetLogic] GetLowestVolumeSliceSpacing] 2]]
  set spacingText "Sp: ${formattedSpacing}mm"
  if { [$_sliceNode GetSliceSpacingMode] == 1 } {
    set spacingText "(Sp: ${formattedSpacing}mm)"
  }

  set spaceText0 ""
  set spaceText1 ""
  switch [$sliceCompositeNode GetAnnotationSpace] {
    "0" {set spaceText0 $xyText}
    "1" {set spaceText0 $ijkText}
    "2" {set spaceText0 $rasText}
    "3" {set spaceText0 $rasText; set spaceText1 $ijkText}
  }

  switch [$sliceCompositeNode GetAnnotationMode] {
    "0" {
      $_annotation SetText 0 ""
      $_annotation SetText 1 ""
      $_annotation SetText 2 ""
      $_annotation SetText 3 ""
    }
    "1" {
      $_annotation SetText 0 "${labelText}\n${voxelText}"
      $_annotation SetText 1 $spaceText0
      $_annotation SetText 2 $spaceText1
      $_annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
    }
    "2" {
      $_annotation SetText 0 "${labelText}"
      $_annotation SetText 1 ""
      $_annotation SetText 2 ""
      $_annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
    }
    "3" {
      $_annotation SetText 0 "${labelText}\n${voxelText}"
      $_annotation SetText 1 ""
      $_annotation SetText 2 ""
      $_annotation SetText 3 "${backgroundname}\n${foregroundname}\n${labelname}\n\n${reformation}\n${spacingText}"
    }
  }

  # jvm - request a render so the annotations on other viewers update
  $this requestDelayedAnnotation
}

itcl::body SliceSWidget::updateStatusAnnotation {r a s} {

  # display a subset of the annotation in the status bar because we may not
  # show the information while moving the mouse

  foreach {x y z} [$this rasToXYZ "$r $a $s"] {}
  $this queryLayers $x $y $z

  set logic [$sliceGUI GetLogic]
  set sliceCompositeNode [$logic GetSliceCompositeNode]

  if { $sliceCompositeNode == "" } {
    # need a composite node to be able to do anything
    return
  }

  set foregroundname "None"
  set backgroundname "None"
  set labelname "None"

  set reformation [$_sliceNode GetOrientationString]
  
  # get the display node for the label volume, extract the name of the colour used to represent the label pixel
  set colorName ""
  if {[info command $_layers(label,node)] != "" && \
      $_layers(label,node) != "" && \
      $_layers(label,pixel) != "" && \
      $_layers(label,pixel) != "Unknown" && \
      $_layers(label,pixel) != "Out of Frame" && \
      $_layers(label,pixel) != "Slice not shown"} {
      set labelDisplayNode [$_layers(label,node) GetDisplayNode]
      if {$labelDisplayNode != "" && [$labelDisplayNode GetColorNodeID] != ""} {
          set colorNode [$labelDisplayNode GetColorNode]
          if {$colorNode != ""} {
              if {[string is integer $_layers(label,pixel)]} {
                  set colorName [$colorNode GetColorName $_layers(label,pixel)]
              }
          }
      }
  }

  if {[info command $_layers(label,node)] != ""
      && $_layers(label,node) != ""} {
      set labelname "[$_layers(label,node) GetName]"
  }

  if {[info command $_layers(foreground,node)] != ""
      && $_layers(foreground,node) != ""} {
      set foregroundname "[$_layers(foreground,node) GetName]"
  }

  if {[info command $_layers(background,node)] != ""
      && $_layers(background,node) != ""} {
      set backgroundname "[$_layers(background,node) GetName]"
  } 

  if { $_layers(label,pixel) == "None"} {
      set labelText ""
  } else {
      set labelText " Lb: $_layers(label,pixel) $colorName,"
  }

  set pixelValue $_layers(foreground,pixel)
  if { $pixelValue != "" && [string is double $pixelValue] } {
      set fgvoxelText [format " Fg: %.1f" $pixelValue]
  } else {
      if { $pixelValue == "None"} {
          set fgvoxelText ""
      } else {
          set fgvoxelText " Fg: $pixelValue,"
      }
  }
  set pixelValue $_layers(background,pixel)
  if { $pixelValue != "" && [string is double $pixelValue] } {
      set bgvoxelText [format " Bg: %.1f" $pixelValue]
  } else {
      if { $pixelValue == "None"} {
          set bgvoxelText ""
      } else {
          set bgvoxelText " Bg: $pixelValue,"
      }
  }
  
  if { $fgvoxelText == "" && $bgvoxelText == "" } {
      set voxelText ""
  } elseif {$fgvoxelText == "" } {
      set voxelText $bgvoxelText
  } elseif {$bgvoxelText == "" } {
      set voxelText $fgvoxelText
  } else {
      set voxelText "$fgvoxelText$bgvoxelText"
  }


  if { [string is integer $_layers(background,i)] } {
      set ijkText [format " Bg IJK: (%d, %d, %d)," \
                       $_layers(background,i) $_layers(background,j) $_layers(background,k)]
  } else {
      set ijkText ""
  }

  if { $backgroundname == "None" } {
      set ijkText ""
  }

  set xyText "XY: ($x, $y)"
  set rasText [format "RAS: (%.1f, %.1f, %.1f)," $r $a $s]

  set statusText "$backgroundname $rasText$ijkText$labelText$voxelText"
  [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText $statusText
}


itcl::body SliceSWidget::incrementSlice {} {
  $this moveSlice $sliceStep
}

itcl::body SliceSWidget::decrementSlice {} {
  $this moveSlice [expr -1.0 * $sliceStep]
}

itcl::body SliceSWidget::moveSlice { delta } {
    set logic [$sliceGUI GetLogic]
    set sliceNode [$logic GetSliceNode]
    set orientString [$sliceNode GetOrientationString]

    set offset [$logic GetSliceOffset]
    set spacing [$logic GetLowestVolumeSliceSpacing]

    set logics [$this getLinkedSliceLogics]

    # set the slice offset for all slice logics (there may be a flaw
    # in this logic as modifying a single logic may trigger a
    # controller widget which then will trigger all the other slice
    # logics. Comes down to whether we want to rely on the existence
    # of SliceControllerWidgets.)
    set numberOfLogics [llength $logics]
    foreach logic $logics {
        set tlink [[$logic GetSliceCompositeNode] GetLinkedControl]
        [$logic GetSliceCompositeNode] SetLinkedControl 0
        $logic SetSliceOffset [expr $offset + $delta]
        [$logic GetSliceCompositeNode] SetLinkedControl $tlink
    }
    $_renderWidget CornerAnnotationVisibilityOff
}

itcl::body SliceSWidget::jumpSlice { r a s } {
  set logic [$sliceGUI GetLogic]
  set sliceNode [$logic GetSliceNode]
  $sliceNode JumpSlice $r $a $s
}

itcl::body SliceSWidget::jumpOtherSlices { r a s } {
  set logic [$sliceGUI GetLogic]
  set sliceNode [$logic GetSliceNode]
  $sliceNode JumpAllSlices $r $a $s
}

itcl::body SliceSWidget::addSliceModelSWidgets {} {
  set class "vtkMRMLModelNode"
  set number [$slicer3::MRMLScene GetNumberOfNodesByClass $class]
  for {set n 0} {$n < $number} {incr n} {
    set modelNode [$::slicer3::MRMLScene GetNthNodeByClass $n $class]
    set modelSWidget [ModelSWidget #auto $sliceGUI]
    $modelSWidget configure -modelID [$modelNode GetID]
    lappend _swidgets $modelSWidget
  }
}


# Return the SliceLogics that are linked to the current 
# SliceNode/SliceCompositeNode.  
#
# The list of linked logics either contains the current slice (because 
# linking is off) or a list of logics (one for the current slice and others 
# for linked slices).
#
itcl::body SliceSWidget::getLinkedSliceLogics { } {
    set logic [$sliceGUI GetLogic]
    set sliceNode [$logic GetSliceNode]
    set orientString [$sliceNode GetOrientationString]

    set logics ""
    set link [$_sliceCompositeNode GetLinkedControl]
    if { $link == 1 && [$this isCompareViewMode] == 1 && ([$sliceNode GetSingletonTag] == "Red" || [$this isCompareViewer] == 1) } {
        set ssgui [[$::slicer3::ApplicationGUI GetApplication] GetModuleGUIByName "Slices"]
        set layout [$::slicer3::ApplicationGUI GetGUILayoutNode]
        set viewArrangement [$layout GetViewArrangement]

        set numsgui [$ssgui GetNumberOfSliceGUI]

        for { set i 0 } { $i < $numsgui } { incr i } {
            if { $i == 0} {
                set sgui [$ssgui GetFirstSliceGUI]
                set lname [$ssgui GetFirstSliceGUILayoutName]
            } else {
                set sgui [$ssgui GetNextSliceGUI $lname]
                set lname [$ssgui GetNextSliceGUILayoutName $lname]
            }

            if { $lname != "Red" && [string first "Compare" $lname] != 0 } {
              continue
            } 

            set currSliceNode [$sgui GetSliceNode]
            set currOrientString [$currSliceNode GetOrientationString]
            if { [string compare $orientString $currOrientString] == 0 || ($_actionStartOrientation != "" && [string compare $_actionStartOrientation $currOrientString] == 0) } {
                lappend logics [$sgui GetLogic]
            }
        }
    } else {
        lappend logics [$sliceGUI GetLogic]
    }

  return $logics
}

# Return the SliceGUIs that are linked to the current 
# SliceNode/SliceCompositeNode.  
#
# The list of linked GUIs either contains the current slice (because 
# linking is off) or a list of GUIs (one for the current slice and others 
# for linked slices).
#
itcl::body SliceSWidget::getLinkedSliceGUIs { } {
    set logic [$sliceGUI GetLogic]
    set sliceNode [$logic GetSliceNode]
    set orientString [$sliceNode GetOrientationString]

    set guis ""
    set link [$_sliceCompositeNode GetLinkedControl]
    if { $link == 1 && [$this isCompareViewMode] == 1 && ([$sliceNode GetSingletonTag] == "Red" || [$this isCompareViewer] == 1) } {
        set ssgui [[$::slicer3::ApplicationGUI GetApplication] GetModuleGUIByName "Slices"]
        set layout [$::slicer3::ApplicationGUI GetGUILayoutNode]
        set viewArrangement [$layout GetViewArrangement]

        set numsgui [$ssgui GetNumberOfSliceGUI]

        for { set i 0 } { $i < $numsgui } { incr i } {
            if { $i == 0} {
                set sgui [$ssgui GetFirstSliceGUI]
                set lname [$ssgui GetFirstSliceGUILayoutName]
            } else {
                set sgui [$ssgui GetNextSliceGUI $lname]
                set lname [$ssgui GetNextSliceGUILayoutName $lname]
            }

            if { $lname != "Red" && [string first "Compare" $lname] != 0 } {
              continue
            } 

            set currSliceNode [$sgui GetSliceNode]
            set currOrientString [$currSliceNode GetOrientationString]
            if { [string compare $orientString $currOrientString] == 0 || ($_actionStartOrientation != "" && [string compare $_actionStartOrientation $currOrientString] == 0)  } {
                lappend guis $sgui
            }
        }
    } else {
        lappend guis $sliceGUI
    }
  return $guis
}


# Are we in a compare view mode?
itcl::body SliceSWidget::isCompareViewMode { } {

  set layout [$::slicer3::ApplicationGUI GetGUILayoutNode]
  set viewArrangement [$layout GetViewArrangement]

  if { $viewArrangement == 12 || $viewArrangement == 13 || $viewArrangement == 17 } {
    return 1
  } else {
    return 0
  }
}

# Is this a compare viewer? (not including the red viewer)
itcl::body SliceSWidget::isCompareViewer { } {

  set lname [$_sliceNode GetSingletonTag]
  if { [string first "Compare" $lname] != 0 } {
    return 0
  } 

  return 1
}


# Locate the SliceSWidget that works with a specific gui
itcl::body SliceSWidget::getSliceSWidgetForGUI {gui} {
    # find the SliceSWidget for sgui
    set found 0
    set itclobjects [itcl::find objects -class SliceSWidget]
    foreach sw $itclobjects {
      if {[$sw cget -sliceGUI] == $gui} {
        set found 1
        break;
      }
    }

    if { $found } {
      return $sw
    } else {
      return ""
    }
}

itcl::body SliceSWidget::getInWidget { } {
    return $_inWidget
}

itcl::body SliceSWidget::startTranslate { x y  windowx windowy  rox roy  ras } {
    $::slicer3::MRMLScene SaveStateForUndo $_sliceNode

    $this requestDelayedAnnotation
    set _actionState "Translate"
    set _actionStartXY "$x $y"
    set _actionStartWindowXY "$windowx $windowy"
    set _actionStartViewportOrigin "$rox $roy"
    set _actionStartRAS $ras
    set _actionLink [$_sliceCompositeNode GetLinkedControl]
    $_sliceCompositeNode SetLinkedControl 0

    $sliceGUI SetGrabID $this
    $sliceGUI SetGUICommandAbortFlag 1

    $o(storeXYToRAS) DeepCopy [$_sliceNode GetXYToRAS]
    $o(storeSliceToRAS) DeepCopy [$_sliceNode GetSliceToRAS]
}

itcl::body SliceSWidget::endTranslate { } {
    $_sliceCompositeNode SetLinkedControl $_actionLink
    set sliceLogics [$this getLinkedSliceLogics]
    foreach logic $sliceLogics {
        set snode [$logic GetSliceNode]
        [$snode GetSliceToRAS] DeepCopy [$_sliceNode GetSliceToRAS]
        $snode UpdateMatrices
    }

    $this requestDelayedAnnotation
    set _actionState ""
    $sliceGUI SetGrabID ""
    $sliceGUI SetGUICommandAbortFlag 1
}
