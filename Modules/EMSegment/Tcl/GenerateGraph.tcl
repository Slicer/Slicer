package require Itcl

# The class definition - define if needed (not when re-sourcing)
if { [itcl::find class EMSegmenterGraph ] == "" }  {
    itcl::class EMSegmenterGraph {
    constructor { }  { }
    destructor { } 


    variable _modGui           ""
    variable _modLogic         ""
    variable _mrmlManager      ""
    variable _workingDN        ""
        variable _inputTarget     "" 
        variable _alignedTarget    "" 
        variable _nullVolume       ""
    variable _slicesGUI ""
    variable _sliceGUI0 ""
    variable _sliceGUI1 ""
    variable _sliceGUI2 ""
    variable _renInteractor0 ""
    variable _renInteractor1 ""
    variable _renInteractor2 ""
    variable _event0 { }
    variable _event1 { } 
    variable _event2 { } 

    method CreateWindow {  }
    method CreateHistogramButton { f index }
    method CreateGraphButton {Sclass Label Color {Above 0} {UpdateGraph 1}} 
    method ConvertToHex { RGB } 
    method GetListOfClassNodeIDs { rootNodeID }
    method GetListOfSuperClassNodeIDs { rootNodeID }
    method ChangeVolumeGraph {VolumeID numGraph} 
    method UpdateClasses {flag}
    # method CalculateClassMeanCovariance { }
    method PlotCurveRegion {numGraph}
    method CalcProb {}
    method MultipleDrawDeleteCurveRegion { Sclass }
    method DrawDeleteCurveRegion { Sclass NumGraph } 
    method GraphXAxisUpdate {path Xmin Xmax Xsca}
    method GraphYAxisUpdate {path Ymin Ymax Ysca}
    method CreateDisplayRedLine {NumGraph Value}
    method CreateDisplayRedCross {NumGraph Xvalue Yvalue} 
    method DeleteRedMarker { graphID }

    method GetGUICallbackCommand { ID event }
    method EnterDisplaySample {x y} 
    method AssignDefaultVolumes  { } 
    # have to be call this way bc otherwise tcl does not find those commands anymore 
    method _GraphChangeMenu { varName path widget xpos ypos } { GraphChangeMenu $this $varName $path $widget $xpos $ypos }
    method _GraphUpdateValues {varName path} { GraphUpdateValues $varName $path }
    method _GraphRescaleAxis {varName path Min Max Sca Axis} { GraphRescaleAxis $varName $path $Min $Max $Sca $Axis }
    method _ExposeTkImageViewer {widget x y w h} { ExposeTkImageViewer $widget $x $y $w $h }
    method _TooltipEnterWidget {widget tip X Y} { TooltipEnterWidget $widget "$tip" $X $Y }
    method _TooltipExitWidget {} { TooltipExitWidget }
    }
}

# itcl::body

