package require Itcl

#########################################################
#
if {0} { ;# comment

  EditBox is a wrapper around a set of kwwidgets and other
  structures to manage the slicer3 edit box.  

# TODO : 

}
#
#########################################################

#
# namespace procs
#

namespace eval EditBox {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog {} {
    ::Box::ShowDialog EditBox
  }

  # set the state of an icon in all edit boxes (e.g. undo/redo)
  proc SetButtonState {effect state} {
    foreach editBox [itcl::find objects -class EditBox] {
      $editBox setButtonState $effect $state
    }
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class EditBox] == "" } {

  itcl::class EditBox {

    inherit Box

    variable _effects ;# array of effects (icons plus classes to invoke)

    # methods
    method create { } {}
    method createButtonRow {parent effects} {}
    method setButtonState {effect state} {}
    method findEffects { {path ""} } {}
    method selectEffect {effect} {}
    method processEvent {{caller ""} {event ""}} {}

    method effects {} {return [array get _effects]}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# - rely on superclass
# ------------------------------------------------------------------


# fill the _effects array bases on what you find in the interpreter
itcl::body EditBox::findEffects { {path ""} } {

  # for now, the built in effects are hard-coded to facilitate
  # the icons and layout

  array unset _effects

  # effects that change the mouse cursor
  set _effects(list,mouseTools) {
    ChangeIsland ChooseColor 
    ImplicitCube ImplicitEllipse ImplicitRectangle 
    Draw RemoveIslands ConnectedComponents 
    ThresholdBucket ThresholdPaintLabel SaveIsland SlurpColor Paint
    DefaultTool LevelTracing MakeModel Wand
  }

  # effects that operate from the menu
  set _effects(list,operations) {
    ErodeLabel DilateLabel DeleteFiducials LabelOpacity
    ChangeLabel FiducialVisibilityOff
    FiducialVisibilityOn GoToEditorModule 
    IdentifyIslands
    LabelVisibilityOff LabelVisibilityOn NextFiducial 
    SnapToGridOff SnapToGridOn
    EraseLabel Threshold PinOpen PreviousFiducial  InterpolateLabels LabelOpacity
    ToggleLabelOutline Watershed PreviousCheckPoint NextCheckPoint
  }

  # these buttons do not switch you out of the current tool
  set _effects(list,nonmodal) {
    FiducialVisibilityOn LabelVisibilityOff LabelVisibilityOn
    NextFiducial PreviousFiducial DeleteFiducials SnapToGridOn SnapToGridOff
    EraseLabel PreviousCheckPoint NextCheckPoint ToggleLabelOutline
    SnapToGridOff SnapToGridOn LabelOpacity
  }

  # these buttons start disabled (check points will re-enable when circumstances are right)
  set _effects(list,disabled) {
    ChooseColor 
    ImplicitCube ImplicitEllipse 
    ConnectedComponents 
    SlurpColor 
    ThresholdPaintLabel ThresholdBucket
    DeleteFiducials LabelOpacity
    FiducialVisibilityOff
    FiducialVisibilityOn 
    LabelVisibilityOff LabelVisibilityOn 
    SnapToGridOff SnapToGridOn
    InterpolateLabels LabelOpacity
    ToggleLabelOutline Watershed Wand
    PreviousCheckPoint NextCheckPoint
  }


  # combined list of all effects
  set _effects(list) [concat $_effects(list,mouseTools) $_effects(list,operations)]

  # for each effect
  # - look for implementation class of pattern *Effect
  # - get an icon  for the pushbutton
  set iconDir [file join [[$::Editor(singleton) GetLogic] GetModuleShareDirectory] "ImageData"]
  
  set reader [vtkPNGReader New]
  foreach effect $_effects(list) {
    if { [info command ${effect}Effect] != "" } {
      set _effects($effect,class) ${effect}Effect
    } else {
      set _effects($effect,class) EffectSWidget
    }
    foreach iconType { "" Selected Disabled} {
      set _effects($effect,imageData$iconType) [vtkNew vtkImageData]
      $reader SetFileName $iconDir/$effect$iconType.png
      $reader Update
      $_effects($effect,imageData$iconType) DeepCopy [$reader GetOutput]
    }

    set iconMode ""
    if { [lsearch $_effects(list,disabled) $effect] != -1 } {
      set iconMode "Disabled"
    }
    set _effects($effect,icon) [vtkNew vtkKWIcon]
    $::slicer3::ApplicationGUI SetIconImage \
        $_effects($effect,icon) $_effects($effect,imageData$iconMode)
  }
  $reader Delete

}

# needs to be a valid effect name and state of "", Disabled, or Selected
itcl::body EditBox::setButtonState {effect state} {
  $::slicer3::ApplicationGUI SetIconImage \
      $_effects($effect,icon) $_effects($effect,imageData$state)
  $o($effect,button) SetImageToIcon $_effects($effect,icon)
  switch $state {
    Selected -
    "" {
      $o($effect,button) SetState 1
    }
    "Disabled" {
      $o($effect,button) SetState 0
    }
  }
}

#
# create a row of the edit box given a list of 
# effect names (items in _effects(list)
#
itcl::body EditBox::createButtonRow {parent effects} {

  set name [vtkNew vtkKWFrame]
  set o($name,buttonFrame) $name
  $o($name,buttonFrame) SetParent $parent
  $o($name,buttonFrame) Create
  pack [$o($name,buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set widgetList ""

  foreach effect $effects {
    set o($effect,button) [vtkNew vtkKWPushButton]
    set pushButton $o($effect,button)
    $pushButton SetParent $o($name,buttonFrame)
    $pushButton Create
    $pushButton SetText $effect
    $pushButton SetImageToIcon $_effects($effect,icon)
    $pushButton SetBalloonHelpString $effect
    $pushButton SetBorderWidth 0
    if { [lsearch $_effects(list,disabled) $effect] != -1 } {
      $pushButton SetStateToDisabled
    }

    #
    # TODO: would prefer to use the events for consistency, but apparently
    # there is no way to observe the InvokedEvent from wrapped languages
    #set tag [$pushButton AddObserver ModifiedEvent "$this processEvent $pushButton"]
    #lappend _observerRecords [list $pushButton $tag]
    #$pushButton SetCommand $pushButton Modified
    ## AND there is no way to pass a script to the command
    #SetCommand(vtkObject *object, const char *method);
    #
    # - THIS IS REALLY ANNOYING!
    #

    # instead I do this, which is elegant, but breaks the KWWidgets abstraction (a bit)
    [$pushButton GetWidgetName] configure -command  "$this selectEffect $effect"

    $pushButton SetBalloonHelpString $effect

    lappend widgetList [$pushButton GetWidgetName]
  }
  eval pack $widgetList -side left -anchor w -padx 4 -pady 2
}

# create the edit box
itcl::body EditBox::create { } {

  if { $frame != "" } {
    set parent $frame
  } else {

    #
    # make the toplevel 
    #
    set o(toplevel) [vtkNew vtkKWTopLevel]
    $o(toplevel) SetApplication $::slicer3::Application
    $o(toplevel) SetTitle "Edit Box"
    $o(toplevel) Create

    # delete this instance when the window is closed
    wm protocol [$o(toplevel) GetWidgetName] \
      WM_DELETE_WINDOW "$this hide"
    bind [$o(toplevel) GetWidgetName] <Key-space> "$this hide; focus [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetWidgetName]"
    bind [$o(toplevel) GetWidgetName] <Key-F1> "$this hide; focus [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetWidgetName]"
    bind [$o(toplevel) GetWidgetName] <Key-Escape> "$this selectEffect DefaultTool; $this hide"

    set parent $o(toplevel)

  }


  $this findEffects

  #
  # the buttons
  #

  $this createButtonRow $parent {DefaultTool EraseLabel}
  $this createButtonRow $parent {Paint Draw LevelTracing ImplicitRectangle}
  $this createButtonRow $parent {IdentifyIslands ChangeIsland RemoveIslands SaveIsland}
  $this createButtonRow $parent {ErodeLabel DilateLabel Threshold ChangeLabel}
  $this createButtonRow $parent {MakeModel}
  $this createButtonRow $parent {PreviousFiducial NextFiducial}
  if { $frame == "" } {
    $this createButtonRow $parent {PreviousCheckPoint NextCheckPoint GoToEditorModule PinOpen}
  } else {
    $this createButtonRow $parent {PreviousCheckPoint NextCheckPoint}
  }
 
  $this setMode $mode 

  if { $frame != "" } {
    # nothing, calling code will pack the frame
  } else {
    $o(toplevel) Display
  }
}

#
# manage the editor effects
#
itcl::body EditBox::selectEffect { effect } {

  if { [lsearch $_effects(list,nonmodal) $effect] == -1 } {
    EffectSWidget::RemoveAll
    EditorSetActiveToolLabel $effect
  }

  # mouse tool changes cursor, and dismisses popup/menu
  set mouseTool 0
  if { [lsearch $_effects(list,mouseTools) $effect] != -1 } {
    set mouseTool 1
  }

  switch $effect {
    "DefaultTool" {
      # do nothing - this will reset cursor mode
      EditorSetActiveToolLabel DefaultTool
    }
    "PinOpen" {
      $this togglePin
      EditorSetActiveToolLabel DefaultTool
    }
    "GoToEditorModule" {
      EditorSelectModule
      EditorSetActiveToolLabel DefaultTool
    }
    "LabelCheckPoint" {
      # save a copy of the current label layer into the scene
      EditorLabelCheckPoint
      EditorSetActiveToolLabel DefaultTool
    }
    "PreviousFiducial" {
      ::FiducialsSWidget::JumpAllToNextFiducial -1
      EditorSetActiveToolLabel DefaultTool
    }
    "NextFiducial" {
      ::FiducialsSWidget::JumpAllToNextFiducial 1
      EditorSetActiveToolLabel DefaultTool
    }
    "EraseLabel" {
      EditorToggleErasePaintLabel
    }
    "PreviousCheckPoint" {
      EditorPerformPreviousCheckPoint
    }
    "NextCheckPoint" {
      EditorPerformNextCheckPoint
    }
    default {

      #
      # create an instance of the effect for each of the active sliceGUIs
      # - have the effect reset the tool label when completed
      #

      set ret [catch "EffectSWidget::Add $_effects($effect,class)" res]
      if { $ret } {
        puts $::errorInfo
        EditorErrorDialog $res
      } else {
        EffectSWidget::ConfigureAll $_effects($effect,class) -exitCommand "EditorSetActiveToolLabel DefaultTool"

        if { $mouseTool } {
          EffectSWidget::SetCursorAll $_effects($effect,class) $_effects($effect,imageData)
        }
      }

    }
  }

  #
  # if an effect was added, find the first instance and have it build an 
  # options GUI
  #
  set w [lindex [itcl::find objects -class $_effects($effect,class)] 0]
  if { $w != "" } {
    $w buildOptions
    $w previewOptions
  }

  switch $mode {
    "popup" -
    "menu" {
      if { $mouseTool } {
        $this hide
      }
    }
    "dialog" {
    }
  }
}



#
# handle gui events
# -basically just map button events onto methods
# - not used due to KWWidgets limitations
#
itcl::body EditBox::processEvent { {caller ""} {event ""} } {

}

#
# TODO: this little helper reloads the editor functionality
#
proc eeeee {} {

  EffectSWidget::RemoveAll

  if { ![info exists ::Editor(singleton)] } {
    error "editor not yet loaded"
  }
  set editor $::Editor(singleton)
  set lib_tcl_dir [file join [[$editor GetLogic] GetModuleLibDirectory] "Tcl"]

  EditorTearDownGUI $::Editor(singleton)

  foreach eff [itcl::find objects -isa EffectSWidget] {
    itcl::delete object $eff
  }
  foreach box [itcl::find objects -isa Box] {
    itcl::delete object $box
  }

  itcl::delete class Box
  itcl::delete class EffectSWidget


  source $lib_tcl_dir/Box.tcl
  source $lib_tcl_dir/EffectSWidget.tcl
  source $lib_tcl_dir/Labeler.tcl
  foreach eff [glob $lib_tcl_dir/*Effect.tcl] {
    source $eff
  }
  foreach box [glob $lib_tcl_dir/*Box.tcl] {
    source $box
  }
  source $lib_tcl_dir/EditColor.tcl

  EditorBuildGUI $editor
}
