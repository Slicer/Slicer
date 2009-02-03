

proc TestFiducial {} {

  set fiducialList [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLFiducialListNode"]
  set numberOfFiducials [$fiducialList GetNumberOfFiducials]

  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    set tenf [expr $f * 10]
    $fiducialList SetNthFiducialLabelText $f RenameTest$f
    $fiducialList SetNthFiducialXYZ $f $tenf $tenf $tenf
    $fiducialList SetNthFiducialOrientation $f $f $f $f 1
  }


  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    set tenf [expr $f * 10]
    if { [$fiducialList GetNthFiducialLabelText $f] != "RenameTest$f" } {
      error "fiducial name didn't change ($f)"
    }
    foreach coord [$fiducialList GetNthFiducialXYZ $f] {
      if { [expr $coord != $tenf] } {
        error "fiducial XYZ didn't change ($f) ($coord is not $tenf)"
      }
    } 
    foreach coord [lrange [$fiducialList GetNthFiducialOrientation $f] 0 2] {
      if { [expr $coord != $f] } {
        error "fiducial orientation didn't change ($f) ($coord is not $f)"
      }
    } 
    update
  }

  puts "fiducial test okay"
  return 0
}


proc CreateFiducials {} {

  foreach f { "0 0 0"
              "10 0 0"
              "10 10 0"
              "0 10 0"
              "50 10 0"
              "50 50 0"
              "10 50 0" } {
    eval FiducialsSWidget::AddFiducial $f
    update
  }
}


proc runtest {} {
 set ret [ catch {
   $::slicer3::Application TraceScript CreateFiducials
   $::slicer3::Application TraceScript TestFiducial
 } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update 
after idle runtest
