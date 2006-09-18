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
  QueryAtlasInitializePicker 
  QueryAtlasRenderView
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


proc QueryAtlasNumberToRGBA {number} {
  set r [expr $number / (256 * 256)]
  set number [expr $number % (256 * 256)]
  set g [expr $number / 256]
  set b [expr $number % 256]

  return "$r $g $b 255"
}

proc QueryAtlasRGBAToNumber {rgba} {
  foreach {r g b a} $rgba {}
  return [expr $r * (256*256) + $g * 256 + $b] 
}



proc QueryAtlasInitializePicker {} {

  #
  # get the polydata for the model
  # - model node comes from the scene (retrieved by the ID)
  # - actor comes from the main Viewer
  # - mapper comes from the actor
  #
  set modelNode [$::slicer3::MRMLScene GetNodeByID $::QA(modelNodeID)]
  set polyData [$modelNode GetPolyData]
  set actor [[$::slicer3::ApplicationGUI GetViewerWidget] GetActorByID $::QA(modelNodeID)]
  set mapper [$actor GetMapper]

  #
  # instrument the polydata with cell number colors
  #

  $polyData Update

  set cellData [$polyData GetCellData]
  set cellNumberColors [$cellData GetArray "CellNumberColors"] 
  if { $cellNumberColors == "" } {
    set cellNumberColors [vtkUnsignedCharArray New]
    $cellNumberColors SetName "CellNumberColors"
    $cellData AddArray $cellNumberColors
    $cellData SetScalars $cellNumberColors
  }
  $cellData SetScalars $cellNumberColors

  set cellNumberColors [$cellData GetArray "CellNumberColors"] 
  $cellNumberColors Initialize
  $cellNumberColors SetNumberOfComponents 4

  set numberOfCells [$polyData GetNumberOfCells]
  for {set i 0} {$i < $numberOfCells} {incr i} {
    eval $cellNumberColors InsertNextTuple4 [QueryAtlasNumberToRGBA $i]
  }

  set ::QA(cellData) $cellData
  set ::QA(numberOfCells) $numberOfCells
  set ::QA(mapper) $mapper
  set ::QA(actor) $actor

  #
  # add the mouse move callback
  # - create the classes that will be used every render and in the callback
  # - add the callback with the current render info
  #
  if { ![info exists ::QA(viewer)] } {
    set ::QA(viewer) [vtkImageViewer New]
    set ::QA(windowToImage) [vtkWindowToImageFilter New]
  }
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set renderer [$renderWidget GetRenderer]
  set interactor [$renderWidget GetRenderWindowInteractor] 
  $interactor AddObserver MouseMoveEvent "QueryAtlasPickCallback $renderer $interactor $::QA(windowToImage)"
}

proc QueryAtlasRenderView {} {

  #
  # get the renderer related instances
  #
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set renderWindow [$renderWidget GetRenderWindow]
  set renderer [$renderWidget GetRenderer]

  #
  # set the render parameters to draw with the cell labels
  #
  set renderState [QueryAtlasOverrideRenderState $::QA(cellData) $::QA(numberOfCells) $::QA(mapper) $::QA(actor)]

  # if needed - remove other props and only add ours 
  # (need to keep track of others somehow so we can restore them)
  #$renderWidget RemoveAllViewProps
  #$renderer AddActor $actor

  #
  # draw the image and get the pixels
  #
  $renderWindow SetSwapBuffers 0
  puts "render renderWidget"; update
  $renderWidget Render
  puts "...render done"; update

  $::QA(viewer) SetColorWindow 255
  $::QA(viewer) SetColorLevel 127.5
  $::QA(windowToImage) SetInputBufferTypeToRGBA
  $::QA(windowToImage) ShouldRerenderOn
  $::QA(windowToImage) ReadFrontBufferOff
  $::QA(windowToImage) SetInput [$renderWidget GetRenderWindow]
  $::QA(windowToImage) Modified
  $::QA(viewer) SetInput [$::QA(windowToImage) GetOutput]
  puts "render viewer"; update
  $::QA(viewer) Render
  puts "...render done"; update

  $renderWindow SetSwapBuffers 1
  QueryAtlasRestoreRenderState $::QA(cellData) $::QA(mapper) $::QA(actor) $renderState

  puts "render renderWidget"; update
  $renderWidget Render
  puts "...render done"; update

}

proc QueryAtlasOverrideRenderState {cellData numberOfCells mapper actor} {

  #
  # save the render state before overriding it with the 
  # parameters needed for cell rendering
  #
  if { [$cellData GetScalars] == "" } {
    set state(activeScalars) ""
  } else {
    set state(activeScalars) [[$cellData GetScalars] GetName]
  }
  set state(scalarVisibility) [$mapper GetScalarVisibility]
  set state(immediateModeRendering) [$mapper GetImmediateModeRendering]
  set state(scalarMode) [$mapper GetScalarMode]
  set state(scalarMaterialMode) [$mapper GetScalarMaterialMode]
  set state(scalarRange) [$mapper GetScalarRange]
  set state(ambient) [[$actor GetProperty] GetAmbient]
  set state(diffuse) [[$actor GetProperty] GetDiffuse]

  parray state

  $cellData SetActiveScalars "CellNumberColors"
  $mapper SetImmediateModeRendering 1
  $mapper SetScalarVisibility 1
  $mapper SetScalarModeToUseCellData
  $mapper SetScalarMaterialModeToAmbient
  $mapper SetScalarRange 0 $numberOfCells
  [$actor GetProperty] SetAmbient 1
  [$actor GetProperty] SetDiffuse 0

  return [array get state]
}

proc QueryAtlasRestoreRenderState {cellData mapper actor renderState} {

  array set state $renderState

  if { $state(activeScalars) != "" } {
    $cellData SetActiveScalars $state(activeScalars)
  }
  $mapper SetScalarVisibility $state(scalarVisibility)
  $mapper SetImmediateModeRendering $state(immediateModeRendering) 
  $mapper SetScalarMode $state(scalarMode)
  $mapper SetScalarMaterialMode $state(scalarMaterialMode)
  eval $mapper SetScalarRange $state(scalarRange)
  [$actor GetProperty] SetAmbient $state(ambient)
  [$actor GetProperty] SetDiffuse $state(diffuse)
}

#
# query the cell number at the mouse location
#
proc QueryAtlasPickCallback {renderer interactor windowToImage} {

  if { ![info exists ::QA(viewer)] } {
    return
  }

  eval $interactor UpdateSize [$renderer GetSize]
  foreach {x y} [$interactor GetEventPosition] {}
  $windowToImage Update
  set color ""
  foreach c {0 1 2 3} {
    lappend color [[$windowToImage GetOutput] GetScalarComponentAsFloat $x $y 0 $c]
  }
  puts "[format {%4d %4d} $x $y]:  [QueryAtlasRGBAToNumber $color] ($color)"
}
