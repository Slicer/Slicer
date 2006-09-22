# assume that the model /projects/birn/freesurfer/data/bert/surf/lh.pial has been read in as the first model, fourth after the default ones

proc QueryAtlasInit { {filename ""} } {
  
  # find the data
  set ::QA(filename) ""
  if { $filename != "" } {
    set ::QA(filename) $filename
  } else {
    set candidates {
      set fileName /projects/birn/freesurfer/data/bert/surf/lh.pial
      set fileName i:/fBIRN-AHM2006/fbph2-000670986943/surf/lh.pial
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

#
# Add the model with the filename to the scene
#
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

#
# use the freesurfer annotation code to put 
# label scalars onto the model
#
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
          $scalars Delete
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
      set ::QA(labelMap) [array get _labels]

      # make the scalars visible
      $mapper SetScalarRange  [lindex $entries 0] [lindex $entries end]
      $mapper SetScalarVisibility 1

      $lut Delete
      $fssar Delete
      [$viewer GetMainViewer] Reset
  }
}

#
# convert a number to an RGBA 
# - A is always 255 (on transp)
# - number is incremented first so that 0 means background
#
proc QueryAtlasNumberToRGBA {number} {
  set number [expr $number + 1]
  set r [expr $number / (256 * 256)]
  set number [expr $number % (256 * 256)]
  set g [expr $number / 256]
  set b [expr $number % 256]

  return "$r $g $b 255"
}

#
# convert a RGBA to number
# - decrement by 1 to avoid ambiguity, since 0 is background
#
proc QueryAtlasRGBAToNumber {rgba} {
  foreach {r g b a} $rgba {}
  return [expr $r * (256*256) + $g * 256 + $b - 1] 
}


#
# set up a picking version of the polyData that can be used
# to render to the back buffer
#
proc QueryAtlasInitializePicker {} {

  #
  # get the polydata for the model
  # - model node comes from the scene (retrieved by the ID)
  # - actor comes from the main Viewer
  # - mapper comes from the actor
  #
  set modelNode [$::slicer3::MRMLScene GetNodeByID $::QA(modelNodeID)]
  set ::QA(polyData) [vtkPolyData New]
  $::QA(polyData) DeepCopy [$modelNode GetPolyData]
  set ::QA(actor) [vtkActor New]
  set ::QA(mapper) [vtkPolyDataMapper New]
  $::QA(mapper) SetInput $::QA(polyData)
  $::QA(actor) SetMapper $::QA(mapper)

  #
  # instrument the polydata with cell number colors
  # - note: even though the array is named CellNumberColors here,
  #   vtk will (sometimes?) rename it to "Opaque Colors" as part of the first 
  #   render pass
  #

  $::QA(polyData) Update

  set cellData [$::QA(polyData) GetCellData]
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

  set numberOfCells [$::QA(polyData) GetNumberOfCells]
  for {set i 0} {$i < $numberOfCells} {incr i} {
    eval $cellNumberColors InsertNextTuple4 [QueryAtlasNumberToRGBA $i]
  }

  set ::QA(cellData) $cellData
  set ::QA(numberOfCells) $numberOfCells

  set scalarNames {"CellNumberColors" "Opaque Colors"}
  foreach scalarName $scalarNames {
    if { [$::QA(cellData) GetScalars $scalarName] != "" } {
      $::QA(cellData) SetActiveScalars $scalarName
      break
    }
  }
  $::QA(mapper) SetScalarModeToUseCellData
  $::QA(mapper) SetScalarVisibility 1
  $::QA(mapper) SetScalarMaterialModeToAmbient
  $::QA(mapper) SetScalarMaterialModeToDiffuse
  $::QA(mapper) SetScalarRange 0 $::QA(numberOfCells)
  [$::QA(actor) GetProperty] SetAmbient 1.0
  [$::QA(actor) GetProperty] SetDiffuse 0.0


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

  $renderer AddActor $::QA(actor)
  $::QA(actor) SetVisibility 1
}


#
# re-render the picking model from the current camera location
#
proc QueryAtlasRenderView {} {

  #
  # get the renderer related instances
  #
  set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
  set renderWindow [$renderWidget GetRenderWindow]
  set renderer [$renderWidget GetRenderer]

  #
  # draw the image and get the pixels
  # - set the render parameters to draw with the cell labels
  # - draw in the back buffer
  # - pull out the pixels
  # - restore the draw state and render
  #
  $renderWindow SetSwapBuffers 0
  puts "render renderWidget"; update
  set renderState [QueryAtlasOverrideRenderState $renderer]
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
  QueryAtlasRestoreRenderState $renderer $renderState

  puts "render renderWidget"; update
  $renderWidget Render
  puts "...render done"; update

}

proc QueryAtlasOverrideRenderState {renderer} {

  #
  # save the render state before overriding it with the 
  # parameters needed for cell rendering
  #

  set actors [$renderer GetActors]
  set numberOfItems [$actors GetNumberOfItems]
  for {set i 0} {$i < $numberOfItems} {incr i} {
    set actor [$actors GetItemAsObject $i]
    set state($i,visibility) [$actor GetVisibility]
    $actor SetVisibility 0
  }

  set state(background) [$renderer GetBackground]
  $renderer SetBackground 0 0 0
  #$renderer AddActor $::QA(actor)
  $::QA(actor) SetVisibility 1

  return [array get state]
}

proc QueryAtlasRestoreRenderState {renderer renderState} {

  array set state $renderState

  #$renderer RemoveActor $::QA(actor)
  eval $renderer SetBackground $state(background)

  set actors [$renderer GetActors]
  set numberOfItems [$actors GetNumberOfItems]
  for {set i 0} {$i < $numberOfItems} {incr i} {
    set actor [$actors GetItemAsObject $i]
    $actor SetVisibility $state($i,visibility)
  }
  $::QA(actor) SetVisibility 0
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
  #puts "[format {%4d %4d} $x $y]:  [QueryAtlasRGBAToNumber $color] ($color)"


  set cell [$::QA(polyData) GetCell [QueryAtlasRGBAToNumber $color]]

  set labels [[$::QA(polyData) GetPointData] GetScalars "labels"]

  array set labelMap $::QA(labelMap)
  set pointLabels ""
  set numberOfPoints [$cell GetNumberOfPoints]

  for {set p 0} {$p < $numberOfPoints} {incr p} {
    set index [$cell GetPointId $p]
    set pointLabel [$labels GetValue $index]
    if { [info exists labelMap($pointLabel)] } {
      set labelName $labelMap($pointLabel)
      if { [lsearch $pointLabels $labelName] == -1 } {
        lappend pointLabels $labelName
      }
    } else {
      lappend pointLabels "unknown"
    }
  }
  regsub -all " " $pointLabels "/" pointLabels

  if { ![info exists ::QA(lastLabels)] } {
    set ::QA(lastLabels) ""
  }
  
  if { $pointLabels != $::QA(lastLabels) } {
    set ::QA(lastLabels) $pointLabels 
    puts $pointLabels
  }

}
