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

    set editBoxes [itcl::find objects -class EditBox]
    if { $editBoxes != "" } {
      set editBox [lindex $editBoxes 0]
      $editBox centerOnPointer
      $editBox show
    } else {
      set editBox [EditBox #auto]
      $editBox configure -mode "popup"
      $editBox create
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
    public variable mode "dialog"  ;# or "menu" or "popup" which goes away after click

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access

    variable _observerRecords "" ;# list of the observers so we can clean up
    variable _effects ;# array of effects (icons plus classes to invoke)

    # methods
    method create {} {}
    method createButtonRow {effects} {}
    method findEffects { {path ""} } {}
    method centerOnPointer { {xy ""} } {}
    method show {} {}
    method hide {} {}
    method selectEffect {effect} {}
    method processEvents {caller} {}
    method errorDialog {errorText} {}

    method objects {} {return [array get o]}
    method effects {} {return [array get _effects]}

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
    set _effects($effect,imageData) [vtkImageData New]
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

  $this createButtonRow {SnapToGridOn ChooseColor SlurpColor}
  $this createButtonRow {ChangeLabel ToggleLabelOutline LabelVisibilityOn}
  $this createButtonRow {PaintLabel ThresholdPaintLabel FreehandDrawLabel ThresholdBucket}
  $this createButtonRow {EraseLabel ImplicitEllipse ImplicitRectangle ImplicitCube}
  $this createButtonRow {IdentifyIslands ChangeIslands DeleteIslands SaveIslands}
  $this createButtonRow {ErodeLabel DilateLabel Threshold ChangeLabel}
  $this createButtonRow {InterpolateLabels MakeModel Watershed ConnectedComponents}
  $this createButtonRow {PreviousFiducial NextFiducial FiducialVisibilityOn DeleteFiducials}
  $this createButtonRow {GoToEditorModule PinOpen}
 
  if { $mode == "popup" } {
  }
  switch $mode {
    "popup" {
      # remove decorations from the toplevel and position at cursor
      wm overrideredirect [$o(toplevel) GetWidgetName] 1
      $o(toplevel) SetDisplayPositionToPointer
    }
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

  $o(toplevel) Display
}

itcl::body EditBox::centerOnPointer { {xy ""} } {

  # find the pointer (or used passed coord)
  if { $xy != "" } {
    foreach {x y} {$xy} {}
  } else {
    foreach {x y} [winfo pointerxy .] {}
  }

  # find the width, height, location of the toplevel (t)
  set t [$o(toplevel) GetWidgetName]
  set geom [wm geometry $t]
  set whxy [split $geom "+x"]
  foreach {tw th tx ty} $whxy {}
  set tw2 [expr $tw / 2]
  set th2 [expr $th / 2]

  # center the toplevel around the cursor
  set newx [expr $x - $tw2]
  set newy [expr $y - $th2]

  # try to keep the window on the screen
  if { $newx < 0 } { set newx 50 }
  if { $newy < 0 } { set newy 50 }
  set sw [winfo screenwidth .]
  set sh [winfo screenheight .]
  if { [expr $newx + $tw] > $sw } { set newx [expr $sw - $tw - 50] }
  if { [expr $newy + $th] > $sh } { set newy [expr $sh - $th - 50] }

  wm geometry $t +$newx+$newy
}

itcl::body EditBox::show {} {
  wm deiconify [$o(toplevel) GetWidgetName]
  raise [$o(toplevel) GetWidgetName]
  focus [$o(toplevel) GetWidgetName]
}

itcl::body EditBox::hide {} {
  wm withdraw [$o(toplevel) GetWidgetName]
}

#
# manage the editor effects
#
itcl::body EditBox::selectEffect { effect } {

  EffectSWidget::RemoveAll
  EffectSWidget::Add $_effects($effect,class)
  EffectSWidget::SetCursorAll $_effects($effect,class) $_effects($effect,imageData)

  switch $mode {
    "popup" -
    "menu" {
      $this hide
    }
  }
}



#
# handle gui events
# -basically just map button events onto methods
#
itcl::body EditBox::processEvents { caller } {

  puts "caller is $caller"

  foreach effect $_effects(list) {
    if { $caller == $o(effect) } {
      EffectSWidget::Add $_effects($effect,class)
      return
    }
  }


#  if { $caller == $o(cancel) } {
#    after idle "itcl::delete object $this"
#    return
#  }
  
  puts "unknown event from $caller"
}

itcl::body EditBox::errorDialog { errorText } {
  puts $errorText
}


