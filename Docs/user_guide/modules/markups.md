# Markups

## Overview

This module is used to create and edit markups (point list, line, angle, curve, closed curve, plane, ROI etc.) and adjust their display properties.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_types.png)

## How to

### Place new markups

1. Click the "Toggle Markups Toolbar" button in the Mouse Interaction toolbar to show/hide the Markups toolbar.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_toggle_toolbar_2022_04_24.png)

Using the Markups toolbar, click a markups type button to create a new object. The mouse interaction mode will automatically switch into control point placement mode.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_toolbar_2022_04_24.png)

Click the down arrow of the control point place button in the Markups toolbar to select the "Place multiple control points" checkbox to keep placing control points continuously, without the need to click the place button after each point.

2. Left-click in a slice view or 3D view to place points.

3. Double-left-click or right-click to finish point placement.

### Edit control point positions in existing markups

- Make sure that the correct markup is selected in the Markups module or Markups toolbar.
- Left-click-and drag a control point to move it.
- Left-click a control point to jump to it in all slice viewers. This helps in adjusting its position along all axes.
- Right-click to delete or rename a control point or change markup properties.
- Ctrl + Left-click to place a new control point on a markups curve.
- Enable Display / Interaction / Visible to show a widget that allows translation/rotation of the entire widget.

### Edit properties of a markup that is picked in a view

To pick a markup in a viewer so that its properties can be edited in the Markups module, right-click on it in a slice view or 3D view and choose "Edit properties".

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_markups_context_menu_properties_2022_04_24.png)

### Edit Plane markups

- Planes can be defined using 3 "plane types": Point normal (default, place one or two points defining the origin and normal), 3 points (place 3 points to define the origin and plane axes), and plane fit (place any number of points that will be fit to a plane).
- When placing a plane with the "point normal" plane type, Alt + Left-click will allow the placement of 2 points. Placing the first point will define the origin of the plane, while the second point will define the normal vector.
- If the handles are not visible, right-click on the plane outline, or on a control point, and check "Interaction handles visible".
- Plane size can be changed using handles on the corners and edges of the plane.
- Left-click-and-drag on interaction handles to change the plane size.
- Resizing a plane will change the size mode to "absolute", preventing changes in the control points from affecting the plane size.
- Plane type and size mode can be changed from the "Plane settings" section of the Markups module.

### Edit ROI markups

- ROI size can be changed using handles on the corners and faces of the ROI.
- If the handles are not visible, right-click on the ROI outline, or on a control point, and check "Interaction handles visible".
- Left-click-and-drag on interaction handles to change the ROI size.
- Alt + Left-click-and-drag to symmetrically adjust the ROI size without changing the position of the center.

## Keyboard shortcuts

The following keyboard shortcuts are active when the markups toolbar is displayed.

| Key                        | Operation                              |
| -------------------------- | -------------------------------------- |
| `Ctrl` + `Shift` + `A`     | create new markup                      |
| `Ctrl` + `Shift` + `T`     | toggle placing multiple control points |
| `Ctrl` + `Shift` + `space` | place new control point                |

## Panels and their use

- Create: click on any of the buttons to create a new markup. Click in any of the viewers to place control points.
- Markups list: Select a markup to populate the GUI and allow modifications. When control point placement is activated on the toolbar, then points are added to this selected markup.

Display section:
- Visibility: Toggle the markup visibility, which will override the individual control point visibility settings. If the eye icon is open, the list is visible, and pressing it will make it invisible. If the eye icon is closed, the list is invisible and pressing the button will make it visible.
- Opacity: overall opacity of the selected markup.
- Glyph Size: Set control point glyph size relative to the screen size (if `absolute` button is not pressed) or as an absolute size (if `absolute` button is depressed).
- Text Scale: Label size relative to screen size.
- Interaction handles:
  - Visibility: check `Visible` to enable translation/rotation/scaling of the entire markups in slice and 3D views with an interactive widget.
  - Translate, Rotate, Scale: enable/disable adjustment types.
  - Size: size of the handles (relative to the application window size).
- Advanced:
  - View: Select which views the markup is displayed in
  - Selected Color: Select the color that will be used to display the glyph and text when the markup is marked as selected.
  - Unselected Color: Select the color that will be used to display the glyph and text when the markup is not marked as selected.
  - Glyph Type: Select the symbol that will be used to mark each location. Default is the Sphere3D.
  - Control Point Labels: check to display a label for each control point.
  - Control Point Projection: Controls the projection of the control points in the 2D viewers onto slices around the one on which the control points have been placed.
    - 2D Projection: Toggle the eye icon open or closed to enable or disable visualization of the projected control points on 2D viewers.
    - Use Markup Color: If checked, color the projections the same color as the markup.
    - Projection Color: If not using markup color for the projection, use this color.
    - Outlined Behind Slice Plane: Control point projection is displayed filled (opacity = Projection Opacity) when on top of slice plane, outlined when behind, and with full opacity when in the plane. Outline isn't used for some glyphs (Dash2D, Cross2D, Starburst).
    - Projection Opacity: A value between 0 (invisible) and 1 (fully visible) for displaying the control point projection.
    - Reset to Defaults: Reset the display properties of this markup to the system defaults.
    - Save to Defaults: Save the display properties of this markup to be the new system defaults. The control point label visibility and properties label visibility are settings that are not included when saving defaults, as typically it is better to initialize these based on the markup type (control point labels are more useful for markups point lists, while the properties label is more useful for other markup types).
