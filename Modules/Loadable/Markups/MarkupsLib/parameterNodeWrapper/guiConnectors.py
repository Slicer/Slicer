from slicer import qSlicerSimpleMarkupsWidget
from slicer.parameterNodeWrapper import (
    isNodeOrUnionOfNodes,
    getNodeTypes,
    GuiConnector,
    parameterNodeGuiConnector,
)


@parameterNodeGuiConnector
class qSlicerSimpleMarkupsWidgetToNodeConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qSlicerSimpleMarkupsWidget and isNodeOrUnionOfNodes(datatype)

    @staticmethod
    def create(widget, datatype):
        if qSlicerSimpleMarkupsWidgetToNodeConnector.canRepresent(widget, datatype):
            return qSlicerSimpleMarkupsWidgetToNodeConnector(widget, datatype)
        return None

    def __init__(self, widget: qSlicerSimpleMarkupsWidget, datatype):
        super().__init__()
        self._widget: qSlicerSimpleMarkupsWidget = widget
        self._widget.markupsSelectorComboBox().nodeTypes = getNodeTypes(datatype)

    def _connect(self):
        self._widget.markupsNodeChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.markupsNodeChanged.disconnect(self.changed)

    def widget(self) -> qSlicerSimpleMarkupsWidget:
        return self._widget

    def read(self):
        return self._widget.currentNode()

    def write(self, value) -> None:
        self._widget.setCurrentNode(value)
