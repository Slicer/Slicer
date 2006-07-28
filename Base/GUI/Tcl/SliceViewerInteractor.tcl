

#
# variables to manage interaction state
# - these can be global since theyere is only ever one interaction
#   going on at a time in the GUI
#
proc SliceViewerInitialize {} {

  if { [info exists ::SliceViewerMode] } {
    return
  }

  if { ![info exists ::SliceViewerMode] } {
    set ::SliceViewerMode ""
  }

  set matrices "::SliceViewerStorageXYToRAS ::SliceViewerStorageSliceToRAS ::SliceViewerScratchMatrix" 
  foreach m $matrices {
    if { ![info exists $m] } {
      set $m [vtkMatrix4x4 New]
    }
  }
}

proc SliceViewerShutdown {} {

  if { [info exists ::SliceViewerMode] } {
    unset ::SliceViewerMode
  }

  set matrices "::SliceViewerStorageXYToRAS ::SliceViewerStorageSliceToRAS ::SliceViewerScratchMatrix" 
  foreach m $matrices {
    if { [info exists $m] } {
      [set $m] Delete
      unset $m
    }
  }
}

proc SliceViewerGetPixel {image i j k} {

  foreach index "i j k" dimension [$image GetDimensions] {
    set ind [set $index]
    if { $ind < 0 || $ind >= $dimension } {return "Unknown"}
  }
  set n [$image GetNumberOfScalarComponents]
  for {set c 0} {$c < $n} {incr c} {
    lappend pixel [$image GetScalarComponentAsDouble $i $j $k $c]
  }
  return $pixel
}

proc SliceViewerSetPixelBlock {image i j k size value} {

  for {set ii [expr $i - $size]} {$ii <= [expr $i + $size]} {incr ii} {
    for {set jj [expr $j - $size]} {$jj <= [expr $j + $size]} {incr jj} {
      for {set kk [expr $k - $size]} {$kk <= [expr $k + $size]} {incr kk} {
        SliceViewerSetPixel $image $ii $jj $kk $value
      }
    }
  }
}

proc SliceViewerSetPixel {image i j k value} {

  foreach index "i j k" dimension [$image GetDimensions] {
    set ind [set $index]
    if { $ind < 0 || $ind >= $dimension } {return -1}
  }
  set n [$image GetNumberOfScalarComponents]
  for {set c 0} {$c < $n} {incr c} {
    set v [lindex $value $c]
    if { $v != "" } { 
      $image SetScalarComponentFromDouble $i $j $k $c $v
      $image Modified
    }
  }
  return 0
}


#
# Handle events passes up by the sliceGUI
#

