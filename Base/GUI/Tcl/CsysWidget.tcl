
if { [info command vtkNew] == "" } {
  proc vtkNew {class {group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {set ::vtkObjects($group) ""}
    set o [$class New]
    set ::vtkObjects($group) "$o $::vtkObjects($group)"
    return $o
  }
}

if { [info command vtkDelete] == "" } {
  proc vtkDelete {{group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {return}
    foreach o $::vtkObjects($group) {
      puts "$o ([$o GetClassName]) refcount [$o GetReferenceCount]"
      $o Delete
    }
    set ::vtkObjects($group) ""
  }
}

if { [info command vtkExists] == "" } {
  proc vtkExists { o {group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {return 0}
    if { [lsearch $::vtkObjects($group) $o] == -1 } {
      return 0
    } else {
      return 1
    }
  }
}

proc CsysWidgetRemove {} {

  set sliceGUIs [vtkSlicerSliceGUI ListInstances]
  foreach sliceGUI $sliceGUIs {
    set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
    CsysWidgetDelete $sliceGUI $renderWidget
  }
}

proc CsysWidgetAdd {} {

  CsysWidgetRemove

  set sliceGUIs [vtkSlicerSliceGUI ListInstances]
  foreach sliceGUI $sliceGUIs {
    set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
    CsysWidgetCreate $sliceGUI $renderWidget
  }
}

proc CsysWidgetCreate { tag renderWidget } {

  set size [[$renderWidget GetRenderWindow]  GetSize]
  foreach {w h} $size {}
  foreach d {w h} c {cx cy} { set $c [expr [set $d] / 2.0] }


  set interactorStyle [[$renderWidget GetRenderWindowInteractor] GetInteractorStyle]
 
  set sphere [vtkNew vtkSphereSource $tag]
  $sphere SetRadius 5
  $sphere SetCenter $cx $cy 0
  set mapper [vtkNew vtkPolyDataMapper2D $tag]
  set actor [vtkNew vtkActor2D $tag]
  $mapper SetInput [$sphere GetOutput]
  $actor SetMapper $mapper

  [$renderWidget GetRenderer] AddActor2D $actor

  foreach l {1 2} {
    set leader($l) [vtkNew vtkLeaderActor2D $tag]
    $leader($l) SetArrowPlacementToBoth
    $leader($l) SetMaximumArrowSize 7
    set leaderMapper($l) [vtkNew vtkPolyDataMapper2D $tag]
    $leader($l) SetMapper $leaderMapper($l)
    [$renderWidget GetRenderer] AddActor2D $leader($l)
  }

  $leader(1) SetPosition  [expr ($cx - 15) / $w] [expr $cy / $h]
  $leader(1) SetPosition2 [expr ($cx + 15) / $w] [expr $cy / $h]
  $leader(2) SetPosition  [expr $cx / $w] [expr ($cy - 15) / $h]
  $leader(2) SetPosition2 [expr $cx / $w] [expr ($cy + 15) / $h]
  
  $renderWidget Render

}

CsysWidgetUpdate { tag renderWidget } {

}

proc CsysWidgetDelete { tag renderWidget } {
  # todo - remove actors
  set actors [[$renderWidget GetRenderer] GetActors2D]
  $actors InitTraversal
  while { 1 } {
    set actor [$actors GetNextActor2D]
    if { $actor == "" } { break }
    if { [vtkExists $actor $tag] } {
      [$renderWidget GetRenderer] RemoveActor2D $actor
    }
  }
  vtkDelete $tag
}
