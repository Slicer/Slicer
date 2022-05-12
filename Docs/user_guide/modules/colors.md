# Colors

## Overview

The Colors module manages color look up tables, stored in Color nodes. These tables translate between a numeric value and a color for displaying of various data types, such as volumes (to specify coloring based on voxel values), models (for displaying scalars), and markups (for displaying curve curvatures and other measurements in 3D views). The module also used for assigning colors to various displayable nodes and show color legend in viewers.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/colors_legend.jpg)

Two lookup table types are available:
- Discrete table: List of named colors are specified (example: GenericAnatomyColors). Discrete tables can be used for continuous mapping as well, in this case the colors are used as samples at equal distance within the specified range, and smoothly interpolating between them (example: Grey).
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/colors_discrete.png)
- Continuous scale: Color is specified for arbitrarily chosen numerical values and color value can be computed by smoothly interpolating between these values (example: PET-DICOM). No names are specified for colors.
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/colors_continuous.png)

## How to

### Edit a color lookup table

All built-in lookup tables are read-only to ensure consistency when using these colors. To edit a lookup table:
- Go to Colors module
- Choose a built-in lookup table that is similar to the desired color mapping. For example, `Labels` can be used as a starting points for a discrete table, `PET-DICOM` can be used for creating an editable continuous scale.
- Click the yellow "copy" folder + icon next to the colors selector to create a copy.
- Specify a name for the new color lookup table and click OK.

The loaded color table will appear in the "User generated" category at the top. Click on the arrow on the category name (or click to select and hit the right-arrow key) to open it and see the color tables in that category.

### Save a color lookup table

To save a modified color table to file, use the application menu: File / Save.

File format is described in the [Developer guide](../../developer_guide/modules/colors).

### Load a color lookup table

To save a modified color table to file, drag-and-drop the file to the application window (or use the application menu: File / Add Data) and click OK.

The loaded color table will appear in the "File" category at the top. Click on the arrow on the category name (or click to select and hit the right-arrow key) to open it and see the color tables in that category.

### Show color legend

There are several ways of showing/hiding color legend:
- In Data module: Right-click on an eye icon in the data tree to open the visibility menu. In that menu, "Show color legend" option can be used for showing/hiding the color legend in all views. The option only appears if a color lookup table is already associated with the selected item.
- In Colors module: Color legend can be added/hidden for any displayable node at one place. Open the Color Legend section, select a displayable node (volume, model, markup). Click `Create` to create a color legend, and adjust visualization options.
- In Volumes, Models, Markups modules: Color Legend section can be used to show legend for the selected node.
- In slice views: Right-click on a volume to open the view context menu. In that menu, "Show color legend" option can be used to show/hide color legend for that volume in slice views.

For displaying mapping from numeric values to colors (for example, for displaying a parametric image), choose `Label text` -> `Value`. For displaying color names (for example, names of anatomical structures for a labelmap volume), choose `Label text` -> `Color name`.

:::{admonition} Tip

