#
# ClipModel GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc ClipModelConstructor {this} {
}

proc ClipModelDestructor {this} {
}

proc ClipModelTearDownGUI {this} {
    $::ClipModel($this,planes) Delete
    $::ClipModel($this,clipper) Delete
    
    # nodeSelector  ;# disabled for now
    set widgets {
        clip displayWidget modelsSelect roiSelect
        modelsOutputSelect modelsFrame insideOut
    }
    
    foreach w $widgets {
        $::ClipModel($this,$w) SetParent ""
        $::ClipModel($this,$w) Delete
    }
    
    if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
        set pageWidget [[$this GetUIPanel] GetPageWidget "ClipModel"]
        [$this GetUIPanel] RemovePage "ClipModel"
    }
    
    unset ::ClipModel(singleton)
    
}

proc ClipModelBuildGUI {this} {
    set ::ClipModel($this,processingMRML) ""
    set ::ClipModel($this,processingGUI) ""
    set ::ClipModel($this,roiNode) ""

    if { [info exists ::ClipModel(singleton)] } {
        error "ClipModel singleton already created"
    }
    set ::ClipModel(singleton) $this
    
    set ::ClipModel($this,planes) [vtkPlanes New]
    set ::ClipModel($this,clipper) [vtkClipPolyData New]
    $::ClipModel($this,clipper) SetClipFunction $::ClipModel($this,planes)
    $::ClipModel($this,clipper) InsideOutOn

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
    
    
    $this SetCategory "Surface Models"
    [$this GetUIPanel] AddPage "ClipModel" "ClipModel" ""
    set pageWidget [[$this GetUIPanel] GetPageWidget "ClipModel"]
    
    #
    # help frame
    #
    set helptext "The ClipModel clips model by an ROI using a box widget.\n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:ClipModel-Documentation-3.6</a>\n"
    set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\nThe Volumes module was contributed by Alex Yarmarkovich, Isomics Inc."
    $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext
    
    
    #
    # ClipModel Volumes
    #
    set ::ClipModel($this,modelsFrame) [vtkSlicerModuleCollapsibleFrame New]
    $::ClipModel($this,modelsFrame) SetParent $pageWidget
    $::ClipModel($this,modelsFrame) Create
    $::ClipModel($this,modelsFrame) SetLabelText "Models"
    pack [$::ClipModel($this,modelsFrame) GetWidgetName] \
        -side top -anchor nw -expand y -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]
    
    set ::ClipModel($this,modelsSelect) [vtkSlicerNodeSelectorWidget New]
    $::ClipModel($this,modelsSelect) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,modelsSelect) Create
    $::ClipModel($this,modelsSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ClipModel($this,modelsSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ClipModel($this,modelsSelect) SetNoneEnabled 1
    $::ClipModel($this,modelsSelect) UpdateMenu
    $::ClipModel($this,modelsSelect) SetLabelText "Source Model:"
    $::ClipModel($this,modelsSelect) SetBalloonHelpString "The Source Model to operate on"
    pack [$::ClipModel($this,modelsSelect) GetWidgetName] -side top -anchor w -padx 2 -pady 2 
     
    set ::ClipModel($this,roiSelect) [vtkSlicerNodeSelectorWidget New]
    $::ClipModel($this,roiSelect) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,roiSelect) Create
    $::ClipModel($this,roiSelect) SetNodeClass "vtkMRMLROINode" "" "" ""
    $::ClipModel($this,roiSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ClipModel($this,roiSelect) SetNoneEnabled 1
    $::ClipModel($this,roiSelect) NewNodeEnabledOn
    $::ClipModel($this,roiSelect) UpdateMenu
    $::ClipModel($this,roiSelect) SetLabelText "Cropping ROI:"
    $::ClipModel($this,roiSelect) SetBalloonHelpString "The ROI to use as cropping region"
    pack [$::ClipModel($this,roiSelect) GetWidgetName] -side top -anchor w -padx 2 -pady 2 
    
    set ::ClipModel($this,modelsOutputSelect) [vtkSlicerNodeSelectorWidget New]
    $::ClipModel($this,modelsOutputSelect) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,modelsOutputSelect) Create
    $::ClipModel($this,modelsOutputSelect) NewNodeEnabledOn
    $::ClipModel($this,modelsOutputSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ClipModel($this,modelsOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ClipModel($this,modelsOutputSelect) UpdateMenu
    $::ClipModel($this,modelsOutputSelect) SetLabelText "Clipped Model:"
    $::ClipModel($this,modelsOutputSelect) SetBalloonHelpString "The target output model"
    pack [$::ClipModel($this,modelsOutputSelect) GetWidgetName] -side top -anchor w -padx 2 -pady 2 
    
    set ::ClipModel($this,clip) [vtkKWCheckButton New]
    $::ClipModel($this,clip) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,clip) Create
    $::ClipModel($this,clip) SetText "Clipping  Enabled"
    $::ClipModel($this,clip) SetSelectedState 0
    $::ClipModel($this,clip) SetBalloonHelpString "Apply clipping"
    pack [$::ClipModel($this,clip) GetWidgetName] -side top -anchor w -padx 2 -pady 2 
    
    set ::ClipModel($this,insideOut) [vtkKWCheckButton New]
    $::ClipModel($this,insideOut) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,insideOut) Create
    $::ClipModel($this,insideOut) SetText "Inside Out  Enabled"
    $::ClipModel($this,insideOut) SetSelectedState 1
    $::ClipModel($this,insideOut) SetBalloonHelpString " When InsideOut is turned on, a vertex is considered inside if it is inside the ROI. When off, a vertex is considered inside if it is outside the ROI. InsideOut is on by default in this module."
    pack [$::ClipModel($this,insideOut) GetWidgetName] -side top -anchor w -padx 2 -pady 2 

    set ::ClipModel($this,displayWidget) [vtkSlicerROIDisplayWidget New]
    $::ClipModel($this,displayWidget) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,displayWidget) Create
    $::ClipModel($this,displayWidget) SetBalloonHelpString "Apply displayWidgeting"
    pack [$::ClipModel($this,displayWidget) GetWidgetName] -side top -anchor w -expand y -fill x -padx 2 -pady 2 
    
    set ::ClipModel($this,init) ""
    set ::ClipModel($this,oldOut) ""
}

