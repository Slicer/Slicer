package provide app-SBuild 1.0
package require Tk
package require SBuildHelpers
package require SlicerBuildCommands

set LogLookup(0) Trace
set LogLookup(1) Debug
set LogLookup(2) Info
set LogLookup(3) Warning
set LogLookup(4) Error

proc Initialize {} {
  global SBuild tcl_platform Plugin
  # Setup some basics
  set SBuild(Plugins) ""
  set SBuild(CMakeLines) ""
  set SBuild(SlicerSourceDir) ""
  set SBuild(SlicerBuildDir) ""
  set SBuild(SlicerLibDir) ""
  set SBuild(LastDir) ""
  set SBuild(BuildType) Debug
  set SBuild(CMakeCXXFlagsDebug) "-g"
  set SBuild(Trace) 0
  set SBuild(Debug) 1
  set SBuild(Info) 2
  set SBuild(Log) 2
  set SBuild(Warning) 3
  set SBuild(Error) 4
  set SBuild(LogLevel) $SBuild(Info)
  set SBuild(CurrentTab) "Slicer3"
  foreach v { isSolaris isWindows isDarwin isLinux } { set SBuild($v) 0 }
  switch $tcl_platform(os) {
    "SunOS" { set SBuild(isSolaris) 1; set SBuild(Architecture) "SunOS" }
    "Linux" { set SBuild(isLinux) 1; set SBuild(Architecture) "Linux" }
    "Darwin" { set SBuild(isDarwin) 1; set SBuild(Architecture) "Darwin" }
    default { set SBuild(isWindows) 1; set SBuild(Architecture) "Windows" }
  }

  if {[info exists ::env(SVN)]} {
    set SBuild(SVNCommand) $::env(SVN)
  } else {
    set SBuild(SVNCommand) svn
  }
  set SBuild(Make) make
  
  LoadPreferences
}

