# source ../slicer3/Modules/Editor/Testing/TestVolume.tcl; EditorMakeTestVolume c:/tmp/test.nrrd
proc EditorMakeTestVolume {filename} {

  set fp [open $filename "w"]

  puts $fp "NRRD0004"
  puts $fp "content: testvolume"
  puts $fp "type: double"
  puts $fp "dimension: 3"
  puts $fp "sizes: 32 32 32"
  puts $fp "centers: cell cell cell"
  puts $fp "space: RAS"
  puts $fp "space directions: (1,0,0) (0,1,0) (0,0,1)"
  puts $fp "encoding: ascii"
  puts $fp ""

  EditorWriteStripedPixels $fp

  close $fp
}


proc EditorWriteStripedPixels {fp} {

  for {set k 0} {$k < 32} {incr k} {
    for {set j 0} {$j < 32} {incr j} {
      for {set i 0} {$i < 32} {incr i} {
        set pixel [expr  \
            ($k % 2) * 10000 + 1000 * $j + $i]
        puts $fp $pixel
      }
    }
  }
}

proc EditorWriteIndexedPixels {fp} {

  for {set k 0} {$k < 32} {incr k} {
    for {set j 0} {$j < 32} {incr j} {
      for {set i 0} {$i < 32} {incr i} {
        set pixel [expr  \
                1000000 +  \
            $k * 100000 +  \
            $j *   1000 + $i]
        puts $fp $pixel
      }
    }
  }
}
