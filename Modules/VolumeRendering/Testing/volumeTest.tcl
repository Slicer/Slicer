#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: is3d.tcl,v $
#   Date:      $Date: 2006/06/04 01:27:12 $
#   Version:   $Revision: 1.17 $
# 
#===============================================================================
# FILE:        is3d.tcl
# PROCEDURES:  
#   is3d::dtor d
#   is3d_demo volume
#   is3d_demo_kw the
#   is3d_demo_kw_spgr
#   is3d_demo_kw_face
#   is3d_demo_movie filebase steps
#==========================================================================auto=

# TODO - won't be needed once iSlicer is a package
package require Iwidgets

#########################################################
#
if {0} { ;# comment

is3d - a widget for looking at Slicer scenes in 3d

currently volume rendering

# TODO : 
    - almost everything
    

}
#
#########################################################

#
# Default resources
# - sets the default colors for the widget components
#
option add *is3d.background #000000 widgetDefault
option add *is3d.longitude 0 widgetDefault
option add *is3d.latitude 0 widgetDefault
option add *is3d.distance 800 widgetDefault
option add *is3d.focalpoint "0 0 0" widgetDefault
option add *is3d.up "z" widgetDefault
option add *is3d.colorscheme "gray" widgetDefault
option add *is3d.rotationgain "1." widgetDefault
option add *is3d.scalegain "1." widgetDefault
option add *is3d.pangain "1." widgetDefault
option add *is3d.linked3d "" widgetDefault
option add *is3d.linkedexpose "" widgetDefault

#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class is3d] == "" } {

    itcl::class is3d {
      inherit iwidgets::Labeledwidget

      constructor {args} {}
      destructor {}

      # another iswidget (TODO - attach to a scene description rather than a single volume)
      public variable isvolume

      #
      # itk_options for widget options that may need to be
      # inherited or composed as part of other widgets
      # or become part of the option database
      #
      itk_option define -background background Background {#000000}
      itk_option define -longitude longitude Longitude {0}
      itk_option define -latitude latitude Latitude {0}
      itk_option define -distance distance Distance {800}
      itk_option define -focalpoint focalpoint Focalpoint {0 0 0}
      itk_option define -up up Up {z}
      itk_option define -colorscheme colorscheme Colorscheme {gray}
      itk_option define -rotationgain rotationgain Rotationgain {1}
      itk_option define -scalegain scalegain Scalegain {1}
      itk_option define -pangain pangain Pangain {1}
      itk_option define -linked3d linked3d Linked3d {}
      itk_option define -linkedexpose linkedexpose Linkedexpose {}

      # widgets for the control area
      variable _controls

      # state variables for interaction
      variable _dragstate ""
      variable _dragpos ""
      variable _draglong ""
      variable _draglat ""
      variable _dragfocal ""
      variable _dragdist ""
      variable _render_pending 0

      # vtk objects in the render
      variable _name
      variable _tkrw
      variable _ren
      variable _mapper
      variable _actor

      # volume rendering vtk objects
      variable _cast
      variable _flip
      variable _opaxfer
      variable _gradxfer
      variable _colxfer
      variable _volprop
      variable _compfunc
      variable _volmapper
      variable _vol

      # methods
      method expose {} {} ;# queues a render for idle time
      method render {} {} ;# immediate mode rendering
      method winresize {} {}
      method bindings { {action "set"} } {}
      method pickcb {cmd x y} {}
      method dragcb {state x y} {}
      method actor {} {return $_actor}
      method mapper {} {return $_mapper}
      method ren {} {return $_ren}
      method tkrw {} {return $_tkrw}
      method rw {} {return [$_tkrw GetRenderWindow]}
      method camera {} {return [$_ren GetActiveCamera]}
      method volprop {} {return $_volprop}
      method controls {} { return $_controls } 
      method longlatdist { {long 0} {lat 0} {dist 0} } {}
      method screensave { filename {imagetype "PNM"}} {} ;# TODO should be moved to superclass
    }
}


# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body is3d::constructor {args} {
    component hull configure -borderwidth 0


    # make a unique name associated with this object
    set _name [namespace tail $this]
    # remove dots from name so it can be used in widget names
    regsub -all {\.} $_name "_" _name

    #
    # build the controls
    # - TODO - split controls into separate class as it gets more complex
    #

    set _controls $itk_interior.controls_$_name
    ::iwidgets::labeledframe $_controls -labeltext "Controls"
    pack $_controls -side bottom -expand false -fill x
    set cs [$_controls childsite]
    
    #
    # build the vtk 3d viewer
    #
    set _tkrw $itk_interior.tkrw
    vtkTkRenderWidget $_tkrw -width 512 -height 512

    pack $_tkrw -expand true -fill both
    bind $_tkrw <Expose> "$this expose"
    bind $_tkrw <Configure> "$this winresize"

    set _ren ::ren_$_name
    set _mapper ::mapper_$_name
    set _actor ::actor_$_name
    catch "$_ren Delete"
    catch "$_mapper Delete"
    catch "$_actor Delete"

    vtkRenderer $_ren
    [$this rw] AddRenderer $_ren

    $this bindings 

    #
    # Initialize the widget based on the command line options.
    #
    eval itk_initialize $args
}


itcl::body is3d::destructor {} {
    destroy $_tkrw 
    catch "$_ren Delete"
    if { [info exists _cast] } {
        catch "$_cast Delete"
        catch "$_flip Delete"
        catch "$_gradxfer Delete"
        catch "$_colxfer Delete"
        catch "$_volprop Delete"
        catch "$_compfunc Delete"
        catch "$_volmapper Delete"
        catch "$_vol Delete"
    }
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

#-------------------------------------------------------------------------------
# OPTION: -background
#
# DESCRIPTION: background color of the image viewer
#-------------------------------------------------------------------------------
itcl::configbody is3d::background {

  if {$itk_option(-background) == ""} {
    return
  }

  set scanned [scan $itk_option(-background) "#%02x%02x%02x" r g b]

  if { $scanned == 3 } {
      $_ren SetBackground [expr ($r/255.)] [expr ($g/255.)] [expr ($b/255.)]
      $this expose
  }

}

#-------------------------------------------------------------------------------
# OPTION: -longitude
#
# DESCRIPTION: rotate around the interior-superior axis
#-------------------------------------------------------------------------------
itcl::configbody is3d::longitude {
    $this longlatdist $itk_option(-longitude) $itk_option(-latitude) $itk_option(-distance) 
}

#-------------------------------------------------------------------------------
# OPTION: -latitude
#
# DESCRIPTION: rotate from the equator toward the north pole
#-------------------------------------------------------------------------------
itcl::configbody is3d::latitude {
    $this longlatdist $itk_option(-longitude) $itk_option(-latitude) $itk_option(-distance) 
}

#-------------------------------------------------------------------------------
# OPTION: -distance
#
# DESCRIPTION: distance from FocalPoint for long/lat view
#-------------------------------------------------------------------------------
itcl::configbody is3d::distance {
    $this longlatdist $itk_option(-longitude) $itk_option(-latitude) $itk_option(-distance) 
}

#-------------------------------------------------------------------------------
# OPTION: -colorscheme
#
# DESCRIPTION: pre-canned rendering parameters
#-------------------------------------------------------------------------------
itcl::configbody is3d::colorscheme {

    if { ! [info exists _opaxfer] } {
        return
    }

    switch $itk_option(-colorscheme) {
        "gray" {
            # Halazar's suggestions:
            $_opaxfer RemoveAllPoints
            $_opaxfer AddPoint  0   0.0
            $_opaxfer AddPoint  10   0.0
            $_opaxfer AddPoint  20   0.8
            $_opaxfer AddPoint  40   1.0

            $_colxfer RemoveAllPoints
            $_colxfer AddRGBPoint      0.0  0.0 0.0 0.0
            $_colxfer AddRGBPoint      4.0  0.5 0.5 0.5
            $_colxfer AddRGBPoint     20.0  1.0 1.0 1.0

            $_gradxfer RemoveAllPoints
            $_gradxfer AddPoint  0   0.0
            $_gradxfer AddPoint  10   0.5
            $_gradxfer AddPoint  30   1.0

            #$_opaxfer RemoveAllPoints
            #$_opaxfer AddPoint  0   0.0
            #$_opaxfer AddPoint  20   0.0
            #$_opaxfer AddPoint  20   0.5
            #$_opaxfer AddPoint  65   0.8
            #$_opaxfer AddPoint  100   0.9
            #$_opaxfer AddPoint  30   1.0

            #$_colxfer RemoveAllPoints
            #$_colxfer AddRGBPoint      0.0 0.0 0.0 0.0
            #$_colxfer AddRGBPoint     10.0 0.5 0.5 0.5
            #$_colxfer AddRGBPoint     10.0 1.0 1.0 1.0
            #$_colxfer AddRGBPoint     128.0 1.0 1.0 1.0
        }
        "noisygray" {
            $_opaxfer RemoveAllPoints
            $_opaxfer AddPoint  0   0.0
            $_opaxfer AddPoint  160   0.0
            $_opaxfer AddPoint  170   0.8
            $_opaxfer AddPoint  180   1.0

            $_colxfer RemoveAllPoints
            $_colxfer AddRGBPoint      0.0  0.0 0.0 0.0
            $_colxfer AddRGBPoint      4.0  0.5 0.5 0.5
            $_colxfer AddRGBPoint     20.0  1.0 1.0 1.0

            $_gradxfer RemoveAllPoints
            $_gradxfer AddPoint  0   0.0
            $_gradxfer AddPoint  10   0.5
            $_gradxfer AddPoint  30   1.0
        }
        "default" {
            $_opaxfer RemoveAllPoints
            $_opaxfer AddPoint  20   0.0
            $_opaxfer AddPoint  255  0.2

            $_colxfer RemoveAllPoints
            $_colxfer AddRGBPoint      0.0 0.0 0.0 0.0
            $_colxfer AddRGBPoint     64.0 1.0 0.0 0.0
            $_colxfer AddRGBPoint    128.0 0.0 0.0 1.0
            $_colxfer AddRGBPoint    192.0 0.0 1.0 0.0
            $_colxfer AddRGBPoint    255.0 0.0 0.2 0.0

            $_gradxfer RemoveAllPoints
            $_gradxfer AddPoint  0   0.0
            $_gradxfer AddPoint  10   0.5
            $_gradxfer AddPoint  30   1.0
        }
    }
}

itcl::configbody is3d::isvolume {

    if { $isvolume == "" } {
        return
    }

    set _cast ::cast_$_name
    set _flip ::flip_$_name
    set _opaxfer ::opaxfer_$_name
    set _gradxfer ::gradxfer_$_name
    set _colxfer ::colxfer_$_name
    set _volprop ::volprop_$_name
    set _compfunc ::compfunc_$_name
    set _volmapper ::volmapper_$_name
    set _vol ::vol_$_name
    catch "$_cast Delete"
    catch "$_flip Delete"
    catch "$_opaxfer Delete"
    catch "$_gradxfer Delete"
    catch "$_colxfer Delete"
    catch "$_volprop Delete"
    catch "$_compfunc Delete"
    catch "$_volmapper Delete"
    catch "$_vol Delete"

    # volume rendering only works on char
    vtkImageFlip $_flip
    #$_flip SetInput [$isvolume imagedata]
    set ee [vtkImageEllipsoidSource New]
    $_flip SetInput [$ee GetOutput]
    $_flip SetFilteredAxis 0

    vtkImageCast $_cast
    $_cast SetInput [$_flip GetOutput]
    $_cast SetOutputScalarTypeToUnsignedChar
    
    # Create transfer mapping scalar value to opacity
    vtkPiecewiseFunction $_opaxfer

    # Create transfer mapping gradient to opacity
    vtkPiecewiseFunction $_gradxfer

    # Create transfer mapping scalar value to color
    vtkColorTransferFunction $_colxfer

    $this configure -colorscheme "gray"

    # The property describes how the data will look
    vtkVolumeProperty $_volprop
    $_volprop SetColor $_colxfer
    $_volprop SetScalarOpacity $_opaxfer
    $_volprop SetGradientOpacity $_gradxfer
    $_volprop ShadeOn
    $_volprop SetInterpolationTypeToLinear

    # The mapper / ray cast function know how to render the data
    vtkVolumeRayCastCompositeFunction  $_compfunc
    catch "__vtkversion Delete"
    vtkVersion __vtkversion
    if { [__vtkversion GetVTKMajorVersion] <= 4 } {
        vtkVolumeRayCastMapper $_volmapper
        $_volmapper SetVolumeRayCastFunction $_compfunc
        $_volmapper SetSampleDistance 0.5
    } else {
        # only do this branch on vtk5
        #vtkVolumeTextureMapper2D $_volmapper
        vtkVolumeTextureMapper3D $_volmapper
    }
    __vtkversion Delete
    $_volmapper SetInput [$_cast GetOutput]

    # The volume holds the mapper and the property and
    # can be used to position/orient the volume
    vtkVolume $_vol
    $_vol SetMapper $_volmapper
    $_vol SetProperty $_volprop

    $_ren AddVolume $_vol
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------


itcl::body is3d::expose {} {

    if { $_render_pending } {
        return
    }
    after idle "$this render"
    set _render_pending 1
    foreach is3d $itk_option(-linked3d) {
        if { $is3d != $this } {
            $is3d expose
        }
    }
    foreach is3d $itk_option(-linkedexpose) {
        if { $is3d != $this } {
            $is3d expose
        }
    }
}

itcl::body is3d::render {} {

    set viewWin [$_tkrw GetRenderWindow]
    set rens [$viewWin GetRenderers]
    set rencount [$rens GetNumberOfItems] 
    for {set r 0} {$r < $rencount} {incr r} {
        set ren [$rens GetItemAsObject $r]
        $ren ResetCameraClippingRange    
    }

    $_tkrw Render
    set _render_pending 0
}

itcl::body is3d::winresize {} {
    global View

    # don't have the output extent change with the window size
    return;

}

itcl::body is3d::bindings { {action "set"} } {

    switch $action {
        "set" {
            #
            # mouse bindings
            #

            bind $_tkrw <Control-ButtonPress-1> "$this pickcb pick %x %y"
            bind $_tkrw <Control-B1-Motion> "$this pickcb pick %x %y"
            bind $_tkrw <Control-ButtonRelease-1> "$this pickcb noop %x %y"

            bind $_tkrw <ButtonPress-1> "$this dragcb rotstart %x %y"
            bind $_tkrw <B1-Motion> "$this dragcb rot %x %y"
            bind $_tkrw <ButtonRelease-1> "$this dragcb rotend %x %y"

            bind $_tkrw <ButtonPress-2> "$this dragcb panstart %x %y"
            bind $_tkrw <B2-Motion> "$this dragcb pan %x %y"
            bind $_tkrw <ButtonRelease-2> "$this dragcb panend %x %y"

            bind $_tkrw <ButtonPress-3> "$this dragcb scalestart %x %y"
            bind $_tkrw <B3-Motion> "$this dragcb scale %x %y"
            bind $_tkrw <ButtonRelease-3> "$this dragcb scaleend %x %y"

            #
            # key bindings
            # TODO - key bindings only work on toplevel windows?
            #

            bind $_tkrw r "$this dragcb reset"
        }
        "clear" {
            #
            # mouse bindings
            #
            bind $_tkrw <Control-ButtonPress-1> ""
            bind $_tkrw <Control-B1-Motion> ""
            bind $_tkrw <Control-ButtonRelease-1> ""

            bind $_tkrw <ButtonPress-1> ""
            bind $_tkrw <B1-Motion> ""
            bind $_tkrw <ButtonRelease-1> ""

            bind $_tkrw <ButtonPress-2> ""
            bind $_tkrw <B2-Motion> ""
            bind $_tkrw <ButtonRelease-2> ""

            bind $_tkrw <ButtonPress-3> ""
            bind $_tkrw <B3-Motion> ""
            bind $_tkrw <ButtonRelease-3> ""

            #
            # key bindings
            # TODO - key bindings only work on toplevel windows?
            #

            bind $_tkrw r ""
        }
    }
}

itcl::body is3d::pickcb {cmd x y} {

if {0} {
    if {$cmd == "noop"} return
    set size [$_ren GetSize]
    set yy [expr [lindex $size 1] - $y]
    set _probestring "Probe value: --"
    if {[$_picker Pick $x $yy 0 $_ren]} {

        set data [$_picker GetDataSet]
        set cid [$_picker GetCellId]
        set cell [$data GetCell $cid]
        set nopts [$cell GetNumberOfPoints]
        set pdata [$data GetPointData]
        set scalars [$pdata GetScalars]
        if {$scalars != "" && $nopts == 3} {
            set value 0.0
            if {$nopts != 3} {error "non triangular element"}
            set pc [$_picker GetPCoords]
            set weights(0) [expr 1. - [lindex $pc 0] - [lindex $pc 1]]
            set weights(1) [lindex $pc 0]
            set weights(2) [lindex $pc 1]
            for {set pt 0} {$pt < $nopts} {incr pt} {
                set pp [$cell GetPointId $pt]
                set value [expr $value + $weights($pt) * [$scalars GetScalar $pp]]
            }

            set _probestring [format "Probe value: %.2f" $value]
        } 
        $cell Delete
    }
}

}

itcl::body is3d::dragcb {state x y} {

    switch -- $state {
        "rotstart" {
            set _dragpos "$x $y"
            set _dragstate "rot"
            set _draglong $itk_option(-longitude)
            set _draglat $itk_option(-latitude)
        }
        "rot" {
            set _dragstate "rot"
            set long [expr $_draglong + \
                ($x - [lindex $_dragpos 0]) * $itk_option(-rotationgain)]
            set lat [expr $_draglat + \
                ($y - [lindex $_dragpos 1]) * $itk_option(-rotationgain)]
            if { $lat  < -89.9 } {set lat  -89.9}
            if { $lat  >  89.9 } {set lat   89.9}

            $this configure -longitude $long -latitude $lat
        }
        "rotend" {
            set _dragstate ""
        }
        "panstart" {
            set _dragpos "$x $y"
            set _dragstate "pan"
            set _dragfocal $itk_option(-focalpoint)
        }
        "pan" {
            set _dragstate "pan"
            set focalx [lindex $_dragfocal 0]
            set focaly [lindex $_dragfocal 1]
            set focalz [lindex $_dragfocal 2]
            set xx [expr $focalx + \
                ($x - [lindex $_dragpos 0]) * $itk_option(-pangain)]
            set zz [expr $focalz + \
                ($y - [lindex $_dragpos 1]) * $itk_option(-pangain)]

            $this configure -focalpoint "$xx $focaly $zz"
        }
        "panend" {
            set _dragstate ""
        }
        "scalestart" {
            set _dragpos "$x $y"
            set _dragstate "scale"
            set _dragdist $itk_option(-distance) 
        }
        "scale" {
            set _dragstate "scale"
            set dist [expr $_dragdist - ($y - [lindex $_dragpos 1]) * $itk_option(-scalegain)]
            if { $dist < 0.1 } {set dist 0.1}
            $this configure -distance $dist
        }
        "scaleend" {
            set _dragstate ""
        }
        "reset" {
            $this configure -longitude 0 -latitude 0 -distance 800 -focalpoint "0 0 0"
        }
    }
    $this expose
    
    foreach is3d $itk_option(-linked3d) {
        if { $is3d != $this } {
            $is3d configure -longitude $itk_option(-longitude) -latitude $itk_option(-latitude) -distance $itk_option(-distance) -focalpoint $itk_option(-focalpoint)
            $is3d expose
        }
    }
}

#-------------------------------------------------------------------------------
# .PROC is3d::dtor
# Return the conversion of the input to radians
# .ARGS
# int d degrees
# .END
#-------------------------------------------------------------------------------
proc is3d::dtor {d} {
    # pi rad = 180 deg => rad/deg = pi/180
    return [expr $d * 3.14159 / 180.]
}

itcl::body is3d::longlatdist { {long 0} {lat 0} {dist 0} } {

    # (0 1 0) is looking down nose 

    set long [is3d::dtor $long]
    set lat [is3d::dtor $lat]

    set x [expr $dist * sin($long) * cos($lat)]
    set y [expr $dist * cos($long) * cos($lat)]
    set z [expr $dist * sin($lat)]

    eval [$this camera] SetFocalPoint $itk_option(-focalpoint)
    switch $itk_option(-up) {
      "y" {
        [$this camera] SetPosition [expr -1 * $x] $z $y
        [$this camera] SetViewUp 0 1 0
      }
      default -
      "z" {
        [$this camera] SetPosition $x $y $z
        [$this camera] SetViewUp 0 0 1
      }
    }
}

itcl::body is3d::screensave { filename {imagetype "PNM"} } {
# TODO should be moved to superclass

    set wif ::wif_$_name
    set imgw ::imgw_$_name
    catch "$wif Delete"
    catch "$imgw Delete"

    vtkWindowToImageFilter $wif 
    $wif SetInput [[$this tkrw] GetRenderWindow]

    switch $imagetype {
        "PNM" - "PPM" {
            vtkPNMWriter $imgw 
        }
        "JPG" - "JPEG" {
            vtkJPEGWriter $imgw 
        }
        "BMP" {
            vtkBMPWriter $imgw 
        }
        "PS" - "PostScript" - "postscript" {
            vtkPostScriptWriter $imgw 
        }
        "TIF" - "TIFF" {
            vtkTIFFWriter $imgw 
        }
        "PNG" {
            vtkPNGWriter $imgw 
        }
        default {
            error "unknown image format $imagetype; options are PNM, JPG, BMP, PS, TIFF, PNG"
        }
    }
        
    $imgw SetInput [$wif GetOutput]
    $imgw SetFileName $filename
    $imgw Write

    $imgw Delete
    $wif Delete
} 

#-------------------------------------------------------------------------------
# .PROC is3d_demo
# 
# .ARGS
# binary volume 
# .END
#-------------------------------------------------------------------------------
proc is3d_demo { {volume "off"} } {

    catch "destroy .is3ddemo"
    toplevel .is3ddemo
    wm title .is3ddemo "is3d demo"
    wm geometry .is3ddemo +30+50

    pack [is3d .is3ddemo.is3d] -fill both -expand true
}

#-------------------------------------------------------------------------------
# .PROC is3d_demo_kw
# 
# .ARGS
# Run the demo using data not loaded from slicer's data model
# .END
#-------------------------------------------------------------------------------
proc is3d_demo_kw { } {

    package require kwwidgets

    catch "app Delete"
    vtkKWApplication app

    catch "win Delete"
    vtkKWWindow win
    app AddWindow win
    win SetSecondaryPanelVisibility 0
    win Create

    catch "vol_panel Delete"
    vtkKWUserInterfacePanel vol_panel
    vol_panel SetName "Volume Interface"
    vol_panel SetUserInterfaceManager [win GetMainUserInterfaceManager]
    vol_panel Create

    vol_panel AddPage "Properties" "Volume Properties" ""
    set page_widget [vol_panel GetPageWidget "Properties"]

    foreach p "1 2 3" {
        vol_panel AddPage "Page$p" "Info About Page $p" ""
        set page_$p [vol_panel GetPageWidget "Page$p"]
    }

    catch "vpw Delete"
    vtkKWVolumePropertyWidget vpw
    vpw SetParent $page_widget
    vpw Create
    
    pack [vpw GetWidgetName] -side left -anchor nw -expand y -padx 2 -pady 2

    catch "vsplit Delete"
    vtkKWSplitFrame vsplit
    vsplit SetParent [win GetViewPanelFrame]
    vsplit SetExpandableFrameToBothFrames
    vsplit SetOrientationToHorizontal
    vsplit Create

    pack [vsplit GetWidgetName] -expand true -fill both -padx 0 -pady 0


    catch "ellip Delete"
    vtkImageEllipsoidSource ellip
    ellip SetWholeExtent 0 255  0 255 0 255

    set f1 [[vsplit GetFrame1] GetWidgetName]
    set f2 [[vsplit GetFrame2] GetWidgetName]

    #pack [isvolume $f1.isv] -fill both -expand true
    #$f1.isv configure -resolution 128 
    #$f1.isv configure -volume [ellip GetOutput]
    #$f1.isv configure -orientation axial


    pack [is3d $f2.is3d] -fill both -expand true

    $f2.is3d configure -isvolume $f1.isv

    vpw SetVolumeProperty [$f2.is3d volprop]
    vpw SetVolumePropertyChangedCommand "" "$f2.is3d expose"

    app Start

}

#-------------------------------------------------------------------------------
# .PROC is3d_demo_kw_spgr
# 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc is3d_demo_kw_spgr {} {
    package require vtkITK
    catch "nr Delete"
    vtkNRRDReader nr
    nr SetFileName d:/data/namic/huva00024864/HUVA00024864_spgr.nhdr
    nr Update

    set f1 [[vsplit GetFrame1] GetWidgetName]
    set f2 [[vsplit GetFrame2] GetWidgetName]
    $f1.isv configure -volume [nr GetOutput]
    $f1.isv configure -orientation AP
    $f2.is3d expose
}

#-------------------------------------------------------------------------------
# .PROC is3d_demo_kw_face
# 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc is3d_demo_kw_face {} {
    catch "dcmr Delete"
    vtkDICOMImageReader dcmr
    dcmr SetDirectoryName d:/data/pieper-face-2005-05-11/1.2.840.113619.2.135.3596.6358736.5118.1115807980.182.uid/0000$series.SER/
    dcmr Update

    set f1 [[vsplit GetFrame1] GetWidgetName]
    set f2 [[vsplit GetFrame2] GetWidgetName]
    $f1.isv configure -volume [dcmr GetOutput]
    $f1.isv configure -orientation AP
    $f2.is3d expose
}

#-------------------------------------------------------------------------------
# .PROC is3d_demo_movie
# 
# .ARGS
# path filebase
# int steps
# .END
#-------------------------------------------------------------------------------
proc is3d_demo_movie { filebase {steps 10} } {

    raise .is3ddemo

    set delta [expr 360. / $steps]

    set f 1
    for {set l 0} {$l <= 360} { set l [expr $l + $delta] } {
        puts -nonewline "$f..." ; flush stdout
        .is3ddemo.is3d configure -longitude $l
        .is3ddemo.is3d expose
        update
        #.is3ddemo.is3d screensave [format c:/tmp/is%04d.ppm $f]
        .is3ddemo.is3d screensave [format ${filebase}%04d.png $f] PNG
        incr f
    }

    return

    puts ""
    puts "encoding..."

    set ret [catch "exec c:/apps/cygwin/usr/local/bin/ffmpeg.exe -i c:/tmp/is%04d.ppm -y $filename" res]
    puts $res

    puts "deleting..."
    for {set ff 1} {$ff <= $f} {incr ff} {
        file delete [format c:/tmp/is%04d.ppm $ff]
    }
    puts "done."
}


