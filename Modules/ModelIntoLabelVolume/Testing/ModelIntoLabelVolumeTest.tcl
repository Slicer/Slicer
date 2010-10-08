proc TestUsingCTHeadAxial {} {

# get the volumes logic
set volLogic [$::slicer3::VolumesGUI GetLogic]
# add the testing volume 
set volumeNode [$volLogic AddArchetypeVolume $::env(Slicer_HOME)/../Slicer3/Testing/Data/Input/CTHeadAxial.nhdr "CTHeadAxial" 0]

# get the models logic
set modLogic [$::slicer3::ModelsGUI GetLogic]
# add the testing model, can I get it from $::env(Slicer_MODULES_DIR)
set modelNode [$modLogic AddModel $::env(Slicer_HOME)/../Slicer3/Modules/ModelIntoLabelVolume/Testing/TestData/Neck.vtk]

# do it
set labelVolume [ModelIntoLabelVolumeIntersect  $modelNode $volumeNode]

# test it
set testVolumeNode [$volLogic AddArchetypeVolume $::env(Slicer_HOME)/../Slicer3/Modules/ModelIntoLabelVolume/Testing/TestData/ModelIntoLabelVolumeTest.nrrd "ModelIntoLabelVolumeTest" 1]

# there should be 2100 2's in each image

}

proc TestWithArtificialData {} {
    set vol [vtkMRMLScalarVolumeNode New]
    $vol SetName Black
    $::slicer3::MRMLScene AddNode $vol

    # make a black test volume
    set imData [vtkImageData New]
    $imData SetScalarTypeToFloat
    $imData SetDimensions 20 20 20
    $imData SetNumberOfScalarComponents 1
    $imData AllocateScalars
    for {set i 0} {$i < 20} {incr i} {
        for {set j 0} {$j < 20} {incr j} {
            for {set k 0} {$k < 20} {incr k} {
                $imData SetScalarComponentFromFloat $i $j $k 0 0.0
            }
        }
    }
    
    $vol SetAndObserveImageData $imData 
    $imData Delete

    # now make up a model
    set mod [vtkMRMLModelNode New]
    $mod SetName Diamond
    $::slicer3::MRMLScene AddNode $mod

    # stealing the diamond glyph from the fiducials list widget
    set pts [vtkPoints New]
    $pts SetNumberOfPoints 6
    $pts InsertPoint 0 15.0 10.0 10.0
    $pts InsertPoint 1 10.0 15.0 10.0
    $pts InsertPoint 2 10.0 10.0 15.0
    $pts InsertPoint 3 5.0 10.0 10.0
    $pts InsertPoint 4 10.0 5.0 10.0
    $pts InsertPoint 5 10.0 10.0 5.0
    
    set polys [vtkCellArray New]
    $polys InsertNextCell 4
    $polys InsertCellPoint 0
    $polys InsertCellPoint 1
    $polys InsertCellPoint 3
    $polys InsertCellPoint 4

    $polys InsertNextCell 4
    $polys InsertCellPoint 1
    $polys InsertCellPoint 2
    $polys InsertCellPoint 4
    $polys InsertCellPoint 5

    $polys InsertNextCell 4
    $polys InsertCellPoint 2
    $polys InsertCellPoint 0
    $polys InsertCellPoint 5
    $polys InsertCellPoint 3

    set lines [vtkCellArray New]
    $lines InsertNextCell 2
    $lines InsertCellPoint 0
    $lines InsertCellPoint 3

    $lines InsertNextCell 2
    $lines InsertCellPoint 1
    $lines InsertCellPoint 4

    $lines InsertNextCell 2
    $lines InsertCellPoint 2
    $lines InsertCellPoint 5

    set pd [vtkPolyData New]
    $pd SetPoints $pts
    $pts Delete
    $pd SetPolys $polys
    $pd SetLines $lines
    $polys Delete
    $lines Delete

    $mod SetAndObservePolyData $pd
    $pd Delete

    # now intersect
    set labelVolume [ModelIntoLabelVolumeIntersect  $mod $vol]

    set labelData [$labelVolume GetImageData]
 
    set numMisplaced 0

    if {[$labelData GetScalarComponentAsFloat 15 10 10 0] != 2.0} {
        incr numMisplaced
    }
    if {[$labelData GetScalarComponentAsFloat 10 15 10 0] != 2.0} {
        incr numMisplaced
    }
    if {[$labelData GetScalarComponentAsFloat 10 10 15 0] != 2.0} {
        incr numMisplaced
    }
    if {[$labelData GetScalarComponentAsFloat 5 10 10 0] != 2.0} {
        incr numMisplaced
    }
    if {[$labelData GetScalarComponentAsFloat 10 5 10 0] != 2.0} {
        incr numMisplaced
    }
    if {[$labelData GetScalarComponentAsFloat 10 10 5 0] != 2.0} {
        incr numMisplaced
    }

    $vol Delete
    $mod Delete

    if {$numMisplaced != 0} {
        puts "ERROR: $numMisplaced points out of 6."
        return 1
    } else {
        return 0
    }
}

set ret [TestWithArtificialData]
if {$ret} {
    exit 1
} else {
    exit 0
}

