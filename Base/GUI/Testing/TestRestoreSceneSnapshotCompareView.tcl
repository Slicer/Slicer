
proc TestRestoreSceneSnapshotCompareView {} {
    # load the scene
    $::slicer3::MRMLScene SetURL $::SlicerHome/share/MRML/Testing/restoreSceneSnapshotCompareView.mrml
    $::slicer3::MRMLScene Connect

    # restore the snapshot
    set node [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLSceneSnapshotNode"]
    $::slicer3::MRMLScene SaveStateForUndo
    $node RestoreScene
}

proc runtest {} {
    set ret [ catch {
         $::slicer3::Application TraceScript TestRestoreSceneSnapshotCompareView
    } res]

 if { $ret } {
   puts stderr $res
   exit 1
 }
 exit 0
}

update
after idle runtest
