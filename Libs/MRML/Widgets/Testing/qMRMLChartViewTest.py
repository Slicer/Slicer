from __future__ import print_function
import slicer
import math

ln = slicer.mrmlScene.GetFirstNodesByClass('vtkMRMLLayoutNode')
print(ln.GetID())
ln.SetViewArrangement(24)

cvn = slicer.mrmlScene.GetFirstNodesByClass('vtkMRMLChartViewNode')
print(cvn.GetID())

dn = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
print(dn.GetID())
a = dn.GetArray()
a.SetNumberOfTuples(600)
x = range(0, 600)
for i in range(len(x)):
    a.SetComponent(i, 0, x[i]/50.0)
    a.SetComponent(i, 1, math.sin(x[i]/50.0))
    a.SetComponent(i, 2, 0)

dn2 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
print(dn2.GetID())
a = dn2.GetArray()
a.SetNumberOfTuples(600)
x = range(0, 600)
for i in range(len(x)):
    a.SetComponent(i, 0, x[i]/50.0)
    a.SetComponent(i, 1, math.cos(x[i]/50.0))
    a.SetComponent(i, 2, 0)


cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())
print(cn.GetID())
cn.AddArray('A double array', dn.GetID())
cn.AddArray('Another double array', dn2.GetID())
cn.SetProperty('default', 'title', 'A simple chart with 2 curves')
cn.SetProperty('default', 'xAxisLabel', 'Something in x')
cn.SetProperty('default', 'yAxisLabel', 'Something in y')

cvn.SetChartNodeID(cn.GetID())

cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())
print(cn.GetID())
cn.AddArray('Just one array', dn.GetID())
cn.SetProperty('default', 'title', 'A simple chart with 1 curve')
cn.SetProperty('default', 'xAxisLabel', 'Just x')
cn.SetProperty('default', 'yAxisLabel', 'Just y')


cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())
print(cn.GetID())
cn.AddArray('The other array', dn2.GetID())
cn.SetProperty('default', 'title', 'A simple chart with another curve')
cn.SetProperty('default', 'xAxisLabel', 'time')
cn.SetProperty('default', 'yAxisLabel', 'velocity')
cn.SetProperty('The other array', 'showLines', 'on')
cn.SetProperty('The other array', 'showMarkers', 'off')
cn.SetProperty('The other array', 'color', '#fe7d20')


dn3 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
print(dn3.GetID())
a = dn3.GetArray()
a.SetNumberOfTuples(12)
x = range(0, 12)
for i in range(len(x)):
    a.SetComponent(i, 0, x[i]/4.0)
    a.SetComponent(i, 1, math.sin(x[i]/4.0))
    a.SetComponent(i, 2, 0)

cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())
print(cn.GetID())
cn.AddArray('Periodic', dn3.GetID())
cn.SetProperty('default', 'title', 'A bar chart')
cn.SetProperty('default', 'xAxisLabel', 'time')
cn.SetProperty('default', 'yAxisLabel', 'velocity')
cn.SetProperty('default', 'type', 'Bar');
