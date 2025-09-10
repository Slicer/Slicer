import sys
from typing import Any
from unittest.mock import MagicMock

from LayerDMManagerLib import vtkMRMLLayerDMScriptedPipeline
from slicer import (
    vtkMRMLAbstractViewNode,
    vtkMRMLInteractionEventData,
    vtkMRMLLayerDMPipelineManager,
    vtkMRMLNode,
    vtkMRMLScene,
)
from vtk import vtkCamera, vtkRenderer, vtkObject


class MockPipeline(vtkMRMLLayerDMScriptedPipeline):
    """
    Mock scripted layer displayable manager pipeline.
    Provides mocks for all overridden methods and default behavior following the base scripted pipeline.
    """

    def __init__(
        self,
        layer=0,
        widgetState=0,
        canProcess=False,
        processDistance=sys.float_info.max,
        didProcess=False,
        mouseCursor=0,
    ):
        super().__init__()
        self.mockCanProcess = MagicMock(return_value=(canProcess, processDistance))
        self.mockGetCamera = MagicMock(return_value=None)
        self.mockGetMouse = MagicMock(return_value=mouseCursor)
        self.mockGetRenderLayer = MagicMock(return_value=layer)
        self.mockGetWidgetState = MagicMock(return_value=widgetState)
        self.mockLoseFocus = MagicMock()
        self.mockOnDefaultCameraModified = MagicMock()
        self.mockOnRendererAdded = MagicMock()
        self.mockOnRendererRemoved = MagicMock()
        self.mockOnUpdate = MagicMock()
        self.mockProcess = MagicMock(return_value=didProcess)
        self.mockSetDisplayNode = MagicMock()
        self.mockSetViewNode = MagicMock()
        self.mockSetScene = MagicMock()
        self.mockSetPipelineManager = MagicMock()
        self.mockUpdatePipeline = MagicMock()

    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        return self.mockCanProcess(eventData)

    def GetCamera(self) -> vtkCamera | None:
        return self.mockGetCamera()

    def GetMouseCursor(self) -> int:
        return self.mockGetMouse()

    def GetRenderLayer(self) -> int:
        return self.mockGetRenderLayer()

    def GetWidgetState(self) -> int:
        return self.mockGetWidgetState()

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData) -> None:
        self.mockLoseFocus(eventData)

    def OnDefaultCameraModified(self, camera: vtkCamera) -> None:
        self.mockOnDefaultCameraModified(camera)

    def OnRendererAdded(self, renderer: vtkRenderer) -> None:
        self.mockOnRendererAdded(renderer)

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        self.mockOnRendererRemoved(renderer)

    def OnUpdate(self, obj: vtkObject, eventId: int, callData: Any) -> None:
        self.mockOnUpdate(obj, eventId, callData)

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        return self.mockProcess(eventData)

    def SetDisplayNode(self, displayNode: vtkMRMLNode) -> None:
        self.mockSetDisplayNode(displayNode)
        super().SetDisplayNode(displayNode)

    def SetViewNode(self, viewNode: vtkMRMLAbstractViewNode) -> None:
        self.mockSetViewNode(viewNode)
        super().SetViewNode(viewNode)

    def SetScene(self, scene: vtkMRMLScene) -> None:
        self.mockSetScene(scene)
        super().SetScene(scene)

    def SetPipelineManager(self, pipelineManager: vtkMRMLLayerDMPipelineManager) -> None:
        self.mockSetPipelineManager(pipelineManager)
        super().SetPipelineManager(pipelineManager)

    def UpdatePipeline(self) -> None:
        self.mockUpdatePipeline()
