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


namespace eval Loader {

  # 
  # utility to bring up the current window or create a new one
  # - optional path is added to dialog
  #
  proc ShowDialog { {path ""} } {

    set loaders [itcl::find objects -class Loader]
    if { $loaders != "" } {
      set loader [lindex $loaders 0]
      array set o [$loader objects]
      raise [$o(toplevel) GetWidgetName]
    } else {
      set loader [Loader #auto]
    }
    if { $path != "" } {
      # deal with spaces in a path name
      lappend pathList $path
      $loader add $pathList
    }
  }

  # 
  # utility to load an archetype 
  # - either an archetype filename
  # - or a directory, from which the first file is selected
  #   (assumes only one set of files in the directory)
  #
  proc LoadArchetype { path {centered 0} {labelMap 0} {name ""} } {

    if { ![file exists $path] } {
      error "path does not exist: $path"
    }

    if { [file isdir $path] } {
      set files [glob -nocomplain $path/*]
      set archetype [lindex $files 0] 
    } else {
      set archetype $path
    }

    if { $name == "" } {
      set name [file tail [file root $path]]
    }

    set volumeLogic [$::slicer3::VolumesGUI GetLogic]
    ## set node [$volumeLogic AddArchetypeVolume $archetype $centered $labelMap $name]
    ## There is no need to provide single file loading option when "Add Data"
    set loadingOptions [expr $labelMap * 1 + $centered * 2]
    set node [$volumeLogic AddArchetypeVolume $path $name $loadingOptions]
    set selNode [$::slicer3::ApplicationLogic GetSelectionNode]

    if { $node == "" } {
      error "Could not open $archetype"
    } else {
      if { $labelMap } {
        $selNode SetReferenceActiveLabelVolumeID [$node GetID]
      } else {
        $selNode SetReferenceActiveVolumeID [$node GetID]
      }
      $::slicer3::ApplicationLogic PropagateVolumeSelection
    }
  }

}


#
# The partent class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class Loader] == "" } {

  itcl::class Loader {

    constructor  { } {
    }

    destructor {
      vtkDelete  
    }

    # configure options
    public variable recurse 1  ;# recurse into directories when adding 
    public variable filter "*"  ;# filter for which files to chose when adding directory

    variable _vtkObjects ""

    variable o ;# array of the objects for this widget, for convenient access
    variable col ;# array of the column indices for easy (and readable) access

    variable _volumeExtensions ".hdr .nhdr .nrrd .mhd .mha .vti .nii .mgz"
    variable _modelExtensions ".vtk .vtp .pial .inflated"
    variable _qdecExtensions ".qdec"
    variable _xcedeExtensions ".xcat"
    variable _observerRecords ""
    variable _cleanupDirs ""
    variable browserResult ""

    # methods
    method clear {} {}
    method add {paths} {}
    method addRow { path type } {}
    method processEvent {{caller ""} {event ""}} {}
    method setAll { field value } {}
    method apply {} {}
    method errorDialog {errorText} {}
    method status {message} {}
    method chooseDirectory {} {}
    method getOpenFile {} {}

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
itcl::body Loader::constructor { } {

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
  # actions buttons
  #
  set o(actionFrame) [vtkNew vtkKWFrame]
  $o(actionFrame) SetParent $o(toplevel)
  $o(actionFrame) Create
  pack [$o(actionFrame) GetWidgetName] -side top -anchor nw -fill x

  set actions { 
    "Clear Entries"
    "Select All" "Select None"
    "Label All" "Label None"
    "Centered All" "Centered None"
  }
  set widgets ""
  foreach a $actions {
    set aa [string tolower [string index $a 0]]
    regsub -all " " $a "" nospace
    set aa $aa[string range $nospace 1 end]
    set o($aa) [vtkNew vtkKWPushButton]
    $o($aa) SetParent $o(actionFrame)
    $o($aa) Create
    $o($aa) SetText $a
    set tag [$o($aa) AddObserver ModifiedEvent "$this processEvent $o($aa)"]
    lappend _observerRecords [list $o($aa) $tag]
    $o($aa) SetCommand $o($aa) Modified
    lappend widgets [$o($aa) GetWidgetName]
  }
  eval pack $widgets -side left 

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
  # a status label
  #
  set o(status) [vtkNew vtkKWLabel]
  $o(status) SetParent $o(toplevel)
  $o(status) Create
  $o(status) SetText ""
  $o(status) SetBalloonHelpString "Current status of the load process"
  pack [$o(status) GetWidgetName] -side top -anchor e -padx 2 -pady 2 -expand true -fill x


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
  set tag [$o(addDir) AddObserver ModifiedEvent "$this processEvent $o(addDir)"]
  lappend _observerRecords [list $o(addDir) $tag]
  $o(addDir) SetCommand $o(addDir) Modified

  set o(addFile) [vtkNew vtkKWPushButton]
  $o(addFile) SetParent $o(buttonFrame)
  $o(addFile) Create
  $o(addFile) SetText "Add File(s)"
  $o(addFile) SetBalloonHelpString "Add a file or multiple files to the list of files to load"
  set tag [$o(addFile) AddObserver ModifiedEvent "$this processEvent $o(addFile)"]
  lappend _observerRecords [list $o(addFile) $tag]
  $o(addFile) SetCommand $o(addFile) Modified

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
    [$o(addFile) GetWidgetName] \
    [$o(addDir) GetWidgetName] \
    -side right -anchor w -padx 4 -pady 2


  $o(toplevel) Display
}


itcl::body Loader::destructor {} {

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  $this vtkDelete

  #
  # clean up any temp directories 
  #
  foreach dir $_cleanupDirs {
    file delete -force $dir
  }

}


# remove entries from the list box
itcl::body Loader::clear { } {
  [$o(list) GetWidget] DeleteAllRows
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
    $w InsertCellTextAsInt $i $col(Centered) 0
    $w SetCellWindowCommandToCheckButton $i $col(Centered)

    set seg [string match -nocase "*seg*" $path] 
    $w InsertCellTextAsInt $i $col(LabelMap) $seg
    $w SetCellWindowCommandToCheckButton $i $col(LabelMap)
  } 

}

# add a new archetype or directory to the list box
# - does a search of the path for volumes and models
# TODO: add a recursion counter and offer a dialog box
#       allowing user to cancel out if too many directories
#       are being added
#
itcl::body Loader::add { paths } {

  foreach path $paths {
    $this status "Examining $path"
    set ext [string tolower [file extension $path]]

    #
    # if it's a zip archive, copy the contents to a temp
    # directory and load from there
    #
    if { $ext == ".zip" || $ext == ".xar" } {

      set tmp [$::slicer3::Application GetTemporaryDirectory]
      set tmp $tmp/Loader-[pid]-[clock seconds]-[clock clicks]
      file mkdir $tmp
      $this status "Please wait...\nUnpacking $path into $tmp"
      update

      #
      # look for unzip on the system, if not found try the 
      # tcl fallback
      #
      set unzip ""
      set candidates {
        "c:/cygwin/bin/unzip.exe"
        /usr/bin/unzip /bin/unzip /usr/local/bin/unzip
      }
      foreach c $candidates {
        if { [file exists $c] } {
          set unzip $c
          break
        }
      }
      if { [file exists $unzip] } {
        set cwd [pwd]
        cd $tmp
        set fp [open "| $unzip \"$path\"" "r"]
        while { ![eof $fp] } {
          set line [read $fp]
          $this status $line
          after 50 update
        }
        catch "close $fp"
        $this add [list $tmp]
        lappend _cleanupDirs $tmp
        cd $cwd
      } else {
        # use the tcl vfs zip implementation - slow
        set ret [catch "package require vfs::zip" res]
        if { $ret } {
          $this errorDialog "Cannot find zipfile reader, cannot process $path"
        } else {

          # use new routines
          uplevel #0 eval $::_fixed_zip_code

          # TODO: let user pick the destination to save the unzipped data
          set fd [::vfs::zip::Mount $path /zipfile]
          file copy -force /zipfile $tmp
          $this add [list $tmp]
          ::vfs::zip::Unmount $fd /zipfile
          lappend _cleanupDirs $tmp
        }
      }
      continue
    }

    # 
    # if it's a directory, look at each element (recurse)
    #
    if { [file isdir $path] && $recurse} {
      foreach item [glob -nocomplain $path/*] {
        $this add [list $item]
      }
    } else {
      #
      # if it's a file, see if it's something we know how to load
      #

      if { [string match $filter $path] } {
        $this status "Adding $path"
        if { [lsearch $_volumeExtensions $ext] != -1 } {
          $this addRow $path "Volume"
          $this status ""
        } elseif { [lsearch $_modelExtensions $ext] != -1 } {
          $this addRow $path "Model"
          $this status ""
        } elseif { [lsearch $_xcedeExtensions $ext] != -1 } {
            $this addRow $path "XCEDE"
            $this status ""
        } elseif { [lsearch $_qdecExtensions $ext] != -1 } {
          $this addRow $path "QDEC"
          $this status ""
        } else {
          $this status "Cannot read file $path"
        }
      }
    }
  }
}

#
# load the items from the list box
# into slicer - ignore unselected rows
#
itcl::body Loader::apply { } {

  set mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  set progressGauge [$mainWindow GetProgressGauge]
  $progressGauge SetValue 0
  
  set w [$o(list) GetWidget] 
  set rows [$w GetNumberOfRows]

  for {set row 0} {$row < $rows} {incr row} {

    $progressGauge SetValue [expr 100 * $row / (1. * $rows)]

    if { [$w GetCellTextAsInt $row $col(Select)] } {

      set path [$w GetCellText $row $col(File)]
      set name [$w GetCellText $row $col(Name)]
      $mainWindow SetStatusText "Loading: $path"

      switch [$w GetCellText $row $col(Type)] {
        "Volume" {
          set centered [$w GetCellTextAsInt $row $col(Centered)]
          set labelMap [$w GetCellTextAsInt $row $col(LabelMap)]
          set volumeLogic [$::slicer3::VolumesGUI GetLogic]
          ## set node [$volumeLogic AddArchetypeVolume $path $centered $labelMap $name]
          set loadingOptions [expr $labelMap * 1 + $centered * 2]
          set node [$volumeLogic AddArchetypeVolume $path $name $loadingOptions]
          set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
          if { $node == "" } {
            $this errorDialog "Could not open $path"
          } else {
            if { $labelMap } {
              $selNode SetReferenceActiveLabelVolumeID [$node GetID]
            } else {
              $selNode SetReferenceActiveVolumeID [$node GetID]
            }
            $::slicer3::ApplicationLogic PropagateVolumeSelection
          }
        }
        "Model" {
          set node [[$::slicer3::ModelsGUI GetLogic] AddModel $path]
          if { $node == "" } {
            $this errorDialog "Could not open $path"
          } else {
            $node SetName $name
          }
        }
        "XCEDE" {
          set pass [ XcedeCatalogImport $path ]
          if { $pass == 0 } {
            $this errorDialog "Could not load XCEDE catalog at $path."
          }
        }
        "QDEC" {
          if {[info exists ::slicer3::QdecModuleGUI]} {
            set err [$::slicer3::QdecModuleGUI LoadProjectFile $path]
            if {$err == -1} {
              $this errorDialog "Could not load QDEC project $path (using $tmp to unpack into)"
            }
          } else {
              $this errorDialog "QDEC module not present, cannot open $path"
          }
        }
      }
    }
  }
  $mainWindow SetStatusText ""
  $progressGauge SetValue 0
}


#
# handle gui events
# -basically just map button events onto methods
#
itcl::body Loader::processEvent { {caller ""} {event ""} } {

  if { $caller == $o(addDir) } {
    # TODO: switch to kwwidgets directory browser
    set paths [$this chooseDirectory]
    $this add $paths
    return
  }

  if { $caller == $o(addFile) } {
    # TODO: switch to kwwidgets directory browser
    $this add [$this getOpenFile]
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
  
  if { $caller == $o(clearEntries) } {
    $this clear
    return
  }

  if { $caller == $o(selectAll) } {
    $this setAll Select 1
    return
  }

  if { $caller == $o(selectNone) } {
    $this setAll Select 0
    return
  }

  if { $caller == $o(centeredAll) } {
    $this setAll Centered 1
    return
  }

  if { $caller == $o(centeredNone) } {
    $this setAll Centered 0
    return
  }

  if { $caller == $o(labelAll) } {
    $this setAll LabelMap 1
    return
  }

  if { $caller == $o(labelNone) } {
    $this setAll LabelMap 0
    return
  }

  puts "$this: unknown event from $caller"
}

itcl::body Loader::setAll { field value } {
  set w [$o(list) GetWidget] 
  set rows [$w GetNumberOfRows]
  for {set row 0} {$row < $rows} {incr row} {
    $w SetCellTextAsInt $row $col($field) $value
    $w SetCellWindowCommandToCheckButton $row $col($field)
  }
}

itcl::body Loader::errorDialog { errorText } {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent $o(toplevel)
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

itcl::body Loader::status { message } {
  $o(status) SetText $message
}

itcl::body Loader::chooseDirectory {} {

  if { 0 } {
    #
    # use the kwwidgets load/save dialog
    #
    set dialog [vtkKWLoadSaveDialog New]
    $dialog ChooseDirectoryOn
    $dialog SetParent $o(toplevel)
    $dialog Create
    $dialog RetrieveLastPathFromRegistry "OpenPath"
    $dialog Invoke
    set dir ""
    if { [[$dialog GetFileNames] GetNumberOfValues] } {
      set dir [[$dialog GetFileNames] GetValue 0]
    }

    if { $dir != "" } {
      $dialog SaveLastPathToRegistry "OpenPath"
    }

    $dialog Delete

    return $dir

  } else {
    #
    # use a custom dialog with filtering and recursion options
    #
    set var ::Loader::browserSync_[namespace tail $this]
    set $var ""
    set browser [FilteredDirectoryDialog #auto]
    $browser configure -ok_command "set $var ok"
    $browser configure -cancel_command "set $var cancel"
    vwait $var

    if { [set $var] == "ok" } {
      $this configure -recurse [$browser getRecurse]
      $this configure -filter [$browser getFilter]
      set paths [$browser getPaths]
    } else {
      set paths ""
    }
    itcl::delete object $browser

    return $paths
  }
  
}

itcl::body Loader::getOpenFile {} {


  set dialog [vtkKWFileBrowserDialog New]
  $dialog MultipleSelectionOn
  $dialog SetParent $o(toplevel)
  $dialog Create
  $dialog RetrieveLastPathFromRegistry "OpenPath"
  $dialog Invoke

  set files ""
  set names [$dialog GetFileNames]
  set namesSize [$names GetNumberOfValues]
  for {set i 0} {$i < $namesSize} {incr i} {
    lappend files [$names GetValue $i]
  }
  
  if { $files != "" } {
    $dialog SaveLastPathToRegistry "OpenPath"
  }

  $dialog Delete
  return $files
}



################################################################################
#
# code below comes from updated version of tcl vfs - include it
# here to fix a zero length file problem (but make it work against
# the standard slicer lib distribution)
#
set ::_fixed_zip_code {

    # Using the vfs, memchan and Trf extensions, we ought to be able
    # to write a Tcl-only zip virtual filesystem.  What we have below
    # is basically that.

    namespace eval vfs::zip {}

    # Used to execute a zip archive.  This is rather like a jar file
    # but simpler.  We simply mount it and then source a toplevel
    # file called 'main.tcl'.
    proc vfs::zip::Execute {zipfile} {
        Mount $zipfile $zipfile
        source [file join $zipfile main.tcl]
    }

    proc vfs::zip::Mount {zipfile local} {
        set fd [::zip::open [::file normalize $zipfile]]
        vfs::filesystem mount $local [list ::vfs::zip::handler $fd]
        # Register command to unmount
        vfs::RegisterMount $local [list ::vfs::zip::Unmount $fd]
        return $fd
    }

    proc vfs::zip::Unmount {fd local} {
        vfs::filesystem unmount $local
        ::zip::_close $fd
    }

    proc vfs::zip::handler {zipfd cmd root relative actualpath args} {
        #::vfs::log [list $zipfd $cmd $root $relative $actualpath $args]
        if {$cmd == "matchindirectory"} {
            eval [list $cmd $zipfd $relative $actualpath] $args
        } else {
            eval [list $cmd $zipfd $relative] $args
        }
    }

    proc vfs::zip::attributes {zipfd} { return [list "state"] }
    proc vfs::zip::state {zipfd args} {
        vfs::attributeCantConfigure "state" "readonly" $args
    }

    # If we implement the commands below, we will have a perfect
    # virtual file system for zip files.

    proc vfs::zip::matchindirectory {zipfd path actualpath pattern type} {
        #::vfs::log [list matchindirectory $path $actualpath $pattern $type]

        # This call to zip::getdir handles empty patterns properly as asking
        # for the existence of a single file $path only
        set res [::zip::getdir $zipfd $path $pattern]
        #::vfs::log "got $res"
        if {![string length $pattern]} {
            if {![::zip::exists $zipfd $path]} { return {} }
            set res [list $actualpath]
            set actualpath ""
        }

        set newres [list]
        foreach p [::vfs::matchCorrectTypes $type $res $actualpath] {
            lappend newres [file join $actualpath $p]
        }
        #::vfs::log "got $newres"
        return $newres
    }

    proc vfs::zip::stat {zipfd name} {
        #::vfs::log "stat $name"
        ::zip::stat $zipfd $name sb
        #::vfs::log [array get sb]
        array get sb
    }

    proc vfs::zip::access {zipfd name mode} {
        #::vfs::log "zip-access $name $mode"
        if {$mode & 2} {
            vfs::filesystem posixerror $::vfs::posix(EROFS)
        }
        # Readable, Exists and Executable are treated as 'exists'
        # Could we get more information from the archive?
        if {[::zip::exists $zipfd $name]} {
            return 1
        } else {
            error "No such file"
        }
        
    }

    proc vfs::zip::open {zipfd name mode permissions} {
        #::vfs::log "open $name $mode $permissions"
        # return a list of two elements:
        # 1. first element is the Tcl channel name which has been opened
        # 2. second element (optional) is a command to evaluate when
        #    the channel is closed.

        switch -- $mode {
            "" -
            "r" {
                if {![::zip::exists $zipfd $name]} {
                    vfs::filesystem posixerror $::vfs::posix(ENOENT)
                }
                
                ::zip::stat $zipfd $name sb

                set nfd [vfs::memchan]
                fconfigure $nfd -translation binary

                seek $zipfd $sb(ino) start
                zip::Data $zipfd sb data

                puts -nonewline $nfd $data

                fconfigure $nfd -translation auto
                seek $nfd 0
                return [list $nfd]
            }
            default {
                vfs::filesystem posixerror $::vfs::posix(EROFS)
            }
        }
    }

    proc vfs::zip::createdirectory {zipfd name} {
        #::vfs::log "createdirectory $name"
        vfs::filesystem posixerror $::vfs::posix(EROFS)
    }

    proc vfs::zip::removedirectory {zipfd name recursive} {
        #::vfs::log "removedirectory $name"
        vfs::filesystem posixerror $::vfs::posix(EROFS)
    }

    proc vfs::zip::deletefile {zipfd name} {
        #::vfs::log "deletefile $name"
        vfs::filesystem posixerror $::vfs::posix(EROFS)
    }

    proc vfs::zip::fileattributes {zipfd name args} {
        #::vfs::log "fileattributes $args"
        switch -- [llength $args] {
            0 {
                # list strings
                return [list]
            }
            1 {
                # get value
                set index [lindex $args 0]
                return ""
            }
            2 {
                # set value
                set index [lindex $args 0]
                set val [lindex $args 1]
                vfs::filesystem posixerror $::vfs::posix(EROFS)
            }
        }
    }

    proc vfs::zip::utime {fd path actime mtime} {
        vfs::filesystem posixerror $::vfs::posix(EROFS)
    }

    # Below copied from TclKit distribution

    #
    # ZIP decoder:
    #
    # Format of zip file:
    # [ Data ]* [ TOC ]* EndOfArchive
    #
    # Note: TOC is refered to in ZIP doc as "Central Archive"
    #
    # This means there are two ways of accessing:
    #
    # 1) from the begining as a stream - until the header
    #    is not "PK\03\04" - ideal for unzipping.
    #
    # 2) for table of contents without reading entire
    #   archive by first fetching EndOfArchive, then
    #   just loading the TOC
    #

    namespace eval zip {
        array set methods {
            0   {stored - The file is stored (no compression)}
            1   {shrunk - The file is Shrunk}
            2   {reduce1 - The file is Reduced with compression factor 1}
            3   {reduce2 - The file is Reduced with compression factor 2}
            4   {reduce3 - The file is Reduced with compression factor 3}
            5   {reduce4 - The file is Reduced with compression factor 4}
            6   {implode - The file is Imploded}
            7   {reserved - Reserved for Tokenizing compression algorithm}
            8   {deflate - The file is Deflated}
            9   {reserved - Reserved for enhanced Deflating}
            10   {pkimplode - PKWARE Date Compression Library Imploding}
        }
        # Version types (high-order byte)
        array set systems {
            0   {dos}
            1   {amiga}
            2   {vms}
            3   {unix}
            4   {vm cms}
            5   {atari}
            6   {os/2}
            7   {macos}
            8   {z system 8}
            9   {cp/m}
            10   {tops20}
            11   {windows}
            12   {qdos}
            13   {riscos}
            14   {vfat}
            15   {mvs}
            16   {beos}
            17   {tandem}
            18   {theos}
        }
        # DOS File Attrs
        array set dosattrs {
            1   {readonly}
            2   {hidden}
            4   {system}
            8   {unknown8}
            16   {directory}
            32   {archive}
            64   {unknown64}
            128   {normal}
        }

        proc u_short {n}  { return [expr { ($n+0x10000)%0x10000 }] }
    }

    proc zip::DosTime {date time} {
        set time [u_short $time]
        set date [u_short $date]

        set sec [expr { ($time & 0x1F) * 2 }]
        set min [expr { ($time >> 5) & 0x3F }]
        set hour [expr { ($time >> 11) & 0x1F }]

        set mday [expr { $date & 0x1F }]
        set mon [expr { (($date >> 5) & 0xF) }]
        set year [expr { (($date >> 9) & 0xFF) + 1980 }]

        set dt [format {%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d} \
            $year $mon $mday $hour $min $sec]
        return [clock scan $dt -gmt 1]
    }


    proc zip::Data {fd arr {varPtr ""} {verify 0}} {
        upvar 1 $arr sb

        if { $varPtr != "" } {
            upvar 1 $varPtr data
        }

        set buf [read $fd 30]
        set n [binary scan $buf A4sssssiiiss \
                    hdr sb(ver) sb(flags) sb(method) \
                    time date \
                    sb(crc) sb(csize) sb(size) flen elen]

        if { ![string equal "PK\03\04" $hdr] } {
            binary scan $hdr H* x
            error "bad header: $x"
        }
        set sb(ver)        [u_short $sb(ver)]
        set sb(flags)   [u_short $sb(flags)]
        set sb(method)   [u_short $sb(method)]
        set sb(mtime)   [DosTime $date $time]

        set sb(name) [read $fd [u_short $flen]]
        set sb(extra) [read $fd [u_short $elen]]

        if { $varPtr == "" } {
            seek $fd $sb(csize) current
        } else {
            # Added by Chuck Ferril 10-26-03 to fix reading of OpenOffice
            #  .sxw files. Any files in the zip that had a method of 8
            #  (deflate) failed here because size and csize were zero.
            #  I'm not sure why the above computes the size and csize
            #  wrong, but stat appears works properly. I originally
            #  checked for csize of zero, but adding this change didn't
            #  appear to break the none deflated file access and seemed
            #  more natural.
            zip::stat $fd $sb(name) sb

            set data [read $fd $sb(csize)]
        }

        if { $sb(flags) & 0x4 } {
            # Data Descriptor used
            set buf [read $fd 12]
            binary scan $buf iii sb(crc) sb(csize) sb(size)
        }


        if { $varPtr == "" } {
            return ""
        }

        if { $sb(method) != 0 } {
            if { [catch {
                set data [vfs::zip -mode decompress -nowrap 1 $data]
            } err] } {
                ::vfs::log "$sb(name): inflate error: $err"
                binary scan $data H* x
                ::vfs::log $x
            }
        }
        return
        if { $verify } {
            set ncrc [vfs::crc $data]
            if { $ncrc != $sb(crc) } {
                tclLog [format {%s: crc mismatch: expected 0x%x, got 0x%x} \
                        $sb(name) $sb(crc) $ncrc]
            }
        }
    }

    proc zip::EndOfArchive {fd arr} {
        upvar 1 $arr cb

        # [SF Tclvfs Bug 1003574]. Do not seek over beginning of file.
        seek $fd 0 end
        set n [tell $fd]
        if {$n < 512} {set n -$n} else {set n -512}
        seek $fd $n end

        set hdr [read $fd 512]
        set pos [string first "PK\05\06" $hdr]
        if {$pos == -1} {
            error "no header found"
        }
        set hdr [string range $hdr [expr $pos + 4] [expr $pos + 21]]
        set pos [expr [tell $fd] + $pos - 512]

        binary scan $hdr ssssiis \
            cb(ndisk) cb(cdisk) \
            cb(nitems) cb(ntotal) \
            cb(csize) cb(coff) \
            cb(comment)

        set cb(ndisk)   [u_short $cb(ndisk)]
        set cb(nitems)   [u_short $cb(nitems)]
        set cb(ntotal)   [u_short $cb(ntotal)]
        set cb(comment)   [u_short $cb(comment)]

        # Compute base for situations where ZIP file
        # has been appended to another media (e.g. EXE)
        set cb(base)   [expr { $pos - $cb(csize) - $cb(coff) }]
    }

    proc zip::TOC {fd arr} {
        upvar 1 $arr sb

        set buf [read $fd 46]

        binary scan $buf A4ssssssiiisssssii hdr \
          sb(vem) sb(ver) sb(flags) sb(method) time date \
          sb(crc) sb(csize) sb(size) \
          flen elen clen sb(disk) sb(attr) \
          sb(atx) sb(ino)

        if { ![string equal "PK\01\02" $hdr] } {
            binary scan $hdr H* x
            error "bad central header: $x"
        }

        foreach v {vem ver flags method disk attr} {
            set cb($v) [u_short [set sb($v)]]
        }

        set sb(mtime) [DosTime $date $time]
        set sb(mode) [expr { ($sb(atx) >> 16) & 0xffff }]
        if { ( $sb(atx) & 0xff ) & 16 } {
            set sb(type) directory
        } else {
            set sb(type) file
        }
        set sb(name) [read $fd [u_short $flen]]
        set sb(extra) [read $fd [u_short $elen]]
        set sb(comment) [read $fd [u_short $clen]]
    }

    proc zip::open {path} {
        set fd [::open $path]
        
        if {[catch {
            upvar #0 zip::$fd cb
            upvar #0 zip::$fd.toc toc

            fconfigure $fd -translation binary ;#-buffering none
            
            zip::EndOfArchive $fd cb

            seek $fd $cb(coff) start

            set toc(_) 0; unset toc(_); #MakeArray
            
            for { set i 0 } { $i < $cb(nitems) } { incr i } {
                zip::TOC $fd sb
                
                set sb(depth) [llength [file split $sb(name)]]
                
                set name [string tolower $sb(name)]
                set toc($name) [array get sb]
                FAKEDIR toc [file dirname $name]
            }
        } err]} {
            close $fd
            return -code error $err
        }

        return $fd
    }

    proc zip::FAKEDIR {arr path} {
        upvar 1 $arr toc

        if { $path == "."} { return }


        if { ![info exists toc($path)] } {
            # Implicit directory
            lappend toc($path) \
                    name $path \
                    type directory mtime 0 size 0 mode 0777 \
                    ino -1 depth [llength [file split $path]]
        }
        FAKEDIR toc [file dirname $path]
    }

    proc zip::exists {fd path} {
        #::vfs::log "$fd $path"
        if {$path == ""} {
            return 1
        } else {
            upvar #0 zip::$fd.toc toc
            info exists toc([string tolower $path])
        }
    }

    proc zip::stat {fd path arr} {
        upvar #0 zip::$fd.toc toc
        upvar 1 $arr sb

        set name [string tolower $path]
        if { $name == "" || $name == "." } {
            array set sb {
                type directory mtime 0 size 0 mode 0777 
                ino -1 depth 0 name ""
            }
        } elseif {![info exists toc($name)] } {
            return -code error "could not read \"$path\": no such file or directory"
        } else {
            array set sb $toc($name)
        }
        set sb(dev) -1
        set sb(uid)   -1
        set sb(gid)   -1
        set sb(nlink) 1
        set sb(atime) $sb(mtime)
        set sb(ctime) $sb(mtime)
        return ""
    }

    # Treats empty pattern as asking for a particular file only
    proc zip::getdir {fd path {pat *}} {
        #::vfs::log [list getdir $fd $path $pat]
        upvar #0 zip::$fd.toc toc

        if { $path == "." || $path == "" } {
            set path [string tolower $pat]
        } else {
            set path [string tolower $path]
            if {$pat != ""} {
                append path /[string tolower $pat]
            }
        }
        set depth [llength [file split $path]]

        #puts stderr "getdir $fd $path $depth $pat [array names toc $path]"
        if {$depth} {
            set ret {}
            foreach key [array names toc $path] {
                if {[string index $key end] == "/"} {
                    # Directories are listed twice: both with and without
                    # the trailing '/', so we ignore the one with
                    continue
                }
                array set sb $toc($key)

                if { $sb(depth) == $depth } {
                    if {[info exists toc(${key}/)]} {
                        array set sb $toc(${key}/)
                    }
                    lappend ret [file tail $sb(name)]
                } else {
                    #::vfs::log "$sb(depth) vs $depth for $sb(name)"
                }
                unset sb
            }
            return $ret
        } else {
            # just the 'root' of the zip archive.  This obviously exists and
            # is a directory.
            return [list {}]
        }
    }

    proc zip::_close {fd} {
        variable $fd
        variable $fd.toc
        unset $fd
        unset $fd.toc
        ::close $fd
    }
}

set _dummy {} ;# to avoid printing the zip code above when sourcing this file


