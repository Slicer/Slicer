
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
    method apply3D {} {}
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
  set _scopeOptions "visible"
  $this configure -scope $_scopeOptions
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

  if { $caller == $sliceGUI } {
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
            "3" {
              $this apply3D
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
  }

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::body WandEffect::apply {} {

  if { [$this getInputLabel] == "" || [$this getInputLabel] == "" } {
    $this flashCursor 3
    return
  }

  $this configure -polygonDebugViewer 1

  $this applyPolyMask [$o(wandIJKToXY) GetOutput]
  puts "applied!"
}

itcl::body WandEffect::apply3D {} {

  $this errorDialog "no 3d mode defined yet"

  if { 0 } {

    if { ![info exists o(tracing3DFilter)] } {
      set o(tracing3DFilter) [vtkNew vtkITKLevelTracing3DImageFilter]
    }

    $o(tracing3DFilter) SetInput [$this getInputBackground]
    $o(tracing3DFilter) SetSeed $_layers(background,i) $_layers(background,j) $_layers(background,k) 

    $_layers(label,node) SetAndObserveImageData [$o(tracing3DFilter) GetOutput] 
    $_layers(label,node) Modified

    $o(tracing3DFilter) Update
  }
}

itcl::body WandEffect::preview {} {

  #
  # create pipeline as needed
  #
  if { ![info exists o(wandFilter)] } {

    set o(wandFilter) [vtkNew vtkITKWandImageFilter]

    set o(wandMarching) [vtkNew vtkMarchingSquares]
    $o(wandMarching) SetInput [$o(wandFilter) GetOutput]
    $o(wandMarching) SetNumberOfContours 1
    $o(wandMarching) SetValue 0 1

    set o(stripper) [vtkNew vtkStripper]
    $o(stripper) SetInput [$o(wandMarching) GetOutput]

    set o(cleaner) [vtkNew vtkCleanPolyData]
    $o(cleaner) SetInput [$o(stripper) GetOutput]

    set o(ijkToXY) [vtkNew vtkTransform]
    set o(wandIJKToXY) [vtkNew vtkTransformPolyDataFilter]
    $o(wandIJKToXY) SetTransform $o(ijkToXY)
    $o(wandIJKToXY) SetInput [$o(cleaner) GetOutput]

    set o(wandMapper) [vtkNew vtkPolyDataMapper2D]
    set o(wandActor) [vtkNew vtkActor2D]
    $o(wandActor) SetMapper $o(wandMapper)
    $o(wandMapper) SetInput [$o(wandIJKToXY) GetOutput]
    set property [$o(wandActor) GetProperty]
    $property SetColor [expr 107/255.] [expr 190/255.] [expr 99/255.]
    $property SetLineWidth 1
    [$_renderWidget GetRenderer] AddActor2D $o(wandActor)
    lappend _actors $o(wandActor)
  }

  # 
  # get the event position to use as a seed
  #
  foreach {x y} [$_interactor GetEventPosition] {}
  foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}

  if { $x < 0 || $y < 0 || $x > $windoww || $y > $windowh } {
    puts "bad event position $x, $y not in $windoww $windowh"
    return
  }

  $o(wandFilter) SetInput [$this getInputBackground]
  foreach {w h d} [[$o(wandFilter) GetInput] GetDimensions] {}

  set visibleToWindow [vtkMatrix4x4 New]
  $visibleToWindow Identity
  $visibleToWindow SetElement 0 0 [expr $windoww / $w]
  $visibleToWindow SetElement 1 1 [expr $windowh / $h]
  #$visibleToWindow SetElement 0 3 [$ijkToXY GetElement 0 3]
  #$visibleToWindow SetElement 1 3 [$ijkToXY GetElement 1 3]
  #$visibleToWindow SetElement 2 3 [$ijkToXY GetElement 2 3]

  $visibleToWindow Invert
  set seed [$visibleToWindow MultiplyPoint $x $y 0 1]
  $visibleToWindow Invert
  set seedx [expr int([lindex $seed 0])]
  set seedy [expr int([lindex $seed 1])]
  puts "$o(wandFilter) SetSeed $seedx $seedy 0"
  $o(wandFilter) SetSeed $seedx $seedy 0
  puts "wand $x $y ($seed)"
  if { $seedx < 0 || $seedy < 0 || $seedx > $w || $seedy > $h } {
    puts "seed out of visible range"
    puts "wand $x $y ($seed)"
    return
  }

  $o(wandFilter) SetDynamicRangePercentage $percentage

  $o(ijkToXY) SetMatrix $visibleToWindow
  $o(wandFilter) Update


  $visibleToWindow Delete

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender

  [$o(wandMarching) GetOutput] Update
  puts "marching [[$o(wandMarching) GetOutput] GetBounds]"

  [$o(wandIJKToXY) GetOutput] Update
  puts "transformed: [[$o(wandIJKToXY) GetOutput] GetBounds]"
  puts ""

  #
  # viewer window...
  #
  if { 1 } {
    if { [info command wandViewer] == "" } {
      vtkImageViewer wandViewer
    }
    wandViewer SetColorWindow 2
    wandViewer SetColorLevel 1
    wandViewer SetInput [$o(wandFilter) GetOutput]
    wandViewer Render
  }

  return

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

  if { [$this getInputBackground] == "" || [$this getInputLabel] == "" } {
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
