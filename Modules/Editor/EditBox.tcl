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
  proc ShowDialog { {path ""} } {

    set editBoxes [itcl::find objects -class EditBox]
    if { $editBoxes != "" } {
      set editBox [lindex $editBoxes 0]
      array set o [$editBox objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set editBox [EditBox #auto]
    }
    if { $path != "" } {
      $editBox add $path
    }
  }

}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class EditBox] == "" } {

  itcl::class EditBox {

    constructor  { {frame ""} } {
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable frame ""  ;# the frame to build in (if "", build in dialog)
    public variable mode "dialog"  ;# or "menu" which goes away after click

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access

    variable _observerRecords ;# list of the observers so we can clean up
    variable _effects ;# array of effects (icons plus classes to invoke)

    # methods
    method create {} {}
    method findEffects { {path ""} } {}
    method show {} {}
    method hide {} {}
    method processEvents {caller} {}
    method errorDialog {errorText} {}

    method objects {} {return [array get o]}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this EditBox
    method vtkDelete {} {
      foreach object $_vtkObjects {
        catch "$object Delete"
      }
      set _vtkObjects ""
    }

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body EditBox::constructor { {frame ""} } {

  $this configure -frame $frame

}


itcl::body EditBox::destructor {} {

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this vtkDelete

}

itcl::configbody EditBox::mode {

  switch $mode {
    "menu" {
      # remove decorations from the toplevel
      wm overrideredirect [$o(toplevel) GetWidgetName] 1
    }
    "dialog" {
      wm overrideredirect [$o(toplevel) GetWidgetName] 0
    }
    default {
      error "unknown mode $mode"
    }
  }
}


# fill the _effects array bases on what you find in the interpreter
itcl::body EditBox::findEffects { {path ""} } {

  # for now, the built in effects are hard-coded to facilitate
  # the icons and layout

  array unset _effects

  set _effects(list) {
    ChangeIslands ChangeLabel ChooseColor ConnectedComponents DeleteFiducials
    DeleteIslands DilateLabel EraseLabel ErodeLabel FiducialVisibilityOff
    FiducialVisibilityOn FreehandDrawLabel GoToEditorModule IdentifyIslands
    ImplicitCube ImplicitEllipse ImplicitRectangle InterpolateLabels LabelOpacity
    LabelVisibilityOff LabelVisibilityOn MakeModel NextFiducial PaintLabel
    PinOpen PreviousFiducial SaveIslands SlurpColor SnapToGridOff SnapToGridOn
    ThresholdBucket ThresholdPaintLabel Threshold ToggleLabelOutline Watershed
  }

  set iconDir $::env(SLICER_HOME)/lib/Slicer3/Modules/Packages/Editor/ImageData
  set reader [vtkPNGReader New]
  foreach effect $_effects(list) {
    set _effects($effect,class) EffectSWidget
    set _effects($effect,icon) [vtkNew vtkKWIcon]
    $reader SetFileName $iconDir/$effect.png
    $reader Update
    $::slicer3::ApplicationGUI SetIconImage $_effects($effect,icon) [$reader GetOutput]
  }
  $reader Delete
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
    WM_DELETE_WINDOW "itcl::delete object $this"

  #
  # the buttons
  #

  set o(buttonFrame) [vtkNew vtkKWFrame]
  $o(buttonFrame) SetParent $o(toplevel)
  $o(buttonFrame) Create
  pack [$o(buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set widgetList ""
  foreach effect {PaintLabel ThresholdPaintLabel FreehandDrawLabel ThresholdBucket} {
    set o(effect) [vtkNew vtkKWPushButton]
    $o(effect) SetParent $o(buttonFrame)
    $o(effect) Create
    $o(effect) SetText $effect
    $o(effect) SetImageToIcon $_effects($effect,icon)
    $o(effect) SetBalloonHelpString $effect
    set tag [$o(effect) AddObserver ModifiedEvent "$this processEvents $o(effect)"]
    lappend _observerRecords [list $o(effect) $tag]
    $o(effect) SetCommand $o(effect) Modified
    lappend widgetList [$o(effect) GetWidgetName]
  }

  eval pack $widgetList -side left -anchor w -padx 4 -pady 2

  $o(toplevel) Display
}



#
# handle gui events
# -basically just map button events onto methods
#
itcl::body EditBox::processEvents { caller } {

  foreach effect $_effects(list) {
    if { $caller == $o(effect) } {
      EffectSWidget::Add $_effects($effect,class)
    }
  }

  if { $caller == $o(addDir) } {
    # TODO: switch to kwwidgets directory browser
    $this add [$this chooseDirectory]
    return
  }

  if { $caller == $o(addFile) } {
    # TODO: switch to kwwidgets directory browser
    $this add [$this getOpenFile]
    return
  }

  if { $caller == $o(apply) } {
    $this apply
    after idle "itcl::delete object $this"
    return
  }

  if { $caller == $o(cancel) } {
    after idle "itcl::delete object $this"
    return
  }
  
  puts "unknown event from $caller"
}

itcl::body EditBox::errorDialog { errorText } {
  puts $errorText
}

