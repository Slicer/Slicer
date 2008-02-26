
package require Itcl

#########################################################
#
if {0} { ;# comment

  DrawEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             DrawEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class DrawEffect] == "" } {

  itcl::class DrawEffect {

    inherit Labeler

    constructor {sliceGUI} {Labeler::constructor $sliceGUI} {}
    destructor {}

    variable _lastInsertSlice ""
    variable _actionState ""

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method apply {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}

    method positionActors {} {}
    method createPolyData {} {}
    method resetPolyData {} {}
    method addPoint {r a s} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body DrawEffect::constructor {sliceGUI} {
  set o(xyPoints) [vtkNew vtkPoints]
  set o(rasPoints) [vtkNew vtkPoints]
  set o(polyData) [$this createPolyData]

  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInput $o(polyData)
  $o(actor) SetMapper $o(mapper)
  set property [$o(actor) GetProperty]
  $property SetColor 1 1 0
  $property SetLineWidth 1
  [$_renderWidget GetRenderer] AddActor2D $o(actor)
  lappend _actors $o(actor)

  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]
}

itcl::body DrawEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body DrawEffect::positionActors { } {
  if { ![info exists o(xyPoints)] } {
    # called during startup...
    return
  }
  set rasToXY [vtkTransform New]
  $rasToXY SetMatrix [$_sliceNode GetXYToRAS]
  $rasToXY Inverse
  $o(xyPoints) Reset
  $rasToXY TransformPoints $o(rasPoints) $o(xyPoints)
  $rasToXY Delete
  $o(polyData) Modified
}

itcl::body DrawEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }

  #
  # if another widget has the grab, let this go unless
  # it is a focus event, in which case we want to update
  # out display icon
  #
  set grabID [$sliceGUI GetGrabID]
  if { $grabID != "" && $grabID != $this } {
    if { ![string match "Focus*Event" $event] } {
      return ;# some other widget wants these events
    }
  }

  chain $caller $event

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

  if { $caller == $sliceGUI } {
    switch $event {
      "LeftButtonPressEvent" {
        set _actionState "drawing"
        $sliceGUI SetGrabID $this
        eval $this addPoint $_currentPosition
        $sliceGUI SetGUICommandAbortFlag 1
      }
      "MouseMoveEvent" {
        switch $_actionState {
          "drawing" {
            eval $this addPoint $_currentPosition
            $sliceGUI SetGUICommandAbortFlag 1
          }
          default {
            $sliceGUI SetGrabID ""
          }
        }
      }
      "LeftButtonReleaseEvent" {
        set _actionState ""
        $sliceGUI SetGrabID ""
        eval $this addPoint $_currentPosition
        $sliceGUI SetGUICommandAbortFlag 1
      }
      "KeyPressEvent" { 
        set key [$_interactor GetKeySym]
        if { [lsearch "a" $key] != -1 } {
          $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
          $sliceGUI SetGUICommandAbortFlag 1
          switch [$_interactor GetKeySym] {
            "a" {
              $this apply
              set _actionState ""
            }
          }
        } else {
          # puts "wand ignoring $key"
        }
      }
      "EnterEvent" {
        $o(cursorActor) VisibilityOn
        if { [info exists o(tracingActor)] } {
         $o(tracingActor) VisibilityOn
        }
      }
      "LeaveEvent" {
        $o(cursorActor) VisibilityOff
        if { [info exists o(tracingActor)] } {
         $o(tracingActor) VisibilityOff
        }
      }
    }
  }

  if { $caller != "" && [$caller IsA "vtkMRMLSliceNode"] } {
    # 
    # make sure all points are on the current slice plane
    # - if the SliceToRAS has been modified, then we're on a different plane
    #
    set logic [$sliceGUI GetLogic]
    set currentSlice [$logic GetSliceOffset]
    if { $_lastInsertSlice != $currentSlice } {
      $this resetPolyData
    }
  }

  $this positionActors
  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body DrawEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getInputLabel] == "" } {
    $this flashCursor 3
    return
  }

  set lines [$o(polyData) GetLines]
  if { [$lines GetNumberOfCells] == 0 } {
    return
  }

  $this applyPolyMask $o(polyData)
  $this resetPolyData

}

itcl::body DrawEffect::createPolyData {} {
  # make a single-polyline polydata

  set polyData [vtkNew vtkPolyData]
  $polyData SetPoints $o(xyPoints)

  set lines [vtkCellArray New]
  $polyData SetLines $lines
  set idArray [$lines GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0

  set polygons [vtkCellArray New]
  $polyData SetPolys $polygons
  set idArray [$polygons GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0

  $polygons Delete
  $lines Delete
  return $polyData
}

itcl::body DrawEffect::resetPolyData {} {
  # return the polyline to initial state with no points
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray Reset
  $idArray InsertNextTuple1 0
  $o(xyPoints) Reset
  $o(rasPoints) Reset
  $lines SetNumberOfCells 0

  set _lastInsertSlice ""
}

itcl::body DrawEffect::addPoint {r a s} {

  # store modify time so these points can be cleared if 
  # slice plane is moved
  set logic [$sliceGUI GetLogic]
  set _lastInsertSlice [$logic GetSliceOffset]

  set p [$o(rasPoints) InsertNextPoint $r $a $s]
  set lines [$o(polyData) GetLines]
  set idArray [$lines GetData]
  $idArray InsertNextTuple1 $p
  $idArray SetTuple1 0 [expr [$idArray GetNumberOfTuples] - 1]
  $lines SetNumberOfCells 1
}

itcl::body DrawEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # an appl button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Apply current outline.\nUse the 'a' hotkey to apply in slice window"
  pack [$o(apply) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 


  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel current outline."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # a help button
  #
  set o(help) [vtkNew vtkSlicerPopUpHelpWidget]
  $o(help) SetParent [$this getOptionsFrame]
  $o(help) Create
  $o(help) SetHelpTitle "Draw"
  $o(help) SetHelpText "Use this tool to draw an outline."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(apply) AddObserver AnyEvent "::DrawEffect::ApplyAll"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Drawing"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body DrawEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

proc ::DrawEffect::ApplyAll {} {
  foreach draw [itcl::find objects -class DrawEffect] {
    $draw apply
  }
}
