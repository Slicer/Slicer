#
# ModelIntoLabelVolume logic procs
#

#
# intersect a model and volume, creating a new label map and returning it
# labelValue is the integer value to place in the voxels of the output label volume, default 2
# labelVolumeName is what to name your new volume, if not set, uses the model and volume names to create a new name
# intensityFlag if set to 1, place the intensity in the output label map volume (will produce a random colour display), default 0
# matrixNode not used
proc ModelIntoLabelVolumeIntersect { modelNode volumeNode {labelValue 2} {labelVolumeName ""} {intensityFlag 0} {matrixNode ""} } {
    
  if {$modelNode == ""} {
      puts "You must select a model first"
      return
  } elseif {$volumeNode == ""} {
      puts "You must select a reference volume first"
      return
  } 

  set deleteMt 1
  set debug 0
  set progress 0.0
  set progressGauge [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetProgressGauge]

  if {$labelVolumeName == ""} {
      set labelVolumeName "[$modelNode GetName]-[$volumeNode GetName]"
  }


  # clone the reference volume into a label map
  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelmapVolumeNode [$volumesLogic CreateLabelVolume $::slicer3::MRMLScene $volumeNode $labelVolumeName]

  # now get the volume's matrix
  catch "Mrasijk Delete"
  vtkMatrix4x4 Mrasijk
  # get the volume's RAS to ijk matrix
  $volumeNode GetRASToIJKMatrix Mrasijk 

  # for now use an identity transform, later can pass one in
  catch "IdMatrix Delete"
  set Mt [vtkMatrix4x4 IdMatrix]
  if {$matrixNode != ""} {
      $matrixNode GetMatrixTransformToWorld $Mt
      set deleteMt 0
  }

  # now loop over the points, transform them and write them out
  puts "Transforming RAS vtkPolydata points in IJK"
  set numPoints [[$modelNode GetPolyData] GetNumberOfPoints]
  set foundNumPoints 0

  if {$debug} {
      set fid [open "/spl/tmp/nicole/fids.txt" "w"]
  }
  # Get the vertex points out of the model and transform them
  for {set p 0} {$p < $numPoints} {incr p} {

      # update progress
      if {[expr $p % 100] == 0} {
          $progressGauge SetValue [expr ($p * 1.0 / $numPoints) * 100.0]
      }
      # get the coordinate points
      scan [[$modelNode GetPolyData] GetPoint $p] "%f %f %f" a b c

      # multiply by the transform matrix   ( Mt = Identity matrix)
      set transformPoint [$Mt MultiplyPoint $a $b $c 1]
      
      # multiply by the RAS to IJK matrix from the volume
      set ijkPoint [Mrasijk MultiplyPoint [lindex $transformPoint 0] [lindex $transformPoint 1] [lindex $transformPoint 2] [lindex $transformPoint 3]]
      #set ijkPoint [$Mt MultiplyPoint [lindex $transformPoint 0] [lindex $transformPoint 1] [lindex $transformPoint 2] [lindex $transformPoint 3]]
      ##################### Modif Sonia 2 ############################
        # Transform the IJK (float) into integers
      set i   [lindex $ijkPoint 0]
      set i0   [expr int($i)]

      set tmp [expr abs($i - $i0)]
      if {$tmp > 0.5} {
          set i [expr $i0 +1 ]
      } else {
          set i $i0
      }
      set j   [lindex $ijkPoint 1]
      set j0   [expr int($j)]
      
      set tmp [expr abs($j - $j0)]
      if {$tmp > 0.5} {
          set j [expr $j0 +1 ]
      } else {
          set j $j0
      }
      set k   [lindex $ijkPoint 2]
      set k0   [expr int($k)]
      
      set tmp [expr abs($k - $k0)]
      if {$tmp > 0.5} {
          set k [expr $k0 +1 ]
      } else {
          set k $k0
      }
      ##################################################################
      
      # get the voxelID corresponding to the ijk location
      # set voxelPointID [[Volume($volumeID,vol) GetOutput] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
      
      # Set the intensity to the corresponding voxel in the labelmapvolume
      set voxeloutPointIDFloat [[$labelmapVolumeNode GetImageData] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
      
      # set voxeloutPointID [[$labelmapVolumeNode GetImageData] FindPoint [expr $i * 2.0] [expr $j * 2.0] [expr  $k * 2.0] ]
      set voxeloutPointID [[$labelmapVolumeNode GetImageData] FindPoint $i $j $k]
      if {$voxeloutPointID < 0} {
          puts "Unable to find volume point id for $ijkPoint"
      } else {
          if {$intensityFlag == 0} {
              [[ [$labelmapVolumeNode GetImageData] GetPointData] GetScalars] SetTuple1 $voxeloutPointID $labelValue
          } else {
              # the voxelID corresponding to the ijk locations
              set voxelPointID [[$volumeNode GetImageData] FindPoint [lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]]
              
              if {$voxelPointID < 0} {
                  puts "Unable to find volume point id for $ijkPoint"
              } else {
                  
                  # read the intensity from the FA map
                  set intensity [[[[$volumeNode GetImageData] GetPointData] GetScalars] GetTuple1 $voxelPointID]
                  # and set it
                  [[ [$labelmapVolumeNode GetImageData] GetPointData] GetScalars] SetTuple1 $voxeloutPointID $intensity
              }
          }
          
          if {$debug} { 
            if {$p < 50} {
                # print out the first point's values
                puts "Point number $p:"
                puts "Dvtk =\n\t$a $b $c 1"
                puts "Mt * Dvtk =\n\t$transformPoint"
                puts "IJK Point is : Mvtk * Mt * Dvtk =\n\t$ijkPoint"
                if {$intensityFlag != 0} {
                   puts "Intensity = $intensity"
                }
              }
              # puts $fid "[lindex $ijkPoint 0] [lindex $ijkPoint 1] [lindex $ijkPoint 2]"
              #puts $fid "$p $voxeloutPointID $voxeloutPointIDFloat"
            }              
          incr foundNumPoints
      }
  }
  # end of looping over model points
  $progressGauge SetValue 0.0

  # clean up  
  if {$debug} {
      close $fid
  }
  Mrasijk Delete
  # clean up  
    if {$deleteMt} {
      # only delete it if it's our declared identity, otherwise it's a pointer to 
      # a Slicer variable
      $Mt Delete
  }

  return $labelmapVolumeNode
}

#
# Create a new model node with polydata that has more vertices, subdividing the polydata of the input model
# This is useful if you're not getting a solid trace around the model in the output volume
#
proc ModelIntoLabelVolumeLogicSubdivideModel { modelName {newModelName ""} } {

    if {$modelName == ""} {
        puts "You must provide a model name"
        return
    }
    if {$newModelName == ""} {
        set newModelName ${modelName}-Subdivided
    }

    # get the input model node
    set modelList [$::slicer3::MRMLScene GetNodesByName $modelName]
    if {[$modelList GetNumberOfItems] < 1} {
        puts "Can't find a model with the name $modelName"
        return
    }
    set modelNode [$modelList GetItemAsObject 0]

    # make a new model node 
    set newModelNode [vtkMRMLModelNode New]
    $newModelNode SetName $newModelName

    # set up the filter
    set subdiv [vtkButterflySubdivisionFilter New]
    $subdiv SetInput [$modelNode GetPolyData]
    
    # and go
    $subdiv Update

    puts "Increased number of points in model [$modelNode GetName]'s poly data from [[$modelNode GetPolyData] GetNumberOfPoints] to [[$subdiv GetOutput] GetNumberOfPoints], and saved to model node $newModelName"

    # point the new model to the output of the filter
    $newModelNode SetAndObservePolyData [$subdiv GetOutput]

    # add it to the scene
    set dispNode [vtkMRMLModelDisplayNode New]
    $::slicer3::MRMLScene AddNode $dispNode
    # copy the old node's display settings
    $dispNode Copy [$modelNode GetDisplayNode]
    $::slicer3::MRMLScene AddNode $newModelNode
    $newModelNode SetAndObserveDisplayNodeID [$dispNode GetID]

    # clean up
    $subdiv Delete

    return $newModelNode
}
