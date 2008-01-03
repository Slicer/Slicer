
package require Itcl

#########################################################
#
if {0} { ;# comment

  SeedSWidget a class for slicer fiducials in 2D


# TODO : 

}
#
#########################################################

#
#########################################################
# ------------------------------------------------------------------
#                             SeedSWidget
# ------------------------------------------------------------------
#
# The class definition - define if needed (not when re-sourcing)
#
if { [itcl::find class SeedSWidget] == "" } {

  itcl::class SeedSWidget {

    inherit SWidget

    constructor {args} {}
    destructor {}

    public variable movedCommand ""
    public variable movingCommand ""
    public variable glyph "StarBurst"
    public variable scale "1"
    public variable color "1 0 0"
    public variable selectedColor "1 1 0"
    public variable opacity "1"
    public variable selected "0"
    public variable text ""
    public variable textScale "1"

    variable _startOffset "0 0 0"
    variable _currentPosition "0 0 0"

    # methods
    method processEvent {{caller ""} {event ""}} {}
    method positionActors {} {}
    method pick {} {}
    method place {x y z} {}
    method setRASPosition {r a s} { $this place $x $y $z }
    method getRASPosition {} { return $_currentPosition }
    method getPickState {} { return $_pickState }
    method highlight {} {}
    method createGlyph { {type "StarBurst"} } {}
  }
}

