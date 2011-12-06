# test out loading a new color file, assigning the color to a label map, saving a scene, closing the scene, then loading it again to check that it's there

set ::csnDebug 0
# if use a collection to get the nodes by name from the scene, there is a leak of 1 vtkCollection
set ::csnUseCollection 0

proc TestColorStorageNodeInScene {  } {

    if { [info script] != ""} {
       set colorFileName "[file dirname [info script]]/Libs/MRML/Core/Testing/TestData/ColorTest.ctbl"
    } else {
        set colorFileName [file normalize $::env(SLICER_HOME)/../Slicer3/Libs/MRML/Core/Testing/TestData/ColorTest.ctbl]
    }
    if {$::csnDebug} {
        puts "colorFileName = $colorFileName"
    }
    # get the color logic
    set colorLogic [ $::slicer3::ColorGUI GetLogic ]
    
    # load the color node
    set cnode [$colorLogic LoadColorFile $colorFileName "ColorTest"]

    if {$cnode == ""} {
        puts "Error loading color file $colorFileName"
        return 1
    } else {
        puts "Loaded color file as node named [$cnode GetName]"
    }

    set cnodeID [$cnode GetID]
    set cnodeName [$cnode GetName]
    set testNode [$::slicer3::MRMLScene GetNodeByID $cnodeID]
    if {$testNode == ""} {
        puts "Error getting the color node by id $cnodeID"
        return 1
    }

    # get the storage node
    set snode [$cnode GetStorageNode]
    if {$snode == ""} {
        puts "Error getting the storage node from the color node"
        return 1
    }

    # load a volume file
    set volLogic [$::slicer3::VolumesGUI GetLogic]
    if {$volLogic != ""} {
      set volFileName "[file normalize $::env(SLICER_HOME)/../Slicer3/Libs/MRML/Core/Testing/TestData/helixMask3Labels.nrrd]"
      set volName "helixMask3Labels"
    
      set label 1
      set volNode [$volLogic AddArchetypeVolume $volFileName $volName $label]
      if {$volNode == ""} {
        puts "Error loading a volume file: $volFilename"
        return 1
      }
      set volDispNode [$volNode GetDisplayNode]
      if {$volDispNode == ""} {
          puts "Error getting the volume display node"
          return 1
      }
    
      # observe this new color node
      $volDispNode SetAndObserveColorNodeID $cnodeID
      if {[$volDispNode GetColorNode] == ""} {
        puts "Error setting and getting the color node on the label map"
        return 1
      }
    }
    # save to file
    set sceneFileName "testColorStorageNode.mrml"
    set saveReturn [$::slicer3::MRMLScene Commit $sceneFileName]
    if {$saveReturn != 1} {
        puts "Error saving scene to file $sceneFileName"
        return 1
    }

    # close the scene
    $::slicer3::MRMLScene Clear 0

    set testNode  [$::slicer3::MRMLScene GetNodeByID $cnodeID]
    if {$testNode != ""} {
        puts "Failed to clear new node from scene"
        return 1
    }

    # reload it
    $::slicer3::MRMLScene SetURL $sceneFileName
    set connectReturn [$::slicer3::MRMLScene Connect]
    if {$connectReturn != 1} {
        puts "Failed to reload $sceneFileName"
        return 1
    }

    # the id should be the same
    set testNode  [$::slicer3::MRMLScene GetNodeByID $cnodeID]
    if {$testNode == ""} {
        puts "Error reloading test node"
        return 1
    }

    if {$volLogic != ""} {
        set testVol ""
        if {$::csnUseCollection == 1} {
            set col [$::slicer3::MRMLScene GetNodesByName $volName]
            if {$col == ""} {
                puts "Error getting back the volume name $volName"
               return 1
            }
            if {[$col GetNumberOfItems] != 1} {
                puts "Error getting back the volume named $volName, found [$col GetNumberOfItems] with that name"
                return 1
            }
            set testVol [$col GetItemAsObject 0]
        } else {
            set numNodes [ $::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLScalarVolumeNode"] 
            for {set n 0} { $n < $numNodes} {incr n} {
                set testNode  [ $::slicer3::MRMLScene GetNthNodeByClass $n "vtkMRMLScalarVolumeNode"]
                if {[$testNode GetName] == $volName} {
                    set testVol $testNode
                    break
                }
            }
        }
       
        if {$testVol == ""} {
            puts "Error getting volume named $volName"
            return 1
        }
        if {$::csnUseCollection == 1} {
            $col RemoveAllItems
            puts "Reference count on collection after removing all items, and before Delete = [$col GetReferenceCount]"
            set ret [ catch {$col Delete} res]
            if {$ret} { puts "Error deleting collection:\n$res" }
            set col ""
        }

        set testVolDisp [$testVol GetDisplayNode]
        if {$testVolDisp == ""} {
            puts "Error getting first volume's display node"
            return 1
        }
        set testColNode [$testVolDisp GetColorNode]
        if {$testColNode == ""} {
            puts "Error getting back the color node from the display node"
            return 1
        }
        set testColNodeName [$testColNode GetName]
        if {$testColNodeName != $cnodeName} {
            puts "Got the wrong color node back! Volume is observing $testColNodeName instead of $cnodeName"
            return 1
        }
    }
    # success
    return 0
}

proc runtest { } {
    set ret [ catch {
         $::slicer3::Application TraceScript TestColorStorageNodeInScene
    } res]

 if { $ret } {
   puts stderr $res
     if {$::csnDebug} {
         return 1
     } else {
         exit 1
     }
 }
 if {$::csnDebug} {
    return 0
 } else {
    exit 0
 }
}

update 
after idle runtest
