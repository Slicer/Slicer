# Volume rendering

## Key classes
- [vtkMRMLVolumeRenderingDisplayNode](https://apidocs.slicer.org/main/classvtkMRMLVolumeRenderingDisplayNode.html) controls the volume rendering properties. Each volume rendering technique has its own subclass.
- [vtkSlicerVolumeRenderingLogic](https://apidocs.slicer.org/main/classvtkSlicerVolumeRenderingLogic.html) contains utility functions
- [vtkMRMLScalarVolumeNode](https://apidocs.slicer.org/main/classvtkMRMLScalarVolumeNode.html) contains the volume itself
- [vtkMRMLVolumePropertyNode](https://apidocs.slicer.org/main/classvtkMRMLVolumePropertyNode.html) points to the transfer functions
- [vtkMRMLMarkupsROINode](https://apidocs.slicer.org/main/classvtkMRMLMarkupsROINode.html) controls the clipping planes
- [vtkMRMLVolumeRenderingDisplayableManager](https://apidocs.slicer.org/main/classvtkMRMLVolumeRenderingDisplayableManager.html) responsible for adding VTK actors to the renderers

## Format of .vp.json (volume property JSON)

Since Slicer version 5.9, volume rendering properties are stored in JSON format by default, with the file extension `.vp.json`. This format is self-describing and easier to interpret than the legacy text file format. Details are specified by this JSON schema: https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/VolumeRendering/Resources/Schema/volume-property-schema-v1.0.0.json

Example `.vp.json` file:

```json
{
    "@schema": "https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/VolumeRendering/Resources/Schema/volume-property-schema-v1.0.0.json#",
    "volumeProperties": [
        {
            "effectiveRange": [0.0, 220.0],
            "components": [
                {
                    "shade": true,
                    "lighting": {
                        "diffuse": 1.0,
                        "ambient": 0.2,
                        "specular": 0.0,
                        "specularPower": 1.0
                    },
                    "rgbTransferFunction": {
                        "type": "colorTransferFunction",
                        "points": [
                            { "x": 0.0, "color": [0.0, 0.0, 0.0] },
                            { "x": 20.0, "color": [0.168627, 0.0, 0.0] },
                            { "x": 40.0, "color": [0.403922, 0.145098, 0.0784314] },
                            { "x": 40.0, "color": [0.403922, 0.145098, 0.0784314] },
                            { "x": 120.0, "color": [0.780392, 0.607843, 0.380392] },
                            { "x": 220.0, "color": [0.847059, 0.835294, 0.788235] },
                            { "x": 1024.0, "color": [1.0, 1.0, 1.0] }
                        ]
                    },
                    "scalarOpacityUnitDistance": 1.0,
                    "scalarOpacity": {
                        "type": "piecewiseLinearFunction",
                        "points": [
                            { "x": 0.0, "y": 0.0 },
                            { "x": 120.0, "y": 0.3 },
                            { "x": 220.0, "y": 0.375 },
                            { "x": 1024.0, "y": 0.5 }
                        ]
                    },
                    "gradientOpacity": {
                        "type": "piecewiseLinearFunction",
                        "points": [
                            { "x": 0.0, "y": 1.0 },
                            { "x": 255.0, "y": 1.0 }
                        ]
                    }
                }
            ]
        }
    ]
}

```

## Format of Volume Property (.vp) file

Legacy text file format for storing volume properties. It is only supported for backward compatibility and it is recommended to use the .vp.json file format instead.

Example:

```
1 => interpolation type
1 => shading enabled
0.9 => diffuse reflection
0.1 => ambient reflection
0.2 => specular reflection
10 => specular reflection power
14 -3024 0 -86.9767 0 45.3791 0.169643 139.919 0.589286 347.907 0.607143 1224.16 0.607143 3071 0.616071 => scalar opacity transfer function (total number of values, each control point is defined by a pair of values: intensity and opacity)
4 0 1 255 1 => gradient opacity transfer function (total number of values, each control point is defined by a pair of values: intensity gradient and opacity)
28 -3024 0 0 0 -86.9767 0 0.25098 1 45.3791 1 0 0 139.919 1 0.894893 0.894893 347.907 1 1 0.25098 1224.16 1 1 1 3071 0.827451 0.658824 1 => color transfer function (total number of values, each control point is defined by 4 of values: intensity and R, G, B color components)
```

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.md#volumes).

## Volume rendering presets

Volume rendering presets that are bundled in Slicer core are specified in a preset file and corresponding icon is stored as an application resource.

To add a new volume rendering preset:
- Add a new entry to [presets.xml](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/VolumeRendering/Resources/presets.xml)
- Add a corresponding icon of 128x100 pixels into [presets icons folder](https://github.com/Slicer/Slicer/tree/main/Modules/Loadable/VolumeRendering/Resources/Icons).
- Add the icon to [qSlicerVolumeRenderingModule.qrc](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/VolumeRendering/Resources/qSlicerVolumeRenderingModule.qrc)

Presets can be also added at runtime - see [example in the script repository](/developer_guide/script_repository.md#register-custom-volume-rendering-presets). In this case, icons are loaded from the preset scene file.
