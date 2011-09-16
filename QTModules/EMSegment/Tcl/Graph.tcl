#=auto==========================================================================
#   Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
# 
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
# 
#   Program:   3D Slicer
#   Module:    $RCSfile: Graph.tcl,v $
#   Date:      $Date: 2006/11/20 18:18:59 $
#   Version:   $Revision: 1.12 $
# 
#===============================================================================
# FILE:        Graph.tcl
# PROCEDURES:  
#   GraphCreate varName path graphWidth graphHeight graphLUT graphBgRange graphCurveRad graphDim descrFlag title coordXmin coordXmax coordXsca coordYmin coordYmax coordYsca graphExtFlag whole
#   GraphCreateXLabels 
#   GraphCreateYLabels 
#   GraphCalculateScnScp
#   GraphChangeMenu
#   GraphInteractor
#   GraphUpdateValue
#   GraphRescaleAxis
#   GraphRender  varName path
#   GraphAddCurveRegion  varName path data color type ignore eg
#   GraphCreateGaussianCurveRegion  varDataCurve mean covariance order probability fct dimension Xmin Xmax Xlen Ymin Ymax Ylen
#   GraphCreateHistogramCurve  varDataCurve Volume Xmin Xmax Xlen
#   GraphRemoveCurve  varName path number
#   GraphCalcUnit  Min Max Length
#   GraphCalcInvUnit  Min Max Length
#   GraphHexToRGB  colorcode
#   GraphChangeBackgroundColor varName path number saturation val hue
#   GraphDelete  varName path
#   GraphCreateLine  varName path color radius
#   GraphSetLineCoordinates varName path id coordinates
#   GraphDeleteLine  varName path id
#   GraphTransformValueInCoordinate  varName path value axis
#   GraphCreateDisplayValues varName path values DisplayFlag LogFlag
#   GraphDeleteValueDisplay varName path
#==========================================================================auto=


