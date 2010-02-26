
#------------------------------------------------------------------------------
# utility to grab screenshots
#------------------------------------------------------------------------------
proc SlicerSaveLargeImage { fileName resolutionFactor } {
    
  set appGUI $::slicer3::ApplicationGUI
  set viewerWidget [$appGUI GetActiveViewerWidget]
  set mainViewer [$viewerWidget GetMainViewer]
  set window [ $mainViewer GetRenderWindow ]
  set renderer [$mainViewer GetRenderer]

  #
  # commented out because vtkRenderLargeImage was
  # not handling vtkActors properly (was copying them
  # into each tile rather than magnifiying them once.
  #
  #set renderLargeImage [vtkRenderLargeImage New]
  #$renderLargeImage SetInput $renderer
  #$renderLargeImage SetMagnification $resolutionFactor
  set renderLargeImage [ vtkWindowToImageFilter New ]
  $renderLargeImage SetInput $window
  $renderLargeImage SetMagnification $resolutionFactor

  switch { [string tolower [file ext $fileName]] } {
    "png" {
      set writer [vtkPNGWriter New]
    }
    "jpg" - "jpeg" {
      set writer [vtkJPEGWriter New]
    }
    "tif" - "tiff" {
      set writer [vtkTIFFWriter New]
    }
    "eps" - "ps" - "prn" {
      set writer [vtkPostscriptWriter New]
    }
    "pnm" - "ppm" {
      set writer [vtkPNMWriter New]
    }
    default {
      set writer [vtkPNGWriter New]
    }
  }

  $writer SetInput [$renderLargeImage GetOutput]
  $writer SetFileName $fileName
  $writer Write

  $renderLargeImage Delete
  $writer Delete
  # put the renderer back to normal
  $mainViewer Render
}


proc SlicerSaveEachRenderCancel { {renderer ""} } {
  if { $renderer == "" } {
    set appGUI $::slicer3::ApplicationGUI
    set viewerWidget [$appGUI GetActiveViewerWidget]
    set mainViewer [$viewerWidget GetMainViewer]
    set window [ $mainViewer GetRenderWindow ]
    set renderer [$mainViewer GetRenderer]
  }
  $renderer RemoveObserver $::SLICERSAVE($renderer,observerTag)
}

proc SlicerSaveEachRender { {fileNamePattern /tmp/slicer-%d.png} {resolutionFactor 1} } {

  set appGUI $::slicer3::ApplicationGUI
  set viewerWidget [$appGUI GetActiveViewerWidget]
  set mainViewer [$viewerWidget GetMainViewer]
  set window [ $mainViewer GetRenderWindow ]
  set renderer [$mainViewer GetRenderer]

  if { [info exists ::SLICERSAVE($renderer,observerTag)] } {
    SlicerSaveEachRenderCancel $renderer
  }
  set ::SLICERSAVE($renderer,observerTag) [$renderer AddObserver EndEvent "SlicerSaveEachRenderCallback $renderer"]

  set ::SLICERSAVE($renderer,fileNamePattern) $fileNamePattern
  set ::SLICERSAVE($renderer,resolutionFactor) $resolutionFactor
  set ::SLICERSAVE($renderer,frameNumber) 0
  set ::SLICERSAVE($renderer,saving) 0
}

proc SlicerSaveEachRenderCallback { renderer } {
  if { $::SLICERSAVE($renderer,saving) } {
    # don't do a screen grab triggered by our own render
    puts "skipping"
    return
  }
  set ::SLICERSAVE($renderer,saving) 1

  set fileName [format $::SLICERSAVE($renderer,fileNamePattern) $::SLICERSAVE($renderer,frameNumber)]
  incr ::SLICERSAVE($renderer,frameNumber)

  SlicerSaveLargeImage $fileName $::SLICERSAVE($renderer,resolutionFactor)
  set ::SLICERSAVE($renderer,saving) 0
}




