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

### Display section
- Visibility: Toggle the markup visibility, which will override the individual control point visibility settings. If the eye icon is open, the list is visible, and pressing it will make it invisible. If the eye icon is closed, the list is invisible and pressing the button will make it visible.
- Opacity: Overall opacity of the selected markup.
- Glyph Size: Set control point glyph size relative to the screen size (if `absolute` button is not pressed) or as an absolute size (if `absolute` button is depressed).
- Text Scale: Label size relative to screen size.
- Interaction handles:
  - Visibility: Check `Visible` to enable translation/rotation/scaling of the entire markups in slice and 3D views with an interactive widget.
  - Translate, Rotate, Scale: enable/disable adjustment types.
  - Size: size of the handles (relative to the application window size).
  - More options: Clicking more options will show/hide check boxes for controlling the visibility of each interaction handle axis separately.
- Advanced:
  - View: Select which views the markup is displayed in
  - Selected Color: Select the color that will be used to display the glyph and text when the markup is marked as selected.
  - Unselected Color: Select the color that will be used to display the glyph and text when the markup is not marked as selected.
  - Active Color: Select the color that will be used to display the glyph and text when the mouse hovers over the markup.
  - Glyph Type: Select the symbol that will be used to mark each location. Default is the Sphere3D.
  - Line thickness: The thickness of lines in markups. Defined as either an absolute thickness, or as a percentage of the glyph size.
  - Outline: Visibility and opacity of the markups outline.
  - Fill: Visibility and opacity of the markups fill.
  - Properties Label: Check to display node name and measurements.
  - Control Point Labels: Check to display a label for each control point.
  - Text display:
    - Font: Change the properties of the font used to display the labels.
    - Background: Change the label background color and opacity.
  - 3D Display:
    - Placement mode: Defines how points are placed and moved in views
      - Unconstrained: Point is moved independently from displayed objects in 3D views (e.g., in parallel with camera plane).
      - Snap to visible surface: Point is snapped to any visible surface in 3D views.
    - Occluded visibility: Controls the visibility and opacity of markups that are occluded. If enabled, the markup will remain visible even when it is blocked from view by other nodes (eg. volume rendering, segmentations, models, etc.).
  - 2D Display:
    - Projection visibility: Check to enable or disable visualization of projected control points on 2D viewers. The projection of the control points in the 2D viewers will be displayed onto slices around the one on which the control points have been placed.
    - Use Markup Color: If checked, color the projections the same color as the markup.
    - Projection Color: If not using markup color for the projection, use this color.
    - Outlined Behind Slice Plane: Control point projection is displayed filled (opacity = Projection Opacity) when on top of slice plane, outlined when behind, and with full opacity when in the plane. Outline isn't used for some glyphs (Dash2D, Cross2D, Starburst).
- Scalars: Color markup according to a scalar, e.g. a per-control-point measurement (see Measurements section below)
  - Visibility: Controls the visibility of the scalars on the markups node.
  - Active Scalar: Select the scalar value that should be displayed.
  - Color Table: Select the color table that should be used to display the scalars.
  - Scalar Range Mode: Select the mode that should be used to control the mapping the scalar range onto the color node.
    - Manual: range is set manually
    - Data scalar range: range is set to the value range of the active scalar
    - Color table: use range specified in the color table. Useful for showing several nodes using the same color mapping.
    - Data type: range is set to the possible range of the active scalar's data type. This is only useful mostly for 8-bit scalars.
    - Direct color mapping: if active scalar has 3 or 4 components then those are interpreted as RGB or RGBA values.
  - Displayed Range: The currently used scalar range.
- Color Legend: Controls the color legend for the currently active scalars.
  - Visibility: Controls the visibility of the color legend in the views.
  - Views: Select which views the color legend should be displayed in.
  - Title: Set the title of the color legend.
  - Label text: Display either the scalar values or the name of the color.
  - Number of labels: Change the number of value labels that should be displayed on the legend.
  - Number of colors: Change the maximum number of colors to display.
  - Orientation: Change the display of the color legend between vertical and horizontal.
  - Position: Adjust the position of the legend in the views.
  - Size: Adjust the size of the legend in the views.
  - Title/Label properties: Controls the display of the title.
    - Format: Change the format used to display the scalar values in the legend (number of decimals, etc.) using a printf style string.
    - Color: Change the color of the title/label.
    - Opacity: Change the opacity of the title/label.
    - Font: Change the font used to render the title/label.
    - Style: Change the display properties of the title/label.
    - Size: Change the size of the title.
