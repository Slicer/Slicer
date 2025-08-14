# Line Profile

## Overview

The Line Profile module can compute intensity profile from an image along a line or curve.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_lineprofile.png)

## Panels and their use

- **Parameters**:
  - **Input volume**: The intensity profile will be computed from voxel values of this image.
  - **Input line**: The image will be sampled along this line or curve. Click and select `Create new line` or `Create new curve` and the click in the views.
  - **Line resolution**: Specifies how many samples will be taken from the image along the input line.
- **Advanced**:
  - **Output intensities table**: Intensity values will be stored in this table.
  - **Show plot**: If this option is enabled then any updates will automatically display the intensity plot in the view layout.
  - **Output plot series**: This series will be set up to plot the intensity values.
  - **Plot proportional distance (%)**: If this option is enabled then distance values are relative (in percentage) along the line.
- **Peak detection**:
  - **Peaks output table**: Peak detection results will be stored in this table.
  - **Minimum peak width**: Minimum width of the peak. Use higher values to reject small peaks detected due to image noise.
  - **Height for width measurement**: Height at which the peak width is measured. 50% computes full width at half maximum. Larger value means that the height is measured near the top of the peak.
  - **Peak is maximum**: If checked then peaks are local maximum values. If unchecked then peaks are local minimum values.
- **Compute intensity profile**: Click the button to compute the intensity profile. Check the checkbox in the button to enable auto-update, i.e., automatically update the intensity profile when the line or any parameter is changed.

## Contributors

Andras Lasso (PerkLab), Natalie Johnston (Revvity), James Butler (Revvity), Mike Bindschadler, Adam Aji, chir.set, Csaba Pinter (Ebatinca)

## Acknowledgements

This work was was partially funded by Revvity and Children’s Hospital of Philadelphia.