#-------------------------------------------------------------------------------
# .PROC GraphCreate
# Call this procedure to create create a graph. This function will create 
# a canvas, which defines the functions necessary to display functions in 
# a graph.
# Structure of the vtkImage* filters:
# vtkImageCurveRegion - calcluates all the curves / region - high calculation cost 
# vtkImageGraph       - plots the graph                    - medium calculation cost
# vtkImageDrawObject  - adds objects like lines to the graph, only activated if graphExtFlag is set - low computational cost 
# .ARGS
# array  varName        This is typically the name of the module.
# widget path           Frame where canvas should go on 
# int    graphWidth     width of the graph (without coordinate system)
# int    graphHeight    height of the graph (without coordinate system)
# vtkIndirectLookupTable graphLUT Lookup table for the background - the default is a white background
# array  graphBgRange   Range for the background setting - graphLUT has to be nonuniform to be effective 
# int    graphCurveRad  Radius of the curve that plots the graph - only relevant in 1D case
# int    graphDim       Dimension of the graph ((1D or 2D)
# bool   descrFlag      should the graph have any coordinates/title or other desciption displayed (0 = OFF, 1 = ON)  
# str    title          The title of the graph
# int    coordXmin      Minimum value for X-coordinate
# int    coordXmax      Maximum value for X-coordinate
# int    coordXsca      Scalling on X-coordinate 
# int    coordYmin      Minimum value for Y-coordinate
# int    coordYmax      Maximum value for Y-coordinate
# int    coordYsca      Scalling on Y-coordinate 
# bool   graphExtFlag   Graph Extension - allows you to add lines and other objects to the graph without replotting
#                       the whole graph => very fast
# .END
#-------------------------------------------------------------------------------
proc GraphCreate {this varName path {graphWidth 140}  {graphHeight 50} {graphLUT ""} {graphBgRange "0 0"} {graphCurveRad 0} {graphDim 1} {descrFlag 0}\
                 {title ""} {coordXmin 0} {coordXmax 100} {coordXsca 50} {coordXformat "%d"} {coordYmin 0.0} {coordYmax 1.0} {coordYsca 0.5} \
         {coordYformat "%01.2f"} {graphExtFlag 0} {XLabelSca ""} {XTitle ""} {Xoffset 0}}  {
    global Gui
    # 1. Step : Set up structur 
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphCreate"
        return
    }
    # I do not know jet , if the user should be able to change the follwoing paramters
    # Define Border between canvas and coord system on the rigth and left /up and down

    # Width with coordinate system
    set localArray(Graph,$path,Xlen) $graphWidth
    set localArray(Graph,$path,Ylen) $graphHeight
    # Set some global variables  
    if {$XLabelSca != ""} {
    set coordXsca 1
        set coordXmin 0
    set coordXmax [expr [llength $XLabelSca] -1] 
    }  
    set localArray(Graph,$path,XLabelSca) $XLabelSca
    set localArray(Graph,$path,Xmin)      $coordXmin
    set localArray(Graph,$path,Xmax)      $coordXmax
    set localArray(Graph,$path,Xoffset)   $Xoffset
    set localArray(Graph,$path,descrFlag) $descrFlag
    set localArray(Graph,$path,Xsca)      $coordXsca
    set localArray(Graph,$path,XInvUnit)  [GraphCalcInvUnit $coordXmin $coordXmax $graphWidth $Xoffset]

    set localArray(Graph,$path,Xformat) $coordXformat
    set localArray(Graph,$path,Ysca) $coordYsca
    set localArray(Graph,$path,Ymin) $coordYmin
    set localArray(Graph,$path,Ymax) $coordYmax
    set localArray(Graph,$path,Yoffset) 0   
    if {$graphDim == 2} {
      set localArray(Graph,$path,YInvUnit)  [GraphCalcInvUnit $coordYmin $coordYmax  $graphHeight 0]
    }
    set localArray(Graph,$path,Yformat) $coordYformat
    # Currently only defined for 1 dimension
    set localArray(Graph,$path,Dimension) $graphDim

    # Number of curves displayed in the graph
    set localArray(Graph,$path,CurveIndex) -1

    # graphExtFlag 
    set localArray(Graph,$path,graphExtFlag) $graphExtFlag
 
    frame $path.fGraph -bg $Gui(activeWorkspace)
    pack $path.fGraph -side top -padx 0 -pady 0 

    GraphCalculateScnScp $varName $path 0 
    GraphCalculateScnScp $varName $path 1

    if {$descrFlag} {
      set localArray(Graph,$path,XboL) 50
      set localArray(Graph,$path,XboR) 30
      # This is a frame around the graph where nothing gets printed
      set localArray(Graph,$path,YboU) 10
      # Space for titel text 
      set localArray(Graph,$path,YboUadd) 15 
      if {$title != ""} { incr localArray(Graph,$path,YboU) $localArray(Graph,$path,YboUadd)}
      set localArray(Graph,$path,YboD) 12
      set localArray(Graph,$path,YboD) 22
    set localArray(Graph,$path,Font) "ArialBlack 12"


      set localArray(Graph,$path,YboDadd) 15 
      if {$XTitle != ""} { incr localArray(Graph,$path,YboD) $localArray(Graph,$path,YboDadd)}
      # Length of scalling lines 
      set localArray(Graph,$path,Xscl) 3
      set localArray(Graph,$path,Yscl) 3
      # Difference bettween Text and Scalling line on Axis 
      set localArray(Graph,$path,Xsct) 5
      set localArray(Graph,$path,Ysct) 12
    
      # Set Colors of coordinates 
      set localArray(Graph,$path,CoordColor) black
      canvas $path.fGraph.caYAxis -width $localArray(Graph,$path,XboL)\
        -height [expr $graphHeight + $localArray(Graph,$path,YboD)+$localArray(Graph,$path,YboU)]\
        -background $Gui(activeWorkspace) -border 0 -highlightthickness 0
      GraphInteractor $this $varName $path $path.fGraph.caYAxis
      frame $path.fGraph.fRight -bg $Gui(activeWorkspace)
      pack $path.fGraph.caYAxis $path.fGraph.fRight -side left -padx 0 -pady 0 

      set XLength [expr $graphWidth  + $localArray(Graph,$path,XboR)]
      canvas $path.fGraph.fRight.caTitle -width $XLength -height $localArray(Graph,$path,YboU)\
          -background $Gui(activeWorkspace) -border 0 -highlightthickness 0
      GraphInteractor $this $varName $path $path.fGraph.fRight.caTitle
      frame $path.fGraph.fRight.fMiddle -bg $Gui(activeWorkspace)
      canvas $path.fGraph.fRight.caXAxis -width $XLength -height $localArray(Graph,$path,YboD)\
          -background $Gui(activeWorkspace) -border 0 -highlightthickness 0

      if {$XTitle != ""} {
    set y [expr $localArray(Graph,$path,Xsct) + $localArray(Graph,$path,Xscl)]
        $path.fGraph.fRight.caXAxis create text [expr ($XLength - $localArray(Graph,$path,XboR))  /2]  [expr $localArray(Graph,$path,YboD) + $localArray(Graph,$path,Xsct) - $localArray(Graph,$path,YboDadd)]  -text $XTitle \
        -tag XAxisTitle -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor) 
      } 



      GraphInteractor $this $varName $path $path.fGraph.fRight.caXAxis
      pack $path.fGraph.fRight.caTitle $path.fGraph.fRight.fMiddle $path.fGraph.fRight.caXAxis -side top -padx 0 -pady 0 

      canvas $path.fGraph.fRight.fMiddle.caGraph -width $graphWidth -height $graphHeight\
          -background $Gui(activeWorkspace) -border 0 -highlightthickness 0
      canvas $path.fGraph.fRight.fMiddle.caRBorder -width $localArray(Graph,$path,XboR) -height $graphHeight\
          -background $Gui(activeWorkspace) -border 0 -highlightthickness 0
      GraphInteractor $this  $varName $path $path.fGraph.fRight.fMiddle.caRBorder
      pack $path.fGraph.fRight.fMiddle.caGraph $path.fGraph.fRight.fMiddle.caRBorder -side left -padx 0  -pady 0

      # Set path variables 
      set localArray(Graph,$path,caYAxis)   $path.fGraph.caYAxis
      set localArray(Graph,$path,caXAxis)   $path.fGraph.fRight.caXAxis
      set localArray(Graph,$path,caGraph)   $path.fGraph.fRight.fMiddle.caGraph 
      set localArray(Graph,$path,caTitle)   $path.fGraph.fRight.caTitle
      set localArray(Graph,$path,caRBorder) $path.fGraph.fRight.fMiddle.caRBorder
      # 2. Step:  Define a title if necessary 
      $localArray(Graph,$path,caTitle) create text [expr $graphWidth/2] [expr $localArray(Graph,$path,YboU) - $localArray(Graph,$path,YboUadd)  + $localArray(Graph,$path,Xsct)]\
          -text $title -tag Title -font $localArray(Graph,$path,Font) -fill $Gui(textDark)

      set localArray(Graph,$path,Title) $title

      # 3. Step:  Create coordinates
      $localArray(Graph,$path,caXAxis) create line 0 0 $graphWidth 0 -width 1 -tag Grayvalue -fill $localArray(Graph,$path,CoordColor)

      set x [expr $localArray(Graph,$path,XboL)-1]
      $localArray(Graph,$path,caYAxis) create line $x $localArray(Graph,$path,YboU) $x [expr $graphHeight + $localArray(Graph,$path,YboU) + $localArray(Graph,$path,Xscl)] -width 1 -tag Probability -fill $localArray(Graph,$path,CoordColor)
      # Scalers of X and Y Coordinate Axis
      # Right now only 1 Dim Graph defined 
      GraphCreateXLabels $varName $path
      GraphCreateYLabels $varName $path 

      # Kilian - Might want to include later - Define Log Gray Value Box 
      # EMSegmentCreateLogGrayValueDisplay 0 $EMSegment(SelVolList,VolumeList) $index
      # Kilian - Initialize value for 2D Graph
      # EMSegmentCreateGraphPixels $index $EMSegment(Graph,Xmin,$index) $EMSegment(Graph,Ymin,$index) $EMSegment(Graph,Xmax,$index) $EMSegment(Graph,Ymax,$index)
    } else {
      # Just create the graph itself 
      canvas $path.fGraph.caGraph -width $graphWidth -height $graphHeight\
          -background $Gui(activeWorkspace) -border 0 -highlightthickness 0
      pack $path.fGraph.caGraph -side top -padx 0  -pady 0
      # Set path variables 
      set localArray(Graph,$path,caGraph) $path.fGraph.caGraph 
    }
    # 4. Create Graph itself 
    # the follwoing line creates the filters histMapper and histWin
    # Function is defined in Gui.tcl
    set localArray(Graph,$path,hist) ${path}hist
    MakeVTKImageWindow $localArray(Graph,$path,hist)
 
    vtkImageGraph ${varName}(Graph,$path,vtkImageGraph)
    if {$graphLUT == ""} { 
      set graphLUT  [${varName}(Graph,$path,vtkImageGraph) CreateUniformIndirectLookupTable] 
      set localArray(Graph,$path,bgLUTDefault) 1
    } else {set localArray(Graph,$path,bgLUTDefault) 0}
    set localArray(Graph,$path,graphLUT) $graphLUT

    ${varName}(Graph,$path,vtkImageGraph) SetLookupTable    $graphLUT
    ${varName}(Graph,$path,vtkImageGraph) SetDataBackRange  [lindex $graphBgRange 0] [lindex $graphBgRange 1]
    ${varName}(Graph,$path,vtkImageGraph) SetDimension      $localArray(Graph,$path,Dimension)
    ${varName}(Graph,$path,vtkImageGraph) SetXlength        $localArray(Graph,$path,Xlen)
    ${varName}(Graph,$path,vtkImageGraph) SetYlength        $localArray(Graph,$path,Ylen)
    ${varName}(Graph,$path,vtkImageGraph) SetCurveThickness $graphCurveRad
    ${varName}(Graph,$path,vtkImageGraph) Update

    set hist $localArray(Graph,$path,hist)
    if {$localArray(Graph,$path,graphExtFlag)} {
    vtkImageDrawObjects ${varName}(Graph,$path,vtkImageDrawObjects)
    ${varName}(Graph,$path,vtkImageDrawObjects) SetInput [${varName}(Graph,$path,vtkImageGraph) GetOutput]
    ${varName}(Graph,$path,vtkImageDrawObjects) Update
    ${hist}Mapper SetInput [${varName}(Graph,$path,vtkImageDrawObjects) GetOutput]   
    } else {
    ${hist}Mapper SetInput [${varName}(Graph,$path,vtkImageGraph) GetOutput]   
    }
    vtkTkRenderWidget $localArray(Graph,$path,caGraph).fHist -rw ${hist}Win -width $graphWidth -height $graphHeight  
    bind $localArray(Graph,$path,caGraph).fHist <Expose>  "$localArray(this) _ExposeTkImageViewer %W %x %y %w %h"
    GraphInteractor $this $varName $path $localArray(Graph,$path,caGraph).fHist
    pack $localArray(Graph,$path,caGraph).fHist

    ${hist}Win Render
}

