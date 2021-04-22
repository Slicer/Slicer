# Markups

## Markups json file format (.mrk.json)

All markups node types (fiducials, line, angle, curve, etc.) can be saved to and loaded from json files.

A simple example that specifies a markups fiducial list with 3 points that can be saved to a `myexample.mrk.json` file and loaded into Slicer:

```
{"@schema": "https://raw.githubusercontent.com/slicer/slicer/master/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.0.json#",
"markups": [{"type": "Fiducial", "coordinateSystem": "LPS", "controlPoints": [
    { "label": "F-1", "position": [-53.388409961685827, -73.33572796934868, 0.0] },
    { "label": "F-2", "position": [49.8682950191571, -88.58955938697324, 0.0] },
    { "label": "F-3", "position": [-25.22749042145594, 59.255268199233729, 0.0] }
]}]}
```

All elements and properties are specified in this [JSON schema](https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.0.json).


## Markups fiducial point list file format (.fcsv)

The Markups Fiducial storage node uses a comma separated value file to store the fiducials on disk. The format is:

A leading comment line giving the Slicer version number:

    # Markups fiducial file version = 4.11

A comment line giving the coordinate system. In earlier versions of Slicer, numeric codes were used: RAS = 0, LPS = 1.

    # CoordinateSystem = LPS

A comment line explaining the fields in the csv

    # columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID

Then comes the fiducials, one per line, for example:

    vtkMRMLMarkupsFiducialNode_0,1.29,-40.18,60.15,0,0,0,1,1,1,0,F-1,,

- id = a string giving a unique id for this fiducial, usually based on the class name
- x,y,z = the floating point coordinate of the fiducial point
- ow,ox,oy,oz = the orientation quaternion of this fiducial, angle and axis, default 0,0,0,1 (or 0,0,0,0,0,0,1.0)
- vis = the visibility flag for this fiducial, 0 or 1, default 1
- sel = the selected flag for this fiducial, 0 or 1, default 1
- lock = the locked flag for this fiducial, 0 or 1, default 0
- label = the name for this fiducial, displayed beside the glyph, with quotes around it if there is a comma in the field
- desc = a string description for this fiducial, optional
- associatedNodeID = an id of a node in the scene with which the fiducial is associated, for example the volume or model on which the fiducial was placed, optional

## References

- [History and design considerations](https://www.slicer.org/wiki/Documentation/Labs/Improving_Markups)

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.html#markups).
