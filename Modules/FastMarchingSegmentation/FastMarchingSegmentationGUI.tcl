#
# FastMarchingSegmentation GUI Procs
# - the 'this' argument to all procs is a vtkScriptedModuleGUI
#

proc FastMarchingSegmentationConstructor {this} {
}

proc FastMarchingSegmentationDestructor {this} {
}

proc FastMarchingSegmentationTearDownGUI {this} {


  # nodeSelector  ;# disabled for now
  set widgets {
    outputSelector
    runButton inputSelector 
    fiducialsSelector segVolumeThumbWheel
    timeScrollScale labelColorSpin
    initFrame outputParametersFrame
    fmFrame
    timescrollRange
    volRenderCheckbox
    acceptButton
  }

  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) SetParent ""
    $::FastMarchingSegmentation($this,$w) Delete
  }

  set filters {
    cast rescale fastMarchingFilter
  }

  foreach f $filters {
    $::FastMarchingSegmentation($this,$f) Delete
  }

  # volume rendering
  catch {$::FastMarchingSegmentation($this,renderMapper) Delete}
  catch {$::FastMarchingSegmentation($this,renderVolumeProperty) Delete}
  catch {$::FastMarchingSegmentation($this,renderVolume) Delete}
  catch {$::FastMarchingSegmentation($this,renderFilter) Delete}
  catch {$::FastMarchingSegmentation($this,renderColorMapping) Delete}

  if { [[$this GetUIPanel] GetUserInterfaceManager] != "" } {
    set pageWidget [[$this GetUIPanel] GetPageWidget "FastMarchingSegmentation"]
    [$this GetUIPanel] RemovePage "FastMarchingSegmentation"
  }

  unset ::FastMarchingSegmentation(singleton)

}

