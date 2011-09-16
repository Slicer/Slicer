#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: Gui.tcl,v $
#   Date:      $Date: 2006/03/06 19:22:49 $
#   Version:   $Revision: 1.57 $
# 
#===============================================================================
# FILE:        Gui.tcl
# PROCEDURES:  
#   GuiInit
#   GuiApplyStyle stylenames args
#   ValidateFloat var
#   ValidateName s
#   ValidateInt s
#   InfoWidget f
#   MailWidget f
#   MsgPopup win x y msg title font
#   YesNoPopup win x y msg yesCmd noCmd justify title font
#   ShowPopup w x y
#   MakeVTKImageWindow name input,
#   ExposeTkImageViewer widget x y w h
#   MakeVTKObject shape name
#   ScrollSet scrollbar geoCmd offset size
#   MakeColor str
#   MakeColorNormalized str
#   ColorSlider widget rgb
#   ExpandPath filename
#   Uncap s
#   Cap s
#   WaitWindow text x y
#   WaitPopup text
#   WaitDestroy
#   IsInt str
#   tkHorizontalLine name args
#   tkVerticalLine name args
#   tkSpace name args
#==========================================================================auto=

#-------------------------------------------------------------------------------
# .PROC GuiInit
#
# Initialise global variables for this module. Defined Gui strings for a uniform
# interface:<br>
# Tab Attributes (TA)<br>
# Workspace Button Attributes (WBA)<br>
# Workspace Menu-Button Attributes (WMBA)<br>
# Workspace Menu Attributes (WMA)<br>
# Workspace Label Attributes (WLA)<br>
# Workspace Frame Attributes (WFA)<br>
# Workspace Title Attributes (WTA)<br>
# Workspace Entry Attributes (WEA)<br>
# Workspace Entry Disabled Attributes (WEDA)<br>
# Workspace Checkbox Attributes (WCA)<br>
# Workspace Radiobutton Attributes (WRA)<br>
# Workspace Scale Attributes (WSA)<br>
# Workspace Tooltip Attributes (WTTA)<br>
# Workspace Pad Attributes (WPA)<br>
# Backdrop Label Attributes (BLA)<br>
# Backdrop Scale Attributes (BSA)<br>
# System Menu Attributes (SMA)<br>
# Workspace Scrollbar Attributes (WSBA)<br>
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GuiInit {} {
    global Gui Path tcl_platform SLICER

    # enable tooltips by default.  This should check user preferences somehow.
    #TooltipEnable

    # Are we running under Windows?
    set Gui(pc) [string match $tcl_platform(platform) "windows"]
    set Gui(linux) [string match $tcl_platform(os) "Linux"]
    set Gui(mac) [string match $tcl_platform(os) "Darwin"]
    # Else we assume we're under Solaris

    # 
    # set standard font scaling independent of screen size (since some UI elements
    # have hard-coded sizes, this should even out differences among platforms)
    # This overrides a dots-per-inch calculation done by Tk at startup
    #
    tk scaling 1.25
    set Gui(smallFont) 1

    if {$Gui(pc) == 0} {
        set Gui(xViewer) 365
    } else {
        set Gui(xViewer) 365
    }

    set Gui(progressText) "Working..."
    set Gui(waitWin) ""
    set Gui(title) "Slicer 3"
    
    # INIT
    set Gui(waitSemaphore) 0
    set Gui(magDim) 175
    set Gui(borderWidth) 2
    set Gui(minShort) -32768
    set Gui(maxShort) 32767
    set Gui(pad) 5
    set Gui(unsavedDag) 0

    # COLORS

    set Gui(darkGray)     " 81  81  81"
    set Gui(mediumGray)   "132 132 132"
    set Gui(lightGray)    "181 181 181"
    set Gui(darkCream)    "250 250 250"
    set Gui(lightCream)   "239 239 239"
    set Gui(brightCream)  "255 255 255"
    set Gui(black)        "  0   0   0"
    set Gui(white)        "255 255 255"
    set Gui(red)          "200 150 150"
    set Gui(saturatedRed) "150   0   0"
    set Gui(green)        "150 200 150"
    set Gui(yellow)       "220 220 100"
    set Gui(lightYellow)  "250 250 200"

    set Gui(backdrop)          [MakeColor $Gui(mediumGray)]
    set Gui(inactiveWorkspace) [MakeColor $Gui(white)]
    set Gui(activeWorkspace)   [MakeColor $Gui(darkCream)]
    # set Gui(activeWorkspace)   [MakeColor $Gui(white)]
    set Gui(normalButton)      [MakeColor $Gui(lightCream)]
    set Gui(activeButton)      [MakeColor $Gui(brightCream)]
    set Gui(indicatorColor)    [MakeColor $Gui(brightCream)]
    set Gui(textDark)          [MakeColor $Gui(black)]
    set Gui(textLight)         [MakeColor $Gui(white)]
    set Gui(textDisabled)      [MakeColor $Gui(mediumGray)]
    set Gui(slice0)            [MakeColor $Gui(red)]
    set Gui(slice1)            [MakeColor $Gui(yellow)]
    set Gui(slice2)            [MakeColor $Gui(green)]
    set Gui(toolTip)           [MakeColor $Gui(lightYellow)]

    # ATTRIBUTES
    set attr ""

    # Tab Attributes (TA)
    lappend attr TA
    set Gui(TA) { -font {helvetica 8}\
        -bg $Gui(backdrop) -fg $Gui(textLight) \
        -activebackground $Gui(backdrop) -activeforeground $Gui(textLight) \
        -bd 0 -padx 0 -pady 2 -relief flat \
        -highlightthickness 0 \
        -cursor hand2}

    # Workspace Button Attributes (WBA)
    lappend attr WBA 
    set Gui(WBA) { -font {helvetica 8}\
        -bg $Gui(normalButton) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -activeforeground $Gui(textDark) \
        -bd $Gui(borderWidth) -padx 0 -pady 0 -relief raised \
        -highlightthickness 0 \
        -cursor hand2}

    # Workspace Menu-Button Attributes (WMBA)
    lappend attr WMBA 
    set Gui(WMBA) { -font {helvetica 8}\
        -bg $Gui(normalButton) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -activeforeground $Gui(textDark) \
        -bd $Gui(borderWidth) -padx 0 -pady 0 -relief raised \
        -highlightthickness 0 \
        -cursor hand2}

    # Workspace Menu Attributes (WMA)
    lappend attr WMA 
    set Gui(WMA) { -font {helvetica 8}\
        -bg $Gui(normalButton) -fg $Gui(textDark) -activeborderwidth 2 \
        -activebackground $Gui(activeButton) -activeforeground $Gui(textDark) \
        -bd $Gui(borderWidth) -tearoff 0}

    # Workspace Label Attributes (WLA)
    lappend attr WLA 
    set Gui(WLA) { -font {helvetica 8}\
        -bg $Gui(activeWorkspace) -fg $Gui(textDark) \
        -bd 0 -padx 1 -pady 1 -relief flat }

    # Workspace Frame Attributes (WFA)
    lappend attr WFA 
    set Gui(WFA) {-bg $Gui(activeWorkspace)}

    # Workspace Title Attributes (WTA)
    lappend attr WTA 
    set Gui(WTA) {-font {helvetica 8 bold} \
        -bg $Gui(activeWorkspace) -fg $Gui(textDark) \
        -bd 0 -padx 1 -pady 1 -relief flat }

    # Workspace Entry Attributes (WEA)
    lappend attr WEA 
    set Gui(WEA) { -font {helvetica 8}\
        -bg $Gui(normalButton) -fg $Gui(textDark) \
        -highlightthickness 0 \
        -bd $Gui(borderWidth) -relief sunken}

    # Workspace Entry Disabled Attributes (WEDA)
    lappend attr WEDA 
    set Gui(WEDA) { -font {helvetica 8}\
        -bg $Gui(normalButton) -fg $Gui(textDisabled) \
        -highlightthickness 0 \
        -bd $Gui(borderWidth) -relief sunken}

    # Workspace Checkbox Attributes (WCA)
    lappend attr WCA 
    set Gui(WCA) { -font {helvetica 8}\
        -bg $Gui(activeWorkspace) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -highlightthickness 0 \
        -bd $Gui(borderWidth) -selectcolor $Gui(indicatorColor) \
        -padx 0 -pady 0 -relief flat}

    # Workspace Radiobutton Attributes (WRA)
    lappend attr WRA 
    set Gui(WRA) { -font {helvetica 8}\
        -bg $Gui(activeWorkspace) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -highlightthickness 0 \
        -selectcolor $Gui(indicatorColor) \
        -bd $Gui(borderWidth) -relief flat}

    # Workspace Scale Attributes (WSA)
    lappend attr WSA 
    set Gui(WSA) { -font {helvetica 8}\
        -bg $Gui(activeWorkspace) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -troughcolor $Gui(normalButton) \
        -highlightthickness 0 -orient horizontal -showvalue 0 -sliderlength 24 \
        -bd $Gui(borderWidth) -relief flat}

    # Workspace Tooltip Attributes (WTTA)
    lappend attr WTTA 
    set Gui(WTTA) { -font {helvetica 8}\
        -bg $Gui(toolTip) -fg $Gui(textDark) \
        -bd 2 -padx 2 -pady 2 -relief raised }

    # Workspace Pad Attributes (WPA)
    lappend attr WPA
    set Gui(WPA) {-padx $Gui(pad) -pady $Gui(pad)}

    # Backdrop Label Attributes (BLA)
    lappend attr BLA 
    set Gui(BLA) { -font {helvetica 8}\
        -bg $Gui(backdrop) -fg $Gui(textLight) \
        -bd 0 -padx 1 -pady 1 -relief flat }

    # Backdrop Scale Attributes (BSA)
    lappend attr BSA 
    set Gui(BSA) { -font {helvetica 8}\
        -bg $Gui(backdrop) -fg $Gui(textDark) \
        -activebackground $Gui(activeButton) -troughcolor $Gui(normalButton) \
        -highlightthickness 0 -orient vertical -showvalue 0 -sliderlength 24 \
        -bd $Gui(borderWidth) -relief flat}

    # System Menu Attributes (SMA)
    lappend attr SMA 
    set Gui(SMA) { -tearoff 0}

    # Workspace Scrollbar Attributes (WSBA)
    lappend attr WSBA 
    set Gui(WSBA) { -bg $Gui(activeWorkspace) \
        -activebackground $Gui(activeButton) -troughcolor $Gui(normalButton) \
        -highlightthickness 0 -bd $Gui(borderWidth) -relief flat}
}

