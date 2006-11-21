
#
# test of importing a slicer2 mrml scene using the 
# vtk xml data parser mechanism
#


proc ImportSlicer2Scene {sceneFile} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $sceneFile
  $parser Parse

  set root [$parser GetRootElement]

  set ::S2(dir) [file dirname [file normalize $sceneFile]]
  set ::S2(stack) ""
  ImportElement $root
}

proc ImportElement {element} {
  
  ImportNode $element

  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    ImportElement $nestElement
  }
}

proc ImportNode {element} {

  set nAtts [$element GetNumberOfAttributes]
  for {set i 0} {$i < $nAtts} {incr i} {
    set attName [$element GetAttributeName $i]
    set node($attName) [$element GetAttributeValue $i]
  } 

  set nodeType [$element GetName]
  set handler ImportNode$nodeType
  if { [info command $handler] == "" } {
    error "no handler for $nodeType"
  }


  # call the handler for this element
  $handler node
}

proc ImportNodeMRML {node} {
  # no op
  puts MRML
}

proc ImportNodeTransform {node} {
}

proc ImportNodeMatrix {node} {
  upvar $node n

  #parray n
}

proc ImportNodeVolume {node} {
  upvar $node n

  #parray n
}

proc ImportNodeModel {node} {
  upvar $node n

  set modelNode [vtkMRMLModelNode New]
  set modelStorageNode [vtkMRMLModelStorageNode New]
  set modelDisplayNode [vtkMRMLModelDisplayNode New]

  $modelNode SetName $n(name)
  if { [file pathtype $n(fileName)] == "relative" } {
    $modelStorageNode SetFileName $::S2(dir)/$n(fileName)
  } else {
    $modelStorageNode SetFileName $n(fileName)
  }

  if { [$modelStorageNode ReadData $modelNode] != 0 } {

    $::slicer3::MRMLScene AddNode $modelStorageNode
    $::slicer3::MRMLScene AddNode $modelDisplayNode

    $modelNode SetStorageNodeID [$modelStorageNode GetID]
    $modelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]

    $::slicer3::MRMLScene AddNode $modelNode

  } else {
    puts stderr "warning: could not read data for model node from $n(fileName)"
  }

  $modelNode Delete
  $modelStorageNode Delete
  $modelDisplayNode Delete
}

proc ImportNodeOptions {node} {
  puts stderr "warning: option nodes cannot be imported"
}

ImportSlicer2Scene c:/data/tutorial/tutorial.xml

