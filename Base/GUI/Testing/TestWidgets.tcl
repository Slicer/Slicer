
foreach arg $args {
  tk_messageBox -message "arg is $arg"
}

## admin

if { [file exists c:/Tcl/bin/tkcon.tcl] } {
  source c:/Tcl/bin/tkcon.tcl
  tkcon::Init
  tkcon::Attach Main
}

set ::vtkObjects ""

proc vtkNew {class} {
  set o [$class New]
  set ::vtkObjects "$o $::vtkObjects"
  return $o
}

proc vtkDelete {} {
  foreach o $::vtkObjects {
    $o Delete
  }
  set ::vtkObjects ""
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

  set widgetType "Affine"

  switch $widgetType {

    "Affine" {
      set affineRep [vtkNew vtkAffineRepresentation2D]

      set affineWidget [vtkNew vtkAffineWidget]
      $affineWidget SetInteractor $rwi
      $affineWidget SetRepresentation $affineRep

      $affineWidget On
    }
    "Contour" {
      set contourWidget [vtkNew vtkContourWidget]
      $contourWidget SetInteractor $rwi

      $contourWidget On
    }
    "Distance" {
      set pointRep [vtkNew vtkPointHandleRepresentation2D]
      [$pointRep GetProperty] SetColor 1 0 0 
      
      set distRep [vtkNew vtkDistanceRepresentation2D]
      $distRep SetHandleRepresentation $pointRep

      set distWidget [vtkNew vtkDistanceWidget]
      $distWidget SetRepresentation $distRep
      $distWidget SetInteractor $rwi

      $distWidget On
    }
    "Seed" {
      # Seed
      set pointRep [vtkNew vtkPointHandleRepresentation2D]
      [$pointRep GetProperty] SetColor 1 0 0 
      
      set seedRep [vtkNew vtkSeedRepresentation]
      $seedRep SetHandleRepresentation $pointRep

      set seedWidget [vtkNew vtkSeedWidget]
      $seedWidget SetRepresentation $seedRep
      $seedWidget SetInteractor $rwi

      $seedWidget On
    }
    "BiDimensional" {
      set biRep [vtkNew vtkBiDimensionalRepresentation2D]
      
      set biWidget [vtkNew vtkBiDimensionalWidget]
      $biWidget SetRepresentation $biRep
      $biWidget SetInteractor $rwi

      $biWidget On
    }
    "Caption" {
      set capRep [vtkNew vtkCaptionRepresentation]

      [$capRep GetCaptionActor2D] SetCaption "Where in Slicer\nwill we use this?"
      [[[$capRep GetCaptionActor2D] GetTextActor] GetTextProperty] SetJustificationToCentered
      [[[$capRep GetCaptionActor2D] GetTextActor] GetTextProperty] SetVerticalJustificationToCentered

      set capWidget [vtkNew vtkCaptionWidget]
      $capWidget SetRepresentation $capRep
      $capWidget SetInteractor $rwi

      $capWidget On
    }
  }

vwait ::quit

## cleanup 


$slicerApp Exit

vtkDelete

exit


