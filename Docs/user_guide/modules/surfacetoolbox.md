# Surface Toolbox

## Overview

This module supports various cleanup and optimization processes on surface models.

## Panels and their use

Select the input and output models, and then enable the stages of the pipeline by selecting the tool buttons.

Stages that include parameters will open up when they are enabled.

Click apply to activate the pipeline and then click the Toggle button to compare the model before and after the operation.

The module includes tools for:

- [Surface model cleaning](#clean)
- [Surface model uniform remeshing](#uniform-remeshing)
- [Surface model decimation](#decimate) (reduction of the number of triangles).
- [Surface model smoothing](#smooth)
- [Surface model holes filling](#fill-holes)
- [Surface model normal computation](#compute-surface-normals)
- [Surface model mirroring](#mirror)
- Surface model geometric transformations: [Scale](#scale) and [Translate](#translate)
- [Edge extraction](#extract-edges)
- [Extraction of the largest component in the surface model](#extract-largest-component)

These tools can be combined for multiple surface model processing effects.

### Clean

This tool can merge coincident points, remove unused points (i.e. not used by
any cell) and treat degenerate cells.

### Uniform remeshing

Uniformly remesh the surface using [ACVD algorithm](https://github.com/pyvista/pyacvd).

This resampling typically provides higher quality meshes than decimation, with similar computation time.

- **Number of points:** Number of desired points in the output mesh. Use higher number to preserve more details.
- **Subdivide:** Number of subdivision to perform before remeshing. Each subdivision creates 4 triangles for each input triangle. This is needed if the required number of desired points is higher than the number of points in the input mesh, or there are some too large cells in the input mesh.

:::{note}
It requires `pyacvd` Python package for which the user is required to confirm the installation.
:::

### Decimate

Perform a topology-preserving reduction of surface triangles. The user can
modify the following parameters:

- **Reduction:** Target reduction factor during decimation. Ratio of triangles that are requested to be eliminated. 0.8 means that the surface model size is requested to be reduced by 80%.
- **Boundary deletion:** If enabled then *FastQuadric* method is used (it provides more even element sizes but cannot be forced to preserve boundary), otherwise *DecimatePro* method is used (that can preserve boundary edges but tend to create more ill-shaped triangles).

### Smooth

Performs surface model smoothing on a surface model according to the following parameters:

- **Method:** Selects the smoothing method: either a *Laplacian* filter or *Taubin's non-shrinking* algorithm.
- **Iterations:** Number of smoothing iterations.
- **Pass band:** The pass-band value for the windowed sinc filter in the Taubin's non-shrinking algorithm. This should be between 0 and 2, where lower values cause more smoothing.
- **Boundary smoothing:** If enabled, boundary edges will be smoothed. Otherwise, the edges will remain fixed.

### Fill holes

Fills up a hole in a open surface model.

- **Maximum hole size:** Specifies the maximum size of holes that will be filled. This is represented as a radius to the bounding circumsphere containing the hole. Note that this is an approximate area; the actual area cannot be computed without first triangulating the hole.

### Compute surface normals

Generate surface normals for geometry algorithms or for improving visualization through lighting.

- **Auto orient normals:** Orient normals outwards from a closed surface.
- **Flip normals:** Flip normal direction from its current or auto-oriented state.
- **Splitting:** Allow sharp change in normals where angle between neighbor faces is above a threshold (indicated feature angle).
  - **Feature angle for splitting:** Normals will be split only along those edges where angle is larger than this value.

### Mirror

Mirror the surface model along one or more axes.

- **X axis:** Enable/disable the use of the X axis for the mirroring operation.
- **Y axis:** Enable/disable the use of the Y axis for the mirroring operation.
- **Z axis:** Enable/disable the use of the Z axis for the mirroring operation.

### Scale

Performs a scaling transformation on the surface. This transformation can be non-uniform
(different scaling factors for different axes).

- **Scale X:** Scaling factor along the X axis.
- **Scale Y:** Scaling factor along the Y axis.
- **Scale Z:** Scaling factor along the Z axis.

### Translate

Performs a translation transformation on the surface.

- **Center:** enables/disables centering the model in the origin (this is done by translating the center of the object's bounding box to the origin) before performing the translation.

- **Translate X:** specifies the translation along the X axis.
- **Translate Y:** specifies the translation along the Y axis.
- **Translate Z:** specifies the translation along the Z axis.

### Extract edges

Extract the edges of the surface model. The extraction can be controlled with the
following parameters:

- **Boundary edges:** Enable/disable the extraction of boundary edges (edges used only by one polygon or a line cell).
- **Feature edges:** Enable/disable the extraction of feature edges (edges used by two triangles and whose dihedral angle is larger than the specified feature angle).
- **Feature angle:** Minimum angle to consider an edge to be a feature edge.
- **Manifold edges:** Enable/disable the extraction of manifold edges (edges used exactly by two polygons).
- **Non-manifold edges:** Enable/disable the extraction of non-manifold edges (edges used by three or more polygons).

### Extract largest component

Enable the extraction of the largest connected component.

## Contributors

- Luca Antiga (Orobix)
- Steve Pieper (BWH)
- Beatriz Paniagua (Kitware)
- Martin Styner (UNC)
- Hans Johnson (University of Iowa)
- Ben Wilson (Kitware)
- Jean-Christophe Fillion-Robin (Kitware)
- Andras Lasso (PerkLab, Queen's University)
- Sara Rolfe (Seattle Children's Research Institute)

## Acknowledgments

Development was funded in part by NIH National Institute of Biomedical Imaging Bioengineering R01EB021391 (Shape Analysis Toolbox for Medical Image Computing Projects) and by the NSF National Science Foundation under Grant DBI-1759883 (An Integrated Platform for Retrieval, Visualization and Analysis of 3D Morphology From Digital Biological Collections).





