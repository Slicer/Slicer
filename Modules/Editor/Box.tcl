package require Itcl

#########################################################
#
if {0} { ;# comment

  Box is a wrapper around a set of kwwidgets and 
  others to create a popup dialog

# TODO : 

}
#
#########################################################

#
# namespace procs
#

namespace eval Box {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog { class } {

    set boxes [itcl::find objects -class $class]
    foreach box $boxes {
      array set o [$box objects]
      if { [info exists o(toplevel)] } {
        $box centerOnPointer
        $box update
        $box show
        return
      }
    } 
    # only get here if no toplevel box exists yet

    [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText "Creating Popup Box..."
    $::slicer3::Application ProcessPendingEvents
    set box [$class #auto]
    $box configure -mode "popup"
    $box create
    [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText ""
  }
}

# 
# utility to run method only if instance hasn't already been deleted
# (this is useful in event handling)
#
namespace eval Box {
  proc ProtectedCallback {instance args} {
    if { [info command $instance] != "" } {
      if { [catch "eval $instance $args" res] } {
        puts $res
        puts $::errorInfo
      }
    }
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class Box] == "" } {

  itcl::class Box {

    constructor  { {frame ""} } {
      $this configure -frame $frame
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

    # methods
    method create {} {}
    method update {} {}
    method centerOnPointer { {xy ""} } {}
    method show {} {}
    method hide {} {}
    method setMode {mode} {}
    method togglePin {} {}
    method processEvent {{caller ""} {event ""}} {}
    method errorDialog {errorText} {}

    method objects {} {return [array get o]}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this Box
    method vtkDelete {} {
      foreach object $_vtkObjects {
        if { [$object IsA "vtkKWWidget"] } {
          $object SetParent ""
        }
        if { [catch "$object Delete" res] } {
          tk_messageBox -message "cleaning box, got:\n\n$res"
        }
      }
      set _vtkObjects ""
    }

    # interact with the status line on the main window
    method statusText {msg} {
      [$::slicer3::ApplicationGUI GetMainSlicerWindow]  SetStatusText $msg
    }

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body Box::constructor { {frame ""} } {

  $this configure -frame $frame

}


itcl::body Box::destructor {} {

  EffectSWidget::RemoveAll

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this vtkDelete

}

itcl::configbody Box::mode {
  $this setMode $mode
}

# break this out from the configbody so it can be called
# directly from create (but then can be changed later)
itcl::body Box::setMode {mode} {

  if { [info exists o(toplevel)] } {
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
  }
}


# create the edit box
itcl::body Box::create { } {

  if { $frame != "" } {
    error "packing in existing frame not yet supported"
  }

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Box"
  $o(toplevel) Create

  # delete this instance when the window is closed
  wm protocol [$o(toplevel) GetWidgetName] \
    WM_DELETE_WINDOW "$this hide"
  bind [$o(toplevel) GetWidgetName] <Key-space> "$this hide"

  $this setMode $mode


  $o(toplevel) Display
}

itcl::body Box::centerOnPointer { {xy ""} } {


  if { ![info exists o(toplevel)] } {
    # box is not a toplevel, don't bother centering
    return
  }

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

itcl::body Box::show {} {
  wm deiconify [$o(toplevel) GetWidgetName]
  raise [$o(toplevel) GetWidgetName]
  focus [$o(toplevel) GetWidgetName]
}

itcl::body Box::hide {} {
  wm withdraw [$o(toplevel) GetWidgetName]
}

itcl::body Box::togglePin {} {
  switch $mode {
    "popup" -
    "menu" {
      $this configure -mode "dialog"
      if { [info exists o(toplevel)] } {
        after idle raise [$o(toplevel) GetWidgetName]
      }
    }
    "dialog" {
      $this configure -mode "popup"
    }
  }
}
  

#
# handle gui events
# -basically just map button events onto methods
#
itcl::body Box::processEvent { {caller ""} {event ""} } {

  puts "unknown event from $caller"
}

itcl::body Box::errorDialog { errorText } {
  puts $errorText
}

