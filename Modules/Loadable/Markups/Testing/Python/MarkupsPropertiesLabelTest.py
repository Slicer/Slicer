import json
import os
import tempfile
import unittest

import vtk
import slicer


class MarkupsPropertiesLabelTest(unittest.TestCase):
    def setUp(self):
        slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
        self.originalNodeIDs = {node.GetID() for node in slicer.util.getNodesByClass("vtkMRMLNode")}
        self.selectionNode = slicer.app.applicationLogic().GetSelectionNode()
        self.originalActiveID = self.selectionNode.GetActivePlaceNodeID()
        self.originalActiveClass = self.selectionNode.GetActivePlaceNodeClassName()
        self.angle = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsAngleNode", "Label test")
        self.display = self.angle.GetDisplayNode()
        # Keep test geometry out of the application's existing views.
        self.display.AddViewNodeID("PropertiesLabelTestView")
        for point in [(30, 0, 0), (0, 0, 0), (0, 30, 0)]:
            self.angle.AddControlPoint(vtk.vtkVector3d(*point))
        self.renderer = vtk.vtkRenderer()
        self.window = vtk.vtkRenderWindow()
        self.window.SetOffScreenRendering(True)
        self.window.SetSize(600, 600)
        self.window.AddRenderer(self.renderer)
        self.renderer.GetActiveCamera().SetPosition(0, 0, 200)
        self.renderer.GetActiveCamera().SetFocalPoint(0, 0, 0)
        self.renderer.GetActiveCamera().ParallelProjectionOn()
        self.renderer.GetActiveCamera().SetParallelScale(100)
        self.viewScene = slicer.vtkMRMLScene()
        self.views = []
        self.representations = []

    def tearDown(self):
        self.renderer.RemoveAllViewProps()
        self.representations.clear()
        self.window.Finalize()
        for node in slicer.util.getNodesByClass("vtkMRMLNode"):
            if node.GetID() not in self.originalNodeIDs:
                slicer.mrmlScene.RemoveNode(node)
        self.selectionNode.SetReferenceActivePlaceNodeID(self.originalActiveID)
        self.selectionNode.SetReferenceActivePlaceNodeClassName(self.originalActiveClass)

    def representation(self, sliceView=False):
        # Keep test views in a separate scene: adding an incomplete slice node to
        # the application scene triggers layout-manager view construction.
        view = slicer.vtkMRMLSliceNode() if sliceView else slicer.vtkMRMLViewNode()
        view.SetSingletonTag("PropertiesLabelTest")
        view.SetLayoutName("PropertiesLabelTest")
        self.viewScene.AddNode(view)
        self.views.append(view)
        self.display.AddViewNodeID(view.GetID())
        if sliceView:
            view.SetDimensions(600, 600, 1)
            view.SetFieldOfView(200, 200, 1)
            rep = slicer.vtkSlicerAngleRepresentation2D()
        else:
            rep = slicer.vtkSlicerAngleRepresentation3D()
        rep.SetRenderer(self.renderer)
        rep.SetViewNode(view)
        rep.SetMarkupsDisplayNode(self.display)
        rep.UpdateFromMRML(None, 0)
        self.representations.append(rep)
        return rep

    def updateDisplay(self, rep):
        rep.UpdateFromMRML(self.angle, slicer.vtkMRMLDisplayableNode.DisplayModifiedEvent)
        rep.GetTextActor().SetDisplayPosition(450, 450)

    def pick(self, rep, position):
        event = slicer.vtkMRMLInteractionEventData()
        event.SetType(vtk.vtkCommand.MouseMoveEvent)
        event.SetDisplayPosition(position)
        component, index, distance = vtk.mutable(-1), vtk.mutable(-1), vtk.mutable(1e99)
        rep.CanInteract(event, component, index, distance)
        return int(component), int(index), float(distance)

    def testText(self):
        measurement = self.angle.GetMeasurement("angle")
        value = measurement.GetValueWithUnitsAsPrintableString()
        rep = self.representation()
        self.assertTrue(self.display.GetPropertiesLabelIncludesNodeName())
        self.assertEqual(self.angle.GetPropertiesLabelText(), "Label test: " + value)
        self.assertEqual(rep.GetTextActor().GetInput(), self.angle.GetPropertiesLabelText())
        self.display.SetPropertiesLabelIncludesNodeName(False)
        self.updateDisplay(rep)
        self.assertEqual(self.angle.GetPropertiesLabelText(), value)
        self.assertEqual(rep.GetTextActor().GetInput(), value)
        self.assertTrue(measurement.GetEnabled())
        self.angle.SetName("Renamed")
        self.assertEqual(self.angle.GetPropertiesLabelText(), value)
        self.display.SetPropertiesLabelIncludesNodeName(True)
        self.updateDisplay(rep)
        self.assertEqual(rep.GetTextActor().GetInput(), "Renamed: " + value)
        self.angle.SetName("")
        self.assertEqual(self.angle.GetPropertiesLabelText(), value)
        measurement.SetEnabled(False)
        self.assertEqual(self.angle.GetPropertiesLabelText(), "")
        self.angle.SetName("Name only")
        self.assertEqual(self.angle.GetPropertiesLabelText(), "Name only")
        self.display.SetPropertiesLabelIncludesNodeName(False)
        self.assertEqual(self.angle.GetPropertiesLabelText(), "")

    def testMultipleMeasurements(self):
        extra = slicer.vtkMRMLStaticMeasurement()
        extra.SetName("extra")
        extra.SetUnits("mm")
        extra.SetValue(12)
        self.angle.AddMeasurement(extra)
        self.angle.UpdateAllMeasurements()
        first = "angle: " + self.angle.GetMeasurement("angle").GetValueWithUnitsAsPrintableString()
        second = "extra: " + extra.GetValueWithUnitsAsPrintableString()
        self.assertEqual(self.angle.GetPropertiesLabelText(), "Label test:\n" + first + "\n" + second)
        self.display.SetPropertiesLabelIncludesNodeName(False)
        self.assertEqual(self.angle.GetPropertiesLabelText(), first + "\n" + second)

    def testVisibility(self):
        for sliceView in (False, True):
            rep = self.representation(sliceView)
            for visible in (False, True, False, True):
                self.display.SetPropertiesLabelVisibility(visible)
                self.updateDisplay(rep)
                self.assertEqual(bool(rep.GetTextActor().GetVisibility()), visible)
            self.angle.UnsetNthControlPointPosition(2)
            rep.UpdateFromMRML(self.angle, slicer.vtkMRMLMarkupsNode.PointPositionUndefinedEvent)
            self.assertFalse(rep.GetTextActor().GetVisibility())
            self.angle.SetNthControlPointPosition(2, 0, 30, 0)
            rep.UpdateFromMRML(self.angle, slicer.vtkMRMLMarkupsNode.PointPositionDefinedEvent)
            self.assertTrue(rep.GetTextActor().GetVisibility())

    def testPersistence(self):
        self.display.SetPropertiesLabelIncludesNodeName(False)
        copied = slicer.vtkMRMLMarkupsDisplayNode()
        copied.CopyContent(self.display)
        self.assertFalse(copied.GetPropertiesLabelIncludesNodeName())
        with tempfile.TemporaryDirectory() as directory:
            filename = os.path.join(directory, "angle.mkp.json")
            self.assertTrue(slicer.util.saveNode(self.angle, filename))
            with open(filename) as stream:
                document = json.load(stream)
            self.assertIs(document["markups"][0]["display"]["propertiesLabelIncludesNodeName"], False)
            loaded = slicer.util.loadMarkups(filename)
            self.assertFalse(loaded.GetDisplayNode().GetPropertiesLabelIncludesNodeName())
            # Older files omit the option and must retain the default name prefix.
            del document["markups"][0]["display"]["propertiesLabelIncludesNodeName"]
            with open(filename, "w") as stream:
                json.dump(document, stream)
            legacy = slicer.util.loadMarkups(filename)
            self.assertTrue(legacy.GetDisplayNode().GetPropertiesLabelIncludesNodeName())
        scene = slicer.vtkMRMLScene()
        scene.AddNode(copied)
        scene.SetSaveToXMLString(True)
        scene.Commit()
        restoredScene = slicer.vtkMRMLScene()
        restoredScene.SetLoadFromXMLString(True)
        restoredScene.SetSceneXMLString(scene.GetSceneXMLString())
        restoredScene.Import()
        restored = restoredScene.GetFirstNodeByClass("vtkMRMLMarkupsDisplayNode")
        self.assertIsNotNone(restored)
        self.assertFalse(restored.GetPropertiesLabelIncludesNodeName())

    def testDisplayPanel(self):
        widget = slicer.qMRMLMarkupsDisplayNodeWidget()
        try:
            widget.setMRMLScene(slicer.mrmlScene)
            widget.setMRMLMarkupsDisplayNode(self.display)
            checkbox = slicer.util.findChild(widget, "PropertiesLabelIncludesNodeNameCheckBox")
            self.assertIsNotNone(checkbox)
            self.assertTrue(checkbox.checked)
            checkbox.setChecked(False)
            self.assertFalse(self.display.GetPropertiesLabelIncludesNodeName())
            self.assertEqual(self.angle.GetPropertiesLabelText(), self.angle.GetMeasurement("angle").GetValueWithUnitsAsPrintableString())
            checkbox.setChecked(True)
            self.assertTrue(self.display.GetPropertiesLabelIncludesNodeName())
        finally:
            widget.deleteLater()

    def testPicking(self):
        for sliceView in (False, True):
            rep = self.representation(sliceView)
            actor = rep.GetTextActor()
            # Put the label away from geometry, and exercise different text justifications.
            for justification in (vtk.VTK_TEXT_LEFT, vtk.VTK_TEXT_CENTERED, vtk.VTK_TEXT_RIGHT):
                actor.GetTextProperty().SetJustification(justification)
                actor.SetDisplayPosition(450, 450)
                bounds = [0.0] * 4
                actor.GetBoundingBox(self.renderer, bounds)
                position = (round(450 + (bounds[0] + bounds[1]) / 2), round(450 + (bounds[2] + bounds[3]) / 2))
                picked = self.pick(rep, position)[:2]
                self.assertEqual(picked, (self.display.ComponentPropertiesLabel, 0), (sliceView, justification, bounds, position, picked))
                outside = (round(450 + bounds[1] + 10), position[1])
                self.assertEqual(self.pick(rep, outside)[0], self.display.ComponentNone)
            self.display.SetActiveComponent(self.display.ComponentPropertiesLabel, 0)
            self.updateDisplay(rep)
            self.assertEqual(actor.GetTextProperty().GetColor(), self.display.GetActiveColor())
            self.display.SetActiveComponent(self.display.ComponentNone, -1)
            self.display.SetPropertiesLabelVisibility(False)
            self.updateDisplay(rep)
            self.assertEqual(self.pick(rep, position)[0], self.display.ComponentNone)
            self.display.SetPropertiesLabelVisibility(True)
            self.updateDisplay(rep)
            self.angle.SetLocked(True)
            self.assertEqual(self.pick(rep, position)[0], self.display.ComponentNone)
            self.angle.SetLocked(False)
            for index in range(3):
                self.angle.SetNthControlPointLocked(index, True)
            # Point lock changes update the representation and restore the
            # computed label position, so move it back to the test location.
            rep.UpdateFromMRML(self.angle, slicer.vtkMRMLMarkupsNode.PointModifiedEvent)
            actor.SetDisplayPosition(450, 450)
            actor.GetBoundingBox(self.renderer, bounds)
            position = (round(450 + (bounds[0] + bounds[1]) / 2), round(450 + (bounds[2] + bounds[3]) / 2))
            self.assertEqual(self.pick(rep, position)[0], self.display.ComponentPropertiesLabel)
            self.display.SetVisibility(False)
            self.updateDisplay(rep)
            self.assertEqual(self.pick(rep, position)[0], self.display.ComponentNone)
            self.display.SetVisibility(True)

    def testLabelClick(self):
        rep = self.representation()
        actor = rep.GetTextActor()
        actor.SetDisplayPosition(450, 450)
        bounds = [0.0] * 4
        actor.GetBoundingBox(self.renderer, bounds)
        position = (round(450 + (bounds[0] + bounds[1]) / 2), round(450 + (bounds[2] + bounds[3]) / 2))
        widget = slicer.vtkSlicerAngleWidget()
        widget.SetRepresentation(rep)
        event = slicer.vtkMRMLInteractionEventData()
        event.SetDisplayPosition(position)
        event.SetWorldPosition((0, 0, 0))
        event.SetType(vtk.vtkCommand.MouseMoveEvent)
        self.assertTrue(widget.ProcessInteractionEvent(event))
        self.assertEqual(widget.GetWidgetState(), widget.WidgetStateOnPropertiesLabel)
        initialPoints = [tuple(self.angle.GetNthControlPointPositionVector(i)) for i in range(3)]
        event.SetType(vtk.vtkCommand.LeftButtonPressEvent)
        self.assertTrue(widget.ProcessInteractionEvent(event))
        self.assertEqual(widget.GetWidgetState(), widget.WidgetStateOnPropertiesLabel)
        event.SetType(vtk.vtkCommand.LeftButtonReleaseEvent)
        self.assertFalse(widget.ProcessInteractionEvent(event))
        event.SetType(slicer.vtkMRMLInteractionEventData.LeftButtonClickEvent)
        self.assertTrue(widget.ProcessInteractionEvent(event))
        self.assertEqual(self.selectionNode.GetActivePlaceNodeID(), self.angle.GetID())
        self.assertEqual(initialPoints, [tuple(self.angle.GetNthControlPointPositionVector(i)) for i in range(3)])
        receivedEvents = []

        @vtk.calldata_type(vtk.VTK_OBJECT)
        def onEvent(caller, eventName, eventData):
            receivedEvents.append((eventData.GetComponentType(), eventData.GetComponentIndex()))

        for eventType, displayEvent in [(slicer.vtkMRMLInteractionEventData.RightButtonClickEvent, self.display.MenuEvent)]:
            observer = self.display.AddObserver(displayEvent, onEvent)
            try:
                event.SetType(eventType)
                self.assertTrue(widget.ProcessInteractionEvent(event))
                self.assertEqual(receivedEvents[-1], (self.display.ComponentPropertiesLabel, 0))
            finally:
                self.display.RemoveObserver(observer)


suite = unittest.defaultTestLoader.loadTestsFromTestCase(MarkupsPropertiesLabelTest)
result = unittest.TextTestRunner(verbosity=2).run(suite)
if not result.wasSuccessful():
    raise RuntimeError("Properties label regression tests failed")
