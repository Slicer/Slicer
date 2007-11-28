
package require Itcl

#########################################################
#
if {0} { ;# comment

  WandEffect an editor effect


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             WandEffect
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class WandEffect] == "" } {

  itcl::class WandEffect {

    inherit Labeler

    constructor {sliceGUI} {Labeler::constructor $sliceGUI} {}
    destructor {}

    public variable percentage "0.1"

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method preview {} {}
    method apply {} {}
    method buildOptions {} {}
    method updateMRMLFromGUI {} {}
    method setMRMLDefaults { } {}
    method updateGUIFromMRML {} {}
    method tearDownOptions {} {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body WandEffect::constructor {sliceGUI} {
}

itcl::body WandEffect::destructor {} {
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body WandEffect::processEvent { {caller ""} {event ""} } {

  if { [$this preProcessEvent $caller $event] } {
    # superclass processed the event, so we don't
    return
  }

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

  switch $event {
    "LeftButtonPressEvent" {
      $this apply
      $sliceGUI SetGUICommandAbortFlag 1
    }
    "MouseMoveEvent" {
      $this preview
    }
    "KeyPressEvent" { 
      set key [$_interactor GetKeySym]
      if { [lsearch "minus equal plus" $key] != -1 } {
        $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
        $sliceGUI SetGUICommandAbortFlag 1
        switch [$_interactor GetKeySym] {
          "minus" {
            set node [EditorGetParameterNode]
            set percentage [$node GetParameter "Wand,percentage"] 
            $node SetParameter "Wand,percentage" [expr $percentage - 0.01]
          }
          "equal" - "plus" {
            set node [EditorGetParameterNode]
            set percentage [$node GetParameter "Wand,percentage"] 
            $node SetParameter "Wand,percentage" [expr $percentage + 0.01]
          }
        }
      } else {
        # puts "wand ignoring $key"
      }
    }
    "EnterEvent" {
      $o(cursorActor) VisibilityOn
      if { [info exists o(wandActor)] } {
       $o(wandActor) VisibilityOn
      }
    }
    "LeaveEvent" {
      $o(cursorActor) VisibilityOff
      if { [info exists o(wandActor)] } {
       $o(wandActor) VisibilityOff
      }
    }
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body WandEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getOutputLabel] == "" } {
    $this flashCursor 3
    return
  }

  $this postApply
}

itcl::body WandEffect::preview {} {


  # first try to undo - removes last apply
  $this undoLastApply

  # 
  # get the event position to use as a seed
  foreach {x y} [$_interactor GetEventPosition] {}
  $this queryLayers $x $y

  #
  # create pipeline as needed
  if { ![info exists o(wandFilter)] } {
    set o(wandFilter) [vtkNew vtkITKWandImageFilter]
  }

  $o(wandFilter) SetInput [$this getInputBackground]
  $o(wandFilter) SetSeed $_layers(background,i) $_layers(background,j) $_layers(background,k) 
  $o(wandFilter) SetDynamicRangePercentage $percentage


  set extents [[$this getInputBackground] GetExtent]
  set ijkToXY [vtkMatrix4x4 New]
  $ijkToXY DeepCopy [[$_layers(background,logic) GetXYToIJKTransform] GetMatrix]
  $ijkToXY Invert


  # figure out which plane to use
  # - transform the slice's extents into xy bounds for use with the Labeler
  #
  foreach {i0 j0 k0 l0} [$_layers(background,xyToIJK) MultiplyPoint $x $y 0 1] {}
  set x1 [expr $x + 1]; set y1 [expr $y + 1]
  foreach {i1 j1 k1 l1} [$_layers(background,xyToIJK) MultiplyPoint $x1 $y1 0 1] {}

  #
  # Note there seems to be a bug (ITK smart pointer crash)
  # for some slice planes.  This Effect is disabled in EditBox for now.
  #
puts "orig extents: $extents"
  if { $i0 == $i1 } { 
    $o(wandFilter) SetPlaneToJK
    set extents [lreplace $extents 0 1 $i0 $i0]
  }
  if { $j0 == $j1 } {
    $o(wandFilter) SetPlaneToIK
    set extents [lreplace $extents 2 3 $j0 $j0]
  }
  if { $k0 == $k1 } { 
    $o(wandFilter) SetPlaneToIJ
    set extents [lreplace $extents 4 5 $k0 $k0]
  }

  foreach {ilo ihi jlo jhi klo khi} $extents {}
puts "extents: $extents"
  set xylo [$ijkToXY MultiplyPoint $ilo $jlo $klo 1]
  set xyhi [$ijkToXY MultiplyPoint $ihi $jhi $khi 1]
  foreach {xlo ylo zlo wlo} $xylo {}
  foreach {xhi yhi zhi whi} $xyhi {}
  if { $xlo > $xhi } { set tmp $xhi; set xhi $xlo; set xlo $tmp }
  if { $ylo > $yhi } { set tmp $yhi; set yhi $ylo; set ylo $tmp }
  set bounds "$xlo $xhi $ylo $yhi 0 0"
puts "bounds: $bounds"
  $ijkToXY Delete


  $this setProgressFilter $o(wandFilter) "Magic Wand Connected Components"
  $o(wandFilter) Update

  puts "$this applyImageMask \
    [$_sliceNode GetXYToRAS]  \
    [ $o(wandFilter) GetOutput] \
    $bounds"
return

  $this applyImageMask \
    [$_sliceNode GetXYToRAS]  \
    [ $o(wandFilter) GetOutput] \
    $bounds

}
  
itcl::body WandEffect::buildOptions {} {

  # call superclass version of buildOptions
  chain

  #
  # a slider to set the percentage of the dynamic range
  #
  set o(percentage) [vtkNew vtkKWScaleWithEntry]
  $o(percentage) SetParent [$this getOptionsFrame]
  $o(percentage) PopupModeOn
  $o(percentage) SetResolution 0.01
  $o(percentage) SetLabelPositionToTop
  $o(percentage) Create
  $o(percentage) SetRange 0.0 1.0
  $o(percentage) SetValue $percentage
  $o(percentage) SetLabelText "Dynamic range percentage"
  $o(percentage) SetBalloonHelpString "Set the percentage of the dynamic range to group with the seed (default 0.1).\n\nUse the + and - keys in the slice windows to change interactively."
  pack [$o(percentage) GetWidgetName] \
    -side top -anchor e -fill x -padx 2 -pady 2 

  #
  # a cancel button
  #
  set o(cancel) [vtkNew vtkKWPushButton]
  $o(cancel) SetParent [$this getOptionsFrame]
  $o(cancel) Create
  $o(cancel) SetText "Cancel"
  $o(cancel) SetBalloonHelpString "Cancel wand without applying to label map."
  pack [$o(cancel) GetWidgetName] \
    -side right -anchor e -padx 2 -pady 2 

  #
  # event observers - TODO: if there were a way to make these more specific, I would...
  #
  set tag [$o(percentage) AddObserver AnyEvent "after idle $this updateMRMLFromGUI"]
  lappend _observerRecords "$o(percentage) $tag"
  set tag [$o(cancel) AddObserver AnyEvent "after idle ::EffectSWidget::RemoveAll"]
  lappend _observerRecords "$o(cancel) $tag"

  if { [$this getInputBackground] == "" || [$this getOutputLabel] == "" } {
    $this errorDialog "Background and Label map needed for wand"
    after idle ::EffectSWidget::RemoveAll
  }

  $this updateGUIFromMRML
}

itcl::body WandEffect::updateMRMLFromGUI { } {
  #
  # set the node to the current value of the GUI
  # - this will be saved/restored with the scene
  # - all instances of the effect are observing the node,
  #   so changes will propogate automatically
  #
  chain
  set node [EditorGetParameterNode]
  $node SetParameter "Wand,percentage" [$o(percentage) GetValue]
}

itcl::body WandEffect::setMRMLDefaults { } {
  chain
  set node [EditorGetParameterNode]
  foreach {param default} {
    percentage 0.1
  } {
    set pvalue [$node GetParameter Wand,$param] 
    if { $pvalue == "" } {
      $node SetParameter Wand,$param $default
    } 
  }
}


itcl::body WandEffect::updateGUIFromMRML { } {
  #
  # get the parameter from the node
  # - set default value if it doesn't exist
  #
  chain

  # set the GUI and effect parameters to match node
  # (only if this is the instance that "owns" the GUI
  $this configure -percentage $percentage
  if { [info exists o(percentage)] } {
    $o(percentage) SetValue $percentage
  }
  $this preview
}

itcl::body WandEffect::tearDownOptions { } {

  # call superclass version of tearDownOptions
  chain

  foreach w "percentage cancel" {
    if { [info exists o($w)] } {
      $o($w) SetParent ""
      pack forget [$o($w) GetWidgetName] 
    }
  }
}
