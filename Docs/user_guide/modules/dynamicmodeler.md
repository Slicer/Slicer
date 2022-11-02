# Dynamic Modeler

## Overview

This is a module that provides an extensible framework for automatic processing of mesh nodes by executing “Tools” on the input to generate output mesh. Output of a tool can be used as input in another tool, which allows specification of complex editing operations. This is similar to “parametric editing” in engineering CAD software, but this module is specifically developed to work well on complex meshes used in biomedical applications (while most engineering CAD software does not directly support parametric editing of complex polygonal meshes).

See examples of potential applications using the Dynamic Modeler module [here](https://www.youtube.com/watch?v=F6fNMQTxD-4).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_dynamic_modeler.png)

## How to use

To create a new tool node, switch to the Dynamic Modeler module, and click on one of the tool buttons along the top of the module representing the tool that you would like to create.

Within the "Input nodes" section, select all required nodes. Note that some inputs can be selected multiple times, denoted by the `[#]` following the input name.

Within the "Parameters" section, you can adjust all parameters as needed.

Within the "Output nodes" section, select all desired outputs. At least one output must be selected.

Click on the Apply button to run the tool once, or check the checkbox on the Apply button to enable automatic update (so that outputs are automatically recomputed whenever inputs change). Tools cannot be run continuously if one of the input nodes is present in the output. The tool can still be run on demand by clicking the apply button.

## Tools

### Plane cut
Cut a plane into two separate meshes using any number of markup planes or slice views. The planes can be combined using union, intersection and difference boolean operation.
#### Input nodes
- `Model node`: The surface model to be cut.
- `Plane node (repeatable)`: The planes used to cut the model.
#### Parameters
- `Cap surface`: If enabled, creates a closed surface by triangulating the clipped region.
- `Operation type`: The method that will be used to combine multiple planes for cutting (union, intersection and difference boolean operations).
#### Output nodes
- `Clipped output model (positive side)`: Portion of the cut model that is on the same side of the plane as the normal.
- `Clipped output model (negative side)`: Portion of the cut model that is on the opposite side of the plane as the normal.

### Curve cut
Extracts a region from the surface that is enclosed by a markup curve.
#### Input nodes
- `Model`: The surface model to be cut.
- `Curve`: The curve node (open or closed) that will be used to cut the model.
- `Inside point (optional)`: Points list node defining the region that will be considered the inside. If not defined, then the smallest region will be defined as the inside.
#### Parameters
- `Straight cut`: If enabled, the surface will be cut as close to the curve as possible. Otherwise the original edges will be preserved.
#### Output nodes
- `Inside model`: The cut region of the original surface model that represents the inside region.
- `Outside model`: The cut region of the original surface model that represents the outside  region.

### Boundary cut
Extracts a region from the surface that is enclosed by many markup curves and planes. In instances where there is ambiguity about which region should be extracted, a markup fiducial can be used to specify the region of interest.
#### Input nodes
- `Model node`: The surface model to be cut.
- `Border node (repeatable)`: Plane or curve nodes that define the boundaries of the regions to be cut.
- `Seed point node (optional)`: Points list node defining the regions that will be preserved in the output. If no points are defined, then the preserved region will be the region that is near the center of the defined boundaries.
#### Output nodes
- `Model node`: The region of the surface mesh that was cut by the specified boundaries.

### Hollow
Create a shell from the surface of the model, effectively making it hollow.
#### Input nodes
- `Model`: The surface model to be converted to a shell.
#### Parameters
- `Shell thickness`: The thickness in millimeters of the resulting shell.
#### Output nodes
- `Hollowed model`: The output shell model.

### Margin
Expands or shrinks a model by the specified margin.

#### Input nodes
- `Model`: Model to grow or shrink by the specified margin. Surface normals must be computed for the model, for example using Surface Toolbox module.
:::{note}
Requires input models with precomputed surface normals.

Normals can be computed using SurfaceToolbox module or in Python scripting using the `vtkPolyDataNormals` VTK filter.

May create self-intersecting mesh if the margin value is large or the model has sharp edges.
:::
#### Parameters
- `Margin`: If positive value is specified then the model will be expanded by this much towards the surface normal, if negative then the model will be shrunken. Keep the absolute value low to avoid self-intersection.
#### Output nodes
- `Output model`: Model generated by growing or shrinking the input model.

### Mirror
Reflects the points in a model across the input plane. Useful in conjunction with the plane cut tool to cut a model in half and then mirror the selected half across the cutting plane.
#### Input nodes
- `Model`: Surface model to be mirrored.
- `Mirror plane`: Plane that the input model will be mirrored across.
#### Output nodes
- `Mirrored model`: Model mirrored across the plane.

### Append
Combine multiple models into a single output model node.
#### Input nodes
- `Model (repeatable)`: Models to be appended in the output.
#### Output nodes
- `Appended model`: Output model combining the input models.

### ROI cut
Clips a vtkMRMLModelNode and returns the region of the model that is inside or outside the ROI. The tool can also add caps to the clipped regions to maintain a closed surface.
#### Input nodes
- `Model node`: Model node to be cut.
- `ROI node`: ROI node to cut the model node.
#### Parameters
- `Cap surface`: If enabled, create a closed surface by triangulating the clipped region.
#### Output nodes
- `Clipped output model (inside)`: Portion of the cut model that is inside the ROI.
- `Clipped output model (outside)`: Portion of the cut model that is outside the ROI.

### Select by points
Allows selecting region(s) of a model node by specifying by markups fiducial points. Model points that are closer to the points than the specified selection distance are selected.
#### Input nodes
- `Model node`: Model node to select faces from.
- `Fiducials node`: Fiducials node to make the selection of model's faces.
#### Parameters
- `Selection distance`: Selection distance of model's points to input fiducials in millimeters.
- `Selection algorithm`: Method used to calculate points distance to seeds. "SphereRadius" method uses straight line distance. "GeodesicDistance" method uses distance on surface. Geodesic distance is computed using code from Krishnan K. "Geodesic Computations on Surfaces." article published in the [VTK journal in June 2013](https://www.vtkjournal.org/browse/publication/891).
#### Output nodes
- `Model with selection scalars`: All model points have a selected scalar value that is 0 or 1.
- `Model of the selected cells`: Model that only contains the selected faces of the input model.

## Information for developers

See examples for using Dynamic Modeler tools in the [Script repository](../../developer_guide/script_repository.md#create-a-hollow-model-from-boundary-of-solid-segment).

## Contributors

Authors:
- Kyle Sunderland (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Jean-Christophe Fillion-Robin (Kitware)
- Mauro I. Dominguez
- Csaba Pinter (Ebatinca)

## Acknowledgements

This module was originally developed with support from CANARIE's Research Software Program, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