#-------------------------------------------------------------------------------
# .PROC GuiApplyStyle
# Apply a Gui style to a list of widgets
# 
# .ARGS
# list stylenames a list of stylenames to apply (in order)
# list args a list of widgets to which to apply the styles
# .END
#-------------------------------------------------------------------------------
proc GuiApplyStyle {stylenames args} {
    global Gui
    foreach widget $args {
        foreach style $stylenames {
            eval $widget config $Gui($style)
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC ValidateFloat
# Return 1 if valid float number, else 0
# 
# .ARGS
# float var the value to check
# .END
#-------------------------------------------------------------------------------
proc ValidateFloat { var } {
    return [expr {![ catch { expr $var + 1.0 } ]}] 
}

#-------------------------------------------------------------------------------
# .PROC ValidateName
# Return 1 if valid, else 0. Only allow a-z, A-Z, 0-9, underscore and dashes in
# a valid name.
# .ARGS
# str s the string to check
# .END
#-------------------------------------------------------------------------------
proc ValidateName {s} {
    return [regexp {^([a-zA-Z0-9_-]*)$} $s]
}

#-------------------------------------------------------------------------------
# .PROC ValidateInt
# Return 1 if valid, else 0. Only allow minus sign, and numbers 0-9.
# .ARGS
# str s the number to check.
# .END
#-------------------------------------------------------------------------------
proc ValidateInt {s} {
    return [expr {[regexp {^(-*[0-9][0-9]*)$} $s]}]
}

#-------------------------------------------------------------------------------
# .PROC InfoWidget
# Set up a text string and return it.
# .ARGS
# windowpath f the parent frame.
# .END
#-------------------------------------------------------------------------------
proc InfoWidget {f} {
    global Gui

    set t [text $f.t -height 6 -setgrid true -wrap word \
        -yscrollcommand "$f.sy set" -cursor arrow -insertontime 0]
    scrollbar $f.sy -orient vert -command "$f.t yview"

    pack $f.sy -side right -fill y
    pack $f.t -side left -fill both -expand true
    return $t
}

#-------------------------------------------------------------------------------
# .PROC MailWidget
# Set up a widget and return it.
# .ARGS
# windowpath f the parent window.
# .END
#-------------------------------------------------------------------------------
proc MailWidget {f} {
    frame $f
    pack $f -side top -fill both -expand true

    set t [text $f.t -setgrid true -wrap word -width 42 -height 14 \
        -yscrollcommand "$f.sy set"]
    scrollbar $f.sy -orient vert -command "$f.t yview"

    pack $f.sy -side right -fill y
    pack $f.t -side left -fill both -expand true
    return $t
}

#-------------------------------------------------------------------------------
# .PROC MsgPopup
# Calls YesNoPopup with default values.
# .ARGS
# str win Window Name
# int x positions on the screen
# int y positions on the screen
# str msg the message to display
# str title optional title of the window, defaults to empty string
# str font optional font information, defaults to empty string
# .END
#-------------------------------------------------------------------------------
proc MsgPopup {win x y msg {title ""} {font ""}} {
    YesNoPopup $win $x $y $msg "" "" left $title $font
}

#-------------------------------------------------------------------------------
# .PROC YesNoPopup
# 
# This pops up a Yes/No window.
# The user is allowed to focus on other windows
# before answering 
#
# .ARGS
# str win Window Name, almost irrelevant.
# int x positions on the screen
# int y positions on the screen
# str msg the message to display
# str yesCmd the command to perform if yes is chosen, defaults to empty string
# str noCmd the command to perform if no is chosen, defaults to empty string
# str justify optional justification setting, defaults to center
# str title  optional title of the window, defaults to empty string
# str font optional font information, defaults to empty string
# .END
#-------------------------------------------------------------------------------
proc YesNoPopup {win x y msg {yesCmd ""} {noCmd ""} \
    {justify center} {title ""} {font ""}} {
    global Gui

    set w .w$win
    if {[RaisePopup $w] == 1} {return}

    if {$title == ""} {
        set title $Gui(title)
    }
    CreatePopup $w $title $x $y 

    set f $w
    frame $f.fMsg  -bg $Gui(activeWorkspace) -bd 1 -relief raised
    frame $f.fBtns -bg $Gui(inactiveWorkspace)
    pack $f.fMsg $f.fBtns -side top -pady $Gui(pad) -padx $Gui(pad)
    
    set f $w.fMsg
    eval {label $f.l -justify $justify -text "$msg"} $Gui(WLA)
    if {$font != ""} { 
        $f.l config -font $font
    }

    pack $f.l -padx 5 -pady 5 

    set f $w.fBtns
    if {$yesCmd == ""} {
        eval {button $f.bOk -text "Ok" -width 4 -command "destroy $w"} $Gui(WBA)
        pack $f.bOk -side left -padx $Gui(pad)
    } else {
        eval {button $f.bYes -text "Yes" -width 4 \
            -command "destroy $w; $yesCmd"} $Gui(WBA)
        eval {button $f.bNo -text "No" -width 4 \
            -command "destroy $w; $noCmd"} $Gui(WBA)
        pack $f.bYes $f.bNo -side left -padx $Gui(pad)
    }
    RaisePopup $w
}

#-------------------------------------------------------------------------------
# RaisePopup
#
# If the window 'w' exists, then this procedure raises it, gives it the focus,
# and returns 1.  Else returns 0
#
# .ARGS
# windowpath w the window to raise.
# .END
#-------------------------------------------------------------------------------
proc RaisePopup {w} {
    if {[winfo exists $w] != 0} {
        raise $w
        focus $w
        wm deiconify $w
        return 1
    }
    return 0
}

#-------------------------------------------------------------------------------
# CreatePopup
#
# Create window 'w' at screen coordinates (x, y) with 'title'
#
# .ARGS
# windowpath w the window to create
# str title the title of the window
# int x horizontal display coordinate
# int y vertical display coordinate
# .END
#-------------------------------------------------------------------------------
proc CreatePopup {w title x y } {
    global Gui
    
    toplevel $w -class Dialog -bg $Gui(inactiveWorkspace)
    wm title $w $title
    wm iconname $w Dialog
    if {$Gui(pc) == "0"} {
        wm transient $w .
    }
    wm geometry $w +$x+$y
    focus $w
}

#-------------------------------------------------------------------------------
# .PROC ShowPopup
# Deiconify and place a window.
# 
# .ARGS
# windowpath w the window to display
# int x horizontal position of the window
# int y vertical position of the window
# .END
#-------------------------------------------------------------------------------
proc ShowPopup {w x y} {

    # The window must draw itself before we can see its size
    wm deiconify $w
    update idletasks

    set wWin [winfo width  $w]
    set hWin [winfo height $w]
    set wScr [winfo screenwidth  .]
    set hScr [winfo screenheight .]
    
    set xErr [expr $wScr - 30 - ($x + $wWin)]
    if {$xErr < 0} {
        set x [expr $x + $xErr]
    }
    set yErr [expr $hScr - 30 - ($y + $hWin)]
    if {$yErr < 0} {
        set y [expr $y + $yErr]
    }
    
    raise $w
    wm geometry $w +$x+$y
}

#-------------------------------------------------------------------------------
# .PROC MakeVTKImageWindow
# Builds the vtk image mapper, actor, imager and win.
# .ARGS
# str name prefix string to use when naming vtk variables
# str input, optional input to the vtk image mapper, defaults to empty string
# .END
#-------------------------------------------------------------------------------
proc MakeVTKImageWindow {name {input ""}} {
    global Gui

    catch "${name}Mapper Delete"
    catch "${name}Actor Delete"
    catch "${name}Imager Delete"
    catch "${name}Win Delete"

    vtkImageMapper ${name}Mapper
    ${name}Mapper SetColorWindow 255
    ${name}Mapper SetColorLevel 127.5

    if {$input != ""} {
        ${name}Mapper SetInput [$input GetOutput]
    }

    vtkActor2D ${name}Actor
        ${name}Actor SetMapper ${name}Mapper
    vtkRenderer ${name}Imager
        ${name}Imager AddActor2D ${name}Actor
    vtkRenderWindow ${name}Win
        ${name}Win AddRenderer ${name}Imager

    # This line prevents repainting the screen after each 2D 
    # actor is drawn.  It must be called before creating the window.
    ${name}Win DoubleBufferOn
}

#-------------------------------------------------------------------------------
# .PROC ExposeTkImageViewer
# Don't rerender if already rendering, but otherwise empty the queue of any other
# expose events and render the widget.
# .ARGS
# windowpath widget the window to render
# int x x position of the window
# int y y position of the winodw
# int w width of the window
# int h height of the window
# .END
#-------------------------------------------------------------------------------
proc ExposeTkImageViewer {widget x y w h} {

   # Do not render if we are already rendering
   if {[::vtk::get_widget_variable_value $widget Rendering] == 1} {
      return
   }

   # empty the queue of any other expose events
   ::vtk::set_widget_variable_value $widget Rendering 1
   update
   ::vtk::set_widget_variable_value $widget Rendering 0

   # ignore the region to redraw for now.
   $widget Render
}

#-------------------------------------------------------------------------------
# .PROC MakeVTKObject
# Create a souce, mapper, and actor and add it via MainAddActor.
# .ARGS
# str shape a valid name for a vtk"shape"Source
# str name prefix for vtk variables
# .END
#-------------------------------------------------------------------------------
proc MakeVTKObject {shape name} {
    global View

    vtk${shape}Source ${name}Source
    vtkPolyDataMapper ${name}Mapper
    ${name}Mapper SetInput [${name}Source GetOutput]
   
# Note: Immediate mode rendering is necessary for rendering the same model
# to more than one render window.  However, it greatly slows performance.

    vtkActor ${name}Actor
    ${name}Actor SetMapper ${name}Mapper
    [${name}Actor GetProperty] SetColor 1.0 0.0 0.0
    MainAddActor ${name}Actor 

}

#-------------------------------------------------------------------------------
# .PROC ScrollSet
# Set up a scrollbar.
# .ARGS
# windowpath scrollbar the path to the scrollbar to set 
# list geoCmd list of commands, the manager should be the zeroth element
# float offset scrollbar offset
# float size scrollbar size
# .END
#-------------------------------------------------------------------------------
proc ScrollSet {scrollbar geoCmd offset size} {
    if {$offset != 0.0 || $size != 1.0} {
        eval $geoCmd     ;
        $scrollbar set $offset $size
    } else {
        set manager [lindex $geoCmd 0]
        $manager forget $scrollbar ;
    }
}

#-------------------------------------------------------------------------------
# ScrolledListbox
# Create a scrolled list box.
# .ARGS
# windowpath frame to create and populate
# int xAlways is 1 if the x scrollbar should be always visible
# int yAlways is 1 if the y scrollbar should be always visible
# list args used to configure the scrolled list box
# .END
#-------------------------------------------------------------------------------
proc ScrolledListbox {f xAlways yAlways {args ""}} {
    global Gui
    
    frame $f -bg $Gui(activeWorkspace)
    if {$xAlways == 1 && $yAlways == 1} { 
        listbox $f.list -selectmode single \
            -xscrollcommand "$f.xscroll set" \
            -yscrollcommand "$f.yscroll set"
    
    } elseif {$xAlways == 1 && $yAlways == 0} { 
        listbox $f.list -selectmode single \
            -xscrollcommand "$f.xscroll set" \
            -yscrollcommand [list ScrollSet $f.yscroll \
                [list grid $f.yscroll -row 0 -column 1 -sticky ns]]

    } elseif {$xAlways == 0 && $yAlways == 1} { 
        listbox $f.list -selectmode single \
            -xscrollcommand [list ScrollSet $f.xscroll \
                [list grid $f.xscroll -row 1 -column 0 -sticky we]] \
            -yscrollcommand "$f.yscroll set"

    } else {
        listbox $f.list -selectmode single \
            -xscrollcommand [list ScrollSet $f.xscroll \
                [list grid $f.xscroll -row 1 -column 0 -sticky we]] \
            -yscrollcommand [list ScrollSet $f.yscroll \
                [list grid $f.yscroll -row 0 -column 1 -sticky ns]]
    }

    eval {$f.list configure \
        -font {helvetica 7 bold} \
        -bg $Gui(normalButton) -fg $Gui(textDark) \
        -selectbackground $Gui(activeButton) \
        -selectforeground $Gui(textDark) \
        -highlightthickness 0 -bd $Gui(borderWidth) \
        -relief sunken -selectborderwidth $Gui(borderWidth)}

    if {$args != ""} {
        eval {$f.list configure} $args
    }

    scrollbar $f.xscroll -orient horizontal \
        -command [list $f.list xview] \
        -bg $Gui(activeWorkspace) \
        -activebackground $Gui(activeButton) -troughcolor $Gui(normalButton) \
        -highlightthickness 0 -bd $Gui(borderWidth)
    scrollbar $f.yscroll -orient vertical \
        -command [list $f.list yview] \
        -bg $Gui(activeWorkspace) \
        -activebackground $Gui(activeButton) -troughcolor $Gui(normalButton) \
        -highlightthickness 0 -bd $Gui(borderWidth)

    grid $f.list $f.yscroll -sticky news
    grid $f.xscroll -sticky news
    grid rowconfigure $f 0 -weight 1
    grid columnconfigure $f 0 -weight 1
    return $f.list
}

#-------------------------------------------------------------------------------
# .PROC MakeColor
# Returns a formatted colour string, as 3 floats from 0-1
# .ARGS
# list str R, G, B as floats 0-1
# .END
#-------------------------------------------------------------------------------
proc MakeColor {str} {
    return [format "#%02x%02x%02x" \
    [format %.0f [lindex $str 0]] \
    [format %.0f [lindex $str 1]] \
    [format %.0f [lindex $str 2]]]
}

#-------------------------------------------------------------------------------
# .PROC MakeColorNormalized
# Returns a formatted colour string, as floats from 0-255.
# .ARGS
# list str R, G, B as floats between 0-1
# .END
#-------------------------------------------------------------------------------
proc MakeColorNormalized {str} {
    return [format "#%02x%02x%02x" \
    [format %.0f [expr [lindex $str 0] * 255.0]] \
    [format %.0f [expr [lindex $str 1] * 255.0]] \
    [format %.0f [expr [lindex $str 2] * 255.0]]]
}


#-------------------------------------------------------------------------------
# .PROC ColorSlider
# This proc is introduced to color the troughs of sliders.  It only
# reconfigures the color if the color will change, since otherwise
# there is a bug under Linux.  (Under Linux the slider gets into an
# infinite loop since configuring it calls the procedure bound to it,
# which generally configures it again.  This freezes the slicer. 
# This must be a bug in tcl/tk with configuring scale widgets.)
# .ARGS
# widget widget the name of the slider
# list rgb three integers, in "R G B" format
# .END
#-------------------------------------------------------------------------------
proc ColorSlider {widget rgb} {

    if {[winfo exists $widget] == 0} {
        if {$::Module(verbose)} {
            puts "Gui.tcl ColorSlider: widget $widget doesn't exist!"
        }
        return
    }
    set color [MakeColorNormalized $rgb]

    # ask the widget what color it is now (returns a descriptive list)
    set oldColorDescription [$widget config -troughcolor]

    foreach descrip  $oldColorDescription {
        # if the new color is the same as the old color, return
        if {$descrip == $color} { 
            return
        }
    }

    # if the color is different, color away!
    $widget config -troughcolor $color   
}

#-------------------------------------------------------------------------------
# .PROC ExpandPath
# If the filename doesn't exist as is, return the Path(program) var prepended to it.
# .ARGS
# filepath filename the file to test and possibly expand
# .END
#-------------------------------------------------------------------------------
proc ExpandPath {filename} {
    global Path

    set home $Path(program)
 
    if {[file exists $filename] == 1} {
        return $filename 
    } else {
        return [file join $home $filename]
    }
}

#-------------------------------------------------------------------------------
# .PROC Uncap
# Returns the input string with the first letter lower case.
# .ARGS
# str s the string to work on
# .END
#-------------------------------------------------------------------------------
proc Uncap {s} {
    set a [string tolower [string index $s 0]]
    set b [string range $s 1 [string length $s]]
    return $a$b
}

#-------------------------------------------------------------------------------
# .PROC Cap
# Returns the input string with the first letter upper case.
# .ARGS
# str s the string to work on
# .END
#-------------------------------------------------------------------------------
proc Cap {s} {
    set a [string toupper [string index $s 0]]
    set b [string range $s 1 [string length $s]]
    return $a$b
}

#-------------------------------------------------------------------------------
# .PROC WaitWindow
# Wait for a window to appear on screen.
# .ARGS
# str text text to display in the waiting window
# int x the horizontal location of the window
# int y the vertical location of the window
# .END
#-------------------------------------------------------------------------------
proc WaitWindow {text x y} {
    global Gui

    set tWait .waitWindow
    catch {destroy $tWait}
    
    toplevel $tWait -bg $Gui(backdrop) 
    wm geometry $tWait +$x+$y
    wm title $tWait "Wait..." 

    set f $tWait
    eval {label $f.lDesc -text "$text" \
        -width [expr [string length $text] + 2]} $Gui(BLA)
    pack $f.lDesc -padx 10 -pady 10

    tkwait visibility $tWait

    return $tWait
}

#-------------------------------------------------------------------------------
# .PROC WaitPopup
# Set up and display a wait window, incrementing the wait semaphore.
# .ARGS
# str text optional message, defaults to "Please wait..."
# .END
#-------------------------------------------------------------------------------
proc WaitPopup {{text "Please wait..."}} {
    global Gui

    incr Gui(waitSemaphore)
    if {$Gui(waitSemaphore) > 1} {return}

    set Gui(waitWin) [WaitWindow $text 150 500]
    raise $Gui(waitWin)
    update

    .tMain config -cursor watch
}

#-------------------------------------------------------------------------------
# .PROC WaitDestroy
# Destroy the wait window if the wait semaphore is zero.
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc WaitDestroy {} {
    global Gui

    set Gui(waitSemaphore) [expr $Gui(waitSemaphore) - 1]
    if {$Gui(waitSemaphore) > 0} {return}

    if {$Gui(waitWin) == ""} {return}
    destroy $Gui(waitWin)
    set Gui(waitWin) ""
    .tMain config -cursor arrow
}

#-------------------------------------------------------------------------------
# .PROC IsInt
# Returns one if the input string contains an integer (only minus and 0 to 9 allowed)
# .ARGS
# str str string to check
# .END
#-------------------------------------------------------------------------------
proc IsInt {str} {
    set len [string length $str]
    for {set i 0} {$i < $len} {incr i} {
        set c [string index $str $i]
        set n [string match {[0-9-]} $c]
        if {$n == 0} {return 0}
    }
    return 1
}

#-------------------------------------------------------------------------------
# .PROC tkHorizontalLine
# 
#  Draw a Tk horizontal line widget
# .ARGS
# str name name of the widget
# list args list of widget arguments
# .END
#-------------------------------------------------------------------------------
proc tkHorizontalLine {name args} {
    return [eval frame $name -class TkHorizontalLine -relief sunken -height 2 -borderwidth 2 $args]
}

#-------------------------------------------------------------------------------
# .PROC tkVerticalLine
# 
#  Draw a Tk vertical line widget
# .ARGS
# str name name of the widget
# list args list of widget arguments
# .END
#-------------------------------------------------------------------------------
proc tkVerticalLine {name args} {
    return [eval frame $name -class TkVerticalLine -relief sunken -width 2  -borderwidth 2 $args]
}

#-------------------------------------------------------------------------------
# .PROC tkSpace
# 
#  Draw a Tk space (an empty widget)
# .ARGS
# str name name of the widget
# list args list of widget arguments
# .END
#-------------------------------------------------------------------------------
proc tkSpace {name args} {
    return [eval frame $name -class TkSpace -relief flat $args]
}