- Scalars: Color markup according to a scalar, e.g. a per-control-point measurement (see Measurements section below)
  - Visible: Whether scalar coloring should be shown or the original color of the markup
  - Active Scalar: Which scalar array to use for coloring
  - Color Table: Palette used for coloring
  - Scalar Range Mode: Method for determining the range of the scalars (automatic range calculation based on the data is the default)

Control points section:
- Interaction in views: toggle the markups lock state (if it can be moved by mouse interactions in the viewers), which will override the individual control points lock settings.
- Click to Jump Slices: If checked, click in name column to jump slices to that point. The radio buttons control if the slice is centered on the control point or not after the jump. Right click in the table allows jumping 2D slices to a highlighted control point (uses the center/offset radio button settings). Once checked, arrow keys moving the highlighted row will also jump slice viewers to that control point position.
- Buttons: These buttons apply changes to control points in the selected list.
  - Toggle visibility flag: Toggle visibility flag on all control points in the list. Use the drop down menu to set all to visible or invisible.
  - Toggle selected flag: Toggle selected flag on all control points in the list. Use the drop down menu to set all to selected or deselected. Only selected markups will be passed to command line modules.
  - Toggle lock flag: Toggle lock flag on all control points in the list. Use the drop down menu to set all to locked or unlocked.
  - Delete the highlighted control points from the active list: After highlighting rows in the table to select control points, press this button to delete them from the list.
  - Remove all control points from the active list: Pressing this button will delete all of the control points in the active list, leaving it with a list length of 0.
  - Transformed: Check to show the transformed coordinates in the table. This will apply any transform to the points in the list and show that result. Keep unchecked to show the raw RAS values that are stored in MRML. If you harden the transform the transformed coordinates will be the same as the non transformed coordinates.
  - Hide RAS: Check to hide the coordinate columns in the table and uncheck to show them. Right click in rows to see coordinates.
- Control points table: Right click on rows in the table to bring up a context menu to show the full precision coordinates, distance between multiple highlighted control points, delete the highlighted control point, jump slice viewers to that location, refocus 3D viewers to that location, or if there are other lists, to copy or move the control point to another list.
  - Selected: A check box is shown in this column, it's check state depends on if the control point is selected or not. Click to toggle the selected state. Only selected control points will be passed to command line modules.
  - Locked: An open or closed padlock is shown in this column, depending on if the control point is unlocked or locked. Click it to toggle the locked state.
  - Visibility: An open or closed eye icon is shown in this column, depending on if the control point is visible or not. Click it to toggle the visibility state.
  - Name: A short name for this control point, displayed in the viewers as text next to the glyphs.
  - Description: A longer description for this control point, not displayed in the viewers.
  - X, Y, Z: The RAS coordinates of this control point, 3 places of precision are shown.
- Advanced section:
  - Move Up: Move a highlighted control point up one spot in the list.
  - Move Down: Move a highlighted control point down one spot in the list.
  - Add Control Point: Add a new control point to the selected list, placing it at the origin
  - Naming:
    - Name Format: Format for creating names of new control points, using sprintf format style. %N is replaced by the list name, %d by an integer.
    - Apply: Rename all control points in this list according to the current name format, trying to preserve numbers. A quick way to re-number all the control points according to their index is to use a name format with no number in it, rename, then add the number format specifier %d to the format and rename one more time.
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
  - shortest distance on surface: curve points are forced to be on the selected model's surface points, connected with a minimal-cost path across the model mesh's edges
- Constrain to Model: Model to constrain the curve to. For curve types linear, spline, Kochanek spline, and polynomial the curves will be generated from the control points and then projected onto the surface. For `shortest distance on surface` curve type the curve is generated directly on this model.
- Surface: surface used for `shortest distance on surface` curve type and cost function that is minimized to find path connecting two control points
- Advanced:
  - Maximum projection distance: The maximum search radius tolerance defining the allowable projection distance for projecting curve points. It is specified as a percentage of the model's bounding box diagonal in world coordinate system.

Resample section:
- Output node: Replace control points by curve points sampled at equal distances.
- Constrain points to surface: If a model is selected, the resampled points will be projected to the chosen model surface.
- Advanced:
  - Maximum projection distance: The maximum search radius tolerance defining the allowable projection distance for projecting resampled control points. It is specified as a percentage of the model's bounding box diagonal in world coordinate system.

ROI settings section:
- ROI type:
  - Box: ROI is specified by a single control point in the center, axis directions, and size along each axis direction.
  - BoundingBox: ROI is specified as the bounding box of all the control points.
- Inside out: If enabled then the selected region is outside the ROI boundary. It is up to each module to decide if this information is used. For example, if ROI is used for cropping a volume then this flag is ignored, as an image cannot have a hole in it; but for example inside out can make sense for blanking out region of a volume.
- L-R, P-A, I-S Range: Extents of the ROI box along the ROI axes.
- Display ROI: show/hide the ROI in all views.
- Interactive mode: allow changing the ROI position, orientation, and size in views using interaction handles. If interaction handles are disabled, the ROI may still be changed by moving control points (unless control points are locked, too).

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/markups.md) and [Script repository](../../developer_guide/script_repository.md#markups).

## Related modules

- This module will replace [Annotations](annotations.md) module.
- [Endoscopy](endoscopy.md) module uses control points

## Contributors

Authors:
- Andras Lasso (PerkLab, Queen's University)
- Davide Punzo (Kapteyn Astronomical Institute, University of Groningen)
- Kyle Sunderland (PerkLab, Queen's University)
- Nicole Aucoin (SPL, BWH)
- Csaba Pinter (Pixel Medical / Ebatinca)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on NA-MIC can be obtained from the [NA-MIC website](https://www.na-mic.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