# --------------------------------------------------------------
itcl::body EMSegmenterGraph::constructor { } {
    global EMSegment Volume Gui
    # Make sure you have sourced the following files before executing script Gui.tcl Graph.tcl Tooltips.tcl VTK/Wrapping/Tcl/vtkinteraction/setget.tcl
    set _modGui             [ $::slicer3::Application GetModuleGUIByName "EMSegmenter" ]
    set _modLogic           [ $_modGui GetLogic ]
    set _mrmlManager        [ $_modGui GetMRMLManager ]
    set _workingDN          [ $_mrmlManager GetWorkingDataNode]
    #set _slicesGUI          [$::slicer3::ApplicationGUI GetSlicesGUI] 
    set _sliceGUI0          [$::slicer3::ApplicationGUI GetMainSliceGUI "Red"]
    set _sliceGUI1          [$::slicer3::ApplicationGUI GetMainSliceGUI "Yellow"]
    set _sliceGUI2          [$::slicer3::ApplicationGUI GetMainSliceGUI "Green"]
    set _renInteractor0     [[[$_sliceGUI0 GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]
    set _renInteractor1     [[[$_sliceGUI1 GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]
    set _renInteractor2     [[[$_sliceGUI2 GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]

    set _event0 { }
    lappend _event0 [[$_renInteractor0 GetInteractorStyle] AddObserver  MouseMoveEvent  "$this GetGUICallbackCommand 0 0"]
    lappend _event0 [[$_renInteractor0 GetInteractorStyle] AddObserver  LeaveEvent   "$this GetGUICallbackCommand 0 1"]
    set _event1 { }
    lappend _event1 [[$_renInteractor1 GetInteractorStyle] AddObserver  MouseMoveEvent  "$this GetGUICallbackCommand 1 0"]
    lappend _event1 [[$_renInteractor1 GetInteractorStyle] AddObserver  LeaveEvent   "$this GetGUICallbackCommand 1 1"]
    set _event2 { }
    lappend _event2 [[$_renInteractor2 GetInteractorStyle] AddObserver  MouseMoveEvent  "$this GetGUICallbackCommand 2 0" ]
    lappend _event2 [[$_renInteractor2 GetInteractorStyle] AddObserver  LeaveEvent   "$this GetGUICallbackCommand 2 1"]

    set _inputTarget        [ $_workingDN GetInputTargetNode]    
    set _alignedTarget      [ $_workingDN GetAlignedTargetNode]    
    if { $_alignedTarget == "" } {
    puts "EMSegmenterGraph::constructor: Kilian - currently not set up correctly so we simply use input "
    set _alignedTarget $_inputTarget
    # error "EMSegmenterGraph::constructor: Input Nodes are not correctly defined " 
    }

    catch {
        vtkImageData _nullVolume
    }

    GuiInit
    # General EM Parameters and Volume definition 

    # Class Related
    set EMSegment(this) $this
    set EMSegment(GlobalClassList) [ GetListOfClassNodeIDs [ $_mrmlManager GetTreeRootNodeID ] ]
    set EMSegment(GlobalSuperClassList) [ GetListOfSuperClassNodeIDs [ $_mrmlManager GetTreeRootNodeID ] ]

    # puts "Global class list: $EMSegment(GlobalClassList) $EMSegment(GlobalSuperClassList) "
    
    set ColorTable [[[$_mrmlManager GetMRMLScene] GetNodeByID [$_mrmlManager GetColormap] ] GetLookupTable]

    foreach ID $EMSegment(GlobalClassList) {
    set EMSegment(Cattrib,$ID,Label)      [$_mrmlManager GetTreeNodeName $ID]
    set node  [$_mrmlManager GetTreeNode $ID]
    if { $node != "" } {
        set PAR [$node GetParametersNode ] 
        # did not work set EMSegment(Cattrib,$ID,ColorCode)  [ConvertToHex [$PAR GetColorRGB ] ]
        set RGB [lrange [$ColorTable GetTableValue [$_mrmlManager GetTreeNodeIntensityLabel $ID]] 0 2]
        set EMSegment(Cattrib,$ID,ColorCode)  [ConvertToHex $RGB ]

        set EMSegment(Cattrib,$ID,Prob) [$_mrmlManager GetTreeNodeClassProbability $ID]
        # set EMSegment(Cattrib,$ID,LocalPriorWeight) 1.0
        # puts "ssdfds $RGB [ConvertToHex $RGB ]"
        # puts "Result [$ColorTable GetTableValue [$_mrmlManager GetTreeNodeIntensityLabel $ID]]  [$_mrmlManager GetTreeNodeIntensityLabel $ID] $PAR - [$PAR GetColorRGB ] - $ID $EMSegment(Cattrib,$ID,Label) $EMSegment(Cattrib,$ID,ColorCode) $EMSegment(Cattrib,$ID,Prob) "
    } else {
        puts "Node $ID does not exist " 
        set EMSegment(Cattrib,$ID,ColorCode) "#000000"
        set EMSegment(Cattrib,$ID,Prob) 0.0
    }
    }

    # Volume Related
    set Volume(idNone) 0 
    set EMSegment(NumInputChannel) [$_alignedTarget GetNumberOfVolumes]
    set EMSegment(SelVolList,VolumeList) ""
    for { set i 0 } { $i <  $EMSegment(NumInputChannel) } { incr i }  {
    set VolumeNode [$_alignedTarget GetNthVolumeNode $i ]
    set VolumeID [$VolumeNode GetID]
    lappend EMSegment(SelVolList,VolumeList)  $VolumeID
    set Volume($VolumeID,name)  [ $_inputTarget GetNthInputChannelName $i ] 
    set Volume($VolumeID,node)  $VolumeNode
    set Volume($VolumeID,data)  [$VolumeNode GetImageData]
    # puts "What Volume($VolumeID,node)  $VolumeNode"
    
    }

    # There are three NumGraph modes defined
    # 1 = display one 1-D Graph
    # 2 = display two 1-D Graph
    # 3 = display two 1-D Graph and one 2-D Graph (in the middle) => 3rd graph is 2D Graph 
    # How many Graph buttons should be in one row 
    set EMSegment(NumGraph) 3 
    set EMSegment(Graph,ButtonNum) 10
    # Should Probablilites be displayed in graph or not
    set EMSegment(Graph,DisplayProb) 0
    set EMSegment(Graph,DisplayProbNew) $EMSegment(Graph,DisplayProb)

    set EMSegment(Cattrib,0,ColorCode) $Gui(activeWorkspace)
    set EMSegment(Cattrib,0,ColorGraphCode) #ffb2b2 
    set EMSegment(Cattrib,0,LocalPriorWeight) 1.0

    if {$EMSegment(NumGraph) < 3} {
      set SingleGraphLength [expr int(350.0/$EMSegment(NumGraph))] 
    } else {
      set SingleGraphLength [expr int(350.0/(($EMSegment(NumGraph)-1)*2.0))] 
    }


    # Define Number Scalling in value difference
    for {set i 0} { $i < $EMSegment(NumGraph)} {incr i} {
      # Kilian : Have to update it differently bc no control over it anymore 

      set EMSegment(Graph,$i,Xsca) 50
      # Min - Max values
      set EMSegment(Graph,$i,Xmin) 10
      set EMSegment(Graph,$i,Xmax) 260

      if {$i < 2} {
        set EMSegment(Graph,$i,Ysca) 0.5
        set EMSegment(Graph,$i,Ymin) 0.0
        set EMSegment(Graph,$i,Ymax) 1.0
        set EMSegment(Graph,$i,Yfor) "%0.2f"
        set EMSegment(Graph,$i,Dimension) 1
      } else {
        set EMSegment(Graph,$i,Ysca) $EMSegment(Graph,$i,Xsca)
        set EMSegment(Graph,$i,Ymin) $EMSegment(Graph,$i,Xmin)
        set EMSegment(Graph,$i,Ymax) $EMSegment(Graph,$i,Xmax)
        set EMSegment(Graph,$i,Yfor) "%3.0f"
        set EMSegment(Graph,$i,Dimension) 2
        # The Minimum Display value that should be displayed in the graph
        # Leave it away right now set EMSegment(Graph,MinDisplayValue,$i) 0.000000001  
      }

      # Define coord system len in pixels   
      set EMSegment(Graph,$i,Xlen) 370 
      set EMSegment(Graph,$i,Ylen) [expr $EMSegment(Graph,$i,Dimension)*$SingleGraphLength - 17] 
      # What Volume is set for the graph 
      for {set j 0} {$j < $EMSegment(Graph,$i,Dimension)} { incr j} {
        set EMSegment(Graph,$i,VolumeID,$j) -1
        set EMSegment(Graph,$i,LineID,$j) -1
      }
    }           
}

# --------------------------------------------------------------
itcl::body EMSegmenterGraph::destructor { } {
    global Gui
    catch {
    _nullVolume Delete
    }
    catch {
    wm withdraw $Gui(wEMSegment)
    }

    if {$_event0 !=  { } } {
    foreach ev $_event0 {
        $_renInteractor0 RemoveObserver $ev
    }
    set _event0  { }
    }

    if {$_event1 != { } } {
    foreach ev $_event1 {
        $_renInteractor1 RemoveObserver $ev
    }
    set _event1 { }
    }

    if {$_event2 != { } } {
    foreach ev $_event2 {
        $_renInteractor2 RemoveObserver $ev
    }
    set _event2 { }
    }

}

# --------------------------------------------------------------
itcl::body  EMSegmenterGraph::CreateWindow { } {
    global Gui EMSegment

    set w .wEMSegmenterGraph
    set Gui(wEMSegment) $w
    
    if {[winfo exists $Gui(wEMSegment)]} {
    for {set i 0} {$i < $EMSegment(NumGraph)} {incr i} {
        catch {
        GraphDelete EMSegment $EMSegment(Graph,$i,path) 
        }
    }
    destroy  $Gui(wEMSegment) 
    }
    # Set parameters corectly 
    if {$EMSegment(NumInputChannel)  > 1 } {
    set EMSegment(NumGraph) 3 
    } else {
    set EMSegment(NumGraph) 1
    }

    #-------------------------------------------
    # Popup Window
    #-------------------------------------------
    toplevel $w -class Dialog -bg $Gui(activeWorkspace)
    wm title $w "Display Class Distribution"
    wm iconname $w Dialog
    wm protocol $w WM_DELETE_WINDOW "wm withdraw $w"
    # wm resizable $w  0 0
    if {$Gui(pc) == "0"} {
        # wm transient $w .
    }
    # wm withdraw $w
    # $w Display

    # wm withdraw $w
    # regexp {([^x]*)x([^\+]*)} [wm geometry $w] match w h
    # Frames
    set f $w

    # if { $_window == "" } {
    #     set _window [vtkKWWindowBase New]
    #     $_window SupportHelpOn
    #     $_window SetApplication $::slicer3::Application
    #     # $_window SetBackgroundColor $::Gui(activeWorkspace)
    #     $_window SetTitle "Display Class Distribution"
    #     $_window Create
    # }
    # set f $_window
    # $_window Display
    # return 
    # wm positionfrom $w user

    eval {label $f.lTitle -text "Display Class Distribution" } $Gui(WTA)
    pack $f.lTitle -side top -padx 4 -pady 4

    frame $f.fGraphButtons -bg $Gui(activeWorkspace)
    pack $f.fGraphButtons -side top -padx $Gui(pad) -pady 2 -fill x
    set EMSegment(Cl-fGraphButtons) $f.fGraphButtons
    # Create Button for Histogram
    # First Button is selecting the Volume for the Histogram 
    #Define Menu button
    CreateHistogramButton $f.fGraphButtons 0

    set ValueList " "
    if {$EMSegment(NumGraph) < 2} {
      set GraphList {0}
    } elseif {$EMSegment(NumGraph) < 3} {
      set GraphList {0 1}
    } else {
      set GraphList {0 2 1}
      lappend ValueList " "
    } 

    foreach i $GraphList {
       frame $f.fgraph$i -bg $Gui(activeWorkspace) 
       pack $f.fgraph$i -side top -padx 0 -pady 0 -fill x
       GraphCreate $this EMSegment $f.fgraph$i $EMSegment(Graph,$i,Xlen) $EMSegment(Graph,$i,Ylen) "" "0 0" 0 $EMSegment(Graph,$i,Dimension) 1 "" $EMSegment(Graph,$i,Xmin) $EMSegment(Graph,$i,Xmax) $EMSegment(Graph,$i,Xsca) "%d" $EMSegment(Graph,$i,Ymin) $EMSegment(Graph,$i,Ymax) $EMSegment(Graph,$i,Ysca) $EMSegment(Graph,$i,Yfor) 1
       GraphChangeBackgroundColor EMSegment $f.fgraph$i 0.0 0.8 0.0
       GraphCreateDisplayValues EMSegment $f.fgraph$i "$ValueList" 0 0
       set EMSegment(Graph,$i,path) $f.fgraph$i
    }

    frame $f.fGraphButtonsBelow -bg $Gui(activeWorkspace)
    pack $f.fGraphButtonsBelow -side top -padx $Gui(pad) -pady 1 -fill x
    set EMSegment(Cl-fGraphButtonsBelow) $f.fGraphButtonsBelow

    if {$EMSegment(NumGraph) > 1} { CreateHistogramButton $f.fGraphButtonsBelow 1}

    foreach i $EMSegment(GlobalClassList) {
       CreateGraphButton $i $EMSegment(Cattrib,$i,Label) $EMSegment(Cattrib,$i,ColorCode)
    }

    frame $f.fLastLine -bg $Gui(activeWorkspace)
    pack $f.fLastLine -side top -padx 2 -pady 2 -fill x

    frame $f.fLastLine.fGraph -bg $Gui(activeWorkspace)
    frame $f.fLastLine.fButtons -bg $Gui(activeWorkspace)

    pack $f.fLastLine.fButtons -side top -padx 0 -pady 2 
    pack $f.fLastLine.fGraph -side top -padx 0 -pady 2 

    eval {button $f.fLastLine.fButtons.bCancel -text "Cancel" -width 8 -command "wm withdraw $w"} $Gui(WBA)
    DevAddLabel $f.fLastLine.fButtons.lEmpty2 "      "
    eval {button $f.fLastLine.fButtons.bUpdate -text "Update" -width 8 -command "$this UpdateClasses 1"} $Gui(WBA)
    TooltipAdd $this $f.fLastLine.fButtons.bUpdate "Press this button to update class values and graph."

    pack  $f.fLastLine.fButtons.bCancel $f.fLastLine.fButtons.lEmpty2 $f.fLastLine.fButtons.bUpdate -side left  -side left -padx 0  -pady 0

    ShowPopup $Gui(wEMSegment)  100 100 

    # ShowPopup $Gui(wDownload) 100 100
}
#-------------------------------------------------------------------------------
# .PROC CreateHistogramButton
# Defines the Historgramm button
# .ARGS
# windowpath f 
# int index
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::CreateHistogramButton {f index} {
    global Gui  Volume EMSegment

    set VolID $EMSegment(Graph,$index,VolumeID,0)  
    if { ${VolID} == -1 } {
       set MenuText ""
       set volume _nullVolume 
    } else {
       set MenuText $Volume(${VolID},name) 
       set volume [Volume($VolID,node) GetImageData]
    }
    # Kilian - have to change this does not work this way anymore 
    GraphCreateHistogramCurve EMSegment(Graph,$index,Data,0) $volume $EMSegment(Graph,$index,Xmin) $EMSegment(Graph,$index,Xmax) $EMSegment(Graph,$index,Xlen)
    set EMSegment(Graph,$index,ID,0) -1

    set EMSegment(Cl-mbGraphHistogram$index) $f.mbHistogram
    set EMSegment(Cl-mGraphHistogram$index) $f.mbHistogram.m
    
    eval {menubutton $EMSegment(Cl-mbGraphHistogram$index) -text $MenuText  -menu $EMSegment(Cl-mGraphHistogram$index) -width 8} $Gui(WBA) 
    $EMSegment(Cl-mbGraphHistogram$index) configure -bg $EMSegment(Cattrib,0,ColorGraphCode)
    $EMSegment(Cl-mbGraphHistogram$index) configure -activebackground $EMSegment(Cattrib,0,ColorGraphCode)
    
    pack $EMSegment(Cl-mbGraphHistogram$index) -side left -padx $Gui(pad) 
    TooltipAdd  $this $EMSegment(Cl-mbGraphHistogram$index) "Press left mouse button to selct volume - press right mouse button to display volume's histogram "
    bind $EMSegment(Cl-mbGraphHistogram$index) <ButtonPress-3> "$this DrawDeleteCurveRegion 0 $index" 
    
    # Define Menu selection 
    eval {menu $EMSegment(Cl-mGraphHistogram$index)} $Gui(WMA)
    $EMSegment(Cl-mGraphHistogram$index) configure -bg $EMSegment(Cattrib,0,ColorGraphCode)
    $EMSegment(Cl-mGraphHistogram$index) configure -activebackground $EMSegment(Cattrib,0,ColorGraphCode)
    # Add Selection entry

    foreach VolID $EMSegment(SelVolList,VolumeList) { 
        $EMSegment(Cl-mGraphHistogram$index) add command -label $Volume($VolID,name) -command "$this ChangeVolumeGraph $VolID $index"
    }
}

# similar to vtkEMSegmentMRMLManager::GetListOfTreeNodeIDs(vtkIdType rootNodeID, vtkstd::vector<vtkIdType>& idList)
# but only returns children
itcl::body EMSegmenterGraph::GetListOfClassNodeIDs { rootNodeID } { 

    if { [ $_mrmlManager GetTreeNodeIsLeaf $rootNodeID ] } {
    return "$rootNodeID" 
    } 

    set result ""
    set childNum [ $_mrmlManager GetTreeNodeNumberOfChildren $rootNodeID ]
    for { set i 0 } { $i < $childNum } { incr i } { 
    set result "${result}[GetListOfClassNodeIDs [ $_mrmlManager GetTreeNodeChildNodeID $rootNodeID  $i] ] " 
    }
    return "$result"
}

#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::GetListOfSuperClassNodeIDs { rootNodeID } { 
    if { [ $_mrmlManager GetTreeNodeIsLeaf $rootNodeID ] } {
    return ""
    } 
    set result ""
    set childNum [ $_mrmlManager GetTreeNodeNumberOfChildren $rootNodeID ]
    for { set i 0 } { $i < $childNum } { incr i } {
    set childNode [ $_mrmlManager GetTreeNodeChildNodeID $rootNodeID  $i ]
    set result "${result}[GetListOfClassNodeIDs $childNode]" 
    }
    return "${rootNodeID} $result" 
}


#-------------------------------------------------------------------------------
# .PROC EMSegmentCreateGraphButton
# Creates for Class <Sclass> a Button so the class distribution can be diplayed 
# in the graph
# .ARGS
# string Sclass 
# string Label 
# string Color 
# int Above Defaults to 0
# boolean UpdateGraph defaults to 1
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::CreateGraphButton {Sclass Label Color {Above 0} {UpdateGraph 1}} {
    global EMSegment Gui
    set index [lsearch $EMSegment(GlobalClassList) $Sclass]
    set TooltipText "Press button to display Gaussian of $Sclass class  (Label: $Label)" 

    if {($index < $EMSegment(Graph,ButtonNum) ) || $Above} {
       set f $EMSegment(Cl-fGraphButtons).bGraphButton$Sclass
    } else {
       set f $EMSegment(Cl-fGraphButtonsBelow).bGraphButton$Sclass
    }
    eval {button $f -text $Label -width 3 -command "$this MultipleDrawDeleteCurveRegion $Sclass"} $Gui(WBA)
    $f configure -bg $Color -activebackground $Color
    pack $f -side left -padx $Gui(pad)
    TooltipAdd $this $f $TooltipText
    if {$UpdateGraph} {
       for {set i 0} { $i < $EMSegment(NumGraph)} {incr i} {
        # Generate setup for each curve 
        set EMSegment(Graph,$i,ID,$Sclass) -1
        set mean ""
        set cov ""
        set SetFlag 1
        for {set y 0 } { $y < $EMSegment(Graph,$i,Dimension) } {incr y} {

           set NumIndex($y) [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,$i,VolumeID,$y)]
       # puts "ggggg  $NumIndex($y) | $EMSegment(SelVolList,VolumeList) | $EMSegment(Graph,$i,VolumeID,$y) "
           if {$NumIndex($y) < 0} { set SetFlag 0}  }
      
        for {set y 0 } { $y < $EMSegment(Graph,$i,Dimension) } {incr y} {
        if {$SetFlag} {
            lappend mean [$_mrmlManager GetTreeNodeDistributionLogMean $Sclass $NumIndex(y)] 
        } else {
            lappend mean 0.0
        }
        for {set x 0 } { $x < $EMSegment(Graph,$i,Dimension) } {incr x} {
           if {$SetFlag} {
            lappend cov [$_mrmlManager GetTreeNodeDistributionLogCovariance $Sclass $NumIndex(y) $NumIndex(x)] 
            } else {
            if {$y == $x} {lappend cov 1.0
                } else { lappend cov 0.0}
            }
        }
        }
        set Prob [expr ($EMSegment(Graph,DisplayProb) > 0 ?  $EMSegment(Cattrib,$Sclass,Prob) : 1.0)]
        GraphCreateGaussianCurveRegion EMSegment(Graph,$i,Data,$Sclass) $mean $cov $Prob 2 $EMSegment(Graph,$i,Dimension) $EMSegment(Graph,$i,Xmin) $EMSegment(Graph,$i,Xmax) $EMSegment(Graph,$i,Xlen) $EMSegment(Graph,$i,Ymin) $EMSegment(Graph,$i,Ymax) $EMSegment(Graph,$i,Ylen) 
    }
    }
}

#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::ConvertToHex { RGB } { 
    set result "#"
    foreach COL $RGB { 
    set norm [expr int($COL * 255)]
    set result "${result}[format "%02x" $norm]"
    }
    return $result
}


#-------------------------------------------------------------------------------
# .PROC EMSegmentChangeVolumeGraph
# Changes the Volume of the graph for which everything is plotted
# .ARGS
# int VolumeID
# int numGraph
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::ChangeVolumeGraph {VolumeID volButtonIndex} {
    global EMSegment Volume
    # puts "EMSegmenterGraph::ChangeVolumeGraph $VolumeID $volButtonIndex"
    # volButtonIndex can only be 1 or 2 
    set EMSegment(Graph,$volButtonIndex,VolumeID,0) $VolumeID
    PlotCurveRegion $volButtonIndex

    # when we have two dimensional graph then it has to be properly updated 
    if {$EMSegment(NumGraph) == 3} {
       set EMSegment(Graph,2,VolumeID,$volButtonIndex) $VolumeID
       PlotCurveRegion 2
    }

    $EMSegment(Cl-mbGraphHistogram$volButtonIndex) configure -text $Volume($VolumeID,name)
}


#-------------------------------------------------------------------------------
# .PROC  EMSegmentUpdateClasses 
# Updates the class vlaues and plots the new curves 
# .ARGS
# boolean flag
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::UpdateClasses {flag} {
    global EMSegment
    # CalculateClassMeanCovariance
    CalcProb
    # Kilian - Just one 1D right now 
    for {set i 0} { $i < $EMSegment(NumGraph)} {incr i} {
    if {($EMSegment(Graph,$i,VolumeID,0)  == -1) && ($flag)} {
        DevErrorWindow "Please select a volume at the yellow graph button before trying to plot a histogram!"
        return -1
        } else {
        PlotCurveRegion $i
    }
    }
}

#-------------------------------------------------------------------------------
# .PROC PlotCurveRegion
# Plot the curves of a graph
# .ARGS
# int numGraph
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::PlotCurveRegion {numGraph} {
    global EMSegment Volume
    # Update History Plot
    if {$EMSegment(Graph,$numGraph,Dimension) == 1} { 
    set NumIndex [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,$numGraph,VolumeID,0)]
    # puts "PlotCurveRegion NumIndex $NumIndex  $EMSegment(SelVolList,VolumeList) | $EMSegment(Graph,$numGraph,VolumeID,0)"
    if {$NumIndex > -1} {
        EMSegment(Graph,$numGraph,Data,0)Accu SetInput [$Volume($EMSegment(Graph,$numGraph,VolumeID,0),node) GetImageData]
        EMSegment(Graph,$numGraph,Data,0)Accu Update
        EMSegment(Graph,$numGraph,Data,0) Update
    }

    # Update Classes
    foreach j $EMSegment(GlobalClassList) {
        if {$NumIndex > -1} {
        EMSegment(Graph,$numGraph,Data,$j) SetMean        [$_mrmlManager GetTreeNodeDistributionLogMean $j $NumIndex] 0  
        EMSegment(Graph,$numGraph,Data,$j) SetCovariance  [$_mrmlManager GetTreeNodeDistributionLogCovariance $j $NumIndex $NumIndex ] 0 0 
        # puts "EMSegment(Graph,$numGraph,Data,$j) SetCovariance [$_mrmlManager GetTreeNodeDistributionLogMean $j $NumIndex] 0 0" 
        }
        EMSegment(Graph,$numGraph,Data,$j) SetProbability [expr ($EMSegment(Graph,DisplayProb) > 0 ?  $EMSegment(Cattrib,$j,Prob) : 1.0)]
        EMSegment(Graph,$numGraph,Data,$j) Update
    }
    } else {
    set NumIndex(0) [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,$numGraph,VolumeID,0)]
    set NumIndex(1) [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,$numGraph,VolumeID,1)]
    # Update Classes
    foreach j $EMSegment(GlobalClassList) {
        if {($NumIndex(0) > -1) && ($NumIndex(1) > -1)} {
        for {set y 0 } { $y < 2} {incr y} {
            EMSegment(Graph,$numGraph,Data,$j) SetMean     [$_mrmlManager GetTreeNodeDistributionLogMean $j $NumIndex($y)]  $y
            for {set x 0 } {$x < 2} {incr x} {
            EMSegment(Graph,$numGraph,Data,$j) SetCovariance [$_mrmlManager GetTreeNodeDistributionLogCovariance $j $NumIndex($y) $NumIndex($x) ] $y $x 
            # puts "EMSegment(Graph,$numGraph,Data,$j) SetCovariance [$_mrmlManager GetTreeNodeDistributionLogCovariance $j $NumIndex($y) $NumIndex($x) ] $y $x "
            }
        }
        } 
        EMSegment(Graph,$numGraph,Data,$j) SetProbability [expr ($EMSegment(Graph,DisplayProb) > 0 ?  $EMSegment(Cattrib,$j,Prob) : 1.0)]
        EMSegment(Graph,$numGraph,Data,$j) Update
    }
    }
    GraphRender EMSegment $EMSegment(Graph,$numGraph,path)
}

