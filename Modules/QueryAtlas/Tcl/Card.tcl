package require Itcl

# 
# utility to only delete an instance if it hasn't already been deleted
# (this is useful in event handling)
#
namespace eval Card {
  proc ProtectedDelete {instance} {
    if { [info command $instance] != "" } {
      itcl::delete object $instance
    }
  }
}

# 
# utility to run method only if instance hasn't already been deleted
# (this is useful in event handling)
#
namespace eval Card {
  proc ProtectedCallback {instance args} {
    if { [info command $instance] != "" } {
      eval $instance $args
    }
  }
}

proc TestCards {} {
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set card [Card #auto $renderWidget]
  $card configure -text "hoot! - I'm a card"
}

#########################################################
#
if {0} { ;# comment

  Card a class for collecting information about a card in 
  a 3D scene including it's vtk class instances and it's interaction
  state.

  Parent class of other Cards

# TODO :  this may get refactored into an SWidget3D if it makes sense...

}
#
#########################################################


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class Card] == "" } {

  itcl::class Card {

    constructor {renderWidget} {
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable state ""  ;# the interaction state of the Card
    public variable description ""  ;# a status string describing the current state
    public variable renderWidget ""  ;# the vtkKWRenderWidget in which the Card lives
    public variable iconURI ""  ;# the uri from which to grab the image (should be in png format)
    public variable text ""  ;# the text to display - can contain line breaks
    public variable follow 1  ;# make this a follower or not
    public variable font "default"  ;# what font to use (default means arial).

    variable _vtkObjects ""
    variable _pickState "outside"
    variable _actionState ""

    variable o ;# array of the objects for this widget, for convenient access
    variable _observerTags ;# save so destructor can remove observer

    # parts of the sliceGUI saved for easy access
    variable _renderer ""
    variable _interactor ""
    variable _annotation ""

    # methods
    method getObjects {} {return [array get o]}
    method processEvent {caller event} {}
    method removeActors {} {}
    method addActors {} {}
    method updateActors {} {}
    method pick {} {}
    method highlight {} {}
    method place {x y z} {}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this Card
    method vtkDelete {} {
      foreach object $_vtkObjects {
        catch "$object Delete"
      }
      set _vtkObjects ""
    }

  }
}

#
# called on 'widget configure -renderWidget rw'
# - caches handles to useful objects
#
itcl::configbody Card::renderWidget {
  $this removeActors
  #set _renderer [$renderWidget GetRenderer]
  set _renderer [[[$renderWidget GetRenderWindow] GetRenderers] GetItemAsObject 1]
  set _interactor [$renderWidget GetRenderWindowInteractor]
  set _annotation [$renderWidget GetCornerAnnotation]
  $this addActors
}

foreach param {iconURI text follow font} {
  itcl::configbody Card::$param { $this updateActors }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body Card::constructor {renderWidget} {

  $this configure -renderWidget $renderWidget

  set _observerTags ""

  set tag [$renderWidget AddObserver DeleteEvent "::Card::ProtectedDelete $this"]
  lappend _observerTags [list $renderWidget $tag]

  set events {  
    "KeyPressEvent" "LeftButtonPressEvent"
    }
  foreach event $events {
    set tag [$_interactor AddObserver $event "::Card::ProtectedCallback $this processEvent $_interactor $event"]    
    lappend _observerTags [list $_interactor $tag]
  }
}


itcl::body Card::destructor {} {

  $this removeActors

  foreach {object tag} $_observerTags {
    if { [info command $object] != "" } {
      $object RemoveObserver $tag
    }
  }

}

itcl::body Card::processEvent { caller event } {

  if { [info command $caller] == ""} {
      return
  }

  if { [info command $renderWidget] == "" } {
    # the renderWidget was deleted behind our back, so we need to 
    # self destruct
    ::Card::ProtectedDelete $this
    return
  }

  if { $caller == $_interactor } {
    switch $event {
      "KeyPressEvent" {
        switch [$_interactor GetKeySym] {
          default {
            set char [$_interactor GetKeySym]
            $this configure -text $text$char
          }
        }
      }
    }
  }
}

itcl::body Card::removeActors {} {
  if { [info exists o(actor)] } {
    $_renderer RemoveActor $o(actor)
  }
}

itcl::body Card::addActors {} {
  
  set o(tText) [vtkNew vtkTextureText]

  $this updateActors

  if { $follow } {
    set o(actor) [vtkNew vtkFollower]
    $o(actor) SetCamera [$_renderer GetActiveCamera]
  } else {
    set o(actor) [vtkNew vtkActor]
  }
  $o(actor) SetMapper [[$o(tText) GetFollower] GetMapper]
  $o(actor) SetTexture [$o(tText) GetTexture]
  $_renderer AddActor $o(actor)
}

itcl::body Card::updateActors {} {

  set fontParams [$o(tText) GetFontParameters] 
  if { $font != "default" } {
    $fontParams SetFontFileName $font
  } else {
    $fontParams SetFontFileName "SHOWG.TTF"
  }
  $fontParams SetFontDirectory c:/WINDOWS/Fonts/
  $fontParams SetBlur 2
  $fontParams SetStyle 2
  $o(tText) SetText $text
  $o(tText) CreateTextureText
}