proc SliceViewerHandleEvent {sliceGUI event} {

  # initialize on first call 
  # -- allows clean shutdown because each vtkSlicerSliceGUI can shutdown in destructor
  # -- this is very light weight, so no problem calling every time
  SliceViewerInitialize

  set renderer [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetRenderer]
  set interactor [[[$sliceGUI GetSliceViewer] GetRenderWidget] GetRenderWindowInteractor]
  if { $event != "ConfigureEvent" } {
    eval $interactor UpdateSize [$renderer GetSize]
  }
  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set anno [$renderWidget GetCornerAnnotation]
  set sliceNode [[$sliceGUI GetLogic]  GetSliceNode]
  set sliceCompositeNode [[$sliceGUI GetLogic]  GetSliceCompositeNode]
  set background [[$sliceGUI GetLogic]  GetBackgroundLayer]
  set backgroundNode [$background GetVolumeNode]
  set backgroundImage [$backgroundNode GetImageData]
  set foreground [[$sliceGUI GetLogic]  GetForegroundLayer]
  set foregroundNode [$foreground GetVolumeNode]
  set foregroundImage [$foregroundNode GetImageData]
  
  foreach {x y} [$interactor GetEventPosition] {}

  set xyToRAS [$sliceNode GetXYToRAS]
  set ras [$xyToRAS MultiplyPoint $x $y 0 1]
  foreach {r a s t} $ras {}

  set xyToIJK [[$background GetXYToIJKTransform] GetMatrix]
  foreach {i j k l} [$xyToIJK MultiplyPoint $x $y 0 1] {}
  foreach v {i j k} { ;# cast to integer
    set $v [expr int(round([set $v]))]
  }


  set bgPixel [SliceViewerGetPixel $backgroundImage $i $j $k]

  set ignoreEvents "MouseMoveEvent ModifiedEvent TimerEvent RenderEvent"

  if { [lsearch $ignoreEvents $event] == -1 } {
    puts -nonewline "got a $event for $sliceGUI, interactor $interactor at xy $x $y, ras $ras"
    if { [$interactor GetControlKey] } {
      puts -nonewline " with control"
    }
    if { [$interactor GetShiftKey] } {
      puts -nonewline " with shift"
    }
    puts ""
  }

  if { [string match "*PressEvent" $event] } {
    set ::SliceViewerStartRAS $ras 
    set ::SliceViewerStartXY [$interactor GetEventPosition]
  }

  if { [string match "*ReleaseEvent" $event] } {
    set ::SliceViewerMode ""
  }


  switch $event {

    "MouseMoveEvent" {
      #
      # Mouse move behavior governed by global mode
      # - first update the annotation
      # - then handle modifying the view
      #
      # puts "background pixel at $i $j $k is $pixel"

      $anno SetText 0 "Fg:\nBg: $bgPixel"
      $anno SetText 1 "I: $i\nJ:$j\nK: $k"
      $anno SetText 2 "X: $x\nY:$y"
      $anno SetText 3 "R: $r\nA: $a\n S: $s"

      switch $::SliceViewerMode {
        "Translate" {
          #
          # Translate
          #
          set currentRAS [$::SliceViewerStorageXYToRAS MultiplyPoint $x $y 0 1]
          foreach d {dr da ds} start $::SliceViewerStartRAS current $currentRAS {
            set $d [expr $current - $start]
          }
          $::SliceViewerScratchMatrix DeepCopy $::SliceViewerStorageSliceToRAS
          foreach d {dr da ds} i {0 1 2} {
            set v [$::SliceViewerScratchMatrix GetElement $i 3]
            $::SliceViewerScratchMatrix SetElement $i 3 [expr $v - [set $d]]
          }
          [$sliceNode GetSliceToRAS] DeepCopy $::SliceViewerScratchMatrix
          $sliceNode UpdateMatrices
        }
        "Zoom" {
          #
          # Zoom
          #
          set deltay [expr $y - [lindex $::SliceViewerStartXY 1]]
          set h [lindex [[[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetRenderWindow]  GetSize] 1]
          set percent [expr ($h + $deltay) / (1.0 * $h)]
          set newFOV ""
          foreach f $::SliceViewerStorageFieldOfView {
            lappend newFOV [expr $f * $percent]
          }
          eval $sliceNode SetFieldOfView $newFOV
          $sliceNode UpdateMatrices
        }
        "Paint" {
          SliceViewerSetPixel $backgroundImage $i $j $k 0
          #SliceViewerSetPixelBlock $backgroundImage $i $j $k 2 0
          $backgroundNode Modified ;# TODO: the layer logic should be observing the volume node ImageDataChangedEvent
        }
        default {
          # need to render to show the annotation
          [$sliceGUI GetSliceViewer] RequestRender
        }
      }
    }

    "RightButtonPressEvent" {
      set ::SliceViewerMode "Zoom"
      set ::SliceViewerStorageFieldOfView [$sliceNode GetFieldOfView]
      $::slicer3::MRMLScene SaveStateForUndo $sliceNode
    }
    "RightButtonReleaseEvent" { }
    "LeftButtonPressEvent" {
      if { [info command SeedSWidget] != "" } {
        if { [$interactor GetControlKey] } {
          set seedSWidget [SeedSWidget #auto $sliceGUI]
          $seedSWidget place $r $a $s
        }
      }
## Testing
      set ::SliceViewerMode "Paint"
      SliceViewerSetPixel $backgroundImage $i $j $k 0
      #SliceViewerSetPixelBlock $backgroundImage $i $j $k 2 0
      $backgroundNode Modified ;# TODO: the layer logic should be observing the volume node ImageDataChangedEvent
      puts "$backgroundNode Modified"
## Testing
    }
    "LeftButtonReleaseEvent" { }
    "MiddleButtonPressEvent" {
      set ::SliceViewerMode "Translate"
      $::SliceViewerStorageXYToRAS DeepCopy [$sliceNode GetXYToRAS]
      $::SliceViewerStorageSliceToRAS DeepCopy [$sliceNode GetSliceToRAS]
      $::slicer3::MRMLScene SaveStateForUndo $sliceNode
    }
    "MiddleButtonReleaseEvent" { }
    "MouseWheelForwardEvent" { }
    "MouseWheelBackwardEvent" { }
    "ExposeEvent" { }
    "ConfigureEvent" {

      if {0} {
        #
        # the different parts of kww doen't have the same size info!
        # -- the tk widget appears to always have the correct size info, so use it
        #
        set size [[[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetRenderWindow]  GetSize]
        foreach {w h} $size {}
        puts "Configure: Size $size"
        puts "rw width [[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetWidth]"
        puts "rw height [[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetHeight]"
        puts "widget width [winfo width [[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetWidgetName]]"
        puts "widget height [winfo height [[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetWidgetName]]"
      }

      set tkwindow [[[$sliceGUI GetSliceViewer]  GetRenderWidget]  GetWidgetName]
      set w [winfo width $tkwindow]
      set h [winfo height $tkwindow]

      if { $w == "10" && $h == "10" } {
        puts "ignoring bogus resize"
        puts [$sliceGUI Print]
      } else {
        set oldFOV [$sliceNode GetFieldOfView]
        set oldDim [$sliceNode GetDimensions]
        set oldPixelSize [expr [lindex $oldFOV 0] / (1. * [lindex $oldDim 0])]
        $sliceNode SetDimensions $w $h [lindex $oldDim 2]
        $sliceNode SetFieldOfView \
            [expr $oldPixelSize * $w] [expr $oldPixelSize * $h] [lindex $oldFOV 2]
      }
    }
    "EnterEvent" { 
      $renderWidget CornerAnnotationVisibilityOn
      [$::slicer3::ApplicationGUI GetMainSlicerWin]  SetStatusText "Middle Button: Pan; Left Button: Zoom"
    }
    "LeaveEvent" { 
      $renderWidget CornerAnnotationVisibilityOff
      [$::slicer3::ApplicationGUI GetMainSlicerWin]  SetStatusText ""
    }
    "TimerEvent" { }
    "KeyPressEvent" { 
      switch [$interactor GetKeyCode] {
        "v" {
          $sliceNode SetSliceVisible [expr ![$sliceNode GetSliceVisible]]
        }
      }
    }
    "KeyReleaseEvent" { }
    "CharEvent" {
      puts -nonewline "char event [$interactor GetKeyCode]"
      if { [$interactor GetControlKey] } {
        puts -nonewline " with control"
      }
      if { [$interactor GetShiftKey] } {
        puts -nonewline " with shift"
      }
      puts ""
    }
    "ExitEvent" { }

  }
}