#-------------------------------------------------------------------------------
# .PROC GraphCreateXLabels 
# Creates the Lables for the Graph's X-Axis
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphCreateXLabels {varName path} {
    global Gui
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphCreateXLabels"
        return
    }
    set ca $localArray(Graph,$path,caXAxis)
    # Lables of X Coordinate Axis
    set y [expr $localArray(Graph,$path,Xsct) + $localArray(Graph,$path,Xscl)]

    if {$localArray(Graph,$path,Xoffset) == 0} {
    $ca create text -2 $y -text [format "$localArray(Graph,$path,Xformat)" $localArray(Graph,$path,Xmin)] \
        -tag XAxisTX0 -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)
    $localArray(Graph,$path,caYAxis) create text $localArray(Graph,$path,XboL) [expr $y + $localArray(Graph,$path,YboU) + $localArray(Graph,$path,Ylen)] \
        -text [format "$localArray(Graph,$path,Xformat)" $localArray(Graph,$path,Xmin)] \
        -tag XAxisTY0 -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)
    set start 1
    set OffLength 0.0
    } else { 
    set start 0
        set OffLength [expr $localArray(Graph,$path,Xscp) / 2.0]
    }
    for {set i $start} { $i <= $localArray(Graph,$path,Xscn)} {incr i} {
        set x [expr int($i*$localArray(Graph,$path,Xscp) + $OffLength )]
        # Shift it to the right - otherwise it wont be fully displayed ! 
        $ca create line $x 0 $x $localArray(Graph,$path,Xscl) -width 1 -fill $localArray(Graph,$path,CoordColor) -tag XAxisL$i 
    if {$localArray(Graph,$path,XLabelSca) == ""} {
        set labelText [format "$localArray(Graph,$path,Xformat)" [expr ($i * $localArray(Graph,$path,Xsca)) + $localArray(Graph,$path,Xmin)]]
    } else { set labelText [lindex $localArray(Graph,$path,XLabelSca) $i]}
        $ca create text $x $y -text $labelText  -tag XAxisT$i -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)
    } 
}
#-------------------------------------------------------------------------------
# .PROC GraphCreateYLabels 
# Creates the Lables for the Graph's Y-Axis
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphCreateYLabels {varName path} {
    global Gui
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphCreateXLabels"
        return
    }
    # Scalers of Y Coordinate Axis
    set ca $localArray(Graph,$path,caYAxis)
    set XboL [expr $localArray(Graph,$path,XboL) - 1]
    for {set i 0} { $i <= $localArray(Graph,$path,Yscn)} {incr i} {
      set y [expr $localArray(Graph,$path,YboU) + $localArray(Graph,$path,Ylen)  - int($i * $localArray(Graph,$path,Yscp) ) ] 
      $ca create line [expr $XboL - $localArray(Graph,$path,Yscl)] $y $XboL $y -width 1 -tag YAxisL$i -fill $localArray(Graph,$path,CoordColor) 

      # if you get an error message change output format of your graph 
      set TEXT [expr $i*$localArray(Graph,$path,Ysca) + $localArray(Graph,$path,Ymin)] 
      $ca create text [expr $XboL - $localArray(Graph,$path,Yscl) - $localArray(Graph,$path,Ysct)] \
            $y -text [format "$localArray(Graph,$path,Yformat)" $TEXT] -tag YAxisT$i -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)  
    }
}

#-------------------------------------------------------------------------------
# .PROC GraphCalculateScnScp
# It will calulate varName(Graph,$path,${AxisName}scn) and varName(Graph,$path,${AxisName}scp) 
# where AxisName = (axis > 0 : Y : X) 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphCalculateScnScp {varName path axis} {
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphCreate"
        return
    }
    if {$axis} {set Aname Y
    } else { set Aname X }
    if {$localArray(Graph,$path,${Aname}sca) > 0.0 } {
    # Number of scalling lines
    set localArray(Graph,$path,${Aname}scn) [expr int(double($localArray(Graph,$path,${Aname}max) - $localArray(Graph,$path,${Aname}min) + $localArray(Graph,$path,${Aname}offset)) / double($localArray(Graph,$path,${Aname}sca)))]
    } else {
    set localArray(Graph,$path,${Aname}scn) [expr int(double($localArray(Graph,$path,${Aname}max) - $localArray(Graph,$path,${Aname}min) + $localArray(Graph,$path,${Aname}offset)))]
    }
    # Distance between scalling lines 
    set dist [expr double($localArray(Graph,$path,${Aname}max) - $localArray(Graph,$path,${Aname}min) + $localArray(Graph,$path,${Aname}offset))]
    if {$dist > 0.0} {
    set localArray(Graph,$path,${Aname}scp) [expr $localArray(Graph,$path,${Aname}len)*$localArray(Graph,$path,${Aname}sca) / $dist] 
    } else {
    set localArray(Graph,$path,${Aname}scp) $localArray(Graph,$path,${Aname}len)
    }
}

