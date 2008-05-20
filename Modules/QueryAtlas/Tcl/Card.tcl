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

namespace eval Card {
  proc HitTest {x y} {
    foreach card [itcl::find objects -class Card] {
      $card configure -highlight 0
    }
    foreach card [itcl::find objects -class Card] {
      if { [$card hitTest $x $y] } {
        $card configure -highlight 1
        return $card
      } 
    }
    return ""
  }
}



proc randRange {min max} {
  return [expr $min + rand() * ($max-$min)]
}

proc Slicer3TestCards {} {
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  # TODO this line below was commented, if it is used make sure the corresponding
  # file is both configure'ed to the build dir and installed
  #set icondir $::env(Slicer3_HOME)/../Slicer3/Utilities/Launcher/3DSlicerLogoICOimages
  set icon $icondir/3DSlicerLogo-DesktopIcon-48x48x256.png
  for { set i 0 } { $i < 20 } { incr i } {
    set card [Card #auto $renderWidget]
    set ras [list [randRange -100 100] [randRange -100 100] [randRange -100 100]]
    set scale [randRange 1 30]
    $card configure -text "card $i" -ras $ras -scale $scale -icon $icon -follow 0
  }
}

proc TestCards {} {
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set icondir $::env(Slicer3_HOME)/share/Slicer3/Modules/QueryAtlas/ImageData
  foreach icon [glob $icondir/*.png] {
    set card [Card #auto $renderWidget]
    set ras [list [randRange -200 200] [randRange -200 200] [randRange -200 200]]
    set anchor [list [randRange -100 100] [randRange -100 100] [randRange -100 100]]
    set scale [randRange 1 30]
    $card configure -text "card [expr round([randRange 0 100])]" \
      -ras $ras -anchor $anchor -scale $scale -icon $icon -follow 0 
  }
}


namespace eval Card {
  proc ConfigureAll {args} {
    foreach c [itcl::find objects -class Card] {
      eval $c configure $args
    }
  }
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
    public variable icon ""  ;# the uri from which to grab the image (should be in png format)
    public variable text ""  ;# the text to display - can contain line breaks
    public variable follow 1  ;# make this a follower or not
    public variable font "default"  ;# what font to use (default means arial).
    # public variable fontDir "$::env(Slicer3_HOME)/share/Slicer3/Modules/QueryAtlas/Fonts/"  ;# where to find the fonts
    public variable fontDir "c:/windows/fonts"  ;# where to find the fonts
    public variable ras "0 0 0"  ;# what font to use (default means arial).
    public variable anchor "0 0 0"  ;# where to draw the anchor line to 
    public variable scale "1"  ;# what font to use (default means arial).
    public variable highlight "0"  ;# should the card be highlighted or not

    variable _vtkObjects ""
    variable _pickState "outside"
    variable _actionState ""

    variable o ;# array of the objects for this widget, for convenient access
    variable _observerTags ;# save so destructor can remove observer

    # parts of the renderWidget saved for easy access
    variable _renderer ""
    variable _camera ""
    variable _interactor ""
    variable _style ""
    variable _annotation ""

    # methods
    method getObjects {} {return [array get o]}
    method processEvent {{caller ""} {event ""}} {}
    method removeActors {} {}
    method addActors {} {}
    method updateActors {} {}
    method positionActors {} {}
    method hitTest {x y} {}
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
# called on 'widget configure -blah '
#
itcl::configbody Card::renderWidget {
  $this removeActors
  set _renderer [$renderWidget GetRenderer]
  #set _renderer [$renderWidget GetOverlayRenderer]
  #set _renderer [[[$renderWidget GetRenderWindow] GetRenderers] GetItemAsObject 1]
  set _camera [$_renderer GetActiveCamera]
  set _interactor [$renderWidget GetRenderWindowInteractor]
  set _style [$_interactor GetInteractorStyle] 
  set _annotation [$renderWidget GetCornerAnnotation]
  $this addActors
}

itcl::configbody Card::icon {
  
  if { ![info exists o(iconImage)] } {
    set o(iconImage) [vtkNew vtkImageData]
  }
  set reader [vtkPNGReader New]
  $reader SetOutput $o(iconImage)
  $reader SetFileName $icon
  $reader Update
  $reader Delete
  $this updateActors
}

# generic configbody for several different parameters
foreach param {text follow font ras scale highlight} {
  itcl::configbody Card::$param { $this updateActors }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body Card::constructor {renderWidget} {

  $this configure -renderWidget $renderWidget

  set _observerTags ""

  set tag [$renderWidget AddObserver DeleteEvent \
        "::Card::ProtectedDelete $this"]
  lappend _observerTags [list $renderWidget $tag]

    
  set events {  
    "EndInteractionEvent"
  }
  foreach event $events {
    set tag [$_style AddObserver $event \
        "::Card::ProtectedCallback $this processEvent $_style $event"]    
    lappend _observerTags [list $_interactor $tag]
  }

  set events {  
    "KeyPressEvent"
  }
  foreach event $events {
    set tag [$_interactor AddObserver $event \
        "::Card::ProtectedCallback $this processEvent $_interactor $event"]    
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

itcl::body Card::processEvent { {caller ""} {event ""} } {

  if { [info command $caller] == ""} {
      return
  }

  if { [info command $renderWidget] == "" } {
    # the renderWidget was deleted behind our back, so we need to 
    # self destruct
    ::Card::ProtectedDelete $this
    return
  }


  if { $caller ==$_interactor } {
    switch $event {
      "KeyPressEvent" {
        switch [$_interactor GetKeySym] {
          default {
            #set char [$_interactor GetKeySym]
            #$this configure -text $text$char
          }
        }
      }
    }
  } elseif { $caller == $_style } {
    switch $event {
      "EndInteractionEvent" {
        $this positionActors
      }
    }
  } elseif { $caller == $_camera } {
    $this positionActors
  }
}

itcl::body Card::removeActors {} {

  foreach actor {actor iconActor lineActor} {
    if { [info exists o($actor)] } {
      #$_renderer RemoveActor $o($actor)
      $renderWidget RemoveViewProp $o($actor)
    }
  }
}

itcl::body Card::addActors {} {
  
  #
  # set up the texture tex
  #
  set o(tText) [vtkNew vtkTextureText]
  set o(actor) [vtkNew vtkFollower]
  $o(actor) SetMapper [[$o(tText) GetFollower] GetMapper]
  $o(actor) SetTexture [$o(tText) GetTexture]
  #$_renderer AddActor $o(actor)
  $renderWidget AddViewProp $o(actor)
  
  #
  # set up the icon
  #
  set o(iconSource) [vtkNew vtkPlaneSource]
  set o(iconTexture) [vtkNew vtkTexture]
  set o(iconMapper) [vtkNew vtkPolyDataMapper]
  set o(iconActor) [vtkNew vtkFollower]
  $o(iconMapper) SetInput [$o(iconSource) GetOutput]
  $o(iconActor) SetMapper $o(iconMapper)
  $o(iconActor) SetTexture $o(iconTexture)
  #$_renderer AddActor $o(iconActor)
  $renderWidget AddViewProp $o(iconActor)

  #
  # set up the anchor line
  #
  set o(lineSource) [vtkNew vtkLineSource]
  set o(lineMapper) [vtkNew vtkPolyDataMapper]
  set o(lineActor) [vtkNew vtkActor]
  $o(lineMapper) SetInput [$o(lineSource) GetOutput]
  $o(lineActor) SetMapper $o(lineMapper)
  #$_renderer AddActor $o(lineActor)
  $renderWidget AddViewProp $o(lineActor)

  $this updateActors
}

itcl::body Card::updateActors {} {

  $this positionActors

  # update text actor

  set fontParams [$o(tText) GetFontParameters] 
  $fontParams SetFontDirectory $fontDir
  if { $font != "default" } {
    $fontParams SetFontFileName $font
  } else {
    $fontParams SetFontFileName "arial.ttf"
  }
  $fontParams SetBlur 2
  $fontParams SetStyle 2
  $o(tText) SetText $text
  $o(tText) CreateTextureText

  # update icon actor

  if { [info exists o(iconImage)] } {
    $o(iconTexture) SetInput $o(iconImage)
  }

}

itcl::body Card::positionActors {} {

  set viewM [vtkMatrix4x4 New]
  $viewM DeepCopy [[$_camera GetViewTransformObject] GetMatrix]
  foreach i {0 1 2} {
    $viewM SetElement $i 3  0
  }
  $viewM Invert

  # update text actor

  foreach {r a s} $ras {}

  set delta [$viewM MultiplyPoint [expr 0.6 * $scale] 0 0 0]
  foreach {dr da ds dw} $delta {}
  $o(actor) SetPosition [expr $r + $dr] [expr $a + $da] [expr $s + $ds] 
  $o(actor) SetScale $scale
  $o(actor) SetUserMatrix $viewM

  if { $follow } {
    $o(actor) SetCamera $_camera
  } else {
    $o(actor) SetCamera ""
  }
  
  # update icon actor

  $o(iconActor) SetPosition $r $a [expr $s + ($scale / 4.)]
  if { $highlight } {
    $o(iconActor) SetScale [expr 1.1 * $scale]
  } else {
    $o(iconActor) SetScale $scale
  }
  $o(iconActor) SetUserMatrix $viewM

  if { $follow } {
    $o(iconActor) SetCamera $_camera
  } else {
    $o(iconActor) SetCamera ""
  }

  # update line actor

  eval $o(lineSource) SetPoint1 $anchor
  eval $o(lineSource) SetPoint2 $ras

  $viewM Delete

  $this hitTest 0 0
}

itcl::body Card::hitTest {hitx hity} {

  set tkwindow [$renderWidget  GetWidgetName]
  set width [winfo width $tkwindow]
  set height [winfo height $tkwindow]

  set m [$_camera GetCompositePerspectiveTransformMatrix [expr (1. * $width) / $height] 0 1]
  set vport [eval $m MultiplyPoint $ras 1]
  set w [lindex $vport 3]
  set vx [expr [lindex $vport 0] / $w]
  set vy [expr [lindex $vport 1] / $w]


  set x [expr $width * (1. + $vx)/2.]
  set y [expr $height * (1. + $vy)/2.]

  # puts "$this $ras -> $vport --> $vx $vy --> $x $y"

  if { [expr abs( $hitx - $x )/2.] < $scale && [expr abs( $hity - $y )/2.] < $scale } {
    return 1
  }
  return 0
}
