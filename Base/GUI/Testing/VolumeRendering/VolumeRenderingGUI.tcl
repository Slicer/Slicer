#
# Editor GUI Procs
#
global this
proc VolumeRenderingConstructor {this} {
}

proc VolumeRenderingDestructor {this} {

    if {0} {
      set editBoxes [itcl::find objects -class EditBox]
      foreach e $editBoxes {
        itcl::delete object $e
      }
    }
}


# Note: not a method - this is invoked directly by the GUI
# - remove the GUI from the current Application
# - re-source the code (this file)
# - re-build the GUI
# Note: not a method - this is invoked directly by the GUI
proc VolumeRenderingTearDownGUI {this} {

  # nodeSelector  ;# disabled for now
  set widgets {
   rw vpw nodeSelector
  }

  foreach w $widgets {
    $::VR($this,w) SetParent ""
    $::VR($this,w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "VolumeRendering"]
    [$this GetUIPanel] RemovePage "VolumeRendering"
  }

}

proc VolumeRenderingBuildGUI {this} {
    #Save the Singleton
    if { [info exists ::VR(singleton)] } {
        error "VolumeRendering singleton already created"
    }
    set ::VR(singleton) $this

    #
    # create and register the node class
    # - since this is a generic type of node, only do it if
    #   it hasn't already been done by another module
    #
    set mrmlScene [[$this GetLogic] GetMRMLScene]
    set tag [$mrmlScene GetTagByClassName "vtkMRMLScriptedModuleNode"]
    if { $tag == "" } {
        set node [vtkMRMLScriptedModuleNode New]
        $mrmlScene RegisterNodeClass $node
        $node Delete
    }

    #Add Module Volume Rendering
    [$this GetUIPanel] AddPage "VolumeRendering" "VolumeRendering" ""
    set pageWidget [[$this GetUIPanel] GetPageWidget "VolumeRendering"]

    #
    # help frame shown in Slicer
    #
    set helptext "VolumeRendering. 3D Segmentation This module is currently a prototype and will be under active development throughout 3DSlicer's Beta release."
    set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
    $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


    set ::VR($this,cFrameVolumes) [vtkSlicerModuleCollapsibleFrame New]
    $::VR($this,cFrameVolumes) SetParent $pageWidget
    $::VR($this,cFrameVolumes) Create
    $::VR($this,cFrameVolumes) SetLabelText "Volumes"
    pack [$::VR($this,cFrameVolumes) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

    #Create a NodeSelector for  vtkMRMLScalarVolumeNode
    set ::VR($this,nodeSelector) [vtkSlicerNodeSelectorWidget New]
    puts "nodeSelector ID $::VR($this,nodeSelector)"
    $::VR($this,nodeSelector) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,nodeSelector) Create
    $::VR($this,nodeSelector) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::VR($this,nodeSelector) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
    $::VR($this,nodeSelector) UpdateMenu
    $::VR($this,nodeSelector) SetLabelText "Source Volume:"
    $::VR($this,nodeSelector) SetBalloonHelpString "The Source Volume will define the dimensions and directions for the new label map"
    pack [$::VR($this,nodeSelector) GetWidgetName] -side top -anchor e -padx 2 -pady 2

    #Button to load Node
    set ::VR($this,buttonLoadNode) [vtkKWPushButton New]
    puts "buttonLoadNode: $::VR($this,buttonLoadNode)"
    $::VR($this,buttonLoadNode) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonLoadNode) Create
    $::VR($this,buttonLoadNode) SetText "Load Node"
    $::VR($this,buttonLoadNode) SetBalloonHelpString "Load selected for Volume Rendering"
    pack [$::VR($this,buttonLoadNode) GetWidgetName] -side top -anchor e -padx 2 -pady 2

    #Button to make all Models invisible
    set ::VR($this,buttonAllModelsInvisible) [vtkKWPushButton New]
    puts "buttonLoadNode: $::VR($this,buttonAllModelsInvisible)"
    $::VR($this,buttonAllModelsInvisible) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonAllModelsInvisible) Create
    $::VR($this,buttonAllModelsInvisible) SetText "AllModelsInvisible"
    $::VR($this,buttonAllModelsInvisible) SetBalloonHelpString "Make all models invisible"
    pack [$::VR($this,buttonAllModelsInvisible) GetWidgetName] -side top -anchor e -padx 0 -pady 2
    

    #GetInstance of RenderWidget
    set ::VR($this,renderWidget) [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
    set ::VR($this,renWin) [$::VR($this,renderWidget) GetRenderWindow]
    #    $::VR($this,renWin) AddObserver AbortCheckEvent {TkCheckAbort}
    puts "renderWidget: $::VR($this,renderWidget)"
    # Create a volume property widget
    set ::VR($this,vpw) [vtkKWVolumePropertyWidget New]
    puts "vtkKWVolumePropertyWidget: $::VR($this,vpw)"
    $::VR($this,vpw) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,vpw) ComponentWeightsVisibilityOff
    $::VR($this,vpw) SetWindowLevel 128 128
    $::VR($this,vpw) Create
    #No Observer:performance!
    #$::VR($this,vpw) AddObserver AnyEvent "$::VR($this,renderWidget) Render"
    pack [$::VR($this,vpw) GetWidgetName] -side top -anchor nw -expand y -padx 2 -pady 2
    
    #Scale for opacity
    set ::VR($this,opacityScale) [vtkKWScaleWithLabel New]
    puts "opactiyScale"
    $::VR($this,opacityScale) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,opacityScale) Create
    $::VR($this,opacityScale) SetLabelText "Opacity for labelmaps"
    [$::VR($this,opacityScale) GetWidget] SetRange 0 1
    [$::VR($this,opacityScale) GetWidget] SetResolution 0.1
    [$::VR($this,opacityScale) GetWidget] SetValue 0.2
    $::VR($this,opacityScale) SetBalloonHelpString "set opacity for labelmaps"
    $::VR($this,opacityScale) EnabledOff
    pack [$::VR($this,opacityScale) GetWidgetName] -side top -anchor nw -expand y -padx 2 -pady 2
    
    set ::VR($this,buttonOpacity) [vtkKWPushButton New]
    puts "buttonOpacity: $::VR($this,buttonAllModelsInvisible)"
    $::VR($this,buttonOpacity) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonOpacity) Create
    $::VR($this,buttonOpacity) SetText "OpactiyChange"
    $::VR($this,buttonOpacity) SetBalloonHelpString "Change Opacity"
    $::VR($this,buttonOpacity) EnabledOff
    pack [$::VR($this,buttonOpacity) GetWidgetName] -side top -anchor nw -padx 0 -pady 2
    
    set ::VR($this,buttonMIP) [vtkKWPushButton New]
    puts "buttonMIP: $::VR($this,buttonMIP)"
    $::VR($this,buttonMIP) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonMIP) Create
    $::VR($this,buttonMIP) SetText "USE MIP"
    $::VR($this,buttonMIP) SetBalloonHelpString "USE MIP"
    pack [$::VR($this,buttonMIP) GetWidgetName] -side top -anchor nw -padx 0 -pady 2
    
    set ::VR($this,buttonThreeD) [vtkKWPushButton New]
    puts "buttonThreeD: $::VR($this,buttonThreeD)"
    $::VR($this,buttonThreeD) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonThreeD) Create
    $::VR($this,buttonThreeD) SetText "USE 3D"
    $::VR($this,buttonThreeD) SetBalloonHelpString "USE 3D"
    pack [$::VR($this,buttonThreeD) GetWidgetName] -side top -anchor nw -padx 0 -pady 2
    puts "End VolumeRendering BuildGui"
    
        set ::VR($this,buttonTwoD) [vtkKWPushButton New]
    puts "buttonThreeD: $::VR($this,buttonThreeD)"
    $::VR($this,buttonTwoD) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonTwoD) Create
    $::VR($this,buttonTwoD) SetText "USE 2D"
    $::VR($this,buttonTwoD) SetBalloonHelpString "USE 2D"
    pack [$::VR($this,buttonTwoD) GetWidgetName] -side top -anchor nw -padx 0 -pady 2

    
    set ::VR($this,presetSelector) [vtkKWVolumePropertyPresetSelector New]
    puts "preset Selector"
    $::VR($this,presetSelector) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,presetSelector) Create
    pack [$::VR($this,presetSelector) GetWidgetName] -side top -anchor nw -padx 0 -pady 2
    
    
    set ::VR($this,presetMapping) [vtkKWMenuButtonWithLabel New]
    puts " preset Mapping"
    $::VR($this,presetMapping) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,presetMapping) Create
    $::VR($this,presetMapping) SetLabelText "Preset Mapping for CT"
    [[$::VR($this,presetMapping) GetWidget] GetMenu] AddRadioButton "Bone"
    [[$::VR($this,presetMapping) GetWidget] GetMenu] AddRadioButton "Vessels"
    [[$::VR($this,presetMapping) GetWidget] GetMenu] AddRadioButton "Bone and Vessels"
    [[$::VR($this,presetMapping) GetWidget] GetMenu] AddRadioButton "T1"
    
    pack [$::VR($this,presetMapping) GetWidgetName] -side top -anchor nw -padx 0 -pady 2
        puts "End VolumeRendering BuildGui"
}
proc VolumeRenderingEnter {this} {
puts "Enter"
}
proc VolumeRenderingAddGUIObservers {this} {
    $this AddObserverByNumber $::VR($this,buttonLoadNode) 10000
    $this AddObserverByNumber $::VR($this,buttonAllModelsInvisible) 10000
    $this AddObserverByNumber $::VR($this,buttonOpacity) 10000
    $this AddObserverByNumber $::VR($this,buttonMIP) 10000
    $this AddObserverByNumber $::VR($this,buttonThreeD) 10000
    $this AddObserverByNumber $::VR($this,buttonTwoD) 10000
    $this AddObserverByNumber  [[$::VR($this,presetMapping) GetWidget] GetMenu] 10005
    [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] AddObserver AbortCheckEvent {TkCheckAbort}

    puts "Observer Added VolumeRendering"
}

