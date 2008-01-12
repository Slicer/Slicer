
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Surface ICP Registration</title>
  <description>
Performes registration of the input surface onto a target surface using on the Iterative Closest Point algorithm.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Surface ICP Registration Parameters</label>
    <description>Parameters for surface registration</description>

    <string-enumeration>
      <name>landmarkTransformMode</name>
      <longflag>landmarkTransformMode</longflag>
      <description>Mode of operation of internal landmark transform</description>
      <label>Landmark transform mode</label>
      <default>RigidBody</default>
      <element>RigidBody</element>
      <element>Similarity</element>
      <element>Affine</element>
    </string-enumeration>

    <string-enumeration>
      <name>meanDistanceMode</name>
      <longflag>meanDistanceMode</longflag>
      <label>Mean distance mode</label>
      <description>Set how the mean distance should be computed, RMS (square root of the average of the sum of squares of the closest point distances) or AbsoluteValue (mean of the sum of absolute values of the closest point distances)</description>
      <default>RMS</default>
      <element>RMS</element>
      <element>AbsoluteValue</element>
    </string-enumeration>

    <integer>
      <name>maximumNumberOfIterations</name>
      <longflag>maximumNumberOfIterations</longflag>
      <label>Maximum number of iterations</label>
      <description>Maximum number of Interative Closest Point iterations</description>
      <default>50</default>
      <constraints>
        <minimum>1</minimum>
        <maximum>1000</maximum>
        <step>1</step>
      </constraints>
    </integer>

    <integer>
      <name>maximumNumberOfLandmarks</name>
      <longflag>maximumNumberOfLandmarks</longflag>
      <label>Maximum number of landmarks</label>
      <description>Maximum number of landmarks on source and target surfaces</description>
      <default>200</default>
      <constraints>
        <minimum>1</minimum>
        <maximum>1000</maximum>
        <step>1</step>
      </constraints>
    </integer>

    <boolean>
      <name>startByMatchingCentroids</name>
      <longflag>startByMatchingCentroids</longflag>
      <label>Start by matching centroids</label>
      <description>Toggle starting registration by first translating the source centroid to the target centroid</description>
      <default>false</default>
    </boolean>

    <boolean>
      <name>checkMeanDistance</name>
      <longflag>checkMeanDistance</longflag>
      <label>Check mean distance</label>
      <description>Force the algorithm to check the mean distance between two iterations</description>
      <default>false</default>
    </boolean>

    <double>
      <name>maximumMeanDistance</name>
      <longflag>maximumMeanDistance</longflag>
      <label>Maximum mean distance</label>
      <description>Maximum distance between two iterations used for determining convergence</description>
      <default>0.01</default>
    </double>

  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <geometry>
      <name>inputSurface</name>
      <label>Input Surface</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input surface to be registered</description>
    </geometry>

    <geometry>
      <name>targetSurface</name>
      <label>Target Surface</label>
      <channel>input</channel>
      <index>1</index>
      <description>Target surface for registration</description>
    </geometry>

    <geometry>
      <name>outputSurface</name>
      <label>Output Surface</label>
      <channel>output</channel>
      <index>2</index>
      <description>Output registered surface</description>
    </geometry>
  </parameters>

</executable>
"""


def Execute (inputSurface, targetSurface, outputSurface, \
            landmarkTransformMode="RigidBody", meanDistanceMode="RMS", \
            maximumNumberOfIterations=50, maximumNumberOfLandmarks=200, \
            startByMatchingCentroids=False, checkMeanDistance=False, maximumMeanDistance=0.01):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene
    inputSurface = scene.GetNodeByID(inputSurface)
    targetSurface = scene.GetNodeByID(targetSurface)
    outputSurface = scene.GetNodeByID(outputSurface)

    icpTransform = slicer.vtkIterativeClosestPointTransform.New()
    icpTransform.SetSource(inputSurface.GetPolyData())
    icpTransform.SetTarget(targetSurface.GetPolyData())
    if landmarkTransformMode == "RigidBody":
        icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    elif landmarkTransformMode == "Similarity":
        icpTransform.GetLandmarkTransform().SetModeToSimilarity()
    elif landmarkTransformMode == "Affine":
        icpTransform.GetLandmarkTransform().SetModeToAffine()
    if meanDistanceMode == "RMS":
        icpTransform.SetMeanDistanceModeToRMS()
    elif meanDistanceMode == "AbsoluteValue":
        icpTransform.SetMeanDistanceModeToAbsoluteValue()
    icpTransform.SetMaximumNumberOfIterations(maximumNumberOfIterations)
    icpTransform.SetMaximumNumberOfLandmarks(maximumNumberOfLandmarks)
    icpTransform.SetStartByMatchingCentroids(int(startByMatchingCentroids))
    icpTransform.SetCheckMeanDistance(int(checkMeanDistance))
    icpTransform.SetMaximumMeanDistance(maximumMeanDistance)

    transformFilter = slicer.vtkTransformPolyDataFilter.New()
    transformFilter.SetInput(inputSurface.GetPolyData())
    transformFilter.SetTransform(icpTransform)        
    transformFilter.Update()
    
    outputSurface.SetAndObservePolyData(transformFilter.GetOutput())

    icpTransform.Delete()
    transformFilter.Delete()

    return

