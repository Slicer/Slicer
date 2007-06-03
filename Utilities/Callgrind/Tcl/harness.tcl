
set __comment__ {

  start with 
    valgrind --tool=callgrind --instr-atstart=no --collect-atstart=no --dump-instr=yes --trace-jump=yes --separate-threads=yes bin/Slicer3-real --script /home/pieper/slicer3/profiling/Slicer3/Utilities/Callgrind/Tcl/harness.tcl
  or
    valgrind --tool=callgrind --dump-instr=yes --trace-jump=yes --separate-threads=yes bin/Slicer3-real --script /home/pieper/slicer3/profiling/Slicer3/Utilities/Callgrind/Tcl/harness.tcl

}

proc harness {code} {

  puts [exec callgrind_control -z]
  update; after 1000
  puts [exec callgrind_control -i on]
  update; after 1000
  puts [exec callgrind_control -b]
  update; after 1000
  puts "running: $code"
  update; after 1000
  eval $code
  update; after 1000
  puts [exec callgrind_control -d]
  update; after 1000
  puts [exec callgrind_control -i off]
  update; after 1000
  puts "finished profiling"
}

proc h1 {} {

  harness "$::slicer3::MRMLScene Modified"
}

proc h2 {} {

  harness h2_body
}

proc h2_body {} {
  catch "sc Delete"
  vtkMRMLScene sc

  #sc SetURL C:/alexy/slicer2-head/Slicer3/Libs/MRML/Testing/volScene3.xml
  sc SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/volScene3.xml
  sc Connect

  puts "GetNumberOfNodesByClass [sc GetNumberOfNodesByClass vtkMRMLScalarVolumeNode]"
  puts "GetNodeClasses [sc GetNodeClasses]"
  puts "GetNthNode"

  set v0 [sc GetNthNodeByClass 0 vtkMRMLScalarVolumeNode]
  puts "Print volume node 0"
  puts "[$v0 Print]"

  set v1 [sc GetNthNodeByClass 1 vtkMRMLScalarVolumeNode]
  puts "Print volume node 1"
  puts "[$v1 Print]"


  set n [sc GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]
  puts "Print Storage node 0"
  puts "[$n Print]"

  set n [sc GetNthNodeByClass 1 vtkMRMLVolumeArchetypeStorageNode]
  puts "Print Storage node 1"
  puts "[$n Print]"

  set n [sc GetNthNodeByClass 0 vtkMRMLVolumeDisplayNode]
  puts "Print Display node 0"
  puts "[$n Print]"

  set n [sc GetNthNodeByClass 0 vtkMRMLLinearTransformNode]
  puts "Print Transform node 0"
  puts "[$n Print]"

  set t0 [$v0 GetParentTransformNode]
  puts "Print Transform for volume 0"
  puts "[$t0 Print]"

  set t1 [$v1 GetParentTransformNode]
  puts "Print Transform for volume 1"
  puts "[$t1 Print]"

  set m0 [$t0 GetMatrixTransformToParent]
  puts "Print Matrix for transform node 0"
  puts "[$m0 Print]"

  set m1 [$t1 GetMatrixTransformToParent]
  puts "Print Matrix for transform node 1"
  puts "[$m1 Print]"

  catch "mm0 Delete"
  vtkMatrix4x4 mm0
  mm0 Identity
  $t0 GetMatrixTransformToWorld mm0
  puts "Print GetMatrixTransformToWorld transform node 0"
  puts "[mm0 Print]"

  catch "mm1 Delete"
  vtkMatrix4x4 mm1
  mm1 Identity
  $t1 GetMatrixTransformToWorld mm1
  puts "Print GetMatrixTransformToWorld transform node 1"
  puts "[mm1 Print]"

  mm0 Identity
  $t0 GetMatrixTransformToNode $t1 mm0
  puts "Print GetMatrixTransformToNode from 0 to 1"
  puts "[mm0 Print]"

  mm0 Delete
  mm1 Delete
  sc Delete
}
