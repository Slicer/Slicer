#
# ExtractSubvolumeROI GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

#proc func {name argl body} {proc $name $argl [list expr $body]}
proc min {a b} {expr $a<$b? $a: $b}


proc ExtractSubvolumeROIConstructor {this} {
}

proc ExtractSubvolumeROIDestructor {this} {
}

proc ExtractSubvolumeROITearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    initFrame runButton inputSelector outputSelector roiSelector samplingScale
    resamplingFrame roiVisibility
  }
  set nonwidgets {
    visIcons
  }

  foreach w $widgets {
    $::ExtractSubvolumeROI($this,$w) SetParent ""
    $::ExtractSubvolumeROI($this,$w) Delete
  }
  foreach w $nonwidgets {
    $::ExtractSubvolumeROI($this,$w) Delete
  }


  $::ExtractSubvolumeROI($this,resliceFilter) Delete

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "ExtractSubvolumeROI"]
    [$this GetUIPanel] RemovePage "ExtractSubvolumeROI"
  }

  unset ::ExtractSubvolumeROI(singleton)

  catch {$::ExtractSubvolumeROI($this,labelMap) Delete}
}

proc ExtractSubvolumeROIBuildGUI {this} {

  set ::ExtractSubvolumeROI($this,redGUIObserverTags) ""
  set ::ExtractSubvolumeROI($this,greenGUIObserverTags) ""
  set ::ExtractSubvolumeROI($this,yellowGUIObserverTags) ""

  if { [info exists ::ExtractSubvolumeROI(singleton)] } {
    error "ExtractSubvolumeROI singleton already created"
  }
  set ::ExtractSubvolumeROI(singleton) $this

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
  [$this GetUIPanel] AddPage "ExtractSubvolumeROI" "ExtractSubvolumeROI" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "ExtractSubvolumeROI"]

  #
  # help frame
  #
  set helptext "This module extracts subvolume of the image described by Region of Interest widget.\n\nTo use:\n(1) specify input volume\n(2) specify ROI\n(3) adjust ROI\n(4) specify output volume\n(5) choose interpolation method\n(6) choose resampling scale\nNote: 0.5 as resampling scale will multiply spacing in each dimension by 0.5 to get new spacing (resampled image will have twice as many pixels in each dimension), 2 as resampling scale will result in the new image with two times *less* pixels in each dimension.\n\nUse mouse in slice views to adjust ROI: Left click -- resize, Right click -- re-center\n"
  set abouttext "This module was developed by Andriy Fedorov and Ron Kikinis.\nThis work was funded by Brain Science Foundation, and is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext

  set slicerBaseIcons [vtkSlicerBaseAcknowledgementLogoIcons New]

  set logo [vtkKWIcon New]
  set logoReader [vtkPNGReader New]
  set iconDir [file join [[$::ExtractSubvolumeROI(singleton) GetLogic] GetModuleShareDirectory] "ImageData"]
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
  # ExtractSubvolumeROI input and initialization parameters
  #
  set ::ExtractSubvolumeROI($this,initFrame) [vtkSlicerModuleCollapsibleFrame New]
  set initFrame $::ExtractSubvolumeROI($this,initFrame)
  $initFrame SetParent $pageWidget
  $initFrame Create
  $initFrame SetLabelText "Input/initialization parameters"
  pack [$initFrame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  # Input frame widgets

  set ::ExtractSubvolumeROI($this,inputSelector) [vtkSlicerNodeSelectorWidget New]
  set select $::ExtractSubvolumeROI($this,inputSelector)
  $select SetParent [$initFrame GetFrame]
  $select Create
  $select SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $select SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $select UpdateMenu
  $select SetLabelText "Input volume:"
  $select SetBalloonHelpString "The Source Volume to operate on"
  pack [$select GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::ExtractSubvolumeROI($this,roiSelector) [vtkSlicerNodeSelectorWidget New]
  set roi $::ExtractSubvolumeROI($this,roiSelector)
  $roi SetParent [$initFrame GetFrame]
  $roi Create
  $roi NewNodeEnabledOn
  $roi SetNodeClass "vtkMRMLROINode" "" "" "Subvolume_ROI"
  $roi SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $roi UpdateMenu
  $roi SetLabelText "ROI:"
  $roi SetBalloonHelpString "Region of interest that defines the subvolume"

  set ::ExtractSubvolumeROI($this,visIcons) [vtkSlicerVisibilityIcons New]
  set visIcons $::ExtractSubvolumeROI($this,visIcons)
  set ::ExtractSubvolumeROI($this,roiVisibility) [vtkKWPushButtonWithLabel New]
  set roiVisibility $::ExtractSubvolumeROI($this,roiVisibility)
  $roiVisibility SetParent [$initFrame GetFrame]
  $roiVisibility Create
  $roiVisibility SetLabelText "ROI visibility"
  $roiVisibility SetReliefToFlat
  $roiVisibility SetBalloonHelpString "Toggle visibility of ROI widget"
  [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetVisibleIcon]

  pack [$roi GetWidgetName] [$roiVisibility GetWidgetName] -side top -anchor e -padx 2 -pady 2
  
  # Output volume definition
  set ::ExtractSubvolumeROI($this,outputSelector) [vtkSlicerNodeSelectorWidget New]
  set out $::ExtractSubvolumeROI($this,outputSelector)
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
  set ::ExtractSubvolumeROI($this,samplingScale) [vtkKWSpinBoxWithLabel New]
  set sampling $::ExtractSubvolumeROI($this,samplingScale)
  $sampling SetParent [$initFrame GetFrame]
  $sampling Create
  $sampling SetLabelText "Input spacing scaling constant"
  [$sampling GetWidget] SetValue 1.
  $sampling SetBalloonHelpString "Spacing of the input image in each dimension will be multiplied by this number to get output spacing. E.g., \"1\" will preserve original spacing, \"0.5\" will *increase* number of pixels in each dimension by 2, \"2\" will *reduce* number of pixels in each dimension by 2"
  pack [$sampling GetWidgetName] -side top -anchor e -padx 2 -pady 2

  # Resampler selection
  set ::ExtractSubvolumeROI($this,resamplingFrame) [vtkKWRadioButtonSetWithLabel New]
  set resamplingFrame $::ExtractSubvolumeROI($this,resamplingFrame)
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

  set ::ExtractSubvolumeROI($this,runButton) [vtkKWPushButton New]
  set run $::ExtractSubvolumeROI($this,runButton)
  $run SetParent [$initFrame GetFrame]
  $run Create
  $run SetText "Do ROI resample"
  pack [$run GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  set ::ExtractSubvolumeROI($this,resliceFilter) [vtkImageReslice New]
  set ::ExtractSubvolumeROI($this,inputVolume) ""

  #  set up pointers, observers will be initialized on module enter
  set appGUI $::slicer3::ApplicationGUI
  set ::ExtractSubvolumeROI($this,rwiRedInteractorStyle) \
    [[[[[$appGUI GetMainSliceGUI "Red"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor] GetInteractorStyle]
  set ::ExtractSubvolumeROI($this,rwiGreenInteractorStyle) \
    [[[[$appGUI GetMainSliceGUI "Green"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor]
  set ::ExtractSubvolumeROI($this,rwiYellowInteractorStyle) \
    [[[[$appGUI GetMainSliceGUI "Yellow"] GetSliceViewer] \
    GetRenderWidget ] GetRenderWindowInteractor]
}

proc ExtractSubvolumeROIAddGUIObservers {this} {
  $this AddObserverByNumber $::ExtractSubvolumeROI($this,runButton) 10000 
  $this AddObserverByNumber $::ExtractSubvolumeROI($this,inputSelector)  11000
  $this AddObserverByNumber $::ExtractSubvolumeROI($this,outputSelector)  10000
  $this AddObserverByNumber $::ExtractSubvolumeROI($this,roiSelector) 11000
  $this AddObserverByNumber [$::ExtractSubvolumeROI($this,roiVisibility) GetWidget] 10000
#  $this AddMRMLObserverByNumber [[[$this GetLogic] GetApplicationLogic] GetSelectionNode] 33
    
  # From Slicer console
  # set style [[[[[$::slicer3::ApplicationGUI GetMainSliceGUI "Red"]
  # GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]
  # GetInteractorStyle]
#  [ $rwiGreen GetInteractorStyle ] AddObserver 12 
#  [ $rwiYellow GetInteractorStyle ] AddObserver 12 

}

proc ExtractSubvolumeROIRemoveGUIObservers {this} {
}

proc ExtractSubvolumeROIRemoveLogicObservers {this} {
}

proc ExtractSubvolumeROIRemoveMRMLNodeObservers {this} {
  catch {$this RemoveMRMLObserverByNumber $::ExtractSubvolumeROI($this,observedROINode) \
    [$this GetNumberForVTKevent ModifiedEvent] }
}

proc ExtractSubvolumeROIProcessLogicEvents {this caller event} {
}

proc ExtractSubvolumeROIProcessGUIEvents {this caller event} {
  if { $caller == $::ExtractSubvolumeROI($this,runButton) } {
  
    set inputVolume [$::ExtractSubvolumeROI($this,inputSelector) GetSelected]
    if {$inputVolume == ""} {
      ExtractSubvolumeROIErrorDialog $this "Please specify input volume!"
      return
    }
    ExtractSubvolumeROIApply $this
  } 

  if {$caller == $::ExtractSubvolumeROI($this,roiSelector)} {
    catch {$this RemoveMRMLObserverByNumber $::ExtractSubvolumeROI($this,observedROINode) \
      [$this GetNumberForVTKEvent ModifiedEvent] }
    set ::ExtractSubvolumeROI($this,observedROINode) [$::ExtractSubvolumeROI($this,roiSelector) GetSelected]
    $this AddMRMLObserverByNumber $::ExtractSubvolumeROI($this,observedROINode) \
      [$this GetNumberForVTKEvent ModifiedEvent]
  }

  if {$caller == [$::ExtractSubvolumeROI($this,roiVisibility) GetWidget] } {
    if {$::ExtractSubvolumeROI($this,observedROINode) == ""} {
      return
    }
    if { [$::ExtractSubvolumeROI($this,observedROINode) GetVisibility] } {
      $::ExtractSubvolumeROI($this,observedROINode) SetVisibility 0
    } else {
      $::ExtractSubvolumeROI($this,observedROINode) SetVisibility 1
    }
    $::ExtractSubvolumeROI($this,observedROINode) Modified
  }

  if {$caller == $::ExtractSubvolumeROI($this,inputSelector) || \
      $caller == $::ExtractSubvolumeROI($this,roiSelector) } {
    set roiNode [$::ExtractSubvolumeROI($this,roiSelector) GetSelected]

    set inputVolume [$::ExtractSubvolumeROI($this,inputSelector) GetSelected]
    if {$inputVolume == "" || $inputVolume == $::ExtractSubvolumeROI($this,inputVolume) || $roiNode == ""} {
      return
    }
    set ::ExtractSubvolumeROI($this,inputVolume) $inputVolume
    set dim [[$inputVolume GetImageData] GetWholeExtent]
    catch {$::ExtractSubvolumeROI($this,labelMap) Delete}
    set ::ExtractSubvolumeROI($this,labelMap) [vtkImageRectangularSource New]
    set labelMap $::ExtractSubvolumeROI($this,labelMap)
    $labelMap SetWholeExtent 0 [expr [lindex $dim 1]] 0 [expr [lindex $dim 3]] 0 [expr [lindex $dim 5]]
    $labelMap SetOutputScalarTypeToShort
    $labelMap SetInsideGraySlopeFlag 0
    $labelMap SetInValue 17
    $labelMap SetOutValue 0
    $labelMap Update

    # create label volume to keep ROI visualization
    # TODO: delete the older label volume if it existed
    set scene [[$this GetLogic] GetMRMLScene]
    set volumesLogic [$::slicer3::VolumesGUI GetLogic]
    set ::ExtractSubvolumeROI($this,labelMapNode) [$volumesLogic CreateLabelVolume $scene $inputVolume "Subvolume_ROI_Label"]
#    $::ExtractSubvolumeROI($this,labelMapNode) SetHideFromEditors 1
    $::ExtractSubvolumeROI($this,labelMapNode) SetAndObserveImageData [$labelMap GetOutput]

    set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
    $selectionNode SetReferenceActiveLabelVolumeID [$::ExtractSubvolumeROI($this,labelMapNode) GetID]
    $selectionNode Modified
    [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 0

    # set the label node
    set redCompositeNode [[[$::slicer3::ApplicationGUI GetMainSliceGUI "Red"] GetLogic ] GetSliceCompositeNode]
    set greenCompositeNode [[[$::slicer3::ApplicationGUI GetMainSliceGUI "Green"] GetLogic ] GetSliceCompositeNode]
    set yellowCompositeNode [[[$::slicer3::ApplicationGUI GetMainSliceGUI "Yellow"] GetLogic ] GetSliceCompositeNode]
    $redCompositeNode SetLabelOpacity .6
    $greenCompositeNode SetLabelOpacity .6
    $yellowCompositeNode SetLabelOpacity .6

    # trigger update of the label image contents
    ExtractSubvolumeROIUpdateLabelMap $this
  }


  if {$event == "LeftButtonPressEvent" || $event == "RightButtonPressEvent"} {
  
    set redStyle $::ExtractSubvolumeROI($this,rwiRedInteractorStyle)
    set greenStyle $::ExtractSubvolumeROI($this,rwiGreenInteractorStyle)
    set yellowStyle $::ExtractSubvolumeROI($this,rwiYellowInteractorStyle)

    set roiNode [$::ExtractSubvolumeROI($this,roiSelector) GetSelected]
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
# Accessors to ExtractSubvolumeROI state
#


# get the ExtractSubvolumeROI parameter node, or create one if it doesn't exist
proc ExtractSubvolumeROICreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "ExtractSubvolumeROI"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the ExtractSubvolumeROI parameter node, or create one if it doesn't exist
proc ExtractSubvolumeROIGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "ExtractSubvolumeROI" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    ExtractSubvolumeROICreateParameterNode
    set node [ExtractSubvolumeROIGetParameterNode]
  }

  return $node
}


proc ExtractSubvolumeROIGetLabel {} {
  set node [ExtractSubvolumeROIGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc ExtractSubvolumeROISetLabel {index} {
  set node [ExtractSubvolumeROIGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc ExtractSubvolumeROIUpdateMRML {this} {
}

proc ExtractSubvolumeROIProcessMRMLEvents {this callerID event} {
  set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
  if { $caller == "" } {
    return
  }
  if { $caller == $::ExtractSubvolumeROI($this,observedROINode) } {
    set visIcons $::ExtractSubvolumeROI($this,visIcons)
    set roiVisibility $::ExtractSubvolumeROI($this,roiVisibility)
    if { [$::ExtractSubvolumeROI($this,observedROINode) GetVisibility] } {
      [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetVisibleIcon]
    } else {
      [$roiVisibility GetWidget] SetImageToIcon [$visIcons GetInvisibleIcon]
    }
    if { $::ExtractSubvolumeROI($this,labelMap) == ""} {
      return
    }
    ExtractSubvolumeROIUpdateLabelMap $this
  }
}

proc ExtractSubvolumeROIApply {this} {

#  if { ![info exists ::ExtractSubvolumeROI($this,processing)] } { 
#    set ::ExtractSubvolumeROI($this,processing) 0
#  }

#  if { $::ExtractSubvolumeROI($this,processing) } {
#    return
#  }

  #
  # check that inputs are valid
  #
  set errorText ""
  
  set outVolumeNode [$::ExtractSubvolumeROI($this,outputSelector) GetSelected]
  set roiNode [$::ExtractSubvolumeROI($this,roiSelector) GetSelected]
  set volumeNode [$::ExtractSubvolumeROI($this,inputSelector) GetSelected]
  set userSpacing [[$::ExtractSubvolumeROI($this,samplingScale) GetWidget] GetValue]

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
  if { [$volumeNode GetParentTransformNode] != "" } {
    set errorText "Input volume cannot be under transform!"
  }
  if { [$roiNode GetParentTransformNode] != "" } {
    set errorText "ROI node cannot be under transform!"
  }

  set ijk2ras [vtkMatrix4x4 New]
  $volumeNode GetIJKToRASMatrix $ijk2ras
  set m01 [$ijk2ras GetElement 0 1]
  set m02 [$ijk2ras GetElement 0 2]
  set m12 [$ijk2ras GetElement 1 2]
  $ijk2ras Delete

  if {[expr abs($m01) || abs($m02) || abs($m12)]} {
    set errorText "The specified input volume is not axis-aligned! \
    ExtractSubvolumeROI does not support non axis-aligned input images at this \
    time.\n\nPlease resample the input image to be axis-aligned to use this \
    module."
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

  set ijk2ras [vtkMatrix4x4 New]
  set ras2ijk [vtkMatrix4x4 New]
  $volumeNode GetIJKToRASMatrix $ijk2ras
  $volumeNode GetRASToIJKMatrix $ras2ijk

  set bboxIJKMinX [expr int($::ExtractSubvolumeROI($this,bboxIJKMinX))]
  set bboxIJKMinY [expr int($::ExtractSubvolumeROI($this,bboxIJKMinY))]
  set bboxIJKMinZ [expr int($::ExtractSubvolumeROI($this,bboxIJKMinZ))]
  set bboxIJKMaxX [expr int($::ExtractSubvolumeROI($this,bboxIJKMaxX))]
  set bboxIJKMaxY [expr int($::ExtractSubvolumeROI($this,bboxIJKMaxY))]
  set bboxIJKMaxZ [expr int($::ExtractSubvolumeROI($this,bboxIJKMaxZ))]

  # prepare the output volume node
#  $outVolumeNode CopyWithScene $volumeNode
  if { [$outVolumeNode GetDisplayNode] == ""} {
    set outDisplayNode [vtkMRMLScalarVolumeDisplayNode New]
    $outDisplayNode CopyWithScene [$volumeNode GetDisplayNode]
    set scene [[$this GetLogic] GetMRMLScene]
    $scene AddNodeNoNotify $outDisplayNode
    $outVolumeNode SetAndObserveDisplayNodeID [$outDisplayNode GetID]
    $outDisplayNode Delete
  }

  $outVolumeNode SetAndObserveStorageNodeID ""
  $outVolumeNode SetAndObserveImageData ""
  $outVolumeNode SetRASToIJKMatrix $ras2ijk
  $outVolumeNode SetIJKToRASMatrix $ijk2ras
  $outVolumeNode SetModifiedSinceRead 1
  
  set newOrigin [lrange [eval $ijk2ras MultiplyPoint $bboxIJKMinX $bboxIJKMinY $bboxIJKMinZ 1] 0 2]

  set inputSpacing [$volumeNode GetSpacing]

  set changeInf [vtkImageChangeInformation New]
  $changeInf SetInput [$volumeNode GetImageData]
  eval $changeInf SetOutputSpacing $inputSpacing
  $changeInf Update

  set imageClip [vtkImageClip New]
  $imageClip SetInput [$changeInf GetOutput]
  $imageClip SetOutputWholeExtent $bboxIJKMinX $bboxIJKMaxX $bboxIJKMinY \
                                  $bboxIJKMaxY $bboxIJKMinZ $bboxIJKMaxZ
  $imageClip ClipDataOn
  $imageClip Update

  set changeInf2 [vtkImageChangeInformation New]
  $changeInf2 SetInput [$imageClip GetOutput]
  $changeInf2 SetOutputExtentStart 0 0 0
  $changeInf2 Update
  

#  puts "Resampling spacing: $newSpacing"

  set dimBefore [ [$changeInf2 GetOutput] GetDimensions]
  
  set resampler [vtkImageResample New]
  $resampler SetDimensionality 3

  set nnButton [[$::ExtractSubvolumeROI($this,resamplingFrame) GetWidget] GetWidget 0]
  set linButton [[$::ExtractSubvolumeROI($this,resamplingFrame) GetWidget] GetWidget 1]
  set cubButton [[$::ExtractSubvolumeROI($this,resamplingFrame) GetWidget] GetWidget 2]

  if { [$nnButton GetSelectedState] } {
    $resampler SetInterpolationModeToNearestNeighbor
  }
  if { [$linButton GetSelectedState] } {
    $resampler SetInterpolationModeToLinear
  }
  if { [$cubButton GetSelectedState] } {
    $resampler SetInterpolationModeToCubic
  }
 
  $resampler SetInput [$changeInf2 GetOutput]
  
  set newSpacingX [expr [lindex $inputSpacing 0]*$userSpacing]
  set newSpacingY [expr [lindex $inputSpacing 1]*$userSpacing]
  set newSpacingZ [expr [lindex $inputSpacing 2]*$userSpacing]

  $resampler SetAxisOutputSpacing 0 $newSpacingX
  $resampler SetAxisOutputSpacing 1 $newSpacingY
  $resampler SetAxisOutputSpacing 2 $newSpacingZ
  $resampler ReleaseDataFlagOff 
  $resampler Update

  set dimAfter [ [$resampler GetOutput] GetDimensions]

  set changeInf3 [vtkImageChangeInformation New]
  $changeInf3 SetInput [$resampler GetOutput]
  $changeInf3 SetOutputSpacing 1. 1. 1.
  $changeInf3 Update

  set output [vtkImageData New]
  $output DeepCopy [$changeInf3 GetOutput]

  $outVolumeNode SetAndObserveImageData $output
  eval $outVolumeNode SetSpacing "$newSpacingX $newSpacingY $newSpacingZ"
  eval $outVolumeNode SetOrigin $newOrigin
  $outVolumeNode Modified

  $ras2ijk Delete
  $ijk2ras Delete

  $changeInf Delete
  $changeInf2 Delete
  $changeInf3 Delete
  $output Delete
  $resampler Delete
  $imageClip Delete

  # set background volume to the resampled volume
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$outVolumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID ""
  $selectionNode Modified
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 1

  # set a descriptive name for the output volume
  set inputName [$volumeNode GetName]
  set outputName "${inputName}-Subvolume-resample_scale-${userSpacing}"
  $outVolumeNode SetName $outputName
  $outVolumeNode Modified

  set scene [[$this GetLogic] GetMRMLScene]
  $scene InvokeEvent 66000
}

proc ExtractSubvolumeROIErrorDialog {this errorText} {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

proc ExtractSubvolumeROIUpdateLabelMap {this} {
  set labelMap $::ExtractSubvolumeROI($this,labelMap)
  set roiNode $::ExtractSubvolumeROI($this,observedROINode)
  set volumeNode $::ExtractSubvolumeROI($this,inputVolume)

  set roiXYZ [$roiNode GetXYZ]
  set roiRadiusXYZ [$roiNode GetRadiusXYZ]

  set bboxRAS0(0) {[expr [lindex $roiXYZ 0] - [lindex $roiRadiusXYZ 0]]}
  set bboxRAS0(1) {[expr [lindex $roiXYZ 1] - [lindex $roiRadiusXYZ 1]]}
  set bboxRAS0(2) {[expr [lindex $roiXYZ 2] - [lindex $roiRadiusXYZ 2]]}

  set bboxRAS1(0) {[expr [lindex $roiXYZ 0] + [lindex $roiRadiusXYZ 0]]}
  set bboxRAS1(1) {[expr [lindex $roiXYZ 1] + [lindex $roiRadiusXYZ 1]]}
  set bboxRAS1(2) {[expr [lindex $roiXYZ 2] + [lindex $roiRadiusXYZ 2]]}

  set ras2ijk [vtkMatrix4x4 New]
  $volumeNode GetRASToIJKMatrix $ras2ijk
  scan [lrange [eval $ras2ijk MultiplyPoint $bboxRAS0(0) $bboxRAS0(1) $bboxRAS0(2) 1 ] 0 2] "%f%f%f" \
    bboxIJK0(0) bboxIJK0(1) bboxIJK0(2)
  scan [lrange [eval $ras2ijk MultiplyPoint $bboxRAS1(0) $bboxRAS1(1) $bboxRAS1(2) 1 ] 0 2] "%f%f%f" \
    bboxIJK1(0) bboxIJK1(1) bboxIJK1(2)
  set roiIJK [lrange [eval $ras2ijk MultiplyPoint [lindex $roiXYZ 0] [lindex $roiXYZ 1] [lindex $roiXYZ 2] 1] 0 2]
  $ras2ijk Delete

  for {set i 0} {$i<3} {incr i} {
    if {$bboxIJK0($i) > $bboxIJK1($i)} {
      set tmp $bboxIJK0($i)
      set bboxIJK0($i) $bboxIJK1($i)
      set bboxIJK1($i) $tmp
    }
  }

  

  $labelMap SetCenter [expr int([lindex $roiIJK 0])] [expr int([lindex $roiIJK 1])] [expr int([lindex $roiIJK 2])]
  set sizeX [expr int([expr $bboxIJK1(0)-$bboxIJK0(0)-1])]
  set sizeY [expr int([expr $bboxIJK1(1)-$bboxIJK0(1)-1])]
  set sizeZ [expr int([expr $bboxIJK1(2)-$bboxIJK0(2)-1])]

  $labelMap SetSize $sizeX $sizeY $sizeZ
  $labelMap Update
  $::ExtractSubvolumeROI($this,labelMapNode) Modified

  set ::ExtractSubvolumeROI($this,bboxIJKMinX) [expr int($bboxIJK0(0))]
  set ::ExtractSubvolumeROI($this,bboxIJKMinY) [expr int($bboxIJK0(1))]
  set ::ExtractSubvolumeROI($this,bboxIJKMinZ) [expr int($bboxIJK0(2))]

  set ::ExtractSubvolumeROI($this,bboxIJKMaxX) [expr int($bboxIJK1(0))]
  set ::ExtractSubvolumeROI($this,bboxIJKMaxY) [expr int($bboxIJK1(1))]
  set ::ExtractSubvolumeROI($this,bboxIJKMaxZ) [expr int($bboxIJK1(2))]
}

proc ExtractSubvolumeROICreateOutputVolume {this} {

  set volumeNode [$::ExtractSubvolumeROI($this,inputSelector) GetSelected]
  set outputVolumeNode [$::ExtractSubvolumeROI($this,outputSelector) GetSelected]

  set inputVolumeName [$volumeNode GetName]
  set outputVolumeName [$outputVolumeNode GetName]
  $outputVolumeNode SetName "ResampledVolume"

  # from vtkSlicerVolumesLogic
  set outputDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
  set scene [[$this GetLogic] GetMRMLScene]
  $scene AddNode $outputDisplayNode

#  $outputDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeLabels"

  $outputVolumeNode SetAndObserveDisplayNodeID [$outputDisplayNode GetID]
  $outputVolumeNode SetModifiedSinceRead 1
#  $outputVolumeNode SetLabelMap 1
  
  set thresh [vtkImageThreshold New]
  $thresh SetReplaceIn 1
  $thresh SetReplaceOut 1
  $thresh SetInValue 0
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToShort
  $thresh SetInput [$volumeNode GetImageData]
  [$thresh GetOutput] Update
  $outputVolumeNode SetAndObserveImageData [$thresh GetOutput]

  $thresh Delete
  $outputDisplayNode Delete

  set ::FastMarchingSegmentation($this,labelVolume) $outputVolumeNode

  set ras2ijk [vtkMatrix4x4 New]
  set ijk2ras [vtkMatrix4x4 New]
  $volumeNode GetRASToIJKMatrix $ras2ijk
  $volumeNode GetIJKToRASMatrix $ijk2ras

  $outputVolumeNode SetRASToIJKMatrix $ras2ijk
  $outputVolumeNode SetIJKToRASMatrix $ijk2ras
  
  $ras2ijk Delete
  $ijk2ras Delete

  scan [$volumeNode GetOrigin] "%f%f%f" originX originY originZ
  $outputVolumeNode SetOrigin $originX $originY $originZ

  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveLabelVolumeID [$outputVolumeNode GetID]
  $selectionNode Modified
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 0

  # this is here to trigger updates on node selectors
  $scene InvokeEvent 66000
  return
}

proc ExtractSubvolumeROIEnter {this} {

  set rwiRed $::ExtractSubvolumeROI($this,rwiRedInteractorStyle)
  set rwiGreen $::ExtractSubvolumeROI($this,rwiGreenInteractorStyle)
  set rwiYellow $::ExtractSubvolumeROI($this,rwiYellowInteractorStyle)

#  set redGUIObserverTags $::ExtractSubvolumeROI($this,redGUIObserverTags)
#  set greenGUIObserverTags $::ExtractSubvolumeROI($this,greenGUIObserverTags)
#  set yellowGUIObserverTags $::ExtractSubvolumeROI($this,yellowGUIObserverTags)

  lappend redGUIObserverTags [$::ExtractSubvolumeROI($this,rwiRedInteractorStyle) \
    AddObserver LeftButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiRed LeftButtonPressEvent" 1]
  lappend redGUIObserverTags [$::ExtractSubvolumeROI($this,rwiRedInteractorStyle) \
    AddObserver RightButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiRed RightButtonPressEvent" 1]

  lappend greenGUIObserverTags [$::ExtractSubvolumeROI($this,rwiGreenInteractorStyle) \
    AddObserver LeftButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiGreen LeftButtonPressEvent" 1]
  lappend greenGUIObserverTags [$::ExtractSubvolumeROI($this,rwiGreenInteractorStyle) \
    AddObserver RightButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiGreen RightButtonPressEvent" 1]

  lappend yellowGUIObserverTags [$::ExtractSubvolumeROI($this,rwiYellowInteractorStyle) \
    AddObserver LeftButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiYellow LeftButtonPressEvent" 1]
  lappend yellowGUIObserverTags [$::ExtractSubvolumeROI($this,rwiYellowInteractorStyle) \
    AddObserver RightButtonPressEvent "ExtractSubvolumeROIProcessGUIEvents $this \
    $rwiYellow RightButtonPressEvent" 1]

  set ::ExtractSubvolumeROI($this,redGUIObserverTags) $redGUIObserverTags
  set ::ExtractSubvolumeROI($this,greenGUIObserverTags) $greenGUIObserverTags
  set ::ExtractSubvolumeROI($this,yellowGUIObserverTags) $yellowGUIObserverTags
}

proc ExtractSubvolumeROIExit {this} {

  set rwiRed $::ExtractSubvolumeROI($this,rwiRedInteractorStyle)
  set rwiGreen $::ExtractSubvolumeROI($this,rwiGreenInteractorStyle)
  set rwiYellow $::ExtractSubvolumeROI($this,rwiYellowInteractorStyle)

  # remove all slice observers
  foreach tag $::ExtractSubvolumeROI($this,redGUIObserverTags) {
    $rwiRed RemoveObserver $tag
  }
  foreach tag $::ExtractSubvolumeROI($this,greenGUIObserverTags) {
    $rwiGreen RemoveObserver $tag
  }
  foreach tag $::ExtractSubvolumeROI($this,yellowGUIObserverTags) {
    $rwiYellow RemoveObserver $tag
  }

  set ::ExtractSubvolumeROI($this,redGUIObserverTags) ""
  set ::ExtractSubvolumeROI($this,greenGUIObserverTags) ""
  set ::ExtractSubvolumeROI($this,yellowGUIObserverTags) ""
}
