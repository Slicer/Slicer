# Performance Tests

## Overview

Module to run interactive performance tests on the core of slicer.

## Panels and their use

- **Get Sample Data**
- **Reslicing**: Go into a loop that stresses reslice by calling ``sliceNode.SetSliceOffset()``. Average time is logged and time associated with each iteration are stored in a ``vtkMRMLTableNode`` named ``Reslice performance``.
- **Crosshair Jump**: Go into a loop that stresses jumping to slices by moving crosshair using ``slicer.util.clickAndDrag()``. Average time is logged.
- **Memory Check**: Run a periodic memory check in a window.

## Contributors

- Steve Pieper (Isomics)
- Jean-Christophe Fillion-Robin (Kitware)
- Andras Lasso (PerkLab, Queen's)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
