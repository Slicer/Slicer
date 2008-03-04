package provide SBuildHelpers 1.0

proc FindFile { Queue filelist } {
  set Dir $Queue
  Debug "FindFile starting $Queue $filelist"
  # Look for a file in the list filelist
  while { [llength $Queue] != 0 } \
  {
    set Filename [lindex $Queue 0]
    set Queue [lrange $Queue 1 end]

    if { [file isdirectory $Filename] } \
    {
      set Status [catch { set Queue [concat $Queue [glob -nocomplain -- [file join $Filename *]]] } Result]
      continue;
    }
    foreach f $filelist {
      Trace "FindFile: matching $Filename and $f" 
      set idx [lsearch -glob [file tail $Filename] $f]
      if { $idx != -1 } {
        Debug "Returning $Filename"
        return $Filename
      }
    }
  }
  set msg "Failed to find \"$filelist\" in $Dir"
  tk_messageBox -parent . -title "Failed to find file" -message $msg -type ok -icon error
  Error $msg
}

proc ExecutePluginMethod { plugin m } {
  global SBuild Plugin
  if { $SBuild(SlicerLibDir) == "" } { return }
  file mkdir $SBuild(SlicerLibDir)
  cd $SBuild(SlicerLibDir)
  if { [info command $plugin-$m-$SBuild(Architecture)] != "" } {
    Debug "Executing $plugin-$m-$SBuild(Architecture)"
    return [$plugin-$m-$SBuild(Architecture)]
  }
  return [$plugin-$m]
}

proc Update { plugin } {
  return [ExecutePluginMethod $plugin Update]
}
    
proc LoadPreferences {} {
  global SBuild env

  if { ![file exists [file join $env(HOME) .SBuildPrefs] ] } {
    SavePreferences
  }
  
  set fid [open [file join $env(HOME) .SBuildPrefs] r]
  array set tmp [gets $fid]
  foreach n [array names tmp] {
    set SBuild($n) $tmp($n)
  }
  close $fid
}

proc SavePreferences {} {
  global SBuild env
  catch {  set SBuild(CurrentTab) [$SBuild(Notebook) select] }
  set fid [open [file join $env(HOME) .SBuildPrefs] "w"]
  puts $fid [array get SBuild]
  close $fid
}


proc CollectBackground { fid ID Logger } {
  global Background Fetch
  if { [eof $fid] } {
    # Log "ExecuteInBackground completed for $ID"
    set Background(Completed,$ID) 1
    set Background(Status,$ID) [catch { close $fid } Result]
    set Background(CloseMessage,$ID) $Result
  } else {
    if { [gets $fid Line] != -1 } {
      $Logger $Line
      append Background(Output,$ID) "$Line\n"
    }
  }
}

proc ExecuteCommand { args } { return [ExecuteInBackground $args] }
proc ExecuteInBackground { Command {Logger Log} } {
  # Start up a process, collect all the data, and return
  global Background

  set fid [open "| $Command" r]
  set ID [incr Background(Job)]
  Log "Job $ID: $Command in [pwd]"
  set Background(Output,$ID) ""
  set Background(Completed,$ID) 0
  fileevent $fid readable [list CollectBackground $fid $ID $Logger]
  vwait Background(Completed,$ID)

  set Output $Background(Output,$ID)
  unset Background(Output,$ID)
  set Status $Background(Status,$ID)
  if { $Status != 0 } {
    Error "Job Completed $Command with Status $Status\n$Background(CloseMessage,$ID)"
  } else {
    Info "Job Completed $Command"
  }
  return [list $Output $Status]
}
  


proc PrintEnv {} {
  global SBuild
  foreach key [lsort [array names SBuild]] {
    Log "SBuild($key) : $SBuild($key)"
  }
}
