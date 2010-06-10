package require Itcl

#########################################################
#
if {0} { ;# comment

  ColorBox is a wrapper around a set of kwwidgets and other
  structures to manage show a color picker

# TODO : 

}
#
#########################################################

#
# namespace procs
#

namespace eval ColorBox {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog {} {
    ::Box::ShowDialog ColorBox
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class ColorBox] == "" } {

  itcl::class ColorBox {

    inherit Box

    public variable selectCommand ""
    public variable colorNode ""

    variable col ;# array of column indices (for easy access)

    # methods
    method create {} {}
    method update {} {}
    method processEvent {{caller ""} {event ""}} {}
    method show {} {}
    method getColorNode {} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# - rely on superclass
# ------------------------------------------------------------------


# create the edit box
itcl::body ColorBox::create { } {

  if { $frame != "" } {
    error "packing in existing frame not yet supported"
  }

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Color Box"
  $o(toplevel) Create

  # hide this instance when the window is closed or key pressed
  wm protocol [$o(toplevel) GetWidgetName] WM_DELETE_WINDOW "$this hide"
  bind [$o(toplevel) GetWidgetName] <KeyPress> "$this hide"


  $this update
  set colorNode [$this getColorNode]
  if { $colorNode == "" } {
    set o(colors) [vtkNew vtkKWPushButton]
    $o(colors) SetParent $o(toplevel)
    $o(colors) SetText "Cannot display colors.\nNo label layer is selected."
    $o(colors) Create

    set tag [$o(colors) AddObserver AnyEvent "::Box::ProtectedCallback $this processEvent $o(colors)"]
    lappend _observerRecords [list $o(colors) $tag]

  } else {

    # pay attention to the color node - fill a list box with entries
    set o(colors) [vtkNew vtkKWMultiColumnListWithScrollbars]
    $o(colors) SetParent $o(toplevel)
    $o(colors) Create
    set w [$o(colors) GetWidget]
    $w SetSelectionTypeToRow
    $w SetSelectionModeToSingle
    $w MovableRowsOff
    $w MovableColumnsOn
    $w SetPotentialCellColorsChangedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
    $w SetColumnSortedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

    foreach column {Number Color Name} text {Number Color Name} width {7 6 25} {
      set col($column) [$w AddColumn $column]
      $w ColumnEditableOff $col($column)
      $w SetColumnWidth $col($column) $width
    }

    # fill in colors to pick from
    set numberOfColors [$colorNode GetNumberOfColors]
    set lut [$colorNode GetLookupTable]
    for {set c 0} {$c < $numberOfColors} {incr c} {
      # add it to the listbox
      set name [$colorNode GetColorName $c]
      if { $name != "(none)" } {
        set row [$w GetNumberOfRows]
        $w InsertCellText $row $col(Number) "$c"
        eval $w SetCellBackgroundColor $row $col(Color) [lrange [$lut GetTableValue $c] 0 2]
        $w InsertCellText $row $col(Name) [$colorNode GetColorName $c]
      }
    }

    $this configure -selectCommand "EditorSetPaintLabel %d"

    set SelectionChangedEvent 10000
    $::slicer3::Broker AddObservation [$o(colors) GetWidget] $SelectionChangedEvent "::Box::ProtectedCallback $this processEvent $o(colors)"
  }
  pack [$o(colors) GetWidgetName] \
    -side top -anchor e -fill both -expand true -padx 2 -pady 2 

  $this setMode $mode

  $o(toplevel) Display
}

itcl::body ColorBox::show {} {
  set oldSelectCommand [$this cget -selectCommand]
  $this configure -selectCommand ""
  set w [$o(colors) GetWidget]
  $w ClearSelection
  $this configure -selectCommand $oldSelectCommand
  chain
}

#
# handle gui events
# -basically just map button events onto methods
# - not used due to KWWidgets limitations
#
itcl::body ColorBox::processEvent { {caller ""} {event ""} } {

  set colorIndex 1
  if { $caller == $o(colors) } {
    if { [$o(colors) GetClassName] != "vtkKWPushButton" } {
      set row [[$o(colors) GetWidget] GetIndexOfFirstSelectedRow]
      if { $row == -1 } {
        # no valid row is selected, so ignore event
        return
      }
      set colorIndex [[$o(colors) GetWidget] GetCellText $row $col(Number)]
    }
    if { $selectCommand != "" } {
      set cmd [format $selectCommand $colorIndex]
      eval $cmd
    } 
    $this hide
  }
}

#
# get the color node for the label map in the Red slice
#
itcl::body ColorBox::getColorNode {} {
  #if { $colorNode != "" } {
  #  return $colorNode
  #}
  set logic [[$::slicer3::ApplicationLogic GetSliceLogic "Red"] GetLabelLayer]
  set volumeDisplayNode [$logic GetVolumeDisplayNode]
  if { $volumeDisplayNode == "" } {
    return ""
  }
  return [$volumeDisplayNode GetColorNode]
}
