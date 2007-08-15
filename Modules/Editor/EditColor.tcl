package require Itcl

#########################################################
#
if {0} { ;# comment

  EditColor is a wrapper around a set of kwwidgets and other
  structures to manage show a color picker for the editor

# TODO : 

}
#
#########################################################

#
# namespace procs
#

namespace eval EditColor {

  # 
  # simple test harness (doesn't clean up)
  #
  proc test {} {
    set t [vtkKWTopLevel New]
    $t SetApplication $::slicer3::Application
    $t SetTitle "Color Test"
    $t Create
    set c [EditColor #auto $t]
    $c configure -frame $t
    $c create
    puts [$c objects]
    $t Display
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class EditColor] == "" } {

  itcl::class EditColor {

    #TODO collect utilities into parent specific for widgets
    # - for now, use the Box parent
    #inherit SlicerWidget
    inherit Box

    public variable selectCommand ""
    public variable colorNode ""

    # methods
    method create {} {}
    method processEvents {caller} {}
    method getColorNode {} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# - rely on superclass
# ------------------------------------------------------------------


# create the edit box
itcl::body EditColor::create { } {

  if { $frame == "" } {
    error "need a parent frame"
  }

  set o(colorSpin) [vtkNew vtkKWSpinBoxWithLabel]
  $o(colorSpin) SetParent $frame
  $o(colorSpin) SetLabelText "Label"
  $o(colorSpin) Create
  [$o(colorSpin) GetWidget] SetWidth 3

  set o(colorOption) [vtkNew vtkKWMenuButton]
  $o(colorOption) SetParent $frame
  $o(colorOption) Create

  set o(colorPatch) [vtkNew vtkKWCanvas]
  $o(colorPatch) SetParent $frame
  $o(colorPatch) Create
  $o(colorPatch) SetWidth 15
  $o(colorPatch) SetHeight 15
  $o(colorPatch) SetBackgroundColor 1 0 .5
  $o(colorPatch) SetBorderWidth 2

  #set node [$this getColorNode]

  pack \
    [$o(colorSpin) GetWidgetName] \
    [$o(colorOption) GetWidgetName] \
    [$o(colorPatch) GetWidgetName] \
    -side left -anchor e -fill x -padx 2 -pady 2 

  # TODO: need to listen for AnyEvent because there's no we to specify specific events
  foreach object [list [$o(colorSpin) GetWidget] $o(colorOption)] {
    set tag [$object AddObserver AnyEvent "$this processEvents $object"]
    lappend _observerRecords [list $object $tag]
  }
}

#
# handle gui events
# -basically just map button events onto methods
# - not used due to KWWidgets limitations
#
itcl::body EditColor::processEvents { caller } {

  puts "got event from $caller"
  return

  if { $caller == $o(colorSpin) } {
  }
}

#
# get the color node for the label map in the Red slice
#
itcl::body EditColor::getColorNode {} {
  set logic [[$::slicer3::ApplicationGUI GetMainSliceLogic0] GetLabelLayer]
  set volumeDisplayNode [$logic GetVolumeDisplayNode]
  if { $volumeDisplayNode == "" } {
    return ""
  }
  return [$volumeDisplayNode GetColorNode]
}
