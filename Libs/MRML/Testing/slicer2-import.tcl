
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
  
  # import this element
  ImportNode $element

  # leave a place holder in case we are a group (transform) node
  lappend ::S2(transformIDStack) "NestingMarker"

  # process all the sub nodes, which may include a sequence of matrices
  # and/or nested transforms
  set nNested [$element GetNumberOfNestedElements]
  for {set i 0} {$i < $nNested} {incr i} {
    set nestElement [$element GetNestedElement $i]
    ImportElement $nestElement
  }

  # strip away any accumulated transform ids
  while { $::S2(transformIDStack) != "" && [lindex $::S2(transformIDStack) end] != "NestingMarker" } {
    set ::S2(transformIDStack) [lrange $::S2(transformIDStack) 0 end-1]
  }
  # strip away the nesting marker
  set ::S2(transformIDStack) [lrange $::S2(transformIDStack) 0 end-1]
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

  # no op - handled by ImportElement
  puts Transform
}

#
# a slicer2 matrix corresponds to a slicer3 Transform
#
proc ImportNodeMatrix {node} {
  upvar $node n

  set transformNode [vtkMRMLLinearTransformNode New]

  set matrix [$transformNode GetMatrixTransformToParent]
  $transformNode SetName $n(name)
  eval $matrix DeepCopy $n(matrix)

  $::slicer3::MRMLScene AddNode $transformNode

  set parentTransform ""
  set index [expr [llength $::S2(transformIDStack)] - 1]
  for {} { $index > 0 } { incr index -1 } {
    set element [lindex $::S2(transformIDStack) $index]
    if { $element != "NestingMarker" } {
      set parentTransform $element
      break
    }
  }
  $transformNode SetAndObserveTransformNodeID $parentTransform

  lappend ::S2(transformIDStack) [$transformNode GetID]

  $transformNode Delete
}


proc ImportNodeVolume {node} {
  upvar $node n

  if { [info exists n(fileType)] && $n(fileType) == "Archetype" } {
    puts stderr "Archetype nodes not yet supported!"
  } else {

    #
    # first, parse the slicer2 node
    #
    if { ![info exists n(Dimensions)] } {
      set n(Dimensions) "256 256"
    }

    if { ![info exists n(scalarType)] } {
      set n(scalarType) "Short"
    }

    if { ![info exists n(littleEndian)] } {
      set n(littleEndian) "false"
    }
    if { $n(littleEndian) } {
      set fileLittleEndian 1
    } else {
      set fileLittleEndian 0
    }
    if { $::tcl_platform(byteOrder) == "littleEndian" } {
      set platformLittleEndian 1
    } else {
      set platformLittleEndian 0
    }
    if { $fileLittleEndian != $platformLittleEndian } {
      set swap 1
    } else {
      set swap 0
    }

    #
    # next, read the image data
    #
    set imageReader [vtkImageReader New]

    if { [file pathtype $n(filePrefix)] == "relative" } {
      $imageReader SetFilePrefix  $::S2(dir)/$n(filePrefix)
    } else {
      $imageReader SetFilePrefix  $n(filePrefix)
    }
    $imageReader SetFilePattern  $n(filePattern)

    foreach {w h} $n(Dimensions) {}
    foreach {zlo zhi} $n(imageRange) {}
    set d [expr $zhi - $zlo]
    $imageReader SetDataExtent 0 [expr $w -1] 0 [expr $h - 1] 0 [expr $d -1]
    $imageReader SetFileNameSliceOffset $zlo
    $imageReader SetDataScalarTypeTo$n(scalarType)
    $imageReader SetSwapBytes $swap
    $imageReader Update


    #
    # now, construct the slicer3 node
    # - volume
    # - transform
    # - display
    #
    
    set volumeNode [vtkMRMLScalarVolumeNode New]
    $volumeNode SetAndObserveImageData [$imageReader GetOutput]
    $volumeNode SetName $n(name)
    $volumeNode SetDescription $n(description)

    set rasToVTK [vtkMatrix4x4 New]
    eval $rasToVTK DeepCopy $n(rasToVtkMatrix)
    $volumeNode SetRASToIJKMatrix $rasToVTK
    $rasToVTK Delete

    # use the current top of stack (might be "" if empty, but that's okay)
    set transformID [lindex $::S2(transformIDStack) end]
    $volumeNode SetAndObserveTransformNodeID $transformID

    if { [info exists n(labelMap)] && $n(labelMap) == "yes" } {
        $volumeNode SetLabelMap 1
    }

    set volumeDisplayNode [vtkMRMLVolumeDisplayNode New]
    switch { $n(colorLUT) } {
      "0" {
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorNodeGrey"
      }
      default {
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorNodeGrey"
      }
    }
    if { [info exists n(labelMap)] && $n(labelMap) == "yes" } {
        $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorNodeLabels"
    }

    if { [info exists n(applyThreshold)] && $n(applyThreshold) == "yes" } {
        $volumeDisplayNode SetApplyThreshold 1
    }
    $volumeDisplayNode SetWindow $n(window)
    $volumeDisplayNode SetLevel $n(level)
    $volumeDisplayNode SetLowerThreshold $n(lowerThreshold)
    $volumeDisplayNode SetUpperThreshold $n(upperThreshold)


    #
    # add nodes to the scene
    #

    $::slicer3::MRMLScene AddNode $volumeDisplayNode
    $volumeNode SetAndObserveDisplayNodeID [$volumeDisplayNode GetID]
    $::slicer3::MRMLScene AddNode $volumeNode


    #
    # clean up
    #
    $imageReader Delete
    $volumeNode Delete
    $volumeDisplayNode Delete

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


