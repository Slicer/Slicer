XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Python Modules</category>
  <title>Python Extract Baseline DWI Volume</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image type="diffusion-weighted">
      <name>inputVolume</name>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>
  </parameters>
</executable>
"""

def Execute (inputVolume):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene


  inputVolume = scene.GetNodeByID(inputVolume)
  data = inputVolume.GetImageData().ToArray()
  dims = inputVolume.GetImageData().GetDimensions()

  grad = slicer.vtkDoubleArray()
  grad = inputVolume.GetDiffusionGradients()
  G = grad.ToArray()

  shapeD = data.shape

  bId = 0
  for i in range(shapeD[3]):
     if G[i,0] == 0 and  G[i,1] == 0 and  G[i,2] == 0:
        bId = i
        break
  
  dvol1 = data[..., bId]
  r1 = slicer.vtkMRMLScalarVolumeNode()
  r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
  scene.AddNode(r11)

  r1.AddAndObserveDisplayNodeID(r11.GetName())

  imgD = slicer.vtkImageData()
  imgD.SetDimensions(dims[0], dims[1], dims[2])
  imgD.SetScalarTypeToShort()

  org = inputVolume.GetOrigin()
  spa = inputVolume.GetSpacing()

  mat = slicer.vtkMatrix4x4()
  inputVolume.GetIJKToRASMatrix(mat)

  r1.SetAndObserveImageData(imgD)
  r1.SetIJKToRASMatrix(mat)
  r1.SetOrigin(org[0], org[1], org[2])
  r1.SetSpacing(spa[0], spa[1], spa[2])

  scene.AddNode(r1)

  tmp = r1.GetImageData().ToArray()
  tmp[:] = dvol1[:]

  r1.GetDisplayNode().SetDefaultColorMap()
  r1.Modified()

  return