#-------------------------------------------------------------------------------
# .PROC  GraphChangeMenu
# Menue to change paramters 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphChangeMenu {this varName path widget xpos ypos} {
    global Gui Graph
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphChangeMenu"
        return
    }
    
    set w .wGraphChangeMenu
    set Gui(wGraphChangeMenu) $w

    if {[winfo exists $Gui(wGraphChangeMenu)]} {
       destroy  $Gui(wGraphChangeMenu) 
    }
    #-------------------------------------------
    # Popup Window
    #-------------------------------------------
    toplevel $w -class Dialog -bg $Gui(activeWorkspace)
    wm title $w "Graph - Change Menue"
    wm iconname $w Dialog
    wm protocol $w WM_DELETE_WINDOW "wm withdraw $w"
    # wm resizable $w  0 0
    if {$Gui(pc) == "0"} {
        wm transient $w .
    }
    wm withdraw $w

    # regexp {([^x]*)x([^\+]*)} [wm geometry $w] match w h
    # Frames
    set f $w
    
    # wm positionfrom $w user

    # Set Paramters
    # Set Frame structure
    eval {label $f.lTitle -text "Change Menu" } $Gui(WTA)
    frame $f.fEdit     -bg $Gui(activeWorkspace)
    frame $f.fButtons -bg $Gui(activeWorkspace)
    pack $f.lTitle $f.fEdit $f.fButtons -side top -padx 2 -pady 2

    # Title
    set Graph(Title) $localArray(Graph,$path,Title)
    frame $f.fEdit.fTitle -bg $Gui(activeWorkspace)
    pack $f.fEdit.fTitle -side top -padx 2 -pady 2 -fill x
    DevAddLabel $f.fEdit.fTitle.lTitle "Title :"
    eval {entry $f.fEdit.fTitle.eTitle -width 20 -textvariable Graph(Title) } $Gui(WEA)
    TooltipAdd $this $f.fEdit.fTitle.eTitle "Change the title of the graph"
    pack $f.fEdit.fTitle.lTitle $f.fEdit.fTitle.eTitle -side left  -padx $Gui(pad) -pady 2 -anchor w 

    # X and Y Axis
    foreach axis "X Y" {
    set Graph(${axis}min)  $localArray(Graph,$path,${axis}min)
    set Graph(${axis}max)  $localArray(Graph,$path,${axis}max)
    set Graph(${axis}sca)  $localArray(Graph,$path,${axis}sca)

    frame $f.fEdit.f${axis}Axis -bg $Gui(activeWorkspace) -bd 2 -relief groove
        pack $f.fEdit.f${axis}Axis  -side left  -padx 2 -pady 2 

    eval {label $f.fEdit.f${axis}Axis.lTitle  -text "${axis}-Axis" } $Gui(WTA)
    frame $f.fEdit.f${axis}Axis.fFields -bg $Gui(activeWorkspace) 
        pack $f.fEdit.f${axis}Axis.lTitle $f.fEdit.f${axis}Axis.fFields -side top 

    frame $f.fEdit.f${axis}Axis.fFields.fRight -bg $Gui(activeWorkspace)
    frame $f.fEdit.f${axis}Axis.fFields.fLeft  -bg $Gui(activeWorkspace)
    pack $f.fEdit.f${axis}Axis.fFields.fRight $f.fEdit.f${axis}Axis.fFields.fLeft -side left -padx 2 -pady 2 -fill x

        DevAddLabel $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}min "Minimum:"
        eval {entry $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}min -width 4 -textvariable Graph(${axis}min) } $Gui(WEA)
        TooltipAdd $this $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}min "Define the minimum value on the ${axis}-Axis"

        DevAddLabel $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}max "Maximum:"
        eval {entry $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}max -width 4 -textvariable Graph(${axis}max) } $Gui(WEA)
        TooltipAdd $this $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}max "Define the maximum value on the ${axis}-Axis"

        DevAddLabel $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}sca "Scaling:"
        eval {entry $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}sca -width 4 -textvariable Graph(${axis}sca) } $Gui(WEA)
        TooltipAdd $this $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}sca "Define the scaling value on the ${axis}-Axis"

        pack $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}min $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}max $f.fEdit.f${axis}Axis.fFields.fRight.l${axis}sca \
        -side top -padx $Gui(pad) -pady 2 -anchor w 
        pack $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}min $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}max $f.fEdit.f${axis}Axis.fFields.fLeft.e${axis}sca \
        -side top -anchor w
    }

    if {$localArray(Graph,$path,Dimension) < 2} {
    $f.fEdit.fYAxis.fFields.fLeft.eYmin configure -state disabled
    $f.fEdit.fYAxis.fFields.fLeft.eYmax configure -state disabled
    }

    eval {button $f.fButtons.bCancel -text "Cancel" -width 8 -command "wm withdraw $w"} $Gui(WBA)
    DevAddLabel $f.fButtons.lEmpty2 "      "
    eval {button $f.fButtons.bUpdate -text "Update" -width 8 -command "$localArray(this) _GraphUpdateValues $varName \"$path\""} $Gui(WBA)
    TooltipAdd $this $f.fButtons.bUpdate "Press this button to update graph values."

    pack  $f.fButtons.bCancel $f.fButtons.lEmpty2 $f.fButtons.bUpdate -side left  -side left -padx 0  -pady 0

    update 
    ShowPopup $Gui(wGraphChangeMenu) $xpos $ypos
}

#-------------------------------------------------------------------------------
# .PROC  GraphInteractor
# Defines Binding for graph 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphInteractor {this varName path widget} {
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
        DevErrorWindow "Error finding $varName in GraphInteractor"
        return
   }
   if {$localArray(Graph,$path,descrFlag)} {
     bind $widget <Button-1>  "$localArray(this) _GraphChangeMenu $varName \"$path\" %W %X %Y"
     TooltipAdd $this $widget "Press left mouse button to change graph settings"
   }
}

