# assume that the model /projects/birn/freesurfer/data/bert/surf/lh.pial has been read in as the first model, fourth after the default ones
set fileName /projects/birn/freesurfer/data/bert/label/lh.aparc.annot

# get the model out of the scene
set _id 4
set model [$::slicer3::MRMLScene GetNodeByID vtkMRMLModelNode${_id}]
set displaynode [$::slicer3::MRMLScene GetNodeByID vtkMRMLModelDisplayNode${_id}]
set actor [[$::slicer3::ApplicationGUI GetViewerWidget] GetActorByID vtkMRMLModelNode${_id}]
set mapper [$actor GetMapper]

if [file exists $fileName] {
    puts "Reading $fileName for model $model"

    set polydata [$model GetPolyData]
    set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]

    if { $scalaridx == "-1" } {
        puts "labels scalar doesn't exist for model $model, creating"
        set scalars "scalars_${_id}"
        catch "$scalars Delete"

        vtkIntArray $scalars
        $scalars SetName "labels"
        [$polydata GetPointData] AddArray $scalars
        [$polydata GetPointData] SetActiveScalars "labels"
    } else {
        set scalars [[$polydata GetPointData] GetScalars $scalaridx]
    }

    catch "fssarlut_${_id} Delete"
    set lut [vtkLookupTable fssarlut_${_id}]

    set fssar fssar_${_id}
    catch "$fssar Delete"
    vtkFSSurfaceAnnotationReader $fssar
    $fssar SetFileName $fileName
    $fssar SetOutput $scalars
    $fssar SetColorTableOutput $lut
    # try reading an internal colour table first
    $fssar UseExternalColorTableFileOff

    set retval [$fssar ReadFSAnnotation]
    if {$retval == 6} {
        puts "ERROR: no internal colour table"
        
        return
    }

    # set the look up table
    $mapper SetLookupTable $lut
    
    array unset _labels
    array set _labels [$fssar GetColorTableNames]
    array unset ::vtkFreeSurferReadersLabels${_id}
    array set ::vtkFreeSurferReadersLabels${_id} [array get _labels]
    set entries [lsort -integer [array names _labels]]

    # print them out
    parray ::vtkFreeSurferReadersLabels${_id}

    # make the scalars visible
    $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
    $mapper SetScalarVisibility 1
}
