set directory [file dirname [info script]]/../../../Libs/MRML/Testing

update
set scenes {volScene.mrml cube.mrml cube.mrml volScene.mrml}
foreach scene $scenes {
  $::slicer3::MRMLScene SetURL $directory/$scene
  $::slicer3::MRMLScene Connect
  update
}

after idle "$::slicer3::Application Exit"
