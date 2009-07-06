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
  }

  foreach w $widgets {
    $::FastMarchingSegmentation($this,$w) SetParent ""
    $::FastMarchingSegmentation($this,$w) Delete
  }

  $::FastMarchingSegmentation($this,cast) Delete
  $::FastMarchingSegmentation($this,rescale) Delete
  $::FastMarchingSegmentation($this,fastMarchingFilter) Delete

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
  set helptext "THIS MODULE IS UNDER DEVELOPMENT!\n\n\nThis module performs segmentation using fast marching method with automatic estimation of region statistics. The core C++ classes were contributed by Eric Pichon in slicer2.\n\nIn order to benefit from this module, please use the following steps:\n(1) specify input scalar volume to be segmented\n(2) define fiducial seeds within the region you want to segment\n(3) specify the expected volume of the structure to be segmented. Note, overestimation of this volume is OK, because you will be able to adjust the actual volume once the segmentation is complete\n(4) specify the label color for the segmentation\n(5) Run sementation\n(6) use volume control slider to adjust segmentation result.\nDocumentation (in progress): <a>http://wiki.na-mic.org/Wiki/index.php/Slicer3:FastMarchingSegmentation</a>"
  set abouttext "This module was designed and implemented by Andriy Fedorov and Ron Kikinis based on the original implementation of Eric Pichon in Slicer2.\nThis work was funded by Brain Science Foundation, and is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details."
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
  $select SetBalloonHelpString "The Source Volume to operate on"
  pack [$select GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::FastMarchingSegmentation($this,fiducialsSelector) [vtkSlicerNodeSelectorWidget New]
  set fiducials $::FastMarchingSegmentation($this,fiducialsSelector)
  $fiducials SetParent [$initFrame GetFrame]
  $fiducials Create
  $fiducials NewNodeEnabledOn
  $fiducials SetNodeClass "vtkMRMLFiducialListNode" "" "" "FastMarchingSeeds"
  $fiducials SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $fiducials UpdateMenu
  $fiducials SetLabelText "Input seeds:"
  $fiducials SetBalloonHelpString "List of fiducials to be used as seeds for segmentation"
  pack [$fiducials GetWidgetName] -side top -anchor e -padx 2 -pady 2

  set ::FastMarchingSegmentation($this,outputSelector) [vtkSlicerNodeSelectorWidget New]
  set outselect $::FastMarchingSegmentation($this,outputSelector)
  $outselect SetParent [$initFrame GetFrame]
  $outselect Create
  $outselect NewNodeEnabledOn
  $outselect SetNodeClass "vtkMRMLScalarVolumeNode" "LabelMap" "1" "FastMarchingLabel"
  $outselect SetMRMLScene [[$this GetLogic] GetMRMLScene]
  $outselect UpdateMenu
  $outselect SetLabelText "Output volume:"
  $outselect SetBalloonHelpString "The Output Volume to keep segmentation result"
  pack [$outselect GetWidgetName] -side top -anchor e -padx 2 -pady 2 
 
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
  $segvolume SetBalloonHelpString "Overestimate of the segmented structure volume"
  pack [$segvolume GetWidgetName] -side top -anchor e -padx 2 -pady 2 

  set ::FastMarchingSegmentation($this,labelColorSpin) [vtkKWScaleWithEntry New]
  set outColor $::FastMarchingSegmentation($this,labelColorSpin)
  $outColor SetParent [$initFrame GetFrame]
  $outColor PopupModeOn
  $outColor Create
  $outColor SetLabelText "Output label value:"
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
  $ckbutton SelectedStateOn
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


  # initialize volume rendering
  set ::FastMarchingSegmentation($this,renderMapper) [vtkFixedPointVolumeRayCastMapper New]
  set ::FastMarchingSegmentation($this,renderFilter) [vtkPiecewiseFunction New]
  set ::FastMarchingSegmentation($this,renderColorMapping) [vtkColorTransferFunction New]
  set ::FastMarchingSegmentation($this,renderVolumeProperty) [vtkVolumeProperty New]
  set ::FastMarchingSegmentation($this,renderVolume) [vtkVolume New]

  set renderFilter $::FastMarchingSegmentation($this,renderFilter)
  set renderMapper $::FastMarchingSegmentation($this,renderMapper)
  set renderColorMapping $::FastMarchingSegmentation($this,renderColorMapping)
  set renderVolumeProperty $::FastMarchingSegmentation($this,renderVolumeProperty)
  set renderVolume $::FastMarchingSegmentation($this,renderVolume)
  puts "Before initialization"
  # Fast marching filters
  set ::FastMarchingSegmentation($this,cast) [vtkImageCast New]
  set ::FastMarchingSegmentation($this,rescale) [vtkImageShiftScale New]
  set ::FastMarchingSegmentation($this,fastMarchingFilter) [vtkPichonFastMarching New]
}

