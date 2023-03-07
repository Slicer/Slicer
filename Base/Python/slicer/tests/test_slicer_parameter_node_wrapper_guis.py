import enum
import pathlib
import unittest
from typing import Annotated, Union

import ctk
import qt

import slicer

from slicer import (
    qMRMLSubjectHierarchyTreeView,
    vtkMRMLModelNode,
    vtkMRMLScalarVolumeNode,
)
from slicer.parameterNodeWrapper import *


# This is a copy-paste of what is in wrapper.py on purpose.
# Using "SlicerParameterName" in the .ui files will not be able to go through the variable,
# so if someone changes the value from "SlicerParameterName" it is good for a test to fail.
SlicerParameterNamePropertyName = "SlicerParameterName"


def newParameterNode():
    node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")
    return node


class ParameterNodeWrapperGuiTest(unittest.TestCase):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def impl_ButtonToBool(self, widgettype):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: bool
            bravo: Annotated[bool, Default(True)]

        widgetAlpha = widgettype()
        widgetAlpha.deleteLater()
        widgetBravo = widgettype()
        widgetBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": widgetAlpha,
            "bravo": widgetBravo,
        }
        tag = param.connectParametersToGui(mapping)
        self.assertFalse(param.alpha)
        self.assertFalse(widgetAlpha.checked)
        self.assertTrue(param.bravo)
        self.assertTrue(widgetBravo.checked)

        # Phase 1 - write True to GUI
        widgetAlpha.checked = True
        self.assertTrue(param.alpha)
        self.assertTrue(widgetAlpha.checked)
        self.assertTrue(param.bravo)
        self.assertTrue(widgetBravo.checked)

        # Phase 2 - write False to parameterNode
        param.alpha = False
        self.assertFalse(param.alpha)
        self.assertFalse(widgetAlpha.checked)
        self.assertTrue(param.bravo)
        self.assertTrue(widgetBravo.checked)

        # Phase 3 - disconnect parameterNode from GUI
        param.disconnectGui(tag)

        param.alpha = True
        self.assertTrue(param.alpha)
        self.assertFalse(widgetAlpha.checked)

        param.alpha = False
        widgetAlpha.checked = True
        self.assertFalse(param.alpha)
        self.assertTrue(widgetAlpha.checked)

        # Phase 4 - reconnect to GUI after changing parameterNode
        param.alpha = True
        widgetAlpha.checked = False
        tag = param.connectParametersToGui(mapping)
        self.assertTrue(param.alpha)
        self.assertTrue(widgetAlpha.checked)
        self.assertTrue(param.bravo)
        self.assertTrue(widgetBravo.checked)

    def test_QCheckBoxToBool(self):
        self.impl_ButtonToBool(qt.QCheckBox)

    def test_QPushButtonToBool(self):
        self.impl_ButtonToBool(qt.QPushButton)

    def impl_QSliderOrSpinBoxToIntConnector(self, unboundedWidgetType, boundedWidgetType):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: int
            bravo: Annotated[int, Default(4), Minimum(2), Maximum(5)]
            charlie: Annotated[int, WithinRange(0, 9), SingleStep(9)]

        widgetAlpha = unboundedWidgetType()
        widgetAlpha.deleteLater()
        widgetBravo = boundedWidgetType()
        widgetBravo.deleteLater()
        widgetCharlie = boundedWidgetType()
        widgetCharlie.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": widgetAlpha,
            "bravo": widgetBravo,
            "charlie": widgetCharlie,
        }
        param.connectParametersToGui(mapping)

        # alpha
        self.assertEqual(param.alpha, 0)
        self.assertEqual(widgetAlpha.value, 0)
        self.assertLessEqual(widgetAlpha.minimum, -2**31)
        self.assertGreaterEqual(widgetAlpha.maximum, 2**31 - 1)
        # bravo
        self.assertEqual(param.bravo, 4)
        self.assertEqual(widgetBravo.value, 4)
        self.assertEqual(widgetBravo.minimum, 2)
        self.assertEqual(widgetBravo.maximum, 5)
        # charlie
        self.assertEqual(param.charlie, 0)
        self.assertEqual(widgetCharlie.value, 0)
        self.assertEqual(widgetCharlie.minimum, 0)
        self.assertEqual(widgetCharlie.maximum, 9)
        self.assertEqual(widgetCharlie.singleStep, 9)

        # Phase 1 - write to GUI
        widgetAlpha.value = 292
        widgetBravo.value = 2
        widgetCharlie.value = 9
        # alpha
        self.assertEqual(param.alpha, 292)
        self.assertEqual(widgetAlpha.value, 292)
        # bravo
        self.assertEqual(param.bravo, 2)
        self.assertEqual(widgetBravo.value, 2)
        # charlie
        self.assertEqual(param.charlie, 9)
        self.assertEqual(widgetCharlie.value, 9)

        # Phase 2 - write to parameterNode
        param.alpha = -4444
        param.bravo = 5
        param.charlie = 0
        # alpha
        self.assertEqual(param.alpha, -4444)
        self.assertEqual(widgetAlpha.value, -4444)
        # bravo
        self.assertEqual(param.bravo, 5)
        self.assertEqual(widgetBravo.value, 5)
        # charlie
        self.assertEqual(param.charlie, 0)
        self.assertEqual(widgetCharlie.value, 0)

    def test_QSliderToInt(self):
        self.impl_QSliderOrSpinBoxToIntConnector(qt.QSpinBox, qt.QSlider)

        with self.assertRaises(RuntimeError):
            self.impl_QSliderOrSpinBoxToIntConnector(qt.QSlider, qt.QSlider)

    def test_QSpinBoxToInt(self):
        self.impl_QSliderOrSpinBoxToIntConnector(qt.QSpinBox, qt.QSpinBox)

    def impl_QDoubleSpinBoxCtkSliderWidgetToFloatConnector(self, unboundedWidgetType, boundedWidgetType):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: float
            bravo: Annotated[float, Default(4.2), Minimum(2.1), Maximum(5.8)]
            charlie: Annotated[float, WithinRange(0.1, 9.5), Default(0.2), Decimals(2), SingleStep(0.1)]

        widgetAlpha = unboundedWidgetType()
        widgetAlpha.deleteLater()
        widgetBravo = boundedWidgetType()
        widgetBravo.deleteLater()
        widgetCharlie = boundedWidgetType()
        widgetCharlie.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": widgetAlpha,
            "bravo": widgetBravo,
            "charlie": widgetCharlie,
        }
        param.connectParametersToGui(mapping)

        # alpha
        self.assertEqual(param.alpha, 0.0)
        self.assertEqual(widgetAlpha.value, 0.0)
        self.assertEqual(widgetAlpha.minimum, float("-inf"))
        self.assertEqual(widgetAlpha.maximum, float("inf"))
        # bravo
        self.assertEqual(param.bravo, 4.2)
        self.assertEqual(widgetBravo.value, 4.2)
        self.assertEqual(widgetBravo.minimum, 2.1)
        self.assertEqual(widgetBravo.maximum, 5.8)
        # charlie
        self.assertEqual(param.charlie, 0.2)
        self.assertEqual(widgetCharlie.value, 0.2)
        self.assertEqual(widgetCharlie.minimum, 0.1)
        self.assertEqual(widgetCharlie.maximum, 9.5)
        self.assertEqual(widgetCharlie.decimals, 2)
        self.assertEqual(widgetCharlie.singleStep, 0.1)

        # Phase 1 - write to GUI
        widgetAlpha.value = 2.3e565
        widgetBravo.value = 4.4
        widgetCharlie.value = 9.5
        # alpha
        self.assertEqual(param.alpha, 2.3e565)
        self.assertEqual(widgetAlpha.value, 2.3e565)
        # bravo
        self.assertEqual(param.bravo, 4.4)
        self.assertEqual(widgetBravo.value, 4.4)
        # charlie
        self.assertEqual(param.charlie, 9.5)
        self.assertEqual(widgetCharlie.value, 9.5)

        # Phase 2 - write to parameterNode
        param.alpha = -4444.4
        param.bravo = 2.1
        param.setValue("charlie", 0.1)
        # alpha
        self.assertEqual(param.alpha, -4444.4)
        self.assertEqual(widgetAlpha.value, -4444.4)
        # bravo
        self.assertEqual(param.bravo, 2.1)
        self.assertEqual(widgetBravo.value, 2.1)
        # charlie
        self.assertEqual(param.charlie, 0.1)
        self.assertEqual(widgetCharlie.value, 0.1)

    def test_QDoubleSpinBoxToFloat(self):
        self.impl_QDoubleSpinBoxCtkSliderWidgetToFloatConnector(qt.QDoubleSpinBox, qt.QDoubleSpinBox)

    def test_ctkSliderWidgetToFloat(self):
        self.impl_QDoubleSpinBoxCtkSliderWidgetToFloatConnector(qt.QDoubleSpinBox, ctk.ctkSliderWidget)

        with self.assertRaises(RuntimeError):
            self.impl_QDoubleSpinBoxCtkSliderWidgetToFloatConnector(ctk.ctkSliderWidget, ctk.ctkSliderWidget)

    def test_QComboBoxToStringable(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: Annotated[int, Choice([1, 2, 3]), Default(1)]
            bravo: Annotated[float, Choice([1.1, 2.2, 3.3]), Default(3.3)]
            charlie: Annotated[str, Choice(["a", "b", "c", "d"]), Default("a")]
            delta: Annotated[bool, Choice([True, False]), Default(False)]

        comboboxAlpha = qt.QComboBox()
        comboboxAlpha.deleteLater()
        comboboxBravo = qt.QComboBox()
        comboboxBravo.deleteLater()
        comboboxCharlie = qt.QComboBox()
        comboboxCharlie.deleteLater()
        comboboxDelta = qt.QComboBox()
        comboboxDelta.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": comboboxAlpha,
            "bravo": comboboxBravo,
            "charlie": comboboxCharlie,
            "delta": comboboxDelta,
        }
        param.connectParametersToGui(mapping)
        # alpha
        self.assertEqual(param.alpha, 1)
        self.assertEqual(comboboxAlpha.currentIndex, 0)
        self.assertEqual(comboboxAlpha.currentText, "1")
        self.assertEqual(comboboxAlpha.count, 3)
        self.assertEqual(comboboxAlpha.itemText(0), "1")
        self.assertEqual(comboboxAlpha.itemText(1), "2")
        self.assertEqual(comboboxAlpha.itemText(2), "3")
        # bravo
        self.assertEqual(param.bravo, 3.3)
        self.assertEqual(comboboxBravo.currentIndex, 2)
        self.assertEqual(comboboxBravo.currentText, "3.3")
        self.assertEqual(comboboxBravo.count, 3)
        self.assertEqual(comboboxBravo.itemText(0), "1.1")
        self.assertEqual(comboboxBravo.itemText(1), "2.2")
        self.assertEqual(comboboxBravo.itemText(2), "3.3")
        # charlie
        self.assertEqual(param.charlie, "a")
        self.assertEqual(comboboxCharlie.currentIndex, 0)
        self.assertEqual(comboboxCharlie.currentText, "a")
        self.assertEqual(comboboxCharlie.count, 4)
        self.assertEqual(comboboxCharlie.itemText(0), "a")
        self.assertEqual(comboboxCharlie.itemText(1), "b")
        self.assertEqual(comboboxCharlie.itemText(2), "c")
        self.assertEqual(comboboxCharlie.itemText(3), "d")
        # delta
        self.assertEqual(param.delta, False)
        self.assertEqual(comboboxDelta.currentIndex, 1)
        self.assertEqual(comboboxDelta.currentText, "False")
        self.assertEqual(comboboxDelta.count, 2)
        self.assertEqual(comboboxDelta.itemText(0), "True")
        self.assertEqual(comboboxDelta.itemText(1), "False")

        # Phase 1 - write to GUI
        comboboxAlpha.currentIndex = 2
        comboboxBravo.currentIndex = 0
        comboboxCharlie.currentIndex = 1
        comboboxDelta.currentIndex = 0
        # alpha
        self.assertEqual(param.alpha, 3)
        self.assertEqual(comboboxAlpha.currentIndex, 2)
        self.assertEqual(comboboxAlpha.currentText, "3")
        # bravo
        self.assertEqual(param.bravo, 1.1)
        self.assertEqual(comboboxBravo.currentIndex, 0)
        self.assertEqual(comboboxBravo.currentText, "1.1")
        # charlie
        self.assertEqual(param.charlie, "b")
        self.assertEqual(comboboxCharlie.currentIndex, 1)
        self.assertEqual(comboboxCharlie.currentText, "b")
        # delta
        self.assertEqual(param.delta, True)
        self.assertEqual(comboboxDelta.currentIndex, 0)
        self.assertEqual(comboboxDelta.currentText, "True")

        # Phase 2 - write to parameterNode
        param.alpha = 2
        param.bravo = 2.2
        param.charlie = "c"
        param.delta = False
        # alpha
        self.assertEqual(param.alpha, 2)
        self.assertEqual(comboboxAlpha.currentIndex, 1)
        self.assertEqual(comboboxAlpha.currentText, "2")
        # bravo
        self.assertEqual(param.bravo, 2.2)
        self.assertEqual(comboboxBravo.currentIndex, 1)
        self.assertEqual(comboboxBravo.currentText, "2.2")
        # charlie
        self.assertEqual(param.charlie, "c")
        self.assertEqual(comboboxCharlie.currentIndex, 2)
        self.assertEqual(comboboxCharlie.currentText, "c")
        # delta
        self.assertEqual(param.delta, False)
        self.assertEqual(comboboxDelta.currentIndex, 1)
        self.assertEqual(comboboxDelta.currentText, "False")

    def test_QComboBoxToEnum(self):
        class TestEnum(enum.Enum):
            A = 'A'
            B = 'B'
            C = 'C'
        
        class TestIntEnum(enum.IntEnum):
            X = 1
            Y = 3
            Z = 7

        class TestLabeledEnum(enum.Enum):
            TopLeft = 1
            BottomRight = 2

            def label(self):
                if self is TestLabeledEnum.TopLeft:
                    return "Top left"
                elif self is TestLabeledEnum.BottomRight:
                    return "Bottom right"
                raise ValueError("Unknown enum value")

        @parameterNodeWrapper
        class ParameterNodeWrapper:
            normal: TestEnum
            integer: Annotated[TestIntEnum, Default(TestIntEnum.Z)]
            labeled: TestLabeledEnum

        comboboxNormal = qt.QComboBox()
        comboboxNormal.deleteLater()
        comboboxInt = qt.QComboBox()
        comboboxInt.deleteLater()
        comboboxLabeled = qt.QComboBox()
        comboboxLabeled.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "normal": comboboxNormal,
            "integer": comboboxInt,
            "labeled": comboboxLabeled,
        }
        param.connectParametersToGui(mapping)

        # normal
        self.assertEqual(param.normal, TestEnum.A)
        self.assertEqual(comboboxNormal.currentIndex, 0)
        self.assertEqual(comboboxNormal.currentText, "A")
        self.assertEqual(comboboxNormal.count, 3)
        self.assertEqual(comboboxNormal.itemText(0), "A")
        self.assertEqual(comboboxNormal.itemText(1), "B")
        self.assertEqual(comboboxNormal.itemText(2), "C")
        # int
        self.assertEqual(param.integer, TestIntEnum.Z)
        self.assertEqual(comboboxInt.currentIndex, 2)
        self.assertEqual(comboboxInt.currentText, "Z")
        self.assertEqual(comboboxInt.count, 3)
        self.assertEqual(comboboxInt.itemText(0), "X")
        self.assertEqual(comboboxInt.itemText(1), "Y")
        self.assertEqual(comboboxInt.itemText(2), "Z")
        # labeled
        self.assertEqual(param.labeled, TestLabeledEnum.TopLeft)
        self.assertEqual(comboboxLabeled.currentIndex, 0)
        self.assertEqual(comboboxLabeled.currentText, "Top left")
        self.assertEqual(comboboxLabeled.count, 2)
        self.assertEqual(comboboxLabeled.itemText(0), "Top left")
        self.assertEqual(comboboxLabeled.itemText(1), "Bottom right")

        # Phase 1 - write to GUI
        comboboxNormal.currentIndex = 1
        comboboxInt.currentIndex = 0
        comboboxLabeled.currentIndex = 1
        # normal
        self.assertEqual(param.normal, TestEnum.B)
        self.assertEqual(comboboxNormal.currentIndex, 1)
        self.assertEqual(comboboxNormal.currentText, "B")
        # int
        self.assertEqual(param.integer, TestIntEnum.X)
        self.assertEqual(comboboxInt.currentIndex, 0)
        self.assertEqual(comboboxInt.currentText, "X")
        # labeled
        self.assertEqual(param.labeled, TestLabeledEnum.BottomRight)
        self.assertEqual(comboboxLabeled.currentIndex, 1)
        self.assertEqual(comboboxLabeled.currentText, "Bottom right")

        # Phase 2 - write to parameterNode
        param.normal = TestEnum.C
        param.integer = TestIntEnum.Y
        param.labeled = TestLabeledEnum.TopLeft
        # normal
        self.assertEqual(param.normal, TestEnum.C)
        self.assertEqual(comboboxNormal.currentIndex, 2)
        self.assertEqual(comboboxNormal.currentText, "C")
        # int
        self.assertEqual(param.integer, TestIntEnum.Y)
        self.assertEqual(comboboxInt.currentIndex, 1)
        self.assertEqual(comboboxInt.currentText, "Y")
        # labeled
        self.assertEqual(param.labeled, TestLabeledEnum.TopLeft)
        self.assertEqual(comboboxLabeled.currentIndex, 0)
        self.assertEqual(comboboxLabeled.currentText, "Top left")

    def test_QLineEditToStr(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: str
            bravo: Annotated[str, Default("someval")]

        lineEditAlpha = qt.QLineEdit()
        lineEditAlpha.deleteLater()
        lineEditBravo = qt.QLineEdit()
        lineEditBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": lineEditAlpha,
            "bravo": lineEditBravo,
        }
        param.connectParametersToGui(mapping)
        self.assertEqual(param.alpha, "")
        self.assertEqual(lineEditAlpha.text, "")
        self.assertEqual(param.bravo, "someval")
        self.assertEqual(lineEditBravo.text, "someval")

        # Phase 1 - write to GUI
        lineEditAlpha.text = "hello, world"
        self.assertEqual(param.alpha, "hello, world")
        self.assertEqual(lineEditAlpha.text, "hello, world")
        self.assertEqual(param.bravo, "someval")
        self.assertEqual(lineEditBravo.text, "someval")

        # Phase 2 - write to parameterNode
        param.alpha = "goodbye"
        self.assertEqual(param.alpha, "goodbye")
        self.assertEqual(lineEditAlpha.text, "goodbye")
        self.assertEqual(param.bravo, "someval")
        self.assertEqual(lineEditBravo.text, "someval")

    def test_QTextEditPlainTextToStr(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: str
            bravo: Annotated[str, Default("someval\nsomeval2")]

        lineEditAlpha = qt.QTextEdit()
        lineEditAlpha.deleteLater()
        lineEditBravo = qt.QTextEdit()
        lineEditBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": lineEditAlpha,
            "bravo": lineEditBravo,
        }
        param.connectParametersToGui(mapping)
        self.assertEqual(param.alpha, "")
        self.assertEqual(lineEditAlpha.toPlainText(), "")
        self.assertEqual(param.bravo, "someval\nsomeval2")
        self.assertEqual(lineEditBravo.toPlainText(), "someval\nsomeval2")

        # Phase 1 - write to GUI
        lineEditAlpha.setPlainText("hello, world")
        self.assertEqual(param.alpha, "hello, world")
        self.assertEqual(lineEditAlpha.toPlainText(), "hello, world")
        self.assertEqual(param.bravo, "someval\nsomeval2")
        self.assertEqual(lineEditBravo.toPlainText(), "someval\nsomeval2")

        lineEditAlpha.setHtml("hello,<br/>world")
        self.assertEqual(param.alpha, "hello,\nworld")
        self.assertEqual(lineEditAlpha.toPlainText(), "hello,\nworld")
        self.assertEqual(param.bravo, "someval\nsomeval2")
        self.assertEqual(lineEditBravo.toPlainText(), "someval\nsomeval2")

        # Phase 2 - write to parameterNode
        param.alpha = "goodbye"
        self.assertEqual(param.alpha, "goodbye")
        self.assertEqual(lineEditAlpha.toPlainText(), "goodbye")
        self.assertEqual(param.bravo, "someval\nsomeval2")
        self.assertEqual(lineEditBravo.toPlainText(), "someval\nsomeval2")

    def test_ctkRangeWidgetToRange(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: Annotated[FloatRange, RangeBounds(-10, 10)]
            bravo: Annotated[FloatRange, RangeBounds(0, 20), Default(FloatRange(5, 10.5))]

        rangeWidgetAlpha = ctk.ctkRangeWidget()
        rangeWidgetAlpha.deleteLater()
        rangeWidgetBravo = ctk.ctkRangeWidget()
        rangeWidgetBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": rangeWidgetAlpha,
            "bravo": rangeWidgetBravo,
        }
        param.connectParametersToGui(mapping)
        self.assertEqual(param.alpha, FloatRange(0, 0))
        self.assertEqual(param.bravo, FloatRange(5, 10.5))

        # Phase 1 - write to GUI
        rangeWidgetAlpha.minimumValue = -1
        self.assertEqual(rangeWidgetAlpha.minimumValue, -1)
        self.assertEqual(param.alpha, FloatRange(-1, 0))
        rangeWidgetBravo.maximumValue = 19
        self.assertEqual(rangeWidgetBravo.maximumValue, 19)
        self.assertEqual(param.bravo, FloatRange(5, 19))

        # Phase 2 - write to parameterNode
        param.alpha = FloatRange(-9, 9)
        self.assertEqual(param.alpha, FloatRange(-9, 9))
        self.assertEqual(rangeWidgetAlpha.minimumValue, -9)
        self.assertEqual(rangeWidgetAlpha.maximumValue, 9)
        param.bravo.maximum = 6
        self.assertEqual(param.bravo, FloatRange(5, 6))
        self.assertEqual(rangeWidgetBravo.minimumValue, 5)
        self.assertEqual(rangeWidgetBravo.maximumValue, 6)

    def test_ctkPathLineEditToPath(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: pathlib.Path
            bravo: Annotated[pathlib.Path, Default(pathlib.Path("some/path"))]

        pathLineEditAlpha = ctk.ctkPathLineEdit()
        pathLineEditAlpha.deleteLater()
        pathLineEditBravo = ctk.ctkPathLineEdit()
        pathLineEditBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": pathLineEditAlpha,
            "bravo": pathLineEditBravo,
        }
        param.connectParametersToGui(mapping)
        self.assertEqual(param.alpha, pathlib.Path())
        self.assertEqual(pathlib.Path(pathLineEditAlpha.currentPath), pathlib.Path())
        self.assertEqual(param.bravo, pathlib.Path("some/path"))
        self.assertEqual(pathlib.Path(pathLineEditBravo.currentPath), pathlib.Path("some/path"))

        # Phase 1 - write to GUI
        pathLineEditAlpha.currentPath = "pathy/path"
        self.assertEqual(param.alpha, pathlib.Path("pathy/path"))
        self.assertEqual(pathlib.Path(pathLineEditAlpha.currentPath), pathlib.Path("pathy/path"))
        self.assertEqual(param.bravo, pathlib.Path("some/path"))
        self.assertEqual(pathlib.Path(pathLineEditBravo.currentPath), pathlib.Path("some/path"))

        # Phase 2 - write to parameterNode
        param.bravo = pathlib.Path("magnificent/path/bravo")
        self.assertEqual(param.alpha, pathlib.Path("pathy/path"))
        self.assertEqual(pathlib.Path(pathLineEditAlpha.currentPath), pathlib.Path("pathy/path"))
        self.assertEqual(param.bravo, pathlib.Path("magnificent/path/bravo"))
        self.assertEqual(pathlib.Path(pathLineEditBravo.currentPath), pathlib.Path("magnificent/path/bravo"))

    def test_ctkDirectoryButtonToPath(self):
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: pathlib.Path
            bravo: Annotated[pathlib.Path, Default(pathlib.Path("some/path"))]

        directoryButtonAlpha = ctk.ctkDirectoryButton()
        directoryButtonAlpha.deleteLater()
        directoryButtonBravo = ctk.ctkDirectoryButton()
        directoryButtonBravo.deleteLater()
        param = ParameterNodeWrapper(newParameterNode())

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": directoryButtonAlpha,
            "bravo": directoryButtonBravo,
        }
        param.connectParametersToGui(mapping)
        self.assertEqual(param.alpha, pathlib.Path())
        self.assertEqual(pathlib.Path(directoryButtonAlpha.directory), pathlib.Path())
        self.assertEqual(param.bravo, pathlib.Path("some/path"))
        self.assertEqual(pathlib.Path(directoryButtonBravo.directory), pathlib.Path("some/path"))

        # Phase 1 - write to GUI
        directoryButtonAlpha.directory = "pathy/path"
        self.assertEqual(param.alpha, pathlib.Path("pathy/path"))
        self.assertEqual(pathlib.Path(directoryButtonAlpha.directory), pathlib.Path("pathy/path"))
        self.assertEqual(param.bravo, pathlib.Path("some/path"))
        self.assertEqual(pathlib.Path(directoryButtonBravo.directory), pathlib.Path("some/path"))

        # Phase 2 - write to parameterNode
        param.bravo = pathlib.Path("magnificent/path/bravo")
        self.assertEqual(param.alpha, pathlib.Path("pathy/path"))
        self.assertEqual(pathlib.Path(directoryButtonAlpha.directory), pathlib.Path("pathy/path"))
        self.assertEqual(param.bravo, pathlib.Path("magnificent/path/bravo"))
        self.assertEqual(pathlib.Path(directoryButtonBravo.directory), pathlib.Path("magnificent/path/bravo"))

    def impl_qMRMLToNodeConnector(self, widgettype, currentNodeFunc, clearFunc):
        """
        The tests for the qMRMLNodeComboBox and the qMRMLSubjectHierarchyTreeView are nearly identical, but their
        interfaces are just different enough (especially in Python) to make it difficult. In the places where
        the interfaces differ, a function was added to abstract that away.
        """
        @parameterNodeWrapper
        class ParameterNodeWrapper:
            alpha: vtkMRMLModelNode
            bravo: Union[vtkMRMLModelNode, vtkMRMLScalarVolumeNode, None]

        param = ParameterNodeWrapper(newParameterNode())

        widgetAlpha = widgettype()
        widgetAlpha.setMRMLScene(slicer.mrmlScene)
        widgetAlpha.deleteLater()
        widgetBravo = widgettype()
        widgetBravo.setMRMLScene(slicer.mrmlScene)
        widgetBravo.deleteLater()

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "alpha": widgetAlpha,
            "bravo": widgetBravo,
        }
        param.connectParametersToGui(mapping)

        # alpha
        self.assertIsNone(param.alpha)
        self.assertIsNone(currentNodeFunc(widgetAlpha))
        self.assertEqual(widgetAlpha.nodeTypes, ('vtkMRMLModelNode', ))
        # bravo
        self.assertIsNone(param.bravo)
        self.assertIsNone(currentNodeFunc(widgetBravo))
        #    order is unimportant here
        self.assertEqual(sorted(widgetBravo.nodeTypes), sorted(['vtkMRMLModelNode', 'vtkMRMLScalarVolumeNode']))

        model1 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode', "model1")
        model2 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode', "model2")
        volume1 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScalarVolumeNode', "volume1")
        volume2 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScalarVolumeNode', "volume2")

        # should not auto add
        self.assertIsNone(param.alpha)
        self.assertIsNone(currentNodeFunc(widgetAlpha))
        self.assertIsNone(param.bravo)
        self.assertIsNone(currentNodeFunc(widgetBravo))

        # Phase 1 - write to GUI
        widgetAlpha.setCurrentNode(model2)
        self.assertIs(param.alpha, model2)
        self.assertIs(currentNodeFunc(widgetAlpha), model2)

        clearFunc(widgetAlpha)
        self.assertIsNone(param.alpha)
        self.assertIsNone(currentNodeFunc(widgetAlpha))

        widgetBravo.setCurrentNode(model1)
        self.assertIs(param.bravo, model1)
        self.assertIs(currentNodeFunc(widgetBravo), model1)

        widgetBravo.setCurrentNode(volume2)
        self.assertIs(currentNodeFunc(widgetBravo), volume2)
        self.assertIs(param.bravo, volume2)

        clearFunc(widgetBravo)
        self.assertIs(param.bravo, None)
        self.assertIs(currentNodeFunc(widgetBravo), None)

        # Phase 2 - write to parameterNode
        param.alpha = model1
        self.assertIs(param.alpha, model1)
        self.assertIs(currentNodeFunc(widgetAlpha), model1)

        param.alpha = None
        self.assertIsNone(param.alpha)
        self.assertIsNone(currentNodeFunc(widgetAlpha))

        param.bravo = volume1
        self.assertIs(param.bravo, volume1)
        self.assertIs(currentNodeFunc(widgetBravo), volume1)

        param.bravo = model2
        self.assertIs(param.bravo, model2)
        self.assertIs(currentNodeFunc(widgetBravo), model2)

        param.bravo = None
        self.assertIs(param.bravo, None)
        self.assertIs(currentNodeFunc(widgetBravo), None)

    def test_qMRMLNodeComboBoxToNode(self):
        self.impl_qMRMLToNodeConnector(slicer.qMRMLNodeComboBox, lambda box: box.currentNode(), lambda box: box.setCurrentNode(None))

    def test_qMRMLSubjectHierarchyTreeViewToNode(self):
        def getNodeFromSubjectTree(tree):
            itemId = tree.currentItem()
            shNode = tree.subjectHierarchyNode()
            if itemId == shNode.GetInvalidItemID():
                return None
            else:
                return shNode.GetItemDataNode(itemId)

        self.impl_qMRMLToNodeConnector(qMRMLSubjectHierarchyTreeView, getNodeFromSubjectTree, lambda tree: tree.clearSelection())

    def test_parameterPacks_through_dotted_name(self):
        @parameterPack
        class Pack:
            iterations: Annotated[int, Minimum(0)]
            doPreprocessing: bool
            style: Annotated[str, Choice(["forward", "backward", "sideways"]), Default("forward")]

        @parameterNodeWrapper
        class ParameterNodeWrapper:
            pack: Pack
            otherNumber: float

        param = ParameterNodeWrapper(newParameterNode())

        widgetIterations = qt.QSpinBox()
        widgetIterations.deleteLater()
        widgetDoPreprocessing = qt.QCheckBox()
        widgetDoPreprocessing.deleteLater()
        widgetStyle = qt.QComboBox()
        widgetStyle.deleteLater()
        widgetOtherNumber = qt.QDoubleSpinBox()
        widgetOtherNumber.deleteLater()

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "pack.iterations": widgetIterations,
            "pack.doPreprocessing": widgetDoPreprocessing,
            "pack.style": widgetStyle,
            "otherNumber": widgetOtherNumber,
        }
        param.connectParametersToGui(mapping)

        self.assertEqual(param.pack.iterations, 0)
        self.assertEqual(widgetIterations.value, 0)
        self.assertEqual(widgetIterations.minimum, 0)
        self.assertEqual(widgetIterations.maximum, 2**31 - 1)
        self.assertFalse(param.pack.doPreprocessing)
        self.assertFalse(widgetDoPreprocessing.checked)
        self.assertEqual(param.pack.style, "forward")
        self.assertEqual(widgetStyle.currentIndex, 0)
        self.assertEqual(widgetStyle.currentText, "forward")
        self.assertEqual(widgetStyle.count, 3)
        self.assertEqual(widgetStyle.itemText(0), "forward")
        self.assertEqual(widgetStyle.itemText(1), "backward")
        self.assertEqual(widgetStyle.itemText(2), "sideways")
        self.assertEqual(param.otherNumber, 0.0)
        self.assertEqual(widgetOtherNumber.value, 0.0)

        # Phase 1 - write to GUI
        widgetIterations.value = 44
        self.assertEqual(param.pack.iterations, 44)
        self.assertEqual(widgetIterations.value, 44)
        widgetDoPreprocessing.checked = True
        self.assertTrue(param.pack.doPreprocessing)
        self.assertTrue(widgetDoPreprocessing.checked)
        widgetStyle.currentIndex = 2
        self.assertEqual(param.pack.style, "sideways")
        self.assertEqual(widgetStyle.currentIndex, 2)
        param.otherNumber = -77.6
        self.assertEqual(param.otherNumber, -77.6)
        self.assertEqual(widgetOtherNumber.value, -77.6)

        # Phase 2 - write to parameterNode
        param.pack.iterations = 55
        self.assertEqual(param.pack.iterations, 55)
        self.assertEqual(widgetIterations.value, 55)
        param.pack.doPreprocessing = False
        self.assertFalse(param.pack.doPreprocessing)
        self.assertFalse(widgetDoPreprocessing.checked)
        param.pack.style = "backward"
        self.assertEqual(param.pack.style, "backward")
        self.assertEqual(widgetStyle.currentIndex, 1)
        param.otherNumber = 3333
        self.assertEqual(param.otherNumber, 3333)
        self.assertEqual(widgetOtherNumber.value, 3333)

        param.pack = Pack(iterations=1, doPreprocessing=True, style="forward")
        self.assertEqual(param.pack.iterations, 1)
        self.assertEqual(widgetIterations.value, 1)
        self.assertTrue(param.pack.doPreprocessing)
        self.assertTrue(widgetDoPreprocessing.checked)
        self.assertEqual(param.pack.style, "forward")
        self.assertEqual(widgetStyle.currentIndex, 0)
        self.assertEqual(param.otherNumber, 3333)
        self.assertEqual(widgetOtherNumber.value, 3333)

    # Testing multiple ways of connecting parameter packs, so making the parameterNodeWrapper
    # and running the sets/gets were factored out, so the only thing the test functions do
    # differently is how they connect the guis to the wrapper.
    def impl_parameterPacks_make_parameter_node_wrapper(self):

        @parameterPack
        class SubPack:
            iterations: Annotated[int, WithinRange(1, 33), Default(15)]
            description: str

        @parameterPack
        class Pack:
            sub1: Annotated[SubPack, Default(SubPack(22, "submarine"))]
            sub2: SubPack
            useFirst: Annotated[bool, Default(True)]

        @parameterNodeWrapper
        class ParameterNodeWrapper:
            pack: Pack
            style: Annotated[str, Choice(["forward", "backward", "sideways"]), Default("forward")]

        return ParameterNodeWrapper

    def impl_parameterPacks_test_connected_parameter_node_wrapper(self, param, ui):
        # make sure the widgets got updated with the initial values
        self.assertEqual(param.pack.sub1.iterations, 22)
        self.assertEqual(param.pack.sub1.description, "submarine")
        self.assertEqual(param.pack.sub2.iterations, 15)
        self.assertEqual(param.pack.sub2.description, "")
        self.assertTrue(param.pack.useFirst)
        self.assertEqual(param.style, "forward")

        self.assertEqual(ui.sub1Iterations.value, 22)
        self.assertEqual(ui.sub1Description.text, "submarine")
        self.assertEqual(ui.sub2Iterations.value, 15)
        self.assertEqual(ui.sub2Description.text, "")
        self.assertTrue(ui.useFirst.checked)
        self.assertEqual(ui.style.currentIndex, 0)
        self.assertEqual(ui.style.currentText, "forward")
        self.assertEqual(ui.style.count, 3)
        self.assertEqual(ui.style.itemText(0), "forward")
        self.assertEqual(ui.style.itemText(1), "backward")
        self.assertEqual(ui.style.itemText(2), "sideways")

        # Phase 1 - write to GUI
        ui.sub1Iterations.value = 33
        self.assertEqual(param.pack.sub1.iterations, 33)
        self.assertEqual(ui.sub1Iterations.value, 33)
        self.assertEqual(param.pack.sub2.iterations, 15)
        self.assertEqual(ui.sub2Iterations.value, 15)
        ui.sub2Description.text = "hello, world"
        self.assertEqual(param.pack.sub1.description, "submarine")
        self.assertEqual(ui.sub1Description.text, "submarine")
        self.assertEqual(param.pack.sub2.description, "hello, world")
        self.assertEqual(ui.sub2Description.text, "hello, world")

        # Phase 2 - write to parameterNode
        param.pack.sub2.iterations = 4
        self.assertEqual(param.pack.sub1.iterations, 33)
        self.assertEqual(ui.sub1Iterations.value, 33)
        self.assertEqual(param.pack.sub2.iterations, 4)
        self.assertEqual(ui.sub2Iterations.value, 4)
        param.pack.sub1.setValue("description", "airplane")
        self.assertEqual(param.pack.sub1.description, "airplane")
        self.assertEqual(ui.sub1Description.text, "airplane")
        self.assertEqual(param.pack.sub2.description, "hello, world")
        self.assertEqual(ui.sub2Description.text, "hello, world")

    def test_nested_parameterPacks_through_dotted_name(self):
        ParameterNodeWrapper = self.impl_parameterPacks_make_parameter_node_wrapper()

        param = ParameterNodeWrapper(newParameterNode())

        ui = type('', (), {})()  # empty object
        ui.sub1Iterations = qt.QSpinBox()
        ui.sub1Iterations.deleteLater()
        ui.sub1Description = qt.QLineEdit()
        ui.sub1Description.deleteLater()
        ui.sub2Iterations = qt.QSpinBox()
        ui.sub2Iterations.deleteLater()
        ui.sub2Description = qt.QLineEdit()
        ui.sub2Description.deleteLater()
        ui.useFirst = qt.QPushButton()
        ui.useFirst.checkable = True
        ui.useFirst.deleteLater()
        ui.style = qt.QComboBox()
        ui.style.deleteLater()

        # Phase 0 - connect parameterNode to GUI
        mapping = {
            "pack.sub1.iterations": ui.sub1Iterations,
            "pack.sub1.description": ui.sub1Description,
            "pack.sub2.iterations": ui.sub2Iterations,
            "pack.sub2.description": ui.sub2Description,
            "pack.useFirst": ui.useFirst,
            "style": ui.style,
        }
        param.connectParametersToGui(mapping)
        self.impl_parameterPacks_test_connected_parameter_node_wrapper(param, ui)

    def test_connect_via_SlicerParameterName(self):
        ParameterNodeWrapper = self.impl_parameterPacks_make_parameter_node_wrapper()

        # Phase 0 - connect parameterNode to GUI

        # this is very similar to what slicer.util.childWidgetVariables does
        ui = type('', (), {})()  # empty object

        def addWidget(widget, paramName):
            if paramName:
                widget.setProperty(SlicerParameterNamePropertyName, paramName)
            widget.deleteLater()
            return widget

        ui.sub1Iterations = addWidget(qt.QSpinBox(), "pack.sub1.iterations")
        ui.sub1Description = addWidget(qt.QLineEdit(), "pack.sub1.description")
        ui.sub2Iterations = addWidget(qt.QSpinBox(), "pack.sub2.iterations")
        ui.sub2Description = addWidget(qt.QLineEdit(), "pack.sub2.description")
        ui.useFirst = addWidget(qt.QCheckBox(), "pack.useFirst")
        ui.style = addWidget(qt.QComboBox(), "style")
        ui.unusedWidget1 = addWidget(qt.QSpinBox(), None)
        ui.unusedWidget2 = addWidget(qt.QLineEdit(), None)

        param = ParameterNodeWrapper(newParameterNode())
        param.connectGui(ui)
        ui.unusedWidget1.value = 10
        ui.unusedWidget2.text = "I am unused"
        self.impl_parameterPacks_test_connected_parameter_node_wrapper(param, ui)

    def test_parameterPacks_through_widget_children(self):
        ParameterNodeWrapper = self.impl_parameterPacks_make_parameter_node_wrapper()

        param = ParameterNodeWrapper(newParameterNode())

        def addWidget(widget, /, packParam):
            widget.setProperty(SlicerPackParameterNamePropertyName, packParam)
            return widget

        ui = type('', (), {})()  # empty object
        ui.style = qt.QComboBox()
        ui.style.deleteLater()

        # setup a widget with children that matches the structure of the pack
        ui.pack = qt.QWidget()
        ui.pack.deleteLater()
        ui.sub1 = addWidget(qt.QWidget(ui.pack), packParam="sub1")
        ui.sub1Iterations = addWidget(qt.QSpinBox(ui.sub1), packParam="iterations")
        ui.sub1Description = addWidget(qt.QLineEdit(ui.sub1), packParam="description")
        ui.sub2 = addWidget(qt.QWidget(ui.pack), packParam="sub2")
        ui.sub2Iterations = addWidget(qt.QSpinBox(ui.sub2), packParam="iterations")
        ui.sub2Description = addWidget(qt.QLineEdit(ui.sub2), packParam="description")
        ui.useFirst = addWidget(qt.QCheckBox(ui.pack), packParam="useFirst")
        # extra widgets that don't have a SlicerPackParameterNamePropertyName don't affect anything
        qt.QLabel(ui.pack)
        qt.QLabel(ui.sub1)
        qt.QLabel(ui.sub2Iterations)

        mapping = {
            "pack": ui.pack,
            "style": ui.style,
        }
        param.connectParametersToGui(mapping)

        self.impl_parameterPacks_test_connected_parameter_node_wrapper(param, ui)
