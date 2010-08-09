
package require Itcl

#########################################################
#
if {0} { ;# comment

  RemoveIslandsEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             RemoveIslandsEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class RemoveIslandsEffect] == "" } {

  itcl::class RemoveIslandsEffect {

    inherit EffectSWidget

    constructor {sliceGUI} {EffectSWidget::constructor $sliceGUI} {}
    destructor {}

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method apply {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method goToModelMaker {} {}
    method findNonZeroBorderPixel {imageData} {}

  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body RemoveIslandsEffect::constructor {sliceGUI} {
  set _scopeOptions "all visible"
}

itcl::body RemoveIslandsEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body RemoveIslandsEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

  if { $caller == $sliceGUI } {
    switch $event {
      "LeftButtonPressEvent" {
        $this apply
        $sliceGUI SetGUICommandAbortFlag 1
      }
      "EnterEvent" {
        $o(cursorActor) VisibilityOn
      }
      "LeaveEvent" {
        $o(cursorActor) VisibilityOff
      }
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

# search the border of the image data looking for the first 
# - usually whole border will be nonzero, but in some cases
#   it may not be.  So check corners first, and then
#   if can't find it, give up and use 1 (to avoid exhaustive search)
itcl::body RemoveIslandsEffect::findNonZeroBorderPixel { imageData } {

  foreach {w h d} [$imageData GetDimensions] {}
  foreach v {w h d} { set $v [expr [set $v] - 1] }

  set corners [list "0 0 0" "$w 0 0" "0 $h 0" "$w $h 0" \
                    "0 0 $d" "$w 0 $d" "0 $h $d" "$w $h $d"]
  foreach corner $corners {
    foreach {i j k} $corner {}
    set p [$imageData GetScalarComponentAsDouble $i $j $k 0]
    if { $p != 0 } {
      return $p
    }
  }
  return 1
}

itcl::body RemoveIslandsEffect::apply {} {

  if { [$this getInputLabel] == "" } {
    $this errorDialog "Label map needed for Island operations"
    return
  }

  set label [EditorGetPaintLabel]

  # first, create an inverse binary version of the image
  # so that islands inside segemented object will be detected, along
  # with a big island of the background
  set preThresh [vtkImageThreshold New]
  $preThresh SetInValue 0
  $preThresh SetOutValue 1
  $preThresh ReplaceOutOn
  $preThresh ThresholdBetween $label $label
  $preThresh SetInput [$this getInputLabel]
  $preThresh SetOutputScalarTypeToUnsignedLong

  # now identify the islands in the inverted volume
  # and find the pixel that corresponds to the background
  set islands [vtkITKIslandMath New]
  $islands SetInput [$preThresh GetOutput]
  $islands SetFullyConnected [$o(fullyConnected) GetSelectedState]
  $this setProgressFilter $islands "Calculating Islands..."
  $islands Update
  set bgLabel [$this findNonZeroBorderPixel [$islands GetOutput]]

  # now rethreshold so that everything which is not background becomes the label
  set postThresh [vtkImageThreshold New]
  $postThresh SetInValue 0
  $postThresh SetOutValue $label
  $postThresh ReplaceOutOn
  $postThresh ThresholdBetween $bgLabel $bgLabel
  $postThresh SetOutputScalarTypeToShort
  $postThresh SetInput [$islands GetOutput]
  $postThresh SetOutput [$this getOutputLabel]
  $this setProgressFilter $postThresh "Applying to Label Map..."
  $postThresh Update
  $this postApply

  $preThresh Delete
  $islands Delete
  $postThresh Delete

}

itcl::body RemoveIslandsEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # 4 or 8 neighbors
  #

  set o(fullyConnected) [vtkKWCheckButton New]
  $o(fullyConnected) SetParent [$this getOptionsFrame]
  $o(fullyConnected) Create
  $o(fullyConnected) SetText "Fully Connected"
  $o(fullyConnected) SetBalloonHelpString "When on, do not treat diagonally adjacent voxels as neighbors."
  $o(fullyConnected) SetSelectedState 0
  pack [$o(fullyConnected) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 -expand true

  #
  # an apply button
  #
  set o(apply) [vtkNew vtkKWPushButton]
  $o(apply) SetParent [$this getOptionsFrame]
  $o(apply) Create
  $o(apply) SetText "Apply"
  $o(apply) SetBalloonHelpString "Run the selected operation."
  pack [$o(apply) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 


  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel this effect."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # a help button
  #
  set o(help) [vtkNew vtkSlicerPopUpHelpWidget]
  $o(help) SetParent [$this getOptionsFrame]
  $o(help) Create
  $o(help) SetHelpTitle "RemoveIslands"
  $o(help) SetHelpText "Use this tool to remove isolated islands of background (0) within a segmented region (current label color).  Note that only completely isolated islands of background are removed.  You may need to manually cut the connections between interior regions and the background using one of the paint or draw tools."
  $o(help) SetBalloonHelpString "Bring up help window."
  pack [$o(help) GetWidgetName] \
    -side right -anchor sw -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(apply) AddObserver AnyEvent "$this apply"]
  lappend _observerRecords "$o(apply) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getOutputLabel] == "" } {
    $this errorDialog "Label map needed for RemoveIslandsing"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body RemoveIslandsEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "minSize fullyConnected help cancel apply" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}

