
package require Itcl

#########################################################
#
if {0} { ;# comment

  SaveIslandEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             SaveIslandEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SaveIslandEffect] == "" } {

  itcl::class SaveIslandEffect {

    inherit EffectSWidget

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    # methods
    method processEvent {} {}
    method preview {} {}
    method apply {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body SaveIslandEffect::constructor {sliceGUI} {
  # rely on superclass constructor
}

itcl::body SaveIslandEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body SaveIslandEffect::processEvent { } {

  if { [[$this superclass] preProcessEvent] } {
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

  switch $event {
    "LeftButtonPressEvent" {
      set _actionState "painting"
      foreach {x y} [$_interactor GetEventPosition] {}
      if { $paintDropper } {
        # in Dropper mode, set the paint color to be the first pixel you touch
        $this queryLayers $x $y
        set paintColor [$this getPixel $_layers(label,image) \
          $_layers(label,i) $_layers(label,j) $_layers(label,k)]
      }
      $this paintAddPoint $x $y
      $sliceGUI SetGUICommandAbortFlag 1
      $sliceGUI SetGrabID $this
      [$_renderWidget GetRenderWindow] HideCursor
    }
    "MouseMoveEvent" {
      if {0} {
        $o(cursorActor) VisibilityOn
        switch $_actionState {
          "painting" {
            foreach {x y} [$_interactor GetEventPosition] {}
            $this paintAddPoint $x $y
          }
          default {
          }
        }
      }
    }
    "LeftButtonReleaseEvent" {
      $this paintApply
      [$_renderWidget GetRenderWindow] ShowCursor
      $_layers(label,node) Modified
      set _actionState ""
      $sliceGUI SetGrabID ""
      set _description ""
    }
    "EnterEvent" {
      set _description "Ready Save Islands!"
      $o(cursorActor) VisibilityOn
    }
    "LeaveEvent" {
      set _description ""
      $o(cursorActor) VisibilityOff
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body SaveIslandEffect::apply {} {
  # to be overridden by subclass
}


proc SaveIslandEffect::AddSaveIsland {} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      SaveIslandEffect #auto $sliceGUI
    }
  }
}

proc SaveIslandEffect::RemoveSaveIsland {} {
  foreach pw [itcl::find objects -class SaveIslandEffect] {
    itcl::delete object $pw
  }
}

proc SaveIslandEffect::ToggleSaveIsland {} {
  if { [itcl::find objects -class SaveIslandEffect] == "" } {
    SaveIslandEffect::AddSaveIsland
  } else {
    SaveIslandEffect::RemoveSaveIsland
  }
}

proc SaveIslandEffect::ConfigureAll { args } {
  foreach pw [itcl::find objects -class SaveIslandEffect] {
    eval $pw configure $args
  }
}
