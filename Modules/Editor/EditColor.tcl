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

    # methods
    method create {} {}
    method processEvent {{caller ""} {event ""}} {}
    method updateGUI {label} {}
    method updateParameterNode {} {}
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
  [$o(colorSpin) GetWidget] SetValue [EditorGetPaintLabel]
  $o(colorSpin) SetBalloonHelpString "Click colored patch at right to bring up color selection pop up window.  Use the 'c' key to bring up color popup menu."

  set o(colorOption) [vtkNew vtkKWMenuButton]
  $o(colorOption) SetParent $frame
  $o(colorOption) Create

  set o(colorPatch) [vtkNew vtkKWCanvas]
  $o(colorPatch) SetParent $frame
  $o(colorPatch) Create
  $o(colorPatch) SetWidth 15
  $o(colorPatch) SetHeight 15
  $o(colorPatch) SetBackgroundColor 0 0 0
  $o(colorPatch) SetBorderWidth 2
  $o(colorPatch) SetReliefToSolid
  # TODO: can't get events from Canvas through kww, need to access tk directly
  bind [$o(colorPatch) GetWidgetName] <1> "::Box::ShowDialog ColorBox"

  # TODO: don't pack this until it's integrated better: [$o(colorOption) GetWidgetName]

  pack \
    [$o(colorSpin) GetWidgetName] \
    [$o(colorPatch) GetWidgetName] \
    -side left -anchor e -fill x -padx 2 -pady 2 

  # TODO: need to listen for AnyEvent because there's no we to specify specific events
  foreach object [list [$o(colorSpin) GetWidget] $o(colorOption)] {
    set tag [$object AddObserver AnyEvent "::Box::ProtectedCallback $this processEvent $object"]
    lappend _observerRecords [list $object $tag]
  }

  #
  # observe the scene to know when to get the parameter node
  #
  set scene $::slicer3::MRMLScene
  set tag [$scene AddObserver ModifiedEvent "$this updateParameterNode"]
  lappend _observerRecords [list $scene $tag]

  $this updateGUI [EditorGetPaintLabel]
}

#
# update the parameter node when the scene changes
#
itcl::body EditColor::updateParameterNode { } {
  #
  # observe the scene to know when to get the parameter node
  #
  set node [EditorGetParameterNode]
  set tag [$node AddObserver ModifiedEvent "::Box::ProtectedCallback $this processEvent $node"]
  lappend _observerRecords [list $node $tag]
}

#
# handle gui events
# -basically just map button events onto methods
# - not used due to KWWidgets limitations
#
itcl::body EditColor::processEvent { {caller ""} {event ""} } {

  set node [EditorGetParameterNode]
  if { $caller == $node } {
    $this updateGUI [EditorGetPaintLabel]
    return
  }

  if { $caller == [$o(colorSpin) GetWidget] } {
    EditorSetPaintLabel [[$o(colorSpin) GetWidget] GetValue]
  }

}

#
# update the GUI for the given label
#
itcl::body EditColor::updateGUI {label} {

  set label [expr int($label)]
  [$o(colorSpin) GetWidget] SetValue $label


  # TODO: 
  # $o(colorOption) udpate the selection

  set colorNode [$this getColorNode]
  if { $colorNode != "" } {
    set lut [$colorNode GetLookupTable]
    eval $o(colorPatch) SetBackgroundColor [lrange [$lut GetTableValue $label] 0 2]
  }
}

#
# get the color node for the label map in the Red slice
#
itcl::body EditColor::getColorNode {} {
  set sliceLogic [$::slicer3::ApplicationLogic GetSliceLogic "Red"]
  if { $sliceLogic != "" } {
    set logic [$sliceLogic GetLabelLayer]
    if { $logic != "" } {
      set volumeDisplayNode [$logic GetVolumeDisplayNode]
      if { $volumeDisplayNode != "" } {
        return [$volumeDisplayNode GetColorNode]
      }
    }
  }
  return ""
}
