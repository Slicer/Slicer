package require Itcl

#########################################################
#
if {0} { ;# comment

  Loader is a wrapper around a set of kwwidgets and 
  file manipulation logic to do intelligent loading
  of files, directories, and other objects.

# TODO : 
  - part of this should be split out into logic helper methods
  - this could be reimplemented in C++ pretty easily
  - it would be nice if vtkITKArchetype* could be used
    to tell us the filenames for a given archetype without
    actually reading.  That would allow the add method
    to be intelligent about dicom files and other series

}
#
#########################################################


# 
# utility to only delete an instance if it hasn't already been deleted
# (this is useful in event handling)
#
namespace eval Loader {
  proc ShowDialog { {path ""} } {
    set loaders [itcl::find objects -class Loader]
    if { $loaders != "" } {
      set loader [lindex $loaders 0]
      array set o [$loader objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set loader [Loader #auto]
    }
    $loader add $path
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class Loader] == "" } {

  itcl::class Loader {

    constructor  { {root ""} } {
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable root ""  ;# the root to scan from

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access
    variable col ;# array of the column indices for easy (and readable) access

    variable _volumeExtensions ".hdr .nhdr .mhd .vti .mgz"
    variable _modelExtensions ".vtk .vtp"

    # methods
    method clear {} {}
    method add {path} {}
    method addRow { path type } {}
    method processEvents {caller} {}
    method apply {} {}
    method errorDialog {errorText} {}

    method objects {} {return [array get o]}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this Loader
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
itcl::body Loader::constructor { {root ""} } {

  $this configure -root ""

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Add Data"
  $o(toplevel) Create

  # delete this instance when the window is closed
  wm protocol [$o(toplevel) GetWidgetName] \
    WM_DELETE_WINDOW "itcl::delete object $this"


  #
  # the listbox of data to load
  #

  set o(list) [vtkNew vtkKWMultiColumnListWithScrollbars]
  $o(list) SetParent $o(toplevel)
  $o(list) Create
  $o(list) SetHeight 4
  set w [$o(list) GetWidget]
  $w SetSelectionTypeToCell
  $w MovableRowsOff
  $w MovableColumnsOn 
  $w SetPotentialCellColorsChangedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
  $w SetColumnSortedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

  foreach column {Select File Type Name LabelMap Centered} {
    set col($column) [$w AddColumn $column]
  }
  $w SetColumnFormatCommandToEmptyOutput $col(Select)

  # configure the entries
  foreach column {Select LabelMap Centered} {
    $w SetColumnEditWindowToCheckButton $col($column)
  }
  foreach column {Select File Name LabelMap Centered} {
    $w ColumnEditableOn $col($column)
  }
  $w SetColumnWidth $col(Select) 6
  $w SetColumnWidth $col(File) 45
  $w SetColumnWidth $col(Type) 7

  pack [$o(list) GetWidgetName] -side top -anchor e -padx 2 -pady 2 -expand true -fill both


  #
  # the apply and cancel buttons
  #

  set o(buttonFrame) [vtkNew vtkKWFrame]
  $o(buttonFrame) SetParent $o(toplevel)
  $o(buttonFrame) Create
  pack [$o(buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set o(addDir) [vtkNew vtkKWPushButton]
  $o(addDir) SetParent $o(buttonFrame)
  $o(addDir) Create
  $o(addDir) SetText "Add Directory"
  $o(addDir) SetBalloonHelpString "Add all contents of a directory to the list of files to load"
  $o(addDir) AddObserver ModifiedEvent "$this processEvents $o(addDir)"
  $o(addDir) SetCommand $o(addDir) Modified

  set o(addFile) [vtkNew vtkKWPushButton]
  $o(addFile) SetParent $o(buttonFrame)
  $o(addFile) Create
  $o(addFile) SetText "Add File"
  $o(addFile) SetBalloonHelpString "Add a file to the list of files to load"
  $o(addFile) AddObserver ModifiedEvent "$this processEvents $o(addFile)"
  $o(addFile) SetCommand $o(addFile) Modified

  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent $o(buttonFrame)
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Load the listed files into slicer"
  $o(apply) AddObserver ModifiedEvent "$this processEvents $o(apply)"
  $o(apply) SetCommand $o(apply) Modified

  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent $o(buttonFrame)
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Close window without loading files"
  $o(cancel) AddObserver ModifiedEvent "$this processEvents $o(cancel)"
  $o(cancel) SetCommand $o(cancel) Modified

  pack \
    [$o(cancel) GetWidgetName] \
    [$o(apply) GetWidgetName] \
    [$o(addFile) GetWidgetName] \
    [$o(addDir) GetWidgetName] \
    -side right -anchor w -padx 4 -pady 2

  $o(toplevel) Display
}


itcl::body Loader::destructor {} {

  $o(toplevel) Delete

}


itcl::configbody Loader::root {
}

# remove entries from the list box
itcl::body Loader::clear { } {
}

itcl::body Loader::addRow { path type } {

  set w [$o(list) GetWidget] 
  set i [$w GetNumberOfRows]
  # make entry selected by default
  $w InsertCellTextAsInt $i $col(Select) 1
  $w SetCellWindowCommandToCheckButton $i $col(Select)
  $w InsertCellText $i $col(File) $path
  $w InsertCellText $i $col(Type) $type
  $w InsertCellText $i $col(Name) [file tail [file root $path]]

  if { $type == "Volume" } {
    $w InsertCellTextAsInt $i $col(Centered) 1
    $w SetCellWindowCommandToCheckButton $i $col(Centered)

    set seg [string match -nocase "*seg*" $path] 
    $w InsertCellTextAsInt $i $col(LabelMap) $seg
    $w SetCellWindowCommandToCheckButton $i $col(LabelMap)
  } 

}

# add a new archetype or directory to the list box
# - does a search of the path for volumes and models
itcl::body Loader::add { path } {

  set ext [string tolower [file extension $path]]

  #
  # if it's a zip archive, copy the contents to a temp
  # directory and load from there
  #
  if { $ext == ".zip" || $ext == ".xar" } {
    set ret [catch "package require vfs::zip" res]
    if { $ret } {
      $this errorDialog "Cannot find zipfile reader, cannot process $path"
    } else {
      set tmp [$::slicer3::Application GetTemporaryDirectory]/Loader-[pid]-[clock seconds]
      file mkdir $tmp
      ::vfs::zip::Mount $path /zipfile
      file copy -force /zipfile $tmp
      $this add $tmp
    }
  }

  # 
  # if it's a directory, look at each element (recurse)
  #
  if { [file isdir $path] } {
    foreach item [glob -nocomplain $path/*] {
      $this add $item
    }
  } else {
    #
    # if it's a file, see if it's something we know how to load
    #
    if { [lsearch $_volumeExtensions $ext] != -1 } {
      $this addRow $path "Volume"
    }
    if { [lsearch $_modelExtensions $ext] != -1 } {
      $this addRow $path "Model"
    }
  }
}

#
# load the items from the list box
# into slicer - ignore unselected rows
#
itcl::body Loader::apply { } {

  set w [$o(list) GetWidget] 
  set rows [$w GetNumberOfRows]

  for {set row 0} {$row < $rows} {incr row} {
    if { [$w GetCellTextAsInt $row $col(Select)] } {
      set path [$w GetCellText $row $col(File)]
      set name [$w GetCellText $row $col(Name)]
      switch [$w GetCellText $row $col(Type)] {
        "Volume" {
          set centered [$w GetCellTextAsInt $row $col(Centered)]
          set labelMap [$w GetCellTextAsInt $row $col(LabelMap)]
          [$::slicer3::VolumesGUI GetLogic] AddArchetypeVolume $path $centered $labelMap $name
        }
        "Model" {
          set node [[$::slicer3::ModelsGUI GetLogic] AddModel $path]
          $node SetName $name
        }
      }
    }
  }
}


#
# handle gui events
# -basically just map button events onto methods
#
itcl::body Loader::processEvents { caller } {

  if { $caller == $o(addDir) } {
    # TODO: switch to kwwidgets directory browser
    $this add [tk_chooseDirectory]
    return
  }

  if { $caller == $o(addFile) } {
    # TODO: switch to kwwidgets directory browser
    $this add [tk_getOpenFile]
    return
  }

  if { $caller == $o(apply) } {
    $this apply
    itcl::delete object $this
    return
  }

  if { $caller == $o(cancel) } {
    itcl::delete object $this
    return
  }
  
  puts "unknown event from $caller"
}

