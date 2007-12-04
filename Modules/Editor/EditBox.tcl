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
    ChangeIsland ChooseColor GoToEditorModule 
    ImplicitCube ImplicitEllipse ImplicitRectangle 
    Draw EraseLabel RemoveIslands ConnectedComponents 
    ThresholdBucket ThresholdPaintLabel SaveIsland SlurpColor Paint
    DefaultTool LevelTracing Wand
  }

  # effects that operate from the menu
  set _effects(list,operations) {
    ErodeLabel DilateLabel DeleteFiducials LabelOpacity
    ChangeLabel FiducialVisibilityOff
    FiducialVisibilityOn GoToEditorModule 
    IdentifyIslands
    LabelVisibilityOff LabelVisibilityOn MakeModel NextFiducial 
    SnapToGridOff SnapToGridOn
    Threshold PinOpen PreviousFiducial  InterpolateLabels LabelOpacity
    ToggleLabelOutline Watershed
  }

  set _effects(list,disabled) {
    ChooseColor 
    ImplicitCube ImplicitEllipse ImplicitRectangle 
    ConnectedComponents 
    SlurpColor  Wand
    DeleteFiducials LabelOpacity
    FiducialVisibilityOff
    FiducialVisibilityOn 
    IdentifyIslands
    LabelVisibilityOff LabelVisibilityOn 
    SnapToGridOff SnapToGridOn
    InterpolateLabels LabelOpacity
    ToggleLabelOutline Watershed
  }


  # combined list of all effects
  set _effects(list) [concat $_effects(list,mouseTools) $_effects(list,operations)]

  set iconDir $::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/Editor/ImageData
  set reader [vtkPNGReader New]
  foreach effect $_effects(list) {
    if { [info command ${effect}Effect] != "" } {
      set _effects($effect,class) ${effect}Effect
    } else {
      set _effects($effect,class) EffectSWidget
    }
    set _effects($effect,icon) [vtkNew vtkKWIcon]
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
    $::slicer3::ApplicationGUI SetIconImage \
        $_effects($effect,icon) $_effects($effect,imageData$iconMode)
  }
  $reader Delete
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
    [$pushButton GetBalloonHelpManager] SetDelay 300
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

  $this createButtonRow $parent {DefaultTool SnapToGridOn ChooseColor SlurpColor}
  $this createButtonRow $parent {LabelOpacity ToggleLabelOutline LabelVisibilityOn Wand}
  $this createButtonRow $parent {Paint ThresholdPaintLabel Draw ThresholdBucket}
  $this createButtonRow $parent {EraseLabel ImplicitEllipse ImplicitRectangle ImplicitCube}
  $this createButtonRow $parent {IdentifyIslands ChangeIsland RemoveIslands SaveIsland}
  $this createButtonRow $parent {ErodeLabel DilateLabel Threshold ChangeLabel}
  $this createButtonRow $parent {InterpolateLabels MakeModel Watershed LevelTracing}
  $this createButtonRow $parent {PreviousFiducial NextFiducial FiducialVisibilityOn DeleteFiducials}
  $this createButtonRow $parent {GoToEditorModule PinOpen }
 
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

  EffectSWidget::RemoveAll
  EditorSetActiveToolLabel $effect

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
    "MakeModel" {
      #TODO: invoke the real modelmaker.  Figure out which label map to use (each slice
      # could have a different label layer -- for now use the red one...
      EditorTestQuickModel
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
    default {

      #
      # create an instance of the effect for each of the active sliceGUIs
      # - have the effect reset the tool label when completed
      #
      
      EffectSWidget::Add $_effects($effect,class)
      EffectSWidget::ConfigureAll $_effects($effect,class) -exitCommand "EditorSetActiveToolLabel DefaultTool"

      if { $mouseTool } {
        EffectSWidget::SetCursorAll $_effects($effect,class) $_effects($effect,imageData)
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
  itcl::delete class Box
  itcl::delete class EffectSWidget

  source $::env(SLICER_HOME)/../Slicer3/Modules/Editor/Box.tcl
  source $::env(SLICER_HOME)/../Slicer3/Modules/Editor/EffectSWidget.tcl
  foreach eff [glob $::env(SLICER_HOME)/../Slicer3/Modules/Editor/*Effect.tcl] {
    source $eff
  }
  foreach box [glob $::env(SLICER_HOME)/../Slicer3/Modules/Editor/*Box.tcl] {
    source $box
  }
}