#-------------------------------------------------------------------------------
# .PROC EMSegmentCalculateClassMeanCovariance
# Calculates  the mean and Sigma 
# .ARGS
# .END
#-------------------------------------------------------------------------------
proc CalculateClassMeanCovariance { } {
    global EMSegment 

    # save old values for later change check
    set flag 1
    foreach ID $EMSegment(GlobalClassList) {
    for {set y 0} {$y < $EMSegment(NumInputChannel)} {incr y} {
        if {[catch {set oldMean($y)  $EMSegment(Cattrib,$ID,LogMean,$y)} ] } {
        set oldMean($y) -1
        }
        for {set x 0} {$x < $EMSegment(NumInputChannel)} {incr x} {
        if {[catch { set oldCovariance($y,$x) $EMSegment(Cattrib,$ID,LogCovariance,$y,$x) } ] } { 
            set oldCovariance($y,$x) -1
        }
        }
    }

    # don't use sample values
    for {set y 0} {$y < $EMSegment(NumInputChannel)} {incr y} {
        set EMSegment(Cattrib,$ID,LogMean,$y)  [$_mrmlManager GetTreeNodeDistributionLogMean $ID $y]
        for {set x 0} {$x < $EMSegment(NumInputChannel)} {incr x} { 
        set EMSegment(Cattrib,$ID,LogCovariance,$y,$x)  [$_mrmlManager GetTreeNodeDistributionLogCovariance $ID $y $x ]
        }
    }

    # check to see if values have changed from before
    if { $flag == 0 } {
        for {set y 0} {$y < $EMSegment(NumInputChannel)} {incr y} {
        if {$EMSegment(Cattrib,$ID,LogMean,$y) != $oldMean($y) } { set flag 1}
        for {set x 0} {$x < $EMSegment(NumInputChannel)} {incr x} {
            if { $EMSegment(Cattrib,$ID,LogCovariance,$y,$x)  != $oldCovariance($y,$x)} { set flag 1}
        }
        }
    }
    }
    return $flag
}

