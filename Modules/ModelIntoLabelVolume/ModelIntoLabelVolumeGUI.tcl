#
# ModelIntoLabelVolume GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc ModelIntoLabelVolumeConstructor {this} {
}

proc ModelIntoLabelVolumeDestructor {this} {
}

proc ModelIntoLabelVolumeTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    run modelsSelect
    volumesSelect labelValueEntry intensity labelVolumeName settingsFrame 
  }

  foreach w $widgets {
    $::ModelIntoLabelVolume($this,$w) SetParent ""
    $::ModelIntoLabelVolume($this,$w) Delete
  }

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "ModelIntoLabelVolume"]
    [$this GetUIPanel] RemovePage "ModelIntoLabelVolume"
  }

  unset ::ModelIntoLabelVolume(singleton)

}

proc ModelIntoLabelVolumeBuildGUI {this} {

  if { [info exists ::ModelIntoLabelVolume(singleton)] } {
    error "ModelIntoLabelVolume singleton already created"
  }
  set ::ModelIntoLabelVolume(singleton) $this

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
  [$this GetUIPanel] AddPage "ModelIntoLabelVolume" "ModelIntoLabelVolume" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "ModelIntoLabelVolume"]

  #
  # help frame
  #
  set helptext "The ModelIntoLabelVolume is a scripted module in Tcl. It intersects a model with a reference volume and creates an output label map volume with the model vertices labelled with the given label value. This is useful once tracts have been generated to turn them into a label volume. If the intensity option is checked, it will use the voxel values from the reference volume (you can change the output label volume to use the grey colour node).<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:ModelIntoLabelVolume-Documentation</a>"
  set abouttext "Contributed by Nicole Aucoin, SPL, BWH (Ron Kikinis). This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext


  #
  # ModelIntoLabelVolume Settings
  #
  set ::ModelIntoLabelVolume($this,settingsFrame) [vtkSlicerModuleCollapsibleFrame New]
  $::ModelIntoLabelVolume($this,settingsFrame) SetParent $pageWidget
  $::ModelIntoLabelVolume($this,settingsFrame) Create
  $::ModelIntoLabelVolume($this,settingsFrame) SetLabelText "Settings"
  pack [$::ModelIntoLabelVolume($this,settingsFrame) GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

 set ::ModelIntoLabelVolume($this,modelsSelect) [vtkSlicerNodeSelectorWidget New]
    $::ModelIntoLabelVolume($this,modelsSelect) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
    $::ModelIntoLabelVolume($this,modelsSelect) Create
    $::ModelIntoLabelVolume($this,modelsSelect) SetNodeClass "vtkMRMLModelNode" "" "" ""
    $::ModelIntoLabelVolume($this,modelsSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
    $::ModelIntoLabelVolume($this,modelsSelect) SetNoneEnabled 1
    $::ModelIntoLabelVolume($this,modelsSelect) UpdateMenu
    $::ModelIntoLabelVolume($this,modelsSelect) SetLabelText "Model:"
    $::ModelIntoLabelVolume($this,modelsSelect) SetBalloonHelpString "The Model to use"
    pack [$::ModelIntoLabelVolume($this,modelsSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 


  set ::ModelIntoLabelVolume($this,volumesSelect) [vtkSlicerNodeSelectorWidget New]
  $::ModelIntoLabelVolume($this,volumesSelect) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,volumesSelect) Create
  $::ModelIntoLabelVolume($this,volumesSelect) SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $::ModelIntoLabelVolume($this,volumesSelect) SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $::ModelIntoLabelVolume($this,volumesSelect) UpdateMenu
  $::ModelIntoLabelVolume($this,volumesSelect) SetLabelText "Reference Volume:"
  $::ModelIntoLabelVolume($this,volumesSelect) SetBalloonHelpString "The Reference Volume to intersect with"
  pack [$::ModelIntoLabelVolume($this,volumesSelect) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  # label value to use in the new label volume

  set ::ModelIntoLabelVolume($this,labelValueEntry) [vtkKWEntryWithLabel New]
  $::ModelIntoLabelVolume($this,labelValueEntry) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,labelValueEntry) Create
  $::ModelIntoLabelVolume($this,labelValueEntry) SetLabelText "Label Value:"
  [ $::ModelIntoLabelVolume($this,labelValueEntry) GetWidget] SetValue 2
  $::ModelIntoLabelVolume($this,labelValueEntry) SetBalloonHelpString "The label value to insert in the label map output volume"
  pack [$::ModelIntoLabelVolume($this,labelValueEntry) GetWidgetName] -side top -anchor e -padx 2 -pady 2 
  
  # name for the new label map
  set ::ModelIntoLabelVolume($this,labelVolumeName) [vtkKWEntryWithLabel New]
  $::ModelIntoLabelVolume($this,labelVolumeName) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,labelVolumeName) Create
  $::ModelIntoLabelVolume($this,labelVolumeName) SetLabelText "Label Volume Name:"
  [$::ModelIntoLabelVolume($this,labelVolumeName) GetWidget] SetValue "ModelInLabelMap"
  $::ModelIntoLabelVolume($this,labelVolumeName) SetBalloonHelpString "The name to use for the label map output volume"
  pack [$::ModelIntoLabelVolume($this,labelVolumeName) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  # or else use intensity from the reference volume
  set ::ModelIntoLabelVolume($this,intensity) [vtkKWCheckButton New]
    $::ModelIntoLabelVolume($this,intensity) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
    $::ModelIntoLabelVolume($this,intensity) Create
    $::ModelIntoLabelVolume($this,intensity) SetText "Use Intensity"
    $::ModelIntoLabelVolume($this,intensity) SetSelectedState 0
    $::ModelIntoLabelVolume($this,intensity) SetBalloonHelpString "Use the intensity value from the reference volume, overrides the label value and produces a more colourful output until you change the colour node on the label volume to grey"
    pack [$::ModelIntoLabelVolume($this,intensity) GetWidgetName] -side top -anchor w -padx 2 -pady 2 

  # apply button
  set ::ModelIntoLabelVolume($this,run) [vtkKWPushButton New]
  $::ModelIntoLabelVolume($this,run) SetParent [$::ModelIntoLabelVolume($this,settingsFrame) GetFrame]
  $::ModelIntoLabelVolume($this,run) Create
  $::ModelIntoLabelVolume($this,run) SetText "Apply"
  $::ModelIntoLabelVolume($this,run) SetBalloonHelpString "Apply."
  pack [$::ModelIntoLabelVolume($this,run) GetWidgetName] -side top -anchor e -padx 2 -pady 2 

}

proc ModelIntoLabelVolumeAddGUIObservers {this} {
  $this AddObserverByNumber $::ModelIntoLabelVolume($this,run) 10000 

  $this AddObserverByNumber $::ModelIntoLabelVolume($this,modelsSelect) 11000    

  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
    
}

proc ModelIntoLabelVolumeRemoveGUIObservers {this} {
  $this RemoveMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 31
  $this RemoveObserverByNumber $::ModelIntoLabelVolume($this,modelsSelect) 11000
  $this RemoveObserverByNumber $::ModelIntoLabelVolume($this,run) 10000 
}

proc ModelIntoLabelVolumeRemoveLogicObservers {this} {
}

proc ModelIntoLabelVolumeRemoveMRMLNodeObservers {this} {
}

proc ModelIntoLabelVolumeProcessLogicEvents {this caller event} {
}

proc ModelIntoLabelVolumeProcessGUIEvents {this caller event} {
  
  if { $caller == $::ModelIntoLabelVolume($this,modelsSelect) } {
        set mod [$::ModelIntoLabelVolume($this,modelsSelect) GetSelected]
        if { $mod != "" && [$mod GetPolyData] != ""} {
            # do something 
        }
    }

  if { $caller == $::ModelIntoLabelVolume($this,run) } {
    switch $event {
      "10000" {
        ModelIntoLabelVolumeApply $this
      }
    }
  } 

  ModelIntoLabelVolumeUpdateMRML $this
}

#
# Accessors to ModelIntoLabelVolume state
#


# get the ModelIntoLabelVolume parameter node, or create one if it doesn't exist
proc ModelIntoLabelVolumeCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "ModelIntoLabelVolume"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the ModelIntoLabelVolume parameter node, or create one if it doesn't exist
proc ModelIntoLabelVolumeGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "ModelIntoLabelVolume" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    ModelIntoLabelVolumeCreateParameterNode
    set node [ModelIntoLabelVolumeGetParameterNode]
  }

  return $node
}


