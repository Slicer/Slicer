package require Itcl

#########################################################
#
if {0} { ;# comment

  LoadVolume is a wrapper around a set of kwwidgets and 
  file manipulation logic load a single volume file

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


namespace eval LoadVolume {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog {} {

    set loaders [itcl::find objects -class LoadVolume]
    if { $loaders != "" } {
      set loader [lindex $loaders 0]
      array set o [$loader objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set loader [LoadVolume #auto]
    }
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class LoadVolume] == "" } {

  itcl::class LoadVolume {

    constructor  {} {
    }

    destructor {
      vtkDelete  
    }

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access

    variable _volumeExtensions ".hdr .nhdr .nrrd .mhd .mha .vti .mgz"
    variable _observerRecords ""
    variable _processingEvents 0

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method apply {} {}
    method status { message } {}
    method errorDialog {errorText} {}

    method objects {} {return [array get o]}

    # make a new instance of a class and add it to the list for cleanup
    method vtkNew {class} {
      set object [$class New]
      set _vtkObjects "$object $_vtkObjects"
      return $object
    }

    # clean up the vtk classes instanced by this LoadVolume
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
itcl::body LoadVolume::constructor {} {

  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Add Volume"
  $o(toplevel) Create
  $o(toplevel) SetGeometry 800x600

  # delete this instance when the window is closed
  wm protocol [$o(toplevel) GetWidgetName] \
    WM_DELETE_WINDOW "itcl::delete object $this"

  #
  # top frame
  #
  set o(topFrame) [vtkNew vtkKWFrame]
  $o(topFrame) SetParent $o(toplevel)
  $o(topFrame) Create
  pack [$o(topFrame) GetWidgetName] -side top -anchor nw -fill x

  #
  # the file browser widget
  #
  set o(browser) [vtkNew vtkKWFileBrowserWidget]
  $o(browser) SetParent $o(topFrame)
  $o(browser) Create

  $::slicer3::Application RequestRegistry "OpenPath"
  set path [$::slicer3::Application GetRegistryHolder]
  $o(browser) OpenDirectory $path

  set fileTable [$o(browser) GetFileListTable]
  $fileTable SetSelectionModeToSingle
  set tag [$fileTable AddObserver AnyEvent "$this processEvent $o(browser)"]
  lappend _observerRecords [list $fileTable $tag]

  set directoryExplorer [$o(browser) GetDirectoryExplorer]
  set tag [$directoryExplorer AddObserver AnyEvent "$this processEvent $o(browser)"]
  lappend _observerRecords [list $fileTable $tag]

  #
  # the current Path
  #
  set o(path) [vtkNew vtkKWEntryWithLabel]
  $o(path) SetParent $o(topFrame)
  $o(path) SetLabelText "Path: "
  $o(path) Create
  set tag [[$o(path) GetWidget] AddObserver AnyEvent "$this processEvent $o(path)"]
  lappend _observerRecords [list $fileTable $tag]


  #
  # the options frame
  #
  set o(options) [vtkNew vtkKWFrameWithLabel]
  $o(options) SetParent $o(topFrame)
  $o(options) SetLabelText "Volume Options"
  $o(options) Create

  pack \
    [$o(browser) GetWidgetName] \
    [$o(path) GetWidgetName] \
    [$o(options) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand true -fill both

  #
  # the options contents
  #
  set o(centered) [vtkNew vtkKWCheckButton]
  $o(centered) SetParent [$o(options) GetFrame]
  $o(centered) SetText "Centered"
  $o(centered) Create

  set o(label) [vtkNew vtkKWCheckButton]
  $o(label) SetParent [$o(options) GetFrame]
  $o(label) SetText "Label Map"
  $o(label) Create

  set o(name) [vtkNew vtkKWEntryWithLabel]
  $o(name) SetParent [$o(options) GetFrame]
  $o(name) SetLabelText "Name: "
  $o(name) Create

  pack \
    [$o(centered) GetWidgetName] \
    [$o(label) GetWidgetName] \
    [$o(name) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand true -fill both



  #
  # a status label
  #
  set o(status) [vtkNew vtkKWLabel]
  $o(status) SetParent $o(toplevel)
  $o(status) Create
  $o(status) SetText ""
  $o(status) SetBalloonHelpString "Current status of the load process"
  pack [$o(status) GetWidgetName] -side top -anchor e -padx 2 -pady 2 -expand false -fill x


  #
  # the apply and cancel buttons
  #

  set o(buttonFrame) [vtkNew vtkKWFrame]
  $o(buttonFrame) SetParent $o(toplevel)
  $o(buttonFrame) Create
  pack [$o(buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent $o(buttonFrame)
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Load the listed files into slicer"
  set tag [$o(apply) AddObserver ModifiedEvent "$this processEvent $o(apply)"]
  lappend _observerRecords [list $o(apply) $tag]
  $o(apply) SetCommand $o(apply) Modified

  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent $o(buttonFrame)
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Close window without loading files"
  set tag [$o(cancel) AddObserver ModifiedEvent "$this processEvent $o(cancel)"]
  lappend _observerRecords [list $o(cancel) $tag]
  $o(cancel) SetCommand $o(cancel) Modified

  pack \
    [$o(cancel) GetWidgetName] \
    [$o(apply) GetWidgetName] \
    -side right -anchor w -padx 4 -pady 2

  $o(toplevel) Display
}


itcl::body LoadVolume::destructor {} {

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
itcl::body LoadVolume::apply { } {


  set centered [$o(centered) GetSelectedState]
  set labelMap [$o(label) GetSelectedState]
  set fileTable [$o(browser) GetFileListTable]
  set fileName [$fileTable GetNthSelectedFileName 0]
  if { $fileName == "" } {
    $this errorDialog "No file selected"
  }
  set name [[$o(name) GetWidget] GetValue]
  if { $name == "" } {
    set name [file root [file tail $fileName]]
  }

  set volumeLogic [$::slicer3::VolumesGUI GetLogic]
  set ret [catch [list $volumeLogic AddArchetypeVolume "$fileName" $centered $labelMap $name] node]
  if { $ret } {
    $this errorDialog "Could not load $fileName as a volume\n\nError is:\n$node"
    return
  }
  set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
  if { $node == "" } {
    $this errorDialog "Could not load $fileName as a volume"
  } else {
    if { $labelMap } {
      $selNode SetReferenceActiveLabelVolumeID [$node GetID]
    } else {
      $selNode SetReferenceActiveVolumeID [$node GetID]
    }
    $::slicer3::ApplicationLogic PropagateVolumeSelection

    $::slicer3::Application SetRegistry "OpenPath" [file dirname $fileName]
  }
}


#
# handle gui events
# -basically just map button events onto methods
#
itcl::body LoadVolume::processEvent { {caller ""} {event ""} } {

  if { $_processingEvents } {
    return
  }

  if { $caller == $o(apply) } {
    $this apply
    after idle "itcl::delete object $this"
    return
  }

  if { $caller == $o(cancel) } {
    after idle "itcl::delete object $this"
    return
  }

  if { $caller == $o(browser) } {
    set _processingEvents 1
    set fileTable [$o(browser) GetFileListTable]
    set fileName [$fileTable GetNthSelectedFileName 0]
    if { $fileName == "" } {
      set directoryExplorer [$o(browser) GetDirectoryExplorer]
      set directory [$directoryExplorer GetSelectedDirectory]
      [$o(path) GetWidget] SetValue $directory
      $this status "Directory: $directory"
      [$o(name) GetWidget] SetValue ""
    } else {
      [$o(path) GetWidget] SetValue $fileName
      $this status "File Name: $fileName"
      set volName [file root [file tail $fileName]]
      [$o(name) GetWidget] SetValue $volName
    }
    set _processingEvents 0
    return
  }

  if { $caller == $o(path) } {
    set _processingEvents 1
    set path [[$o(path) GetWidget] GetValue]
    set fileTable [$o(browser) GetFileListTable]
    set directoryExplorer [$o(browser) GetDirectoryExplorer]

    if { [file isdirectory $path] } {
      set dir $path
    } else {
      set dir [file dirname $path]
    }

    $directoryExplorer SelectDirectory $dir
    $fileTable ClearSelection
    $fileTable SelectFileName $path
    set _processingEvents 0
    return
  }
  
  puts "unknown event from $caller"
}

itcl::body LoadVolume::errorDialog { errorText } {
  tk_messageBox -message $errorText
}

itcl::body LoadVolume::status { message } {
  $o(status) SetText $message
}