#-------------------------------------------------------------------------------
# .PROC EMSegmentCalcProb
# Norm the Probablilities  => all are between 0 an 1 
# .ARGS
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::CalcProb { } { 
    global EMSegment
    foreach nodeID $EMSegment(GlobalSuperClassList) { 
    set NormProb [$_mrmlManager  GetTreeNodeChildrenSumClassProbability $nodeID]
    # Round it up or down otherwise it recalculates everything every time this function is called  
    if {($NormProb != 0.0) && ($NormProb != 1.0)} {
        set numChildren [ $_mrmlManager GetTreeNodeNumberOfChildren $nodeID ] 
        for {set i 0 } { $i < $numChildren } { incr i }  {
        set childID [ $_mrmlManager GetTreeNodeChildNodeID $nodeID $i ] 
        set prob    [ $_mrmlManager GetTreeNodeClassProbability $childID ]
        set EMSegment(Cattrib,$childID,Prob) [expr round($prob  / $NormProb * 100) / 100.0]
        }
    }
    }
}


#-------------------------------------------------------------------------------
# .PROC EMSegmentMultipleDrawDeleteCurveRegion
# Draws or delete curves/regions in all exisitng graphs 
# in the graph
# .ARGS
# string Sclass
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::MultipleDrawDeleteCurveRegion {Sclass} {
    global EMSegment
    for {set i 0} { $i < $EMSegment(NumGraph)} {incr i} {
       if {[DrawDeleteCurveRegion $Sclass $i] < 1} {return}
    }
}

 #-------------------------------------------------------------------------------