proc ClipModelAddGUIObservers {this} {
    $this AddObserverByNumber $::ClipModel($this,clip) 10000 
    $this AddObserverByNumber $::ClipModel($this,insideOut) 10000 
    $this AddObserverByNumber $::ClipModel($this,modelsSelect) 11000  
    $this AddObserverByNumber $::ClipModel($this,roiSelect) 11000  
    $this AddObserverByNumber $::ClipModel($this,modelsOutputSelect) 11000  
    #$this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    #  [$this GetNumberForVTKEvent ModifiedEvent]
    
}

proc ClipModelRemoveGUIObservers {this} {
    #$this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] \
    #  [$this GetNumberForVTKEvent ModifiedEvent]
    $this RemoveObserverByNumber $::ClipModel($this,clip) 10000
    $this RemoveObserverByNumber $::ClipModel($this,insideOut) 10000
    $this RemoveObserverByNumber $::ClipModel($this,modelsSelect) 11000
    $this RemoveObserverByNumber $::ClipModel($this,roiSelect) 11000
    $this RemoveObserverByNumber $::ClipModel($this,modelsOutputSelect) 11000
}

proc ClipModelProcessGUIEvents {this caller event} {
    #puts "in ClipModelProcessGUIEvents"
    if { $::ClipModel($this,processingMRML) != ""} {
        return
    }

    set ::ClipModel($this,processingGUI) 1

    if { $caller == $::ClipModel($this,roiSelect) } {
        set roi [$::ClipModel($this,roiSelect) GetSelected]
        if { $::ClipModel($this,roiNode) != "" } {
            $this RemoveMRMLObserverByNumber $::ClipModel($this,roiNode) \
              [$this GetNumberForVTKEvent ModifiedEvent]
        }
        set ::ClipModel($this,roiNode) $roi
        $this AddMRMLObserverByNumber $::ClipModel($this,roiNode) \
          [$this GetNumberForVTKEvent ModifiedEvent]
        $::ClipModel($this,displayWidget) SetROINode $roi
        #puts "ROI node"
        #puts [$::ClipModel($this,roiNode) Print]
    }

    if { $caller == $::ClipModel($this,modelsSelect) } {
        set mod [$::ClipModel($this,modelsSelect) GetSelected]
        if { $mod != "" && [$mod GetPolyData] != ""} {
            eval $::ClipModel($this,planes)  SetBounds [[$mod GetPolyData] GetBounds]
            #puts "Setting Box Bounds"
        }
    }

    if { $caller == $::ClipModel($this,modelsOutputSelect) } {
        set mod [$::ClipModel($this,modelsOutputSelect) GetSelected]
        if { $mod != "" && $::ClipModel($this,oldOut) != "" && $mod != $::ClipModel($this,oldOut)} {
            #puts "New output: copy polydata"
            set poly [$::ClipModel($this,oldOut) GetPolyData]
            set cpoly [vtkPolyData New]
            $cpoly CopyStructure $poly
            #$::ClipModel($this,oldOut) SetAndObservePolyData ""
            $::ClipModel($this,oldOut) SetAndObservePolyData $cpoly
            $cpoly Delete
            $::ClipModel($this,oldOut) Register $this
            $::slicer3::MRMLScene RemoveNode $::ClipModel($this,oldOut)
            $::slicer3::MRMLScene AddNode $::ClipModel($this,oldOut)
            $::ClipModel($this,oldOut) UnRegister $this

            #$::slicer3::ViewerWidget Render

            set ::ClipModel($this,oldOut) $mod

             set ::ClipModel($this,processingGUI) ""
            return
        }
    } 


    ClipModelApply $this
    
    set ::ClipModel($this,processingGUI) ""
}

