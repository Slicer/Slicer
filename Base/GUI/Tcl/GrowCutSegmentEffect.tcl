
package require Itcl

#########################################################
#
if {0} { ;# comment

   GrowCutSegmentEffect an editor effect
# TODO : 

}

#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             GrowCutSegmentEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class GrowCutSegmentEffect] == "" } {

    itcl::class GrowCutSegmentEffect {

  inherit PaintEffect 

  constructor {sliceGUI} {PaintEffect::constructor $sliceGUI} {}
  destructor {}
  
  public variable objectSize 5
  public variable contrastNoiseRatio 0.8
  public variable priorStrength 0.003
  public variable segmented 2

  variable _segmentedImage ""
  variable _gestureImage ""
  variable _foregroundID ""
  variable _labelID ""
  variable _foundPainted 0
  variable _maxObjectSize 0
  
  variable _changedImage 0
  variable _iterations 0

  variable _imageDimension ""
  variable _imageSpacing ""
  variable _activeColoring 0

  method processEvent {{caller ""} {event ""} } {}
  method buildOptions {} {}
  method tearDownOptions {} {}
  method setMRMLDefaults {} {}
  method updateMRMLFromGUI {} {}
  method updateGUIFromMRML {} {}
  method apply {} {} 
  method initialize {} {}
  method updateRadius1 {} {}
  method updateRadius2 {} {}
  method updateRadius3 {} {}
  method updateRadius4 {} {}
  method updateRadius5 {} {}
}
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body GrowCutSegmentEffect::constructor {sliceGUI} {
    
   $this configure -sliceGUI $sliceGUI

   puts "GrowCutSegmentEffect"

   set _segmentedImage [vtkNew vtkImageData]
   set _gestureImage [vtkNew vtkImageData]

   set _imageDimension [[$this getInputLabel] GetDimensions]
   
   set _imageSpacing [[$this getInputLabel] GetSpacing]

   set dim [[$this getInputLabel] GetDimensions]
   set x [lindex $dim 0]
   set y [lindex $dim 1]
   set z [lindex $dim 2]
   
   set _maxObjectSize [expr $x * [expr $y * $z]]
   puts "max object size $_maxObjectSize $x $y $z"

   $this initialize
  
}



itcl::body GrowCutSegmentEffect::destructor {} {

  if { [info command $sliceGUI] != "" } {
    foreach tag $_guiObserverTags {
      $sliceGUI RemoveObserver $tag
    }
  }

  if { [info command $_sliceNode] != "" } {
    foreach tag $_nodeObserverTags {
      $_sliceNode RemoveObserver $tag
    }
  }

  if { [info command $_renderer] != "" } {
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
  }
}


# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body GrowCutSegmentEffect::initialize {} {
    
    set grabID [$sliceGUI GetGrabID]
    if { ($grabID != "") && ($grabID != $this) } {
      return
    }

    set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLSliceCompositeNode"]
    set j 0
    set cnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLSliceCompositeNode"]
    set _foregroundID [$cnode GetForegroundVolumeID]
    set _labelID [$cnode GetLabelVolumeID]

    set dim [[$this getInputLabel] GetDimensions]
    set x [lindex $dim 0]
    set y [lindex $dim 1]
    set z [lindex $dim 2]

    set extent [[$this getInputLabel] GetExtent]
    set origin [[$this getInputLabel] GetOrigin]
    set spacing [[$this getInputLabel] GetSpacing]

    $_segmentedImage SetDimensions $x $y $z
    $_segmentedImage SetExtent [lindex $extent 0] [lindex $extent 1] \
  [lindex $extent 2] [lindex $extent 3] \
  [lindex $extent 4] [lindex $extent 5]

    $_segmentedImage SetOrigin [lindex $origin 0] [lindex $origin 1] [lindex $origin 2]
    $_segmentedImage SetSpacing [lindex $spacing 0] [lindex $spacing 1] [lindex $spacing 2]
    $_segmentedImage SetScalarType [[$this getInputForeground] GetScalarType]
    $_segmentedImage AllocateScalars
    $_segmentedImage DeepCopy [$this getInputForeground]

    $_gestureImage SetDimensions $x $y $z
    $_gestureImage SetExtent [lindex $extent 0] [lindex $extent 1] \
  [lindex $extent 2] [lindex $extent 3] \
  [lindex $extent 4] [lindex $extent 5]

    $_gestureImage SetOrigin [lindex $origin 0] [lindex $origin 1] [lindex $origin 2]
    $_gestureImage SetSpacing [lindex $spacing 0] [lindex $spacing 1] [lindex $spacing 2]
    $_gestureImage SetScalarType [[$this getInputLabel] GetScalarType]
    $_gestureImage AllocateScalars
    $_gestureImage DeepCopy [$this getInputLabel]

  # set range [$_gestureImage GetScalarRange]  
  # if { [llength $range] == 2 } {
  #    set maxRange [lindex $range 1]
  #    puts "range is $range"
  #    if {$maxRange > 0 } {
  #        set _foundPainted 1
  #      }
  #  }
  # if { $_foundPainted == 0 } {
  #   $this setInputImageData $_segmentedImage $_foregroundID
     #$this setInputImageData $_segmentedImage $_labelID

  # } 
   puts "Done Initializing"
}

# ------------------------------------------------------------------
#                             HELPER METHODS
# ------------------------------------------------------------------

itcl::body GrowCutSegmentEffect::processEvent { {caller ""} {event ""} } {

  #puts "In ProcessEvent : GrowCutSegmentEffect "

  # chain to superclass
  chain $caller $event

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return 
  }

  set grabID [$sliceGUI GetGrabID]
  if { ($grabID != "") && ($grabID != $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionActors
    [$sliceGUI GetSliceViewer] RequestRender
    return 
 }


 set event [$sliceGUI GetCurrentGUIEvent] 
  switch $event {
    "LeftButtonPressEvent" {
        # puts "swapping foreground to gestures..."
        # $this swapInputForegroundLabel $_foregroundID $_labelID
  
        set _changedImage 1

       foreach {x y} [$_interactor GetEventPosition] {}              
       if { $smudge } {
         continue
       } else {
        set activeColor [EditorGetPaintLabel]
         }   
       }
    }
}


itcl::body GrowCutSegmentEffect::updateRadius1 {} {
  
  set node [EditorGetParameterNode]
  $node SetParameter "Paint,radius" 1
}



itcl::body GrowCutSegmentEffect::updateRadius2 {} {
  
  set node [EditorGetParameterNode]
  $node SetParameter "Paint,radius" 2
}


itcl::body GrowCutSegmentEffect::updateRadius3 {} {
  
  set node [EditorGetParameterNode]
  $node SetParameter "Paint,radius" 3
}


itcl::body GrowCutSegmentEffect::updateRadius4 {} {
  
  set node [EditorGetParameterNode]
  $node SetParameter "Paint,radius" 4
}


itcl::body GrowCutSegmentEffect::updateRadius5 {} {
  
  set node [EditorGetParameterNode]
  $node SetParameter "Paint,radius" 5
}