#-------------------------------------------------------------------------------
# .PROC  GraphUpdateValue
# Menue to change paramters 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphUpdateValues {varName path} {
   global Graph
   # This is the same as global <varname>
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphChangeMenu"
      return
   }

   if {$Graph(Title) != $localArray(Graph,$path,Title)} {
       set localArray(Graph,$path,Title) $Graph(Title) 
       $localArray(Graph,$path,caTitle) itemconfigure  Title -text $Graph(Title)
   } 
   # First check if entries are correct 
   foreach axis "X Y" {
       if {$localArray(Graph,$path,${axis}format) == "%d" } {
          set TYPE integer
       } else {
          set TYPE double
       }

       if { [string is $TYPE -strict  $Graph(${axis}min) ] == 0 } {
           set Graph(${axis}min) $localArray(Graph,$path,${axis}min)
       } 

       if { [string is $TYPE -strict  $Graph(${axis}max) ] == 0 } {
           set Graph(${axis}max) $localArray(Graph,$path,${axis}max)
       } 

       if { [string is $TYPE -strict  $Graph(${axis}sca) ] == 0  } {
           set Graph(${axis}sca) $localArray(Graph,$path,${axis}sca)
       }    

       if { [expr $Graph(${axis}min) >= $Graph(${axis}max) ] } {
           set Graph(${axis}min) $localArray(Graph,$path,${axis}min)
           set Graph(${axis}max) $localArray(Graph,$path,${axis}max)
       }
   }

   if {($Graph(Xmin) != $localArray(Graph,$path,Xmin)) || ($Graph(Xmax) != $localArray(Graph,$path,Xmax)) || ($Graph(Xsca) != $localArray(Graph,$path,Xsca))} {
        $localArray(this) _GraphRescaleAxis  $varName $path $Graph(Xmin) $Graph(Xmax) $Graph(Xsca) 0}

   if {($Graph(Ymin) != $localArray(Graph,$path,Ymin)) || ($Graph(Ymax) != $localArray(Graph,$path,Ymax)) || ($Graph(Ysca) != $localArray(Graph,$path,Ysca))} {
       $localArray(this) _GraphRescaleAxis $varName $path $Graph(Ymin) $Graph(Ymax) $Graph(Ysca) 1}
}

#-------------------------------------------------------------------------------
# .PROC GraphRescaleAxis
# It will rescale the Graph for the changes in the ${AxisName}-Axis 
# where AxisName = (axis > 0 : Y : X) 
# All curves/region in the graph have to be updated by user
# if axis = 0 :
#  This function will call ${varname}GraphXAxisUpdate path Xmin Xmax Xsca if it exists  
#   => define how to update of the curves/region at this section (see EMSegment.tcl as an example)
# if axis = 1 and dimension = 2:
#  This function will call ${varname}GraphYAxisUpdate path Ymin Ymax Ysca if it exists  
#   => define how to update of the region at this section (see EMSegment.tcl as an example)
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc GraphRescaleAxis {varName path Min Max Sca Axis} {
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphRescaleAxis"
      return
    }

    if {$Axis} {set Aname Y
    } else { set Aname X }

    if {($Min ==  $localArray(Graph,$path,${Aname}min)) && \
        ($Max ==  $localArray(Graph,$path,${Aname}max)) && \
        ($Sca ==  $localArray(Graph,$path,${Aname}sca))} { return }

    # Rescale Axis if necessary
    set localArray(Graph,$path,${Aname}min) $Min
    set localArray(Graph,$path,${Aname}max) $Max
    set localArray(Graph,$path,${Aname}sca) $Sca
    set localArray(Graph,$path,${Aname}InvUnit)  [GraphCalcInvUnit $Min $Max $localArray(Graph,$path,${Aname}len) $localArray(Graph,$path,${Aname}offset)]
    # Delete Labeling of ${Aname}-Axis
    if {$localArray(Graph,$path,descrFlag)} {
      if {$Axis} {
        set start 0
      } else {
    $localArray(Graph,$path,ca${Aname}Axis) delete XAxisTX0
    $localArray(Graph,$path,caYAxis) delete XAxisTY0
        set start 1
      }
      for {set i $start} { $i <= $localArray(Graph,$path,${Aname}scn)} {incr i} {
        $localArray(Graph,$path,ca${Aname}Axis) delete ${Aname}AxisL$i
        $localArray(Graph,$path,ca${Aname}Axis) delete ${Aname}AxisT$i
      }
    }
    # calulate localArray(Graph,$path,${Aname}leU), localArray(Graph,$path,${Aname}scn) and localArray(Graph,$path,${Aname}scp) 
    GraphCalculateScnScp $varName $path $Axis
    # Relabel ${Aname}-Axis
    if {$localArray(Graph,$path,descrFlag)} {GraphCreate${Aname}Labels $varName $path}
    
    if {($Axis == 0) || ($localArray(Graph,$path,Dimension) == 2) } {
    set localArray(Graph,$path,${Aname}InvUnit) [GraphCalcInvUnit $Min $Max $localArray(Graph,$path,${Aname}len) $localArray(Graph,$path,${Aname}offset)]

    if { [ catch { $localArray(this) Graph${Aname}AxisUpdate $path $Min $Max $Sca } ]  } {
        puts stderr "$localArray(this) Graph${Aname}AxisUpdate $path $Min $Max $Sca "
        puts stderr "Graph.tcl:GraphRescale${Aname}Axis: Error: Please define function '$localArray(this) Graph${Aname}AxisUpdate path ${Aname}min ${Aname}max ${Aname}sca' to update curves/region" 
    }
    GraphRender $varName $path
    }
}

