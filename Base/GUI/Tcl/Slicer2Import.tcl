
#
# test of importing a slicer2 mrml scene using the 
# vtk xml data parser mechanism
#

#######
#
# for debugging - run the command when the script is read...
#
#after idle {
#  puts -nonewline "importing..."
#  ImportSlicer2Scene c:/data/tutorial/tutorial.xml
#  set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
#  [$viewer GetMainViewer] Reset
#  puts "done"
#}

# main entry point...
proc ImportSlicer2Scene {sceneFile} {

  set parser [vtkXMLDataParser New]
  $parser SetFileName $sceneFile
  $parser Parse

  set root [$parser GetRootElement]

  set ::S2(dir) [file dirname [file normalize $sceneFile]]
  set ::S2(transformIDStack) ""
  set ::S2(fiducialListNode) ""
  set ::S2_HParent_ID ""
  array unset ::S2_Model_ID ""

  ImportElement $root

  $parser Delete

  $::slicer3::MRMLScene SetErrorCode 0
}

#
# recursive routine to import all elements and their
# nested parts
#
proc ImportElement {element} {
  # save current parent locally
  set parent $::S2_HParent_ID

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

  # restore parent locally
  set ::S2_HParent_ID $parent
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
    set err [$::slicer3::MRMLScene GetErrorMessagePointer]
    $::slicer3::MRMLScene SetErrorMessage "$err\nno handler for $nodeType"
    $::slicer3::MRMLScene SetErrorCode 1
  }

  # call the handler for this element
  $handler node
}

#####
#
# handler procs for each type of node
#

proc ImportNodeMRML {node} {
  # no op, just a marker
}

proc ImportNodeTransform {node} {

  # no op - handled by ImportElement
}