# .PROC EMSegmentDrawDeleteCurveRegion
# Depending if the graph for the class <Sclass> exist it deletes it or 
# otherwise cretes a new one
# .ARGS
# string Sclass
# int NumGraph
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::DrawDeleteCurveRegion {Sclass NumGraph} {
    global EMSegment Volume

    set ClassIndex [lsearch -exact $EMSegment(GlobalClassList) $Sclass]

    if {$EMSegment(Graph,$NumGraph,ID,$Sclass) > -1} {
       GraphRemoveCurve EMSegment $EMSegment(Graph,$NumGraph,path) $EMSegment(Graph,$NumGraph,ID,$Sclass)
       set EMSegment(Graph,$NumGraph,ID,$Sclass) -1
       # Delete Line and raise button
       if {$Sclass > 0} {
       # Lower Button  
       if {$ClassIndex < $EMSegment(Graph,ButtonNum)} { $EMSegment(Cl-fGraphButtons).bGraphButton$Sclass configure -relief raised
       } else { $EMSegment(Cl-fGraphButtonsBelow).bGraphButton$Sclass configure -relief raised }
       } else {
         $EMSegment(Cl-mbGraphHistogram$NumGraph) configure -relief raised
       }
    } else {
    # Draw Graph and lower button
      set result 0
      if {$Sclass > 0} {
        # Check Setting
        if {$EMSegment(NumInputChannel) == 0} {
            DevErrorWindow "Before triing to plot a graph please do the following :\n 1.) Define volume(s) to be segmented \n 2.) Define which volume's parameters to plot by selecting one at the yellow graph button"
            return -1
        } 
    # CalculateClassMeanCovariance

        if {$NumGraph < 2} {
           if { $EMSegment(Graph,$NumGraph,VolumeID,0) == -1 } {
              DevErrorWindow "Please define which volume's parameters to plot for graph [expr $NumGraph+1] by selecting one at the yellow graph button"
              return -1
           }
           # set index [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,$NumGraph,VolumeID,0)]
           # if {($EMSegment(Cattrib,$Sclass,LogMean,$index) < 0) || ($EMSegment(Cattrib,$Sclass,LogCovariance,$index,$index) < 0)} { 
           #   DevErrorWindow "Before graph can be plotted Mean and Covariance have to be defined for Volume [Volume($index,node) GetName]!"
           #   return -1
           #}
        } 
      } 
      # Draw Curve
      CalcProb
      set flag [expr ($Sclass > 0 ? 0 : 1)]
      if { !$flag } {
      # puts "Draw EMSegment(Graph,$NumGraph,Data,$Sclass)" 
          set EMSegment(Graph,$NumGraph,ID,$Sclass) [GraphAddCurveRegion EMSegment $EMSegment(Graph,$NumGraph,path) [EMSegment(Graph,$NumGraph,Data,$Sclass) GetOutput] [GraphHexToRGB [string range $EMSegment(Cattrib,$Sclass,ColorCode) 1 6]] $flag $flag]
      } else {
         set EMSegment(Graph,$NumGraph,ID,$Sclass) [GraphAddCurveRegion EMSegment $EMSegment(Graph,$NumGraph,path) [EMSegment(Graph,$NumGraph,Data,$Sclass) GetOutput] [GraphHexToRGB [string range $EMSegment(Cattrib,$Sclass,ColorGraphCode) 1 6]] $flag $flag]
      }
 
      if {$Sclass > 0} {
        if {$ClassIndex < $EMSegment(Graph,ButtonNum)} { $EMSegment(Cl-fGraphButtons).bGraphButton$Sclass configure -relief sunken
        } else { $EMSegment(Cl-fGraphButtonsBelow).bGraphButton$Sclass configure -relief sunken }
      } else {
        $EMSegment(Cl-mbGraphHistogram$NumGraph) configure -relief raised
      }
  }
  return 1
}



