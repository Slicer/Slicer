

proc TestFiducial {} {
  set fiducialList [$::slicer3::MRMLScene GetNodeByID vtkMRMLFiducialListNode1] 

  set numberOfFiducials [$fiducialList GetNumberOfFiducials]

  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    $fiducialList SetNthFiducialLabelText $f RenameTest$f
    $fiducialList SetNthFiducialXYZ $f $f $f $f
    $fiducialList SetNthFiducialOrientation $f $f $f $f 1
  }


  for {set f 0} {$f < $numberOfFiducials} {incr f} {
    if { [$fiducialList GetNthFiducialLabelText $f] != "RenameTest$f" } {
      error "fiducial name didn't change ($fiducial)"
    }
    if { [string trim [$fiducialList GetNthFiducialXYZString $f]] != "$f $f $f" } {
      error "fiducial XYZ didn't change ($fiducial) (XYZ is [$fiducial GetXYZ] not $f $f $f)"
    }
    if { [string trim [$fiducialList GetNthFiducialOrientationString $f]] != "$f $f $f 1" } {
      error "fiducial OrientationWXYZ didn't change ($fiducial)"
    }
  }

  puts "fiducial test okay"
  return 0
}


