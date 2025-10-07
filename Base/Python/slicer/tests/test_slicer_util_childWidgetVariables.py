import unittest
import slicer
import qt

class SlicerUtilChildWidgetVariablesTests(unittest.TestCase):

    def test_noEmptyAttribute(self):
        widget = qt.QWidget()
        childWidgetWithName = qt.QWidget(widget)
        childWidgetWithName.setObjectName("childWidgetWithName")
        childWidgetWithoutName = qt.QWidget(widget)
        ui = slicer.util.childWidgetVariables(widget)
        print(ui.__dir__())
        self.assertTrue(hasattr(ui, "childWidgetWithName"))
        self.assertFalse(hasattr(ui, ""))
