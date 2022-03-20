# Colors

## Color table file format (.txt, .ctbl)

The color file format can store a [color node](https://apidocs.slicer.org/master/classvtkMRMLColorNode.html) in a plain text file with the `.txt` or `.ctbl` extension. It is a text file with values separated by space, with a custom header to specify lookup table type. Header lines are prefixed with `#`.

### Discrete scale color lookup table

Header starts with `# Color table file`. Each data line contains `color index` (integer), `color name` (string, if the name contains spaces then the spaces must be replaced by underscore), `red` (0-255), `green` (0-255), `blue` (0-255), and `opacity` (0-255).

Example:

```
# Color table file C:/Users/andra/OneDrive/Projects/SlicerTesting2022/20220109-ColorLegend/Segmentation-label_ColorTable.ctbl
# 4 values
0 Background 0 0 0 0
1 artery 216 101 79 255
2 bone 241 214 145 255
3 connective_tissue 111 184 210 255
```

### Continuous scale color lookup table

Header starts with `# Color procedural file`. Each data line contains `position` (mapped value, a floating-point number), `red` (0.0-1.0), `green` (0.0-1.0), `blue` (0.0-1.0).

Example:

```
# Color procedural file /path/to/file.txt
# 5 points
# position R G B
0 0 0 0
63 0 0.501961 0.490196
128 0.501961 0 1
192 1 0.501961 0
255 1 1 1
```

## Debugging

### Access scalar bar actor

Access to the scalar bar VTK actor may can be useful for debugging and for experimenting with new features. This code snippet shows how to access the actor in the Red slice view using Python:

```python
displayableNode = getNode('Model')
colorLegendDisplayNode = slicer.modules.colors.logic().GetColorLegendDisplayNode(displayableNode)
sliceView = slicer.app.layoutManager().sliceWidget('Red').sliceView()
displayableManager = sliceView.displayableManagerByClassName("vtkMRMLColorLegendDisplayableManager")
colorLegendActor = displayableManager.GetColorLegendActor(colorLegendDisplayNode)

# Experimental adjustment of a parameter that is not exposed via the colorLegendDisplayNode
colorLegendActor.SetBarRatio(0.2)
sliceView.forceRender()
```