# ------------------------------------------------------------------
#                        CONSTRUCTOR/DESTRUCTOR
# ------------------------------------------------------------------
itcl::body SeedSWidget::constructor {sliceGUI} {

  $this configure -sliceGUI $sliceGUI
 
  set o(glyph) [$this createGlyph]
  set o(glyphTransform) [vtkNew vtkTransform]
  set o(glyphTransformFilter) [vtkNew vtkTransformPolyDataFilter]
  $o(glyphTransformFilter) SetInput $o(glyph)
  $o(glyphTransformFilter) SetTransform $o(glyphTransform)
  set o(mapper) [vtkNew vtkPolyDataMapper2D]
  set o(actor) [vtkNew vtkActor2D]
  $o(mapper) SetInput [$o(glyphTransformFilter) GetOutput]
  $o(actor) SetMapper $o(mapper)
  set _renderer [$_renderWidget GetRenderer]
  $_renderer AddActor2D $o(actor)
  lappend _actors $o(actor)

  set o(textActor) [vtkNew vtkTextActor]
  $_renderer AddActor2D $o(textActor)
  set textProperty [$o(textActor) GetTextProperty]
  $textProperty ShadowOn
  lappend _actors $o(textActor)

  set _startPosition "0 0 0"
  set _currentPosition "0 0 0"

  $this processEvent

  set _guiObserverTags ""
  lappend _guiObserverTags [$sliceGUI AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  foreach event {LeftButtonPressEvent LeftButtonReleaseEvent MouseMoveEvent} {
    lappend _guiObserverTags [$sliceGUI AddObserver $event "::SWidget::ProtectedCallback $this processEvent"]    }
  set node [[$sliceGUI GetLogic] GetSliceNode]
  lappend _nodeObserverTags [$node AddObserver DeleteEvent "::SWidget::ProtectedDelete $this"]
  lappend _nodeObserverTags [$node AddObserver AnyEvent "::SWidget::ProtectedCallback $this processEvent"]

}

itcl::body SeedSWidget::destructor {} {

  $o(glyphTransformFilter) SetInput ""
  $o(glyphTransformFilter) SetTransform ""
  $o(glyph) Delete

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


itcl::configbody SeedSWidget::glyph {

  set validGlyphTypes {None Vertex Dash Cross ThickCross Triangle
    Square Circle Diamond Arrow ThickArrow HookedArrow StarBurst}

  if { [lsearch $validGlyphTypes $glyph] == -1 } {
    set glyph "StarBurst"
  }

  $o(glyph) Delete
  set o(glyph) [$this createGlyph $glyph]
  $o(glyphTransformFilter) SetInput $o(glyph)

}

itcl::configbody SeedSWidget::scale {
  $o(glyphTransform) Identity
  $o(glyphTransform) Scale $scale $scale $scale 
  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::color {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::selectedColor {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::opacity {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::selected {
  $this highlight
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::text {
  $o(textActor) SetInput $text
  [$sliceGUI GetSliceViewer] RequestRender
}

itcl::configbody SeedSWidget::textScale {
  set textProperty [$o(textActor) GetTextProperty]
  set fontSize [expr round(2.5 * $textScale)]
  $textProperty SetFontSize $fontSize
  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

# ------------------------------------------------------------------
#                             METHODS
# ------------------------------------------------------------------

itcl::body SeedSWidget::createGlyph { {type "StarBurst"} } {
  
  set glyphSource [vtkSlicerGlyphSource2D New]
  $glyphSource SetGlyphTypeTo$type
  set polyData [$glyphSource GetOutput]
  $polyData Register ""
  $polyData Update
  [$polyData GetCellData] SetScalars ""
  $glyphSource SetOutput ""
  $glyphSource Delete
  return $polyData
}

itcl::body SeedSWidget::pick {} {

  foreach {x y z} [$this rasToXYZ $_currentPosition] {}
  foreach {wx wy} [$_interactor GetEventPosition] {}
  foreach {ex ey ez} [$this dcToXYZ $wx $wy] {}
  if { [expr abs($ex - $x) < 15] && [expr abs($ey - $y) < 15] } {
    set _pickState "over"
    set _startOffset [list [expr $x - $ex] [expr $y - $ey] [expr $z - $ez]]
  } else {
    set _pickState "outside"
  }
}

itcl::body SeedSWidget::place {x y z} {
  set _currentPosition "$x $y $z"
  $this positionActors
}

itcl::body SeedSWidget::positionActors { } {

  # determine the xyz location of the fiducial
  set xyzw [$this rasToXYZ $_currentPosition]
  foreach {x y z w} $xyzw {}
  $o(actor) SetPosition $x $y
  set x [expr $x + $scale]
  set y [expr $y + $scale]
  $o(textActor) SetPosition $x $y

  # determine which renderer based on z position
  set k [expr int($z + 0.5)]

  # remove the seed from the old renderer and add it to the new one
  if { [info command $_renderer] != ""} {
    $_renderer RemoveActor2D $o(actor)
    $_renderer RemoveActor2D $o(textActor)
  }

  if { $k >= 0 && $k < [$_renderWidget GetNumberOfRenderers] } {
    set _renderer [$_renderWidget GetNthRenderer $k]
    if { [info command $_renderer] != ""} {
      $_renderer AddActor2D $o(actor)
      $_renderer AddActor2D $o(textActor)
    }
  }
}

itcl::body SeedSWidget::highlight { } {

  set property [$o(actor) GetProperty]
  set textProperty [$o(textActor) GetTextProperty]
  if { $selected } {
    eval $property SetColor $selectedColor
    eval $textProperty SetColor $selectedColor
  } else {
    eval $property SetColor $color
    eval $textProperty SetColor $color
  }
  $property SetLineWidth 1
  $property SetOpacity $opacity
  $textProperty SetOpacity $opacity

  set _description ""
  switch $_actionState {
    "dragging" {
      $property SetColor 0 1 0
      set _description "Move mouse with left button down to drag"
    }
    default {
      switch $_pickState {
        "over" {
          $property SetColor 1 1 0
          $property SetLineWidth 2
          set _description "Move mouse with left button down to drag"
        }
      }
    }
  }
}

itcl::body SeedSWidget::processEvent { {caller ""} {event ""} } {

  if { [info command $sliceGUI] == "" } {
    # the sliceGUI was deleted behind our back, so we need to 
    # self destruct
    itcl::delete object $this
    return
  }

  set grabID [$sliceGUI GetGrabID]
  if { ! ($grabID == "" || $grabID == $this) } {
    # some other widget wants these events
    # -- we can position wrt the current slice node
    $this positionActors
    [$sliceGUI GetSliceViewer] RequestRender
    return 
  }

  if { $_actionState != "dragging" } {
    # only check pick if we haven't grabbed (avoid 'dropping' the widget
    # when the mouse moves quickly)
    $this pick
  }

  set event [$sliceGUI GetCurrentGUIEvent] 

  switch $_pickState {
    "outside" {
      # when mouse isn't over us, we don't do anything
      set _actionState ""
      $sliceGUI SetGrabID ""
    }
    "over" {
      # when mouse is over us, we pay attention to the
      # event and tell others not to look at it
      $sliceGUI SetGUICommandAbortFlag 1
      switch $event {
        "LeftButtonPressEvent" {
          set _actionState "dragging"
          $sliceGUI SetGrabID $this
        }
        "MouseMoveEvent" {
          switch $_actionState {
            "dragging" {
              foreach {wx wy} [$_interactor GetEventPosition] {}
              foreach {ex ey ez} [$this dcToXYZ $wx $wy] {}
              foreach {dx dy dz} $_startOffset {}
              set newxyz [list [expr $ex + $dx] [expr $ey + $dy] [expr $ez + $dz]]
              set _currentPosition [$this xyzToRAS $newxyz]
              eval $movingCommand
            }
            default {
            }
          }
        }
        "LeftButtonReleaseEvent" {
          set _actionState ""
          $sliceGUI SetGrabID ""
          set _description ""
          eval $movedCommand
        }
      }
    }
  }

  $this highlight
  $this positionActors
  [$sliceGUI GetSliceViewer] RequestRender
}

proc SeedSWidget::ManyWidgetTest { sliceGUI } {

  set s 0
  for { set r -95. } { $r <= 95. } { set r [expr $r + 20] } {
    puts [time {
      for { set a -95. } { $a <= 95. } { set a [expr $a + 20] } {
        set seedSWidget [SeedSWidget #auto $sliceGUI]
        $seedSWidget place $r $a $s
      }
    }]
  }
}

