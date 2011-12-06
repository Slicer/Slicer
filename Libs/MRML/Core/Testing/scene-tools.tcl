
proc MRMLScenePrint {} {
 
  set nNodes [$::slicer3::MRMLScene GetNumberOfNodes]

  for {set i 0} {$i < $nNodes} {incr i} {
    set node [$::slicer3::MRMLScene GetNthNode $i]
    puts "$node [$node GetID] [$node GetClassName]"
  }
}
