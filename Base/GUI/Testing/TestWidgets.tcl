
if { [info exists args] } {
  foreach arg $args {
    tk_messageBox -message "arg is $arg"
  }
}

## admin

if { [info command vtkKWTkcon?*] == "" && ![winfo exists .vtkKWTopLevel0] } {
  if { [file exists c:/Tcl/bin/tkcon.tcl] } {
    source c:/Tcl/bin/tkcon.tcl
    tkcon::Init
    tkcon::Attach Main
  }
}

proc vtkNew {class {group "default"}} {
  if { ![info exists ::vtkObjects($group)] } {set ::vtkObjects($group) ""}
  set o [$class New]
  set ::vtkObjects($group) "$o $::vtkObjects($group)"
  return $o
}

proc vtkDelete {{group "default"}} {
  if { ![info exists ::vtkObjects($group)] } {return}
  foreach o $::vtkObjects($group) {
    puts "$o ([$o GetClassName]) refcount [$o GetReferenceCount]"
    $o Delete
  }
  set ::vtkObjects($group) ""
}

proc vtkRefPrint {{group "default"}} {
  if { ![info exists ::vtkObjects($group)] } {return}
  foreach o $::vtkObjects($group) {
    puts "$o ([$o GetClassName]) refcount [$o GetReferenceCount]"
  }
}

proc createWidget {rwi widgetType} {

  switch $widgetType {

    "User" {
      set ::userRep [vtkNew vtkUserRepresentation widgets]
      set ::userWidget [vtkNew vtkUserWidget widgets]
      $::userWidget SetInteractor $rwi
      $::userWidget SetRepresentation $::userRep
      $::userWidget On
    }

    "Affine" {
      set affineRep [vtkNew vtkAffineRepresentation2D widgets]
      set affineWidget [vtkNew vtkAffineWidget widgets]
      $affineWidget SetInteractor $rwi
      $affineWidget SetRepresentation $affineRep
      $affineWidget On

      set affineRep2 [vtkNew vtkAffineRepresentation2D widgets]
      set affineWidget2 [vtkNew vtkAffineWidget widgets]
      $affineWidget2 SetInteractor $rwi
      $affineWidget2 SetRepresentation $affineRep2
      $affineWidget2 On
    }
    "Box" {
      set ::boxWidget [vtkNew vtkBoxWidget widgets]
      $::boxWidget SetInteractor $rwi

      $::boxWidget SetRotationEnabled 0
      $::boxWidget SetScalingEnabled 0

      $::boxWidget On
    }
    "Contour" {
      set contourWidget [vtkNew vtkContourWidget widgets]
      $contourWidget SetInteractor $rwi

      $contourWidget On
    }
    "Distance" {
      set pointRep [vtkNew vtkPointHandleRepresentation2D widgets]
      [$pointRep GetProperty] SetColor 1 0 0 
      
      set distRep [vtkNew vtkDistanceRepresentation2D widgets]
      $distRep SetHandleRepresentation $pointRep

      set distWidget [vtkNew vtkDistanceWidget widgets]
      $distWidget SetRepresentation $distRep
      $distWidget SetInteractor $rwi

      $distWidget On
    }
    "Seed" {
      # Seed
      set pointRep [vtkNew vtkPointHandleRepresentation2D widgets]
      #[$pointRep GetProperty] SetColor 1 0 0 
      
      set seedRep [vtkNew vtkSeedRepresentation widgets]
      $seedRep SetHandleRepresentation $pointRep

      set seedWidget [vtkNew vtkSeedWidget widgets]
      $seedWidget SetRepresentation $seedRep
      $seedWidget SetInteractor $rwi

      $seedWidget On
    }
    "BiDimensional" {
      set biRep [vtkNew vtkBiDimensionalRepresentation2D widgets]
      
      set biWidget [vtkNew vtkBiDimensionalWidget widgets]
      $biWidget SetRepresentation $biRep
      $biWidget SetInteractor $rwi

      $biWidget On
    }
    "Caption" {
      set capRep [vtkNew vtkCaptionRepresentation widgets]

      [$capRep GetCaptionActor2D] SetCaption "Where in Slicer\nwill we use this?"
      [[[$capRep GetCaptionActor2D] GetTextActor] GetTextProperty] SetJustificationToCentered
      [[[$capRep GetCaptionActor2D] GetTextActor] GetTextProperty] SetVerticalJustificationToCentered

      set capWidget [vtkNew vtkCaptionWidget widgets]
      $capWidget SetRepresentation $capRep
      $capWidget SetInteractor $rwi

      $capWidget On
    }
  }
}

## kwwidgets 

set slicerApp [vtkNew vtkSlicerApplication]

set topLevel [vtkNew vtkKWTopLevel]
$topLevel SetApplication $slicerApp
$topLevel Create

set pushButton [vtkNew vtkKWPushButton]
$pushButton SetParent $topLevel
$pushButton SetText "Quit"
set ::quit 0
proc WidgetQuit {} {set ::quit 1}
$pushButton SetCommand $slicerApp "Evaluate WidgetQuit"
$pushButton Create
pack [$pushButton GetWidgetName]

set renderWidget [vtkNew vtkKWRenderWidget]
$renderWidget SetParent $topLevel
$renderWidget Create
pack [$renderWidget GetWidgetName] -fill both -expand true
$topLevel Display
$renderWidget AddObserver DeleteEvent "puts BYErenderWidget"


set rwi [[$renderWidget GetRenderWindow] GetInteractor]

#### wwidget code

# Logo
set logoRep [vtkNew vtkLogoRepresentation]
set ellip [vtkNew vtkImageEllipsoidSource]
$logoRep SetImage [$ellip GetOutput]

set logoWidget [vtkNew vtkLogoWidget]
$logoWidget SetInteractor $rwi
$logoWidget SetRepresentation $logoRep

$logoWidget On


set widgets { "User" "Affine" "Box" "Contour" "Distance" "Seed" "BiDimensional" "Caption" }

foreach w $widgets {
  set pushButton [vtkNew vtkKWPushButton]
  $pushButton SetParent $topLevel
  $pushButton SetText "$w"
  $pushButton SetCommand $slicerApp "Evaluate \"createWidget $rwi $w\""
  $pushButton Create
  pack [$pushButton GetWidgetName]
}


vwait ::quit

vtkRefPrint 

## cleanup 

$slicerApp Exit

vtkDelete widgets
vtkDelete

exit


