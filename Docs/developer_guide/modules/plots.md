# Plots

## Design

### MRML nodes

- [vtkMRMLTableNode](https://apidocs.slicer.org/master/classvtkMRMLTableNode.html): Table node stores values that specify data point positions or bar heights in the plots.
- [vtkMRMLPlotSeriesNode](https://apidocs.slicer.org/master/classvtkMRMLPlotSeriesNode.html): Defines a data series by referring to a table node and column name(s) for X and Y axes and labels.
  - It also defines display properties, such as plot type, color, line style.
  - Line and bar plots only require Y axis (points along X axis are equally spaced), scatter plots require two input data columns, for X and Y axes.
- [vtkMRMLPlotChartNode](https://apidocs.slicer.org/master/classvtkMRMLPlotChartNode.html): Specifies which data series need to be shown in the chart.
  - Also contains global display properties for the chart, such as titles and font style.
- [vtkMRMLPlotViewNode](https://apidocs.slicer.org/master/classvtkMRMLPlotViewNode.html): Specifies which chart is to be displayed in the plot view and how the user can interact with it.
    - There has to be exactly one plot view node for each plot view widget. This class can not be created or copied unless is connected with a plot view.

### Widgets

- [qMRMLPlotView](https://apidocs.slicer.org/master/classqMRMLPlotView.html): Displays a plot. It can be embedded into a module user interface.
- [qMRMLPlotWidget](https://apidocs.slicer.org/master/classqMRMLPlotWidget.html): Displays a plot and in a popup window a plot view controller widget.
- [qMRMLPlotViewControllerWidget](https://apidocs.slicer.org/master/classqMRMLPlotViewControllerWidget.html): plot view controller widget.
- [qMRMLPlotSeriesPropertiesWidget](https://apidocs.slicer.org/master/classqMRMLPlotSeriesPropertiesWidget.html): Display/edit properties of a plot series node.
- [qMRMLPlotChartPropertiesWidget](https://apidocs.slicer.org/master/classqMRMLPlotChartPropertiesWidget.html): Display/edit properties of a plot series node.
- [qMRMLPlotViewControllerWidget](https://apidocs.slicer.org/master/classqMRMLPlotViewControllerWidget.html): Display/edit properties of a plot view node.

### Signals

qMRMLPlotView objects provide `dataSelected(vtkStringArray* mrmlPlotSeriesIDs, vtkCollection* selectionCol)` signal that allow modules to respond to user interactions with the Plot canvas. The signal is emitted when a data point or more has been selected. Returns the series node IDs and a list of selected point IDs (as a collection of `vtkIdTypeArray` objects).

Python API example:

```python
# Switch to a layout that contains a plot view to create a plot widget
layoutManager = slicer.app.layoutManager()
layoutWithPlot = slicer.modules.plots.logic().GetLayoutWithPlot(layoutManager.layout)
layoutManager.setLayout(layoutWithPlot)

# Select chart in plot view
plotWidget = layoutManager.plotWidget(0)
plotViewNode = plotWidget.mrmlPlotViewNode()

# Add a PlotCharNode 
# plotViewNode.SetPlotChartNodeID(''PlotChartNode''.GetID())

# Print selected point IDs
def onDataSelected(mrmlPlotDataIDs, selectionCol):
    print("Selection changed:")
    for selectionIndex in range(mrmlPlotDataIDs.GetNumberOfValues()):
        pointIdList = []
        pointIds = selectionCol.GetItemAsObject(selectionIndex)
        for pointIndex in range(pointIds.GetNumberOfValues()):
            pointIdList.append(pointIds.GetValue(pointIndex))
        print("  {0}: {1}".format(mrmlPlotDataIDs.GetValue(selectionIndex), pointIdList))


# Connect the signal with a slot ''onDataSelected''
plotView = plotWidget.plotView()
plotView.connect("dataSelected(vtkStringArray*, vtkCollection*)", self.onDataSelected) 
```

## Examples

Examples for common DICOM operations are provided in the [script repository](../script_repository.md#plots).
