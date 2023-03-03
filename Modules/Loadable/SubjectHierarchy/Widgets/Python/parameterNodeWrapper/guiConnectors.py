from slicer import qMRMLSubjectHierarchyTreeView
from slicer.parameterNodeWrapper import (
    isNodeOrUnionOfNodes,
    getNodeTypes,
    GuiConnector,
    parameterNodeGuiConnector,
)


@parameterNodeGuiConnector
class qMRMLSubjectHierarchyTreeViewToNodeConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qMRMLSubjectHierarchyTreeView and isNodeOrUnionOfNodes(datatype)

    @staticmethod
    def create(widget, datatype):
        if qMRMLSubjectHierarchyTreeViewToNodeConnector.canRepresent(widget, datatype):
            return qMRMLSubjectHierarchyTreeViewToNodeConnector(widget, datatype)
        return None

    def __init__(self, widget: qMRMLSubjectHierarchyTreeView, datatype):
        super().__init__()
        self._widget: qMRMLSubjectHierarchyTreeView = widget
        self._widget.nodeTypes = getNodeTypes(datatype)

    def _connect(self):
        self._widget.currentItemsChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.currentItemsChanged.disconnect(self.changed)

    def widget(self) -> qMRMLSubjectHierarchyTreeView:
        return self._widget

    def read(self):
        itemId = self._widget.currentItem()
        shNode = self._widget.subjectHierarchyNode()
        if itemId == shNode.GetInvalidItemID():
            return None
        else:
            return shNode.GetItemDataNode(itemId)

    def write(self, value) -> None:
        if value is not None:
            self._widget.setCurrentNode(value)
        else:
            self._widget.clearSelection()
