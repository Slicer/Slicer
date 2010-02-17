
proc TestRestoreSceneSnapshotCompareView {} {
    # load the scene
    $::slicer3::MRMLScene SetURL $::Slicer3_HOME/share/MRML/Testing/restoreSceneSnapShotCompareView.mrml
    $::slicer3::MRMLScene Connect

    # restore the snapshot
    set nodes [$::slicer3::MRMLScene GetNodesByName view1]
    $nodes InitTraversal
    set nd [$nodes GetNextItemAsObject]
    $::slicer3::MRMLScene SaveStateForUndo
    $nd RestoreScene

    exit 0
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
