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

    #GetInstance of RenderWidget
    set ::VR($this,renderWidget) [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
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
    puts "End VolumeRendering BuildGui"

}
proc VolumeRenderingEnter {this} {
puts "Enter"
}
proc VolumeRenderingAddGUIObservers {this} {
    $this AddObserverByNumber $::VR($this,buttonLoadNode) 10000
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
    if { $caller == $::VR($this,buttonLoadNode) } {
        switch $event {
            "10000" {
            puts "buttonLoadNode"
            buttonLoadNode $this
            } ;#Switch inner
        } ;#Switch

    } ;#if
    puts "Events processed"
}
proc buttonLoadNode {this} {
puts "Begin buttonLoadNode"
    set ::VR($this,aMRMLNODE) [$::VR($this,nodeSelector) GetSelected]
    #Something selected?
    if { $::VR($this,aMRMLNODE) == "" } {
        puts "noDATA"
    } else { ;#There is something selected
        puts "DATA $::VR($this,aMRMLNODE)"
        set ::VR($this,aImageData) [$::VR($this,aMRMLNODE) GetImageData]
        set reader  $::VR($this,aImageData)
        puts "278"


            #Histogram bauen
              #$reader Update
              set pfed_hist [vtkKWHistogram New]
              puts "pfed_hist: $pfed_hist"
              $pfed_hist BuildHistogram \
                [[$reader GetPointData] GetScalars] 0
            puts "285"
            #$pfed_hist Update
            
            set opacityTransferFunction [vtkPiecewiseFunction New]
            puts "opacityTransferFunction: $opacityTransferFunction"
            
            set colorTransferFunction [vtkColorTransferFunction New]
            puts "colorTransferFunction: $colorTransferFunction"
            
            #mapping functions
                
            #Get Lookup Table for RGB COLORS
            #Get Display Node
            
            set colorNode [[$::VR($this,aMRMLNODE) GetVolumeDisplayNode] GetColorNode]
            puts "Color: $colorNode"
            #$colorNode SetTypeToLabels
            set lookupRGB [$colorNode GetLookupTable]
            
            set lowerRange [lindex [$pfed_hist GetRange] 0]
            set upperRange [lindex [$pfed_hist GetRange] 1]
            set index 1
            set opacity .2
            set countTreshold 10
            if 1 {
                while {$index<$upperRange} {
                    if {[$pfed_hist GetOccurenceAtValue $index]>$countTreshold} {
                        puts "Color at $index"
                        ##Opacity
                        #Set Lower Points
                        $opacityTransferFunction AddPoint [expr $index - .5] 0
                        $opacityTransferFunction AddPoint [expr $index - .4999999] $opacity
                        #Set upper Point
                        $opacityTransferFunction AddPoint [expr $index + .5] 0
                        $opacityTransferFunction AddPoint [expr $index + .4999999] $opacity
                        
                        #Colors
                        set color [$lookupRGB GetColor $index]
                        puts "$color"
                        $colorTransferFunction AddRGBPoint $index [lindex $color 0] [lindex $color 1] [lindex $color 2]
                        
                        puts "Color at $index finished"
                    }
                    incr index
                }
            }

        
          set gradientOpacityTransferFunction [vtkPiecewiseFunction New]
        puts "gradientOpacityTransferFunction: $gradientOpacityTransferFunction"
        $gradientOpacityTransferFunction AddPoint 1    1
            #$gradientOpacityTransferFunction AddPoint  41   0.0
            #$gradientOpacityTransferFunction AddPoint  45  1
            $gradientOpacityTransferFunction AddPoint 145    1

        set volumeProperty [vtkVolumeProperty New]
        puts "volumeProperty: $volumeProperty"
            $volumeProperty SetScalarOpacity $opacityTransferFunction
            $volumeProperty SetColor $colorTransferFunction
            $volumeProperty SetGradientOpacity $gradientOpacityTransferFunction
            $volumeProperty SetInterpolationTypeToNearest
            $volumeProperty ShadeOff

        puts "334"
        #Switch here for MIP and Normal Mode
        if 0 {
        set volumeMapperFunction [vtkVolumeRayCastMIPFunction New]
        $volumeMapperFunction SetMaximizeMethodToOpacity
        set volumeMapper [vtkVolumeRayCastMapper New]
        $volumeMapper SetInputConnection [$reader GetOutputPort]
        $volumeMapper SetVolumeRayCastFunction $volumeMapperFunction
        }
        if 1 {
        set volumeMapper [vtkVolumeTextureMapper3D New]
            $volumeMapper SetInput $reader
            $volumeMapper SetSampleDistance 0.1
        }
            puts "348"
        set volume [vtkVolume New]
        $volume SetMapper $volumeMapper
        $volume SetProperty $volumeProperty
        set matrix [vtkMatrix4x4 New]
        puts "after setMatrix"
        puts "after cast"
        $::VR($this,aMRMLNODE) GetIJKToRASMatrix $matrix
        puts "after get"
        $volume PokeMatrix $matrix
        puts "PokeMatrix"
        #Add data to vtkKWVolumePropertyWidget
        $::VR($this,vpw) SetVolumeProperty $volumeProperty
        $::VR($this,vpw) ComponentWeightsVisibilityOff
        $::VR($this,vpw) Update

        #Add property to RenderWidget
        $::VR($this,renderWidget) AddViewProp $volume
        #Add Histograms to Widget
        [$::VR($this,vpw) GetScalarOpacityFunctionEditor]  SetHistogram $pfed_hist
        [$::VR($this,vpw) GetScalarColorFunctionEditor] SetHistogram $pfed_hist
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

proc VolumeRenderingUpdateMRML {this} {
}

proc VolumeRenderingProcessMRMLEvents {this callerID event} {
}

proc VRDebug {this output} {

    if 1 {
        puts "$output"
    }
}