proc ModelIntoLabelVolumeGetLabel {} {
  set node [ModelIntoLabelVolumeGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc ModelIntoLabelVolumeSetLabel {index} {
  set node [ModelIntoLabelVolumeGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc ModelIntoLabelVolumeUpdateMRML {this} {
}

proc ModelIntoLabelVolumeProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc ModelIntoLabelVolumeEnter {this} {
}

proc ModelIntoLabelVolumeExit {this} {
}

proc ModelIntoLabelVolumeApply {this} {

  set debug 0

  set modelNode [$::ModelIntoLabelVolume($this,modelsSelect) GetSelected]
  set labelValue [[$::ModelIntoLabelVolume($this,labelValueEntry) GetWidget] GetValueAsInt]
  set labelVolumeName [[$::ModelIntoLabelVolume($this,labelVolumeName) GetWidget] GetValue]
  set volumeNode [$::ModelIntoLabelVolume($this,volumesSelect)  GetSelected]
  set matrixNode ""
  set deleteMt 1
  set intensityFlag [$::ModelIntoLabelVolume($this,intensity) GetSelectedState]

  if {$labelVolumeName == ""} {
      set labelVolumeName "[$modelNode GetName]-[$volumeNode GetName]"
  }

  if {$modelNode == ""} {
      set msg "You must select a model first"
  } elseif {$volumeNode == ""} {
      set msg "You must select a reference volume first"
  } else {
      set msg ""
      # set msg "Working on model [$modelNode GetName], ref vol = [$volumeNode GetName], label value = $labelValue, label volume name = $labelVolumeName, intensityFlag = $intensityFlag"
  }
  if {$msg != ""} {
      set dialog [vtkKWMessageDialog New]
      $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
      $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
      $dialog SetStyleToMessage
      $dialog SetText $msg
      $dialog Create
      $dialog Invoke
      $dialog Delete
      return
  }
  # clone the reference volume into a label map
  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelmapVolumeNode [$volumesLogic CreateLabelVolume $::slicer3::MRMLScene $volumeNode $labelVolumeName]

  # now get the volume's matrix
  catch "Mrasijk Delete"
  vtkMatrix4x4 Mrasijk
  # get the volume's RAS to ijk matrix
  $volumeNode GetRASToIJKMatrix Mrasijk 

  # for now use an identity transform, later can pass one in
  catch "IdMatrix Delete"
  set Mt [vtkMatrix4x4 IdMatrix]
  if {$matrixNode != ""} {
      $matrixNode GetMatrixTransformToWorld $Mt
      set deleteMt 0
  }

  # now loop over the points, transform them and write them out
  puts "Transforming RAS vtkPolydata points in IJK"
  set numPoints [[$modelNode GetPolyData] GetNumberOfPoints]
  set foundNumPoints 0

  # Get the vertex points out of the model and transform them
  for {set p 0} {$p < $numPoints} {incr p} {
      # get the coordinate points
      scan [[$modelNode GetPolyData] GetPoint $p] "%f %f %f" a b c

      # multiply by the transform matrix   ( Mt = Identity matrix)
      set transformPoint [$Mt MultiplyPoint $a $b $c 1]
      
      # multiply by the RAS to IJK matrix from the volume
      set ijkPoint [Mrasijk MultiplyPoint [lindex $transformPoint 0] [lindex $transformPoint 1] [lindex $transformPoint 2] [lindex $transformPoint 3]]
      #set ijkPoint [$Mt MultiplyPoint [lindex $transformPoint 0] [lindex $transformPoint 1] [lindex $transformPoint 2] [lindex $transformPoint 3]]
      ##################### Modif Sonia 2 ############################
        # Transform the IJK (float) into integers
      set i   [lindex $ijkPoint 0]
      set i0   [expr int($i)]

      set tmp [expr abs($i - $i0)]
      if {$tmp > 0.5} {
          set i [expr $i0 +1 ]
      } else {
          set i $i0
      }
      set j   [lindex $ijkPoint 1]
      set j0   [expr int($j)]
      
      set tmp [expr abs($j - $j0)]
      if {$tmp > 0.5} {
          set j [expr $j0 +1 ]
      } else {
          set j $j0
      }
      set k   [lindex $ijkPoint 2]
      set k0   [expr int($k)]
      
      set tmp [expr abs($k - $k0)]
      if {$tmp > 0.5} {
          set k [expr $k0 +1 ]
      } else {
          set k $k0
      }
      ##################################################################
      
      # get the voxelID corresponding to the ijk location
      # set voxelPointID [[Volume($volumeID,vol) GetOutput] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
      
      # Set the intensity to the corresponding voxel in the labelmapvolume
      #set voxeloutPointID [[Volume($labelmapvolumeID,vol) GetOutput] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
      
      # set voxeloutPointID [[$labelmapVolumeNode GetImageData] FindPoint [expr $i * 2.0] [expr $j * 2.0] [expr  $k * 2.0] ]
      set voxeloutPointID [[$labelmapVolumeNode GetImageData] FindPoint $i $j $k]
      if {$voxeloutPointID < 0} {
          puts "Unable to find volume point id for $ijkPoint"
      } else {
          if {$intensityFlag == 0} {
              [[ [$labelmapVolumeNode GetImageData] GetPointData] GetScalars] SetTuple1 $voxeloutPointID $labelValue
          } else {
              # the voxelID corresponding to the ijk locations
              set voxelPointID [[$volumeNode GetImageData] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
              
              if {$voxelPointID < 0} {
                  puts "Unable to find volume point id for $ijkPoint"
              } else {
                  
                  # read the intensity from the FA map
                  set intensity [[[[$volumeNode GetImageData] GetPointData] GetScalars] GetTuple1 $voxelPointID]
                  # and set it
                  [[ [$labelmapVolumeNode GetImageData] GetPointData] GetScalars] SetTuple1 $voxeloutPointID $intensity
              }
          }
          
          if {$debug && $p < 5} {
              # print out the first point's values
              puts "Point number $p:"
              puts "Dvtk =\n\t$a $b $c 1"
              puts "Mt * Dvtk =\n\t$transformPoint"
              puts "IJK Point is : Mvtk * Mt * Dvtk =\n\t$ijkPoint"
              if {$intensityFlag != 0} {
                  puts "Intensity = $intensity"
              }
              
          }          
          incr foundNumPoints
      }
  }

  # set the new label map to be active
  [$::slicer3::ApplicationLogic GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
  [$::slicer3::ApplicationLogic GetSelectionNode] SetReferenceActiveLabelVolumeID [$labelmapVolumeNode GetID]
  $::slicer3::ApplicationLogic PropagateVolumeSelection 0
    
  # clean up  
  Mrasijk Delete
    if {$deleteMt} {
      # only delete it if it's our declared identity, otherwise it's a pointer to 
      # a Slicer variable
      $Mt Delete
  }
}
