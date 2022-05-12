# Markups

## Markups json file format (.mrk.json)

All markups node types (point list, line, angle, curve, etc.) can be saved to and loaded from json files. Detailed specification of all elements of the file is available in the [JSON schema](https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Markups/Resources/Schema).

A simple example that specifies a markups point list with 3 points that can be saved to a `myexample.mrk.json` file and loaded into Slicer:

```
{"@schema": "https://raw.githubusercontent.com/slicer/slicer/master/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.0.json#",
"markups": [{"type": "Fiducial", "coordinateSystem": "LPS", "controlPoints": [
    { "label": "F-1", "position": [-53.388409961685827, -73.33572796934868, 0.0] },
    { "label": "F-2", "position": [49.8682950191571, -88.58955938697324, 0.0] },
    { "label": "F-3", "position": [-25.22749042145594, 59.255268199233729, 0.0] }
]}]}
```

## Markups fiducial point list file format (.fcsv)

vtkMRMLMarkupsFiducialStorageNode uses a comma separated value file with a custom header to store the control points on disk. A simple example:

```
# Markups fiducial file version = 4.13
# CoordinateSystem = LPS
# columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID
0,-19.906699999999987,13.9347,29.442970822281154,0,0,0,1,1,1,0,F-1,,
1,-7.3939,-76.94990495817181,17.552540297898375,0,0,0,1,1,1,0,F-2,,
2,81.73332450520303,-42.9415,9.625586614976527,0,0,0,1,1,1,0,F-3,,
```

File header:
- Line 1: a comment line specifying the Slicer version that created the file
- Line 2: a comment line specifying the coordinate system (`CoordinateSystem = LPS` or `CoordinateSystem = RAS`). In earlier versions of Slicer, numeric codes were used: RAS = 0, LPS = 1.
- Line 3: a comment line explaining the fields in the csv (`columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID`)

Each line after the header specifies a control point. Meaning of columns:
- id: a string giving a unique id for this control point, usually based on the class name
- x,y,z: the floating point coordinate of the control point
- ow,ox,oy,oz: the orientation quaternion of this control point, angle and axis, default 0,0,0,1 (or 0,0,0,0,0,0,1.0)
- vis: the visibility flag for this control point, 0 or 1, default 1
- sel: the selected flag for this control point, 0 or 1, default 1
- lock: the locked flag for this control point, 0 or 1, default 0
- label: the name for this control point, displayed beside the glyph, with quotes around it if there is a comma in the field
- desc: a string description for this control point, optional
- associatedNodeID = an id of a node in the scene with which the control point is associated, for example the volume or model on which the control point was placed, optional

## Markups control points table file format (.csv, .tsv)

Markups control points can be imported from and exported to a table node that can be written to/read from standard comma (or tab) separated file format. A simple example:

```
label,l,p,s,defined,selected,visible,locked,description
F-1,-19.9067,13.9347,29.443,1,1,1,0,
F-2,-7.3939,-76.9499,17.5525,1,1,1,0,
F-3,81.7333,-42.9415,9.62559,1,1,1,0,
```

Definition of columns:
- label: label that is displayed next to each control point
- l, p, s: coordinate values in LPS coordinate system (in this case, l and a columns should not be used)
- r, a, s: coordinate values in RAS coordinate system (in this case, r and a columns should not be used)
- defined: 0 = the position of the point is not defined (coordinate values can be ignored; useful for creating templates); 1: the position is defined
- selected: 0 = unselected; 1 = selected (the control point appears with different colors an may be used as additional input for analysis)
- visible: 0 = hidden; 1 = visible
- locked: 0 = the point can be interactively moved; 1 = the point position is locked
- description: text providing additional information for the point

## References

- [History and design considerations](https://www.slicer.org/wiki/Documentation/Labs/Improving_Markups)

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.md#markups).
