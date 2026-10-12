import unittest

import numpy as np

import slicer


class VolumeRenderingMultiComponentTest(unittest.TestCase):
    """Transfer functions of all components of a multi-component volume must follow the image intensity range."""

    def setUp(self):
        slicer.mrmlScene.Clear()
        self.addCleanup(slicer.mrmlScene.Clear)

        # 16-bit 4-component volume (similar to multichannel microscopy), the components have different
        # intensity ranges that are well beyond the 0..1024 range of VTK default transfer functions.
        # The last component is constant, as alpha channels often are.
        shape = (10, 12, 14)
        rng = np.random.default_rng(0)
        voxels = np.zeros((*shape, 4), dtype=np.uint16)
        voxels[..., 0] = rng.integers(154, 3735, shape)
        voxels[..., 1] = rng.integers(46, 7159, shape)
        voxels[..., 2] = rng.integers(75, 7690, shape)
        voxels[..., 3] = 255
        self.volume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode")
        slicer.util.updateVolumeFromArray(self.volume, voxels)
        self.volume.CreateDefaultDisplayNodes()

        self.logic = slicer.modules.volumerendering.logic()
        self.displayNode = self.logic.CreateDefaultVolumeRenderingNodes(self.volume)
        self.volumeProperty = self.displayNode.GetVolumePropertyNode().GetVolumeProperty()
        self.volumeProperty.SetIndependentComponents(True)
        self.scalars = self.volume.GetImageData().GetPointData().GetScalars()

    def assertComponentTransferFunctionsMatchImage(self):
        for component in range(1, 4):
            with self.subTest(component=component):
                dataRange = self.scalars.GetRange(component)
                opacity = self.volumeProperty.GetScalarOpacity(component)
                color = self.volumeProperty.GetRGBTransferFunction(component)
                self.assertAlmostEqual(opacity.GetRange()[0], dataRange[0], places=2)
                self.assertAlmostEqual(opacity.GetRange()[1], dataRange[1], places=2)
                self.assertAlmostEqual(color.GetRange()[0], dataRange[0], places=2)
                self.assertAlmostEqual(color.GetRange()[1], dataRange[1], places=2)
                # Lowest intensity must be transparent, otherwise the volume is rendered as a solid block
                self.assertEqual(opacity.GetValue(dataRange[0]), 0.0)

    def testDefaultTransferFunctions(self):
        self.assertComponentTransferFunctionsMatchImage()

    def testApplyPresetKeepsAdditionalComponents(self):
        slicer.util.selectModule("VolumeRendering")
        moduleWidget = slicer.modules.volumerendering.widgetRepresentation()
        moduleWidget.setMRMLVolumeNode(self.volume)
        presetComboBox = slicer.util.findChild(moduleWidget, "PresetComboBox")
        presetComboBox.applyPreset(self.logic.GetPresetByName("MR-Default"))
        self.assertComponentTransferFunctionsMatchImage()

    def testNoGeneratedAlphaChannelForIndependentComponents(self):
        # An alpha channel is generated for 3-component (RGB) volumes. In independent components mode it must not be added,
        # because it would be rendered as an additional, fully opaque component (the volume would appear as a solid block).
        rgbVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode")
        slicer.util.updateVolumeFromArray(rgbVolume, np.random.default_rng(0).integers(0, 256, (10, 12, 14, 3)).astype(np.uint8))
        rgbVolume.CreateDefaultDisplayNodes()
        displayNode = self.logic.CreateDefaultVolumeRenderingNodes(rgbVolume)
        displayNode.SetVisibility(True)
        volumeProperty = displayNode.GetVolumePropertyNode().GetVolumeProperty()

        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
        view = layoutManager.threeDWidget(0).threeDView()
        displayableManager = view.displayableManagerByClassName("vtkMRMLVolumeRenderingDisplayableManager")

        def renderedComponents():
            mapper = displayableManager.GetVolumeMapper(rgbVolume)
            mapper.GetInputAlgorithm().Update()
            return mapper.GetInput().GetNumberOfScalarComponents()

        volumeProperty.SetIndependentComponents(False)
        slicer.app.processEvents()
        self.assertEqual(renderedComponents(), 4)

        volumeProperty.SetIndependentComponents(True)
        slicer.app.processEvents()
        self.assertEqual(renderedComponents(), 3)

    def testChartsExtentFollowsCurrentComponent(self):
        slicer.util.selectModule("VolumeRendering")
        moduleWidget = slicer.modules.volumerendering.widgetRepresentation()
        moduleWidget.setMRMLVolumeNode(self.volume)
        slicer.app.processEvents()
        componentSpinBox = slicer.util.findChild(moduleWidget, "ComponentSpinBox")
        volumePropertyWidget = slicer.util.findChild(moduleWidget, "VolumePropertyWidget")
        # Switch back and forth, as the extent of the previously displayed component must not be reused
        for component in [1, 2, 3, 0, 2, 1]:
            with self.subTest(component=component):
                componentSpinBox.setValue(component)
                slicer.app.processEvents()
                extent = volumePropertyWidget.chartsExtent()
                opacityRange = self.volumeProperty.GetScalarOpacity(component).GetRange()
                self.assertLessEqual(extent[0], opacityRange[0])
                self.assertGreaterEqual(extent[1], opacityRange[1])
