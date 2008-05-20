update
set scenes {volScene.mrml cube.mrml cube.mrml volScene.mrml}
foreach scene $scenes {
  $::slicer3::MRMLScene SetURL $::Slicer3_HOME/share/MRML/Testing/$scene
  $::slicer3::MRMLScene Connect
  update
}

after idle "$::slicer3::Application Exit"