#-------------------------------------------------------------------------------
# .PROC EMSegmentGraphXAxisUpdate path Xmin Xmax Xsca 
# Called from Graph when X axis is update 
# .ARGS
# string path
# int Xmin
# int Xmax
# int Xsca
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::GraphXAxisUpdate {path Xmin Xmax Xsca} {
    global EMSegment
    set NumGraph 0  
    while { ($NumGraph < $EMSegment(NumGraph)) && ($EMSegment(Graph,$NumGraph,path) != $path)   } {incr NumGraph}
    if {$NumGraph == $EMSegment(NumGraph)} { 
    puts "EMSegmentGraphXAxisUpdate:Error: Could not find graph with path $path" 
    return
    }

    # Could be done nicer but works right now
    set EMSegment(Graph,$NumGraph,Xmin) $Xmin
    set EMSegment(Graph,$NumGraph,Xmax) $Xmax
    set EMSegment(Graph,$NumGraph,Xsca) $Xsca
    if {$NumGraph < 2} {
      # Update Histogram
      set dist [expr $Xmax - $Xmin]
      EMSegment(Graph,$NumGraph,Data,0)Accu SetComponentOrigin $Xmin 0.0 0.0 
      EMSegment(Graph,$NumGraph,Data,0)Accu SetComponentExtent 0 [expr int($dist - 1)] 0 0 0 0
      EMSegment(Graph,$NumGraph,Data,0)Accu UpdateWholeExtent
      EMSegment(Graph,$NumGraph,Data,0)Accu Update
    # If only the scalling changed we do not have to go through all the fuss
    if  {[ expr int($dist * [EMSegment(Graph,$NumGraph,Data,0) GetAxisMagnificationFactor 0 ""])]  != $EMSegment(Graph,$path,Xlen)} {
        set XInvUnit $EMSegment(Graph,$path,XInvUnit) 
        EMSegment(Graph,$NumGraph,Data,0)  SetAxisMagnificationFactor 0 $XInvUnit
        EMSegment(Graph,$NumGraph,Data,0)  Update

        set extent [[EMSegment(Graph,$NumGraph,Data,0) GetOutput] GetExtent]
        while {[expr [lindex $extent 1] - [lindex $extent 0] + 1] <  $EMSegment(Graph,$path,Xlen) } {
        set XInvUnit [expr $XInvUnit * 1.001]
        EMSegment(Graph,$NumGraph,Data,0) SetAxisMagnificationFactor 0 $XInvUnit
        EMSegment(Graph,$NumGraph,Data,0) Update 
        set extent [[EMSegment(Graph,$NumGraph,Data,0) GetOutput] GetExtent]
        }
    } else {
        EMSegment(Graph,$NumGraph,Data,0)  Update
    }
    }
    # Update Classes
    foreach i $EMSegment(GlobalClassList) {
    EMSegment(Graph,$NumGraph,Data,$i) SetXmin $Xmin
    EMSegment(Graph,$NumGraph,Data,$i) SetXmax $Xmax
    EMSegment(Graph,$NumGraph,Data,$i) Update
    }
    
    if {$EMSegment(NumGraph) > 2} {
    if {$NumGraph == 2} {
        if {($EMSegment(Graph,0,Xmin) != $EMSegment(Graph,2,Xmin)) ||  ($EMSegment(Graph,0,Xmax) != $EMSegment(Graph,2,Xmax)) ||  ($EMSegment(Graph,0,Xsca) != $EMSegment(Graph,2,Xsca))} {
        _GraphRescaleAxis EMSegment $EMSegment(Graph,0,path) $Xmin $Xmax $Xsca 0
        }
    } else { 
 if {($EMSegment(Graph,$NumGraph,Xmin) != $EMSegment(Graph,2,Xmin)) ||  ($EMSegment(Graph,$NumGraph,Xmax) != $EMSegment(Graph,2,Xmax)) ||  ($EMSegment(Graph,$NumGraph,Xsca) != $EMSegment(Graph,2,Xsca))} {
        _GraphRescaleAxis EMSegment $EMSegment(Graph,2,path) $Xmin $Xmax $Xsca $NumGraph
          }
        }
    }
}



