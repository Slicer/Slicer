import slicer
import vtk
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

#
# CurvedPlanarReformation
#


class CurvedPlanarReformation(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "CurvedPlanarReformation"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Lee Newberg (Kitware)", "Andras Lasso (PerkLab)"]
        self.parent.helpText = """
This test has been added to check that an example using curved planar reformation code does not return any errors.
      """
        self.parent.acknowledgementText = """
This file was contributed by Lee Newberg, based on testing logic originally
developed by Andras Lasso for the CurvedPlanarReformat module in the
SlicerSandbox extension.
"""


#
# CurvedPlanarReformationWidget
#


class CurvedPlanarReformationWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    def __init__(self, parent=None) -> None:
        """Called when the user opens the module the first time and the widget is initialized."""
        ScriptedLoadableModuleWidget.__init__(self, parent)

    def setup(self) -> None:
        """Called when the user opens the module the first time and the widget is initialized."""
        ScriptedLoadableModuleWidget.setup(self)


#
# CurvedPlanarReformationLogic
#


class CurvedPlanarReformationLogic(ScriptedLoadableModuleLogic):
    pass


#
# CurvedPlanarReformationTest
#


class CurvedPlanarReformationTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear()

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_VolumeResamplerRegistered()
        self.test_CurvedPlanarReformation1()

    def test_VolumeResamplerRegistered(self):
        resamplerName = "ResampleScalarVectorDWIVolume"
        self.assertIsNotNone(slicer.modules.generalizedreformat.logic())
        self.assertTrue(slicer.modules.generalizedreformat.logic().IsVolumeResamplerRegistered(resamplerName))

    def test_CurvedPlanarReformation1(self):
        """Ideally you should have several levels of tests.  At the lowest level
        tests should exercise the functionality of the logic with different inputs
        (both valid and invalid).  At higher levels your tests should emulate the
        way the user would interact with your code and confirm that it still works
        the way you intended.
        One of the most important features of the tests is that it should alert other
        developers when their changes will have an impact on the behavior of your
        module.  For example, if a developer removes a feature that you depend on,
        your test should break so they know that the feature is needed.
        """

        self.delayDisplay("Starting the test")

        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
        generalizedReformatLogic = slicer.modules.generalizedreformat.logic()
        self.assertIsNotNone(generalizedReformatLogic)

        # Get a dental CT scan
        import SampleData

        volumeNode = SampleData.SampleDataLogic().downloadDentalSurgery()[1]

        # Define curve
        curveNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode")
        curveNode.CreateDefaultDisplayNodes()
        # Add more curve points between control points than the default 10
        curveNode.GetCurveGenerator().SetNumberOfPointsPerInterpolatingSegment(25)
        curveNode.AddControlPoint(vtk.vtkVector3d(-45.85526315789473, -104.59210526315789, 74.67105263157896))
        curveNode.AddControlPoint(vtk.vtkVector3d(-50.9078947368421, -90.06578947368418, 66.4605263157895))
        curveNode.AddControlPoint(vtk.vtkVector3d(-62.27631578947368, -78.06578947368419, 60.7763157894737))
        curveNode.AddControlPoint(vtk.vtkVector3d(-71.86705891666716, -58.04403581456746, 57.84679891116521))
        curveNode.AddControlPoint(vtk.vtkVector3d(-74.73084356325877, -48.67611043794342, 57.00664267528636))
        curveNode.AddControlPoint(vtk.vtkVector3d(-88.17105263157895, -35.75, 55.092105263157904))
        curveNode.AddControlPoint(vtk.vtkVector3d(-99.53947368421052, -35.75, 55.092105263157904))
        curveNode.AddControlPoint(vtk.vtkVector3d(-107.75, -43.96052631578948, 55.092105263157904))
        curveNode.AddControlPoint(vtk.vtkVector3d(-112.80263157894736, -59.118421052631575, 56.355263157894754))
        curveNode.AddControlPoint(vtk.vtkVector3d(-115.32894736842104, -73.01315789473684, 60.144736842105274))
        curveNode.AddControlPoint(vtk.vtkVector3d(-125.43421052631578, -83.74999999999999, 60.7763157894737))
        curveNode.AddControlPoint(vtk.vtkVector3d(-132.3815789473684, -91.96052631578947, 63.934210526315795))
        curveNode.AddControlPoint(vtk.vtkVector3d(-137.43421052631578, -103.96052631578947, 67.72368421052633))

        fieldOfView = [40.0, 40.0]
        outputSpacing = [0.5, 0.5, 1.0]

        straighteningTransformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", "Straightening transform")
        generalizedReformatLogic.ComputeStraighteningTransform(straighteningTransformNode, curveNode, fieldOfView, outputSpacing[2])

        straightenedVolume = slicer.modules.volumes.logic().CloneVolume(volumeNode, volumeNode.GetName() + " straightened")
        generalizedReformatLogic.StraightenVolume(straightenedVolume, volumeNode, outputSpacing, straighteningTransformNode)

        panoramicVolume = slicer.modules.volumes.logic().CloneVolume(straightenedVolume, straightenedVolume.GetName() + " panoramic")
        generalizedReformatLogic.ProjectVolume(panoramicVolume, straightenedVolume)

        slicer.util.setSliceViewerLayers(background=straightenedVolume, fit=True, rotateToVolumePlane=True)

        self.delayDisplay("Test passed!")
