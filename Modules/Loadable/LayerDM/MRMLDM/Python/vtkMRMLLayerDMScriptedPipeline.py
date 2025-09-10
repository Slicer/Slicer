from typing import Any

from slicer import (
    vtkMRMLAbstractViewNode,
    vtkMRMLAbstractWidget,
    vtkMRMLInteractionEventData,
    vtkMRMLLayerDMPipelineI,
    vtkMRMLLayerDMPipelineManager,
    vtkMRMLLayerDMScriptedPipelineBridge,
    vtkMRMLNode,
    vtkMRMLScene,
)
from vtk import vtkCamera, vtkRenderer, vtkObject


class vtkMRMLLayerDMScriptedPipeline(vtkMRMLLayerDMScriptedPipelineBridge):
    """
    Python base class for all Layer Displayable Manager pipelines.
    """

    def __init__(self):
        self.SetPythonObject(self)

    @property
    def viewNode(self) -> vtkMRMLAbstractViewNode:
        """
        Property returning the current view node on which the pipeline is attached.
        """
        return self.GetViewNode()

    @property
    def displayNode(self) -> vtkMRMLNode:
        """
        Property returning the current display node for which the pipeline was created.
        """
        return self.GetDisplayNode()

    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        """
        Should return true + distance2 to interaction if the pipeline can process the input event data.
        :param eventData: The MRML event needing to be processed
        :return: (bool, distance2) default = False, float_max
        """
        import sys

        return False, sys.float_info.max

    def GetCamera(self) -> vtkCamera | None:
        """
        Custom pipeline camera.
        If the returned value is not None, then the pipeline (or dedicated logic) is expected to handle its own camera.
        Otherwise, the pipeline will be moved in a renderer with a default camera synchronized on its view default camera.

        :return: None by default
        """
        return None

    def GetMouseCursor(self) -> int:
        """
        Custom mouse cursor from VTK mouse cursor enum.
        This value is only used if the pipeline actually processes an event and is ignore otherwise.
        :return: 0 by default.
        """
        return 0

    def GetRenderOrder(self) -> int:
        """
        Arbitrary render order number where the pipeline wants to be displayed.
        Return 0 to be at the default order (main 3D Slicer pipelines)
        Return larger values to be rendered on top of pipelines with lower render orders.

        See also: vtkMRMLLayerDMLayerManager
        :return: default = 0
        """
        return 0

    def GetWidgetState(self) -> int:
        """
        Current widget state of the pipeline.
        :return: default = WidgetStateIdle
        """
        return vtkMRMLAbstractWidget.WidgetStateIdle

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData | None) -> None:
        """
        Triggered when the pipeline had focus (processed an interaction) and loses the focus (other pipeline
        handled the new interaction or window leave event).
        default behavior: does nothing.
        :param eventData: Optional event data which triggerred the lose focus
        """
        pass

    def OnDefaultCameraModified(self, camera: vtkCamera) -> None:
        """
        Triggered when the default camera is modified.
        default behavior: does nothing.
        :param camera: Instance of the default camera
        """
        pass

    def OnRendererAdded(self, renderer: vtkRenderer | None) -> None:
        """
        Triggered when the pipeline is displayed on a new renderer.
        default behavior: does nothing.
        See also: self.GetRenderer()
        See also: self.ResetDisplay()
        See also: self.RequestRender()

        :param renderer: Optional instance or renderer on which the pipeline is added
        """
        pass

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is removed from its previous renderer.
        default behavior: does nothing.
        See also: self.GetRenderer()

        :param renderer: Optional instance or renderer from which the pipeline was removed
        :return:
        """
        pass

    def OnUpdate(self, obj: vtkObject, eventId: int, callData: Any | None) -> None:
        """
        Observer update callback.
        Triggered when any object & events observed using UpdateObserver is triggered.

        :param obj: vtkObject instance which triggerred the callback
        :param eventId: Event id which triggerred the callback
        :param callData: Optional observer call data. Use self.CastCallData(callData, vtkType) to convert to Python
        """
        pass

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        """
        Triggered when the pipeline can process the interaction and is at the top of the priority list.
        default behavior: does nothing and returns false.

        :param eventData: The MRML event needing to be processed
        :return: True if event was processed. False otherwise (default = false)
        """
        return False

    def SetDisplayNode(self, displayNode: vtkMRMLNode) -> None:
        """
        Set the display node for the pipeline has changed (initialization).
        default behavior: Stored and display node is observed for vtkCommand::ModifiedEvent.
        See also: self.UpdateObserver(prevObj, newObj, eventIds)
        See also: self.OnUpdate(obj, eventId, callData)

        :param displayNode: The new instance of display node for the pipeline
        """
        vtkMRMLLayerDMPipelineI.SetDisplayNode(self, displayNode)

    def SetPipelineManager(self, pipelineManager: vtkMRMLLayerDMPipelineManager) -> None:
        """
        Set the pipeline manager (initialization).
        default behavior: Stores the pipeline manager to delegate request render calls (no active observer).

        See also: self.GetPipelineManager()
        See also: self.GetNodePipeline(node)
        :param pipelineManager: The instance of pipeline manager managing the current pipeline
        """
        vtkMRMLLayerDMPipelineI.SetPipelineManager(self, pipelineManager)

    def SetScene(self, scene: vtkMRMLScene) -> None:
        """
        Set the pipeline scene (initialization).
        default behavior: Stores the scene for access (no active observer).
        """
        vtkMRMLLayerDMPipelineI.SetScene(self, scene)

    def SetViewNode(self, viewNode: vtkMRMLAbstractViewNode) -> None:
        """
        Set the pipeline view node  (initialization).
        default behavior: Stored and view node is observed for vtkCommand::ModifiedEvent.
        :param viewNode: The instance of viewNode the pipeline is attached to
        """
        vtkMRMLLayerDMPipelineI.SetViewNode(self, viewNode)

    def UpdatePipeline(self) -> None:
        """
        Triggered by self.ResetDisplay() calls
        Override to update the representation of the pipeline in the different views.

        See also: self.RequestRender()
        default behavior: does nothing.
        """
        pass