#-------------------------------------------------------------------------------
# .PROC GraphRender 
# Renders the changes defined in ${varName}(Graph,$path,vtkImageGraph)
# .ARGS
# array          varName This is typically the name of the module.
# widget         path    Frame where canvas should go on 
# .END
#-------------------------------------------------------------------------------
proc GraphRender {varName path} {
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphChangeMenu"
      return
   }
   ${varName}(Graph,$path,vtkImageGraph) Update 
   if {$localArray(Graph,$path,Dimension) == 1} {
       set GraphMin [${varName}(Graph,$path,vtkImageGraph) GetGraphMin]
       set GraphMax [${varName}(Graph,$path,vtkImageGraph) GetGraphMax]
       if {(($GraphMax != $GraphMin) && ($localArray(Graph,$path,Yscn) > 0.0))} {
       set Ysca [expr ($GraphMax - $GraphMin) / $localArray(Graph,$path,Yscn)]
       # This is necessary otherwise y-scalling gets due to error in calculation  
       while {[expr ($GraphMax - $GraphMin) / $Ysca ] <  $localArray(Graph,$path,Yscn)} {
           set Ysca [expr $Ysca * 0.999] 
       }
       GraphRescaleAxis $varName $path $GraphMin $GraphMax $Ysca 1
       } 
       # Do not update scalling when it is not correct 
   }
   if {$localArray(Graph,$path,graphExtFlag)} {
       ${varName}(Graph,$path,vtkImageDrawObjects) Update
   }
   set hist $localArray(Graph,$path,hist)
   ${hist}Win Render 
   return $localArray(Graph,$path,CurveIndex)
}

#-------------------------------------------------------------------------------
# .PROC GraphAddCurveRegion 
# Displays the given curve/region at the graph 
# .ARGS
# array          varName This is typically the name of the module.
# widget         path    Frame where canvas should go on 
# vtkImageData   data    Data to be ploted
# array          color   3Dim array from [0,1] defining the color of the curve 
# int            type    Type of curve (0 = continous data /; 1 = for discrete data _|)
# bool           ignore  Ignore the GraphMin/GraphMax definition of the Graph (1= Yes; 0 = No) 
#                        (necessary eg when displaying a gausian curve and histogram) 
# .END
#-------------------------------------------------------------------------------
proc GraphAddCurveRegion {varName path data color type ignore} {
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphChangeMenu"
      return
   }
   incr localArray(Graph,$path,CurveIndex)
   # Kilian: Should check if it really got assigned a new ID => otherwise not added to the graph 
   set localArray(Graph,$path,Curve,$localArray(Graph,$path,CurveIndex),ID) [${varName}(Graph,$path,vtkImageGraph) AddCurveRegion $data [lindex $color 0] [lindex $color 1] [lindex $color 2] $type $ignore]
   GraphRender $varName $path
   # puts [$data Print]
   return $localArray(Graph,$path,CurveIndex)
}

#-------------------------------------------------------------------------------
# .PROC GraphCreateGaussianCurveRegion 
# Creates a gaussian curve  
# To add it to the graph call GraphAddCurve $varName $path ${varName}(Graph,$path,Curve,$Number,Data) .... 
# .ARGS
# string varDataCurve Name of the variable that the curve/region will be saved under
# array  mean         Depending if it is a 1D graph or 2D Graph 1 or 2 values have to be defined
# array  covariance   Depending if it is a 1D graph or 2D Graph 1 or 4 values have to be 
#                     defined order "a(1,1) a(1,2) a(2,1) a(2,2)"
# double probability  Global Probability of the curve (you can just set it to 1.0)
# int    fct          Should the log gausian be calculated (2) or just the normal one (1)  
# int    dimension    Dimension of the curve (1D or 2D)
# float  Xmin         Start Value X-Axis        
# float  Xmax         End Value  X-Axis      
# float  Xlen         Caluclation unit on the X-Axis       
# float  Ymin         Start Value Y-Axis (only improtant for dimension 2)       
# float  Ymax         End Value Y-Axis (only improtant for dimension 2)            
# float  Ylen         Caluclation unit on the Y-Axis (only improtant for dimension 2)             
# .END
#-------------------------------------------------------------------------------
proc GraphCreateGaussianCurveRegion {varDataName mean covariance probability fct dimension Xmin Xmax Xlen {Ymin 0} {Ymax 0} {Ylen 0} } {
   catch {$varDataName Delete} 
   vtkImageCurveRegion $varDataName 
   $varDataName SetDimension $dimension
   $varDataName SetFunction  $fct
   set index 0
   for {set x 0 } {$x < $dimension} {incr x} {
       $varDataName SetMean [lindex $mean $x] $x
       for {set y 0 } {$y < $dimension} {incr y} {
       $varDataName SetCovariance [lindex $covariance $index] $y $x
       incr index
       }
   }
   $varDataName SetProbability $probability
   $varDataName SetXmin    $Xmin 
   $varDataName SetXmax    $Xmax 
   $varDataName SetXlength $Xlen 

   if {$dimension == 2} {
       $varDataName SetYmin $Ymin 
       $varDataName SetYmax $Ymax 
       $varDataName SetYlength $Ylen
   }

   $varDataName Update
   return $varDataName
}  

#-------------------------------------------------------------------------------
# .PROC GraphCreateHistogramCurve 
# Creates the histogram of a volume 
# To add it to the graph call GraphAddCurve $varName $path ${varName}(Graph,$path,Curve,$Number,Data) .... 
# .ARGS
# string        varDataCurve Name of the variable that the curve/region will be saved under
# vtkImageData  Volume       for which the histogram should be created 
# float         Xmin         Start Value        
# float         Xmax         End Value        
# float         Xlen         Caluclation unit   
# .END
#-------------------------------------------------------------------------------
proc GraphCreateHistogramCurve {varDataName Volume Xmin Xmax Xlen} {
    catch "${varDataName}Accu Delete"
  
    vtkImageAccumulate ${varDataName}Accu
    ${varDataName}Accu SetInput $Volume
  
    # set data [${varDataName}Accu GetInput]
    # $data Update
    # set inputRange [[[$data GetPointData] GetScalars] GetRange]
    # set origin [lindex $inputRange 0]
    # set numBins [lindex $inputRange 1]
 
    # Kilian changed set extent  [expr int($Xmax - $Xmin - 1)
    set extent  [expr int($Xmax - $Xmin)]
    # How many bins , e.g. 10
    # ${varDataName}Accu SetComponentExtent 0 [expr $Xlen-1] 0 0 0 0
    ${varDataName}Accu SetComponentExtent 0 $extent 0 0 0 0
    # Start value e.g. 1000
    ${varDataName}Accu SetComponentOrigin $Xmin 0.0 0.0 
    # Thicknes of the bin e.g 1000 - 2000 with 10 bins => 100   
    ${varDataName}Accu SetComponentSpacing 1.0 1.0 1.0
    ${varDataName}Accu UpdateWholeExtent
    ${varDataName}Accu Update

    # This is necessary because otherwise you get a wired histograph (you could defined it also in vtkImageAccu... but results are not good
    set XInvUnit [GraphCalcInvUnit $Xmax $Xmin $Xlen 0]
    GraphCreateResampledCurve ${varDataName} [${varDataName}Accu GetOutput] $XInvUnit
    # Believe it or not I still have to do the following check otherwise things go bad 
    GraphAdjustResampledCurve ${varDataName} $XInvUnit $Xlen
}