proc FastMarchingSegmentationBuildGUI {this} {

  if { [info exists ::FastMarchingSegmentation(singleton)] } {
    error "FastMarchingSegmentation singleton already created"
  }
  set ::FastMarchingSegmentation(singleton) $this

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


  $this SetCategory "Segmentation"
  [$this GetUIPanel] AddPage "FastMarchingSegmentation" "FastMarchingSegmentation" ""
  set pageWidget [[$this GetUIPanel] GetPageWidget "FastMarchingSegmentation"]

  #
  # help frame
  #
  set helptext "This module performs segmentation using fast marching method with automatic estimation of region statistics. The core C++ classes were contributed by Eric Pichon in slicer2.\n\nIn order to benefit from this module, please use the following steps:\n(1) specify input scalar volume to be segmented\n(2) define fiducial seeds within the region you want to segment\n(3) specify the expected volume of the structure to be segmented. Note, overestimation of this volume is OK, because you will be able to adjust the actual volume once the segmentation is complete\n(4) specify the label color for the segmentation\n(5) Run sementation\n(6) use volume control slider to adjust segmentation result.\nDocumentation: <a>http://wiki.slicer.org/slicerWiki/index.php/Modules:FastMarchingSegmentation-Documentation-3.6</a>"
  set abouttext "This module was designed and implemented by Andriy Fedorov and Ron Kikinis based on the original implementation of Eric Pichon in Slicer2. PI: Ron Kikinis. \nThis work was funded by Brain Science Foundation, and is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
  $this BuildHelpAndAboutFrame $pageWidget $helptext $abouttext

  set slicerBaseIcons [vtkSlicerBaseAcknowledgementLogoIcons New]

  set logo [vtkKWIcon New]
  set logoReader [vtkPNGReader New]
  set iconDir [file join [[$::FastMarchingSegmentation(singleton) GetLogic] GetModuleShareDirectory] "ImageData"]
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
  # FastMarchingSegmentation main frame
  #
  set ::FastMarchingSegmentation($this,fmFrame) [vtkSlicerModuleCollapsibleFrame New]
  set fmFrame $::FastMarchingSegmentation($this,fmFrame)
  $fmFrame SetParent $pageWidget
  $fmFrame Create
  $fmFrame SetLabelText "Fast Marching Segmentation"
  pack [$fmFrame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2 -in [$pageWidget GetWidgetName]

  #
  # FastMarchingSegmentation input and initialization parameters
  #
  set ::FastMarchingSegmentation($this,initFrame) [vtkSlicerModuleCollapsibleFrame New]
  set initFrame $::FastMarchingSegmentation($this,initFrame)
  $initFrame SetParent [$fmFrame GetFrame]
  $initFrame Create
  $initFrame SetLabelText "IO and initialization parameters"
  pack [$initFrame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2

  #
  # FastMarchingSegmentation output parameters
  #
  set ::FastMarchingSegmentation($this,outputParametersFrame) [vtkSlicerModuleCollapsibleFrame New]
  set outputParametersFrame $::FastMarchingSegmentation($this,outputParametersFrame)
  $outputParametersFrame SetParent [$fmFrame GetFrame]
  $outputParametersFrame Create
  $outputParametersFrame SetLabelText "Segmentation result adjustment"
  pack [$outputParametersFrame GetWidgetName] \
    -side top -anchor nw -fill x -padx 2 -pady 2

  # Input frame widgets

  set ::FastMarchingSegmentation($this,inputSelector) [vtkSlicerNodeSelectorWidget New]
  set select $::FastMarchingSegmentation($this,inputSelector)
  $select SetParent [$initFrame GetFrame]
  $select Create
  $select SetNodeClass "vtkMRMLScalarVolumeNode" "" "" ""
  $select SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $select UpdateMenu
  $select SetLabelText "Input volume:"
  $select SetLabelWidth 15
  $select SetBalloonHelpString "The Source Volume to operate on"
  pack [$select GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  set ::FastMarchingSegmentation($this,fiducialsSelector) [vtkSlicerNodeSelectorWidget New]
  set fiducials $::FastMarchingSegmentation($this,fiducialsSelector)
  $fiducials SetParent [$initFrame GetFrame]
  $fiducials Create
  $fiducials NewNodeEnabledOn
  $fiducials SetNodeClass "vtkMRMLFiducialListNode" "" "" "FM"
  $fiducials SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $fiducials UpdateMenu
  $fiducials SetLabelText "Input seeds:"
  $fiducials SetLabelWidth 15
  $fiducials SetBalloonHelpString "List of fiducials to be used as seeds for segmentation"
  pack [$fiducials GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  set ::FastMarchingSegmentation($this,outputSelector) [vtkSlicerNodeSelectorWidget New]
  set outselect $::FastMarchingSegmentation($this,outputSelector)
  $outselect SetParent [$initFrame GetFrame]
  $outselect Create
  $outselect NewNodeEnabledOn
  $outselect SetNodeClass "vtkMRMLScalarVolumeNode" "LabelMap" "1" "FastMarchingLabel"
  $outselect SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $outselect UpdateMenu
  $outselect SetLabelText "Output volume:"
  $outselect SetLabelWidth 15
  $outselect SetBalloonHelpString "The Output Volume to keep segmentation result"
  pack [$outselect GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x
 
  set ::FastMarchingSegmentation($this,segVolumeThumbWheel) [vtkKWThumbWheel New]
  set segvolume $::FastMarchingSegmentation($this,segVolumeThumbWheel)
  $segvolume SetParent [$initFrame GetFrame]
  $segvolume PopupModeOn
  $segvolume Create
  $segvolume DisplayEntryAndLabelOnTopOn
  $segvolume DisplayEntryOn
  $segvolume DisplayLabelOn
  $segvolume SetResolution 1
  $segvolume SetMinimumValue 0
  $segvolume SetClampMinimumValue 0
  $segvolume SetValue 0
  [$segvolume GetLabel] SetText "Target segmented volume (mL):"
  $segvolume SetBalloonHelpString "Overestimate of the segmented structure volume. Hint: human mandible is about 30 mL, human head is approximately 5000 mL"
  pack [$segvolume GetWidgetName] -side top -anchor e -padx 2 -pady 2

  set ::FastMarchingSegmentation($this,labelColorSpin) [vtkKWScaleWithEntry New]
  set outColor $::FastMarchingSegmentation($this,labelColorSpin)
  $outColor SetParent [$initFrame GetFrame]
  $outColor PopupModeOn
  $outColor Create
  $outColor SetLabelText "Output label value:"
  $outColor SetLabelWidth 20
  $outColor RangeVisibilityOn
  $outColor SetResolution 1
  $outColor SetRange 1 255
  $outColor SetValue 6.0
  $outColor SetBalloonHelpString "Specify color for the output label"
  pack [$outColor GetWidgetName] -side top -anchor e -padx 2 -pady 2

  set ::FastMarchingSegmentation($this,runButton) [vtkKWPushButton New]
  set run $::FastMarchingSegmentation($this,runButton)
  $run SetParent [$initFrame GetFrame]
  $run Create
  $run SetText "Run Segmentation"
  pack [$run GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  # FastMarching output parameters
  set ::FastMarchingSegmentation($this,volRenderCheckbox) [vtkKWCheckButton New]
  set ckbutton $::FastMarchingSegmentation($this,volRenderCheckbox)
  $ckbutton SetParent [$outputParametersFrame GetFrame]
  $ckbutton Create
  $ckbutton SetText "interactive volume rendering of segmentation results"
  $ckbutton SelectedStateOff
  $ckbutton SetBalloonHelpString "Uncheck this to disable volume rendering if\
    you experience performance problems"
  pack [$ckbutton GetWidgetName] -side top -anchor e -padx 2 -pady 2

  set ::FastMarchingSegmentation($this,timescrollRange) [vtkKWRange New]
  set tsRange $::FastMarchingSegmentation($this,timescrollRange)
  $tsRange SetParent [$outputParametersFrame GetFrame]
  $tsRange Create
  $tsRange SetLabelText "Segmented volume selection range:"
  $tsRange SymmetricalInteractionOff
  $tsRange SetLabelPositionToTop
  $tsRange SetEntry1PositionToTop
  $tsRange SetEntry2PositionToTop
  $tsRange SetSliderSize 4
  $tsRange SetBalloonHelpString "Define the sub-range of segmented volume (mL)"
  pack [$tsRange GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x
  # turn off volume rendering

  set ::FastMarchingSegmentation($this,timeScrollScale) [vtkKWScaleWithEntry New]
  set timescroll $::FastMarchingSegmentation($this,timeScrollScale)
  $timescroll SetParent [$outputParametersFrame GetFrame]
  $timescroll Create
  $timescroll SetRange 0.0 1.0
  $timescroll SetValue 1.0
  $timescroll SetResolution 0.001
  $timescroll SetLength 150
  $timescroll SetBalloonHelpString "Scroll back in segmentation process"
  $timescroll SetLabelText "Output segmentation volume (mL): "
  $timescroll SetLabelPositionToTop
  $timescroll SetEntryPositionToTop
  $timescroll RangeVisibilityOn
  pack [$timescroll GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  set ::FastMarchingSegmentation($this,acceptButton) [vtkKWPushButton New]
  set accept $::FastMarchingSegmentation($this,acceptButton)
  $accept SetParent [$outputParametersFrame GetFrame]
  $accept Create
  $accept SetText "Accept Segmentation Result"
  $accept SetBalloonHelpString "Once accepted, segmentation cannot be adjusted with the slider. It is important to accept when you are happy with the result to free Slicer memory!!!"
  pack [$accept GetWidgetName] -side top -anchor e -padx 2 -pady 2 -fill x

  # Fast marching filters
  set ::FastMarchingSegmentation($this,cast) [vtkImageCast New]
  set ::FastMarchingSegmentation($this,rescale) [vtkImageShiftScale New]
  set ::FastMarchingSegmentation($this,fastMarchingFilter) [vtkPichonFastMarching New]

  # disable results adjustment frame to reduce confusion
  FastMarchingSegmentationDisableAdjustFrameGUI $this

  set ::FastMarchingSegmentation($this,userWarnedFlag) 0
  set ::FastMarchingSegmentation($this,newFiducialListFlag) 0
}

proc FastMarchingSegmentationAddGUIObservers {this} {
  $this AddObserverByNumber $::FastMarchingSegmentation($this,runButton) 10000 
  $this AddObserverByNumber $::FastMarchingSegmentation($this,acceptButton) 10000 
  $this AddObserverByNumber $::FastMarchingSegmentation($this,timeScrollScale) 10000
  $this AddObserverByNumber $::FastMarchingSegmentation($this,timeScrollScale) 10001
  # fiducial list selector: listen to new node to change the glyph, and to node
  # selection to change the active fiducial list
  $this AddObserverByNumber $::FastMarchingSegmentation($this,fiducialsSelector) 11001
  $this AddObserverByNumber $::FastMarchingSegmentation($this,fiducialsSelector) 11000
  $this AddObserverByNumber $::FastMarchingSegmentation($this,timescrollRange) 10001
  $this AddObserverByNumber $::FastMarchingSegmentation($this,volRenderCheckbox) 10000
}

proc FastMarchingSegmentationRemoveGUIObservers {this} {
}

proc FastMarchingSegmentationRemoveLogicObservers {this} {
}

proc FastMarchingSegmentationRemoveMRMLNodeObservers {this} {
}

proc FastMarchingSegmentationProcessLogicEvents {this caller event} {
}

proc FastMarchingSegmentationProcessGUIEvents {this caller event} {
  
  if { $caller == $::FastMarchingSegmentation($this,runButton) } {
  
    set inputFiducials [$::FastMarchingSegmentation($this,fiducialsSelector) GetSelected]
    set outputVolume [$::FastMarchingSegmentation($this,outputSelector) GetSelected]
    set inputVolume [$::FastMarchingSegmentation($this,inputSelector) GetSelected]
    if {$inputFiducials == ""} {
      FastMarchingSegmentationErrorDialog $this "Please specify input fiducial list!"
      return
    }
    if { [$inputFiducials GetNumberOfFiducials] == 0} {
      FastMarchingSegmentationErrorDialog $this "Input fiducial list must be non-empty!"
      return
    } 
    if {$outputVolume == ""} {
      FastMarchingSegmentationErrorDialog $this "Please specify output volume!"
      return
    }
    if {$inputVolume == $outputVolume } {
      FastMarchingSegmentationErrorDialog $this "Input and output volumes cannot be the same!"
      return
    } 
    if { [$::FastMarchingSegmentation($this,segVolumeThumbWheel) GetValue] == 0} {
      FastMarchingSegmentationErrorDialog $this "Target segmentation volume cannot be 0!"
      return
    }

    if { $::FastMarchingSegmentation($this,userWarnedFlag) == 0 } {
      FastMarchingSegmentationErrorDialog $this "WARNING: The segmentation \
      produced by this method cannot approach closer than 3 pixels to the \
      boundary of the image.\n\nPlease make sure there is sufficient pixel margin \
      around the structure you are trying to segment!"
      set ::FastMarchingSegmentation($this,userWarnedFlag) 1
    }

    # try to prevent user from messing up the module
    FastMarchingSegmentationDisableIOFrameGUI $this

    FastMarchingSegmentationPrepareInput $this
    puts "Input prepared"

    FastMarchingSegmentationCreateLabelVolume $this
    puts "Output prepared"

    FastMarchingSegmentationInitializeFilter $this
    puts "FM filter initialized"

    FastMarchingSegmentationExpand $this
    puts "FM expansion completed"

    
    
    set timescroll $::FastMarchingSegmentation($this,timeScrollScale)
    set tsRange $::FastMarchingSegmentation($this,timescrollRange)
    set segmentedVolume [$::FastMarchingSegmentation($this,segVolumeThumbWheel) GetValue]
    set knownpoints [$::FastMarchingSegmentation($this,fastMarchingFilter) nKnownPoints]
    if { $knownpoints == 0 } {
      FastMarchingSegmentationErrorDialog "Unknown error occured. Please save \
      the scene and submit a bug report with the data to reproduce this error \
      to the developer."
      return
    }
    $tsRange SetWholeRange 0.0 $segmentedVolume
    $tsRange SetRange [expr 0.05*$segmentedVolume] [expr 0.95*$segmentedVolume]
    set range [$tsRange GetRange]
    $timescroll SetRange [lindex $range 0] [lindex $range 1]
    $timescroll SetValue [expr  [expr [lindex $range 0]+[lindex $range 1]]/2.]
    $tsRange SetResolution [ expr [expr double($segmentedVolume)] / [expr double($knownpoints)] ]
    $timescroll SetResolution [ expr [expr double($segmentedVolume)] / [expr double($knownpoints)] ]

    FastMarchingSegmentationEnableAdjustFrameGUI $this

    # TODO: popup information box reminding to hit Accept button when done

    # initialize volume rendering
    FastMarchingSegmentationCreateRender $this
    if { [eval $::FastMarchingSegmentation($this,volRenderCheckbox) GetSelectedState] } {
      FastMarchingSegmentationShowRender $this
    }

    # set mouse mode to ViewTransform
    set interactionNode [ [ [$this GetLogic] GetApplicationLogic ] \
      GetInteractionNode]
    $interactionNode SetLastInteractionMode [$interactionNode \
      GetCurrentInteractionMode]
    $interactionNode SetCurrentInteractionMode 7
    $interactionNode Modified
  } 

  if { $caller == $::FastMarchingSegmentation($this,timeScrollScale) } {
    FastMarchingSegmentationUpdateTime $this    
    if {$event == 10001} {
      set viewerWidget [ [$this GetApplicationGUI] GetActiveViewerWidget ]
      $viewerWidget RequestRender
    }
  } 

  if {$caller == $::FastMarchingSegmentation($this,fiducialsSelector) } {
    # make sure that the newly created fiducials list is selected in the MRMLSelectionNode
    set selectionNode [[[$this GetLogic] GetApplicationLogic] GetSelectionNode]
    set fmFiducialListID [ [$::FastMarchingSegmentation($this,fiducialsSelector) GetSelected] GetID ]
    set fmFiducialList [$::slicer3::MRMLScene GetNodeByID $fmFiducialListID]

    if { $event == 11001 } {
      set ::FastMarchingSegmentation($this,newFiducialListFlag) 1
    }

    if { $event == 11000 } {
      $selectionNode SetReferenceActiveFiducialListID $fmFiducialListID
      if { $::FastMarchingSegmentation($this,newFiducialListFlag) == 1 } {
        set ::FastMarchingSegmentation($this,newFiducialListFlag) 0
        puts "Setting props for $fmFiducialList"
        puts "New node event"
        $fmFiducialList SetGlyphTypeFromString "Sphere3D"
        $fmFiducialList SetSymbolScale 2
        $fmFiducialList SetTextScale 3
      }
    }

  }

  if {$caller == $::FastMarchingSegmentation($this,timescrollRange) } {
    set tsRange $::FastMarchingSegmentation($this,timescrollRange)
    set timescroll $::FastMarchingSegmentation($this,timeScrollScale)
    set range [$tsRange GetRange]
    $timescroll SetRange [lindex $range 0] [lindex $range 1]
  }
  
  if { $caller == $::FastMarchingSegmentation($this,volRenderCheckbox) } {
    if { [info exists ::FastMarchingSegmentation($this,renderVolume)] } {
      if { [eval $caller GetSelectedState] } {
        FastMarchingSegmentationShowRender $this
      } else {
        FastMarchingSegmentationHideRender $this
      }
    } 
  }

  if { $caller == $::FastMarchingSegmentation($this,acceptButton) } {
    if { [eval $::FastMarchingSegmentation($this,volRenderCheckbox) GetSelectedState] == 1 } {
     FastMarchingSegmentationHideRender $this
    }
    FastMarchingSegmentationDestroyRender $this
    FastMarchingSegmentationFinalize $this
    FastMarchingSegmentationDisableAdjustFrameGUI $this
    FastMarchingSegmentationEnableIOFrameGUI $this
  }
}

#
# Accessors to FastMarchingSegmentation state
#


# get the FastMarchingSegmentation parameter node, or create one if it doesn't exist
proc FastMarchingSegmentationCreateParameterNode {} {
  set node [vtkMRMLScriptedModuleNode New]
  $node SetModuleName "FastMarchingSegmentation"

  # set node defaults
  $node SetParameter label 1

  $::slicer3::MRMLScene AddNode $node
  $node Delete
}

# get the FastMarchingSegmentation parameter node, or create one if it doesn't exist
proc FastMarchingSegmentationGetParameterNode {} {

  set node ""
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScriptedModuleNode"]
  for {set i 0} {$i < $nNodes} {incr i} {
    set n [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLScriptedModuleNode"]
    if { [$n GetModuleName] == "FastMarchingSegmentation" } {
      set node $n
      break;
    }
  }

  if { $node == "" } {
    FastMarchingSegmentationCreateParameterNode
    set node [FastMarchingSegmentationGetParameterNode]
  }

  return $node
}


proc FastMarchingSegmentationGetLabel {} {
  set node [FastMarchingSegmentationGetParameterNode]
  if { [$node GetParameter "label"] == "" } {
    $node SetParameter "label" 1
  }
  return [$node GetParameter "label"]
}

proc FastMarchingSegmentationSetLabel {index} {
  set node [FastMarchingSegmentationGetParameterNode]
  $node SetParameter "label" $index
}

#
# MRML Event processing
#

proc FastMarchingSegmentationUpdateMRML {this} {
}

proc FastMarchingSegmentationProcessMRMLEvents {this callerID event} {

    set caller [[[$this GetLogic] GetMRMLScene] GetNodeByID $callerID]
    if { $caller == "" } {
        return
    }
}

proc FastMarchingSegmentationEnter {this} {
  if { [eval $::FastMarchingSegmentation($this,volRenderCheckbox) GetSelectedState] } {
      FastMarchingSegmentationShowRender $this
  }
}

proc FastMarchingSegmentationExit {this} {
  if { [eval $::FastMarchingSegmentation($this,volRenderCheckbox) GetSelectedState] } {
      FastMarchingSegmentationHideRender $this
  }
}

proc FastMarchingSegmentationProgressEventCallback {filter} {

  set mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  set progressGauge [$mainWindow GetProgressGauge]
  set renderWidget [[$::slicer3::ApplicationGUI GetViewControlGUI] GetNavigationWidget]

  if { $filter == "" } {
    $mainWindow SetStatusText ""
    $progressGauge SetValue 0
    $renderWidget SetRendererGradientBackground 0
  } else {
    # TODO: this causes a tcl 'update' which re-triggers the module (possibly changing
    # values while it is executing!  Talk about evil...
    $mainWindow SetStatusText [$filter GetClassName]
    $progressGauge SetValue [expr 100 * [$filter GetProgress]]
  }
  update
}

proc FastMarchingSegmentationErrorDialog {this errorText} {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetMasterWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

proc FastMarchingSegmentationDisableIOFrameGUI {this} {
  $::FastMarchingSegmentation($this,initFrame) CollapseFrame
  $::FastMarchingSegmentation($this,initFrame) EnabledOff

  set widgets {
    outputSelector
    runButton inputSelector 
    fiducialsSelector 
    labelColorSpin
    segVolumeThumbWheel
  }
  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) EnabledOff
  }
}

proc FastMarchingSegmentationEnableIOFrameGUI {this} {
  $::FastMarchingSegmentation($this,initFrame) ExpandFrame
  $::FastMarchingSegmentation($this,initFrame) EnabledOn

  set widgets {
    outputSelector
    runButton inputSelector 
    fiducialsSelector 
    labelColorSpin
    segVolumeThumbWheel
  }
  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) EnabledOn
  }
}

proc FastMarchingSegmentationDisableAdjustFrameGUI {this} {
  $::FastMarchingSegmentation($this,outputParametersFrame) CollapseFrame
  $::FastMarchingSegmentation($this,outputParametersFrame) EnabledOff

  set widgets {
    timeScrollScale
    timescrollRange
    volRenderCheckbox
    acceptButton
  }
  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) EnabledOff
  }
}

proc FastMarchingSegmentationEnableAdjustFrameGUI {this} {
  $::FastMarchingSegmentation($this,outputParametersFrame) ExpandFrame
  $::FastMarchingSegmentation($this,outputParametersFrame) EnabledOn

  set widgets {
    timeScrollScale
    timescrollRange
    volRenderCheckbox
    acceptButton
  }
  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) EnabledOn
  }
}

proc FastMarchingSegmentationCreateRender {this} {

  set ::FastMarchingSegmentation($this,renderMapper) [vtkFixedPointVolumeRayCastMapper New]
  set ::FastMarchingSegmentation($this,renderFilter) [vtkPiecewiseFunction New]
  set ::FastMarchingSegmentation($this,renderColorMapping) [vtkColorTransferFunction New]
  set ::FastMarchingSegmentation($this,renderVolumeProperty) [vtkVolumeProperty New]
  set ::FastMarchingSegmentation($this,renderVolume) [vtkVolume New]
  set ::FastMarchingSegmentation($this,renderMatrix) [vtkMatrix4x4 New]

  set renderFilter $::FastMarchingSegmentation($this,renderFilter)
  set renderMapper $::FastMarchingSegmentation($this,renderMapper)
  set renderColorMapper $::FastMarchingSegmentation($this,renderColorMapping)
  set renderVolumeProperty $::FastMarchingSegmentation($this,renderVolumeProperty)
  set renderVolume $::FastMarchingSegmentation($this,renderVolume)
  set renderMatrix $::FastMarchingSegmentation($this,renderMatrix)

  $renderVolumeProperty SetShade 1
  $renderVolumeProperty SetAmbient 0.3
  $renderVolumeProperty SetDiffuse 0.6
  $renderVolumeProperty SetSpecular 0.5
  $renderVolumeProperty SetSpecularPower 40.
  $renderVolumeProperty SetScalarOpacity $renderFilter
  $renderVolumeProperty SetColor $renderColorMapper
  $renderVolumeProperty SetInterpolationTypeToNearest
  $renderVolumeProperty ShadeOn

  $renderVolume SetProperty $renderVolumeProperty
  $renderVolume SetMapper $renderMapper

# set up volume rendering
  $renderMapper SetInput [$::FastMarchingSegmentation($this,labelVolume) GetImageData]
  $::FastMarchingSegmentation($this,labelVolume) GetIJKToRASMatrix $renderMatrix
  $renderVolume PokeMatrix $renderMatrix

# set up opacity filter
  set labelValue [expr double([[$::FastMarchingSegmentation($this,labelColorSpin) \
    GetWidget] GetValue])]
  $renderFilter RemoveAllPoints
  $renderFilter AddPoint 0. 0.
  $renderFilter AddPoint 256. 0.
  $renderFilter AddPoint $labelValue 1.
  $renderFilter AddPoint [expr $labelValue-.1] 0.
  $renderFilter AddPoint [expr $labelValue+.1] 0.
  $renderFilter ClampingOff

  # get the color that corresponds to the label from the lookup table
  set displayNode [$::FastMarchingSegmentation($this,labelVolume) GetDisplayNode]
  set colorNode [$displayNode GetColorNode]
  set colorLUT [$colorNode GetLookupTable]

  set color [$colorLUT GetColor $labelValue]

  $renderColorMapper RemoveAllPoints
  $renderColorMapper AddRGBPoint $labelValue [lindex $color 0] [lindex $color 1] [lindex $color 2]

}

proc FastMarchingSegmentationDestroyRender {this} {
  $::FastMarchingSegmentation($this,renderVolume) Delete
  $::FastMarchingSegmentation($this,renderMapper) Delete
  $::FastMarchingSegmentation($this,renderFilter) Delete
  $::FastMarchingSegmentation($this,renderColorMapping) Delete
  $::FastMarchingSegmentation($this,renderVolumeProperty) Delete
  $::FastMarchingSegmentation($this,renderMatrix) Delete

  set ::FastMarchingSegmentation($this,renderVolume) ""
}

proc FastMarchingSegmentationShowRender {this} {
  set renderVolume $::FastMarchingSegmentation($this,renderVolume)
  set viewerWidget [ [$this GetApplicationGUI] GetActiveViewerWidget ]
  
  if {$renderVolume == ""} {
    return
  }

  [$viewerWidget GetMainViewer ] AddViewProp $renderVolume
  $viewerWidget RequestRender
}

proc FastMarchingSegmentationHideRender {this} {
  set renderVolume $::FastMarchingSegmentation($this,renderVolume)
  set viewerWidget [ [$this GetApplicationGUI] GetActiveViewerWidget ]
  
  if {$renderVolume == ""} {
    return
  }

  [$viewerWidget GetMainViewer ] RemoveViewProp $renderVolume
  $viewerWidget RequestRender
}

proc FastMarchingSegmentationShowOutputLabel {this} {
  set appGUI [$::slicer3::Application GetApplicationGUI]
  set redGUI [$appGUI GetMainSliceGUI "Red"]
  set yellowGUI [$appGUI GetMainSliceGUI "Yellow"]
  set greenGUI [$appGUI GetMainSliceGUI "Green"]

#  [[$redGUI GetLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID
#    [$::FastMarchingSegmentation($this,inputVolume) GetID]
#  [[$yellowGUI GetLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID
#    [$::FastMarchingSegmentation($this,inputVolume) GetID]
#  [[$greenGUI GetLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID
#    [$::FastMarchingSegmentation($this,inputVolume) GetID]
    
  set outputLabelID [$::FastMarchingSegmentation($this,labelVolume) GetID]

  [[$redGUI GetLogic] GetSliceCompositeNode] SetReferenceLabelVolumeID $outputLabelID
  [[$yellowGUI GetLogic] GetSliceCompositeNode] SetReferenceLabelVolumeID $outputLabelID
  [[$greenGUI GetLogic] GetSliceCompositeNode] SetReferenceLabelVolumeID $outputLabelID

  $::slicer3::MRMLScene Modified
}
