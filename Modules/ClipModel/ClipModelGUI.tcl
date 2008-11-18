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
        clip interact modelsSelect
        modelsOutputSelect modelsFrame 
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
    
    
    $this SetCategory "Model Generation"
    [$this GetUIPanel] AddPage "ClipModel" "ClipModel" ""
    set pageWidget [[$this GetUIPanel] GetPageWidget "ClipModel"]
    
    #
    # help frame
    #
    set helptext "The ClipModel clips model by a box using a box widget."
    set abouttext "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
    $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext
    
    
    #
    # ClipModel Volumes
    #
    set ::ClipModel($this,modelsFrame) [vtkSlicerModuleCollapsibleFrame New]
    $::ClipModel($this,modelsFrame) SetParent $pageWidget
    $::ClipModel($this,modelsFrame) Create
    $::ClipModel($this,modelsFrame) SetLabelText "Models"
    pack [$::ClipModel($this,modelsFrame) GetWidgetName] \
        -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]
    
    set ::ClipModel($this,modelsSelect) [vtkSlicerNodeSelectorWidget New]
    $::ClipModel($this,modelsSelect) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,modelsSelect) Create
    $::ClipModel($this,modelsSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ClipModel($this,modelsSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ClipModel($this,modelsSelect) SetNoneEnabled 1
    $::ClipModel($this,modelsSelect) UpdateMenu
    $::ClipModel($this,modelsSelect) SetLabelText "Source Model:"
    $::ClipModel($this,modelsSelect) SetBalloonHelpString "The Source Model to operate on"
    pack [$::ClipModel($this,modelsSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
    
    set ::ClipModel($this,modelsOutputSelect) [vtkSlicerNodeSelectorWidget New]
    $::ClipModel($this,modelsOutputSelect) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,modelsOutputSelect) Create
    $::ClipModel($this,modelsOutputSelect) NewNodeEnabledOn
    $::ClipModel($this,modelsOutputSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ClipModel($this,modelsOutputSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ClipModel($this,modelsOutputSelect) UpdateMenu
    $::ClipModel($this,modelsOutputSelect) SetLabelText "Clipped Model:"
    $::ClipModel($this,modelsOutputSelect) SetBalloonHelpString "The target output model"
    pack [$::ClipModel($this,modelsOutputSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
    
    set ::ClipModel($this,clip) [vtkKWCheckButton New]
    $::ClipModel($this,clip) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,clip) Create
    $::ClipModel($this,clip) SetText "Clipping  Enabled"
    $::ClipModel($this,clip) SetSelectedState 0
    $::ClipModel($this,clip) SetBalloonHelpString "Apply clipping"
    pack [$::ClipModel($this,clip) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
    
    set ::ClipModel($this,interact) [vtkKWCheckButton New]
    $::ClipModel($this,interact) SetParent [$::ClipModel($this,modelsFrame) GetFrame]
    $::ClipModel($this,interact) Create
    $::ClipModel($this,interact) SetText "Clip While Interacting"
    $::ClipModel($this,interact) SetSelectedState 0
    $::ClipModel($this,interact) SetBalloonHelpString "Apply interactping"
    pack [$::ClipModel($this,interact) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
    
    set ::ClipModel($this,init) ""
    set ::ClipModel($this,oldOut) ""
}

proc ClipModelAddGUIObservers {this} {
    $this AddObserverByNumber $::ClipModel($this,clip) 10000 
    $this AddObserverByNumber $::ClipModel($this,interact) 10000 
    $this AddObserverByNumber $::ClipModel($this,modelsSelect) 11000  
    $this AddObserverByNumber $::ClipModel($this,modelsOutputSelect) 11000  
    $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc ClipModelRemoveGUIObservers {this} {
    $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    $this RemoveObserverByNumber $::ClipModel($this,clip) 10000
    $this RemoveObserverByNumber $::ClipModel($this,interact) 10000
    $this RemoveObserverByNumber $::ClipModel($this,modelsSelect) 11000
    $this RemoveObserverByNumber $::ClipModel($this,modelsOutputSelect) 11000
}

proc ClipModelProcessGUIEvents {this caller event} {
    #puts "in ClipModelProcessGUIEvents"
    if { $caller == $::ClipModel($this,interact) } {
        set interact [$::ClipModel($this,interact) GetSelectedState]
        if { $interact == 0} {
            $::ClipModel($this,box) RemoveObservers InteractionEvent
            $::ClipModel($this,box) AddObserver InteractionEvent ClipModelRender
            $::ClipModel($this,box) RemoveObservers EndInteractionEvent
            $::ClipModel($this,box) AddObserver EndInteractionEvent ClipModelClipModel
        }
        if { $interact == 1} {
            $::ClipModel($this,box) RemoveObservers InteractionEvent
            $::ClipModel($this,box) AddObserver InteractionEvent ClipModelClipModel
        }
        $::slicer3::ViewerWidget Render
        return
    }

    if { $caller == $::ClipModel($this,modelsSelect) } {
        set mod [$::ClipModel($this,modelsSelect) GetSelected]
        if { $mod != "" && [$mod GetPolyData] != ""} {
            eval $::ClipModel($this,planes)  SetBounds [[$mod GetPolyData] GetBounds]
            eval $::ClipModel($this,boxRep)  PlaceWidget [[$mod GetPolyData] GetBounds]
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
            return
        }
    } 


    ClipModelApply $this
    
    ClipModelUpdateMRML $this
}

proc ClipModelRemoveLogicObservers {this} {
}

proc ClipModelRemoveMRMLNodeObservers {this} {
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


proc ClipModelGetLabel {} {
    set node [ClipModelGetParameterNode]
    if { [$node GetParameter "label"] == "" } {
        $node SetParameter "label" 1
    }
    return [$node GetParameter "label"]
}

proc ClipModelSetLabel {index} {
    set node [ClipModelGetParameterNode]
    $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc ClipModelUpdateMRML {this} {
}

proc ClipModelProcessMRMLEvents {this callerID event} {
    
    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
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
        set ::ClipModel($this,box) [$::slicer3::ViewerWidget GetBoxWidget]
        set ::ClipModel($this,boxRep) [$::slicer3::ViewerWidget GetBoxWidgetRepresentation]
        $::slicer3::ViewerWidget SetBoxWidgetInteractor
        
        $::ClipModel($this,box) AddObserver EndInteractionEvent ClipModelClipModel
        $::ClipModel($this,box) AddObserver InteractionEvent ClipModelRender
        $::ClipModel($this,box) SetPriority 1 
        $::ClipModel($this,boxRep)  SetPlaceFactor  1.0
        
        set ::ClipModel($this,init) "1"
    }
}

proc ClipModelApply {this} {
    #puts "in ClipModelApply"

    ClipModelInit $this
    
    set clip [$::ClipModel($this,clip) GetSelectedState]
    if { $clip == 0} {
        $::ClipModel($this,box) Off
        $::slicer3::ViewerWidget Render
        return
    }
    
    if { $clip == 1} {
        $::ClipModel($this,box) On
        $::slicer3::ViewerWidget Render
    }
    
    set mod [$::ClipModel($this,modelsSelect) GetSelected]
    set modOut [$::ClipModel($this,modelsOutputSelect) GetSelected]
    #puts "here"
    #puts $mod
    if { $mod == ""} {
        $::ClipModel($this,box) Off
        $::slicer3::ViewerWidget Render
        return
    }
    if { $modOut == ""} {
        return
    }
    
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
    #ClipModelClipModel $this
}

proc ClipModelClipModel {} {
    #puts "in ClipModelClipModel"
    set this $::ClipModel(singleton)
    $::ClipModel($this,boxRep) GetPlanes $::ClipModel($this,planes)
    ClipModelApply $this
    [[[$::slicer3::ViewerWidget GetMainViewer] GetRenderWindow] GetInteractor] ReInitialize
}

proc ClipModelRender {} {
    #puts "in ClipModelRender"
    $::slicer3::ViewerWidget Render
    [[[$::slicer3::ViewerWidget GetMainViewer] GetRenderWindow] GetInteractor] ReInitialize
}
