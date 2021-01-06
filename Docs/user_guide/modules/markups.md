# Markups

## Overview

This module is used to create and edit markups (fiducial list, line, angle, curve, plane, etc.) and adjust their display properties.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_types.png)

## How to

### Place new markups

1. Click "Create and place" button on the toolbar to activate place mode.

Click the down-arrow icon on the button to choose markups type.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_place_toolbar.png)

Click "Persistent" checkbox to keep placing more points after the current markups is completed.

2. Left-click in the slice or 3D views to place points

3. Right-click to finish point placement

### Edit point positions in existing markups

- Make sure that the correct markups node is selected in Markups module.
- Left-click-and drag a control point to move it
- Left-click a control point to show it in all slice viewers. This helps in adjusting its position along all axes.
- Right-click to delete or rename a point or change node properties.
- Ctrl + Left-click to place a new point on a curve.
- Enable Display / Interaction / Visible to show a widget that allows translation/rotation of the entire widget.

### Edit properties of a markups node that is picked in a view

To pick a markups node in a viewer so that its properties can be edited in Markups module, right-click on it in a slice or 3D view and choose "Edit properties".

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_context_menu_properties.png)

## Panels and their use

- Create: click on any of the buttons to create a new markup. Click in any of the viewers to place markup points.
- Markups list: Select a markups node to populate the GUI and allow modifications. When markup placement is activated on the toolbar, then points are added to this selected markups node.

Display section:
- Visibility: Toggle the markups visibility, which will override the individual markup visibility settings. If the eye icon is open, the list is visible, and pressing it will make it invisible. If the eye icon is closed, the list is invisible and pressing the button will make it visible.
- Opacity: overall opacity of the selected markups.
- Glyph Size: Set control point glyph size relative to the screen size (if `absolute` button is not pressed) or as an absolute size (if `absolute` button is depressed).
- Text Scale: Label size relative to screen size.
- Interaction: check `Visible` to enable translation of the entire markups in slice and 3D views with an interactive widget.
- Advanced:
  - View: select which views the markups is displayed in
  - Selected Color: Select the color that will be used to display the glyph and text when the markup is marked as selected.
  - Unselected Color: Select the color that will be used to display the glyph and text when the markup is not marked as selected.
  - Glyph Type: Select the symbol that will be used to mark each location. Default is the Sphere3D.
  - Point Labels: check to display label for each control point.
  - Fiducial projection: A widget that controls the projection of the fiducials in the 2D viewers onto slices around the one on which the fiducial has been placed.
    - 2D Projection: Toggle the eye icon open or closed to enable or disable visualization of projected fiducial on 2D viewers.
    - Use Fiducial Color: If checked, color the projections the same color as the fiducials.
    - Projection Color: If not using fiducial color for the projection, use this color.
    - Outlined Behind Slice Plane: Fiducial projection is displayed filled (opacity = Projection Opacity) when on top of slice plane, outlined when behind, and with full opacity when in the plane. Outline isn't used for some glyphs (Dash2D, Cross2D, Starburst).
    - Projection Opacity: A value between 0 (invisible) and 1 (fully visible) for displaying the fiducial projection.
    - Reset to Defaults: Reset the display properties of this markups node to the system defaults.
    - Save to Defaults: Save the display properties of this markups node to be the new system defaults.
- Scalars: Color markup according to a scalar, e.g. a per-control-point measurement (see Measurements section below)
  - Visible: Whether scalar coloring should be shown or the original color of the markup
  - Active Scalar: Which scalar array to use for coloring
  - Color Table: Palette used for coloring
  - Scalar Range Mode: Method for determining the range of the scalars (automatic range calculation based on the data is the default)

Control points section:
- Interaction in views: toggle the markups lock state (if it can be moved by mouse interactions in the viewers), which will override the individual markup lock settings.
- Click to Jump Slices: If checked, click in name column to jump slices to that point. The radio buttons control if the slice is centered on the markup or not after the jump. Right click in the table allows jumping 2D slices to a highlighted fiducial (uses the center/offset radio button settings). Once checked, arrow keys moving the highlighted row will also jump slice viewers to that markup position.
- Buttons: These buttons apply changes to markups in the selected list.
  - Toggle visibility flag: Toggle visibility flag on all markups in list. Use the drop down menu to set all to visible or invisible.
  - Toggle selected flag: Toggle selected flag on all markups in list. Use the drop down menu to set all to selected or deselected. Only selected markups will be passed to command line modules.
  - Toggle lock flag: Toggle lock flag on all markups in list, use drop down menu to set all to locked or unlocked.
  - Delete the highlighted markups from the active list: After highlighting rows in the table to select markups, press this button to delete them from the list.
  - Remove all markups from the active list: Pressing this button will delete all of the markups in the active list, leaving it with a list length of 0.
  - Transformed: Check to show the transformed coordinates in the table. This will apply any transform nodes to the points in the list and show that result. Keep unchecked to show the raw RAS values that are stored in MRML. If you harden the transform the transformed coordinates will be the same as the non transformed coordinates.
  - Hide RAS: Check to hide the coordinate columns in the table and uncheck to show them. Right click in rows to see coordinates.
