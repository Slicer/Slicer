#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Developer Tools</category>
  <title>Python Numpy Script</title>
  <description>
Run a Python Numpy script on an image.
</description>
  <version>1.0</version>
  <documentation-url>http://www.slicer.org/slicerWiki/index.php/Modules:PythonNumpyScript-Documentation-3.4</documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>
<acknowledgements>
This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
</acknowledgements>

  <parameters>
    <label>Python Numpy Script Parameters</label>
    <description>Parameters for Python script</description>
    <file>
      <name>scriptFileName</name>
      <longflag>scriptFileName</longflag>
      <description>File containing Numpy code to run</description>
      <label>Script file</label>
    </file>
  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <image>
      <name>inputVolume</name>
      <longflag>inputVolume</longflag>
      <label>Input Volume</label>
      <channel>input</channel>
      <description>Input image passed to Numpy</description>
    </image>

    <image>
      <name>inputVolume2</name>
      <longflag>inputVolume2</longflag>
      <label>Input Volume 2</label>
      <channel>input</channel>
      <description>Second input image passed to Numpy</description>
    </image>

    <image>
      <name>outputVolume</name>
      <longflag>outputVolume</longflag>
      <label>Output Volume</label>
      <channel>output</channel>
      <description>Output volume from Numpy</description>
    </image>

  </parameters>

</executable>
"""

def Execute (scriptFileName="", inputVolume="", inputVolume2="", outputVolume=""):

    Slicer = __import__("Slicer")
    slicer = Slicer.slicer
    scene = slicer.MRMLScene

    if inputVolume:
        inputVolume = scene.GetNodeByID(inputVolume)

    if inputVolume2:
        inputVolume2 = scene.GetNodeByID(inputVolume2)

    if outputVolume:
        outputVolume = scene.GetNodeByID(outputVolume)

    inputImageCopy = slicer.vtkImageData()
    inputImage2Copy = slicer.vtkImageData()

    import numpy

    iarray = None
    if inputVolume:
        inputImageCopy.DeepCopy(inputVolume.GetImageData())
        iarray = inputImageCopy.ToArray()

    iarray2 = None
    if inputVolume2:
        inputImage2Copy.DeepCopy(inputVolume2.GetImageData())
        iarray2 = inputImage2Copy.ToArray()

    oarray = None 
    oarray2D = None 

    d = {}
    d['slicer'] = slicer
    d['iarray'] = iarray
    d['iarray2'] = iarray2
    d['oarray'] = oarray
    d['oarray2D'] = oarray2D

    try:
        execfile(scriptFileName,d,d)
    except Exception, error:
        slicer.Application.ErrorMessage("Python Numpy script error: %s\n" % error)
        raise

    try:
        oarray = d['oarray']
        oarray2D = d['oarray2D']
    except KeyError:
        slicer.Application.ErrorMessage("Python Numpy script error: oarray or oarray2D have been deleted within the user script. Please avoid deleting these variables.\n")
        raise

    if outputVolume:
        outputImage = slicer.vtkImageData()
        try:
            if oarray != None:
                outputImage.FromArray(oarray)
            elif oarray2D != None:
                outputImage.FromArray2D(oarray2D)
        except Exception, error:
            slicer.Application.ErrorMessage("Python Numpy script error: %s\n" % error)
            raise
        outputVolume.SetAndObserveImageData(outputImage)
        if inputVolume:
            matrix = slicer.vtkMatrix4x4()
            inputVolume.GetIJKToRASMatrix(matrix)
            outputVolume.SetIJKToRASMatrix(matrix)

    return