proc GraphCreateResampledCurve {varResDataName Input ScaleFactor} {
    catch "${varResDataName} Delete"  
    vtkImageResample ${varResDataName}
    ${varResDataName} SetDimensionality 1
    ${varResDataName} SetAxisOutputSpacing 0 1.0
    ${varResDataName} SetAxisMagnificationFactor 0 $ScaleFactor 
    ${varResDataName} SetInterpolationModeToLinear
    # ${varResDataName} InterpolateOff
    ${varResDataName} SetInput $Input
    ${varResDataName} Update 
} 

proc GraphAdjustResampledCurve {varResDataName ScaleFactor Xlen} {
    set output [$varResDataName GetOutput]
    set extent [$output GetExtent]

    while {[expr [lindex $extent 1] - [lindex $extent 0] + 1] > $Xlen } {
      set ScaleFactor [expr $ScaleFactor * 0.99]
      $varResDataName SetAxisMagnificationFactor 0 $ScaleFactor
      $varResDataName Update 
      set extent [$output GetExtent]
    }

    while {[expr [lindex $extent 1] - [lindex $extent 0] + 1] < $Xlen } {
      set ScaleFactor [expr $ScaleFactor * 1.001]
      $varResDataName SetAxisMagnificationFactor 0 $ScaleFactor
      $varResDataName Update 
      set extent [$output GetExtent]
    }
    $output UpdateInformation 
    return $ScaleFactor
} 

#-------------------------------------------------------------------------------
# .PROC GraphRemoveCurve 
# Takes a curve from the graph 
# .ARGS
# array  varName    This is typically the name of the module.
# widget path       Frame where canvas should go on 
# int    number     Curve Number to be deleted 
# .END
#-------------------------------------------------------------------------------
proc GraphRemoveCurve {varName path number} {
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphChangeMenu"
      return
   }
   if {[${varName}(Graph,$path,vtkImageGraph) DeleteCurveRegion $localArray(Graph,$path,Curve,$number,ID)] == 0} {
       puts "GraphDeleteCurve: Could not delete ${varName}(Graph,$path,Curve,$number) "
   } else {
       unset localArray(Graph,$path,Curve,$number,ID)
   }
   GraphRender $varName $path
}
#-------------------------------------------------------------------------------
# .PROC GraphCalcUnit 
# Calculates the unit given the minimum , maximum and pixel length of an axis
# .ARGS
# float Min    Minimum of axis 
# float Max    Maximum of axis 
# int   Length Pixel length of axis 
# .END
#-------------------------------------------------------------------------------
proc GraphCalcUnit {Min Max Length} {
   set Dist [expr double($Max - $Min) ] 
   set Unit [expr $Dist / double($Length) ]
   if {$Unit > 0} {
       while {[expr $Dist / $Unit] <  $Length} { set Unit [expr $Unit * 0.999] }
       return $Unit
   } 
   return 1.0
}

#-------------------------------------------------------------------------------
# .PROC GraphCalcInvUnit 
# Calculates the inverse of  GraphCalcUnit 
# .ARGS
# floaxmint Min    Minimum of axis 
# float Max    Maximum of axis 
# int   Length Pixel length of axis 
# .END
#-------------------------------------------------------------------------------
proc GraphCalcInvUnit {Min Max Length Offset} {
   set Dist [expr double($Max - $Min + $Offset ) ] 
   if {$Dist > 0} {
      incr  Length -1 
      set InvUnit [expr double($Length) / $Dist ]
      while {[expr int($Dist * $InvUnit) ] >  $Length} { set InvUnit [expr $InvUnit * 0.995] }
      while {[expr int($Dist * $InvUnit) ] <  $Length} { set InvUnit [expr $InvUnit * 1.001] }
      return $InvUnit
    } 
    return 1.0
}
#-------------------------------------------------------------------------------
# .PROC GraphHexToRGB 
# Transforms a color code from hex to rgb format , 
# input e.g. a17B00 output "161/255.0 123/255.0 0"
# .ARGS
# hex    colorcode  colorcode in hex form 
# .END
#-------------------------------------------------------------------------------
proc GraphHexToRGB {colorcode} {
  return "[expr [format %i 0x[string range $colorcode 0 1]]*1.0 /255.0]  [expr [format %i 0x[string range $colorcode 2 3]]*1.0 /255.0]  [expr [format %i 0x[string range $colorcode 4 5]]*1.0 /255.0]"
}

#-------------------------------------------------------------------------------
# .PROC GraphChangeBackgroundColor
# If you used the default background defined in GraphCreate
# this function alllowes you to change its color 
# .ARGS
# array  varName    This is typically the name of the module.
# widget path       Frame where canvas should go on 
# int    number     Curve Number to be deleted
# float  saturation
# float  val
# float  hue 
# .END
#-------------------------------------------------------------------------------
proc GraphChangeBackgroundColor {varName path saturation val hue}    {
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphChangeMenu"
      return
   }    
   ${varName}(Graph,$path,vtkImageGraph) ChangeColorOfIndirectLookupTable $localArray(Graph,$path,graphLUT) $saturation $saturation $val $val $hue $hue 
}

#-------------------------------------------------------------------------------
# .PROC GraphDelete 
# Delete the Graph
# .ARGS
# array  varName     This is typically the name of the module.
# widget path        Frame where canvas should go on  
# .END
#-------------------------------------------------------------------------------
proc GraphDelete {varName path} {
   # This is the same as global <varname>
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphDelete"
      return
   }
   # Automatic generated by Graph.tcl => destroy it too
   for {set i 0} {$i <=  $localArray(Graph,$path,CurveIndex)} {incr i} {
       catch "unset localArray(Graph,$path,Curve,$i,ID)"
   }
   
   if {$localArray(Graph,$path,bgLUTDefault)} {catch "$localArray(Graph,$path,graphLUT) Delete"}
   catch "${varName}(Graph,$path,vtkImageGraph) Delete"
   catch "unset ${varName}(Graph,$path,vtkImageGraph)"
   
   if {$localArray(Graph,$path,graphExtFlag)} {
        catch "${varName}(Graph,$path,vtkImageDrawObjects) Delete"
        catch "unset ${varName}(Graph,$path,vtkImageDrawObjects)"
   }

   set hist $localArray(Graph,$path,hist)
   catch "${hist}Mapper Delete"
   catch "${hist}Actor Delete"
   catch "${hist}Imager Delete"
   catch "${hist}Win Delete"
   destroy $path.fGraph
}