proc ClipModelRemoveLogicObservers {this} {
}

proc ClipModelRemoveMRMLNodeObservers {this} {
    if { [info exists ::ClipModel($this,roiNode)] 
            && $::ClipModel($this,roiNode) != "" } {
      $this RemoveMRMLObserverByNumber $::ClipModel($this,roiNode) \
        [$this GetNumberForVTKEvent ModifiedEvent]
    }
}

proc ClipModelProcessLogicEvents {this caller event} {
}


#
# Accessors to ClipModel state
#


# get the ClipModel parameter node, or create one if it doesn't exist
proc ClipModelCreateParameterNode {} {
    set node [vtkMRMLScriptedModuleNode New]
    $node SetModuleName "ClipModel"
    
    # set node defaults
    $node SetParameter label 1
    
    $::slicer3::MRMLScene AddNode $node
    $node Delete
}

# get the ClipModel parameter node, or create one if it doesn't exist
proc ClipModelGetParameterNode {} {
    
    set node ""
    set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
    for {set i 0} {$i < $nNodes} {incr i} {
        set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
        if { [$n GetModuleName] == "ClipModel" } {
            set node $n
            break;
        }
    }
    
    if { $node == "" } {
        ClipModelCreateParameterNode
        set node [ClipModelGetParameterNode]
    }
    
    return $node
}



#
# MRML Event processing
#

proc ClipModelUpdateMRML {this} {
    #set roi $::ClipModel($this,roiNode)
}

proc ClipModelProcessMRMLEvents {this callerID event} {
    #puts "Processing MRML events"

    if { $::ClipModel($this,processingGUI) != ""} {
        return
    }
    
    set ::ClipModel($this,processingMRML) 1
    
    set caller [$::slicer3::MRMLScene GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
    if { $caller != $::ClipModel($this,roiNode)} {
        return
    }
    
    ClipModelApply $this
         
    set ::ClipModel($this,processingMRML) ""
}

proc ClipModelEnter {this} {
}
proc ClipModelExit {this} {
    #$::ClipModel($this,box) RemoveObservers EndInteractionEvent
    #$::ClipModel($this,box) RemoveObservers InteractionEvent
}


proc ClipModelInit {this} {
    #puts "in ClipModelInit"
    set init  $::ClipModel($this,init)
    if { $init == ""} {
        #puts "init in ClipModelInit"
        set ::ClipModel($this,init) "1"
    }
}

proc ClipModelApply {this} {
    #puts "in ClipModelApply"

    ClipModelInit $this
    
    set clip [$::ClipModel($this,clip) GetSelectedState]
    if { $clip == 0} {
#        $::slicer3::ViewerWidget Render
        return
    }
    
    set insideOut  [$::ClipModel($this,insideOut) GetSelectedState]
    set mod [$::ClipModel($this,modelsSelect) GetSelected]
    set modOut [$::ClipModel($this,modelsOutputSelect) GetSelected]
    #puts "here"
    #puts $mod
    if { $mod == ""} {
#        $::slicer3::ViewerWidget Render
        return
    }
    if { $modOut == ""} {
        return
    }
    
    $::ClipModel($this,clipper) SetInsideOut $insideOut
    $::ClipModel($this,clipper) SetInput [$mod GetPolyData]
    $::ClipModel($this,clipper) Update
    #puts "Updating Cliiping"
    set poly [$::ClipModel($this,clipper) GetOutput]
    $modOut SetAndObservePolyData $poly

    #puts [$poly Print]

    if {[$modOut GetDisplayNode] == ""} {
        set dnode [vtkMRMLModelDisplayNode New]
         $dnode SetColor [expr rand ()] [expr rand ()] [expr rand ()]
        $::slicer3::MRMLScene AddNode $dnode
        $modOut SetAndObserveDisplayNodeID [$dnode GetID]
        $dnode Delete
    }

    set ::ClipModel($this,oldOut) $modOut
    ClipModelClipModel $this
}

proc ClipModelClipModel {this} {
    #puts "in ClipModelClipModel"
    set roi $::ClipModel($this,roiNode)
    if { $roi == ""} {
        #$::slicer3::ViewerWidget Render
        return
    }
    #foreach {rx ry rz} [$roi GetRadiusXYZ] {}
    #foreach {x y z} [$roi GetXYZ] {}
    #$::ClipModel($this,planes) SetBounds [expr $x-$rx] [expr $x+$rx] [expr $y-$ry] [expr $y+$ry] [expr $z-$rz] [expr $z+$rz]

    $roi GetTransformedPlanes $::ClipModel($this,planes)

#    ClipModelRender
}

proc ClipModelRender {} {
    #puts "in ClipModelRender"
    #$::slicer3::ViewerWidget Render
    #[[[$::slicer3::ViewerWidget GetMainViewer] GetRenderWindow] GetInteractor] ReInitialize
}
