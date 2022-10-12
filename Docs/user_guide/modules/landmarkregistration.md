# Landmark Registration

## Overview

This module aligns two images based on a set of corresponding landmarks (paired points).

## How to

### Linear registration

- Load the two images to register
- Enter the Landmark Registration module
- Select the two images as fixed and moving volumes (do not select transformed volume)
- Scroll to the `Registration` area and select `Affine registration`
- Pick Axi/Sag/Cor in the Visualization box (this will create a custom layout with fixed on top, moving in the middle, and fixed + transformed on the bottom)
- Place a point on either the fixed or moving volumes (a corresponding one will be created on the other volume)
- Drag the points in the fixed and moving volumes until they are on the same anatomical location. The blended view will update automatically on mouse release.
- Place and adjust points until registration is good.
- Optional: set `Local Refinement Method` to `Local SimpleITK` (it tends to be more robust than `Local BRAINSFit`) and click on the `Refine landmark ...` button.

### Chose registration type

Similarity mode is Rigid + Scale and can be good for some cross-subject registration.

Affine mode requires more landmarks but should work.

Thin-Plate spline mode works but does not automatically update (click Apply to calculate). It overwrites the transformed volume so you can't go back to Linear mode from Thin-Plate mode.

## Panels and their use

- **Fixed volume** and **Moving volume**: the computed transformation will be computed to transform the moving image into the fixed image.
- **Transformed volume**: output image, obtained by applying the computed transform to the moving volume and resampled to the fixed volume.
- **Target transform**: computed transform that aligns the moving volume with the fixed volume.
- **Visualization**: view layout and view mode to visualize the alignment.
- **Landmarks**: list of corresponding landmark points.
- **Local refinement**: automatically adjust the position of a placed landmark point in the moving image. `Local SimpleITK` method tends to be more robust.
- **Registration type**: choose between linear transform (`Affine Registration`) and warping transform (`ThinPlate Registration`).

## Contributors

- Steve Pieper (Isomics)

## Acknowledgements

This file was originally developed by Steve Pieper, Isomics, Inc. It was partially funded by NIH grant 3P41RR013218-12S1 and P41 EB015902 the Neuroimage Analysis Center (NAC) a Biomedical Technology Resource Center supported by the National Institute of Biomedical Imaging and Bioengineering (NIBIB).
And this work is part of the "National Alliance for Medical Image Computing" (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
Information on the National Centers for Biomedical Computing can be obtained from http://nihroadmap.nih.gov/bioinformatics.
This work is also supported by NIH grant 1R01DE024450-01A1 "Quantification of 3D Bony Changes in Temporomandibular Joint Osteoarthritis" (TMJ-OA).
