# Vector to Scalar Volume

## Overview

This module is useful for converting RGB (vector) volumes to one component (scalar) grayscale volumes.

Vector volumes are typically loaded from a series of `jpg`, `png`, `bmp`, or other classic 2D image formats.

While vector volumes can be visualized in the slice viewers, many slicer operations only operate on scalar volumes (such as segmentation and registration modules).

## Panels and their use

- **Parameters**:
  - **Input vector volume**
  - **Conversion Method**:
    - **Luminance**: convert RGB images to scalar using luminance as implemented in vtkImageLuminance (scalar = 0.30*R + 0.59*G + 0.11*B).
    - **Average**: computes the mean of all the components.
    - **Single Component Extraction**: extract single components from any vector image.
  - **Output scalar volume**


| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_vectortoscalarvolume_visiblehuman_luminance_2012_08_18.png) |
|--|
| Volume of JPG files from the [visible human project](https://www.nlm.nih.gov/research/visible/visible_human.html) converted to a scalar volume using the **Luminance** method. |

## Related extensions and modules

- The [SlicerMorph extension's ImageStacks module](https://github.com/SlicerMorph/SlicerMorph#module-descriptions) supports reading and other operations on volumetric vector (color) image formats.

## Contributors

- Steve Pieper (Isomics)
- Pablo Hernandez-Cerdan (Kitware)
- Jean-Christophe Fillion-Robin (Kitware)
- Andras Lasso (PerkLab)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on NA-MIC can be obtained from the [NA-MIC website](https://www.na-mic.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