proc VolumeRenderingRemoveGUIObservers {this} {

}

proc VolumeRenderingRemoveLogicObservers {this} {
}

proc VolumeRenderingRemoveMRMLNodeObservers {this} {
}

proc VolumeRenderingProcessLogicEvents {this caller event} {
}

proc VolumeRenderingProcessGUIEvents {this caller event} {
     puts "VolumeRenderingEvents: event = $event, callerID = $caller"
    #Load selected volume
    if { $caller == $::VR($this,buttonLoadNode) } {
        switch $event {
            "10000" {
            puts "buttonLoadNode"
            buttonLoadNode $this
            } ;#Switch inner
        } ;#Switch

    } elseif {$caller == $::VR($this,buttonAllModelsInvisible)} {
        switch $event {
            "10000" {
                puts "Make all models invisible"
                allModelsInvisible $this
            } ;#Switch inner
        } ;#Switch
    } elseif {$caller == $::VR($this,buttonOpacity)} {
        puts "update opacity"
        UpdateOpacity $this
    } elseif {$caller == $::VR($this,buttonMIP)} {
        puts "use MIP"
            set ::VR($this,volumeMapper) [vtkFixedPointVolumeRayCastMapper New]
            $::VR($this,volumeMapper) SetInput $::VR($this,aImageData)
            $::VR($this,volume) SetMapper $::VR($this,volumeMapper)
    } elseif {$caller == $::VR($this,buttonThreeD)} {
        puts "use 3D"
            set ::VR($this,volumeMapper) [vtkVolumeTextureMapper3D New]
            $::VR($this,volumeMapper) SetInput $::VR($this,aImageData)
            $::VR($this,volumeMapper) SetSampleDistance 1
            $::VR($this,volume) SetMapper $::VR($this,volumeMapper)
    }  elseif {$caller == $::VR($this,buttonTwoD)} {
            set ::VR($this,converter) [vtkImageShiftScale New]
            $::VR($this,converter) SetOutputscalarTypeToUnsignedShort 
            $::VR($this,converter) SetInput $::VR($this,aImageData)
            set ::VR($this,volumeMapper) [vtkVolumeTextureMapper2D New]
            $::VR($this,volumeMapper) SetInput [$::VR($this,converter) GetOutput]
            $::VR($this,volume) SetMapper $::VR($this,volumeMapper)
        puts "use 2d"
    } elseif {$caller==[[$::VR($this,presetMapping) GetWidget] GetMenu]} {
        puts "something in the menu"
        switch [[$::VR($this,presetMapping) GetWidget] GetValue] {
            "Bone" {
                puts "Bone"
                $::VR($this,opacityTransferFunction) RemoveAllPoints
                $::VR($this,opacityTransferFunction) AddPoint [lindex [$::VR($this,pfed_hist) GetRange] 0] 0.0
                $::VR($this,opacityTransferFunction) AddPoint 1200 0.0
                $::VR($this,opacityTransferFunction) AddPoint 1300 0.2
                $::VR($this,opacityTransferFunction) AddPoint [lindex [$::VR($this,pfed_hist) GetRange] 1] 0.2
                [$::VR($this,vpw) GetScalarOpacityFunctionEditor] Update
                $::VR($this,colorTransferFunction) RemoveAllPoints
                $::VR($this,colorTransferFunction) AddRGBPoint [lindex [$::VR($this,pfed_hist) GetRange] 0] .3 .3 1
                $::VR($this,colorTransferFunction) AddRGBPoint 30 .3 .3 1
                $::VR($this,colorTransferFunction) AddRGBPoint 1000 .3 1 .3
                $::VR($this,colorTransferFunction) AddRGBPoint 1200 1 .3 .3
                $::VR($this,colorTransferFunction) AddRGBPoint 2200 1 1 0
                $::VR($this,colorTransferFunction) AddRGBPoint [lindex [$::VR($this,pfed_hist) GetRange] 1] 1 1 0
                [$::VR($this,vpw) GetScalarColorFunctionEditor] Update
                
            }
            "Vessels" {
            puts "Vessels"
            }
            "Bone and Vessels" {
            puts "Bone and Vessels"
            }
            "T1" {
            puts "T1"
            }
        } ;#switch
    
    } ;#elseif
    puts "Events processed"
} ;#procedure
proc allModelsInvisible {this} {
    set scene [$::slicer3::ApplicationLogic GetMRMLScene]
    set count [$scene GetNumberOfNodesByClass "vtkMRMLModelNode"]
    set index 0

    while {$index<$count} {
        set node [[$scene GetNthNodeByClass $index "vtkMRMLModelNode"] GetModelDisplayNode]
        $node VisibilityOff 
        incr index
    }
}
proc buttonLoadNode {this} {
    puts "Begin buttonLoadNode"
    if {[info exists ::VR($this,volume)]} {
        #Remove old volumes
        puts "remove old volumes"
        $::VR($this,renderWidget) RemoveViewProp $::VR($this,volume)
                    $::VR($this,opacityScale) EnabledOff
            $::VR($this,buttonOpacity) EnabledOff
    } else {
        puts "no old volumes"
    }
    

    set ::VR($this,aMRMLNODE) [$::VR($this,nodeSelector) GetSelected]
    #Something selected?
    if { $::VR($this,aMRMLNODE) == "" } {
        puts "noDATA"
    } else { ;#There is something selected
        puts "DATA $::VR($this,aMRMLNODE)"
        set ::VR($this,aImageData) [$::VR($this,aMRMLNODE) GetImageData]
        puts "278"
        
        
        #Build Histogram and mapping functions
        set ::VR($this,pfed_hist) [vtkKWHistogram New]
        puts "pfed_hist: $$::VR($this,pfed_hist)"
        $::VR($this,pfed_hist) BuildHistogram [[$::VR($this,aImageData) GetPointData] GetScalars] 0
        
        set ::VR($this,opacityTransferFunction) [vtkPiecewiseFunction New]
        puts "opacityTransferFunction: $::VR($this,opacityTransferFunction)"
        
        set ::VR($this,colorTransferFunction) [vtkColorTransferFunction New]
        puts "colorTransferFunction: $::VR($this,colorTransferFunction)"
        
        if {[$::VR($this,aMRMLNODE) GetLabelMap] == 1} {
            puts "volume is labelmap"
            #Look at labelmap for color
            #Get Lookup Table for RGB COLORS
            #Get Display Node
            $::VR($this,opacityScale) EnabledOn
            $::VR($this,buttonOpacity) EnabledOn
            set colorNode [[$::VR($this,aMRMLNODE) GetVolumeDisplayNode] GetColorNode]
            puts "Color: $colorNode"
            #$colorNode SetTypeToLabels
            set lookupRGB [$colorNode GetLookupTable]
            
            set lowerRange [lindex [$::VR($this,pfed_hist) GetRange] 0]
            set upperRange [lindex [$::VR($this,pfed_hist) GetRange] 1]
            set index [expr {$lowerRange + 1}]
            set opacity .2
            set countTreshold 10
            #$::VR($this,opacityTransferFunction) AddPoint $lowerRange 0
            #$::VR($this,opacityTransferFunction) AddPoint $upperRange $opacity
            if 1 {
                while {$index<$upperRange} {
                    if {[$::VR($this,pfed_hist) GetOccurenceAtValue $index]>$countTreshold} {
                        #puts "Color at $index"
                        ##Opacity
                        #Set Lower Points
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index - .5] 0
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index - .4999999] $opacity
                        #Set upper Point
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index + .5] 0
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index + .4999999] $opacity
                        
                        #Colors
                        set color [$lookupRGB GetColor $index]
                        #puts "$color"
                        $::VR($this,colorTransferFunction) AddRGBPoint [expr $index + .01] [lindex $color 0] [lindex $color 1] [lindex $color 2]
                        $::VR($this,colorTransferFunction) AddRGBPoint [expr $index + .99] [lindex $color 0] [lindex $color 1] [lindex $color 2]
                        
                        #puts "Color at $index finished"
                    } 
                    incr index
                } ;#while
            } ;#if
        } else {
            puts "normal grayscaledata"
            $::VR($this,opacityScale) EnabledOff
            # 2tresholds
            set treshold1  [expr [$::VR($this,pfed_hist) GetTotalOccurence]/20.]
            set treshold2 [expr [$::VR($this,pfed_hist) GetTotalOccurence]/2]

            #counters for loop
            set indexTreshold1 10
            set totalTreshold1 0
            set indexTreshold2 10
            set totalTreshold2 0

            #while {$totalTreshold1<$treshold1} {
            #incr totalTreshold1 [$::VR($this,pfed_hist) GetOccurenceAtValue $indexTreshold1]
            #incr indexTreshold1 
            #}
            #while {$totalTreshold2<$treshold2} {
            #incr totalTreshold2 [$::VR($this,pfed_hist) GetOccurenceAtValue $indexTreshold2]
            #incr indexTreshold2 
            #}
            
            $::VR($this,opacityTransferFunction) AddPoint [lindex [$::VR($this,pfed_hist) GetRange] 0] .0
            $::VR($this,opacityTransferFunction) AddPoint  $indexTreshold1  0.0    
            $::VR($this,opacityTransferFunction) AddPoint  $indexTreshold2  0.2
            $::VR($this,opacityTransferFunction) AddPoint [lindex [$::VR($this,pfed_hist) GetRange] 1] .2
            $::VR($this,colorTransferFunction) AddRGBPoint      [lindex [$::VR($this,pfed_hist) GetRange] 0] 0.3 0.3 1.0
            $::VR($this,colorTransferFunction) AddRGBPoint      [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.25] 0.3 0.3 1.0
            $::VR($this,colorTransferFunction) AddRGBPoint      [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.5] 0.3 1.0 0.3
            $::VR($this,colorTransferFunction) AddRGBPoint    [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.75] 1.0 0.3 0.3
            $::VR($this,colorTransferFunction) AddRGBPoint     [lindex [$::VR($this,pfed_hist) GetRange] 1] 1    .3    .3
        } ;#else
            

        set ::VR($this,volumeProperty) [vtkVolumeProperty New]
        puts "volumeProperty: $::VR($this,volumeProperty)"
            $::VR($this,volumeProperty) SetScalarOpacity $::VR($this,opacityTransferFunction)
            $::VR($this,volumeProperty) SetColor $::VR($this,colorTransferFunction)
            #$::VR($this,volumeProperty) SetGradientOpacity $gradientOpacityTransferFunction
            $::VR($this,volumeProperty) SetInterpolationTypeToNearest
            $::VR($this,volumeProperty) ShadeOff
        $::VR($this,presetSelector) SetPresetVolumeProperty 0 $::VR($this,volumeProperty)

        puts "334"
        #Switch here for MIP and Normal Mode
        if 0 {
            set ::VR($this,volumeMapper) [vtkFixedPointVolumeRayCastMapper New]
            $::VR($this,volumeMapper) SetInput $::VR($this,aImageData)
            }
            
        if 1 {
            set ::VR($this,volumeMapper) [vtkVolumeTextureMapper3D New]
            $::VR($this,volumeMapper) SetInput $::VR($this,aImageData)
            $::VR($this,volumeMapper) SetSampleDistance 0.1
        }
            puts "348"
        set ::VR($this,volume) [vtkVolume New]
        $::VR($this,volume) SetMapper $::VR($this,volumeMapper)
        $::VR($this,volume) SetProperty $::VR($this,volumeProperty)
        set ::VR($this,matrix) [vtkMatrix4x4 New]
        puts "after setMatrix"
        puts "after cast"
        $::VR($this,aMRMLNODE) GetIJKToRASMatrix $::VR($this,matrix)
        puts "after get"
        $::VR($this,volume) PokeMatrix $::VR($this,matrix)
        puts "PokeMatrix"
        #Add data to vtkKWVolumePropertyWidget
        $::VR($this,vpw) SetVolumeProperty $::VR($this,volumeProperty)
        $::VR($this,vpw) ComponentWeightsVisibilityOff
        #$::VR($this,vpw) Update

        #Add property to RenderWidget
        $::VR($this,renderWidget) AddViewProp $::VR($this,volume)
        #Add Histograms to Widget
        [$::VR($this,vpw) GetScalarOpacityFunctionEditor]  SetHistogram $::VR($this,pfed_hist)
        [$::VR($this,vpw) GetScalarColorFunctionEditor] SetHistogram $::VR($this,pfed_hist)
        #No Gradient
        #Update vtkKWVolumePropertyWidget
        $::VR($this,renderWidget) Create
            puts "Render ENDE"
    } ;#else
    puts "proc Load Node Processed"
}
#
# MRML Event processing
#
proc UpdateOpacity {this} {
puts "Begin proc UpdateOpacity"


 set lowerRange [lindex [$::VR($this,pfed_hist) GetRange] 0]
            set upperRange [lindex [$::VR($this,pfed_hist) GetRange] 1]
            set index 1
            set opacity [[$::VR($this,opacityScale) GetWidget] GetValue]
            puts "opacity $opacity"
            set countTreshold 10
            if 1 {
                while {$index<$upperRange} {
                    if {[$::VR($this,pfed_hist) GetOccurenceAtValue $index]>$countTreshold} {
                        #puts "Color at $index"
                        ##Opacity
                        #Set Lower Points
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index - .5] 0
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index - .4999999] $opacity
                        #Set upper Point
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index + .5] 0
                        $::VR($this,opacityTransferFunction) AddPoint [expr $index + .4999999] $opacity
                        
                    } ;#if
                    incr index
                } ;#while
                #$::VR($this,opacityTransferFunction) Update
                [$::VR($this,vpw) GetScalarOpacityFunctionEditor] Update
            } ;#if
puts "Ende proc UpdateOpacity"

}
proc VolumeRenderingUpdateMRML {this} {
}

proc VolumeRenderingProcessMRMLEvents {this callerID event} {
}

proc VRDebug {this output} {

    if 1 {
        puts "$output"
    }
}

proc TkCheckAbort {} {
set renWin [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow]
    set foo [$renWin GetEventPending]
    if {$foo != 0} {
    puts "Abort"
    $renWin SetAbortRender 1
    } 
}
#    renWin AddObserver AbortCheckEvent {TkCheckAbort}