#
# a slicer2 matrix corresponds to a slicer3 Transform
#
proc ImportNodeMatrix {node} {
  upvar $node n

  set transformNode [vtkMRMLLinearTransformNode New]

  set matrix [$transformNode GetMatrixTransformToParent]
  if { [info exists n(name)] } {
    $transformNode SetName $n(name)
  } else {
    $transformNode SetName "Imported Transform"
  }
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

  if { ![info exists n(fileType)] } {
    set n(fileType) "Basic"
  }

  if { ![info exists n(name)] } {
    set n(name) "Imported Volume"
  }

  if { ![info exists n(description)] } {
    set n(description) ""
  }

  switch [string tolower $n(fileType)] {

    "nrrd" -
    "generic" {

      if { ![info exists n(fileName)] } {
        set n(fileName) [format $n(filePattern) $n(filePrefix)]
      }

      if { [file pathtype $n(fileName)] == "relative" } {
        set fileName $::S2(dir)/$n(fileName)
      } else {
        set fileName $n(fileName)
      }

      set labelMap 0
      if { [info exists n(labelMap)] && ($n(labelMap) == "yes"  || $n(labelMap) == "true") } {
          set labelMap 1
      }

      set logic [$::slicer3::VolumesGUI GetLogic]
      set volumeNode [$logic AddArchetypeVolume $fileName 1 $labelMap $n(name)]
      set volumeNodeID [$volumeNode GetID]

    }

    "headerless" -
    "basic" {

      #
      # first, parse the slicer2 node
      #
      if { ![info exists n(dimensions)] } {
        set n(dimensions) "256 256"
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

      foreach {w h} $n(dimensions) {}
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
      $imageReader Delete

      if { [info exists n(labelMap)] && ($n(labelMap) == "yes"  || $n(labelMap) == "true") } {
          $volumeNode SetLabelMap 1
          set volumeDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
      } else {
          set volumeDisplayNode [vtkMRMLScalarVolumeDisplayNode New]
      }

      #
      # add nodes to the scene
      #

      $::slicer3::MRMLScene AddNode $volumeDisplayNode
      $::slicer3::MRMLScene AddNode $volumeNode
      $volumeNode SetAndObserveDisplayNodeID [$volumeDisplayNode GetID]
      $volumeNode SetModifiedSinceRead 1

      # use the RASToIJK information from the file, to override what the 
      # archetype reader might have set
      set rasToVTK [vtkMatrix4x4 New]
      eval $rasToVTK DeepCopy $n(rasToVtkMatrix)
      $volumeNode SetRASToIJKMatrix $rasToVTK
      $rasToVTK Delete


      #
      # clean up
      #
      set volumeNodeID [$volumeNode GetID]
      $volumeNode Delete
      $volumeDisplayNode Delete
    }
  }

  set volumeNode [$::slicer3::MRMLScene GetNodeByID $volumeNodeID]

  # use the current top of stack (might be "" if empty, but that's okay)
  set transformID [lindex $::S2(transformIDStack) end]
  $volumeNode SetAndObserveTransformNodeID $transformID

  set volumeDisplayNode [$volumeNode GetDisplayNode]

  switch -- $n(colorLUT) {
    "0" {
      $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
    }
    "-1" {
      $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeLabels"
    }
    default {
      $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
    }
  }

  if { [info exists n(applyThreshold)] && ( $n(applyThreshold) == "yes" || $n(applyThreshold) == "true" ) } {
      if { [$volumeDisplayNode IsA "vtkMRMLScalarVolumeDisplayNode"] } {
          $volumeDisplayNode SetApplyThreshold 1
      } else {
          puts "Slicer2Import.tcl: ImportNodeVolume Can't set apply threshold, $volumeDisplayNode is a [$volumeDisplayNode GetClassName] rather than a vtkMRMLScalarVolumeDisplayNode"
      }
  }
  if { [$volumeDisplayNode IsA "vtkMRMLScalarVolumeDisplayNode"] } {
    $volumeDisplayNode SetWindow $n(window)
    $volumeDisplayNode SetLevel $n(level)
    $volumeDisplayNode SetLowerThreshold $n(lowerThreshold)
    $volumeDisplayNode SetUpperThreshold $n(upperThreshold)
  }

  set logic [$::slicer3::VolumesGUI GetLogic]
  $logic SetActiveVolumeNode $volumeNode

  [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetReferenceActiveVolumeID [$volumeNode GetID]
  [$::slicer3::VolumesGUI GetApplicationLogic] PropagateVolumeSelection
}

proc ImportNodeModel {node} {
  upvar $node n

  if { [file pathtype $n(fileName)] == "relative" } {
    set fileName $::S2(dir)/$n(fileName)
  } else {
    set fileName $n(fileName)
  }

  set logic [$::slicer3::ModelsGUI GetLogic]
  set mnode [$logic AddModel $fileName]
  set dnode [$mnode GetDisplayNode]

  if { ![info exists n(id)] } {
    # model node has no id, so create one
    # - try to get a high number that isn't already used
    for {set i 1000} {$i < 1000000} {incr i} {
      if { [lsearch [array names ::S2_Model_ID] $i] == -1 } {
        set n(id) $i
        break
      }
    }
  }
  set ::S2_Model_ID($n(id)) [$mnode GetID]

  if { [info exists n(visibility)] } {
    if {$n(visibility) == "false"} {
      $dnode SetVisibility 0
    } else {
      $dnode SetVisibility 1
    }
  }

  if { [info exists n(color)] } {

    if { [string tolower $n(color)] == "skin" } {
      # workaround slicer2 ethnocentrism
      set n(color) "peach"
    }

    set cnode [vtkMRMLColorTableNode New]
    foreach colorType "SPLBrainAtlas Labels" {

      $cnode SetTypeTo$colorType
      puts "looking for $n(color) in $colorType node $cnode"
      set saveColor 0
      for {set i 0} {$i < [$cnode GetNumberOfColors]} {incr i} {
        set name [$cnode GetColorName $i]
        if {[string tolower $name] == [string tolower $n(color)]} {
          eval $dnode SetColor [lrange [[$cnode GetLookupTable] GetTableValue $i] 0 2]
          set saveColor 1
          puts " found color $i $name"
          break
        }
      } 
      if {$saveColor == 0} {
        $dnode SetAttribute colorid $n(color)
      } else {
        break
      }
    }
    $cnode Delete
  }
}

proc ImportNodeHierarchy {node} {
  set ::S2_HParent_ID ""
}

proc ImportNodeModelGroup {node} {
  upvar $node n
  set hnode [vtkMRMLModelHierarchyNode New]
  set dnode [vtkMRMLModelDisplayNode New]
  $hnode SetScene $::slicer3::MRMLScene
  $dnode SetScene $::slicer3::MRMLScene

  if { [info exists n(visibility)] } {
    if {$n(visibility) == "false"} {
      $dnode SetVisibility 0
    } else {
      $dnode SetVisibility 1
    }
  }

  if { [info exists n(name)] } {
    $hnode SetName $n(name)
  }

  if { [info exists n(color)] } {
      set cnode [$::slicer3::MRMLScene GetNodeByID vtkMRMLColorTableNodeSPLBrainAtlas]
      for {set i 0} {$i < [$cnode GetNumberOfColors]} {incr i} {
          if {[$cnode GetColorName $i] == $n(color)} {
              eval $dnode SetColor [lrange [[$cnode GetLookupTable] GetTableValue $i] 0 2]
          }
      } 
  }
  set dnode [$::slicer3::MRMLScene AddNodeNoNotify $dnode]
  set hnode [$::slicer3::MRMLScene AddNode $hnode]

  if {$::S2_HParent_ID != ""} {
    $hnode SetParentNodeIDReference $::S2_HParent_ID
  }

  $hnode SetAndObserveDisplayNodeID [$dnode GetID]

  set ::S2_HParent_ID [$hnode GetID]
}

proc ImportNodeModelRef {node} {
  upvar $node n
  set hnode [vtkMRMLModelHierarchyNode New]
  $hnode SetScene $::slicer3::MRMLScene

  $hnode SetExpanded 1
  set id2 $n(ModelRefID)
  set id3 $::S2_Model_ID($id2)
  $hnode SetName [[$::slicer3::MRMLScene GetNodeByID $id3] GetName]

  #set dnode [vtkMRMLModelDisplayNode New]
  #set dnode [$::slicer3::MRMLScene AddNodeNoNotify $dnode]

  set hnode [$::slicer3::MRMLScene AddNode $hnode]

  if {$::S2_HParent_ID != ""} {
    $hnode SetParentNodeIDReference $::S2_HParent_ID
  }
  $hnode SetModelNodeIDReference $id3

  #$hnode SetAndObserveDisplayNodeID [$dnode GetID]

}

proc ImportNodeFiducials {node} {
  upvar $node n

  set fiducialNode [vtkMRMLFiducialListNode New]

#  if { [info exists n(type)] } {
#    $fiducialNode SetGlyphTypeFromString $n(type)
#  }
  if { [info exists n(symbolSize)] } {
    $fiducialNode SetSymbolScale $n(symbolSize)
  }
  if { [info exists n(textSize)] } {
    $fiducialNode SetTextScale $n(textSize)
  }
  if { [info exists n(visibility)] } {
    $fiducialNode SetVisibility $n(visibility)
  }
  if { [info exists n(color)] } {
    foreach {c0 c1 c2} $n(color) {}
    $fiducialNode SetColor $c0 $c1 $c2
  }
  if { [info exists n(name)] } {
    $fiducialNode SetName $n(name)
  } 
  $::slicer3::MRMLScene AddNode $fiducialNode
  set ::S2(fiducialListNode) $fiducialNode

  # set it to be the selected one, last one imported will stick
  set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
  if { $selNode != "" } {
     $selNode SetReferenceActiveFiducialListID [$fiducialNode GetID]
     $::slicer3::ApplicationLogic PropagateFiducialListSelection
  }
}

proc ImportNodePoint {node} {
  upvar $node n
  set f [$::S2(fiducialListNode) AddFiducial]

  if { [info exists n(xyz)] } {
    foreach {x y z} $n(xyz) {}
    $::S2(fiducialListNode) SetNthFiducialXYZ $f $x $y $z
  }
  if { [info exists n(name)] } {
    $::S2(fiducialListNode) SetNthFiducialLabelText $f $n(name)
  } 
}

proc ImportNodeColor {node} {
  upvar $node n
  if { [info exists n(name)] } {
      set id $n(name)
      if { [info exists n(diffuseColor)] } {
          foreach {r g b} $n(diffuseColor) {}
          $::slicer3::MRMLScene InitTraversal
          set ndnodes [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLModelDisplayNode]
          for {set i 0} {$i < $ndnodes} {incr i} {
              set dnode [$::slicer3::MRMLScene GetNthNodeByClass $i vtkMRMLModelDisplayNode]
              set cid [$dnode GetAttribute colorid]
              #$dnode SetAttribute one $id
              #$dnode SetAttribute two $cid
              if {$id == $cid} {
                  $dnode SetColor $r $g $b
              }
          }
      }
  }
}

proc ImportNodeOptions {node} {
  $::slicer3::MRMLScene SetErrorMessage "$err\nwarning: option nodes cannot be imported"
  $::slicer3::MRMLScene SetErrorCode 1
}


