#!/bin/sh
# the next line restarts using tclsh \
    exec wish "$0" "$@"


#
# ExternalProgress.tcl
#
# This program is invoked by the following method in the vtkSlicerApplicationGUI:
#
#    // Description:
#    // Use a helper process to display a progress message and progress bar.
#    // Need to use this for feedback when a normal KWWidgets progress bar
#    // would cause event loops (since it calls the tk 'update' command
#    // to force display)
#    void vtkSlicerApplicationGUI::SetExternalProgress(char *message, float progress);
#
# this should be launched from within slicer as
# 
# $::env(TCL_DIR)/bin/wish8.4 $::env(SLICER_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl/ExternalProgress.tcl
#
# - see launch.tcl.in for how platform specific variations map to these environment variables
  
package require Tk

# the following test should be runnable from within the tkcon of a running
# slicer (build version or install version).  Cut and paste commands to test
proc progress_Test {} {
  set wish $::env(TCL_DIR)/bin/wish8.4 
  set script $::env(SLICER_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl/ExternalProgress.tcl
  set script $::env(SLICER_HOME)/../Slicer3/Base/GUI/Tcl/ExternalProgress.tcl
  set fp [open "| $wish" "w"]
  puts $fp "source $script"; flush $fp
  for {set prog 0} {$prog < 100} {incr prog 1} {
    puts $fp "progress_Window 300x500+100+300 progtest $prog"; flush $fp
    after 50
  }
  #puts $fp "exit"; flush $fp
  #close $fp
}

#
# create the progress window if needed, move it and sized it as instructed
#
proc progress_Window {geometry message progress} {

  if { ![winfo exists .progress] } {

    # need to create the window and other one-time items

    set progfont [font create -family Helvetica -size 10]
    set progfontb [font create -family Helvetica -size 10 -weight bold]

    toplevel .progress -borderwidth 0 -bg white
    wm overrideredirect .progress 1

    label .progress.message -text $message -bg white -fg blue -font $progfont
    label .progress.progress -text $progress -bg white -fg blue -font $progfontb
    label .progress.pulse -text "*" -bg white -bg white -fg red -font $progfontb

    pack .progress.message .progress.progress .progress.pulse -side left
    
    set ::progress_StartTime [clock clicks -milliseconds]
    after 100 progress_Animate 100

    # make the window go away if no progress message arrives in 2 seconds
    # (a new one will be created if needed by a new message)
    after 2000 destroy .progress
  }

  wm withdraw .
  wm geometry .progress $geometry

  .progress.message configure -text $message
  .progress.progress configure -text "$progress %"
  if { [expr $progress == 0.] } {
    wm withdraw .progress
  } else {
    wm deiconify .progress
    raise .progress
  }
  update
}


proc progress_Animate { {delayms 100} } {

  if { [winfo exists .progress] } {

    set ellapsed [expr [clock clicks -milliseconds] - $::progress_StartTime]
    set iteration [expr ($ellapsed / $delayms) % 20]
    if { $iteration > 10 } {
      set iteration [expr 10 - ($iteration - 10)]
    }

    set hex [format "%0.2x" [expr int (255 * ($iteration/10.))]]
    set color "#$hex$hex$hex"

    .progress.pulse configure -fg $color

    update
    after $delayms "progress_Animate $delayms"
  }
}