- Save to Defaults: Save the display properties of this markup to be the new system defaults. The control point label visibility and properties label visibility are settings that are not included when saving defaults, as typically it is better to initialize these based on the markup type (control point labels are more useful for markups point lists, while the properties label is more useful for other markup types).
- Reset to Defaults: Reset the display properties of this markup to the system defaults.
Measurements section below)
  - Visible: Whether scalar coloring should be shown or the original color of the markup
  - Active Scalar: Which scalar array to use for coloring
  - Color Table: Palette used for coloring
  - Scalar Range Mode: Method for determining the range of the scalars (automatic range calculation based on the data is the default)

### Control points section
- Interaction:
  - Locked: Toggle the markups lock state (if it can be moved by mouse interactions in the viewers), which will override the individual control points lock settings.
  - Fixed list of points: Toggle whether control points can be added or removed from the markups. Control point position can still be undefined.
- Click to Jump Slices: If checked, click in name column to jump slices to that point. The radio buttons control if the slice is centered on the control point or not after the jump. Right click in the table allows jumping 2D slices to a highlighted control point (uses the center/offset radio button settings). Once checked, arrow keys moving the highlighted row will also jump slice viewers to that control point position.
- Show slice intersections: Toggle visibility of the slice intersection visibility in the 2D views.
- Buttons: These buttons apply changes to control points in the selected list.
  - Toggle visibility flag: Toggle visibility flag on all control points in the list. Use the drop down menu to set all to visible or invisible.
  - Toggle selected flag: Toggle selected flag on all control points in the list. Use the drop down menu to set all to selected or deselected. Only selected markups will be passed to command line modules.
  - Toggle lock flag: Toggle lock flag on all control points in the list. Use the drop down menu to set all to locked or unlocked.
  - Skip highlighted control points: Clear the current the position and sets the control point state to skip. When placing multiple control points, control points with this state will be skipped over for placement, moving on to the next unplaced control point.
  - Clear highlighted control points: Clear the current the position and sets the control point state to clear. The control point position can be redefined using place mode.
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
  - State: The current state of the control point. Clicking on the current state will cycle through the possible states.
    - Edit: The control point is currently being placed. Only one control point can be in the edit state at a time. If the state of another control point is set to edit, then the current control point state will be set to clear.
    - Skip: The control point is not currently defined, and cannot be selected for placement.
    - Restore: The control point has a defined position. Entering this state will restore the last known position of the control point.
    - Clear: The control point has not yet been placed, and can be selected for placement.
- Advanced section:
  - Move Up: Move a highlighted control point up one spot in the list.
  - Move Down: Move a highlighted control point down one spot in the list.
  - Add Control Point: Add a new unplaced control point to the selected list, creating it with an undefined position.
  - Naming:
    - Name Format: Format for creating names of new control points, using sprintf format style. %N is replaced by the list name, %d by an integer.
    - Apply: Rename all control points in this list according to the current name format, trying to preserve numbers. A quick way to re-number all the control points according to their index is to use a name format with no number in it, rename, then add the number format specifier %d to the format and rename one more time.
    - Reset: Reset the name format field to the default value, %N-%d.
  - Convert annotation fiducials: Uses annotation fiducial hierarchies to convert them to markups. Removes the annotation nodes once completed.

### Measurements section
- This section lists the available measurements of the selected markup
  - `length` for line and curve
  - `angle` for angle markups
  - `curvature mean` and `curvature max` for curve markups
  - `area` for plane markups
  - `volume` for ROI markups
- In the table below the measurement descriptions, the measurements can be enabled/disabled
  - Basic measurements (e.g. length, angle) are enabled by default
  - Curve markups support curvature calculation, which is off by default
    - When turned on, the curvature data can be displayed as scalar coloring (see Display/Scalars above)

### Export/Import Table section
- This section controls the import and export of markups to `vtkMRMLTableNode`.
- Operation: Select the operation, either Export or Import.
- Output/Input table: Select the input/output node.
- Advanced:
  - Export coordinate system: Choose if the markups are exported in RAS or LPS.
- Import/Export: Execute export/import operation.

### Curve setting section
- Curve type:
  - linear: control points are connected with straight line
  - spline, Kochanek spline: smooth interpolating curve
  - polynomial: smooth approximating curve
  - shortest distance on surface: curve points are forced to be on the selected model's surface points, connected with a minimal-cost path across the model mesh's edges