itcl::body GrowCutSegmentEffect::apply {} {

    puts "in Apply method"

    set grabID [$sliceGUI GetGrabID]
    if { ($grabID != "") && ($grabID != $this) } {
      return
    }

    
    if { [$this getInputLabel] == "" || [$this getInputBackground] == "" } {
       $this flashCursor 3
       return
    }

    puts "Applying to run the Grow Cut Segmentation Algorithm"

    if { ![info exists o(growCutFilter)] } {
        puts "CREATING GROW CUT FILTER "
       set o(growCutFilter) [vtkNew vtkITKGrowCutSegmentationImageFilter]
    }

   $this setProgressFilter $o(growCutFilter) "GrowCutSegment Filter"
   catch {
   set numCnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScalarVolumeNode"]
   for { set j 0 } { $j < $numCnodes } { incr j } {
    set vnode [$::slicer3::MRMLScene GetNthNodeByClass $j "vtkMRMLScalarVolumeNode"]
    set vID [$vnode GetID]
    if { $vID == $_foregroundID } {
     
      $_segmentedImage DeepCopy [$vnode GetImageData]
    } 
    if { $vID == $_labelID } {
      $_gestureImage DeepCopy [$vnode GetImageData]
    }
   
   }
   } errCopy
  if { $errCopy != "" } { puts "error in getting the gesture and segmented images $errCopy" }
   # Handle the logic for switching gestures and label layer here
   #
  # set range [$_gestureImage GetScalarRange]  
  # if { [llength $range] == 2 } {
  #    set maxRange [lindex $range 1]
  #    puts "range is $range"
  #    if {$maxRange > 0 } {
  #        set _foundPainted 1
  #      }
  #  }
  # set lrange [$_segmentedImage GetScalarRange]
  # puts "segmented image range $lrange"
  #  puts "found painted is $_foundPainted"
  #  catch {
  #  if { $_foundPainted == 0 } {
  #      puts "swapping the label and gestureimage.." 
  #    
  #      $this swapInputForegroundLabel $_foregroundID $_labelID  
  #      $_gestureImage DeepCopy [$this getInputForeground]
  #      $_segmentedImage DeepCopy [$this getInputLabel] 
  #   
  #      #set _foundPainted 1
  #  }
    $o(growCutFilter) SetInput 1 $_gestureImage 
    $o(growCutFilter) SetInput 2 $_segmentedImage 
    
   #} errSwap
  # if {$errSwap != "" } { puts "error in the SWAPPING LOGIC $errSwap" }
    puts "setting inputs for the grow cut filter ... "
    $o(growCutFilter) SetInput 0 [$this getInputBackground] 
    scan [$_gestureImage GetSpacing] "%f %f %f" dx dy dz
    set voxelvolume [expr $dx * $dy * $dz]
    set conversion 1000
    set voxelamt [expr $objectSize / $voxelvolume]
    set voxelnumber [expr round($voxelamt) * $conversion]
 
  # $_segmentedImage DeepCopy [$this getInputLabel]
  # $_gestureImage DeepCopy [$this getInputForeground] 
    
    #set voxelnumber [ConversiontoVoxels $this $objectSize]
    set cubeIndex [expr 1.0/3.0]
    
    set oSize [expr round([expr pow($voxelnumber,$cubeIndex)] )]

    set objectSize 1
    
    $o(growCutFilter) SetObjectSize $oSize
    $o(growCutFilter) SetContrastNoiseRatio $contrastNoiseRatio
    #$o(growCutFilter) SetGestureColors $gestureColors
    $o(growCutFilter) SetPriorSegmentConfidence  $priorStrength 
    $o(growCutFilter) SetNumberOfThreads 1 
    puts "updating grow cut filter "
    $o(growCutFilter) Update
   #if {$_foundPainted == 0 } {
   #     $this setInputImageData $_gestureImage $_foregroundID
   #}
   $_segmentedImage DeepCopy [$o(growCutFilter) GetOutput]

   $this setInputImageData $_segmentedImage $_foregroundID
   #$this swapInputForegroundLabel $_labelID $_foregroundID

   puts "done grow cut..."
  
}


