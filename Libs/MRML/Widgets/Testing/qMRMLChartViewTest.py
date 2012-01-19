import slicer
import math

lns = slicer.mrmlScene.GetNodesByClass('vtkMRMLLayoutNode')
lns.InitTraversal()
ln = lns.GetNextItemAsObject()
print ln.GetID()
ln.SetViewArrangement(24)

cvns = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
cvns.InitTraversal()
cvn = cvns.GetNextItemAsObject()
print cvn.GetID()

dn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
dn = slicer.mrmlScene.AddNode(dn)
print dn.GetID()
a = dn.GetArray()
a.SetNumberOfTuples(600)
x = range(0, 600)
for i in range(len(x)):
    a.SetComponent(i, 0, x[i]/50.0)
    a.SetComponent(i, 1, math.sin(x[i]/50.0))
    a.SetComponent(i, 2, 0)

dn2 = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
dn2 = slicer.mrmlScene.AddNode(dn2)
print dn2.GetID()
a = dn2.GetArray()
a.SetNumberOfTuples(600)
x = range(0, 600)
for i in range(len(x)):
    a.SetComponent(i, 0, x[i]/50.0)
    a.SetComponent(i, 1, math.cos(x[i]/50.0))
    a.SetComponent(i, 2, 0)


cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
cn = slicer.mrmlScene.AddNode(cn)
print cn.GetID()
cn.AddArray('A double array', dn.GetID())
cn.AddArray('Another double array', dn2.GetID())

cvn.SetChartNodeID(cn.GetID())

cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
cn = slicer.mrmlScene.AddNode(cn)
print cn.GetID()
cn.AddArray('Just one array', dn.GetID())

cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
cn = slicer.mrmlScene.AddNode(cn)
print cn.GetID()
cn.AddArray('The other array', dn2.GetID())