proc BuildGUI {} {
    global SBuild

  # Build the menu
  set mb [menu .mb]
  . configure -menu $mb
  set fm [menu .filemenu]
  $mb add cascade -label File -menu $fm
  $fm add command -label "Print env" -command PrintEnv
  $fm add command -label "Save Preferences" -command SavePreferences

  set lm [menu .logmenu]
  $mb add cascade -label Log -menu $lm
  $lm add command -label Clear -command [list $::SBuild(Log) delete 0.0 end]
  $lm add separator
  foreach key [list Trace Debug Info Warning Error] {
    $lm add radiobutton -label $key -value $::SBuild($key) -variable ::SBuild(LogLevel)
  }

    # Build everything in the top level
    set logframe [::ttk::labelframe .loglabel -text "Build Log"]
    ::ttk::scrollbar $logframe.hsb -orient horizontal -command [list $logframe.text xview]
    ::ttk::scrollbar $logframe.vsb -orient vertical -command [list $logframe.text yview]
    set log [text .loglabel.text -xscrollcommand [list $logframe.hsb set] -yscrollcommand [list $logframe.vsb set]]
    set SBuild(Log) $log
  $log tag configure Warning -foreground red
  $log tag configure Error -foreground red -background yellow
    grid $logframe.text -row 0 -column 0 -stick nsew
    grid $logframe.text -row 0 -column 0 -sticky nsew
    grid $logframe.vsb -row 0 -column 1 -sticky nsew
    grid $logframe.hsb -row 1 -column 0 -sticky nsew
    grid columnconfigure $logframe 0 -weight 1
    grid rowconfigure $logframe 0 -weight 1
    
    grid $logframe -row 1 -column 0 -sticky nsew

    set nb [::ttk::notebook .book]
    grid $nb -row 0 -column 0 -sticky nsew

    # Build a page for the base, required and optional
    set base [::ttk::frame $nb.base]
    set f [::ttk::labelframe $base.dirframe -text "Directories"]
    
    grid [::ttk::label $f.w1 -text "Slicer Source Directory"] -row 0 -column 0
    grid [::ttk::entry $f.w2 -textvar SBuild(SlicerSourceDir)] -row 0 -column 1 -sticky ew
    grid [::ttk::button $f.w3 -text "..." -command [list ChooseDir "Slicer Source Directory" SlicerSourceDir]] -row 0 -column 2
    
    grid [::ttk::label $f.w11 -text "Slicer Build Directory"] -row 1 -column 0
    grid [::ttk::entry $f.w12 -textvar SBuild(SlicerBuildDir)] -row 1 -column 1 -sticky ew
    grid [::ttk::button $f.w13 -text "..." -command [list ChooseDir "Slicer Build Directory" SlicerBuildDir]] -row 1 -column 2

    grid [::ttk::label $f.w21 -text "Slicer Library Directory"] -row 2 -column 0
    grid [::ttk::entry $f.w22 -textvar SBuild(SlicerLibDir)] -row 2 -column 1 -sticky ew
    grid [::ttk::button $f.w23 -text "..." -command [list ChooseDir "Slicer Library Directory" SlicerLibDir]] -row 2 -column 2

    # Build type
    grid [::ttk::label $f.bt1 -text "Build Type"] -row 3 -column 0
    grid [::ttk::combobox $f.bt2 -textvariable SBuild(BuildType) -values [list Debug RelWithDebInfo Release]] -row 3 -column 1 -sticky w

  # Buttons
  set bf [::ttk::frame $f.buttonframe]
  grid $bf -row 4 -column 0 -columnspan 3

  pack [::ttk::button $bf.update -text "Update" -command Slicer-Update] -side left
  pack [::ttk::button $bf.configure -text "Configure" -command Slicer-Configure] -side left
  pack [::ttk::button $bf.build -text "Build" -command Slicer-Build] -side left
  pack [::ttk::button $bf.test -text "Test" -command Slicer-Test] -side left
  pack [::ttk::button $bf.all -text "All" -command Slicer-All] -side left

    grid columnconfigure $f 1 -weight 1
    
    grid $f -row 0 -column 0 -sticky ew
    grid columnconfigure $base 0 -weight 1
    
    $nb add $base -text "Slicer3"
    
    set required [::ttk::frame $nb.required]
    $nb add $required -text "Required Libs"
    BuildRequiredGUI $required

    set optional [::ttk::frame $nb.optional]
    $nb add $optional -text "Optional Libs"
    BuildOptionalGUI $optional
    
    grid columnconfigure . 0 -weight 1
    grid rowconfigure . 1 -weight 1
  if { [info exists SBuild(Window,X)] } {
    wm geometry . =$SBuild(Window,Width)x$SBuild(Window,Height)+$SBuild(Window,X)+$SBuild(Window,Y)
  }
  bind . <Configure> WindowAdjusted
  catch { $nb select $SBuild(CurrentTab) }
  bind $nb <<NotebookTabChanged>> [list "set SBuild(CurrentTab) \[$nb select]" SavePreferences]
}

proc WindowAdjusted {} {
  global SBuild
  # Save position and location
  set SBuild(Window,X) [winfo x .]
  set SBuild(Window,Y) [winfo y .]
  set SBuild(Window,Height) [winfo height .]
  set SBuild(Window,Width) [winfo width .]
  after idle SavePreferences
}

proc Error { text } { Log $text $::SBuild(Error) }
proc Warning { text } { Log $text $::SBuild(Warning) }
proc Debug { text } { Log $text $::SBuild(Debug) }
proc Info { text } { Log $text $::SBuild(Info) }
proc Trace { text } { Log $text $::SBuild(Trace) }

proc Log { text {Level 2}} {
  global SBuild LogLookup
  puts $text
  if { $Level >= $SBuild(LogLevel) } {
    $SBuild(Log) insert end "$text\n" $LogLookup($Level)
    $SBuild(Log) see end
  }
}
    
proc SortPlugins {} {
    global SBuild Plugin
    set l ""
    foreach ns $SBuild(Plugins) {
        lappend l [list $ns $Plugin($ns,Order)]
    }
    set l [lsort -real -index 1 -unique $l]
    Debug $l
    set nl ""
    foreach f $l {
        lappend nl [lindex $f 0]
    }
    set SBuild(Plugins) $nl
}


