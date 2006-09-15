

proc TestFiducial {} {
  set fiducialList [$::slicer3::MRMLScene GetNodeByID vtkMRMLFiducialListNode1] 

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
    if { [string trim [$fiducialList GetNthFiducialXYZString $f]] != "$tenf $tenf $tenf" } {
      error "fiducial XYZ didn't change ($f) (XYZ is [$fiducialList GetNthFiducialXYZString $f] not $tenf $tenf $tenf)"
    }
    if { [string trim [$fiducialList GetNthFiducialOrientationString $f]] != "$f $f $f 1" } {
        error "fiducial OrientationWXYZ didn't change ($f) (WXYZ is [$fiducialList GetNthFiducialOrientationString $f] not $f $f $f 1)"
    }
  }

  puts "fiducial test okay"
  return 0
}


