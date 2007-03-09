# source ../slicer3/Modules/Editor/Testing/TestVolume.tcl; EditorMakeTestVolume c:/tmp/test.nrrd
proc EditorMakeTestVolume {filename {size 16} } {

  set fp [open $filename "w"]

  puts $fp "NRRD0004"
  puts $fp "content: testvolume"
  puts $fp "type: double"
  puts $fp "dimension: 3"
  puts $fp "sizes: $size $size $size"
  puts $fp "centers: cell cell cell"
  puts $fp "space: RAS"
  puts $fp "space directions: (1,0,0) (0,1,0) (0,0,1)"
  puts $fp "encoding: ascii"
  puts $fp ""

  EditorWriteStripedPixels $fp $size
  #EditorWriteIndexedPixels $fp $size

  close $fp
}


proc EditorWriteStripedPixels {fp size} {

  for {set k 0} {$k < $size} {incr k} {
    for {set j 0} {$j < $size} {incr j} {
      for {set i 0} {$i < $size} {incr i} {
        set pixel [expr  \
            ($k % 2) * 10000 + 1000 * $j + $i]
        puts $fp $pixel
      }
    }
  }
}

proc EditorWriteIndexedPixels {fp size} {

  for {set k 0} {$k < $size} {incr k} {
    for {set j 0} {$j < $size} {incr j} {
      for {set i 0} {$i < $size} {incr i} {
        set pixel [expr  \
                1000000 +  \
            $k * 100000 +  \
            $j *   1000 + $i]
        puts $fp $pixel
      }
    }
  }
}