proc UseExternalBuild { p f } {
  global SBuild Plugin
  set w1 [list $f.checkout-$p $f.configure-$p $f.build-$p] 
  set w2 [list $f.userpath-$p $f.choosepath-$p]
  if { $SBuild($p,UseExternalBuild) } {
    foreach w $w1 { $w configure -state disabled }
    foreach w $w2 { $w configure -state normal }
    if { $SBuild($p,ExternalBuildPath) == "" } {
      ChooseDir "External path for $p" $p,ExternalBuildPath
    }
    ExecutePluginMethod $p ConfigureExternal
  } else {
    foreach w $w1 { $w configure -state normal }
    foreach w $w2 { $w configure -state disabled }
  }
  SavePreferences
}

proc BuildRequiredGUI { f } {
  global SBuild Plugin
  # Find the required plugins
  SortPlugins
  set row 0
  foreach p $SBuild(Plugins) {
    if { $Plugin($p,Type) != "required" } { continue }
    set c -1
    # Build the gui
    grid [::ttk::label $f.l-$p -text $p] -row $row -column [incr c]
    grid [::ttk::button $f.checkout-$p -text "Update" -command [list ExecutePluginMethod $p "Update"]] -row $row -column [incr c]
    grid [::ttk::button $f.configure-$p -text "Configure" -command [list ExecutePluginMethod $p "Configure"]] -row $row -column [incr c]
    grid [::ttk::button $f.build-$p -text "Build" -command [list ExecutePluginMethod $p "Build"]] -row $row -column [incr c]

    # put a check box, entry and "..." button for a user defined version, if applicable
    if { [info exists Plugin($p,CanUseUserBuild)] && $Plugin($p,CanUseUserBuild) } {
      if { ![info exists SBuild($p,UseExternalBuild)] } { set SBuild($p,UseExternalBuild) 0; set SBuild($p,ExternalBuildPath) "" }
      grid [::ttk::checkbutton $f.userbuild-$p -text "Use external build" -command [list UseExternalBuild $p $f] -variable SBuild($p,UseExternalBuild)] -row $row -column [incr c]
      grid [::ttk::entry $f.userpath-$p -textvariable SBuild($p,ExternalBuildPath)] -row $row -column [incr c] -sticky ew
      grid columnconfigure $f $c -weight 1
      grid [::ttk::button $f.choosepath-$p -text "..." -command [list ChooseDir "External path for $p" $p,ExternalBuildPath]] -row $row -column [incr c]
      UseExternalBuild $p $f
    }
    incr row
    # Configure the plugin
    Debug "Setting up $p"
    ExecutePluginMethod $p "Setup"
  }
}

proc BuildOptionalGUI { f } {
  global SBuild Plugin
  SortPlugins
  set row 0
  foreach p $SBuild(Plugins) {
    if { $Plugin($p,Type) != "optional" } { continue }
    set c -1
    # Build the gui
    grid [::ttk::label $f.l-$p -text $p] -row $row -column [incr c]
    grid [::ttk::button $f.checkout-$p -text "Update" -command [list ExecutePluginMethod $p "Update"]] -row $row -column [incr c]
    grid [::ttk::button $f.configure-$p -text "Configure" -command [list ExecutePluginMethod $p Configure]] -row $row -column [incr c]
    grid [::ttk::button $f.build-$p -text "Build" -command [list ExecutePluginMethod Build $p]] -row $row -column [incr c]
    incr row
    # Configure the plugin
    ExecutePluginMethod $p "Setup"
  }
}



proc ChooseDir { t var } {
    global SBuild
    set Directory [tk_chooseDirectory -initialdir $SBuild(LastDir) -parent . -title $t]
    if { $Directory == "" } { return }
    if { [file exists $Directory] && ![file isdirectory $Directory] } {
        tk_messageBox -message "$Directory is a file, please choose again" -icon warning -type ok
        return ChooseDir $t $var
    }
    if { ![file exists $Directory] } {
        if { [tk_messageBox -message "Directory $Directory does not exist, Ok to create?" -icon question -type yesno] == "no" } {
            return
        }
    }
    set SBuild($var) $Directory
    set SBuild(LastDir) $Directory
    SavePreferences
}


Initialize
# Only now load our plugins...
set SBuild(Plugins) ""
package require SBuildPlugins
BuildGUI
ExecutePluginMethod Slicer PlatformConfigure
