
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Surface Toolbox</title>
  <description>
Process a surface using several filters.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Surface Decimation</label>
    <description>Parameters for surface decimation</description>
    <boolean>
      <name>decimationEnabled</name>
      <longflag>decimationEnabled</longflag>
      <description>Toggle decimation</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    <double>
      <name>targetReduction</name>
      <longflag>targetReduction</longflag>
      <description>Target percent reduction of the number of polygons</description>
      <label>Target reduction</label>
      <default>0.8</default>
      <constraints>
        <minimum>0</minimum>
        <maximum>1</maximum>
        <step>0</step>
      </constraints>
    </double>
    <boolean>
      <name>boundaryDeletion</name>
      <longflag>boundaryDeletion</longflag>
      <description>Toggle deletion of boundary vertices</description>
      <label>Boundary vertex deletion</label>
      <default>true</default>
    </boolean>
  </parameters>

  <parameters>
    <label>Surface Smoothing</label>
    <description>Parameters for surface smoothing</description>
    <boolean>
      <name>smoothingEnabled</name>
      <longflag>smoothingEnabled</longflag>
      <description>Toggle smoothing</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    <string-enumeration>
      <name>smoothingMethod</name>
      <longflag>smoothingMethod</longflag>
      <description>Selection of smoothing method between Laplace and Taubin</description>
      <label>Smoothing method</label>
      <default>Taubin</default>
      <element>Laplace</element>
      <element>Taubin</element>
    </string-enumeration>
    <boolean>
      <name>boundarySmoothing</name>
      <longflag>boundarySmoothing</longflag>
      <description>Toggle allow smoothing of boundary</description>
      <label>Boundary smoothing</label>
      <default>true</default>
    </boolean>
    <integer>
      <name>laplaceIterations</name>
      <longflag>laplaceIterations</longflag>
      <description>Number of iterations for the Laplace smoothing method</description>
      <label>Laplace number of iterations</label>
      <default>10</default>
    </integer>
    <double>
      <name>laplaceRelaxation</name>
      <longflag>laplaceRelaxation</longflag>
      <description>Relaxation factor value for the Laplace smoothing method</description>
      <label>Laplace relaxation factor</label>
      <default>0.1</default>
    </double>
    <integer>
      <name>taubinIterations</name>
      <longflag>taubinIterations</longflag>
      <description>Number of iterations for the Taubin smoothing method</description>
      <label>Taubin number of iterations</label>
      <default>10</default>
    </integer>
    <double>
      <name>taubinPassband</name>
      <longflag>taubinPassband</longflag>
      <description>Passband value for the Taubin smoothing method</description>
      <label>Taubin passband</label>
      <default>0.1</default>
    </double>
  </parameters>

  <parameters>
    <label>Surface Normals</label>
    <description>Parameters for surface normals</description>
    <boolean>
      <name>normalsEnabled</name>
      <longflag>normalsEnabled</longflag>
      <description>Toggle normals</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    <boolean>
      <name>flipNormals</name>
      <longflag>flipNormals</longflag>
      <description>Toggle flip normals after computing</description>
      <label>Flip normals</label>
      <default>false</default>
    </boolean>
    <boolean>
      <name>splitting</name>
      <longflag>splitting</longflag>
      <description>Toggle splitting surface based on feature angle</description>
      <label>Splitting</label>
      <default>false</default>
    </boolean>
    <double>
      <name>featureAngle</name>
      <longflag>featureAngle</longflag>
      <description>Feature angle value for surface splitting</description>
      <label>Feature angle</label>
      <default>30</default>
      <constraints>
        <minimum>0</minimum>
        <maximum>180</maximum>
        <step>1</step>
      </constraints>
    </double>
  </parameters>

  <parameters>
    <label>Surface Cleaner</label>
    <description>Parameters for surface cleaner</description>
    <boolean>
      <name>cleanerEnabled</name>
      <longflag>cleanerEnabled</longflag>
      <description>Toggle cleaner</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
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


def Execute (inputSurface, outputSurface, \
    decimationEnabled=False, targetReduction=0.8, boundaryDeletion=True, \
    smoothingEnabled=False, smoothingMethod="Taubin", boundarySmoothing=True, laplaceIterations=10, laplaceRelaxation=0.1, taubinIterations=10, taubinPassband=0.1, \
    normalsEnabled=False, flipNormals=False, splitting=False, featureAngle=30.0, \
    cleanerEnabled=False):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene
    inputSurface = scene.GetNodeByID(inputSurface)
    outputSurface = scene.GetNodeByID(outputSurface)

    surface = inputSurface.GetPolyData()

    if decimationEnabled:
        decimation = slicer.vtkDecimatePro.New()
        decimation.SetInput(surface)
        decimation.SetTargetReduction(targetReduction)
        if boundaryDeletion:
            decimation.BoundaryVertexDeletionOn()
        else:
            decimation.BoundaryVertexDeletionOff()
        decimation.PreserveTopologyOn()
        decimation.Update()
        surface = decimation.GetOutput()

    if smoothingEnabled:
        if smoothingMethod == "Laplace":
            smoothing = slicer.vtkSmoothPolyDataFilter.New()
            smoothing.SetInput(surface)
            if boundarySmoothing:
                smoothing.BoundarySmoothingOn()
            else:
                smoothing.BoundarySmoothingOff()
            smoothing.SetNumberOfIterations(laplaceIterations)
            smoothing.SetRelaxationFactor(laplaceRelaxation)
            smoothing.Update()
            surface = smoothing.GetOutput()
        elif smoothingMethod == "Taubin":
            smoothing = slicer.vtkWindowedSincPolyDataFilter.New()
            smoothing.SetInput(surface)
            if boundarySmoothing:
                smoothing.BoundarySmoothingOn()
            else:
                smoothing.BoundarySmoothingOff()
            smoothing.SetNumberOfIterations(taubinIterations)
            smoothing.SetPassBand(taubinPassband)
            smoothing.Update()
            surface = smoothing.GetOutput()

    if normalsEnabled:
        normals = slicer.vtkPolyDataNormals.New()
        normals.SetInput(surface)
        normals.AutoOrientNormalsOn()
        if flipNormals:
            normals.FlipNormalsOn()
        else:
            normals.FlipNormalsOff()
        if splitting:
            normals.SplittingOn()
            normals.SetFeatureAngle(featureAngle)
        else:
            normals.SplittingOff()
        normals.ConsistencyOn()
        normals.Update()
        surface = normals.GetOutput()

    if cleanerEnabled:
        cleaner = slicer.vtkCleanPolyData.New()
        cleaner.SetInput(surface)
        cleaner.Update()
        surface = cleaner.GetOutput()

    outputSurface.SetAndObservePolyData(surface)

    if decimationEnabled:
        decimation.Delete()

    if smoothingEnabled:
        smoothing.Delete()

    if normalsEnabled:
        normals.Delete()

    if cleanerEnabled:
        cleaner.Delete()

    return

