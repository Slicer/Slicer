XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Python Modules</category>
  <title>Python Diffusion Tensor Estimation</title>
  <description>Performs a tensor model estimation from diffusion weighted images.</description> 
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  <acknowledgements>This command module is based on the estimation capabilities of teem</acknowledgements>
  <parameters>
    <label>Estimation Parameters</label>
    <description> Parameters for the estimation algorithm</description>
    <string-enumeration>
      <name>estV</name>
      <longflag>estV</longflag>
      <description>An enumeration of strings</description>
      <label>Estimation Parameters</label>
      <default>Least Squares</default>
      <element>Least Squares</element>
      <element>Weigthed Least Squares</element>
      <element>Non Linear</element>
    </string-enumeration>
    <double>
      <name>otsuV</name>
      <longflag>otsuV</longflag>
      <label>Otsu Omega Threshold Parameter</label>
      <description>Control the sharpness of the threshold in the Otsu computation. 0: lower threshold, 1: higher threhold</description>
      <default>0.5</default>
      <constraints>
        <minimum>0.0</minimum>
        <maximum>1.0</maximum>
        <step>0.1</step>
      </constraints>
    </double>
    <boolean>
     <name>islandsV</name>
     <longflag>islandsV</longflag>
     <label>Remove Islands in Threshold Mask</label>
     <description>Remove Islands in Threshold Mask?</description>
     <default>true</default>
    </boolean>
    <boolean>
     <name>maskV</name>
     <longflag>maskV</longflag>
     <label>Apply Mask to Tensor Image</label>
     <description>Apply Threshold Mask to Tensor?</description>
     <default>true</default>
    </boolean>
  </parameters>
  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image type="diffusion-weighted">
      <name>inputVolumeID</name>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>
    <image type="tensor">
      <name>outputTensorID</name>
      <label>Output DTI Volume</label>
      <channel>output</channel>
      <index>1</index>
      <description>Estimated DTI volume</description>
    </image>
    <image type ="scalar">
      <name>outputBaselineID</name>
      <label>Output Baseline Volume</label>
      <channel>output</channel>
      <index>2</index>
      <description>Estimated baseline volume</description>
    </image>
    <image type ="label">
      <name>thresholdMaskID</name>
      <label>Otsu Threshold Mask</label>
      <channel>output</channel>
      <index>3</index>
      <description>Otsu Threshold Mask</description>
    </image> 
  </parameters>
