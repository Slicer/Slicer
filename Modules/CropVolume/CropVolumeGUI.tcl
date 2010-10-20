#
# CropVolume GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

#proc func {name argl body} {proc $name $argl [list expr $body]}
proc min {a b} {expr $a<$b? $a: $b}


proc CropVolumeConstructor {this} {
}

proc CropVolumeDestructor {this} {
}

proc CropVolumeTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    initFrame runButton inputSelector outputSelector roiSelector samplingScale
    isotropicResampling resamplingFrame roiVisibility
  }
  set nonwidgets {
    visIcons
  }

  foreach w $widgets {
    $::CropVolume($this,$w) SetParent ""
    $::CropVolume($this,$w) Delete
  }
  foreach w $nonwidgets {
    $::CropVolume($this,$w) Delete
  }


  $::CropVolume($this,resliceFilter) Delete

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "CropVolume"]
    [$this GetUIPanel] RemovePage "CropVolume"
  }

  unset ::CropVolume(singleton)
}

proc CropVolumeBuildGUI {this} {

  set ::CropVolume($this,redGUIObserverTags) ""
  set ::CropVolume($this,greenGUIObserverTags) ""
  set ::CropVolume($this,yellowGUIObserverTags) ""

  if { [info exists ::CropVolume(singleton)] } {
    error "CropVolume singleton already created"
  }
  set ::CropVolume(singleton) $this

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


  $this SetCategory "Converters"
  [$this GetUIPanel] AddPage "CropVolume" "CropVolume" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "CropVolume"]

  #
  # help frame
  #
  set helptext "This module extracts subvolume of the image described by Region of Interest widget.\n\nTo use:\n(1) specify input volume\n(2) specify ROI\n(3) adjust ROI\n(4) specify output volume\n(5) choose interpolation method\n(6) choose resampling scale\nNote: 0.5 as resampling scale will multiply spacing in each dimension by 0.5 to get new spacing (resampled image will have twice as many pixels in each dimension), 2 as resampling scale will result in the new image with two times *less* pixels in each dimension.\n\nUse mouse in slice views to adjust ROI: Left click -- resize, Right click -- re-center\nDocumentation:<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:CropVolume-Documentation-3.6</a>"
  set abouttext "This module was developed by Andriy Fedorov and Ron Kikinis. PI: Ron Kikinis. \nThis work was funded by Brain Science Foundation, and is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext

  set slicerBaseIcons [vtkSlicerBaseAcknowledgementLogoIcons New]

  set logo [vtkKWIcon New]
  set logoReader [vtkPNGReader New]
  set iconDir [file join [[$::CropVolume(singleton) GetLogic] GetModuleShareDirectory] "ImageData"]
  $logoReader SetFileName $iconDir/BSFLogo.png
  $logoReader Update

  $logo SetImage [$logoReader GetOutput]

  set logoLabelBSF [vtkKWLabel New]
  $logoLabelBSF SetParent [$this GetLogoFrame]
  $logoLabelBSF Create
  $logoLabelBSF SetImageToIcon $logo

  set logoLabelNAMIC [vtkKWLabel New]
  $logoLabelNAMIC SetParent [$this GetLogoFrame]
  $logoLabelNAMIC Create
  $logoLabelNAMIC SetImageToIcon [$slicerBaseIcons GetNAMICLogo]

  pack [$logoLabelBSF GetWidgetName] [$logoLabelNAMIC GetWidgetName] -side left

  $logoLabelBSF Delete
  $logoLabelNAMIC Delete
  $logoReader Delete
  $logo Delete
  $slicerBaseIcons Delete


  #
  # CropVolume input and initialization parameters
  #
  set ::CropVolume($this,initFrame) [vtkSlicerModuleCollapsibleFrame New]
  set initFrame $::CropVolume($this,initFrame)
  $initFrame SetParent $pageWidget
  $initFrame Create
  $initFrame SetLabelText "Input/initialization parameters"
  pack [$initFrame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  # Input frame widgets

  set ::CropVolume($this,inputSelector) [vtkSlicerNodeSelectorWidget New]
  set select $::CropVolume($this,inputSelector)
  $select SetParent [$initFrame GetFrame]
  $select Create
  $select SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $select SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $select UpdateMenu
  $select SetLabelText "Input volume:"
  $select SetBalloonHelpString "The Source Volume to operate on"
  pack [$select GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::CropVolume($this,roiSelector) [vtkSlicerNodeSelectorWidget New]
  set roi $::CropVolume($this,roiSelector)
  $roi SetParent [$initFrame GetFrame]
  $roi Create
  $roi NewNodeEnabledOn
  $roi SetNodeClass "vtkMRMLROINode" "" "" "Subvolume_ROI"
  $roi SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $roi UpdateMenu
  $roi SetLabelText "ROI:"
  $roi SetBalloonHelpString "Region of interest that defines the subvolume"

  set ::CropVolume($this,visIcons) [vtkSlicerVisibilityIcons New]
  set visIcons $::CropVolume($this,visIcons)
  set ::CropVolume($this,roiVisibility) [vtkKWPushButtonWithLabel New]
  set roiVisibility $::CropVolume($this,roiVisibility)
  $roiVisibility SetParent [$initFrame GetFrame]
  $roiVisibility Create
  $roiVisibility SetLabelText "ROI visibility"
  $roiVisibility SetReliefToFlat
  $roiVisibility SetBalloonHelpString "Toggle visibility of ROI widget"
  [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetVisibleIcon]

  pack [$roi GetWidgetName] [$roiVisibility GetWidgetName] -side top -anchor e -padx 2 -pady 2
  
  # Output volume definition
  set ::CropVolume($this,outputSelector) [vtkSlicerNodeSelectorWidget New]
  set out $::CropVolume($this,outputSelector)
  $out SetParent [$initFrame GetFrame]
  $out Create
  $out NewNodeEnabledOn
  $out SetNodeClass "vtkMRMLScalarVolumeNode" "" "" "Resampled_ROI_Subvolume"
  $out SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $out UpdateMenu
  $out SetLabelText "Output volume:"
  $out SetBalloonHelpString "The Output Volume node"
  pack [$out GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  # Spacing to use for isotropic resampling
  set ::CropVolume($this,samplingScale) [vtkKWSpinBoxWithLabel New]
  set sampling $::CropVolume($this,samplingScale)
  $sampling SetParent [$initFrame GetFrame]
  $sampling Create
  $sampling SetLabelText "Input spacing scaling constant"
  [$sampling GetWidget] SetValue 1.
  $sampling SetBalloonHelpString "Spacing of the input image in each dimension will be multiplied by this number to get output spacing. E.g., \"1\" will preserve original spacing, \"0.5\" will *increase* number of pixels in each dimension by 2, \"2\" will *reduce* number of pixels in each dimension by 2"
  pack [$sampling GetWidgetName] -side top -anchor e -padx 2 -pady 2

  # Isotropic resampling check box
  set ::CropVolume($this,isotropicResampling) [vtkKWCheckButton New]
  set ckbutton $::CropVolume($this,isotropicResampling)
  $ckbutton SetParent [$initFrame GetFrame]
  $ckbutton Create
  $ckbutton SetText "Isotropic voxel size for output volume"
  $ckbutton SelectedStateOff
  $ckbutton SetBalloonHelpString "If checked, the output volume will have isotropic spacing, with the voxel size defined as the minimum input spacing value times the isotropic constant value"
  pack [$ckbutton GetWidgetName] -side top -anchor e -padx 2 -pady 2

  # Resampler selection
  set ::CropVolume($this,resamplingFrame) [vtkKWRadioButtonSetWithLabel New]
  set resamplingFrame $::CropVolume($this,resamplingFrame)
  $resamplingFrame SetParent [$initFrame GetFrame]
  $resamplingFrame Create
  $resamplingFrame SetLabelText "Interpolation type:"
  $resamplingFrame SetBalloonHelpString "Select interpolation type"
  [$resamplingFrame GetWidget] PackHorizontallyOn
  [$resamplingFrame GetWidget] SetMaximumNumberOfWidgetsInPackingDirection 3

  set nnButton [[$resamplingFrame GetWidget] AddWidget 0]
  $nnButton SetValue 0
  $nnButton SetText "Nearest Neighbor"
  $nnButton SetSelectedState 0
  $nnButton SetAnchorToWest
  $nnButton SetBalloonHelpString "Use for resampling binary images"

  set linButton [[$resamplingFrame GetWidget] AddWidget 1]
  $linButton SetValue 1
  $linButton SetText "Linear"
  $linButton SetSelectedState 1
  $linButton SetAnchorToWest
  $linButton SetBalloonHelpString "Use for resampling scalar images"

  set cubButton [[$resamplingFrame GetWidget] AddWidget 2]
  $cubButton SetValue 2
  $cubButton SetText "Cubic"
  $cubButton SetSelectedState 0
  $cubButton SetAnchorToWest
  $cubButton SetBalloonHelpString "Use for resampling scalar images with improved accuracy"

  pack [$resamplingFrame GetWidgetName] -side top -anchor e -padx 2 -pady 2

  set ::CropVolume($this,runButton) [vtkKWPushButton New]
  set run $::CropVolume($this,runButton)
  $run SetParent [$initFrame GetFrame]
  $run Create
  $run SetText "Do ROI resample"
  pack [$run GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  set ::CropVolume($this,resliceFilter) [vtkImageReslice New]
  set ::CropVolume($this,inputVolume) ""
  set ::CropVolume($this,roilabelNode) ""
  set ::CropVolume($this,roilabelImage) ""

  #  set up pointers, observers will be initialized on module enter
  set appGUI $::slicer3::ApplicationGUI
  set ::CropVolume($this,rwiRedInteractorStyle) \
    [[[[[$appGUI GetMainSliceGUI "Red"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor] GetInteractorStyle]
  set ::CropVolume($this,rwiGreenInteractorStyle) \
    [[[[$appGUI GetMainSliceGUI "Green"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor]
  set ::CropVolume($this,rwiYellowInteractorStyle) \
    [[[[$appGUI GetMainSliceGUI "Yellow"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor]
}

proc CropVolumeAddGUIObservers {this} {
  $this AddObserverByNumber $::CropVolume($this,runButton) 10000 
  $this AddObserverByNumber $::CropVolume($this,inputSelector)  11000
  $this AddObserverByNumber $::CropVolume($this,outputSelector)  10000
  $this AddObserverByNumber $::CropVolume($this,roiSelector) 11000
  # observe TransformModifiedEvent
  $this AddObserverByNumber $::CropVolume($this,roiSelector) 15000
  $this AddObserverByNumber [$::CropVolume($this,roiVisibility) GetWidget] 10000
#  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 33
    
  # From Slicer console
  # set style [[[[[$::slicer3::ApplicationGUI GetMainSliceGUI "Red"]
  # GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]
  # GetInteractorStyle]
#  [ $rwiGreen GetInteractorStyle ] AddObserver 12 
#  [ $rwiYellow GetInteractorStyle ] AddObserver 12 

}

proc CropVolumeRemoveGUIObservers {this} {
}

proc CropVolumeRemoveLogicObservers {this} {
}

proc CropVolumeRemoveMRMLNodeObservers {this} {
  catch {$this RemoveMRMLObserverByNumber $::CropVolume($this,observedROINode) \
    [$this GetNumberForVTKevent ModifiedEvent] }
}

proc CropVolumeProcessLogicEvents {this caller event} {
}

proc CropVolumeProcessGUIEvents {this caller event} {
  if { $caller == $::CropVolume($this,runButton) } {
  
    set inputVolume [$::CropVolume($this,inputSelector) GetSelected]
    if {$inputVolume == ""} {
      CropVolumeErrorDialog $this "Please specify input volume!"
      return
    }
    CropVolumeApply $this
  } 

  if {$caller == $::CropVolume($this,roiSelector)} {
    catch {$this RemoveMRMLObserverByNumber $::CropVolume($this,observedROINode) \
      [$this GetNumberForVTKEvent ModifiedEvent] }
    set ::CropVolume($this,observedROINode) [$::CropVolume($this,roiSelector) GetSelected]
    $this AddMRMLObserverByNumber $::CropVolume($this,observedROINode) \
      [$this GetNumberForVTKEvent ModifiedEvent]
  }

  if {$caller == [$::CropVolume($this,roiVisibility) GetWidget] } {
    if {$::CropVolume($this,observedROINode) == ""} {
      return
    }
    if { [$::CropVolume($this,observedROINode) GetVisibility] } {
      $::CropVolume($this,observedROINode) SetVisibility 0
    } else {
      $::CropVolume($this,observedROINode) SetVisibility 1
    }
    $::CropVolume($this,observedROINode) Modified
  }

  if {$caller == $::CropVolume($this,inputSelector) || \
      $caller == $::CropVolume($this,roiSelector) } {
    set roiNode [$::CropVolume($this,roiSelector) GetSelected]

    set inputVolume [$::CropVolume($this,inputSelector) GetSelected]
    if {$inputVolume == "" || $inputVolume == $::CropVolume($this,inputVolume) || $roiNode == ""} {
      puts "Something not set, returning"
      return
    }
    set ::CropVolume($this,inputVolume) $inputVolume

    # ROI label visualization
    set scene [[$this GetLogic] GetMRMLScene]
    set volumesLogic [$::slicer3::VolumesGUI GetLogic]
    if { $::CropVolume($this,roilabelNode) == "" } {
      set ::CropVolume($this,roilabelNode) [$volumesLogic \
        CreateLabelVolume $scene $inputVolume "Subvolume_ROI_Label"]
        
      set colorLogic [vtkSlicerColorLogic New]
      set colorNodeID [$colorLogic GetDefaultColorTableNodeID 10]
      [$::CropVolume($this,roilabelNode) GetDisplayNode] SetAndObserveColorNodeID $colorNodeID
      $colorLogic Delete

      $::CropVolume($this,roilabelNode) SetAndObserveTransformNodeID 0

      set roilabelImage [vtkImageData New]
      $roilabelImage SetExtent 0 1 0 1 0 1
      $roilabelImage SetDimensions 1 1 1
      $roilabelImage SetScalarTypeToChar
      $roilabelImage SetScalarComponentFromFloat 0 0 0 0 17
      $roilabelImage AllocateScalars
      $::CropVolume($this,roilabelNode) SetAndObserveImageData $roilabelImage
      $roilabelImage Delete
    }

    set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]
    for { set j 0 } { $j < $numCnodes } { incr j } {
      set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
      if { [expr [$inputVolume GetLabelMap]] } {
        $cnode SetReferenceLabelVolumeID [$inputVolume GetID]
      } else {
        $cnode SetReferenceBackgroundVolumeID [$inputVolume GetID]
      }
      $cnode SetReferenceForegroundVolumeID [$::CropVolume($this,roilabelNode) GetID]
      $cnode SetForegroundOpacity 0.6
    }

    # trigger update of the label image contents
    CropVolumeUpdateLabelMap $this
  }

  if {$event == "LeftButtonPressEvent" || $event == "RightButtonPressEvent"} {
  
    set redStyle $::CropVolume($this,rwiRedInteractorStyle)
    set greenStyle $::CropVolume($this,rwiGreenInteractorStyle)
    set yellowStyle $::CropVolume($this,rwiYellowInteractorStyle)

    set roiNode [$::CropVolume($this,roiSelector) GetSelected]
    if {$roiNode == ""} {
      return
    }

    if {$caller == $redStyle} {
      set sliceGUI [$::slicer3::ApplicationGUI GetMainSliceGUI "Red"]
      set sliceRWI [[[$sliceGUI GetSliceViewer] GetRenderWidget ] \
        GetRenderWindowInteractor]
    }
    if {$caller == $yellowStyle} {
      set sliceGUI [$::slicer3::ApplicationGUI GetMainSliceGUI "Yellow"]
      set sliceRWI [[[$sliceGUI GetSliceViewer] GetRenderWidget ] \
        GetRenderWindowInteractor]
    }
    if {$caller == $greenStyle} {
      set sliceGUI [$::slicer3::ApplicationGUI GetMainSliceGUI "Green"]
      set sliceRWI [[[$sliceGUI GetSliceViewer] GetRenderWidget ] \
        GetRenderWindowInteractor]
    }

    # now retrieve RAS position for the mouse click
    set xy2ras [[[$sliceGUI GetLogic] GetSliceNode] GetXYToRAS]
    set clickXY [$sliceRWI GetLastEventPosition]
    # this is a bit confusing, because the coordinates of click are actually
    # in RAS, but ROI uses XYZ, not RAS notation, so I stick with it
    scan [lrange [eval $xy2ras MultiplyPoint "$clickXY 0 1"] 0 2] "%f%f%f" clickX clickY clickZ
  
    if {$event == "RightButtonPressEvent"} {
      $roiNode SetXYZ $clickX $clickY $clickZ
    }
    if {$event == "LeftButtonPressEvent"} {
      scan [$roiNode GetXYZ] "%f%f%f" roiX roiY roiZ
      scan [$roiNode GetRadiusXYZ] "%f%f%f" roiRadiusX roiRadiusY roiRadiusZ
      set dX [expr abs($roiX-$clickX)] 
      set dY [expr abs($roiY-$clickY)] 
      set dZ [expr abs($roiZ-$clickZ)] 
      set shiftX [expr abs($dX-$roiRadiusX)/2.]
      set shiftY [expr abs($dY-$roiRadiusY)/2.]
      set shiftZ [expr abs($dZ-$roiRadiusZ)/2.]

      # adjust the center and radius at the same time, so that only the
      # nearest side of the bbox moves
      if { [expr $dX>$roiRadiusX || $dY>$roiRadiusY || $dZ>$roiRadiusZ] } {
        # click outside the box
        if {[expr $dX>$roiRadiusX]} {
          if {[expr $clickX>$roiX]} {
            set roiX [expr $roiX+$shiftX]
          } else {
            set roiX [expr $roiX-$shiftX]
          }
          set roiRadiusX [expr $roiRadiusX+$shiftX]
        }
        if {[expr $dY>$roiRadiusY]} {
          if {[expr $clickY>$roiY]} {
            set roiY [expr $roiY+$shiftY]
          } else {
            set roiY [expr $roiY-$shiftY]
          }
          set roiRadiusY [expr $roiRadiusY+$shiftY]
        } 
        if {[expr $dZ>$roiRadiusZ]} {
          if {[expr $clickZ>$roiZ]} {
            set roiZ [expr $roiZ+$shiftZ]
          } else {
            set roiZ [expr $roiZ-$shiftZ]
          }
          set roiRadiusZ [expr $roiRadiusZ+$shiftZ]
        } 
      } else {
        # in-box click -- reduce ROI. I decide which coordinates are changing
        # in-slice by adjusting the XY, and transforming to RAS. There may
        # well be better ways to figure this out via slice API.
        set clickXYmod {[expr [lindex $clickXY 0]+10] [expr [lindex $clickXY 1]+10]}
        scan [lrange [eval $xy2ras MultiplyPoint "$clickXYmod 0 1"] 0 2] "%f%f%f" clickXmod clickYmod clickZmod
        
        set doModifyX 1
        set doModifyY 1
        set doModifyZ 1
        
        # I am not sure what is the best behavior when reducing ROI. The
        # choice I made is to move only the closest in RAS edge of the ROI on
        # the click. Seems to me to be most intuitive.
        if {$clickX == $clickXmod} {
          set doModifyX 0
          if {$dZ>$dY} {
            set doModifyZ 1
            set doModifyY 0
          } else {
            set doModifyZ 0
            set doModifyY 1
          }
        }
        if {$clickY == $clickYmod} {
          set doModifyY 0
          if {$dX>$dZ} {
            set doModifyX 1
            set doModifyZ 0
          } else {
            set doModifyX 0
            set doModifyZ 1
          }
        }
        if {$clickZ == $clickZmod} {
          set doModifyZ 0
          if {$dX>$dY} {
            set doModifyX 1
            set doModifyY 0
          } else {
            set doModifyX 0
            set doModifyY 1
          }
        }
        
        if {[expr (($dX<$roiRadiusX) && $doModifyX)]} {
          if {[expr $clickX>$roiX]} {
            set roiX [expr $roiX-$shiftX]
          } else {
            set roiX [expr $roiX+$shiftX]
          }
          set roiRadiusX [expr $roiRadiusX-$shiftX]
        }
        if {[expr (($dY<$roiRadiusY) && $doModifyY)]} {
          if {[expr $clickY>$roiY]} {
            set roiY [expr $roiY-$shiftY]
          } else {
            set roiY [expr $roiY+$shiftY]
          }
          set roiRadiusY [expr $roiRadiusY-$shiftY]
        } 
        if {[expr ($dZ<$roiRadiusZ) && $doModifyZ]} {
          if {[expr $clickZ>$roiZ]} {
            set roiZ [expr $roiZ-$shiftZ]
          } else {
            set roiZ [expr $roiZ+$shiftZ]
          }
          set roiRadiusZ [expr $roiRadiusZ-$shiftZ]
        } 
      }

      $roiNode SetXYZ $roiX $roiY $roiZ
      $roiNode SetRadiusXYZ $roiRadiusX $roiRadiusY $roiRadiusZ
    }
  }
}

#
# Accessors to CropVolume state
#


# get the CropVolume parameter node, or create one if it doesn't exist
proc CropVolumeCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "CropVolume"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the CropVolume parameter node, or create one if it doesn't exist
proc CropVolumeGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "CropVolume" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    CropVolumeCreateParameterNode
    set node [CropVolumeGetParameterNode]
  }

  return $node
}


proc CropVolumeGetLabel {} {
  set node [CropVolumeGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc CropVolumeSetLabel {index} {
  set node [CropVolumeGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc CropVolumeUpdateMRML {this} {
}

proc CropVolumeProcessMRMLEvents {this callerID event} {
  set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
  if { $caller == "" } {
    return
  }
  if { $caller == $::CropVolume($this,observedROINode) } {
    set visIcons $::CropVolume($this,visIcons)
    set roiVisibility $::CropVolume($this,roiVisibility)
    if { [$::CropVolume($this,observedROINode) GetVisibility] } {
      [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetVisibleIcon]
    } else {
      [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetInvisibleIcon]
    }
    CropVolumeUpdateLabelMap $this
  }
}

proc CropVolumeApply {this} {

#  if { ![info exists ::CropVolume($this,processing)] } { 
#    set ::CropVolume($this,processing) 0
#  }

#  if { $::CropVolume($this,processing) } {
#    return
#  }

  #
  # check that inputs are valid
  #
  set errorText ""
  set warningText ""
  
  set outVolumeNode [$::CropVolume($this,outputSelector) GetSelected]
  set roiNode [$::CropVolume($this,roiSelector) GetSelected]
  set volumeNode [$::CropVolume($this,inputSelector) GetSelected]
  set userSpacing [[$::CropVolume($this,samplingScale) GetWidget] GetValue]

  if { $volumeNode == "" || [$volumeNode GetImageData] == "" } {
    set errorText "Input volume data appears to be not initialized!"
  }
  if { $outVolumeNode == "" } {
    set errorText "Please specify output volume node!"
  }
  if { $roiNode == "" } {
    set errorText "Please specify ROI node!"
  }
  if {$volumeNode == $outVolumeNode} {
    set errorText "Input volume and output volume cannot be the same!"
  }
  if {$userSpacing <= 0.} {
    set errorText "Sampling must be a floating number greater than 0!"
  }

  if { $errorText != "" } {
    set dialog [vtkKWMessageDialog New]
    $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetStyleToMessage
    $dialog SetText $errorText
    $dialog Create
    $dialog Invoke
    $dialog Delete
    return
  }

  if { $warningText != "" } {
    set dialog [vtkKWMessageDialog New]
    $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
    $dialog SetStyleToMessage
    $dialog SetText $warningText
    $dialog Create
    $dialog Invoke
    $dialog Delete
  }

  # prepare the output volume node
#  $outVolumeNode CopyWithScene $volumeNode

  $outVolumeNode SetAndObserveStorageNodeID ""

  set inputRASToIJK [vtkMatrix4x4 New]
  set inputIJKToRAS [vtkMatrix4x4 New]
  set outputIJKToRAS [vtkMatrix4x4 New]
  set outputRASToIJK [vtkMatrix4x4 New]
  set volumeXform [vtkMatrix4x4 New]
  set roiXform [vtkMatrix4x4 New]
  set T [vtkMatrix4x4 New]

  $volumeNode GetRASToIJKMatrix $inputRASToIJK
  $volumeNode GetIJKToRASMatrix $inputIJKToRAS

  $outputRASToIJK Identity
  $outputIJKToRAS Identity

  $T Identity
  $roiXform Identity
  $volumeXform Identity
  
  set volumeTnode [$volumeNode GetParentTransformNode]
  if { $volumeTnode != ""} {
    $volumeTnode GetMatrixTransformToWorld $volumeXform
    $volumeXform Invert
  }

  set roiTnode [$roiNode GetParentTransformNode]
  if { $roiTnode != ""} {
    $roiTnode GetMatrixTransformToWorld $roiXform
  } 

  scan [$roiNode GetRadiusXYZ] "%f%f%f" rX rY rZ
  scan [$roiNode GetXYZ] "%f%f%f" cX cY cZ

  scan [$volumeNode GetSpacing] "%f%f%f" inputSpacingX inputSpacingY inputSpacingZ

  # scale spacing in each dimension by user-defined value
  set outputSpacingX [expr $inputSpacingX*$userSpacing]
  set outputSpacingY [expr $inputSpacingY*$userSpacing]
  set outputSpacingZ [expr $inputSpacingZ*$userSpacing]

  if { [eval $::CropVolume($this,isotropicResampling) GetSelectedState] } {
    set minSpacing $outputSpacingX
    if { $outputSpacingY < $minSpacing } {
      set minSpacing $outputSpacingY
    }
    if { $outputSpacingZ < $minSpacing} {
      set minSpacing $outputSpacingZ
    }
    set outputSpacingX $minSpacing
    set outputSpacingY $minSpacing
    set outputSpacingZ $minSpacing
  }

  set outputExtentX [expr int(2.*$rX/$outputSpacingX)]
  set outputExtentY [expr int(2.*$rY/$outputSpacingY)]
  set outputExtentZ [expr int(2.*$rZ/$outputSpacingZ)]

  $outputIJKToRAS SetElement 0 0 $outputSpacingX
  $outputIJKToRAS SetElement 1 1 $outputSpacingY
  $outputIJKToRAS SetElement 2 2 $outputSpacingZ
  $outputIJKToRAS SetElement 0 3 [expr $cX-$rX+$outputSpacingX*0.5]
  $outputIJKToRAS SetElement 1 3 [expr $cY-$rY+$outputSpacingY*0.5]
  $outputIJKToRAS SetElement 2 3 [expr $cZ-$rZ+$outputSpacingZ*0.5]

  # account for the ROI parent transform
  $outputIJKToRAS Multiply4x4 $roiXform $outputIJKToRAS $outputIJKToRAS

  $outputRASToIJK DeepCopy $outputIJKToRAS
  $outputRASToIJK Invert

  $T DeepCopy $outputIJKToRAS
  $T Multiply4x4 $volumeXform $T $T
  $T Multiply4x4 $inputRASToIJK $T $T

  set resliceT [vtkTransform New]
  $resliceT SetMatrix $T

  set nnButton [[$::CropVolume($this,resamplingFrame) GetWidget] GetWidget 0]
  set linButton [[$::CropVolume($this,resamplingFrame) GetWidget] GetWidget 1]
  set cubButton [[$::CropVolume($this,resamplingFrame) GetWidget] GetWidget 2]

  set reslicer [vtkImageReslice New]
  if { [$nnButton GetSelectedState] } {
    $reslicer SetInterpolationModeToNearestNeighbor
  }
  if { [$linButton GetSelectedState] } {
    $reslicer SetInterpolationModeToLinear
  }
  if { [$cubButton GetSelectedState] } {
    $reslicer SetInterpolationModeToCubic
  }

  set inImage [$volumeNode GetImageData]
  $reslicer SetInput [$volumeNode GetImageData]
  $reslicer SetOutputExtent  0 $outputExtentX 0 $outputExtentY 0 $outputExtentZ
  $reslicer SetOutputSpacing 1. 1. 1.
  $reslicer SetOutputOrigin 0. 0. 0.
  $reslicer SetResliceTransform $resliceT
  $reslicer UpdateWholeExtent

  set changeInf [vtkImageChangeInformation New]
  $changeInf SetInput [$reslicer GetOutput]
  $changeInf SetOutputOrigin 0. 0. 0.
  $changeInf SetOutputSpacing 1. 1. 1.
  $changeInf Update

  set outputImageData [vtkImageData New]
  $outputImageData DeepCopy [$changeInf GetOutput]

  $outVolumeNode SetAndObserveImageData $outputImageData
  $outputImageData Delete
  $outVolumeNode SetIJKToRASMatrix $outputIJKToRAS
  $outVolumeNode SetRASToIJKMatrix $outputRASToIJK
  
  # set a descriptive name for the output volume
  set inputName [$volumeNode GetName]
  set outputName "${inputName}-Subvolume-resample_scale-${userSpacing}"
  $outVolumeNode SetName $outputName
  $outVolumeNode SetLabelMap [$volumeNode GetLabelMap]

  $outVolumeNode Modified
  $outVolumeNode SetModifiedSinceRead 1

  $reslicer Delete
  $changeInf Delete
  $resliceT Delete
  $inputRASToIJK Delete
  $inputIJKToRAS Delete
  $outputIJKToRAS Delete
  $outputRASToIJK Delete
  $volumeXform Delete
  $roiXform Delete
  $T Delete

  set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]
  for { set j 0 } { $j < $numCnodes } { incr j } {
    set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
      if { [expr [$outVolumeNode GetLabelMap]] } {
        $cnode SetReferenceLabelVolumeID [$outVolumeNode GetID]
      } else {
        $cnode SetReferenceForegroundVolumeID ""
        $cnode SetReferenceBackgroundVolumeID [$outVolumeNode GetID]
      }
  }

  # setup the display node -- may be reset, if the output volume
  # exists, but it's ok
  # Warning: only scalar and label volumes are supported!
  if { [eval $volumeNode GetLabelMap] == 0 } {
    set outDisplayNode [vtkMRMLScalarVolumeDisplayNode New]
  } else {
    set outDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
  }
  $outDisplayNode Copy [$volumeNode GetDisplayNode]
  set scene [[$this GetLogic] GetMRMLScene]
  $scene AddNode $outDisplayNode
  $outVolumeNode SetAndObserveDisplayNodeID [$outDisplayNode GetID]
  $outDisplayNode Delete

  set scene [[$this GetLogic] GetMRMLScene]
  $scene InvokeEvent 66000
}

proc CropVolumeErrorDialog {this errorText} {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

proc CropVolumeUpdateLabelMap {this} {
  set roiNode [$::CropVolume($this,roiSelector) GetSelected]

  set roiTnode [$roiNode GetParentTransformNode]
  set roiXform [vtkMatrix4x4 New]
  $roiXform Identity

  if { $roiTnode != "" } {
    $roiTnode GetMatrixTransformToWorld $roiXform
  }

  scan [$roiNode GetRadiusXYZ] "%f%f%f" rX rY rZ
  scan [$roiNode GetXYZ] "%f%f%f" cX cY cZ

  set outputSpacingX [expr $rX*2.]
  set outputSpacingY [expr $rY*2.]
  set outputSpacingZ [expr $rZ*2.]

  set outputIJKToRAS [vtkMatrix4x4 New]
  $outputIJKToRAS SetElement 0 0 $outputSpacingX
  $outputIJKToRAS SetElement 1 1 $outputSpacingY
  $outputIJKToRAS SetElement 2 2 $outputSpacingZ
  $outputIJKToRAS SetElement 0 3 [expr $cX]
  $outputIJKToRAS SetElement 1 3 [expr $cY]
  $outputIJKToRAS SetElement 2 3 [expr $cZ]

  $outputIJKToRAS Multiply4x4 $roiXform $outputIJKToRAS $outputIJKToRAS
  
  $::CropVolume($this,roilabelNode) SetIJKToRASMatrix $outputIJKToRAS
  $::CropVolume($this,roilabelNode) Modified
  $::CropVolume($this,roilabelNode) SetModifiedSinceRead 1

  $outputIJKToRAS Delete
  $roiXform Delete
}

proc CropVolumeEnter {this} {

  set rwiRed $::CropVolume($this,rwiRedInteractorStyle)
  set rwiGreen $::CropVolume($this,rwiGreenInteractorStyle)
  set rwiYellow $::CropVolume($this,rwiYellowInteractorStyle)

#  set redGUIObserverTags $::CropVolume($this,redGUIObserverTags)
#  set greenGUIObserverTags $::CropVolume($this,greenGUIObserverTags)
#  set yellowGUIObserverTags $::CropVolume($this,yellowGUIObserverTags)

  lappend redGUIObserverTags [$::CropVolume($this,rwiRedInteractorStyle) \
    AddObserver LeftButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiRed LeftButtonPressEvent" 1]
  lappend redGUIObserverTags [$::CropVolume($this,rwiRedInteractorStyle) \
    AddObserver RightButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiRed RightButtonPressEvent" 1]

  lappend greenGUIObserverTags [$::CropVolume($this,rwiGreenInteractorStyle) \
    AddObserver LeftButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiGreen LeftButtonPressEvent" 1]
  lappend greenGUIObserverTags [$::CropVolume($this,rwiGreenInteractorStyle) \
    AddObserver RightButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiGreen RightButtonPressEvent" 1]

  lappend yellowGUIObserverTags [$::CropVolume($this,rwiYellowInteractorStyle) \
    AddObserver LeftButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiYellow LeftButtonPressEvent" 1]
  lappend yellowGUIObserverTags [$::CropVolume($this,rwiYellowInteractorStyle) \
    AddObserver RightButtonPressEvent "CropVolumeProcessGUIEvents $this \
    $rwiYellow RightButtonPressEvent" 1]

  set ::CropVolume($this,redGUIObserverTags) $redGUIObserverTags
  set ::CropVolume($this,greenGUIObserverTags) $greenGUIObserverTags
  set ::CropVolume($this,yellowGUIObserverTags) $yellowGUIObserverTags
}

proc CropVolumeExit {this} {

  set rwiRed $::CropVolume($this,rwiRedInteractorStyle)
  set rwiGreen $::CropVolume($this,rwiGreenInteractorStyle)
  set rwiYellow $::CropVolume($this,rwiYellowInteractorStyle)

  # remove all slice observers
  foreach tag $::CropVolume($this,redGUIObserverTags) {
    $rwiRed RemoveObserver $tag
  }
  foreach tag $::CropVolume($this,greenGUIObserverTags) {
    $rwiGreen RemoveObserver $tag
  }
  foreach tag $::CropVolume($this,yellowGUIObserverTags) {
    $rwiYellow RemoveObserver $tag
  }

  set ::CropVolume($this,redGUIObserverTags) ""
  set ::CropVolume($this,greenGUIObserverTags) ""
  set ::CropVolume($this,yellowGUIObserverTags) ""
}