#-------------------------------------------------------------------------------
# .PROC GraphCreateLine 
# Creates a line on the graph and returns the ID of the line. 
# Do not forget to run GraphRender to see changes   
# If you get an error message invalid command ....
# please the GraphExtFlag in GraphCreate and rerun process
# .ARGS
# array  varName This is typically the name of the module.
# widget path    Frame where canvas should go on 
# array  color   List with 3 values defining the color of the grapp
# int    radius  Radius of the line (0 = 1 pt line; 1 = 3 pt .. ) 
# .END
#-------------------------------------------------------------------------------
proc GraphCreateLine {varName path color radius} {
     return [${varName}(Graph,$path,vtkImageDrawObjects) AddObject 0 0 0 0 [lindex $color 0] [lindex $color 1] [lindex $color 2] 0 $radius]
}

#-------------------------------------------------------------------------------
# .PROC GraphSetLineCoordinates
# Sets coordinates of the line. Do not forget to run GraphRender to see changes   
# .ARGS
# array  varName     This is typically the name of the module.
# widget path        Frame where canvas should go on 
# int    id          ID of the line which should be set  
# array  coordinates List of four values defining x(0) y(0) x(1) y(1)
# .END
#-------------------------------------------------------------------------------
proc GraphSetLineCoordinates {varName path id coordinates} {
    ${varName}(Graph,$path,vtkImageDrawObjects) SetObjectPosition $id [lindex $coordinates 0] [lindex $coordinates 1] [lindex $coordinates 2] [lindex $coordinates 3]
}

#-------------------------------------------------------------------------------
# .PROC GraphDeleteLine 
# Deletes the line.  Do not forget to run GraphRender to see changes  
# .ARGS
# array  varName This is typically the name of the module.
# widget path    Frame where canvas should go on 
# int    id      ID of the line which should be set  
# .END
#-------------------------------------------------------------------------------
proc GraphDeleteLine {varName path id} {
    upvar #0 $varName localArray
    ${varName}(Graph,$path,vtkImageDrawObjects) DeleteObject $id 
}

#-------------------------------------------------------------------------------
# .PROC GraphTransformValueInCoordinate 
# Transforms a given value to a cooridnate on the X or Y Axis
# if the values is bejond the graph axis mimum or maximum it will return 
# the minium or maximum value
# .ARGS
# array  varName This is typically the name of the module.
# widget path    Frame where canvas should go on 
# float  value   Value that should be maped
# bool   axis    0 = X Axis ; 1= Yaxis 
# .END
#-------------------------------------------------------------------------------
proc GraphTransformValueInCoordinate {varName path value axis} {
   # This is the same as global <varname>
   upvar #0 $varName localArray
   if {0 == [info exists localArray]} {
      DevErrorWindow "Error finding $varName in GraphTransformValueInCoordinate"
      return
   }
   if {$axis} {set Aname Y
   } else {set Aname X}
   if {$value > $localArray(Graph,$path,${Aname}max)} { set value  [expr $localArray(Graph,$path,${Aname}max) - $localArray(Graph,$path,${Aname}min)]
   } else { 
       if {$value <  $localArray(Graph,$path,${Aname}min)} { set value 0
       } else {  set value [expr $value - $localArray(Graph,$path,${Aname}min)]}
   }
   return [expr int($value* $localArray(Graph,$path,${Aname}InvUnit))]
}
#-------------------------------------------------------------------------------
# .PROC  GraphCreateDisplayValues
# Creates/displays the entry field in the upper right corner of each graph displaying the 
# current value of the image 
# .ARGS
# array  varName     This is typically the name of the module.
# widget path        Frame where canvas should go on  
# list   values      Value(s) that should be displayed
# bool   DisplayFlag If set value will be displayed otherwise a field will be created
# bool   LogFlag     If set, the values will be dispplayed as log values 
# .END
#-------------------------------------------------------------------------------
proc GraphCreateDisplayValues {varName path values DisplayFlag LogFlag} {
    global Gui
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
    DevErrorWindow "Error finding $varName in GraphCreateDisplayValues"
    return
    }

    if {$DisplayFlag } {
    for {set i 0} { $i < $localArray(Graph,$path,Dimension)} {incr i} {
        if {$LogFlag} { $localArray(Graph,$path,caRBorder) itemconfigure ValueT$i -text [format "%01.3f" [expr log([lindex $values $i]+1)]]
        } else { $localArray(Graph,$path,caRBorder) itemconfigure ValueT$i -text [lindex $values $i]}
    }
    } else {
    set pos 13
    for {set i 0} { $i < $localArray(Graph,$path,Dimension)} {incr i} {
        if {$LogFlag} {
          $localArray(Graph,$path,caRBorder) create text $pos [expr $i*8 + 6] -text [format "%01.3f" [expr log([lindex $values $i]+1)]]\
              -tag ValueT$i -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)
            }  else {
          $localArray(Graph,$path,caRBorder) create text $pos [expr $i*8 + 6] -text [lindex $values $i]  -tag ValueT$i -font $localArray(Graph,$path,Font) -fill $localArray(Graph,$path,CoordColor)
        }
    }
    }
}
#-------------------------------------------------------------------------------
# .PROC  GraphDeleteValueDisplay
# Deletes the entry field in the upper right corner of each graph
# .ARGS
# array  varName     This is typically the name of the module.
# widget path        Frame where canvas should go on  
# .END
#-------------------------------------------------------------------------------
proc GraphDeleteValueDisplay {varName path} {
    global Gui
    # This is the same as global <varname>
    upvar #0 $varName localArray
    if {0 == [info exists localArray]} {
    DevErrorWindow "Error finding $varName in GraphDeleteValueDisplay"
    return
    }

    for {set i 0} { $i < $localArray(Graph,$path,Dimension)} {incr i} {
    $localArray(Graph,$path,caRBorder) delete ValueT$i 
    }
}





proc DevErrorWindow {{message "Unknown Error"}} {
    set oscaling [tk scaling]
    tk scaling 1
    puts "$message"
    # tk_messageBox -title Slicer -icon error -message $message -type ok
    # tk scaling $oscaling
}