- Constrain to Model: Model to constrain the curve to. For curve types linear, spline, Kochanek spline, and polynomial the curves will be generated from the control points and then projected onto the surface. For `shortest distance on surface` curve type the curve is generated directly on this model.
- Surface: surface used for `shortest distance on surface` curve type and cost function that is minimized to find path connecting two control points
- Advanced:
  - Maximum projection distance: The maximum search radius tolerance defining the allowable projection distance for projecting curve points. It is specified as a percentage of the model's bounding box diagonal in world coordinate system.

### Resample section
- Output node: Replace control points by curve points sampled at equal distances.
- Constrain points to surface: If a model is selected, the resampled points will be projected to the chosen model surface.
- Advanced:
  - Maximum projection distance: The maximum search radius tolerance defining the allowable projection distance for projecting resampled control points. It is specified as a percentage of the model's bounding box diagonal in world coordinate system.

### Plane settings section
- Plane type: The method used to define the plane using control points.
  - Three points: Plane can be defined by placing 3 control points. The origin of the plane will be at the first control point, the x-axis will be defined by the second, and plane normal will be defined by the cross product of the vectors from the second and third points to the first point.
  - Point normal: Plane is defined using a single point and a normal vector. The normal vector will be parallel with the view direction. If placing in 3D, can be placed on a surface or volume rendering, which will align the plane normal with the surface normal.
  - Plane fit: The plane is defined by fitting a plane to any number of control points (minimum 3).
- Size mode: Method used define the size of the plane.
  - Auto: Plane size will be automatically defined based on the plane type.
  - Absolute: Plane size will be fixed.
- Size: The width and length of the plane. Can only be modified in absolute plane size mode.
- Bounds: The minimum and maximum bounds of the plane along the plane XY axes. Can only be modified in absolute plane size mode.
- Normal: Controls the plane normal direction arrow visibility and opacity.

### ROI settings section
- ROI type:
  - Box: ROI is specified by a single control point in the center, axis directions, and size along each axis direction.
  - BoundingBox: ROI is specified as the bounding box of all the control points.
- Inside out: If enabled then the selected region is outside the ROI boundary. It is up to each module to decide if this information is used. For example, if ROI is used for cropping a volume then this flag is ignored, as an image cannot have a hole in it; but for example inside out can make sense for blanking out region of a volume.
- L-R, P-A, I-S Range: Extents of the ROI box along the ROI axes.
- Display ROI: show/hide the ROI in all views.
- Interactive mode: allow changing the ROI position, orientation, and size in views using interaction handles. If interaction handles are disabled, the ROI may still be changed by moving control points (unless control points are locked, too).

## Creating Template Landmarks

In order to define a workflow in which a known list of markups must be placed, it is possible to save/load predefined markups lists to use as templates.

Defining a template:
  1. Create a new point list using the toolbar, or the create button in the Markups module. You may want to exit place mode by clicking on the toolbar, or by right-clicking in a view.
  2. In the Markups module, select the newly created points list
  3. In the Control Points - Advanced section, create the required number of control points for the template by clicking on the Add Control Point button.
  4. Set the name/description of the control points in the control points table.
  5. To prevent points from being added or removed, click on the "Fixed list of points" button at the top of the control points section.
  6. Export the template to file by right-clicking on the points list in the markups module, or in the subject hierarchy, and select "Export to file...".

Loading a template:
  1. Drag-and-drop the template file into 3D Slicer.
  2. Select the loaded template in the markups toolbar.
  3. Enter place mode with "Place multiple control points" enabled.
  4. Place points until all of the points in the template have been placed. Each point in the template will be selected for placement sequentially. Once all points have been placed, place mode will be automatically disabled.

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/markups.md) and [Script repository](../../developer_guide/script_repository.md#markups).

## Related modules

- [Endoscopy](endoscopy.md) module uses control points
- This module replaced the legacy Annotations module.

## Contributors

Authors:
- Andras Lasso (PerkLab, Queen's University)
- Davide Punzo (Kapteyn Astronomical Institute, University of Groningen)
- Kyle Sunderland (PerkLab, Queen's University)
- Nicole Aucoin (SPL, BWH)
- Csaba Pinter (Pixel Medical / Ebatinca)
- Sara Rolfe (University of Washington, Seattle Children's Research Institute)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on NA-MIC can be obtained from the [NA-MIC website](https://www.na-mic.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
