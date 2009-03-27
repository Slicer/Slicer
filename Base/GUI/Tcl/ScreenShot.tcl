
#------------------------------------------------------------------------------
# utility to grab screenshots
#------------------------------------------------------------------------------
proc SlicerSaveLargeImage { fileName resolutionFactor } {
    
  set appGUI $::slicer3::ApplicationGUI
  set viewerWidget [$appGUI GetViewerWidget]
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






