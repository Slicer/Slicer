import unittest

import vtk

import slicer


class SliceLogicNLayerTest(unittest.TestCase):
    """Test the N-layer API of vtkMRMLSliceCompositeNode and vtkMRMLSliceLogic.

    Layers 0, 1 and 2 are the background, foreground and label layers. Layers 3 and
    above are additional layers, which are not shown in the GUI yet, so this test also
    serves as an example of how to show more than two volumes in a slice view
    (see https://github.com/Slicer/Slicer/issues/8294).

    The slice logic is not connected to a slice widget, so the test runs without the main
    window. Each layer shows a volume of constant intensity through a grey color table
    (window 255, level 127.5), so a pixel of a layer is exactly (value, value, value, 255)
    inside the volume and fully transparent outside of it. The volumes cover different
    regions, so each blending mode can be checked at points where a known set of layers overlap:

      A = (3, 4): background, foreground, layer 3
      B = (9, 4): background, foreground, layer 3, layer 4
      C = (14, 2): background, foreground, layer 4
      D = (9, 15): background, foreground
      E = (17, 6): background, foreground, layer 4, label
      F = (22, 4): layer 4 only (outside of the background volume)
    """

    BACKGROUND = 120
    FOREGROUND = 60
    LAYER3 = 30
    LAYER4 = 15

    A = (3, 4, 0)
    B = (9, 4, 0)
    C = (14, 2, 0)
    D = (9, 15, 0)
    E = (17, 6, 0)
    F = (22, 4, 0)

    def setUp(self):
        self.scene = slicer.vtkMRMLScene()
        slicer.vtkMRMLSliceNode.AddDefaultSliceOrientationPresets(self.scene)

        self.greyColorNode = self.scene.AddNewNodeByClass("vtkMRMLColorTableNode")
        self.greyColorNode.SetTypeToGrey()
        self.labelsColorNode = self.scene.AddNewNodeByClass("vtkMRMLColorTableNode")
        self.labelsColorNode.SetTypeToLabels()

        # Volume geometry is given as the (i, j, k) index of the first voxel and the number of voxels
        # along each axis. Spacing is 1 mm and IJK to RAS is identity, so voxel (i, j, k) is at (i, j, k).
        self.backgroundVolume = self._addScalarVolume("Background", (0, 0, -1), (20, 20, 3), self.BACKGROUND)
        self.foregroundVolume = self._addScalarVolume("Foreground", (0, 0, -1), (20, 20, 3), self.FOREGROUND)
        self.layer3Volume = self._addScalarVolume("Layer3", (0, 0, -1), (12, 10, 3), self.LAYER3)
        self.layer4Volume = self._addScalarVolume("Layer4", (8, 0, -1), (16, 10, 3), self.LAYER4)
        self.labelVolume = self._addLabelVolume("Label", (0, 0, -1), (20, 20, 3), (16, 5), (3, 3))

        self.sliceLogic = slicer.vtkMRMLSliceLogic()
        self.sliceLogic.SetMRMLScene(self.scene)
        self.sliceNode = self.sliceLogic.AddSliceNode("Red")
        self.sliceNode.SetOrientationToAxial()
        self.sliceLogic.ResizeSliceNode(64, 64)
        self.sliceNode.SetFieldOfView(32.0, 32.0, 1.0)
        self.sliceNode.JumpSliceByCentering(12.0, 10.0, 0.0)

        self.compositeNode = self.sliceLogic.GetSliceCompositeNode()
        self.compositeNode.SetNthLayerVolume(slicer.vtkMRMLSliceCompositeNode.LayerBackground, self.backgroundVolume)
        self.compositeNode.SetNthLayerVolume(slicer.vtkMRMLSliceCompositeNode.LayerForeground, self.foregroundVolume)
        self.compositeNode.SetNthLayerVolume(3, self.layer3Volume)
        self.compositeNode.SetNthLayerVolume(4, self.layer4Volume)
        for layerIndex in range(5):
            self.compositeNode.SetNthLayerOpacity(layerIndex, 1.0)
        self.compositeNode.SetClipToBackgroundVolume(True)
        self.compositeNode.SetCompositing(slicer.vtkMRMLSliceCompositeNode.Alpha)

    def tearDown(self):
        self.sliceLogic.SetMRMLScene(None)
        self.scene.Clear(1)

    def _addScalarVolume(self, name, origin, dimensions, value):
        imageData = vtk.vtkImageData()
        imageData.SetDimensions(dimensions)
        imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        imageData.GetPointData().GetScalars().Fill(value)
        return self._addVolume("vtkMRMLScalarVolumeNode", "vtkMRMLScalarVolumeDisplayNode", name, origin, imageData, self.greyColorNode)

    def _addLabelVolume(self, name, origin, dimensions, labelStart, labelSize):
        imageData = vtk.vtkImageData()
        imageData.SetDimensions(dimensions)
        imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        imageData.GetPointData().GetScalars().Fill(0)
        for i in range(labelStart[0] - origin[0], labelStart[0] - origin[0] + labelSize[0]):
            for j in range(labelStart[1] - origin[1], labelStart[1] - origin[1] + labelSize[1]):
                for k in range(dimensions[2]):
                    imageData.SetScalarComponentFromDouble(i, j, k, 0, 1)
        return self._addVolume("vtkMRMLLabelMapVolumeNode", "vtkMRMLLabelMapVolumeDisplayNode", name, origin, imageData, self.labelsColorNode)

    def _addVolume(self, volumeClassName, displayClassName, name, origin, imageData, colorNode):
        volumeNode = self.scene.AddNewNodeByClass(volumeClassName, name)
        volumeNode.SetOrigin(origin)
        volumeNode.SetSpacing(1.0, 1.0, 1.0)
        volumeNode.SetAndObserveImageData(imageData)
        displayNode = self.scene.AddNewNodeByClass(displayClassName)
        if displayClassName == "vtkMRMLScalarVolumeDisplayNode":
            displayNode.AutoWindowLevelOff()
            displayNode.SetWindowLevel(255.0, 127.5)
            displayNode.SetInterpolate(False)
        displayNode.SetAndObserveColorNodeID(colorNode.GetID())
        volumeNode.SetAndObserveDisplayNodeID(displayNode.GetID())
        return volumeNode

    def _rgbaAt(self, ras, layerIndex=None):
        """Return the RGBA pixel of the blended slice image (or of a single layer) at a RAS position."""
        self.sliceLogic.UpdatePipeline()
        if layerIndex is None:
            port = self.sliceLogic.GetImageDataConnection()
        else:
            port = self.sliceLogic.GetNthLayerImageDataConnection(layerIndex)
        self.assertIsNotNone(port)
        algorithm = port.GetProducer()
        algorithm.Update()
        image = algorithm.GetOutputDataObject(port.GetIndex())
        rasToXY = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Invert(self.sliceNode.GetXYToRAS(), rasToXY)
        xy = rasToXY.MultiplyPoint([ras[0], ras[1], ras[2], 1.0])
        return [int(image.GetScalarComponentAsDouble(round(xy[0]), round(xy[1]), 0, component)) for component in range(4)]

    def assertBlendedColorEqual(self, ras, expectedValue, msg=None):
        self.assertEqual(self._rgbaAt(ras)[:3], [expectedValue] * 3, msg)

    def test_CompositeNodeLayerAPI(self):
        compositeNode = self.compositeNode
        self.assertEqual(compositeNode.GetNumberOfAdditionalLayers(), 2)

        # The predefined layers are also available through the N-layer API
        self.assertEqual(compositeNode.GetNthLayerVolumeID(0), compositeNode.GetBackgroundVolumeID())
        self.assertEqual(compositeNode.GetNthLayerVolumeID(1), compositeNode.GetForegroundVolumeID())
        self.assertIsNone(compositeNode.GetNthLayerVolume(2))

        self.assertEqual(compositeNode.GetNthLayerVolume(3), self.layer3Volume)
        self.assertEqual(compositeNode.GetNthLayerVolumeID(4), self.layer4Volume.GetID())
        self.assertIsNone(compositeNode.GetNthLayerVolume(5))

        # Opacity of a layer that has never been set is 1.0
        self.assertEqual(compositeNode.GetNthLayerOpacity(10), 1.0)
        compositeNode.SetNthLayerOpacity(4, 0.25)
        self.assertEqual(compositeNode.GetNthLayerOpacity(4), 0.25)

        # Opacities of the additional layers are copied with the node content
        copiedCompositeNode = slicer.vtkMRMLSliceCompositeNode()
        copiedCompositeNode.CopyContent(compositeNode)
        self.assertEqual(copiedCompositeNode.GetNthLayerOpacity(4), 0.25)

        # Removing the last additional layer
        compositeNode.SetNthLayerVolume(4, None)
        self.assertEqual(compositeNode.GetNumberOfAdditionalLayers(), 1)
        self.assertIsNone(compositeNode.GetNthLayerVolume(4))

    def test_SliceLogicCreatesAdditionalLayers(self):
        self.sliceLogic.UpdatePipeline()
        for layerIndex, volumeNode in [(3, self.layer3Volume), (4, self.layer4Volume)]:
            layerLogic = self.sliceLogic.GetNthLayer(layerIndex)
            self.assertIsNotNone(layerLogic, f"layer {layerIndex}")
            self.assertEqual(layerLogic.GetVolumeNode(), volumeNode, f"layer {layerIndex}")
            self.assertEqual(self.sliceLogic.GetNthLayerVolumeNode(layerIndex), volumeNode, f"layer {layerIndex}")

        # Each additional layer shows its own volume
        self.assertEqual(self._rgbaAt(self.A, 3), [self.LAYER3] * 3 + [255])
        self.assertEqual(self._rgbaAt(self.C, 4), [self.LAYER4] * 3 + [255])
        self.assertEqual(self._rgbaAt(self.D, 4)[3], 0)

    def test_AlphaCompositing(self):
        # Layers are drawn in index order, so layer 4 is on top of layer 3,
        # and the additional layers are on top of the foreground
        self.assertBlendedColorEqual(self.A, self.LAYER3)
        self.assertBlendedColorEqual(self.B, self.LAYER4)
        self.assertBlendedColorEqual(self.C, self.LAYER4)
        self.assertBlendedColorEqual(self.D, self.FOREGROUND)

        # A layer with zero opacity is not visible
        self.compositeNode.SetNthLayerOpacity(4, 0.0)
        self.assertBlendedColorEqual(self.B, self.LAYER3)
        self.assertBlendedColorEqual(self.C, self.FOREGROUND)

        # A layer with 0.5 opacity is mixed half and half with the layers below it
        self.compositeNode.SetNthLayerOpacity(3, 0.5)
        self.assertBlendedColorEqual(self.A, (self.FOREGROUND + self.LAYER3) // 2)

        # An additional layer that is removed is not shown anymore
        self.compositeNode.SetNthLayerOpacity(3, 1.0)
        self.compositeNode.SetNthLayerOpacity(4, 1.0)
        self.compositeNode.SetNthLayerVolume(4, None)
        self.assertBlendedColorEqual(self.B, self.LAYER3)
        self.assertBlendedColorEqual(self.C, self.FOREGROUND)

    def test_LabelLayerIsAboveAdditionalLayers(self):
        self.compositeNode.SetNthLayerVolume(slicer.vtkMRMLSliceCompositeNode.LayerLabel, self.labelVolume)
        labelRGBA = self._rgbaAt(self.E, slicer.vtkMRMLSliceCompositeNode.LayerLabel)
        self.assertEqual(labelRGBA[3], 255)
        self.assertNotEqual(labelRGBA[:3], [self.LAYER4] * 3)
        self.assertEqual(self._rgbaAt(self.E)[:3], labelRGBA[:3])
        # Outside of the label the additional layer is still visible
        self.assertBlendedColorEqual(self.C, self.LAYER4)

    def test_ClipToBackgroundVolume(self):
        # Point F is covered only by layer 4, which extends beyond the background volume
        self.compositeNode.SetClipToBackgroundVolume(True)
        self.assertEqual(self._rgbaAt(self.F)[3], 0)
        self.compositeNode.SetClipToBackgroundVolume(False)
        self.assertEqual(self._rgbaAt(self.F), [self.LAYER4] * 3 + [255])

    def test_ReverseAlphaCompositing(self):
        # The stacking order is reversed, so the background is drawn on top of all other layers
        self.compositeNode.SetCompositing(slicer.vtkMRMLSliceCompositeNode.ReverseAlpha)
        for point in [self.A, self.B, self.C, self.D]:
            self.assertBlendedColorEqual(point, self.BACKGROUND, f"point {point}")

        # With only background and foreground, the foreground opacity is the opacity
        # of the background over the foreground
        self.compositeNode.SetNthLayerVolume(4, None)
        self.compositeNode.SetNthLayerVolume(3, None)
        self.compositeNode.SetNthLayerOpacity(1, 0.5)
        self.assertBlendedColorEqual(self.D, (self.BACKGROUND + self.FOREGROUND) // 2)

    def test_AddAndSubtractCompositing(self):
        self.compositeNode.SetCompositing(slicer.vtkMRMLSliceCompositeNode.Add)
        self.assertBlendedColorEqual(self.A, self.BACKGROUND + self.FOREGROUND + self.LAYER3)
        self.assertBlendedColorEqual(self.B, self.BACKGROUND + self.FOREGROUND + self.LAYER3 + self.LAYER4)
        self.assertBlendedColorEqual(self.C, self.BACKGROUND + self.FOREGROUND + self.LAYER4)
        self.assertBlendedColorEqual(self.D, self.BACKGROUND + self.FOREGROUND)

        # Opacity of a layer scales the value that is added
        self.compositeNode.SetNthLayerOpacity(3, 0.5)
        self.assertBlendedColorEqual(self.A, self.BACKGROUND + self.FOREGROUND + self.LAYER3 // 2)
        self.compositeNode.SetNthLayerOpacity(3, 1.0)

        self.compositeNode.SetCompositing(slicer.vtkMRMLSliceCompositeNode.Subtract)
        self.assertBlendedColorEqual(self.A, self.BACKGROUND - self.FOREGROUND - self.LAYER3)
        self.assertBlendedColorEqual(self.B, self.BACKGROUND - self.FOREGROUND - self.LAYER3 - self.LAYER4)
        self.assertBlendedColorEqual(self.C, self.BACKGROUND - self.FOREGROUND - self.LAYER4)
        self.assertBlendedColorEqual(self.D, self.BACKGROUND - self.FOREGROUND)