#-------------------------------------------------------------------------------
# .PROC EMSegmentGraphYAxisUpdate path Ymin Ymax Ysca 
# Called from Graph when Y axis is update 
# .ARGS
# string path
# int Ymin
# int Ymax
# int Ysca
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::GraphYAxisUpdate {path Ymin Ymax Ysca} {
    global EMSegment
    set NumGraph 0  
    while { ($NumGraph < $EMSegment(NumGraph)) && ($EMSegment(Graph,$NumGraph,path) != $path)   } {incr NumGraph}
    if {$NumGraph == $EMSegment(NumGraph)} { 
    puts "EMSegmentGraphYAxisUpdate:Error: Could not find graph with path $path" 
    return
    }
    if {$EMSegment(Graph,$NumGraph,Dimension) != 2 } {return }

    # Could be done nicer but works right now
    set EMSegment(Graph,$NumGraph,Ymin) $Ymin
    set EMSegment(Graph,$NumGraph,Ymax) $Ymax
    set EMSegment(Graph,$NumGraph,Ysca) $Ysca
    
    # Update Classes
    foreach i $EMSegment(GlobalClassList) {
    EMSegment(Graph,$NumGraph,Data,$i) SetYmin $Ymin
    EMSegment(Graph,$NumGraph,Data,$i) SetYmax $Ymax
    EMSegment(Graph,$NumGraph,Data,$i) Update
    }
    if {($EMSegment(Graph,1,Xmin) != $EMSegment(Graph,2,Ymin)) ||  ($EMSegment(Graph,1,Xmax) != $EMSegment(Graph,1,Ymax)) ||  ($EMSegment(Graph,1,Xsca) != $EMSegment(Graph,2,Ysca))} {
        _GraphRescaleAxis EMSegment $EMSegment(Graph,1,path) $Ymin $Ymax $Ysca 0
    }
}

#-------------------------------------------------------------------------------
# .PROC EMSegmentCreateDisplayRedLine 
# Creates or displays a red line on the graph 
# .ARGS
# int NumGraph
# string Value
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::CreateDisplayRedLine {NumGraph Value} {
    global EMSegment
    set path $EMSegment(Graph,$NumGraph,path)
    if {$EMSegment(Graph,$NumGraph,LineID,0) < 0} {
    set EMSegment(Graph,$NumGraph,LineID,0) [GraphCreateLine EMSegment $path "1 0 0" 0]
    GraphCreateDisplayValues EMSegment $path $Value 0 1
    } else {
    GraphCreateDisplayValues EMSegment $path $Value 1 1
    }
    set Xcoord [GraphTransformValueInCoordinate  EMSegment $path $Value 0]
    GraphSetLineCoordinates EMSegment $path $EMSegment(Graph,$NumGraph,LineID,0) "$Xcoord 0 $Xcoord $EMSegment(Graph,$NumGraph,Ylen)"
    GraphRender EMSegment $path
    # Value should be displayed
}

#-------------------------------------------------------------------------------
# .PROC EMSegmentCreateDisplayRedCross 
# Creates or displays a cross line on the graph 
# .ARGS
# int NumGraph
# int Xvalue
# int Yvalue
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::CreateDisplayRedCross {NumGraph Xvalue Yvalue} {
    global EMSegment
    set path $EMSegment(Graph,$NumGraph,path)
    if {$EMSegment(Graph,$NumGraph,LineID,0) < 0} {
    set EMSegment(Graph,$NumGraph,LineID,0) [GraphCreateLine EMSegment $path "1 0 0" 0]
    set EMSegment(Graph,$NumGraph,LineID,1) [GraphCreateLine EMSegment $path "1 0 0" 0]
    GraphCreateDisplayValues EMSegment $path "$Xvalue $Yvalue" 0 1
    } else {
    GraphCreateDisplayValues EMSegment $path "$Xvalue $Yvalue" 1 1
    }

    set Xcoord [GraphTransformValueInCoordinate EMSegment $path $Xvalue 0]
    set Ycoord [GraphTransformValueInCoordinate EMSegment $path $Yvalue 1]
    GraphSetLineCoordinates EMSegment $path $EMSegment(Graph,$NumGraph,LineID,0) "$Xcoord [expr $Ycoord-2] $Xcoord [expr $Ycoord+3]"
    GraphSetLineCoordinates EMSegment $path $EMSegment(Graph,$NumGraph,LineID,1) "[expr $Xcoord -2] $Ycoord [expr $Xcoord+3] $Ycoord"
    GraphRender EMSegment $path
    # Value should be displayed
    # GraphCreateDisplayValues EMSegment $EMSegment(Graph,0,path) $ValueList 1 1
}


