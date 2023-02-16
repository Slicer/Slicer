# Custom Widgets for Parameter Packs

Sometimes a parameter pack is used multiple times in a parameter node wrapper. Take the following example:

```py
from slicer.parameterNodeWrapper import *

@parameterPack
class Point:
    x: float
    y: float

@parameterPack
class BoundingBox:
    # can nest parameterPacks
    topLeft: Point
    bottomRight: Point

@parameterNodeWrapper
class ParameterNodeType:
    box1: BoundingBox
    box2: BoundingBox
```

in this example it could be useful to have a `PointWidget` that directly represented the `Point` class. The widget could be reused multiple times (even across multiple files) to ensure a consistent experience for inputting points.

Here is an example on how that could be done. The `PointWidget` is being created in code here, but this could also apply to a custom widget created in a `.ui` file.

```py
class PointWidget(qt.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.setLayout(qt.QHBoxLayout(self))

        self.xWidget = qt.QDoubleSpinBox()
        # set the parameterPack parameter this widget corresponds to
        self.xWidget.setProperty("SlicerPackParameterName", "x")
        self.yWidget = qt.QDoubleSpinBox()
        # set the parameterPack parameter this widget corresponds to
        self.yWidget.setProperty("SlicerPackParameterName", "y")

        self.layout().addWidget(self.xWidget)
        self.layout().addWidget(self.yWidget)


topLeft1Widget = PointWidget()
topLeft2Widget = PointWidget()

param = ParameterNodeType(slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode"))

param.connectParametersToGui({
    "box1.topLeft": topLeft1Widget,
    "box2.topLeft": topLeft2Widget,
})
```

Setting the `"SlicerPackParameterName"` property on a child widget of the custom widget is enough for the GUI connection infrastructure to make the binding.

The infrastructure uses parent-child relationships to work through nested values.

Here is an example of a `BoundingBoxWidget` that is completely separate from `PointWidget` and uses parent-child relationships to match the nesting structure of the parameter packs.

```py
class BoundingBoxWidget(qt.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.setLayout(qt.QVBoxLayout(self))

        # Making a widget for the topLeft point. This particular QWidget doesn't
        # do anything special for the UI, but it parents the x and y widgets for the
        # top left points
        topLeftWidget = qt.QWidget()
        self.layout.addWidget(topLeftWidget)
        topLeftWidget.setProperty("SlicerPackParameterName", "topLeft")
        topLeftWidget.setLayout(qt.QHBoxLayout())

        topLeftXWidget = qt.QDoubleSpinBox()
        topLeftXWidget.setProperty("SlicerPackParameterName", "x")
        topLeftYWidget = qt.QDoubleSpinBox()
        topLeftYWidget.setProperty("SlicerPackParameterName", "y")

        topLeftWidget.layout().addWidget(topLeftXWidget)
        topLeftWidget.layout().addWidget(topLeftYWidget)

        # Making a widget for the bottomRight point. This particular QWidget doesn't
        # do anything special for the UI, but it parents the x and y widgets for the
        # top left points
        bottomRightWidget = qt.QWidget()
        self.layout.addWidget(bottomRightWidget)
        bottomRightWidget.setProperty("SlicerPackParameterName", "bottomRight")
        bottomRightWidget.setLayout(qt.QHBoxLayout())

        bottomRightXWidget = qt.QDoubleSpinBox()
        bottomRightXWidget.setProperty("SlicerPackParameterName", "x")
        bottomRightYWidget = qt.QDoubleSpinBox()
        bottomRightYWidget.setProperty("SlicerPackParameterName", "y")

        bottomRightWidget.layout().addWidget(bottomRightXWidget)
        bottomRightWidget.layout().addWidget(bottomRightYWidget)
```
