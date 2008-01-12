
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Surface Connectivity</title>
  <description>
Identify connected regions of a surface.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Surface Connectivity Parameters</label>
    <description>Parameters for surface connectivity</description>

    <string-enumeration>
      <name>connectivityMode</name>
      <longflag>connectivityMode</longflag>
      <description>Mode of operation of connectivity filter</description>
      <label>Connectivity mode</label>
      <default>AllRegions</default>
      <element>AllRegions</element>
      <element>ClosestToSeed</element>
    </string-enumeration>

    <boolean>
      <name>enableOutputFiducials</name>
      <longflag>enableOutputFiducials</longflag>
      <description>Toggle generation of labeled fiducials corresponding to each extracted region</description>
      <label>Enable output fiducials</label>
      <default>false</default>
    </boolean>

    <point multiple="false">
      <name>seedPoint</name>
      <longflag>seedPoint</longflag>
      <label>Seed Point</label>
      <description>Fiducial to extract closest surface</description>
    </point>
  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <geometry>
      <name>inputSurface</name>
      <label>Input Surface</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input surface to be filtered</description>
    </geometry>

    <geometry>
      <name>outputSurface</name>
      <label>Output Surface</label>
      <channel>output</channel>
      <index>1</index>
      <description>Output filtered surface</description>
    </geometry>
  </parameters>

</executable>
"""


def Execute (inputSurface, outputSurface, connectivityMode="AllRegions", enableOutputFiducials=False, seedPoint=[0.0,0.0,0.0]):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene
    inputSurface = scene.GetNodeByID(inputSurface)
    outputSurface = scene.GetNodeByID(outputSurface)

    connectivityFilter = slicer.vtkPolyDataConnectivityFilter.New()
    connectivityFilter.SetInput(inputSurface.GetPolyData())
    if connectivityMode == "AllRegions":
        connectivityFilter.SetExtractionModeToAllRegions()
        connectivityFilter.ColorRegionsOn()
    elif connectivityMode == "ClosestToSeed":
        connectivityFilter.SetExtractionModeToClosestPointRegion()
        connectivityFilter.SetClosestPoint(*seedPoint)
    connectivityFilter.Update()

    if enableOutputFiducials:
        fiducialList = scene.CreateNodeByClass("vtkMRMLFiducialListNode")
        fiducialList.DisableModifiedEventOn()
        fiducialList.SetScene(scene)
        thresholdPoints = slicer.vtkThresholdPoints.New()
        thresholdPoints.SetInput(connectivityFilter.GetOutput())
        numberOfRegions = connectivityFilter.GetNumberOfExtractedRegions()
        for i in range(numberOfRegions):
            lower = i - 0.5
            upper = i + 0.5
            thresholdPoints.ThresholdBetween(lower,upper)
            thresholdPoints.Update()
            if thresholdPoints.GetOutput().GetNumberOfPoints() > 0:
                point = thresholdPoints.GetOutput().GetPoint(0)
                fid = fiducialList.AddFiducial()
                fiducialList.SetNthFiducialXYZ(fid,*point)
                fiducialList.SetNthFiducialLabelText(fid,"Region %d" % i)
        fiducialList.InvokePendingModifiedEvent()
        fiducialList.DisableModifiedEventOff()
        scene.AddNode(fiducialList)
        thresholdPoints.Delete()

    outputSurface.SetAndObservePolyData(connectivityFilter.GetOutput())

    inputSurface.GetDisplayNode().VisibilityOff()
#FIXME: this is not possible because display node doesn't exist yet; should it exist instead?
#    outputSurface.GetDisplayNode().ScalarVisibilityOn()

    connectivityFilter.Delete()

    return