#-------------------------------------------------------------------------------
# .PROC EMSegmentEnterDisplaySample 
# Sets up everything when the mouse enters the graph with the image to be sampled
# or if DisplayFlag is set it just adjus values/lines/cross to the current position 
# of the mouse 
# .ARGS
# int x
# int y
# .END
#-------------------------------------------------------------------------------
itcl::body EMSegmenterGraph::EnterDisplaySample {x y} {
    global EMSegment Gui
    if {$EMSegment(DisplaySampleFlag) || ([wm state $Gui(wEMSegment)] == "withdrawn") || ($EMSegment(Graph,0,VolumeID,0) < 0)} {return}
    set EMSegment(DisplaySampleFlag) 1
    set SampleList [EMSegmentReadGreyValue $x $y 0]
    set Value(0) [lindex $SampleList [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,0,VolumeID,0)]] 
    EMSegmentCreateDisplayRedLine 0 $Value(0)
    if {$EMSegment(NumGraph) > 1} {
    if {$EMSegment(Graph,1,VolumeID,0) <  0 } { 
       set EMSegment(DisplaySampleFlag) 0
       return 
    }
    set Value(1) [lindex $SampleList [lsearch -exact $EMSegment(SelVolList,VolumeList) $EMSegment(Graph,1,VolumeID,0)]] 
    EMSegmentCreateDisplayRedLine 1 $Value(1)
    if {$EMSegment(NumGraph) > 2} { EMSegmentCreateDisplayRedCross 2 $Value(0) $Value(1) }
    }
    set EMSegment(DisplaySampleFlag) 0
}

#---------------

itcl::body EMSegmenterGraph::DeleteRedMarker { graphID } {
    global EMSegment
    if {$EMSegment(Graph,$graphID,LineID,0) < 0} {
    return
    }
    GraphDeleteLine EMSegment $EMSegment(Graph,$graphID,path) $EMSegment(Graph,$graphID,LineID,0)
    set EMSegment(Graph,$graphID,LineID,0) -1
    if { $graphID == 2 } {
    GraphDeleteLine EMSegment $EMSegment(Graph,$graphID,path) $EMSegment(Graph,$graphID,LineID,1)
    set EMSegment(Graph,$graphID,LineID,1) -1
    }

}

#---------------

itcl::body EMSegmenterGraph::GetGUICallbackCommand { ID event } {
    global EMSegment Volume
    if { $event == 1 } {
      DeleteRedMarker 0
      DeleteRedMarker 1 
      DeleteRedMarker 2
    } 

    # Define Window
    if { $ID == 0 } {
     set sliceGUI $_sliceGUI0
     set ri $_renInteractor0
    } elseif { $ID == 1 } {
      set sliceGUI $_sliceGUI1
      set ri $_renInteractor1
    } else {
      set sliceGUI $_sliceGUI2
      set ri $_renInteractor2
    }
      #    [eval $$tmp]
    #    set tmp _renInteractor$ID
    #   set ri [eval $$tmp]

    # Get Position
    set sliceNode [[$sliceGUI  GetLogic] GetSliceNode]
    set RAS "[lrange [ eval [$sliceNode GetXYToRAS] MultiplyPoint [$ri GetEventPosition] 0 1] 0 2 ] 1"
 
    # Retrieve Intensity values 
    set 2DFlag [expr [llength $EMSegment(SelVolList,VolumeList)] > 1 ? 1 : 0 ]
    set index 0
    set result ""
    foreach vol $EMSegment(SelVolList,VolumeList) {
      set RASToIJK [vtkMatrix4x4 New]
      $Volume($vol,node) GetRASToIJKMatrix $RASToIJK
      set IJK [eval $RASToIJK MultiplyPoint $RAS] 
      $RASToIJK Delete
      set xIJK [expr round([lindex $IJK 0])]
      set yIJK [expr round([lindex $IJK 1])]
      set zIJK [expr round([lindex $IJK 2])]

      scan [$Volume($vol,data) GetExtent]  "%d %d %d %d %d %d" Xmin Xmax Ymin Ymax Zmin Zmax
      if {[expr (($Xmin > $xIJK) || ($Xmax < $xIJK) || ($Ymin > $yIJK) || ($Ymax < $yIJK) || ($Zmin > $zIJK) || ($Zmax < $zIJK))]} {
        set 2DFlag 0
        DeleteRedMarker $index
      } else {
        # update line   
        set pixel [$Volume($vol,data) GetScalarComponentAsFloat $xIJK $yIJK $zIJK 0]
        set result "${result}$pixel "
        CreateDisplayRedLine $index $pixel
      }
      incr index
    }

    # Update Cross
    if {$2DFlag } {
        eval CreateDisplayRedCross 2 $result
    } else {
       DeleteRedMarker 2
    }
    # puts "Update $xIJK $yIJK $zIJK $result" 
}

itcl::body EMSegmenterGraph::AssignDefaultVolumes { } {
    global EMSegment
    ChangeVolumeGraph [lindex $EMSegment(SelVolList,VolumeList) 0 ] 0
    if {$EMSegment(NumGraph) > 1} {
    ChangeVolumeGraph [lindex $EMSegment(SelVolList,VolumeList) 1 ] 1
    }
}

#-------------------------------------------------------------------------------
# .PROC DevAddLabel
#
#  Creates a label.<br>
#  Example:  DevAddLabel $f.lmylabel \"Have a nice day\"
#
# 
# .ARGS
#  str LabelName  Name of the button (i.e. $f.stuff.lmylabel)
#  str Message    The text on the label
#  str Color      Label color and attribs from Gui.tcl (BLA or WLA). Optional
# .END
#-------------------------------------------------------------------------------
proc DevAddLabel { LabelName Message {Color WLA}} {
    global Gui
    eval {label $LabelName -text $Message} $Gui($Color)
}



proc rerun { } {
    global EMSegment
    catch {
    itcl::delete object c
    } 
    catch {
    itcl::delete class  EMSegmenterGraph
    }
    source ../Tcl/GenerateGraph.tcl
    source ../Tcl/Graph.tcl
    source ../Tcl/Gui.tcl

    EMSegmenterGraph c
    c CreateWindow
    c AssignDefaultVolumes
    # press button
    c MultipleDrawDeleteCurveRegion  [lindex $EMSegment(GlobalClassList) 0]
}
