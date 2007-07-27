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
    method create {} {}
    method createButtonRow {effects} {}
    method findEffects { {path ""} } {}
    method selectEffect {effect} {}
    method processEvents {caller} {}

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
    ChangeIslands ChangeLabel ChooseColor 
    ImplicitCube ImplicitEllipse ImplicitRectangle 
    FreehandDrawLabel EraseLabel RemoveIslands ConnectedComponents 
    ThresholdBucket ThresholdPaintLabel SaveIsland SlurpColor PaintLabel
    DefaultTool
  }
  # effects that operate from the menu
  set _effects(list,operations) {
    ErodeLabel DilateLabel DeleteFiducials
    FiducialVisibilityOff
    FiducialVisibilityOn GoToEditorModule 
    IdentifyIslands
    LabelVisibilityOff LabelVisibilityOn MakeModel NextFiducial 
    SnapToGridOff SnapToGridOn
    Threshold PinOpen PreviousFiducial  InterpolateLabels LabelOpacity
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
    set _effects($effect,imageData) [vtkNew vtkImageData]
    $reader SetFileName $iconDir/$effect.png
    $reader Update
    $_effects($effect,imageData) DeepCopy [$reader GetOutput]
    $::slicer3::ApplicationGUI SetIconImage $_effects($effect,icon) [$reader GetOutput]
  }
  $reader Delete
}

#
# create a row of the edit box given a list of 
# effect names (items in _effects(list)
#
itcl::body EditBox::createButtonRow {effects} {

  set name [vtkNew vtkKWFrame]
  set o($name,buttonFrame) $name
  $o($name,buttonFrame) SetParent $o(toplevel)
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
    [$pushButton GetBalloonHelpManager] SetDelay 300

    #
    # TODO: would prefer to use the events for consistency, but apparently
    # there is no way to observe the InvokedEvent from wrapped languages
    #set tag [$pushButton AddObserver ModifiedEvent "$this processEvents $pushButton"]
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
    error "packing in existing frame not yet supported"
  }

  $this findEffects

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
  bind [$o(toplevel) GetWidgetName] <Key-space> "$this hide"

  #
  # the buttons
  #

  $this createButtonRow {DefaultTool SnapToGridOn ChooseColor SlurpColor}
  $this createButtonRow {ChangeLabel ToggleLabelOutline LabelVisibilityOn}
  $this createButtonRow {PaintLabel ThresholdPaintLabel FreehandDrawLabel ThresholdBucket}
  $this createButtonRow {EraseLabel ImplicitEllipse ImplicitRectangle ImplicitCube}
  $this createButtonRow {IdentifyIslands ChangeIslands RemoveIslands SaveIsland}
  $this createButtonRow {ErodeLabel DilateLabel Threshold ChangeLabel}
  $this createButtonRow {InterpolateLabels MakeModel Watershed ConnectedComponents}
  $this createButtonRow {PreviousFiducial NextFiducial FiducialVisibilityOn DeleteFiducials}
  $this createButtonRow {GoToEditorModule PinOpen}
 
  $this setMode $mode

  $o(toplevel) Display
}

#
# manage the editor effects
#
itcl::body EditBox::selectEffect { effect } {

  $this statusText "Selecting $effect"
  EffectSWidget::RemoveAll

  # mouse tool changes cursor, and dismisses popup/menu
  set mouseTool 0
  if { [lsearch $_effects(list,mouseTools) $effect] != -1 } {
    set mouseTool 1
  }

  switch $effect {
    "DefaultTool" {
      # do nothing - this will reset cursor mode
    }
    "PinOpen" {
      $this togglePin
    }
    "GoToEditorModule" {
      EditorSelectModule
    }
    "MakeModel" {
      #TODO: invoke the real modelmaker.  Figure out which label map to use (each slice
      # could have a different label layer -- for now use the red one...
      EditorTestQuickModel
    }
    default {
      EffectSWidget::Add $_effects($effect,class)
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
itcl::body EditBox::processEvents { caller } {

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
