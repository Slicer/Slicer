import vtk

import slicer
from slicer.ScriptedLoadableModule import *


#
# SlicerOrientationSelectorTest
#
class SlicerBoundsTest(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "BoundsTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Johan Andruejol (Kitware Inc)"]
        self.parent.helpText = """
      This test has been added to check the GetRASBounds and GetBounds methods.
      """
        self.parent.acknowledgementText = """
    This file was originally developed by Johan Andruejol, Kitware Inc.
    """


#
# SlicerBoundsTest
#
class SlicerBoundsTestWidget(ScriptedLoadableModuleWidget):
    """
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SlicerTransformInteractionTest1Logic
#
class SlicerBoundsTestLogic(ScriptedLoadableModuleLogic):
    """
    """


class SlicerBoundsTestTest(ScriptedLoadableModuleTest):
    """
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    def assertListAlmostEquals(self, list, expected):
        for list_item, expected_item in zip(list, expected):
            self.assertAlmostEqual(list_item, expected_item)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_Volume()
        self.test_Model()
        self.test_Segmentation()
        self.test_Markup()
        self.test_ROI()
        self.delayDisplay('Test completed.')

    def test_Volume(self):
        """ Test the GetRASBounds & GetBounds method on a volume.
        """
        # self.delayDisplay("Starting test_Volume")
        import SampleData
        volumeNode = SampleData.downloadSample('CTAAbdomenPanoramix')

        bounds = list(range(6))
        volumeNode.GetRASBounds(bounds)
        untransformedBounds = [-165.68283081054688, 161.62185668945312, 7.542130470275879, 245.78431797027588, -347.62799072265625, -25.12799072265625]
        self.assertListAlmostEquals(bounds, untransformedBounds)

        volumeNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        transform = vtk.vtkTransform()
        transform.Translate([-5.0, +42.0, -0.1])
        transform.RotateWXYZ(41, 0.7, 0.6, 75)
        transform.Scale(2, 3, 10)
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformNode.ApplyTransform(transform)

        volumeNode.SetAndObserveTransformNodeID(transformNode.GetID())
        transformedBounds = [-764.990027409413, 222.22539693955437, -157.26286179044325, 825.0196226274144, -3477.0246375801075, -244.4287311630153]
        volumeNode.GetRASBounds(bounds)
        self.assertListAlmostEquals(bounds, transformedBounds)

        volumeNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)
        # self.delayDisplay('test_Volume passed!')

    def test_Model(self):
        """ Test the GetRASBounds & GetBounds method on a model.
        """
        # self.delayDisplay("Starting test_Model")
        # Setup
        cubeSource = vtk.vtkCubeSource()
        cubeSource.SetXLength(500)
        cubeSource.SetYLength(200)
        cubeSource.SetZLength(300)
        cubeSource.SetCenter(10, -85, 0.7)

        rotation = vtk.vtkTransform()
        rotation.RotateX(15.0)
        rotation.RotateZ(78)

        applyTransform = vtk.vtkTransformPolyDataFilter()
        applyTransform.SetTransform(rotation)
        applyTransform.SetInputConnection(cubeSource.GetOutputPort())
        applyTransform.Update()

        modelNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLModelNode())
        modelNode.SetPolyDataConnection(applyTransform.GetOutputPort())

        # Testing
        bounds = list(range(6))
        modelNode.GetRASBounds(bounds)
        untransformedBounds = [-64.5710220336914, 235.01434326171875, -302.91339111328125, 287.3067932128906, -214.92703247070312, 212.1946258544922]
        self.assertListAlmostEquals(bounds, untransformedBounds)

        modelNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        transform = vtk.vtkTransform()
        transform.Translate([-5.0, +42.0, -0.1])
        transform.RotateWXYZ(41, 0.7, 0.6, 75)
        transform.Scale(2, 3, 10)
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformNode.ApplyTransform(transform)

        modelNode.SetAndObserveTransformNodeID(transformNode.GetID())
        transformedBounds = [-684.0789214975257, 961.8640451930095, -737.3987882515103, 1009.8075011032414, -2158.028473491281, 2129.118193451847]
        modelNode.GetRASBounds(bounds)
        self.assertListAlmostEquals(bounds, transformedBounds)

        modelNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)
        # self.delayDisplay('test_Model passed!')

    def test_Segmentation(self):
        """ Test the GetRASBounds & GetBounds method on a segmentation.
        """
        # self.delayDisplay("Starting test_Segmentation")
        cubeSource = vtk.vtkCubeSource()
        cubeSource.SetXLength(500)
        cubeSource.SetYLength(200)
        cubeSource.SetZLength(300)
        cubeSource.SetCenter(10, -85, 0.7)

        rotation = vtk.vtkTransform()
        rotation.RotateX(15.0)
        rotation.RotateZ(78)

        applyTransform = vtk.vtkTransformPolyDataFilter()
        applyTransform.SetTransform(rotation)
        applyTransform.SetInputConnection(cubeSource.GetOutputPort())
        applyTransform.Update()

        modelNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLModelNode())
        modelNode.SetPolyDataConnection(applyTransform.GetOutputPort())

        segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        segmentationLogic = slicer.modules.segmentations.logic()
        segmentationLogic.ImportModelToSegmentationNode(modelNode, segmentationNode)

        # Testing
        bounds = list(range(6))
        segmentationNode.GetRASBounds(bounds)
        untransformedBounds = [-65.41641522206768, 237.23434621664228, -303.75878430165756, 289.7072339384945, -217.463212035832, 213.6873140360733]
        self.assertListAlmostEquals(bounds, untransformedBounds)

        segmentationNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        transform = vtk.vtkTransform()
        transform.Translate([-5.0, +42.0, -0.1])
        transform.RotateWXYZ(41, 0.7, 0.6, 75)
        transform.Scale(2, 3, 10)
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformNode.ApplyTransform(transform)

        segmentationNode.SetAndObserveTransformNodeID(transformNode.GetID())
        transformedBounds = [-690.2701685073093, 966.991271911892, -740.4842166018336, 1018.2608117218165, -2183.4229718546612, 2144.1077463008546]
        segmentationNode.GetRASBounds(bounds)
        self.assertListAlmostEquals(bounds, transformedBounds)

        segmentationNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        # self.delayDisplay('test_Segmentation passed!')

    def test_Markup(self):
        """ Test the GetRASBounds & GetBounds method on a markup.
        """
        # self.delayDisplay("Starting test_Markup")
        markupNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
        markupNode.AddControlPoint([1.0, 0.0, 0.0])
        markupNode.AddControlPoint([-45.0, -90.0, -180.0])
        markupNode.AddControlPoint([-200.0, 500.0, -0.23])
        markupNode.AddControlPoint([1.0, 1003.01, 0.0])

        bounds = list(range(6))
        markupNode.GetRASBounds(bounds)
        untransformedBounds = [-200, 1.0, -90, 1003.01, -180.0, 0.0]
        self.assertListAlmostEquals(bounds, untransformedBounds)

        markupNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        transform = vtk.vtkTransform()
        transform.Translate([-5.0, +42.0, -0.1])
        transform.RotateWXYZ(41, 0.7, 0.6, 75)
        transform.Scale(2, 3, 10)
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformNode.ApplyTransform(transform)

        markupNode.SetAndObserveTransformNodeID(transformNode.GetID())
        transformedBounds = [-1977.3875985837567, 90.6250336838986, -213.3290140037272, 2314.3030541154367, -1801.9498682023534, 24.221433153858232]
        markupNode.GetRASBounds(bounds)
        self.assertListAlmostEquals(bounds, transformedBounds)

        markupNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)
        # self.delayDisplay('test_Markup passed!')

    def test_ROI(self):
        """ Test the GetRASBounds & GetBounds method on a ROI.
        """
        # self.delayDisplay("Starting test_ROI")
        roiNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode")
        roiNode.SetXYZ(100, 300, -0.689)
        roiNode.SetRadiusXYZ(700, 8, 45)

        bounds = list(range(6))
        roiNode.GetRASBounds(bounds)
        untransformedBounds = [-600, 800, 292, 308, -45.689, 44.311]
        self.assertListAlmostEquals(bounds, untransformedBounds)

        roiNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        transform = vtk.vtkTransform()
        transform.Translate([-5.0, +42.0, -0.1])
        transform.RotateWXYZ(41, 0.7, 0.6, 75)
        transform.Scale(2, 3, 10)
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformNode.ApplyTransform(transform)

        roiNode.SetAndObserveTransformNodeID(transformNode.GetID())
        transformedBounds = [-1520.2565880625004, 631.261028317266, -85.93765163061204, 1790.9115952348277, -454.6252695921299, 454.0147697244433]
        roiNode.GetRASBounds(bounds)
        self.assertListAlmostEquals(bounds, transformedBounds)

        roiNode.GetBounds(bounds)
        self.assertListAlmostEquals(bounds, untransformedBounds)

        # self.delayDisplay('test_ROI passed!')
