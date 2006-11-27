
#
# test of importing a slicer2 mrml scene using the 
# vtk xml data parser mechanism
#

#######
#
# for debugging - run the command when the script is read...
#
after idle {
  ImportSlicer2Scene c:/data/tutorial/tutorial.xml
  [$viewer GetMainViewer] Reset
}

# main entry point...
proc ImportSlicer2Scene {sceneFile} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $sceneFile
  $parser Parse

  set root [$parser GetRootElement]

  set ::S2(dir) [file dirname [file normalize $sceneFile]]
  set ::S2(transformIDStack) ""
  ImportElement $root

  $parser Delete
}

#
# recursive routine to import all elements and their
# nested parts
#
proc ImportElement {element} {
  
  ImportNode $element

  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    ImportElement $nestElement
  }
}

#
# parse the attributes of a node into a tcl array
# and then invoke the type-specific handler
#
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

#####
#
# handler procs for each type of node
#

proc ImportNodeMRML {node} {
  # no op
  puts MRML
}

proc ImportNodeTransform {node} {

  set transformNode [vtkMRMLLinearTransformNode New]
  set matrix [$transformNode GetMatrixTransformToParent]
  $::slicer3::MRMLScene AddNode $transformNode
  
  lappend ::S2(transformIDStack) [$transformNode GetID]
}

proc ImportNodeMatrix {node} {
  upvar $node n

  if 0 {
  $matrix Identity
  $matrix SetElement 0 3  6
  $matrix SetElement 1 3  13
  $matrix SetElement 2 3  13
  }

  #parray n
}

proc ImportNodeVolume {node} {
  upvar $node n

  if { [info exists n(fileType)] && $n(fileType) == "Archetype" } {
    puts stderr "Archetype nodes not yet supported!"
  } else {

    parray n

    if { ![info exists n(Dimensions)] } {
      set n(Dimensions) "256 256"
    }
    if { ![info exists n(scalarType)] } {
      set n(scalarType) "Short"
    }

    set imageReader [vtkImageReader New]
    $imageReader SetFileDimensions 

    if { [file pathtype $n(filePrefix)] == "relative" } {
      $imageReader SetFilePrefix  $::S2(dir)/$n(filePrefix)
    } else {
      $imageReader SetFilePrefix  $n(filePrefix)
    }
    $imageReader SetFilePattern  $n(filePattern)

    foreach {w h} $n(Dimensions) {}
    foreach {zlo zhi} $n(imageRange) {}
    set d [expr $zhi - $zlo]
    $imageReader SetDataExtent $w $h $d
    $imageReader SetFileNameSliceOffset $zlo
    $imageReader SetDataScalarTypeTo$n(scalarType)

    

    


n(applyThreshold) = yes
n(colorLUT)       = 0
n(description)    = LR
n(filePattern)    = %s.%03d
n(filePrefix)     = spgr/I
n(imageRange)     = 1 124
n(level)          = 76
n(lowerThreshold) = 17
n(name)           = SPGR
n(positionMatrix) = 0 0 1 -89.95 -1 0 0 133.3 0 1 0 -148 0 0 0 1
n(rasToIjkMatrix) = 0 -1.06667 0 142.187 0 0 -1.06667 98.1333 0.666667 0 0 59.9667 0 0 0 1
n(rasToVtkMatrix) = 0 -1.06667 0 142.187 0 0 1.06667 157.867 0.666667 0 0 59.9667 0 0 0 1
n(upperThreshold) = 355
n(window)         = 146

    $imageReader Delete


  }


  if { 0 } {
    set fileName [file dirname $::QA(directory)]/sirp-hp65-stc-to7-gam.feat/stats/zstat8.nii

    set volumeNode [$volumesLogic AddArchetypeVolume $fileName $centered 0 zstat8]
    $volumeNode SetAndObserveTransformNodeID [$transformNode GetID]
    set ::QA(functional,volumeNodeID) [$volumeNode GetID]
    set volumeDisplayNode [$volumeNode GetDisplayNode]
    $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorNodeIron"
    $volumeDisplayNode SetWindow 3.3
    $volumeDisplayNode SetLevel 3
    $volumeDisplayNode SetUpperThreshold 6.8
    $volumeDisplayNode SetLowerThreshold 1.34
    $volumeDisplayNode SetApplyThreshold 1
  }

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


