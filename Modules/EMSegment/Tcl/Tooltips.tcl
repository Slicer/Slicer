#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: Tooltips.tcl,v $
#   Date:      $Date: 2006/01/06 17:57:05 $
#   Version:   $Revision: 1.13 $
# 
#===============================================================================
# FILE:        Tooltips.tcl
# PROCEDURES:  
#   TooltipAdd widget tip
#   TooltipEnterWidget
#   TooltipExitWidget
#   TooltipPopUp
#   TooltipPopDown
#   TooltipDisable
#   TooltipEnable
#   TooltipToggle
#==========================================================================auto=


# default to disabled tooltips when this file is first sourced
set Tooltips(enabled) 0

#-------------------------------------------------------------------------------
# .PROC TooltipAdd
# Call this procedure to add a tooltip (floating help text) to a widget. 
# The tooltip will pop up over the widget when the user leaves the mouse
# over the widget for a little while.
# .ARGS
# str widget name of the widget
# str tip text that you would like to appear as a tooltip.
# .END
#-------------------------------------------------------------------------------

proc TooltipAdd {this widget tip} {


    # surround the tip string with brackets
    set tip "\{$tip\}"

    # bindings
    bind $widget <Enter> "$this _TooltipEnterWidget %W $tip %X %Y"
    bind $widget <Leave> "$this _TooltipExitWidget"


    # The following are fixes to make buttons work right with tooltips...

    # put the class (i.e. Button) first in the bindtags so it executes earlier
    # (this makes button highlighting work)
    # just swap the first two list elements
    set btags [bindtags $widget]
    if {[llength $btags] > 1} {
    set class [lindex $btags 1]
    set btags [lreplace $btags 1 1 [lindex $btags 0]]
    set btags [lreplace $btags 0 0 $class]
    }
    bindtags $widget $btags

    # if the button is pressed, this should be like a Leave event
    # (otherwise the tooltip will come up incorrectly)
    if {$class == "Button" || $class == "Radiobutton"} {
    set cmd [$widget cget -command]
    set cmd "TooltipExitWidget; $cmd"
    }
}


#-------------------------------------------------------------------------------
# .PROC TooltipEnterWidget
# Internal procedure for Tooltips.tcl.  Called when the mouse enters the widget.
# This proc works with TooltipExitWidget to pop up the tooltip after a delay.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipEnterWidget {widget tip X Y} {
    global Tooltips


    # do nothing if tooltips disabled
    if {$Tooltips(enabled) == 0} {
    return
    }

    # We are over the widget
    set Tooltips(stillOverWidget) 1

    # reset Tooltips(stillOverWidget) after a delay (to end the "vwait")
    set id [after 500 \
        {if {$Tooltips(stillOverWidget) == 1} {set Tooltips(stillOverWidget) 1}}]

    # wait until Tooltips(stillOverWidget) is set (by us or by exiting the widget).
    # "vwait" allows event loop to be entered (but using an "after" does not)
    vwait Tooltips(stillOverWidget)

    # if Tooltips(stillOverWidget) is 1, the mouse is still over widget.
    # So pop up the tooltip!
    if {$Tooltips(stillOverWidget) == 1} {
    TooltipPopUp $widget $tip $X $Y
    } else {
    # the mouse exited the widget already, so cancel the waiting.
    after cancel $id
    }

}

#-------------------------------------------------------------------------------
# .PROC TooltipExitWidget
# Internal procedure for Tooltips.tcl.  Called when the mouse exits the widget. 
# This proc works with TooltipEnterWidget to pop up the tooltip after a delay.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipExitWidget {} {
    global Tooltips

    # mouse is not over the widget anymore, so stop the vwait.
    set Tooltips(stillOverWidget) 0

    # remove the tooltip if there is one.
    TooltipPopDown
}

#-------------------------------------------------------------------------------
# .PROC TooltipPopUp
# Internal procedure for Tooltips.tcl.  Causes the tooltip window to appear. 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipPopUp {widget tip X Y} {
    global Tooltips Gui

    # set tooltip window name
    if {[info exists Tooltips(window)] == 0} {
    set Tooltips(window) .wTooltips
    }

    # get rid of any other existing tooltip
    TooltipPopDown

    # make a new tooltip window
    toplevel $Tooltips(window)

    # add an offset to make tooltips fall below cursor
    set Y [expr $Y+15]

    # don't let the window manager "reparent" the tip window
    wm overrideredirect $Tooltips(window) 1

    # display the tip text...
    wm geometry $Tooltips(window) +${X}+${Y}
    eval {label $Tooltips(window).l -text $tip } $Gui(WTTA)
    pack $Tooltips(window).l -in $Tooltips(window)
}

#-------------------------------------------------------------------------------
# .PROC TooltipPopDown
# Internal procedure for Tooltips.tcl.  Removes the tooltip window. 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipPopDown {} {
    global Tooltips

    catch {destroy $Tooltips(window)}
}

#-------------------------------------------------------------------------------
# .PROC TooltipDisable
# Turn off display of tooltips
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipDisable {} {
    global Tooltips

    # get rid of any other existing tooltip
    TooltipPopDown

    # disable tooltips
    set Tooltips(enabled) 0
}

#-------------------------------------------------------------------------------
# .PROC TooltipEnable
# Turn on display of tooltips
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipEnable {} {
    global Tooltips

    # disable tooltips
    set Tooltips(enabled) 1
}

#-------------------------------------------------------------------------------
# .PROC TooltipToggle
# Toggle tooltip display on/off
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc TooltipToggle {} {
    global Tooltips

    if {$Tooltips(enabled) == 1} {
    set Tooltips(enabled) 0
    } else {
    set Tooltips(enabled) 1
    } 
}

