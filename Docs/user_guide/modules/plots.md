# Plots

## Overview

This module can display interactive line and bar plots in the view layout. A `plot chart` can display one or more `plot series`. The plot series specifies the plot type (line, bar, scatter, scatter bar), data source (table and column for x and y data), and appearance (lines style, marker style, etc.).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_plots.png)

Features:
- Multiple plots types (Line, Bar, Scatter, Scatter bar), shown in standard view layouts.
- Configurable chart and axis title, legend, grid lines.
- Use indexes (0, 1, 2, ...) or a table column for X axis values.
- Multiple plot series can be added to a single plot chart (even showing data from different tables, with different types - plot, bar), and the same series can be added to multiple charts. The same plot chart can be shown in any plot views (even simultaneously).
- Label can be assigned to each data point (shown in tooltip when hovering over a data point).
- Interactive editing of data points by drag-and-drop (data in referenced table nodes is updated).
- Interactive selection of data points (rectangular or free-hand selection).
- Plot data and display properties are stored in the MRML scene.
- Plot view can be embedded into module user interfaces. Plot views can emit signals back to the application as the user interacts with a plot, which allows modules to perform additional processing on the selected data points.

## How to

### Plot values in a table

- Go to Tables module
- Select or create a new table
- Select at least one column with a numeric `Data type`.
  - Open `Column properties` section below the table to see data type of the selected column.
  - If data type is not numeric (double, float, int, ...) already then choose `double` for floating-point values (or `int` for integer values), and then click `Convert`.
- Click the `Generate an interactive plot...` button above the table to create and display a plot from the selected column(s).

If multiple columns are selected then the first numeric column will be used as x axis and others will be used as y axes (each pair will create a new series).

Multiple columns can be selected by holding down Shift or Ctrl key while clicking in the table. if at least one cell is selected in a column then the column is considered as selected.

## Keyboard shortcuts and mouse gestures

The following keyboard shortcuts are active when a plot view has the focus (after clicked in a plot view).

| Key | Operation                                                    |
| --- | ------------------------------------------------------------ |
| `s` | switch interaction mode (pan/select points/.../move points)  |
| `r` | reset view to show all data series                           |
| `u` | unselect the points selection                                |

Mouse gestures:
- Left mouse button: depends on interaction mode of the view
  - Pan view: pan view
  - Select points: select points using rectangular selection
  - Freehand select points: select points using free-form selection
  - Move points: move data points by drag-and-drop; this changes values in the referenced table node
- Middle mouse button: pan view (except in pan view mode; in that case it zooms to selected rectangular region)
- Right mouse button: zoom in/out along X and Y axes

## Panels and their use

Plots module user interface has two sections, one for editing properties of charts and another for editing properties of plots.

### Chart section

Choose one or more plot series in `Plot data series` and then adjust display settings.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_plots_chart.png)

### Series section

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_plots_series.png)

- Plot type:
  - `line` or `bar`: displays a single numeric table column as Y values.
  - `scatter` and `scatter bar`: displays two numeric table columns, one is used as X values, the other as Y values.
- Labels column: a string column can be chosen to be used as labels on values, which are displayed when the mouse hovers over point in the plot.
- Logarithmic scale: this feature is not tested thoroughly, need to be used with caution.

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/plots.md) and [Script repository](../../developer_guide/script_repository.md#plots).

## Related modules

This module replaced the Charts module, which was a very limited module for plotting `DoubleArray` nodes.

## Contributors

Authors:
- Davide Punzo (Kapteyn Astronomical Institute, University of Groningen)
- Andras Lasso (PerkLab, Queen's University)

## Acknowledgements

This work was supported by the European Research Council under the European Union's Seventh Framework Programme (FP/2007-2013)/ERC Grant Agreement nr. 291-531.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_erc.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kapteyn.png)