proc FastMarchingSegmentationAddGUIObservers {this} {
  $this AddObserverByNumber $::FastMarchingSegmentation($this,runButton) 10000 
  $this AddObserverByNumber $::FastMarchingSegmentation($this,timeScrollScale) 10000
  $this AddObserverByNumber $::FastMarchingSegmentation($this,timeScrollScale) 10001
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

    FastMarchingSegmentationInitializeFilter $this

    FastMarchingSegmentationExpand $this
    $::FastMarchingSegmentation($this,timeScrollScale) EnabledOn
    set timescroll $::FastMarchingSegmentation($this,timeScrollScale)
    set tsRange $::FastMarchingSegmentation($this,timescrollRange)
    set segmentedVolume [$::FastMarchingSegmentation($this,segVolumeThumbWheel) GetValue]
    set knownpoints [$::FastMarchingSegmentation($this,fastMarchingFilter) nKnownPoints]
    $tsRange SetWholeRange 0.0 $segmentedVolume
    $tsRange SetRange [expr 0.05*$segmentedVolume] [expr 0.95*$segmentedVolume]
    set range [$tsRange GetRange]
    $timescroll SetRange [lindex $range 0] [lindex $range 1]
    $timescroll SetValue [expr  [expr [lindex $range 0]+[lindex $range 1]]/2.]
    $tsRange SetResolution [ expr [expr double($segmentedVolume)] / [expr double($knownpoints)] ]
    $timescroll SetResolution [ expr [expr double($segmentedVolume)] / [expr double($knownpoints)] ]

    # undisplay old volume, if one exists
    if { [info exists ::FastMarchingSegmentation($this,renderVolume)] } {
      set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
      [$viewerWidget GetMainViewer ] RemoveViewProp $::FastMarchingSegmentation($this,renderVolume)
      puts "Render volume removed"
    }

    # delete old instances
    catch {$::FastMarchingSegmentation($this,renderMapper) Delete}
    catch {$::FastMarchingSegmentation($this,renderVolumeProperty) Delete}
    catch {$::FastMarchingSegmentation($this,renderVolume) Delete}
    catch {$::FastMarchingSegmentation($this,renderFilter) Delete}
    catch {$::FastMarchingSegmentation($this,renderColorMapping) Delete}
   
    # initialize volume rendering
    set ::FastMarchingSegmentation($this,renderMapper) [vtkFixedPointVolumeRayCastMapper New]
    set ::FastMarchingSegmentation($this,renderFilter) [vtkPiecewiseFunction New]
    set ::FastMarchingSegmentation($this,renderColorMapping) [vtkColorTransferFunction New]
    set ::FastMarchingSegmentation($this,renderVolumeProperty) [vtkVolumeProperty New]
    set ::FastMarchingSegmentation($this,renderVolume) [vtkVolume New]

    set renderFilter $::FastMarchingSegmentation($this,renderFilter)
    set renderMapper $::FastMarchingSegmentation($this,renderMapper)
    set renderColorMapper $::FastMarchingSegmentation($this,renderColorMapping)
    set renderVolumeProperty $::FastMarchingSegmentation($this,renderVolumeProperty)
    set renderVolume $::FastMarchingSegmentation($this,renderVolume)
 
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
    set ijk2ras [vtkMatrix4x4 New]
    $inputVolume GetIJKToRASMatrix $ijk2ras
    $renderVolume PokeMatrix $ijk2ras
    $ijk2ras Delete

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

    set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]

    [$viewerWidget GetMainViewer ] AddViewProp $renderVolume
    $viewerWidget RequestRender

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
      set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
      $viewerWidget RequestRender
      FastMarchingSegmentationDeepCopyResult $this
    }
  } 

  if {$caller == $::FastMarchingSegmentation($this,fiducialsSelector) } {
    # make sure that the newly created fiducials list is selected in the MRMLSelectionNode
    set selectionNode [[[$this GetLogic] GetApplicationLogic] GetSelectionNode]
    $selectionNode SetReferenceActiveFiducialListID \
      [ [$::FastMarchingSegmentation($this,fiducialsSelector) GetSelected] GetID ]
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
        set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
        [$viewerWidget GetMainViewer ] AddViewProp $::FastMarchingSegmentation($this,renderVolume)
        $viewerWidget RequestRender
      } else {
        set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
        [$viewerWidget GetMainViewer ] RemoveViewProp $::FastMarchingSegmentation($this,renderVolume)
        $viewerWidget RequestRender
      }
    } 

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
  if { [info exists ::FastMarchingSegmentation($this,renderVolume)] } {
    set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
    [$viewerWidget GetMainViewer ] AddViewProp $::FastMarchingSegmentation($this,renderVolume)
    $viewerWidget RequestRender
  }
}

