

proc TestFiducial {} {
  set fiducialList [$::slicer3::MRMLScene GetNodeByID vtkMRMLFiducialListNode1] 

  set numberOfFiducials [$fiducialList GetNumberOfFiducials]

  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    set fiducial [$fiducialList GetNthFiducial $f]
    $fiducial SetLabelText RenameTest$f
    $fiducial SetXYZ $f $f $f
    $fiducial SetOrientationWXYZ $f $f $f 1
  }


  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    set fiducial [$fiducialList GetNthFiducial $f]
    if { [$fiducial GetLabelText] != "RenameTest$f" } {
      error "fiducial name didn't change ($fiducial)"
    }
    if { [string trim [$fiducial GetXYZ]] != "$f $f $f" } {
      error "fiducial XYZ didn't change ($fiducial) (XYZ is [$fiducial GetXYZ] not $f $f $f)"
    }
    if { [string trim [$fiducial GetOrientationWXYZ]] != "$f $f $f 1" } {
      error "fiducial OrientationWXYZ didn't change ($fiducial)"
    }
  }

  puts "fiducial test okay"
  return 0
}