itcl::body GrowCutSegmentEffect::buildOptions {} {
   

#  call superclass version of buildOptions
  chain

#  forget the options set by the labeler

  foreach w "paintThreshold paintRange cancel" {
      if { [info exists o($w)] } {
    $o($w) SetParent ""
    pack forget [$o($w) GetWidgetName]
      }
  }

  puts "bulding options.... "

#  catch {
  
  puts "max Object Size : $_maxObjectSize"

#  catch {
#  set o(objectSize) [vtkKWThumbWheel New]
#  $o(objectSize) SetParent [$this getOptionsFrame]
#  $o(objectSize) PopupModeOn
#  $o(objectSize) Create
#  $o(objectSize) DisplayEntryAndLabelOnTopOn
#  $o(objectSize) DisplayEntryOn
#  $o(objectSize) DisplayLabelOn
#  $o(objectSize) SetResolution 1
#  $o(objectSize) SetMinimumValue 0
#  $o(objectSize) SetClampMinimumValue 0
#  $o(objectSize) SetValue 0
#  [$o(objectSize) GetLabel] SetText "ROI volume to expand from gestures (mL):"
#  $o(objectSize) SetBalloonHelpString "How much should the ROI be expanded from the overlaid gestures"
#  pack [$o(objectSize) GetWidgetName] -side top -anchor e -padx 2 -pady 2

#  } test0
  
#  if { $test0 != "" } {puts "after objectSize : $test0"}
  #puts "object Size built "

#    set the contrast noise ratio for the selected class 
#  catch {
#  set o(contrastNoiseRatio) [vtkKWScaleWithEntry New]
#  $o(contrastNoiseRatio) SetParent [$this getOptionsFrame]
#  $o(contrastNoiseRatio) PopupModeOn
#  $o(contrastNoiseRatio) Create
#  $o(contrastNoiseRatio) SetRange 0.0 1.0
#  $o(contrastNoiseRatio) SetResolution 0.01
#  $o(contrastNoiseRatio) SetLabelText "Contrast Ratio: " 
#  $o(contrastNoiseRatio) SetValue $contrastNoiseRatio    
# $o(contrastNoiseRatio) SetBalloonHelpString "Set the relative contrast between foreground class and its surrounding background \[0 to 1\]."
#  pack [$o(contrastNoiseRatio) GetWidgetName] \
     -side top -anchor e -fill x -padx 2 -pady 2 
#  } test1

#  puts "contrast noise ratio built "

#  if { $test1 != "" } {puts " after maxIter and cNR : $test1" }

  #  
  # set the prior strength if using a segmented label image to start segmentation
  #
# catch {
# set o(priorStrength) [vtkNew vtkKWThumbWheel]
# $o(priorStrength) SetParent [$this getOptionsFrame]
# $o(priorStrength) PopupModeOn
# $o(priorStrength) Create
# $o(priorStrength) DisplayEntryAndLabelOnTopOn
# $o(priorStrength) DisplayEntryOn
# $o(priorStrength) DisplayLabelOn
# $o(priorStrength) SetValue [[EditorGetParameterNode] GetParameter GrowCutSegment,priorStrength] 
# $o(priorStrength) SetMinimumValue 0
# $o(priorStrength) SetMaximumValue 1
# $o(priorStrength) ClampMinimumValueOn
# $o(priorStrength) ClampMaximumValueOn
# [$o(priorStrength) GetLabel] SetText "Prev Segment Confidence: "
# $o(priorStrength) SetBalloonHelpString "Set the confidence in the initial segmentation if using an existing segmentation for correction \[0 to 1\]."
# pack [$o(priorStrength) GetWidgetName] \
     -side top -anchor e -fill x -padx 2 -pady 2 

#  puts "prior Strength "

# } testPrior
# if { $testPrior != ""} { puts " after priorStrength: $testPrior" }
 
  #
  # set display to overlay gestures
  #
#  catch {
#  set o(overlay) [vtkKWCheckButtonWithLabel New]
#  $o(overlay) SetParent [$this getOptionsFrame]
#  $o(overlay) Create
#  $o(overlay) SetLabelText "Overlay Gestures: "
#  $o(overlay) SetBalloonHelpString "Set the Gestures input from previous interactions to be overlayed on the segmented label map. You may add additional gestures to correct segmentation."
#  [$o(overlay) GetWidget] SetSelectedState [[EditorGetParameterNode] GetParameter GrowCutSegment,overlay] 
#  pack [$o(overlay) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 
#  } testOverlay
#  if { $testOverlay != "" } {puts " after overlay : $testOverlay" }
  
  
  #
  # a help button
  #
  set o(help) [vtkNew vtkSlicerPopUpHelpWidget]
  $o(help) SetParent [$this getOptionsFrame]
  $o(help) Create
  $o(help) SetHelpTitle "Grow Cut Segmentation"
  $o(help) SetHelpText "This tool enables to interactively segment a region of interest. For operation, this tool requires the user to specify at least an 'in-region' and an 'out-region' marked using different colors. To specify 'in-region', select a color from the Label and paint a stroke (gesture) inside the region of interest. Repeat the same with a different label color for the 'out-region'. To correct a segmentation, simply paint on some areas that need correction with the appropriate label color. The tool automatically picks a region of interest (ROI) based on the gestures painted by the user. To extend the ROI, paint on the areas that need to be segmented. This tool also supports 'n-class' segmentation. For faster segmentation, optionally use the CropVolume tool to crop the region of interest and use this tool."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

  #
  # an apply button
  #
   set o(apply) [vtkNew vtkKWPushButton]
   $o(apply) SetParent [$this getOptionsFrame]
   $o(apply) Create
   $o(apply) SetText "Apply"
   $o(apply) SetBalloonHelpString "Apply to run segmentation.\n Use the 'a' hotkey to apply segmentation"
   pack [$o(apply) GetWidgetName] \
      -side right -anchor e -padx 2 -pady 2 

  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel current segmentation."
  pack [$o(cancel) GetWidgetName] \
      -side right -anchor e -padx 2 -pady 2 


 # if { [info exists o(overlay)] } {
   #   puts "o(overlay) exists"
 # } else {
    #  puts "o(overlay) doesn't exist"
 # }

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
#  set tag [$o(objectSize) AddObserver AnyEvent "after idle $this updateMRMLFromGUI"]
#  lappend _observerRecords "$o(objectSize) $tag"
  
 # set tag [$o(contrastNoiseRatio) AddObserver AnyEvent "after idle $this updateMRMLFromGUI"]
 # lappend _observerRecords "$o(contrastNoiseRatio) $tag"

 # set tag [$o(priorStrength) AddObserver AnyEvent "after idle $this updateMRMLFromGUI"]
 # lappend _observerRecords "$o(priorStrength) $tag"

#  set tag [[$o(overlay) GetWidget] AddObserver AnyEvent "after idle $this updateMRMLFromGUI"]
#  lappend _observerRecords "$o(overlay) $tag"

  set tag [$o(apply) AddObserver AnyEvent "$this apply"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getInputLabel] == "" } {
    $this errorDialog "Background and Label map needed for Grow Cut Segmentation"
    after idle ::EffectSWidget::RemoveAll
  }
 
  $this updateGUIFromMRML

