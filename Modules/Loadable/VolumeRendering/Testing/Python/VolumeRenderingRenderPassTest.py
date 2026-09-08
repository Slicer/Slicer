import unittest

import vtk

import slicer


class VolumeRenderingRenderPassTest(unittest.TestCase):
    def setUp(self):
        self.enterContext(slicer.util.RenderBlocker())
        slicer.mrmlScene.Clear()
        self.addCleanup(slicer.mrmlScene.Clear)

        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
        self.view = layoutManager.threeDWidget(0).threeDView()

        source = vtk.vtkRTAnalyticSource()
        source.SetWholeExtent(-2, 2, -2, 2, -2, 2)
        source.Update()
        self.volume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
        self.volume.SetAndObserveImageData(source.GetOutput())
        self.volume.CreateDefaultDisplayNodes()
        displayNode = slicer.modules.volumerendering.logic().CreateDefaultVolumeRenderingNodes(self.volume)
        displayNode.SetVisibility(True)

    def testRenderPassReferences(self):
        """MIP and MinIP must preserve non-SSAO passes and release their references."""
        manager = self.view.displayableManagerByClassName("vtkMRMLVolumeRenderingDisplayableManager")
        information = manager.GetVolumeActor(self.volume).GetPropertyKeys()
        key = vtk.vtkOpenGLRenderPass.RenderPasses()

        for technique in (slicer.vtkMRMLViewNode.MaximumIntensityProjection, slicer.vtkMRMLViewNode.MinimumIntensityProjection):
            with self.subTest(technique=technique):
                self.view.mrmlViewNode().SetRaycastTechnique(technique)
                self.assertFalse(information.Has(key))
                peeling = vtk.vtkDualDepthPeelingPass()
                peelingReferences = peeling.GetReferenceCount()

                # The first Append emits ModifiedEvent while its new list is still empty.
                information.Append(key, peeling)
                self.assertTrue(information.Has(key))
                self.assertEqual(information.Length(key), 1)
                self.assertEqual(information.Get(key, 0), peeling)

                ssao = vtk.vtkSSAOPass()
                ssaoReferences = ssao.GetReferenceCount()
                information.Append(key, ssao)
                # SSAO sets this override after appending itself in PreRenderProp.
                information.Set(vtk.vtkOpenGLActor.GLDepthMaskOverride(), 1)
                self.assertFalse(information.Has(vtk.vtkOpenGLActor.GLDepthMaskOverride()))
                self.assertEqual(information.Length(key), 1)
                self.assertEqual(information.Get(key, 0), peeling)
                self.assertEqual(ssao.GetReferenceCount(), ssaoReferences)

                information.Remove(key, peeling)
                self.assertEqual(peeling.GetReferenceCount(), peelingReferences)
                information.Remove(key)

                # Removing the only SSAO pass must still remove the empty list.
                information.Append(key, ssao)
                information.Set(vtk.vtkOpenGLActor.GLDepthMaskOverride(), 1)
                self.assertFalse(information.Has(key))
                self.assertEqual(ssao.GetReferenceCount(), ssaoReferences)
