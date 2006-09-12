# assume that the model /projects/birn/freesurfer/data/bert/surf/lh.pial has been read in as the first model, fourth after the default ones

proc QueryAtlasInit { {filename ""} } {
  
  # find the data
  set ::QA(filename) ""
  if { $filename != "" } {
    set ::QA(filename) $filename
  } else {
    set candidates {
      set fileName /projects/birn/freesurfer/data/bert/surf/lh.pial
      set fileName c:/data/fBIRN-AHM2006/fbph2-000648622547/surf/lh.pial
    }
    foreach c $candidates {
      if { [file exists $c] } {
        set ::QA(filename) $c
        break
      }
    }
  }

  QueryAtlasAddModel
  QueryAtlasAddAnnotations 
}

proc QueryAtlasAddModel {} {

  # load the data
  set modelNode [vtkMRMLModelNode New]
  set modelStorageNode [vtkMRMLModelStorageNode New]
  set modelDisplayNode [vtkMRMLModelDisplayNode New]

  $modelStorageNode SetFileName $::QA(filename)
  if { [$modelStorageNode ReadData $modelNode] != 0 } {
    $modelNode SetName [file tail $::QA(filename)]

    $modelNode SetScene $::slicer3::MRMLScene
    $modelStorageNode SetScene $::slicer3::MRMLScene
    $modelDisplayNode SetScene $::slicer3::MRMLScene

    $::slicer3::MRMLScene AddNode $modelStorageNode
    $::slicer3::MRMLScene AddNode $modelDisplayNode

    $modelNode SetStorageNodeID [$modelStorageNode GetID]
    $modelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]

    $::slicer3::MRMLScene AddNode $modelNode
    set ::QA(modelNodeID) [$modelNode GetID]
  }

  $modelNode Delete
  $modelStorageNode Delete
  $modelDisplayNode Delete
}

proc QueryAtlasAddAnnotations {} {

  set fileName $::QA(filename)/../../label/lh.aparc.annot

  # get the model out of the scene
  set modelNode [$::slicer3::MRMLScene GetNodeByID $::QA(modelNodeID)]
  set displayNodeID [$modelNode GetDisplayNodeID]
  set displayNode [$::slicer3::MRMLScene GetNodeByID $displayNodeID]
  set viewer [$::slicer3::ApplicationGUI GetViewerWidget] 
  $viewer UpdateFromMRML
  set actor [$viewer GetActorByID [$modelNode GetID]]
  set mapper [$actor GetMapper]

  if [file exists $fileName] {

      set polydata [$modelNode GetPolyData]
      set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]

      if { $scalaridx == "-1" } {
          set scalars [vtkIntArray New]
          $scalars SetName "labels"
          [$polydata GetPointData] AddArray $scalars
          [$polydata GetPointData] SetActiveScalars "labels"
          #$scalars Delete
      } 
      set scalaridx [[$polydata GetPointData] SetActiveScalars "labels"]
      set scalars [[$polydata GetPointData] GetArray $scalaridx]

      set lut [vtkLookupTable New]
      set fssar [vtkFSSurfaceAnnotationReader New]

      $fssar SetFileName $fileName
      $fssar SetOutput $scalars
      $fssar SetColorTableOutput $lut
      # try reading an internal colour table first
      $fssar UseExternalColorTableFileOff

      set retval [$fssar ReadFSAnnotation]
      if {$retval == 6} {
          error "ERROR: no internal colour table"
      }

      # set the look up table
      $mapper SetLookupTable $lut
      
      array unset _labels
      array set _labels [$fssar GetColorTableNames]
      array unset ::vtkFreeSurferReadersLabels_$::QA(modelNodeID)
      array set ::vtkFreeSurferReadersLabels_$::QA(modelNodeID) [array get _labels]
      set entries [lsort -integer [array names _labels]]

      # print them out
      parray ::vtkFreeSurferReadersLabels_$::QA(modelNodeID)
      set ::QA(labels) [array get _labels]

      # make the scalars visible
      $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
      $mapper SetScalarVisibility 1

      $lut Delete
      $fssar Delete
      [$viewer GetMainViewer] Reset
  }
}