- Control points table: Right click on rows in the table to bring up a context menu to show the full precision coordinates, distance between multiple highlighted fiducials, delete the highlighted markup, jump slice viewers to that location, refocus 3D viewers to that location, or if there are other lists, to copy or move the markup to another list.
  - Selected: A check box is shown in this column, it's check state depends on if the markup is selected or not. Click to toggle the selected state. Only selected markups will be passed to command line modules.
  - Locked: An open or closed padlock is shown in this column, depending on if the markup is unlocked or locked. Click it to toggle the locked state.
  - Visibility: An open or closed eye icon is shown in this column, depending on if the markup is visible or not. Click it to toggle the visibility state.
  - Name: A short name for this markup, displayed in the viewers as text next to the glyphs.
  - Description: A longer description for this markup, not displayed in the viewers.
  - X, Y, Z: The RAS coordinates of this markup, 3 places of precision are shown.
- Advanced section:
  - Move Up: Move a highlighted markup up one spot in the list.
  - Move Down: Move a highlighted markup down one spot in the list.
  - Add Markup: Add a new markup to the selected list, placing it at the origin
  - Naming:
    - Name Format: Format for creating names of new markups, using sprintf format style. %N is replaced by the list name, %d by an integer.
    - Apply: Rename all markups in this list according to the current name format, trying to preserve numbers. A quick way to re-number all the fiducials according to their index is to use a name format with no number in it, rename, then add the number format specifier %d to the format and rename one more time.
    - Reset: Reset the name format field to the default value, %N-%d.

Measurements section:
- This section lists the available measurements of the selected markup
  - `length` for line and curve
  - `angle` for angle markups
  - `curvature mean` and `curvature max` for curve markups
- In the table below the measurement descriptions, the measurements can be enabled/disabled
  - Basic measurements (e.g. length, angle) are enabled by default
  - Curve markups support curvature calculation, which is off by default
    - When turned on, the curvature data can be displayed as scalar coloring (see Display/Scalars above)

Curve settings section:
- Curve type:
  - linear: control points are connected with straight line
  - spline, Kochanek spline: smooth interpolating curve
  - polynomial: smooth approximating curve
  - shortest distance on surface: curve points are forced to be on the selected model's surface, connected with a minimal-cost path
- Surface: surface used for `shortest distance on surface` curve type and cost function that is minimized to find path connecting two control points

Resample section: Replace control points by curve points sampled at equal distances. If a model node is selected for `Constrain points to surface` then the resampled points will be projected to the chosen model surface.

## Information for developers

Markups module can be used from other modules as demonstrated in examples in the [Script repository](https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository#Markups).

The Simple Markups Widget can be integrated into slicelets. It is lightweight and can access the Markups features. An example of this use is in [Gel Dosimetry](https://www.slicer.org/wiki/Documentation/Nightly/Modules/GelDosimetry). To use this, access it at [GitHub](https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Markups/Widgets/qSlicerSimpleMarkupsWidget.h).

## Markups json file format

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

### Use markups json files in any Python environment

The examples below show how to use markups json files outside Slicer, in any Python environment.

To access content of a json file it can be either read as a json document or directly into a [pandas](https://pandas.pydata.org/) dataframe using a single command. For example, getting a table of control point labels and positions from the first markups node in the file:
```python
import pandas as pd
controlPointsTable = pd.DataFrame.from_dict(pd.read_json(input_json_filename)['markups'][0]['controlPoints'])
```

Result:
```
>>> controlPointsTable
  label                                        position
0   F-1  [-53.388409961685824, -73.33572796934868, 0.0]
1   F-2     [49.8682950191571, -88.58955938697324, 0.0]
2   F-3   [-25.22749042145594, 59.255268199233726, 0.0]
```

Access position of control points positions in separate x, y, z columns:
```python
controlPointsTable[['x','y','z']] = pd.DataFrame(controlPointsTable['position'].to_list())
del controlPointsTable['position']
```

Write control points to a csv file:
```python
controlPointsTable.to_csv(output_csv_filename)
```

Resulting csv file:
```
,label,x,y,z
0,F-1,-53.388409961685824,-73.33572796934868,0.0
1,F-2,49.8682950191571,-88.58955938697324,0.0
2,F-3,-25.22749042145594,59.255268199233726,0.0
```

## Markups fiducial point list file format

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

## Related modules

- This module will replace [Annotations](Annotations) module.
- [Endoscopy](Endoscopy) module uses fiducials

## References

- [History and design considerations](https://www.slicer.org/wiki/Documentation/Labs/Improving_Markups)

## Contributors

Authors:
- Andras Lasso (PerkLab, Queen's University)
- Davide Punzo (Kapteyn Astronomical Institute, University of Groningen)
- Kyle Sunderland (PerkLab, Queen's University)
- Nicole Aucoin (SPL, BWH)
- Csaba Pinter (Pixel Medical / Ebatinca)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on NA-MIC can be obtained from the [NA-MIC website](http://www.na-mic.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