#    } buildCatch
#    if {$buildCatch != "" } { puts "error in building : $buildCatch" }

  #  puts "done build options"
}


itcl::body GrowCutSegmentEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "help cancel apply" {
      if { [info exists o($w)] } {
    $o($w) SetParent ""
    pack forget [$o($w) GetWidgetName] 
      }
  }
#  if { $swapped == 1 } {
   # $this swapInputForegroundLabel $_foregroundID $_labelID
#  }
}


itcl::body GrowCutSegmentEffect::updateMRMLFromGUI { } {
    
#    puts "updating MRML from GUI"

    chain
    set node [EditorGetParameterNode]

#    if { [info exists o(objectSize)] } {
#       $this configure -objectSize [$o(objectSize) GetValue]
#       $node SetParameter "GrowCutSegment,objectSize" $objectSize   
#    }

#    if { [info exists o(contrastNoiseRatio)] } {
#       $this configure -contrastNoiseRatio [$o(contrastNoiseRatio) GetValue]
#       $node SetParameter "GrowCutSegment,contrastNoiseRatio" $contrastNoiseRatio   
#    }

#    if { [info exists o(priorStrength)] } {
#       $this configure -priorStrength [$o(priorStrength) GetValue]
#       $node SetParameter "GrowCutSegment,priorStrength" $priorStrength   
#    }
    
#    if { [info exists o(overlay)] } {
#       $this configure -overlay [[$o(overlay) GetWidget] GetSelectedState]
#       $node SetParameter "GrowCutSegment,overlay" $overlay
#    }

#    puts "done updating MRML from GUI"
    
}


