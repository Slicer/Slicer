
proc TestFan {} {
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set icondir $::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/ImageData
  set icons [lrange [glob $icondir/*.png] 0 4]
  set cardFan [CardFan #auto [llength $icons]]
  foreach icon $icons card [$cardFan cards] {
    $card configure -icon $icon -text [file tail $icon]
  }
  $cardFan configure -spacing 15 -anchor [list 0 0 0] -radius 50
  $cardFan configureAll -scale 10 -follow 0
}


#########################################################
#
if {0} { ;# comment

  CardFan is a class for managing several Cards relative to a reference model

# TODO :  this may get refactored into an SWidget3D if it makes sense...

}
#
#########################################################


#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class CardFan] == "" } {

  itcl::class CardFan {

    constructor {count} {
    }

    destructor {
    }

    # configure options
    public variable anchor "0 0 0"  ;# the center point of the fan
    public variable radius "50"  ;# the radius for the fan
    public variable spacing "10"  ;# the spacing between the elements

    variable _cards ""

    method cards {} {return $_cards}
    method positionCards {} {}
    method configureAll {args} {
      foreach c $_cards {
        eval $c configure $args
      }
    }
  }
}

#
# called on 'widget configure -blah '
#

# generic configbody for several different parameters
foreach param {anchor radius spacing} {
  itcl::configbody CardFan::$param { $this positionCards }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body CardFan::constructor {count} {

  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]

  for {set c 0} {$c < $count} {incr c} {
    lappend _cards [Card ::#auto $renderWidget]
  }
}


itcl::body CardFan::destructor {} {

  foreach card $_cards {
    itcl::delete object $card
  }
}

itcl::body CardFan::positionCards {} {

  foreach card $_cards {
    $card configure -anchor $anchor
  }

  set halfHeight [expr $spacing * [llength $_cards] / 2.]

  set index 0
  foreach card $_cards {
    foreach {r a s} $anchor {}
    set r [expr $r + $radius]
    set s [expr $s + $halfHeight - ($index * $spacing)]
    $card configure -ras "$r $a $s"
    incr index
  }





}