</executable>
"""

def Execute (inputVolumeID, outputTensorID, outputBaselineID="",\
             thresholdMaskID="", estV="Least Squares",\
             otsuV=0.5, islandsV=True, maskV=True):
  
  Slicer = __import__("Slicer")
  slicer = Slicer.slicer
  scene = slicer.MRMLScene
  
  inputVolume = scene.GetNodeByID(inputVolumeID)
  outputTensor = scene.GetNodeByID(outputTensorID)
  baseline = scene.GetNodeByID(outputBaselineID)
  threshold = scene.GetNodeByID(thresholdMaskID)

  grad = slicer.vtkDoubleArray()
  grad = inputVolume.GetDiffusionGradients()

# access B values  
  bval = slicer.vtkDoubleArray()
  bval = inputVolume.GetBValues()

  inputVolume.Modified()

# make an estimate of the tensor
  estim = slicer.vtkTeemEstimateDiffusionTensor()
  estim.SetInput(inputVolume.GetImageData())
  estim.SetNumberOfGradients(grad.GetNumberOfTuples())
  estim.SetDiffusionGradients(grad)
  estim.SetBValues(bval)

  mf = slicer.vtkMatrix4x4()
  inputVolume.GetMeasurementFrameMatrix(mf)

  i2r = slicer.vtkMatrix4x4()
  inputVolume.GetIJKToRASMatrix(i2r)

  tmp = slicer.vtkMatrix4x4()
# care must be taken with matrix inversion
# deep copy is not supported under Python
  slicer.vtkMatrix4x4().Invert(i2r, tmp)

  for i0 in range(0, 3):
    tmp.SetElement(i0, 3, 0)

  col = [0,0,0]
  for jjj in range(0, 3):
    for iii in range(0 ,3):
      col[iii] = tmp.GetElement(iii, jjj)
    math = slicer.vtkMath()
    math.Normalize(*col)
    for iii in range(0, 3):
      tmp.SetElement(iii, jjj, col[iii])

  trans = slicer.vtkTransform()
  trans.PostMultiply()
  trans.SetMatrix(mf)
  trans.Concatenate(tmp)
  trans.Update()

  estim.SetTransform(trans)
  
  if estV == 'Least Squares':
    estim.SetEstimationMethodToLLS()
  elif estV == 'Weighted Least Squares':
    estim.SetEstimationMethodToWLS()
  elif estV == 'Non Linear':
    estim.SetEstimationMethodToNLS()

  estim.Update()

  #tensorImage = slicer.vtkImageData()
  tensorImage = estim.GetOutput()
  tensorImage.SetScalarTypeToFloat()
  tensorImage.GetPointData().SetScalars(None)

#compute tenor mask
  otsu = slicer.vtkITKNewOtsuThresholdImageFilter()
  otsu.SetInput(estim.GetBaseline())
  otsu.SetOmega (1 + otsuV)
  otsu.SetOutsideValue(1)
  otsu.SetInsideValue(0)
  otsu.Update()

  mask = slicer.vtkImageData()
  mask = otsu.GetOutput()

  dims = [0,0,0]
  dims = mask.GetDimensions()
  px = dims[0]/2
  py = dims[1]/2
  pz = dims[2]/2

  cast = slicer.vtkImageCast()
  cast.SetInput(mask)
  cast.SetOutputScalarTypeToUnsignedChar()
  cast.Update()

  con = slicer.vtkImageSeedConnectivity()
  con.SetInput(cast.GetOutput())
  con.SetInputConnectValue(1)
  con.SetOutputConnectedValue(1)
  con.SetOutputUnconnectedValue(0)
  con.AddSeed(px, py, pz)
  con.Update()

  cast1 = slicer.vtkImageCast()
  cast1.SetInput(con.GetOutput())
  cast1.SetOutputScalarTypeToShort()
  cast1.Update()

  conn = slicer.vtkImageConnectivity()
  if (islandsV):  
    conn.SetBackground(1)
    conn.SetMinForeground( -32768)
    conn.SetMaxForeground( 32767)
    conn.SetFunctionToRemoveIslands()
    conn.SetMinSize(10000)
    conn.SliceBySliceOn()
    conn.SetInput(cast1.GetOutput())
    conn.Update()

# Maks tensor
#TODO: fix tenosr mask
  tensorMask = slicer.vtkTensorMask()
  cast2 = slicer.vtkImageCast()
  cast2.SetOutputScalarTypeToUnsignedChar()
  if maskV == True:
    tensorMask.SetMaskAlpha(0.0)
    tensorMask.SetInput(tensorImage)
      
    if islandsV == True:  
      cast2.SetInput(conn.GetOutput())
    else:
      cast2.SetInput(cast1.GetOutput())
        
    tensorMask.SetMaskInput(cast2.GetOutput())
    tensorMask.Update()
    tensorImage = tensorMask.GetOutput()

  tensorImage.GetPointData().SetScalars(None)
  tensorImage.Update()

  org = inputVolume.GetOrigin()
  spa = inputVolume.GetSpacing()

  tmp2 = slicer.vtkMatrix4x4()
  slicer.vtkMatrix4x4().Invert(tmp, tmp2)

  baseline.SetAndObserveImageData(estim.GetBaseline())
  baseline.SetIJKToRASMatrix( i2r )
  baseline.SetOrigin(org[0],org[1], org[2])
  baseline.SetSpacing(spa[0],spa[1], spa[2])
  baseline.Modified()

  threshold.SetAndObserveImageData(conn.GetOutput())
  threshold.SetIJKToRASMatrix( i2r )
  threshold.SetOrigin(org[0],org[1], org[2])
  threshold.SetSpacing(spa[0],spa[1], spa[2])
  threshold.Modified()

  outputTensor.SetAndObserveImageData(tensorImage)
  outputTensor.SetIJKToRASMatrix(i2r)
  outputTensor.SetMeasurementFrameMatrix( i2r )
  outputTensor.SetOrigin(org[0], org[1], org[2])
  outputTensor.SetSpacing(spa[0], spa[1], spa[2])
  outputTensor.GetImageData().SetScalarTypeToFloat()
  outputTensor.GetImageData().GetPointData().GetTensors().SetName('NRRDImage')

  outputTensor.Modified()

  return

#
#def Cast2Rotation(mat):
#  col = [0,0,0]
#  for jjj in range(0, 3):
#    for iii in range(0 ,3):
#       col[iii] = mat.GetElement(iii, jjj)
#    col = slicer.vtkMath().Normalize(col)
#    for iii in range(0, 3):
#       mat.SetElement(iii, jjj, col[iii])
#
#
#def CopyMatrix(mat):
#  copy = slicer.vtkMatrix4x4() 
#  for jjj in range(0, 3):
#    for iii in range(0, 3):
#      copy.SetElement(iii, jjj, mat.GetElement(iii, jjj))
#  return copy

