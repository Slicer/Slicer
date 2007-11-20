
if {0} {
  source c:/pieper/bwh/slicer3/latest/slicer3/Modules/Tractography/Testing/testglyphs.tcl
}


proc TestSceneClear {} {

  for { set i 0 } { $i < 1 } { incr i } {
    $::slicer3::MRMLScene Clear 0
  }
}

# $::slicer3::Application TraceScript TestSceneClear


proc bounceParam {code min max step} {
  for {set param $min} {$param <= $max} {set param [expr $param + $step]} {
    eval $code $param
    update
  }
  for {set param $max} {$param >= $min} {set param [expr $param - $step]} {
    eval $code $param
    update
  }
}


proc TestGlyphs {} {

  $::slicer3::MRMLScene Clear 0

  set ::gui [$::slicer3::Application GetModuleGUIByName DisplayLoadSave] 
  set ::logic [$::gui GetLogic]

  set testData $::env(SLICER_HOME)/../Slicer3/Modules/Tractography/Testing/TestData/twoFibers.vtk

  set ::fbNode [$::logic AddFiberBundle $testData]

  update

  set ::lineNode [$::fbNode GetLineDisplayNode]
  set ::tubeNode [$::fbNode GetTubeDisplayNode]
  set ::glyphNode [$::fbNode GetGlyphDisplayNode]

  $::tubeNode SetVisibility 1
  $::glyphNode SetVisibility 1
  set ::dtNode [$::glyphNode GetDTDisplayPropertiesNode]

  bounceParam "$::tubeNode SetTubeRadius" 0.5 5.0 0.5
  foreach mode "Lines Tubes Ellipsoids" {
    $::dtNode SetGlyphGeometryTo$mode
    bounceParam "$::dtNode SetGlyphScaleFactor" 50 500 20
  }

}

$::slicer3::Application TraceScript TestGlyphs

