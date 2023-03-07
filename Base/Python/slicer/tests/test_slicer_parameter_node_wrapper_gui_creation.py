import pathlib
import unittest
from typing import Annotated, Union, Optional

import ctk
import qt

import slicer
from slicer.parameterNodeWrapper import *

from MRMLCorePython import (
    vtkMRMLNode,
    vtkMRMLModelNode,
    vtkMRMLScalarVolumeNode,
)


def findChildWithProperty(widget, property, propertyValue):
    for child in widget.findChildren(qt.QObject):
        if child.property(property) == propertyValue:
            return child
    return None


class ParameterNodeWrapperGuiCreationTest(unittest.TestCase):
    def setUp(self) -> None:
        return slicer.mrmlScene.Clear(0)

    def test_guiCreations_bool(self):
        self.assertIsInstance(createGui(bool), qt.QCheckBox)
        self.assertIsInstance(createGui(Annotated[bool, Default(True)]), qt.QCheckBox)
        self.assertIsInstance(createGui(Annotated[bool, Choice([True, False])]), qt.QComboBox)

    def test_guiCreations_int(self):
        self.assertIsInstance(createGui(int), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, Default(3)]), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, Minimum(-3)]), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, Maximum(3)]), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, Minimum(3), Maximum(7)]), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, WithinRange(3, 7)]), qt.QSpinBox)
        self.assertIsInstance(createGui(Annotated[int, Choice([1, 2, 3])]), qt.QComboBox)

    def test_guiCreations_float(self):
        self.assertIsInstance(createGui(float), qt.QDoubleSpinBox)
        self.assertIsInstance(createGui(Annotated[float, Default(3)]), qt.QDoubleSpinBox)
        self.assertIsInstance(createGui(Annotated[float, Minimum(-3)]), qt.QDoubleSpinBox)
        self.assertIsInstance(createGui(Annotated[float, Maximum(3)]), qt.QDoubleSpinBox)
        self.assertIsInstance(createGui(Annotated[float, Minimum(3), Maximum(7)]), ctk.ctkSliderWidget)
        self.assertIsInstance(createGui(Annotated[float, WithinRange(3, 7)]), ctk.ctkSliderWidget)
        self.assertIsInstance(createGui(Annotated[float, Choice([1, 2, 3])]), qt.QComboBox)

    def test_guiCreations_str(self):
        self.assertIsInstance(createGui(str), qt.QLineEdit)
        self.assertIsInstance(createGui(Annotated[str, Default("hello")]), qt.QLineEdit)
        self.assertIsInstance(createGui(Annotated[str, Choice(["a", "b", "c"]), Default("a")]), qt.QComboBox)

    def test_guiCreations_floatRange(self):
        self.assertIsInstance(createGui(FloatRange), ctk.ctkRangeWidget)
        self.assertIsInstance(createGui(Annotated[FloatRange, RangeBounds(0, 10)]), ctk.ctkRangeWidget)

    def test_guiCreations_paths(self):
        for pathtype in (pathlib.Path, pathlib.PosixPath, pathlib.WindowsPath,
                         pathlib.PurePath, pathlib.PurePosixPath, pathlib.PureWindowsPath):
            self.assertIsInstance(createGui(pathtype), ctk.ctkPathLineEdit)
            self.assertIsInstance(createGui(Annotated[pathtype, Default("path")]), ctk.ctkPathLineEdit)

    def test_guiCreations_nodes(self):
        self.assertIsInstance(createGui(vtkMRMLNode), slicer.qMRMLNodeComboBox)
        self.assertIsInstance(createGui(vtkMRMLModelNode), slicer.qMRMLNodeComboBox)
        self.assertIsInstance(createGui(Union[vtkMRMLModelNode, vtkMRMLScalarVolumeNode, None]), slicer.qMRMLNodeComboBox)
        self.assertIsInstance(createGui(Optional[vtkMRMLModelNode]), slicer.qMRMLNodeComboBox)

    def test_guiCreations_parameter_packs(self):
        @parameterPack
        class Pack:
            x: Annotated[int, Label("Xxx")]
            y: int
            z: int
        gui = createGui(Pack)

        # Overall is a qSlicerWidget
        self.assertIsInstance(gui, slicer.qSlicerWidget)

        # There exists a child with this property set to each of the parameter names. The created gui supports int
        xSpinbox = findChildWithProperty(gui, SlicerPackParameterNamePropertyName, "x")
        self.assertIsInstance(xSpinbox, qt.QSpinBox)
        ySpinbox = findChildWithProperty(gui, SlicerPackParameterNamePropertyName, "y")
        self.assertIsInstance(ySpinbox, qt.QSpinBox)
        zSpinbox = findChildWithProperty(gui, SlicerPackParameterNamePropertyName, "z")
        self.assertIsInstance(zSpinbox, qt.QSpinBox)

        # There are labels for each parameter, and the Label annotation is used if set
        labels = sorted(gui.findChildren(qt.QLabel), key=lambda label: label.text)
        self.assertEqual(len(labels), 3)
        self.assertEqual(labels[0].text, "Xxx")
        self.assertEqual(labels[1].text, "y")
        self.assertEqual(labels[2].text, "z")

        # Because this one is more complex, test that it connects to a parameter node wrapper correctly
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            pack: Annotated[Pack, Default(Pack(3, 4, 5))]

        gui.setProperty(SlicerParameterNamePropertyName, "pack")

        param = ParameterNodeWrapper(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))
        param.connectParametersToGui({"pack": gui})

        self.assertEqual(param.pack, Pack(3, 4, 5))
        self.assertEqual(xSpinbox.value, 3)
        self.assertEqual(ySpinbox.value, 4)
        self.assertEqual(zSpinbox.value, 5)

        # update gui
        xSpinbox.value = 7
        ySpinbox.value = -4
        zSpinbox.value = 0
        self.assertEqual(param.pack, Pack(7, -4, 0))
        self.assertEqual(xSpinbox.value, 7)
        self.assertEqual(ySpinbox.value, -4)
        self.assertEqual(zSpinbox.value, 0)

        # update parameter node wholesale
        param.pack = Pack(-1, 2, -3)
        self.assertEqual(param.pack, Pack(-1, 2, -3))
        self.assertEqual(xSpinbox.value, -1)
        self.assertEqual(ySpinbox.value, 2)
        self.assertEqual(zSpinbox.value, -3)

        # update parameter node piecewise
        param.pack.x = 1
        param.pack.y = -2
        param.pack.z = 3
        self.assertEqual(param.pack, Pack(1, -2, 3))
        self.assertEqual(xSpinbox.value, 1)
        self.assertEqual(ySpinbox.value, -2)
        self.assertEqual(zSpinbox.value, 3)

    def test_guiCreations_parameter_packs_setMRMLScene(self):
        @parameterPack
        class Pack:
            model: vtkMRMLModelNode
            volume: vtkMRMLScalarVolumeNode
            iterations: int

        gui = createGui(Pack)

        # Overall is a qSlicerWidget
        self.assertIsInstance(gui, slicer.qSlicerWidget)

        modelWidget = findChildWithProperty(gui, SlicerPackParameterNamePropertyName, "model")
        volumeWidget = findChildWithProperty(gui, SlicerPackParameterNamePropertyName, "volume")

        self.assertIsNone(modelWidget.mrmlScene())
        self.assertIsNone(volumeWidget.mrmlScene())

        gui.setMRMLScene(slicer.mrmlScene)
        self.assertIs(modelWidget.mrmlScene(), slicer.mrmlScene)
        self.assertIs(volumeWidget.mrmlScene(), slicer.mrmlScene)

    def test_guiCreations_parameter_node_wrapper(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            i: Annotated[int, Default(77)]
            f: Annotated[float, Label("THE FLOAT")]
            s: str
        gui = createGui(ParameterNodeWrapper)
        
        # Overall is a qSlicerWidget
        self.assertIsInstance(gui, slicer.qSlicerWidget)

        # There exists a child with this property set to each of the parameter names. The created gui supports int
        iSpinbox = findChildWithProperty(gui, SlicerParameterNamePropertyName, "i")
        self.assertIsInstance(iSpinbox, qt.QSpinBox)
        fDoubleSpinBox = findChildWithProperty(gui, SlicerParameterNamePropertyName, "f")
        self.assertIsInstance(fDoubleSpinBox, qt.QDoubleSpinBox)
        sLineEdit = findChildWithProperty(gui, SlicerParameterNamePropertyName, "s")
        self.assertIsInstance(sLineEdit, qt.QLineEdit)

        # There are labels for each parameter, and the Label annotation is used if set.
        # Sorted by text (caps matter), so not in same order as the parameters.
        labels = sorted(gui.findChildren(qt.QLabel), key=lambda label: label.text)
        self.assertEqual(len(labels), 3)
        self.assertEqual(labels[0].text, "THE FLOAT")
        self.assertEqual(labels[1].text, "i")
        self.assertEqual(labels[2].text, "s")

        param = ParameterNodeWrapper(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))

        param.connectGui(gui)

        # check the connection
        self.assertEqual(param.i, 77)
        self.assertEqual(param.f, 0.0)
        self.assertEqual(param.s, "")
        self.assertEqual(iSpinbox.value, 77)
        self.assertEqual(fDoubleSpinBox.value, 0.0)
        self.assertEqual(sLineEdit.text, "")

        # update gui
        iSpinbox.value = -4
        fDoubleSpinBox.value = 44.3
        sLineEdit.text = "To be or not to be"
        self.assertEqual(param.i, -4)
        self.assertEqual(param.f, 44.3)
        self.assertEqual(param.s, "To be or not to be")
        self.assertEqual(iSpinbox.value, -4)
        self.assertEqual(fDoubleSpinBox.value, 44.3)
        self.assertEqual(sLineEdit.text, "To be or not to be")

        # update parameter node
        param.i = 0
        param.f = -11.3
        param.s = "hello, world"
        self.assertEqual(param.i, 0)
        self.assertEqual(param.f, -11.3)
        self.assertEqual(param.s, "hello, world")
        self.assertEqual(iSpinbox.value, 0)
        self.assertEqual(fDoubleSpinBox.value, -11.3)
        self.assertEqual(sLineEdit.text, "hello, world")

    def test_guiCreations_parameter_node_wrapper_wpacks(self):
        @parameterPack
        class Inputs:
            mesh: vtkMRMLModelNode
            reduction: Annotated[float, Default(1)]
            translateX: Annotated[float, Default(7.2)]
            translateY: float

        @parameterPack
        class Outputs:
            outputMesh: vtkMRMLModelNode

        @parameterNodeWrapper
        class ParameterNodeWrapper:
            inputs: Inputs
            outputs: Outputs

        gui = createGui(ParameterNodeWrapper)
        gui.setMRMLScene(slicer.mrmlScene)

        param = ParameterNodeWrapper(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))
        param.connectGui(gui)

        reductionWidget = findChildWidgetForParameter(gui, "inputs.reduction")
        translateXWidget = findChildWidgetForParameter(gui, "inputs.translateX")
        translateYWidget = findChildWidgetForParameter(gui, "inputs.translateY")

        self.assertEqual(param.inputs.reduction, 1.0)
        self.assertEqual(param.inputs.translateX, 7.2)
        self.assertEqual(param.inputs.translateY, 0.0)
        self.assertEqual(reductionWidget.value, 1.0)
        self.assertEqual(translateXWidget.value, 7.2)
        self.assertEqual(translateYWidget.value, 0.0)

        # update gui
        reductionWidget.value = 94
        translateXWidget.value = -22
        translateYWidget.value = 1
        self.assertEqual(param.inputs.reduction, 94)
        self.assertEqual(param.inputs.translateX, -22)
        self.assertEqual(param.inputs.translateY, 1)
        self.assertEqual(reductionWidget.value, 94)
        self.assertEqual(translateXWidget.value, -22)
        self.assertEqual(translateYWidget.value, 1)

        # update param
        param.inputs.reduction = 99
        param.inputs.translateX = -33
        param.inputs.translateY = 44
        self.assertEqual(param.inputs.reduction, 99)
        self.assertEqual(param.inputs.translateX, -33)
        self.assertEqual(param.inputs.translateY, 44)
        self.assertEqual(reductionWidget.value, 99)
        self.assertEqual(translateXWidget.value, -33)
        self.assertEqual(translateYWidget.value, 44)
