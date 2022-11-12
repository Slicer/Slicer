# Crop volume sequence

## Overview

This module can crop&resample all the volumes of a volume sequence using the same region.

## Panels and their use

- **Parameters**:
  - **Input volume sequence**: sequence node that contain volume nodes that will be cropped
  - **Output volume sequence**: sequence node to store the cropped input volume sequence
  - **Crop volume settings**: crop&resample settings. Click the green arrow button to go to the [Crop volume](cropvolume.md) module to edit these settings.

## Related extensions and modules

This module internally uses [Crop volume](cropvolume.md) module to apply crop&resample operation on each volume of the sequence.

## Contributors

- Andras Lasso (PerkLab)

## Acknowledgements

This work was partially supported by Department of Anesthesia and Critical Care at The Children’s Hospital of Philadelphia (CHOP).
