package require Itcl

#########################################################
#
if {0} { ;# comment

  FilteredDirectoryDialog is a wrapper around a set of kwwidgets 
  to add a Filter entry and recursive checkbox to a directory browser

# TODO : 

}
#
#########################################################


namespace eval FilteredDirectoryDialog {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog {} {

    set loaders [itcl::find objects -class FilteredDirectoryDialog]
    if { $loaders != "" } {
      set loader [lindex $loaders 0]
      array set o [$loader objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set loader [FilteredDirectoryDialog #auto]
    }
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class FilteredDirectoryDialog] == "" } {

  itcl::class FilteredDirectoryDialog {

    constructor  {} {
    }

    destructor {
      vtkDelete  
    }

    public variable ok_command ""
    public variable cancel_command ""

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access

    variable _observerRecords ""

    variable _recurse 1
    variable _filter "*"
    variable _paths ""


    # methods
    method processEvent {{caller ""} {event ""}} {}
    method ok {} {}

    method objects {} {return [array get o]}

    method getRecurse {} {return $_recurse}
    method getFilter {} {return $_filter}
    method getPaths {} {return $_paths}
    method getToplevel {} {return $o(toplevel)}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this FilteredDirectoryDialog
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
itcl::body FilteredDirectoryDialog::constructor {} {

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Select Directory"
  $o(toplevel) Create
  $o(toplevel) SetGeometry 600x700

  # delete this instance when the window is closed
  wm protocol [$o(toplevel) GetWidgetName] \
    WM_DELETE_WINDOW "itcl::delete object $this"

  #
  # top frame
  #
  set o(topFrame) [vtkNew vtkKWFrame]
  $o(topFrame) SetParent $o(toplevel)
  $o(topFrame) Create
  pack [$o(topFrame) GetWidgetName] -side top -anchor nw -fill both -expand true

  #
  # the file browser widget
  #
  set o(browser) [vtkNew vtkKWDirectoryExplorer]
  $o(browser) SetSelectionModeToMultiple
  $o(browser) SetParent $o(topFrame)
  $o(browser) Create

  $::slicer3::Application RequestRegistry "OpenPath"
  set path [$::slicer3::Application GetRegistryHolder]
  $o(browser) OpenDirectory $path

  #
  # the options frame
  #
  set o(options) [vtkNew vtkKWFrameWithLabel]
  $o(options) SetParent $o(topFrame)
  $o(options) SetLabelText "File Selection Options"
  $o(options) Create

  pack \
    [$o(browser) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand true -fill both
  pack \
    [$o(options) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand false -fill x

  #
  # the options contents
  #
  set o(recurse) [vtkNew vtkKWCheckButton]
  $o(recurse) SetParent [$o(options) GetFrame]
  $o(recurse) SetText "Recurse"
  $o(recurse) SetBalloonHelpString "Look in subdirectories (and their subdirectories)"
  $o(recurse) Create
  $o(recurse) SetSelectedState $_recurse

  set o(filter) [vtkNew vtkKWEntryWithLabel]
  $o(filter) SetParent [$o(options) GetFrame]
  $o(filter) SetLabelText "Filter: "
  $o(filter) SetBalloonHelpString "Use wildcards to specify which files to include"
  $o(filter) Create
  [$o(filter) GetWidget] SetValue $_filter

  pack \
    [$o(recurse) GetWidgetName] \
    [$o(filter) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand false -fill x


  #
  # the ok and cancel buttons
  #

  set o(buttonFrame) [vtkNew vtkKWFrame]
  $o(buttonFrame) SetParent $o(toplevel)
  $o(buttonFrame) Create
  pack [$o(buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set o(ok) [vtkNew vtkKWPushButton]
  $o(ok) SetParent $o(buttonFrame)
  $o(ok) Create
  $o(ok) SetText "Ok"
  $o(ok) SetBalloonHelpString "Uses the selected directories and options"
  set tag [$o(ok) AddObserver ModifiedEvent "$this processEvent $o(ok)"]
  lappend _observerRecords [list $o(ok) $tag]
  $o(ok) SetCommand $o(ok) Modified

  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent $o(buttonFrame)
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Close window and cancel current selections"
  set tag [$o(cancel) AddObserver ModifiedEvent "$this processEvent $o(cancel)"]
  lappend _observerRecords [list $o(cancel) $tag]
  $o(cancel) SetCommand $o(cancel) Modified

  pack \
    [$o(cancel) GetWidgetName] \
    [$o(ok) GetWidgetName] \
    -side right -anchor w -padx 4 -pady 2

  $o(toplevel) Display
}


itcl::body FilteredDirectoryDialog::destructor {} {

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this vtkDelete

}


#
# load the items from the list box
# into slicer 
#
itcl::body FilteredDirectoryDialog::ok { } {

  set _recurse [$o(recurse) GetSelectedState]
  set _filter [[$o(filter) GetWidget] GetValue]

  set _paths ""
  for {set i 0} {$i < [$o(browser) GetNumberOfSelectedDirectories]} {incr i} {
    lappend _paths [$o(browser) GetNthSelectedDirectory $i]
  }

  set dir [$o(browser) GetSelectedDirectory]
  if { $dir != "" } {
    $::slicer3::Application SetRegistry "OpenPath" $dir
  }

}


#
# handle gui events
# -basically just map button events onto methods
#
itcl::body FilteredDirectoryDialog::processEvent { {caller ""} {event ""} } {

  if { $caller == $o(ok) } {
    $this ok
    eval $ok_command
    wm withdraw [$o(toplevel) GetWidgetName]
    return
  }

  if { $caller == $o(cancel) } {
    eval $cancel_command
    wm withdraw [$o(toplevel) GetWidgetName]
    return
  }

  puts "unknown event from $caller"
}

