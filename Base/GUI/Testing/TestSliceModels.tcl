
proc SliceModelsRemove {} {

  set nNodes [$::slicer3::MRMLScene GetNumberOfNodes]
  for {set n [expr $nNodes - 1]} {$n >= 0} {incr n -1} {
    set node [$::slicer3::MRMLScene GetNthNode $n]
    if { [string match "SliceModel*" [$node GetDescription]]  } {
      $::slicer3::MRMLScene RemoveNode $node
      #node Delete ;# ????
    }
  }
}

proc SliceModelsAdd {} {

  set sliceGUIs [vtkSlicerSliceGUI ListInstances]

  set sliceModelIndex 0
  foreach sliceGUI $sliceGUIs {

    set modelNode [vtkMRMLModelNode New]
    $modelNode SetName "SliceModel$sliceModelIndex "
    incr sliceModelIndex
    $modelNode SetDescription "SliceModel"

    set plane [vtkPlaneSource New]
    [$plane GetOutput] Update
    $modelNode SetAndObservePolyData [$plane GetOutput]
    $plane Delete

    $::slicer3::MRMLScene AddNode $modelNode
  
    set sliceLogic [$sliceGUI GetLogic]

    SliceModelsUpdate $sliceLogic $modelNode
    $sliceLogic AddObserver ModifiedEvent "SliceModelsUpdate $sliceLogic $modelNode"
  }
}

proc SliceModelsUpdate {sliceLogic modelNode} {

  set sliceNode [$sliceLogic GetSliceNode]
  set polyData [$modelNode GetPolyData]
  set points [$polyData GetPoints]
  foreach {w h d} [$sliceNode GetDimensions] {}
  
  set pts [list [list 0 0 0 1] [list $w 0 0 1] [list 0 $h 0 1] [list $w $h 0 1]]
  set xyToRAS [$sliceNode GetXYToRAS]
  for {set i 0} {$i < 4} {incr i} {
    set pt [lrange [eval $xyToRAS MultiplyPoint [lindex $pts $i]] 0 2]
    eval $points SetPoint $i $pt
  }

  set viewerWidget [lindex [vtkSlicerViewerWidget ListInstances] 0]
  set actor [$viewerWidget GetActorByID [$modelNode GetID]]

  if { [$actor GetTexture] == "" } {
    set texture [vtkTexture New]
    $actor SetTexture $texture
  }
  [$actor GetTexture] SetInput [$sliceLogic GetImageData]
  [$actor GetProperty] SetAmbient 1
  [$actor GetProperty] SetDiffuse 0

  $polyData Modified
}