proc FastMarchingSegmentationExit {this} {
  if { [info exists ::FastMarchingSegmentation($this,renderVolume)] } {
    set viewerWidget [ [$this GetApplicationGUI] GetViewerWidget ]
    [$viewerWidget GetMainViewer ] RemoveViewProp $::FastMarchingSegmentation($this,renderVolume)
    $viewerWidget RequestRender
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

proc FastMarchingSegmentationApply {this} {

  if { ![info exists ::FastMarchingSegmentation($this,processing)] } { 
    set ::FastMarchingSegmentation($this,processing) 0
  }

  if { $::FastMarchingSegmentation($this,processing) } {
    return
  }

  set volumeNode [$::FastMarchingSegmentation($this,volumesSelect) GetSelected]
  set ijkToRAS [vtkMatrix4x4 New]
  $volumeNode GetIJKToRASMatrix $ijkToRAS
  set outVolumeNode [$::FastMarchingSegmentation($this,volumeOutputSelect) GetSelected]
  set outModelNode [$::FastMarchingSegmentation($this,modelOutputSelect) GetSelected]
  set value [$::FastMarchingSegmentation($this,thresholdSelect) GetValue]
  set minFactor [$::FastMarchingSegmentation($this,minFactor) GetValue]
  set cast $::FastMarchingSegmentation($this,cast)
  set dt $::FastMarchingSegmentation($this,distanceTransform)
  set resample $::FastMarchingSegmentation($this,resample)
  set cubes $::FastMarchingSegmentation($this,marchingCubes)
  set changeIn $::FastMarchingSegmentation($this,changeIn)
  set changeOut $::FastMarchingSegmentation($this,changeOut)
  set polyTransformFilter $::FastMarchingSegmentation($this,polyTransformFilter)
  set polyTransform $::FastMarchingSegmentation($this,polyTransform)

  #
  # check that inputs are valid
  #
  set errorText ""
  if { $volumeNode == "" || [$volumeNode GetImageData] == "" } {
    set errorText "No input volume data..."
  }
  if { $outVolumeNode == "" } {
    set errorText "No output volume node..."
  }
  if { $outModelNode == "" } {
    set errorText "No output model node..."
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

  set ::FastMarchingSegmentation($this,processing) 1

  #
  # configure the pipeline
  #
  $changeIn SetInput [$volumeNode GetImageData]
  eval $changeIn SetOutputSpacing [$volumeNode GetSpacing]
  $cast SetInput [$changeIn GetOutput]
  $cast SetOutputScalarTypeToFloat
  $dt SetInput [$cast GetOutput]
  $dt SetSquaredDistance 0
  $dt SetUseImageSpacing 1
  $dt SetInsideIsPositive 0
  $changeOut SetInput [$dt GetOutput]
  $changeOut SetOutputSpacing 1 1 1
  $resample SetInput [$dt GetOutput]
  $resample SetAxisMagnificationFactor 0 $minFactor
  $resample SetAxisMagnificationFactor 1 $minFactor
  $resample SetAxisMagnificationFactor 2 $minFactor
  $cubes SetInput [$resample GetOutput]
  $cubes SetValue 0 $value
  $polyTransformFilter SetInput [$cubes GetOutput]
  $polyTransformFilter SetTransform $polyTransform
  set magFactor [expr 1.0 / $minFactor]
  $polyTransform Identity
  $polyTransform Concatenate $ijkToRAS
  foreach sp [$volumeNode GetSpacing] {
    lappend invSpacing [expr 1. / $sp]
  }
  eval $polyTransform Scale $invSpacing

  #
  # set up progress observers
  #
  set observerRecords ""
  set filters "$changeIn $resample $dt $changeOut $cubes"
  foreach filter $filters {
    set tag [$filter AddObserver ProgressEvent "FastMarchingSegmentationProgressEventCallback $filter"]
    lappend observerRecords "$filter $tag"
  }

  #
  # activate the pipeline
  #
  $polyTransformFilter Update

  # remove progress observers
  foreach record $observerRecords {
    foreach {filter tag} $record {
      $filter RemoveObserver $tag
    }
  }
  FastMarchingSegmentationProgressEventCallback ""

  #
  # create a mrml model display node if needed
  #
  if { [$outModelNode GetDisplayNode] == "" } {
    set modelDisplayNode [vtkMRMLModelDisplayNode New]
    $outModelNode SetScene $::slicer3::MRMLScene
    eval $modelDisplayNode SetColor .5 1 1
    $::slicer3::MRMLScene AddNode $modelDisplayNode
    $outModelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]
  }

  #
  # set the output into the MRML scene
  #
  $outModelNode SetAndObservePolyData [$polyTransformFilter GetOutput]

  $outVolumeNode SetAndObserveImageData [$changeOut GetOutput]
  $outVolumeNode SetIJKToRASMatrix $ijkToRAS
  $ijkToRAS Delete


  set ::FastMarchingSegmentation($this,processing) 0
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

