package require Itcl

#########################################################
#
if {0} { ;# comment

  LoadVolume is a wrapper around a set of kwwidgets and 
  file manipulation logic load a single volume file

# TODO : 
  - part of this should be split out into logic helper methods
  - this could be reimplemented in C++ pretty easily (ha ha!)
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
      $o(toplevel) Display
    } else {
      set loader [LoadVolume #auto]
    }
  }
}

namespace eval LoadVolume set FindSubfiles_Abort 0
namespace eval LoadVolume set FindSubfiles_Depth 0
namespace eval LoadVolume {
  # 
  # utility to find all files in all subdirectories of a given directory
  # - optional command for progress and interrupt (accept 0 or "" as okay to continue)
  # - uses global variable to unwind after abort
  # - operates recusively
  #
  proc FindSubfiles { dir {progressCmd ""} } {
    set ::LoadVolume::FindSubfiles_Abort 0
    incr ::LoadVolume::FindSubfiles_Depth
    if { $::LoadVolume::FindSubfiles_Depth > 100 } {
      puts stderr "FindSubfiles: too many nested levels - possible link loop?"
      return ""
    }
    set files ""
    set entries [glob -nocomplain -directory $dir *]
    foreach e $entries {
      if { $progressCmd != "" } {
        # escape any brackets in entry
        regsub -all {\[} $e {\[} ee
        set ret [eval $progressCmd \"$ee\"]
        update ;# TODO: this update should be in the progressCmd itself
        if { $ret != "" && $ret != 0} {
          set ::LoadVolume::FindSubfiles_Abort 1
          return $files
        }
      }
      if { [file isdirectory $e] } {
        set files [concat $files [::LoadVolume::FindSubfiles $e $progressCmd]]
        if { $::LoadVolume::FindSubfiles_Abort } {
          return $files
        }
      } else {
        lappend files $e
      }
    }
    return $files
  }
}


namespace eval LoadVolume {
  # 
  # math utilities for processing dicom volumes
  #
  proc Cross {x0 x1 x2  y0 y1 y2} {
    set Zx [expr $x1 * $y2 - $x2 * $y1]
    set Zy [expr $x2 * $y0 - $x0 * $y2]
    set Zz [expr $x0 * $y1 - $x1 * $y0];
    return "$Zx $Zy $Zz"
  }

  proc Difference {x0 x1 x2  y0 y1 y2} {
    return [list [expr $x0 - $y0] [expr $x1 - $y1] [expr $x2 - $y2]]
  }

  proc Dot {x0 x1 x2  y0 y1 y2} {
    return [expr $x0 * $y0 + $x1 * $y1 + $x2 * $y2]
  }
}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class LoadVolume] == "" } {

  itcl::class LoadVolume {

    constructor  {} {}
    destructor {}

    # visibility of extra column of hex in the dicom table
    public variable showGroupElement 0
    # tolerance in mm for deciding if slice spacing is irregular
    public variable epsilon 0.01

    variable _vtkObjects "" ;# internal list of object to delete

    variable o ;# array of the objects for this widget, for convenient access

    variable _volumeExtensions ".hdr .nhdr .nrrd .mhd .mha .vti .mgz"
    variable _observerRecords ""
    variable _processingEvents 0
    variable _DICOM ;# map from group/element to name
    variable _dicomColumn ;# keep track of columns
    variable _dicomTree ;# currently loaded dicom directory
    variable _dicomSeriesFileList "" ;# files in the current series list
    variable _dicomWindowLevel ;# keep track of the current series window and level

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method selectArchetype {path name {optionsName ""}} {}
    method apply {} {}
    method status { message } {}
    method errorDialog {errorText} {}
    method yesNoDialog {yesNoText} {}
    method progressDialog { {message "Working..."} } {}
    method loadDICOMDictionary {} {}
    method parseDICOMHeader {fileName arrayName} {}
    method populateDICOMTable {fileName} {}
    method parseDICOMDirectory {directoryName arrayName {includeSubseries 0} } {}
    method organizeDICOMSeries {arrayName {includeSubseries 0} {progressCmd ""} } {}
    method populateDICOMTree {directoryName arrayName} {}
    method safeNodeName {name} {}
    method saveGeometry {} {}
    method loadGeometry {} {}

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

  if { [info command vtkSlicerVolumesGUI] == "" } {
    $this errorDialog "The Volumes module is not available in the current process."
    after idle itcl::delete object $this
    return
  }


  #
  # make the toplevel 
  #
  set o(toplevel) [vtkNew vtkKWTopLevel]
  $o(toplevel) SetApplication $::slicer3::Application
  $o(toplevel) SetTitle "Add Volume"
  set parent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $o(toplevel) SetMasterWindow $parent
  $o(toplevel) ModalOn
  $o(toplevel) Create

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
  # table frame
  # - for browser and dicom info
  #
  set o(tableFrame) [vtkNew vtkKWSplitFrame]
  $o(tableFrame) SetParent $o(topFrame)
  $o(tableFrame) Create
  pack [$o(tableFrame) GetWidgetName] -side top -anchor nw -fill x

  #
  # the file browser widget
  #
  set o(browser) [vtkNew vtkKWFileBrowserWidget]
  $o(browser) SetParent [$o(tableFrame) GetFrame1]
  $o(browser) Create


  set fileTable [$o(browser) GetFileListTable]
  $fileTable SetSelectionModeToSingle
  set tag [$fileTable AddObserver DeleteEvent "itcl::delete $this"]
  lappend _observerRecords [list $fileTable $tag]
  set tag [$fileTable AddObserver AnyEvent "$this processEvent $o(browser)"]
  lappend _observerRecords [list $fileTable $tag]

  set directoryExplorer [$o(browser) GetDirectoryExplorer]
  set tag [$directoryExplorer AddObserver DeleteEvent "itcl::delete $this"]
  lappend _observerRecords [list $directoryExplorer $tag]
  set tag [$directoryExplorer AddObserver AnyEvent "$this processEvent $o(browser)"]
  lappend _observerRecords [list $directoryExplorer $tag]

  #
  # the current Path
  #
  set o(path) [vtkNew vtkKWEntryWithLabel]
  $o(path) SetParent $o(topFrame)
  $o(path) SetLabelText "Path: "
  $o(path) Create
  set tag [[$o(path) GetWidget] AddObserver DeleteEvent "itcl::delete $this"]
  lappend _observerRecords [list [$o(path) GetWidget] $tag]
  set tag [[$o(path) GetWidget] AddObserver AnyEvent "$this processEvent $o(path)"]
  lappend _observerRecords [list [$o(path) GetWidget] $tag]


  #
  # the options frame
  #
  set o(options) [vtkNew vtkKWFrameWithLabel]
  $o(options) SetParent $o(topFrame)
  $o(options) SetLabelText "Volume Options"
  $o(options) Create

  #
  # the dicom frame
  #
  set o(dicom) [vtkNew vtkKWFrameWithLabel]
  $o(dicom) SetParent [$o(tableFrame) GetFrame2]
  $o(dicom) SetLabelText "DICOM Information"
  $o(dicom) Create

  set o(dicomSplit) [vtkNew vtkKWSplitFrame]
  $o(dicomSplit) SetParent [$o(dicom) GetFrame]
  $o(dicomSplit) SetOrientationToVertical
  $o(dicomSplit) Create

  pack \
    [$o(browser) GetWidgetName] \
    [$o(dicom) GetWidgetName] -side left -anchor w -padx 2 -pady 2 -expand true -fill both

  pack \
    [$o(dicomSplit) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand true -fill both

  pack \
    [$o(tableFrame) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand true -fill both

  pack \
    [$o(path) GetWidgetName] \
    [$o(options) GetWidgetName] \
    -side top -anchor e -padx 2 -pady 2 -expand false -fill x

  $o(browser) SetWidth 800

  #
  # the options contents
  #
  set o(centered) [vtkNew vtkKWCheckButton]
  $o(centered) SetParent [$o(options) GetFrame]
  $o(centered) SetText "Centered"
  $o(centered) Create

  set o(orient) [vtkNew vtkKWCheckButton]
  $o(orient) SetParent [$o(options) GetFrame]
  $o(orient) SetText "Ignore File Orientation"
  $o(orient) Create

  set o(label) [vtkNew vtkKWCheckButton]
  $o(label) SetParent [$o(options) GetFrame]
  $o(label) SetText "Label Map"
  $o(label) Create

  set o(singleFile) [vtkNew vtkKWCheckButton]
  $o(singleFile) SetParent [$o(options) GetFrame]
  $o(singleFile) SetText "Single File"
  $o(singleFile) Create

  set o(name) [vtkNew vtkKWEntryWithLabel]
  $o(name) SetParent [$o(options) GetFrame]
  $o(name) SetLabelText "Name: "
  $o(name) Create

  pack [$o(centered) GetWidgetName] \
    -side left -anchor e -padx 2 -pady 2 -expand true -fill both

  pack [$o(orient) GetWidgetName] \
    -side left -anchor e -padx 2 -pady 2 -expand true -fill both

  pack [$o(label) GetWidgetName] \
    -side left -anchor e -padx 2 -pady 2 -expand true -fill both
  pack [$o(singleFile) GetWidgetName] \
    -side left -anchor e -padx 2 -pady 2 -expand true -fill both

  pack [$o(name) GetWidgetName] \
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
  # the recent files option menu and
  # the apply and cancel buttons
  #

  set o(buttonFrame) [vtkNew vtkKWFrame]
  $o(buttonFrame) SetParent $o(toplevel)
  $o(buttonFrame) Create
  pack [$o(buttonFrame) GetWidgetName] -side top -anchor nw -fill x

  set o(recentMenu) [vtkNew vtkKWMenuButtonWithLabel]
  $o(recentMenu) SetParent $o(buttonFrame)
  $o(recentMenu) Create
  $o(recentMenu) SetLabelText "Recent Volumes: "
  $o(recentMenu) SetBalloonHelpString "Quick access to the most recently loaded files from this machine"

  set menuButton [$o(recentMenu) GetWidget]
  set menu [$menuButton GetMenu]
  $::slicer3::Application RequestRegistry "RecentFiles"
  set recentFiles [$::slicer3::Application GetRegistryHolder]
  $menu AddRadioButton "-"
  foreach fileRecord $recentFiles {
    foreach {name file options} $fileRecord {
      $menu AddRadioButton "$name $file"
    }
  }
  $menu AddSeparator
  $menu AddRadioButton "Reset List"
  $menu SelectItem 0
  set tag [$menuButton AddObserver ModifiedEvent "$this processEvent $menuButton"]
  lappend _observerRecords [list $menuButton $tag]

  pack [$o(recentMenu) GetWidgetName] -side left -anchor w -padx 4 -pady 2

  set o(cwd) [vtkNew vtkKWPushButton]
  $o(cwd) SetParent $o(buttonFrame)
  $o(cwd) Create
  $o(cwd) SetText "Browse to CWD"
  $o(cwd) SetBalloonHelpString "Move the directory explorer to the current working directory (where slicer was started)."
  set tag [$o(cwd) AddObserver ModifiedEvent "$this processEvent $o(cwd)"]
  lappend _observerRecords [list $o(cwd) $tag]
  $o(cwd) SetCommand $o(cwd) Modified

  pack [$o(cwd) GetWidgetName] -side left -anchor w -padx 4 -pady 2

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


  #
  # parse dicom button - initially disabled, but enabled when
  # dicom file is selected
  #

  set o(dicomParseFrame) [vtkNew vtkKWFrame]
  $o(dicomParseFrame) SetParent [$o(dicomSplit) GetFrame2]
  $o(dicomParseFrame) Create
  pack [$o(dicomParseFrame) GetWidgetName] -side top -anchor nw -fill x -expand false

  set o(dicomParse) [vtkNew vtkKWPushButton]
  $o(dicomParse) SetParent $o(dicomParseFrame)
  $o(dicomParse) Create
  $o(dicomParse) SetText "Parse Directory"
  $o(dicomParse) SetBalloonHelpString "Parse the current directory to select series to load as volume"
  set tag [$o(dicomParse) AddObserver ModifiedEvent "$this processEvent $o(dicomParse)"]
  lappend _observerRecords [list $o(dicomParse) $tag]
  $o(dicomParse) SetCommand $o(dicomParse) Modified

  #
  # subdivide series based on common dicom tags that indicate volume differences
  # optional - initially disabled, but enabled when
  # dicom file is selected
  #
  set o(dicomParseSubseries) [vtkNew vtkKWCheckButton]
  $o(dicomParseSubseries) SetParent $o(dicomParseFrame)
  $o(dicomParseSubseries) SetText "Divide Subseries"
  $o(dicomParseSubseries) Create
  $o(dicomParseSubseries) SetBalloonHelpString "Based on dicom header tags, create optional 'virtual' series that may map better to volumes (makes parsing slower)"
  $o(dicomParseSubseries) SetSelectedState 0


  pack [$o(dicomParse) GetWidgetName] -side left -anchor nw -fill x -expand true
  pack [$o(dicomParseSubseries) GetWidgetName] -side left -anchor nw -fill x -expand true


  #
  # the listbox of dicom info
  #

  set o(dicomList) [vtkNew vtkKWMultiColumnListWithScrollbars]
  $o(dicomList) SetParent [$o(dicomSplit) GetFrame1]
  $o(dicomList) Create
  $o(dicomList) SetHeight 4
  set w [$o(dicomList) GetWidget]
  $w SetSelectionTypeToCell
  $w MovableRowsOff
  $w MovableColumnsOn 
  $w SetPotentialCellColorsChangedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"
  $w SetColumnSortedCommand $w "ScheduleRefreshColorsOfAllCellsWithWindowCommand"

  if { $showGroupElement } {
    set columns {Group/Element Description Value}
    set widths {12 30 30}
  } else {
    set columns {Description Value}
    set widths {15 15}
  }
  foreach column $columns {
    set _dicomColumn($column) [$w AddColumn $column]
  }
  # configure the entries
  foreach column $columns width $widths {
    $w SetColumnEditWindowToCheckButton $_dicomColumn($column)
    $w ColumnEditableOn $_dicomColumn($column)
    $w SetColumnWidth $_dicomColumn($column) $width
  }


  #
  # SeriesSelection Tree
  #
  set o(dicomTree) [vtkNew vtkKWTreeWithScrollbars]
  $o(dicomTree) SetParent [$o(dicomSplit) GetFrame2]
  [$o(dicomTree) GetWidget] SetSelectionModeToSingle
  $o(dicomTree) Create

  set t [$o(dicomTree) GetWidget]
  set tag [$t AddObserver DeleteEvent "itcl::delete $this"]
  lappend _observerRecords [list $t $tag]
  set tag [$t AddObserver AnyEvent "$this processEvent $t"]
  lappend _observerRecords [list $t $tag]


  pack [$o(dicomTree) GetWidgetName] -side top -anchor e -padx 2 -pady 2 -expand true -fill both
  pack [$o(dicomList) GetWidgetName] -side bottom -anchor e -padx 2 -pady 2 -expand true -fill both

  #
  # pop up the dialog and set size
  # - unfortunately, need to use 'update' to force some of the 
  #   geometry calculations before state can be fully restored.
  #   This is too bad since it causes some flashes, but having the state 
  #   restored correctly is worth it.
  #
  $o(toplevel) Display
  $this loadGeometry
  update
  after idle $this loadGeometry

  #
  # if there was a previously open directory, restore it
  #
  $::slicer3::Application RequestRegistry "OpenPath"
  set path [$::slicer3::Application GetRegistryHolder]
  if { [file exists $path] } {
    after idle $o(browser) OpenDirectory [list $path]
  }
}


itcl::body LoadVolume::destructor {} {

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this saveGeometry 

  $this vtkDelete

}

#
# save and restore the size, location, and splitframe 
# layout.  Use the registry.
#
itcl::body LoadVolume::saveGeometry {} {
  if { [info exists o(toplevel)] } {
    set geo(toplevel) [$o(toplevel) GetGeometry]
    set geo(tableFramePosition) [$o(tableFrame) GetSeparatorPosition]
    set geo(dicomSplitPosition) [$o(dicomSplit) GetSeparatorPosition]
    set geo(dicomDescriptionWidth) [[$o(dicomList) GetWidget] GetColumnWidth 0]
    set geo(dicomValueWidth) [[$o(dicomList) GetWidget] GetColumnWidth 1]
    set geo(browserFavoriteWidth) [[$o(browser) GetMainFrame] GetFrame1Size]
    set geo(browserDirWidth) [[$o(browser) GetDirFileFrame] GetFrame1Size]
    $::slicer3::Application SetRegistry "LoadVolumeGeometry" [array get geo]
  }
}

itcl::body LoadVolume::loadGeometry {} {
  $::slicer3::Application RequestRegistry "LoadVolumeGeometry"
  array set geo [$::slicer3::Application GetRegistryHolder]

  # restore dialog size and position, but keep it on the screen and make 
  # sure the borders are reachable to move and resize (use 150 pixels as a guess)
  if { ![info exists geo(toplevel)] } {
    set geo(toplevel) 900x800+200+200
  }
  set geoPad 150
  scan $geo(toplevel) "%dx%d+%d+%d" w h x y
  set screenwidth [winfo screenwidth [$o(toplevel) GetWidgetName]]
  set screenheight [winfo screenheight [$o(toplevel) GetWidgetName]]
  if { [expr $x + $geoPad] > $screenwidth } {
    set x 0
  }
  if { [expr $y + $geoPad] > $screenheight } {
    set y 0
  }
  if { [expr $x + $w] > $screenwidth } {
    set x 0
    set w [expr $screenwidth - $geoPad]
    if { $w > 900 } {
      set w 900
    }
  }
  if { [expr $y + $h] > $screenheight } {
    set y 0
    set h [expr $screenheight - $geoPad]
    if { $h > 800 } {
      set h 800
    }
  }
  set geo(toplevel) ${w}x${h}+${x}+${y}

  if { ![info exists geo(tableFramePosition)] } {
    set geo(tableFramePosition) 0.7
  }
  if { ![info exists geo(dicomSplitPosition)] } {
    set geo(dicomSplitPosition) 0.5
  }
  $o(toplevel) SetGeometry $geo(toplevel) 
  $o(tableFrame) SetSeparatorPosition $geo(tableFramePosition) 
  $o(dicomSplit) SetSeparatorPosition $geo(dicomSplitPosition) 
  if { [info exists geo(dicomDescriptionWidth)] } {
    [$o(dicomList) GetWidget] SetColumnWidth 0 $geo(dicomDescriptionWidth) 
  }
  if { [info exists geo(dicomValueWidth)] } {
    [$o(dicomList) GetWidget] SetColumnWidth 1 $geo(dicomValueWidth) 
  }
  if { [info exists geo(browserFavoriteWidth)] } {
    [$o(browser) GetMainFrame] SetFrame1Size $geo(browserFavoriteWidth) 
  }
  if { [info exists geo(browserDirWidth)] } {
    [$o(browser) GetDirFileFrame] SetFrame1Size $geo(browserDirWidth) 
  }
}

#
# load the items from the list box
# into slicer return 0 on success, non-zero otherwise
#
itcl::body LoadVolume::apply { } {

  set progressDialog [$this progressDialog]
  $progressDialog SetTitle "Working..."
  $progressDialog SetMessageText "Loading volume..."
  update

  set centered [$o(centered) GetSelectedState]
  set orient [$o(orient) GetSelectedState]
  set label [$o(label) GetSelectedState]
  set singleFile [$o(singleFile) GetSelectedState]
  foreach opt {centered orient label singleFile} {
    set options($opt) [set $opt]
  }

  if { [llength $_dicomSeriesFileList] == 1 } {
    set singleFile 1
  }

  set loadingOptions [expr $label * 1 + $centered * 2 + $singleFile * 4 + $orient * 16]

  set fileTable [$o(browser) GetFileListTable]
  set fileName [$fileTable GetNthSelectedFileName 0]
  if { $fileName == "" } {
    $this errorDialog "No file selected"
    $progressDialog SetParent ""
    $progressDialog SetMasterWindow ""
    $progressDialog Delete
    return 1
  }
  set name [[$o(name) GetWidget] GetValue]
  if { $name == "" } {
    set name [file root [file tail $fileName]]
  }

  set fileList [vtkStringArray New]
  foreach s $_dicomSeriesFileList {
    $fileList InsertNextValue $s
  }

  set volumeLogic [$::slicer3::VolumesGUI GetLogic]
  set ret [catch [list $volumeLogic AddArchetypeVolume "$fileName" $name $loadingOptions $fileList] node]
  if { $ret } {
    $this errorDialog "Could not load $fileName as a volume\n\nError is:\n$node"
    $progressDialog SetParent ""
    $progressDialog SetMasterWindow ""
    $progressDialog Delete
    return 1
  }
  $fileList Delete

  set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
  if { $node == "" } {
    $this errorDialog "Could not load $fileName as a volume"
    $progressDialog SetParent ""
    $progressDialog SetMasterWindow ""
    $progressDialog Delete
    return 1
  } else {
    if { $label } {
      $selNode SetReferenceActiveLabelVolumeID [$node GetID]
    } else {
      $selNode SetReferenceActiveVolumeID [$node GetID]
    }
    # is there a win/level setting?
    if {$_dicomWindowLevel(window) != -1 && $_dicomWindowLevel(level) != -1} {
      if {[$node IsA "vtkMRMLScalarVolumeNode"] == 1} {
        set dispNode [$node GetScalarVolumeDisplayNode]
        if {$dispNode != ""} {
            # get all of the presets out
            foreach dicomwin [split $_dicomWindowLevel(window) {\\}] lev [split $_dicomWindowLevel(level) {\\}] {
                # check to make sure that we don't have a mismatch in numbers of windows versus levels
                if {$dicomwin == ""} {
                    set dicomwin 0.0
                }
                if {$lev == ""} {
                    set lev 0.0
                }
                $dispNode AddWindowLevelPreset $dicomwin $lev
            }
            # now use the first one, this call turns off the auto flag
            $dispNode SetWindowLevelFromPreset 0
        }
      }
    }
    if {0} {
      if {$_dicomWindowLevel(window) != -1} {
        if {[$node IsA "vtkMRMLScalarVolumeNode"] == 1} {
          set dispNode [$node GetScalarVolumeDisplayNode]
          if {$dispNode != ""} {
            $dispNode AutoWindowLevelOff
            # for now, take the first if there's a list
            set dicomwin [lindex [split $_dicomWindowLevel(window) {\\}] 0]
            # puts "Got $dicomwin out of  $_dicomWindowLevel(window)"
            $dispNode SetWindow $dicomwin
          }
        }
      }
      if {$_dicomWindowLevel(level) != -1} {
        if {[$node IsA "vtkMRMLScalarVolumeNode"] == 1} {
          set dispNode [$node GetScalarVolumeDisplayNode]
          if {$dispNode != ""} {
            $dispNode AutoWindowLevelOff
            # for now, take the first if there's a list
            set lev  [lindex [split $_dicomWindowLevel(level) {\\}] 0]
            # puts "Got $lev out of $_dicomWindowLevel(level)"
            $dispNode SetLevel $lev
          }
        }
      }
    }
    $::slicer3::ApplicationLogic PropagateVolumeSelection

    $::slicer3::Application SetRegistry "OpenPath" [file dirname $fileName]

    $::slicer3::Application RequestRegistry "RecentFiles"
    set recentFiles [$::slicer3::Application GetRegistryHolder]
    if { [llength $recentFiles] > 15 } {
      set recentFiles [lrange $recentFiles 0 14]
    }
    set recentFiles [linsert $recentFiles 0 [list $name $fileName [array get options]]]
    $::slicer3::Application SetRegistry "RecentFiles" $recentFiles
  }
  $progressDialog SetParent ""
  $progressDialog SetMasterWindow ""
  $progressDialog Delete
  return 0
}


#
# handle gui events
# -basically just map button events onto methods
#
itcl::body LoadVolume::processEvent { {caller ""} {event ""} } {

  #
  # ignore events that occur while updating widgets that 
  # have related values unless it is a cancel event, in which
  # case we quit so user can start over (unwedges the dialog)
  #

  if { $caller == $o(cancel) } {
    after idle "itcl::delete object $this"
    set _processingEvents 0
    return
  }

  if { $_processingEvents } {
    return
  }
  set _processingEvents 1

  #
  # handle apply button
  #
  if { $caller == $o(apply) } {
    if { [$this apply] == 0 } {
      after idle "itcl::delete object $this"
    }
    set _processingEvents 0
    return
  }

  #
  # handle browse to cwd button
  #
  if { $caller == $o(cwd) } {
    $this selectArchetype [pwd] ""
    return
  }

  #
  # select archetype based on clicks in the file browser 
  #
  if { $caller == $o(browser) } {
    set fileTable [$o(browser) GetFileListTable]
    set fileName [$fileTable GetNthSelectedFileName 0]
    if { $fileName == "" } {
      set directoryExplorer [$o(browser) GetDirectoryExplorer]
      set fileName [$directoryExplorer GetSelectedDirectory]
    }
    if { [file isdirectory $fileName] } {
      set pathName [lindex [glob -directory $fileName -nocomplain *] 0]
      if { ![file isdirectory $pathName] } {
        set fileName $pathName
      }
    }
    set name [file root [file tail $fileName]]
    $this selectArchetype $fileName $name
    set _processingEvents 0
    return
  }

  #
  # select archetype based on selection from recent files menu
  #
  set menuButton [$o(recentMenu) GetWidget]
  if { $caller == $menuButton } {
    set menu [$menuButton GetMenu]
    set selection [$menu GetIndexOfSelectedItem]
    if { $selection == 0 } {
      set _processingEvents 0
      return ;# the '-' empty selection
    }
    if { $selection == [expr [$menu GetNumberOfItems] - 1] } {
      # last menu item is the reset list option
      $::slicer3::Application SetRegistry "RecentFiles" ""
      $menu DeleteAllItems
      after idle $menu SelectItem 0
      set _processingEvents 0
      return;
    }
    $::slicer3::Application RequestRegistry "RecentFiles"
    set recentFiles [$::slicer3::Application GetRegistryHolder]
    set fileRecord [lindex $recentFiles [expr $selection - 1]]
    if { $fileRecord != "" } {
      foreach {name fileName optionsList} $fileRecord {}
      array set options $optionsList
      $this selectArchetype $fileName $name options
      after idle $menu SelectItem 0
    }
    set _processingEvents 0
    return
  }

  #
  # select archetype based values typed in entry field
  #
  if { $caller == $o(path) } {
    set path [[$o(path) GetWidget] GetValue]
    set name [file tail $path]
    $this selectArchetype $path $name
    set _processingEvents 0
    return
  }

  #
  # parse the current directory for dicom files
  #
  if { $caller == $o(dicomParse) } {
    set path [[$o(path) GetWidget] GetValue]
    if { [file isdirectory $path] } {
      set dir $path
    } else {
      set dir [file dirname $path]
    }
    array unset _dicomTree
    array set _dicomTree ""
    set includeSubseries [$o(dicomParseSubseries) GetSelectedState]
    $this parseDICOMDirectory $dir _dicomTree $includeSubseries
    $this populateDICOMTree $dir _dicomTree
    set _processingEvents 0
    return
  }

  #
  # select archetype based on clicks in the dicom tree
  # - ignore the serial number at the start of each node id
  # - always select a series 
  # -- click patient or study selects first series
  # -- click file selects containing series
  #
  set t [$o(dicomTree) GetWidget]
  if { $caller == $t } {
    set selection [$t GetSelection]
    switch -glob $selection {
      {[0-9]*-patient*} {
        set patient $_dicomTree(subscriptName,$selection)
        set study [lindex $_dicomTree($patient,studies) 0]
        set series [lindex $_dicomTree($patient,$study,series) 0]
        set studyNode [lindex [$t GetNodeChildren $selection] 0]
        set seriesNode [lindex [$t GetNodeChildren $studyNode] 0]
      }
      {[0-9]*-study*} {
        set patientNode [$t GetNodeParent $selection]
        set patient $_dicomTree(subscriptName,$patientNode)
        set study $_dicomTree(subscriptName,$selection)
        set series [lindex $_dicomTree($patient,$study,series) 0]
        set seriesNode [lindex [$t GetNodeChildren $selection] 0]
      }
      {[0-9]*-series-*-file*} {
        set seriesNode [$t GetNodeParent $selection]
      }
      {[0-9]*-series*} {
        set seriesNode $selection
      }
      default {
        errorDialog "can't parse selection \"$selection\""
      }
    }

    $t SelectSingleNode $seriesNode

    # extract the file list corresponding to the selection (it is all the files in the selected series)
    # - the archetype file is the first one in the list
    # - use the cache of file names that was created when the dicom tree was populated
    set fileList ""
    foreach fileNode [$t GetNodeChildren $seriesNode] {
      set fileName $_dicomTree(subscriptName,$fileNode)
      lappend fileList $fileName
    }

    # use the first file in the list as the archetype, and use the series name as the default volume name
    set archetype [lindex $fileList 0]

    # - strip extra info from end of node text to get series name
    set seriesName [$t GetNodeText $seriesNode]
    set paren [string last ( $seriesName]
    if { $paren != -1 } {
      set seriesName [string range $seriesName 0 [expr $paren -1]]
    }

    $this selectArchetype $archetype $seriesName

    # set the file list corresponding to the selection (it is all the files in the selected series)
    set _dicomSeriesFileList $fileList

    # try to open the series and see the contents
    # - this is in a catch, since selectArchetype may have changed the layout
    #   of the tree
    set ret [catch "$t OpenNode $seriesNode" res]
    if { $ret } { puts $res }
    set ret [catch "$t SeeNode $selection" res]
    if { $ret } { puts $res }

    set _processingEvents 0
    return
  }
  
  puts "unknown event from $caller"
  set _processingEvents 0
}

#
# single spot where a given path is selected and widgets are updated
# - path can be either file or directory
# - directory portion of path is checked for dicom
# - default name for the selection is given
#
itcl::body LoadVolume::selectArchetype { path name {optionsName ""} } {

  set fileTable [$o(browser) GetFileListTable]
  set directoryExplorer [$o(browser) GetDirectoryExplorer]

  if { [file isdirectory $path] } {
    set directoryName $path
  } else {
    set directoryName [file dirname $path]
  }

  if { $optionsName != "" } {
    upvar $optionsName options
    foreach opt {centered orient label singleFile} {
      if { [info exists options($opt)] } {
        $o($opt) SetSelectedState $options($opt)
      }
    }
  }
  
  $directoryExplorer ClearSelection
  $directoryExplorer SelectDirectory $directoryName
  $fileTable ClearSelection
  $fileTable SelectFileName $path
  $fileTable ScrollToFile [file tail $path]

  [$o(path) GetWidget] SetValue $path
  [$o(name) GetWidget] SetValue $name
  $this populateDICOMTable $path

  #
  # if this is a different directory than we have loaded, then 
  # try to restore from cache.  If can't, then wait until
  # user clicks 'Parse Directory' button
  #
  if { ![info exists _dicomTree(directoryName)] || 
        $_dicomTree(directoryName) != $directoryName } {
    # get the dicom info for this directory if it exists
    set dicomCache [DICOMCache #auto]
    $dicomCache getTreeForDirectory $directoryName _dicomTree 1
    itcl::delete object $dicomCache
    $this populateDICOMTree $directoryName _dicomTree

    # look for the rest of the series and set the file names list 
    # as an instance variable to be used if the user selects apply
    # - use the first series that contains the archetype 
    # - when the file is selected from the tree, then the tree needs
    #   to set the series itself (this code doesn't know, since a file
    #   can be in multiple series)
    if { [lsearch $_dicomSeriesFileList $path] == -1 } {
      set _dicomSeriesFileList ""
      set fileLists [array names _dicomTree *files]
      foreach fileList $fileLists {
        if { [lsearch $_dicomTree($fileList) $path] != -1 } {
          set _dicomSeriesFileList $_dicomTree($fileList)
          break
        }
      }
    }
  }
}


itcl::body LoadVolume::errorDialog { errorText } {
  set dialog [vtkKWMessageDialog New]
  if { [info exists o(toplevel)] } {
    set parent $o(toplevel)
  } else {
    set parent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  }
  $dialog SetParent $parent
  $dialog SetMasterWindow $parent
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

itcl::body LoadVolume::yesNoDialog { yesNoText } {
  set dialog [vtkKWMessageDialog New]
  if { [info exists o(toplevel)] } {
    set parent $o(toplevel)
  } else {
    set parent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  }
  $dialog SetParent $parent
  $dialog SetMasterWindow $parent
  $dialog SetStyleToYesNo
  $dialog SetText $yesNoText
  $dialog Create
  set response [$dialog Invoke]
  $dialog Delete
  return $response
}

# this creates a progress dialog that needs to be managed and deleted by the caller
itcl::body LoadVolume::progressDialog { {message "Working..."} } {

  set progressDialog [vtkKWProgressDialog New]
  if { [info exists o(toplevel)] } {
    set parent $o(toplevel)
  } else {
    set parent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  }
  $progressDialog SetParent $parent
  $progressDialog SetMasterWindow $parent
  $progressDialog SetTitle "Progress"
  $progressDialog SetMessageText $message
  $progressDialog SetDisplayPositionToMasterWindowCenter
  $progressDialog Create
  $progressDialog SetSize 500 75
  $progressDialog ModalOn
  $progressDialog Display

  return $progressDialog
}

itcl::body LoadVolume::status { message } {
  $o(status) SetText $message
}

itcl::body LoadVolume::loadDICOMDictionary {} {
  if { [info exists _DICOM(loaded)] } {
    return
  }

  set itkBinDir ""
  if { [info exists ::env(ITK_BIN_DIR)] } {
    set itkBinDir $::env(ITK_BIN_DIR)
  }
  set dicomDict ""
  set dicomDictCandidates [list \
    $::Slicer_BUILD/include/InsightToolkit/gdcm/Dicts/gdcm.dic \
    $itkBinDir/../../include/InsightToolkit/gdcm/Dicts/gdcm.dic \
    $itkBinDir/../Utilities/gdcm/Dicts/gdcm.dic \
    $itkBinDir/../../Utilities/gdcm/Dicts/gdcm.dic \
    $::Slicer_BUILD/../Slicer3-lib/Insight-build/Utilities/gdcm/Dicts/gdcm.dic \
    ]
  foreach dictFile $dicomDictCandidates {
    if { [file exists $dictFile] } {
      set dicomDict $dictFile
      break
    }
  }

  if { $dicomDict == "" } {
    $this errorDialog "Cannot find dicom dictionary to load"
    set _DICOM(loaded) "Fail"
    return
  }

  set fp [open $dicomDict]
  while { ![eof $fp] } {
    gets $fp line
    set group [lindex $line 0]
    set element [lindex $line 1]
    set type [lindex $line 2]
    set rep [lindex $line 3]
    set description [lrange $line 4 end]
    set _DICOM($group|$element) $description
  }
  close $fp

  set _DICOM(loaded) "Success"
}

#
# fill the listbox with header values
# - save the scroll position to restore (so user can browse files while
#   looking at a particular tag)
# - show the most useful keys first, then show all keys
# - parse directory button is only enabled on valid-looking dicom files
#

itcl::body LoadVolume::populateDICOMTable {fileName} {

  set scrollState [[lindex [[[$o(dicomList) GetWidget] GetWidgetName] cget -yscrollcommand] 0] get]
  [$o(dicomList) GetWidget] DeleteAllRows

  $this parseDICOMHeader $fileName header
  if { $fileName == "" || ![file exists $fileName] || !$header(isDICOM) } {
    return
  }
  set w [$o(dicomList) GetWidget] 

  set firstKeys { 
    0010|0010 0010|0020 0008|0060 0008|0020 
    0008|1030 0008|0080 
    
    0020|0011 0008|103e
  }

  set row 0
  foreach key $firstKeys {
    set description $_DICOM($key)
    if { [info exists header($key,value)] } {
      set value $header($key,value)
    } else {
      set value "Not Available"
    }
    if { $showGroupElement } {
      $w InsertCellText $row $_dicomColumn(Group/Element) $key
    }
    $w InsertCellText $row $_dicomColumn(Description) $description
    $w InsertCellText $row $_dicomColumn(Value) $value
    incr row
  }

  for {set n 0} {$n < $header(numberOfKeys)} {incr n} {
    set key $header($n,key)
    set description $header($key,description)
    set value $header($key,value)
    if { $showGroupElement } {
      $w InsertCellText $row $_dicomColumn(Group/Element) $key
    }
    $w InsertCellText $row $_dicomColumn(Description) $description
    $w InsertCellText $row $_dicomColumn(Value) $value
    incr row
  }
  [[$o(dicomList) GetWidget] GetWidgetName] yview moveto [lindex $scrollState 0]
}

itcl::body LoadVolume::parseDICOMHeader {fileName arrayName} {

  upvar $arrayName header
  set header(fileName $fileName)
  set header(isDICOM) 0
  set _dicomWindowLevel(window) -1
  set _dicomWindowLevel(level) -1

  if { $fileName == "" || ![file exists $fileName] || [file isdirectory $fileName] } {
    return
  }

  $this loadDICOMDictionary

  if { ![info exists o(reader)] } {
    set o(reader) [vtkNew vtkITKArchetypeImageSeriesReader]
  }
  $o(reader) SetArchetype $fileName
  $o(reader) SetSingleFile 1
  set ret [catch "$o(reader) UpdateInformation" res]
  if { $ret } {
    # this isn't a file we can read
    puts "Can't read file $fileName"
    puts $res
    # need to create a fresh reader because "InAlgorithm" state is not updated by vtkExecutive on error
    set o(reader) [vtkNew vtkITKArchetypeImageSeriesReader]
    return
  }

  set isDICOM 0
  set header(numberOfKeys) [$o(reader) GetNumberOfItemsInDictionary]
  for {set n 0} {$n < $header(numberOfKeys)} {incr n} {
    set key [$o(reader) GetNthKey $n]
    set value [$o(reader) GetTagValue $key]

    if { ![string is print $value] } {
      set value "Not printable"
    }
    if { [info exists _DICOM($key)] } {
      set description $_DICOM($key)
      set isDICOM 1
    } else {
      set description "Unknown key"
    }
    set header($n,key) $key
    set header($key,description) $description
    set header($key,value) $value
    if {$description == "Window Center"} {
      set _dicomWindowLevel(level) $value
    }
    if {$description == "Window Width"} {
      set _dicomWindowLevel(window) $value
    }
  }
  set header(isDICOM) $isDICOM
}

itcl::body LoadVolume::safeNodeName {name} {
  set newname ""
  set len [string length $name]
  for {set n 0} {$n < $len} {incr n} {
    set c [string index $name $n]
    if { ![string is ascii $c] || ![string is alnum $c]} {
      scan $c "%c" value
      set c [format %%%02x $value]
    }
    if { [string is space $c] } {
      set c "+"
    }
    set newname $newname$c
  }
  return $newname
}

itcl::body LoadVolume::populateDICOMTree {directoryName arrayName} {

  upvar $arrayName tree

  # use a flag to know when tree is up to date
  # - this relies on the fact that the tree is always cleared each
  #   time the data is changed, so if this flag exists, we are in sync.
  if { [info exists tree(treePopulated)] } {
    return
  }
  set tree(directoryName) $directoryName

  set t [$o(dicomTree) GetWidget]
  $t DeleteAllNodes

  if { ![info exists tree(patients)] } {
    return
  }

  set progressDialog [$this progressDialog]
  $progressDialog SetTitle "Working..."
  $progressDialog SetMessageText "Displaying DICOM Tree for {$directoryName}..."
  update

  set n 0 ;# serial number of node
  foreach patient [lsort -dictionary $tree(patients)] {
    set patientNode $n-patient-[$this safeNodeName $patient]
    set tree(subscriptName,$patientNode) $patient
    incr n
    if { ![info exists tree(patients,displayName,$patient)] } {
      set tree(patients,displayName,$patient) $patient
    }
    $t AddNode "" $patientNode $tree(patients,displayName,$patient)
    $t OpenNode $patientNode
    foreach study [lsort -dictionary $tree($patient,studies)] {
      set studyNode $n-study-[$this safeNodeName $study]
      set tree(subscriptName,$studyNode) $study
      incr n
      if { ![info exists tree($patient,studies,displayName,$study)] } {
        set tree($patient,studies,displayName,$study) $study
      }
      $t AddNode $patientNode $studyNode $tree($patient,studies,displayName,$study)
      $t OpenNode $studyNode
      foreach series [lsort -dictionary $tree($patient,$study,series)] {
        set seriesNode $n-series-[$this safeNodeName $series]
        set tree(subscriptName,$seriesNode) $series
        incr n
        set fileCount [llength $tree($patient,$study,$series,files)]
        if { $fileCount == 1 } {set countString "file" } else { set countString "files" }
        if { ![info exists tree($patient,$study,series,displayName,$series)] } {
          set tree($patient,$study,series,displayName,$series) $series
        }
        $t AddNode $studyNode $seriesNode "$tree($patient,$study,series,displayName,$series) ($fileCount $countString)"
        foreach file $tree($patient,$study,$series,files) {
          set fileNode $n-$seriesNode-file-[$this safeNodeName $file]
          set tree(subscriptName,$fileNode) $file
          incr n
          $t AddNode $seriesNode $fileNode "[file tail $file] ($file)"
          if { [expr $n % 200] == 0 } {
            $progressDialog SetMessageText "$n files processed..."
            update
          }
        }
        if { [info exists tree($patient,$study,$series,warning)] } {
          set nodeText [$t GetNodeText $seriesNode]
          $t SetNodeText $seriesNode "$nodeText -- Warning! $tree($patient,$study,$series,warning)"
        }
      }
    }
  }

  set tree(treePopulated) 1

  $progressDialog SetParent ""
  $progressDialog SetMasterWindow ""
  $progressDialog Delete
}

itcl::body LoadVolume::parseDICOMDirectory {directoryName arrayName {includeSubseries 0} } {

  upvar $arrayName tree

  set progressDialog [$this progressDialog]
  $progressDialog SetTitle "Parsing DICOM Files in {$directoryName}..."
  $progressDialog SetMessageText "Starting..."

  set PATIENT "0010|0010"
  set STUDY "0008|1030"
  set SERIES "0008|103e"
  set SERIESNUMBER "0020|0011"

  set files [glob -nocomplain -directory $directoryName *]
  set subdirs ""
  foreach f $files {
    if { [file isdirectory $f] } {
      lappend subdirs $f
    }
  }

  set parseSubdirs 0
  if { $subdirs != "" } {
    set t "Directory\n$directoryName\nContains subdirectories.\n\nWould you like to also parse them?"
    set parseSubdirs [$this yesNoDialog $t]
  }

  # accumulate a list of all subdirs
  if { $parseSubdirs } {
    set files [::LoadVolume::FindSubfiles $directoryName "$progressDialog SetMessageText "]
  }

  set totalFiles [llength $files]
  set fileCount 0

  if { $totalFiles == 0 } {
    $this errorDialog "No files found in directory\n$directoryName\n"
  }

  set tree(patients) ""
  set tree(directoryName) $directoryName
  foreach f $files {

    set ff [file tail $f]
    incr fileCount
    set progress [expr pow((1. * $fileCount) / $totalFiles,2)]
    $progressDialog SetMessageText [format %.40s "Examining $ff..."]
    $progressDialog UpdateProgress $progress
    update


    $this parseDICOMHeader $f header
    if { !$header(isDICOM) } {
      continue
    }

    foreach key "patient study series" {
      set tag [set [string toupper $key]]
      if { ![info exists header($tag,value)] } {
        set $key "Unknown[string totitle $key]" ;# missing group/element in header
        set display$key [set $key]
      } elseif { $header($tag,value) == "" } {
        # one of the keys has an empty string value - create a dummy
        # TODO: could be using UIDs
        set $key "Unnamed[string totitle $key]"
        set display$key [set $key]
      } else {
        set display$key $header($tag,value) ;# normal tag value
        set $key [$this safeNodeName $header($tag,value)] ;# escaped version
      }

      if { $key == "series" && [info exists header($SERIESNUMBER,value)] } {
        set seriesNumber $header($SERIESNUMBER,value)
        set series "$seriesNumber-$series"
        set displayseries "$seriesNumber-$displayseries"
      }
    }

    if { [lsearch $tree(patients) $patient] == -1 } {
      lappend tree(patients) $patient
      set tree(patients,displayName,$patient) $displaypatient
      set tree($patient,studies) ""
    }
    if { [lsearch $tree($patient,studies) $study] == -1 } {
      lappend tree($patient,studies) $study
      set tree($patient,studies,displayName,$study) $displaystudy
      set tree($patient,$study,series) ""
    }
    if { [lsearch $tree($patient,$study,series) $series] == -1 } {
      lappend tree($patient,$study,series) $series
      set tree($patient,$study,series,displayName,$series) $displayseries
    }
    lappend tree($patient,$study,$series,files) $f
    set tree($f,header) [array get header]
  }

  $progressDialog SetMessageText "Organizing Files..."
  $progressDialog UpdateProgress 1.
  $this organizeDICOMSeries tree $includeSubseries "$progressDialog SetMessageText "

  $progressDialog SetParent ""
  $progressDialog SetMasterWindow ""
  $progressDialog Delete

  # save the tree for later access...
  set dicomCache [DICOMCache #auto]
  $dicomCache setTreeForDirectory $tree(directoryName) tree
  itcl::delete object $dicomCache
}

#
# Organize a dicom study into coherent and correctly ordered volues
# corresponding to the series information
#
itcl::body LoadVolume::organizeDICOMSeries {arrayName {includeSubseries 0} {progressCmd ""} } {

  upvar $arrayName tree

  #
  # Here, subdivide series if they have different 
  # diffusion gradents, orientations, content timings, etc.
  # The difficulty is that different modalities (and vendors?) put
  # files together into the same series that slicer would consider
  # distinct volumes and in some cases these can contradict.
  # The approach here is to have multiple "virtual" series that 
  # divide the data in different ways and let the user select
  # the desired one.
  #
  #  SliceLocation                  0020|1041
  # 

  if { $includeSubseries } {
    set subseriesSpecs {
      SeriesInstanceUID              0020|000E
      ContentTime                    0008|0033
      TriggerTime                    0018|1060
      DiffusionGradientOrientation   0018|9089 
      ImageOrientationPatient        0020|0037
    }
  } else {
    set subseriesSpecs ""
  }

  foreach patient $tree(patients) {
    foreach study $tree($patient,studies) {
      foreach series $tree($patient,$study,series) {
        if { $progressCmd != "" } {
          eval $progressCmd [list "Sorting series \n$patient\n$study\n$series\n"]
          update ;# TODO: this update should be in the progressCmd itself
        }

        # first, look for subseries within this series
        array unset subseriesValues
        array set subseriesValues ""
        foreach {name tag} $subseriesSpecs {
          foreach f $tree($patient,$study,$series,files) {
            array set header $tree($f,header)
            if { ![info exists header($tag,value)] } {
              set value "Unknown"
            } else {
              set value $header($tag,value)
            }
            lappend subseriesValues($name,$value,files) $f
          }
        }


        # second, for any specs that have more than one value, create a new
        # virtual series
        if { $progressCmd != "" } {
          set ret [eval $progressCmd [list "Sorting sub series \n$patient\n$study\n$series\n"]]
          update ;# TODO: this update should be in the progressCmd itself
        }
        foreach {name tag} $subseriesSpecs {
          set subseries [array names subseriesValues $name*files]
          if { [llength $subseries] > 1 } {
            foreach sub $subseries {
              foreach {name value files} [split $sub ","] {}
              set seriesName $tree($patient,$study,series,displayName,$series)
              set newSeries "$seriesName for $name $value"
              lappend tree($patient,$study,series) $newSeries
              set tree($patient,$study,$newSeries,files) $subseriesValues($sub)
            }
          }
        }
      }
    }
  }



  #
  # sort each series geometrically
  #
  # TODO: more consistency checks:
  # - is there gantry tilt?
  # - are the orientations the same for all slices?
  #
  set POSITION "0020|0032"
  set ORIENTATION "0020|0037"
  set NUMBER_OF_FRAMES "0028|0008"
  set spaceWarnings 0
  foreach patient $tree(patients) {
    foreach study $tree($patient,studies) {
      foreach series $tree($patient,$study,series) {
        if { $progressCmd != "" } {
          set ret [eval $progressCmd [list "Geometric analysis of \n$patient\n$study\n$series\n"]]
          update ;# TODO: this update should be in the progressCmd itself
        }
        #
        # use the first file to get the ImageOrientationPatient for the 
        # series and calculate the scan direction (assumed to be perpendicular
        # to the acquisition plane)
        # - note: DICOM uses backslashes as delimeters, use regsub to make lists
        #
        set refFile [lindex $tree($patient,$study,$series,files) 0]
        array set refHeader $tree($refFile,header)

        if { [lsearch [array names refHeader] $NUMBER_OF_FRAMES,value] != -1 } {
          set tree($patient,$study,$series,warning) "Multi-frame image. If slice orientation or spacing is non-uniform then the image may be displayed incorrectly. Use with caution."
          continue
        }

        set validGeometry 1
        foreach tag {POSITION ORIENTATION} {
          set key [set $tag],value
          if { [lsearch [array names refHeader] $key] == -1 } {
            set tree($patient,$study,$series,warning) "reference image in series does not contain a value for tag $tag Please use caution."
            set validGeometry 0
            break
          }
        }

        if { $validGeometry == 0 } {
          continue
        }

        set refOrientation $refHeader($ORIENTATION,value)
        regsub -all "\\\\" $refOrientation " " sliceAxes
        set scanAxis [eval ::LoadVolume::Cross $sliceAxes]
        set refPosition $refHeader($POSITION,value)
        regsub -all "\\\\" $refPosition " " scanOrigin

        #
        # for each file in series, calculate the distance along
        # the scan axis, sort files by this
        #
        set origFiles $tree($patient,$study,$series,files)
        set sortList ""
        foreach f $origFiles {
          array set header $tree($f,header)
          regsub -all "\\\\" $header($POSITION,value) " " pos
          set vec [eval ::LoadVolume::Difference $pos $scanOrigin]
          set dist [eval ::LoadVolume::Dot $vec $scanAxis]
          lappend sortList [list $f $dist]
        }
        set sortedFiles [lsort -real -index 1 $sortList]
        set tree($patient,$study,$series,files) ""
        foreach element $sortedFiles {
          foreach {file dist} $element {}
          lappend tree($patient,$study,$series,files) $file
          set tree($patient,$study,$series,$file,dist) $dist
        }

        #
        # confirm equal spacing between slices
        # - use public variable 'epsilon' to determine the tolerance
        #
        set fileCount [llength $tree($patient,$study,$series,files)]
        if { $fileCount > 1 } {
          set file0 [lindex $tree($patient,$study,$series,files) 0]
          set file1 [lindex $tree($patient,$study,$series,files) 1]
          set dist0 $tree($patient,$study,$series,$file0,dist)
          set dist1 $tree($patient,$study,$series,$file1,dist)
          set spacing0 [expr $dist1 - $dist0]
          for {set n 2} {$n < $fileCount} {incr n} {
            set fileN [lindex $tree($patient,$study,$series,files) $n]
            set fileNminus1 [lindex $tree($patient,$study,$series,files) [expr $n - 1]]
            set distN $tree($patient,$study,$series,$fileN,dist)
            set distNminus1 $tree($patient,$study,$series,$fileNminus1,dist)
            set spacingN [expr $distN - $distNminus1]
            set spaceError [expr $spacingN - $spacing0]
            if { [expr abs($spaceError)] > $epsilon } {
              incr spaceWarnings
              set tree($patient,$study,$series,warning) "images are not equally spaced (a difference of $spaceError in spacings was detected).  Slicer will load this series as if it had a spacing of $spacing0.  Please use caution."
              break
            }
          }
        }
      }
    }
  }
  if { $spaceWarnings != 0 } {
    $this errorDialog "Geometric issues were found with $spaceWarnings of the series.  Please use caution."
  }
}
