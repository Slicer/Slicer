
package require Itcl

#########################################################
#
if {0} { ;# comment

  EffectSWidget a superclass for editor effects

  - subclasses of this need to define their specialness

# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             EffectSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class EffectSWidget] == "" } {

  itcl::class EffectSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable scope "all"
    public variable animationSteps "20"
    public variable animationDelay "100"
    public variable exitCommand ""

    variable _renderer ""
    variable _startPosition "0 0 0"
    variable _currentPosition "0 0 0"
    variable _cursorActors ""
    variable _outputLabel ""
    variable _observerRecords "" ;# list of the observers so we can clean up
    variable _cursorAnimationTag ""
    variable _cursorAnimationState 0

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method preProcessEvent {{caller ""} {event ""}} {}
    method positionCursor {} {}
    method createCursor {} {}
    method setCursor {imageData} { $o(cursorMapper) SetInput $imageData }
    method preview {} {}
    method apply {} {}
    method postApply {} {}
    method getInputBackground {} {}
    method getInputLabel {} {}
    method getOutputLabel {} {}
    method getOptionsFrame {} {}
    method buildOptions {} {}
    method tearDownOptions {} {}
    method previewOptions {} {}
    method applyOptions {} {}
    method setMRMLDefaults {} {}
    method updateGUIFromMRML {} {}
    method flashCursor { {repeat 1} {delay 50} } {}
    method animateCursor { {onOff "on"} } {}
    method setAnimationState { p } {}
    method setProgressFilter { filter {description ""} } {}
    method progressCallback { event caller description } {}
    method errorDialog { message } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body EffectSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  $this createCursor

  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"

  $this processEvent

  set tag [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _observerRecords "$sliceGUI $tag"
  set events {
    LeftButtonPressEvent LeftButtonReleaseEvent 
    RightButtonPressEvent RightButtonReleaseEvent 
    MouseMoveEvent KeyPressEvent EnterEvent LeaveEvent
  }
  foreach event $events {
    set tag [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent $sliceGUI"]
    lappend _observerRecords "$sliceGUI $tag"
  }

  set node [[$sliceGUI GetLogic] GetSliceNode]
  set tag [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _observerRecords "$node $tag"
  set tag [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]
  lappend _observerRecords "$node $tag"

  set node [EditorGetParameterNode]
  set tag [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _observerRecords "$node $tag"
  set tag [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent $node"]
  lappend _observerRecords "$node $tag"
}

itcl::body EffectSWidget::destructor {} {

  $this animateCursor off
  $this tearDownOptions

  foreach record $_observerRecords {
    foreach {obj tag} $record {
      if { [info command $obj] != "" } {
        $obj RemoveObserver $tag
      }
    }
  }

  if { [info command $_renderer] != "" } {
    foreach a $_cursorActors {
      $_renderer RemoveActor2D $a
    }
    foreach a $_actors {
      $_renderer RemoveActor2D $a
    }
    [$sliceGUI GetSliceViewer] RequestRender
  }

  if { $exitCommand != "" } {
    eval $exitCommand
  }

}

itcl::configbody EffectSWidget::scope {
  if { [lsearch "all visible" $scope] == -1 } {
    set scope "all"
    error "invalid scope for EffectSWidget $this"
  }
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body EffectSWidget::createCursor {} {

  set o(cursorDummyImage) [vtkNew vtkImageData]
  $o(cursorDummyImage) AllocateScalars
  set o(cursorMapper) [vtkNew vtkImageMapper]
  $o(cursorMapper) SetInput $o(cursorDummyImage)
  set o(cursorActor) [vtkNew vtkActor2D]
  $o(cursorActor) SetMapper $o(cursorMapper)
  $o(cursorMapper) SetColorWindow 255
  $o(cursorMapper) SetColorLevel 128

  lappend _cursorActors $o(cursorActor)

  set _renderer [$_renderWidget GetRenderer]
  foreach actor $_cursorActors {
    $_renderer AddActor2D $o(cursorActor)
  }

  $o(cursorActor) VisibilityOff
}

itcl::body EffectSWidget::positionCursor {} {
  set xyzw [$this rasToXY $_currentPosition]
  foreach {x y z w} $xyzw {}

  if { $x == "nan" } {
    puts "Bad cursor position in $this"
    return
  }

  set x [expr $x + 16]
  set y [expr $y - 32]
  foreach actor $_cursorActors {
    eval $actor SetPosition $x $y
  }
}

itcl::body EffectSWidget::getOptionsFrame { } {
  EditorSelectModule
  return [EditorGetOptionsFrame $::Editor(singleton)]
}

itcl::body EffectSWidget::buildOptions { } {
  # default implementation, there is nothing
}

itcl::body EffectSWidget::tearDownOptions { } {
  # default implementation, there is nothing
}



itcl::body EffectSWidget::flashCursor { {repeat 1} {delay 50} } {

  for {set i 0} {$i < $repeat} {incr i} {
    set oldVisibility [$o(cursorActor) GetVisibility]
    $o(cursorActor) SetVisibility [expr !$oldVisibility]
    [$sliceGUI GetSliceViewer] RequestRender
    update
    after $delay
    $o(cursorActor) SetVisibility $oldVisibility
    [$sliceGUI GetSliceViewer] RequestRender
    update
  }
}

#
# background animation for the cursor
# - useful for showing events as 'pending'
#
itcl::body EffectSWidget::animateCursor { {onOff "on"} } {
  
  if { $onOff == "off" } {
    if { $_cursorAnimationTag != "" } {
      after cancel $_cursorAnimationTag
      set $_cursorAnimationTag ""
    }
    $this setAnimationState 1
    set _cursorAnimationState 0
    [$sliceGUI GetSliceViewer] Render
    return
  }

  set p [expr $_cursorAnimationState / (1.0 * $animationSteps)]

  $this setAnimationState $p

  # force a render
  [$sliceGUI GetSliceViewer] RequestRender
  #[$sliceGUI GetSliceViewer] Render

  incr _cursorAnimationState
  set _cursorAnimationTag [after $animationDelay "$this animateCursor on"]
}

#
# for virtual override 
# - p will be 0 to 1 loop
#
itcl::body EffectSWidget::setAnimationState { p } {
  # example - set opacity of actor in cosine
  # (won't work for image actors)
  # see ThresholdEffect for example on image
  set amt [expr 0.5 * (1 + cos(6.2831852 * ($p - floor($p)))) ]
  foreach a $_cursorActors {
    [$a GetProperty] SetOpacity $amt
  }
}

#
# returns 1 if the event is 'swallowed' by
# the superclass, otherwise returns 0 and the
# subclass should do normal processing
#
itcl::body EffectSWidget::preProcessEvent { {caller ""} {event ""} } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return 1
  }

  set grabID [$sliceGUI GetGrabID]
  if { ($grabID != "") && ($grabID != $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionCursor
    [$sliceGUI GetSliceViewer] RequestRender
    return 1
  }

  #
  # if the caller was the parameter node, invoke the subclass's 
  # updateGUIFromMRML method which will copy the parameters into the 
  # GUI and into the configuration options of the effect
  #
  if { $caller == [EditorGetParameterNode] } {
    $this updateGUIFromMRML
    return 1
  }


  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $event {
    "KeyPressEvent" {
      set key [$_interactor GetKeySym]
      if { [lsearch "Escape" $key] != -1 } {
        $sliceGUI SetCurrentGUIEvent "" ;# reset event so we don't respond again
        $sliceGUI SetGUICommandAbortFlag 1
        switch [$_interactor GetKeySym] {
          "Escape" {
            after idle ::EffectSWidget::RemoveAll
            return 1
          }
        }
      } else {
        # puts "effect ignoring $key"
      }
    }
  }

  return 0
}

#
# manage the data to work on:
# - input is always the current contents of the 
#   background and label layers
#   -- can be either the whole volume or just visible portion
# - output is a temp buffer that gets re-inserted into
#   the label layer using the postApply method
# 

itcl::body EffectSWidget::getInputBackground {} {
  set logic [[$sliceGUI GetLogic]  GetBackgroundLayer]
  switch $scope {
    "all" {
      set node [$logic GetVolumeNode]
      if { $node != "" } {
        return [$node GetImageData]
      }
    }
    "visible" {
      #return [$logic GetImageData]
      return [[$logic GetReslice] GetOutput]
    }
  }
  return ""
}

itcl::body EffectSWidget::getInputLabel {} {
  set logic [[$sliceGUI GetLogic]  GetLabelLayer]
  switch $scope {
    "all" {
      set node [$logic GetVolumeNode]
      if { $node != "" } {
        return [$node GetImageData]
      }
    }
    "visible" {
      return [$logic GetImageData]
    }
  }
  return ""
}

itcl::body EffectSWidget::getOutputLabel {} {
  if { $_outputLabel == "" } {
    set _outputLabel [vtkImageData New]
  }
  return $_outputLabel
}

itcl::body EffectSWidget::postApply {} {
  set logic [[$sliceGUI GetLogic]  GetLabelLayer]
  switch $scope {
    "all" {
      set node [$logic GetVolumeNode]
      [$node GetImageData] DeepCopy $_outputLabel
      $node SetModifiedSinceRead 1
      $node Modified
    }
    "visible" {
      # TODO: need to use vtkImageSlicePaint to insert visible
      # paint back into the label volume
      error "not yet supported"
    }
  }
  $_outputLabel Delete
  set _outputLabel ""
}


#
# default implementations of methods to be overridden by subclass
#

itcl::body EffectSWidget::preview {} {
  # to be overridden by subclass
}


itcl::body EffectSWidget::processEvent { {caller ""} {event ""} } {
  # to be overridden by subclass
  # - should include call to superclass preProcessEvent
  #   to handle 'friendly' interaction with other SWidgets

  $this preProcessEvent

  # your event processing can replace the dummy code below...

  set event [$sliceGUI GetCurrentGUIEvent] 
  set _currentPosition [$this xyToRAS [$_interactor GetEventPosition]]

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

  $this positionCursor
  [$sliceGUI GetSliceViewer] RequestRender

}

itcl::body EffectSWidget::apply {} {
  # default behavior, just flash...
  $this flashCursor 3
}

itcl::body EffectSWidget::errorDialog { errorText } {
  set dialog [vtkKWMessageDialog New]
  $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  $dialog SetStyleToMessage
  $dialog SetText $errorText
  $dialog Create
  $dialog Invoke
  $dialog Delete
}

itcl::body EffectSWidget::setProgressFilter { filter {description ""} } {

  foreach event {StartEvent ProgressEvent EndEvent} {
    $filter AddObserver $event [list $this progressCallback $event $filter "$description"]
  }

}

itcl::body EffectSWidget::progressCallback { event caller description } {

  set mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  set progressGauge [$mainWindow GetProgressGauge]

  switch $event {
    "StartEvent" { 
      $mainWindow SetStatusText "$description"
      $progressGauge SetValue 0
    }
    "ProgressEvent" {
      $progressGauge SetValue [expr 100 * [$caller GetProgress]]
    }
    "DeleteEvent" - 
    "EndEvent" {
      $progressGauge SetValue 0
      $mainWindow SetStatusText ""
    }
  }
}



#
# helper procs to manage effects
#

proc EffectSWidget::Add {effect} {
  foreach sw [itcl::find objects -class SliceSWidget] {
    set sliceGUI [$sw cget -sliceGUI]
    if { [info command $sliceGUI] != "" } {
      $effect #auto $sliceGUI
    }
  }
}

proc EffectSWidget::RemoveAll {} {
  foreach ew [itcl::find objects -isa EffectSWidget] {
    itcl::delete object $ew
  }
}

proc EffectSWidget::Remove {effect} {
  foreach pw [itcl::find objects -class $effect] {
    itcl::delete object $pw
  }
}

proc EffectSWidget::Toggle {effect} {
  if { [itcl::find objects -class $effect] == "" } {
    EffectSWidget::Add $effect
  } else {
    EffectSWidget::Remove $effect
  }
}

proc EffectSWidget::ConfigureAll { effect args } {
  foreach pw [itcl::find objects -class $effect] {
    eval $pw configure $args
  }
}

proc EffectSWidget::SetCursorAll { effect imageData } {

  foreach ew [itcl::find objects -class $effect] {
    $ew setCursor $imageData
  }
}
