set directory [file dirname [info script]]/../../../Libs/MRML/Testing
$::slicer3::MRMLScene SetURL $directory/volScene.mrml
$::slicer3::MRMLScene Connect
$::slicer3::MRMLScene SetURL $directory/cube.mrml
$::slicer3::MRMLScene Import
$::slicer3::MRMLScene SetURL $directory/cube.mrml
$::slicer3::MRMLScene Connect
$::slicer3::MRMLScene SetURL $directory/volScene.mrml
$::slicer3::MRMLScene Import
after idle "$::slicer3::Application Exit"
