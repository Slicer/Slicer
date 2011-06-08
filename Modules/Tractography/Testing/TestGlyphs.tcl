
set ::Tractography_Exit_After_Script 1
if {0} {
  source c:/pieper/bwh/slicer3/latest/slicer3/Modules/Tractography/Testing/testglyphs.tcl
  set ::Tractography_Exit_After_Script 0
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


proc SetupGlyphs {} {

  $::slicer3::MRMLScene Clear 0

  set ::gui [$::slicer3::Application GetModuleGUIByName FiberBundles] 
  if {$::gui == ""} {
      puts "Cannot find FibreBundles module"
      return 1
  }
  set ::seed [$::slicer3::Application GetModuleGUIByName FiducialSeeding]
  if {$::seed == ""} {
    puts "Cannot find FiducialSeeding module"
    return 1
  }
  $::seed OverwritePolyDataWarningOff

  set ::logic [$::gui GetLogic]

  # TODO: $::env(SLICER_HOME)/share/Slicer/Modules should be avoided, since
  # the module could have been loaded from the user module paths (see
  # vtkSlicerApplication::GetModulePaths), therefore its testing data are
  # not inside env(SLICER_HOME). Fix this by using 
  # vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the Tractography
  # module reference, its logic, and call this method to retrive the full
  # path to the share/ directory for this module)

  set testData $::env(SLICER_HOME)/share/Slicer/Modules/Tractography/Testing/TestData/twoFibers.vtk
  #set testData c:/data/tracts/Seeding1.vtp

  set ::fbNode [$::logic AddFiberBundle $testData 1]

  update

  set ::lineNode [$::fbNode GetLineDisplayNode]
  set ::tubeNode [$::fbNode GetTubeDisplayNode]
  set ::glyphNode [$::fbNode GetGlyphDisplayNode]

  return 0
}

proc TestProperties {} {
  $::tubeNode SetVisibility 1
  $::glyphNode SetVisibility 1
  set ::dtNode [$::glyphNode GetDiffusionTensorDisplayPropertiesNode]

  bounceParam "$::tubeNode SetTubeRadius" 0.5 5.0 0.5
  foreach mode "Lines Tubes Ellipsoids" {
    $::dtNode SetGlyphGeometryTo$mode
    bounceParam "$::dtNode SetGlyphScaleFactor" 50 500 20
  }
}

proc TestColor { color } {

  $::lineNode SetVisibility 1
  $::glyphNode SetVisibility 1

  set ::glyphDTNode [$::glyphNode GetDiffusionTensorDisplayPropertiesNode]
  $::glyphDTNode SetGlyphGeometryToEllipsoids

  $::lineNode SetAndObserveColorNodeID "vtkMRMLColorTableNode$color"
  set camera [$::slicer3::MRMLScene GetNthNodeByClass 0 "vtkMRMLCameraNode"]
  if { $camera != "" } {
    bounceParam "$camera SetPosition 0 500 " 0 100 10
  }
}

set retval [SetupGlyphs]

if {$retval == 1} {
    puts "Error setting up glyphs"
    # for now, reset the retval since it's succeeded at testing if this test could be run or not
    set retval 0
} else {


MRMLWatcher #auto


$::slicer3::Application TraceScript TestProperties
$::slicer3::Application TraceScript "TestColor Rainbow"
$::slicer3::Application TraceScript "TestColor Ocean"

}
if { $::Tractography_Exit_After_Script } {
    if {$retval == 1} {
       exit 1
    } else {
       exit 0
    }
}