itcl::body GrowCutSegmentEffect::setMRMLDefaults { } {
    
    puts "setting MRML defaults in Grow Cut"

    chain
    
    set node [EditorGetParameterNode]
#    $node SetParameter "GrowCutSegment,maxIterations" $_maxObjectSize
#    $node SetParameter "GrowCutSegment,contrastNoiseRatio" 0.8
#    $node SetParameter "GrowCutSegment,priorStrength" 0.0003
#    $node SetParameter "GrowCutSegment,overlay" 0
#    $node SetParameter "GrowCutSegment,segmented" 2
    

    foreach {param default} {
      #objectSize 10
      contrastNoiseRatio 0.8
      priorStrength 0.0003
      #overlay 0
      segmented 2
    } {
        set pvalue [$node GetParameter Paint,$param] 
 if { $pvalue == "" } {
     $node SetParameter GrowCutSegment,$param $default
 }
    }

   # $node SetParameter "GrowCutSegment,objectSize" 100 
    $node SetParameter "Paint, radius" 3
   # puts "done setting MRML defaults in Grow Cut"
}



itcl::body GrowCutSegmentEffect::updateGUIFromMRML { } {
  #
  # get the parameter from the node
  # - set default value if it doesn't exist
  #
  chain

#  puts "updating gui from MRML"
    

  set node [EditorGetParameterNode]
  # set the GUI and effect parameters to match node
  # (only if this is the instance that "owns" the GUI
  
#  catch {
#  set objectSize [$node GetParameter GrowCutSegment,objectSize] 
#  $this configure -objectSize $objectSize
#  if { [info exists o(objectSize)] } {
#      $o(objectSize) SetValue $objectSize
#  }

#  set contrastNoiseRatio [$node GetParameter GrowCutSegment,contrastNoiseRatio] 
#  $this configure -contrastNoiseRatio $contrastNoiseRatio
#  if { [info exists o(contrastNoiseRatio)] } {
#      $o(contrastNoiseRatio) SetValue $contrastNoiseRatio
#  }

# set priorStrength [$node GetParameter GrowCutSegment,priorStrength] 
#  $this configure -priorStrength $priorStrength
#  if { [info exists o(priorStrength)] } {
#    $o(priorStrength) SetValue $priorStrength
#  }

 #set overlay [$node GetParameter GrowCutSegment,overlay] 
 #$this configure -overlay $overlay
 #if { [info exists o(overlay)] } {
 #  [$o(overlay) GetWidget] SetSelectedState $overlay
 #}

#  } err1
#  if { $err1 != ""} {puts $err1}

#  set segmented [$node GetParameter GrowCutSegment,segmented] 
#  $this configure -segmented $segmented
#  if { [info exists o(segmented)] } {
#    [$o(segmented) GetWidget] SetSelectedState $segmented
#  }

  $this preview

#  puts "done updating gui from MRML"
}

proc GrowCutSegmentEffect::ConfigureAll { args } {
    foreach pw [itcl::find objects -class GrowCutSegmentEffect] {
     eval $pw configure $args
    }
}