For displaying color legend for segment names: [Export the segments to a labelmap volume](segmentations.md#export-segmentation-to-labelmap-volume). This creates a color table, which can be displayed as a color legend.

:::

## Panels and their use

- **Colors:** a drop down menu from which to select from the list of loaded color nodes
  - Copy Color Node button: Duplicate the current color node to edit the color entries. Necessary, because built-in color nodes are note editable.
- **Discrete table**
  - **Number of Colors:** the number of colors in the currently selected table.
  - **Hide empty Colors:** When checked, hide the unnamed color entries in the list below.
  - **Scalar Range:** The range of scalar values that are mapped to to the full range of colors. This only changes the display range, not the values in the table. It is used if the scalar range in a model or markups node is chosen as "Color table (LUT)"
  - **Table of currently selected colors**
    - **Index:** the integer value giving the index of this color in the look up table, used to match it up with a scalar value in a volume voxel.
    - **Color:** a box showing the current color. When viewing an editable table, double click on it to bring up a color picker.
    - **Label:** the text description of the color, often an anatomical label. For some tables, the name is automatically generated from the RGBA values.
    - **Opacity:** a value between 0 and 1 describing how opaque this color is. The background color at index 0 is usually set to 0 and other colors to 1.
- **Continuous Display Panel**: Inspect the color values by clicking on the circles that determine the points of the continuous function. Click elsewhere in the color display to add new points.
  - **Point:** the index of this point.
  - **Color box:** click on this to edit the color for this point.
  - **X:** the numeric value that maps to this color.
- **Color legend**
  - **Displayable node:** node that the color legened will ve displayed for.
  - **Color legend**
    - **Create**: create color legend for the selected Displayable node, using the Colors node chosen at the top.
    - **Use current colors**: set the Colors node chosen at the top for showing the Displayable node.
    - **Delete**: remove the Displayable node's color legend.
  - **Visible:** show/hide the color legend.
  - **View:** Choose which views the color legend is allowed to be appear in. Color legend for models and markups is only displayable in the views where these nodes appear.
  - **Title:** Title displayed at the top of the color legend. Set to the Displayable node's name by default.
  - **Label text**
    - **Value:** Numeric value is displayed in labels next to the color bar. Intended for displaying continuous numeric scale.
    - **Color name:** The color name is displayed in labels next to the color bar. Intended for displaying names of labels (segments).
  - **Number of labels:** Number of labels to display. Only applicable if `Value` is used as label text.
  - **Number of colors:** Maximum number of colors displayed. Reduce the number to see discrete colors instead of a continuous color gradient. Only applicable if `Value` is used as label text.
  - **Orientation**: choose between horizontal or vertical color bar orientation.
  - **Position**: horizontal and vertical position in the view
  - **Size**: length of the long and short side of the legend box.
  - **Title properties** and **Label properties**: controls the format of the title and label text.
  - **Format:** label format (only for label properties). Defined using [printf specifiers](https://www.cplusplus.com/reference/cstdio/printf/). Examples:
    - display with 1 fractional digit: `%.1f`
    - display integer values: `%.0f`
    - display with 4 significant digits: `%.4g`
    - display string label annotation: `%s`
  - **Color:** text color.
  - **Opacity:** allows displaying the text semi-transparently.
  - **Font:** the font used to display the title text. Arial: sans-serif font. Courier: fixed-width font. Times: serif font.
  - **Style:** check these boxes if you wish to adjust the style of the title text font. **Shadow** makes the text visible over both dark and bright background.
  - **Size:** font size of the title. Label size is auto-scaled to fit the specified size of the color legend.

## Built-in color lookup tables

- Discrete
  - Labels: A legacy color table that contains some anatomical mapping
  - DiscreteFullRainbow.png ]] FullRainbow: A full rainbow of 256 colors, goes from red to red with all rainbow colors in between. Useful for colorful display of a label map.
  - Grey: A grey scale ranging from black at 0 to white at 255. Useful for displaying MRI volumes.
  - Iron: A scale from red to  yellow, 157 colors.
  - Rainbow: Goes from red to purple, passing through the colors of the rainbow in between. Useful for a colorful display of a label map.
  - Ocean: A lighter blue scale of 256 values, useful for showing registration results.
  - Desert: Red to yellow/orange scale, 256 colous.
  - InvertedGrey: A white to black scale, 256 colors, useful to highlight negative versions, or to flip intensities of signal values.
  - ReverseRainbow: A colorful display option, 256 colors going from purple to red.
  - fMRI: A combination of Ocean (0-22) and Desert (23-42), useful for displaying functional MRI volumes (highlights activation).
  - fMRIPA: A small fMRI positive activation scale going from red to yellow from 0-19, useful for displaying functional MRI volumes when don't need the blue of the fMRI scale.
  - Random: A random selection of 256 rgb colors, useful to distinguish between a small number of labeled regions (especially outside of the brain).
  - Red: A red scale of 256 values. Useful for layering with Cyan.
  - Green: A green scale of 256 values, useful for layering with Magenta.
  - Blue: A blue scale of 256 values from black to pure blue, useful for layering with Yellow.
  - Yellow: A yellow scale of 256 values, from black to pure yellow, useful for layering with blue (it's complementary, layering yields gray).
  - Cyan: A cyan ramp of 256 values, from black to cyan, complementary ramp to red, layering yields gray.
  - Magenta: A magenta scale of 256 colors from black to magenta, complementary ramp to green, layering yields gray.
  - Warm1: A scale from yellow to red, of 256 colors, ramp of warm colors that's complementary to Cool1.
  - Warm2: A scale from green to yellow, 256 colors, ramp of warm colors that's complementary to Cool2.
  - Warm3: A scale from cyan to green, 256 colors, ramp of warm colors that's complementary to Cool3.
  - Cool1: A scale from blue to cyan, 256 colors, ramp of cool colors that's complementary to Warm1.
  - Cool2: A scale from magenta to blue, 256 colous, ramp of cool colors that's complementary to Warm2.
  - Cool3: A scale from red to magenta, ramp of cool colors that's complementary to Warm3.
  - RandomIntegers: A random scale with 1000 entries.
  - GenericAnatomyColors: a list of whole body anatomy labels and useful colors for them, the default for the Editor module creating new label map volumes
  - Generic Colors: a list of colors with the names being the same as the integer value for each entry
- Shade:
  - WarmShade1: A scale from black to red, of 256 colors, ramp of warm colors with variation in value that's complementary to CoolShade1.
  - WarmShade2: A scale from black to yellow, through green, of 256 colors, ramp of warm colors with variation in value that's complementary to CoolShade2.
  - WarmShade3: A scale from black to green, of 256 colors, ramp of warm colors with variation in value that's complementary to CoolShade3.
  - CoolShade1: A scale from black to cyan, 256 colors, ramp of cool colors with variation in value that is complementary to WarmShade1.
  - CoolShade2: A scale from black to blue through purple, 256 colors, ramp of cool colors with variation in value that is complementary to WarmShade2.
  - CoolShade3: A scale from black to magenta, 256 colors, ramp of cool colors with variation in value that is complementary to WarmShade3.
- Tint:
  - WarmTint1: A scale from white to red, 256 colors, ramp of warm colors with variation in saturation that's complementary to CoolTint1.
  - WarmTint2: A scale from white to yellow, 256 colors, ramp of warm colors with variation in saturation that's complementary to CoolTint2.
  - WarmTint3: A scale from white to green, 256 colors, ramp of warm colors with variation in saturation that's complementary to CoolTint3.
  - CoolTint1: A scale from white to cyan, 256 colors, ramp of cool colors with variations in saturation that's complementary to WarmTint1.
  - CoolTint2: A scale from white to blue, 256 colors, ramp of cool colors with variations in saturation that's complementary to WarmTint2.
  - CoolTint3: A scale from white to magenta, 256 colors, ramp of cool colors with variations in saturation that's complementary to WarmTint3.
- Continuous
  - RedGreenBlue: A scale defined from -6.0 to 6.0 that maps to a rainbow from red to blue through green.
- FreeSurfer
  - Heat: The Heat FreeSurfer color table, shows hot spots with high activation
  - BlueRed: A FreeSurfer color scale, 256 colors, from blue to red
  - RedBlue: A FreeSurfer color scale, 256 colors, from red to blue
  - RedGreen: A FreeSurfer color scale, 256 colors, from red to green, used to highlight sulcal curvature
  - GreenRed: A FreeSurfer color scale, 256 colors, from green to red, used to highlight sulcal curvature
  - FreeSurferLabels: A color table read in from a text file, each line of the format: IntegerLabel  Name  R  G  B  Alpha
- PET
  - PET-Heat: Useful for displaying colorized PET data.
  - PET-Rainbow: Useful for displaying colorized PET data.
  - PET-MaximumIntensityProjection: Useful for displaying inverse grey PET data.
  - PET-Rainbow2: `rainbow` color table from the FIJI PET-CT plugin.
  - PET-DICOM: [DICOM standard color lookup table](https://dicom.nema.org/medical/Dicom/2017d/output/chtml/part06/chapter_B.html) `PET`.
  - PET-HotMetalBlue:  [DICOM standard color lookup table](https://dicom.nema.org/medical/Dicom/2017d/output/chtml/part06/chapter_B.html) `Hot Metal Blue`.
- Cartilage MRI
  - dGEMRIC-1.5T: Useful for displaying 1.5 tesla delayed gadolinium-enhanced MRI of cartilage
  - dGEMRIC-3T: Useful for displaying 3 Tesla delayed gadolinium-enhanced MRI of cartilage
- Default Labels from File: a list of color nodes loaded from files, from the default Slicer color directory.
  - ColdToHotRainbow: a shifted rainbow that runs from blue to red, useful when needing to display a volume for which larger values are hotter.
  - HotToColdRainbow: a shifted rainbow that runs from red to blue, useful when needing to display a volume for which larger values are colder.
  - PelvisColor: useful for displaying segmented pelvic MRI volumes.
  - Slicer3_2010_Brain_Labels: a brain segmentation table with 16 labels defined.
  - 64Color-Nonsemantic: A color table with no semantic labels, pure color information.
  - MediumChartColors: A Stephen Few palette. Similar to the Dark Bright color but in the medium range of intensity. May be a better choice for bar charts.
  - DarkBrightChartColors: Palette designed by Stephen Few in "Practical Rules for Using Colors in Charts". Similar to the Maureen Stone palette. Stephen Few recommends this palette for highlighting data. This palette is useful when display small points or thin lines. Again, this palette is good for categorical data.
  - Slicer3_2010_LabelColors: a table with 16 labels defined.
  - AbdomenColors: useful for displaying segmented abdominal MRI volumes
  - SPL-BrainAtlas-ColorFile: useful for displaying segmented brain MRI volumes
  - SPL-BrainAtlas-2012-ColorFile: an updated brain segmentation color node
  - LightPaleChartColors: A light pale palette from Stephen Few, useful for charts.
  - SPL-BrainAtlas-2009-ColorFile: an updated brain segmentation color node
- File: If you load a color file from File -> Add Data, it will appear here.
- User Generated: A user defined color table, use the editor to specify it. Copies of other color tables get displayed in this category. If you create a new color node, it will appear here.

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/colors).

## Contributors

- Nicole Aucoin (SPL, BWH)
- Jim Miller (GE)
- Julien Finet (Kitware Inc.)
- Andras Lasso (PerkLab, Queen's)
- Mikhail Polkovnikov (IHEP)

## Acknowledgements

This work is part of the [National Alliance for Medical Image Computing](https://www.na-mic.org/) (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ge.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
