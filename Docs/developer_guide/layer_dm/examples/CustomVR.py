"""
This example shows how to create a custom volume rendering for a volume node using the SlicerLayerDisplayableManager
module.

It goes over the following concepts:
    - Creates a VR display pipeline on a selected volume node in the scene
    - Register the pipeline creation mechanism

Usage:
    This example is implemented as a scripted module and can be added as such to Slicer.
"""

import qt
import slicer
from slicer.ScriptedLoadableModule import ScriptedLoadableModule, ScriptedLoadableModuleWidget


from LayerDMLib import vtkMRMLLayerDMScriptedPipeline
from slicer import (
    vtkMRMLAbstractViewNode,
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLNode,
    vtkMRMLScene,
    vtkMRMLScriptedModuleNode,
    vtkMRMLTransformNode,
    vtkMRMLViewNode,
    vtkMRMLVolumeNode,
)
from vtk import (
    VTK_OBJECT,
    calldata_type,
    vtkColorTransferFunction,
    vtkGPUVolumeRayCastMapper,
    vtkGeneralTransform,
    vtkMatrix4x4,
    vtkPiecewiseFunction,
    vtkRenderer,
    vtkTransform,
    vtkVolume,
    vtkVolumeProperty,
)


class CustomVR(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "Custom VR Pipeline Example"
        self.parent.categories = ["qSlicerAbstractCoreModule", "Examples"]
        self.parent.dependencies = []
        self.parent.contributors = []
        self.parent.helpText = ""
        self.parent.acknowledgementText = ""


class CustomVRWidget(ScriptedLoadableModuleWidget):
    """In this example, we will display a custom volume rendering and use VTK compute shader for processing the volume."""

    def setup(self) -> None:
        registerPipeline()
        ScriptedLoadableModuleWidget.setup(self)

        widget = qt.QWidget()
        layout = qt.QVBoxLayout(widget)

        # Configure volume selector node
        self._volumeNodeSelector = slicer.qMRMLNodeComboBox(widget)
        self._volumeNodeSelector.nodeTypes = ["vtkMRMLVolumeNode"]
        self._volumeNodeSelector.selectNodeUponCreation = True
        self._volumeNodeSelector.addEnabled = False
        self._volumeNodeSelector.removeEnabled = False
        self._volumeNodeSelector.showHidden = False
        self._volumeNodeSelector.renameEnabled = True
        self._volumeNodeSelector.setMRMLScene(slicer.mrmlScene)
        layout.addWidget(self._volumeNodeSelector)

        # Configure toggle button
        toggleDisplayButton = qt.QPushButton("Toggle display")
        toggleDisplayButton.clicked.connect(self._toggleVolumeDisplay)
        layout.addWidget(toggleDisplayButton)
        layout.addStretch()

        self.layout.addWidget(widget)

    def _toggleVolumeDisplay(self, *_):
        volumeNode = self._volumeNodeSelector.currentNode()
        if not volumeNode:
            return

        wasVisible = CustomVRPipeline.GetVRNodeVisibility(volumeNode, slicer.mrmlScene)
        vrNode = CustomVRPipeline.CreateVRNode(volumeNode, slicer.mrmlScene)
        CustomVRPipeline.SetVRNodeVisible(vrNode, not wasVisible)

    def onReload(self):
        """Customization of reload to allow reloading of the CustomVRLib files."""
        import importlib

        packageName = "CustomVRLib"
        submodules = ["CustomVRPipeline"]

        # Reload the package
        module = importlib.import_module(packageName)
        importlib.reload(module)

        # Reload submodules
        for sub in submodules:
            fullName = f"{packageName}.{sub}"
            submodule = importlib.import_module(fullName)
            importlib.reload(submodule)

        ScriptedLoadableModuleWidget.onReload(self)


class CustomVRPipeline(vtkMRMLLayerDMScriptedPipeline):
    """Custom VR pipeline"""

    def __init__(self):
        """
        In the pipeline creation, we create the different VTK objects.
        Here, the mapper properties are static, but they could be set in the data node and be reactive.
        """
        super().__init__()

        self._mapper = vtkGPUVolumeRayCastMapper()
        self._actor = vtkVolume()
        self._actor.SetMapper(self._mapper)

        # Arbitrary color / opacity functions
        color_function = vtkColorTransferFunction()
        color_function.AddRGBPoint(0, 0.0, 0.0, 0.0)
        color_function.AddRGBPoint(3900, 1.0, 1.0, 1.0)

        # Create the opacity
        opacity_function = vtkPiecewiseFunction()
        opacity_function.AddPoint(1000, 0.0)
        opacity_function.AddPoint(1900, 1.0)
        opacity_function.AddPoint(3900, 1.0)

        # Create the volume property and set functions
        self._volumeProperty = vtkVolumeProperty()
        self._volumeProperty.SetScalarOpacity(opacity_function)
        self._volumeProperty.SetColor(color_function)
        self._volumeProperty.SetInterpolationTypeToLinear()
        self._volumeProperty.ShadeOn()

        self._actor.SetProperty(self._volumeProperty)

        # The attributes below are used to connect observers on the volumeNode and the volumeTransform ModifiedEvent
        # The vtkMRMLLayerDMScriptedPipeline base class provides convenience methods to simply observers
        # See also: OnUpdate
        # See also: UpdateObserver
        self._volumeNode = None
        self._volumeTransform = None
        self._imageData = None

    @classmethod
    def CreateVRNode(cls, volumeNode, scene) -> vtkMRMLScriptedModuleNode:
        node = cls.GetVRNode(volumeNode, scene)
        if node:
            return node

        node = vtkMRMLScriptedModuleNode()
        node.SetAttribute("PipelineType", cls._GetClassName())
        node.SetAttribute("VolumeNodeID", volumeNode.GetID())
        node.SetAttribute("IsVisible", str(0))
        return scene.AddNode(node)

    @classmethod
    def GetVRNodeVisibility(cls, volumeNode, scene) -> bool:
        vrNode = cls.GetVRNode(volumeNode, scene)
        if vrNode is None:
            return False

        return cls.GetVisibility(vrNode)

    @classmethod
    def GetVisibility(cls, node):
        if node is None:
            return False
        return bool(int(node.GetAttribute("IsVisible")))

    @classmethod
    def SetVRNodeVisible(cls, node, isVisible: bool):
        node.SetAttribute("IsVisible", str(int(isVisible)))

    @classmethod
    def IsPipelineNode(cls, node):
        """
        Returns True if the input vktMRMLNode is a scripted node and has the pipeline type attribute matching the
        current pipeline class.
        """
        return isinstance(node, vtkMRMLScriptedModuleNode) and node.GetAttribute("PipelineType") == cls._GetClassName()

    @classmethod
    def TryCreatePipeline(
            cls, viewNode: vtkMRMLAbstractViewNode, node: vtkMRMLNode,
    ) -> vtkMRMLLayerDMScriptedPipeline | None:
        """
        Since we are creating pipelines for 3D views only, we check here if the view node is a ThreedView node and
        if the node matches the current pipeline type (i.e. was created using the _CreatePipelineNode method).
        """

        if not cls.IsPipelineNode(node) or not isinstance(viewNode, vtkMRMLViewNode):
            return None

        return cls()

    @classmethod
    def _GetClassName(cls) -> str:
        """
        Convenience method to get the name of the current class.
        This method will return the actual class name for inheriting classes.
        """
        return cls.__name__

    def OnRendererAdded(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is displayed on a new renderer.
        default behavior: does nothing.

        Here, we add our actor to the input renderer.
        If the pipeline renderer has changed, the pipeline's ResetDisplay method will be triggered and in turn its
        UpdatePipeline method will be triggered.

        Since we don't control the actual renderer used by the pipeline, this should be used systematically.
        See also: self.GetRenderer()
        """

        if renderer is None or renderer.HasViewProp(self._actor):
            return
        renderer.AddViewProp(self._actor)

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is removed from its previous renderer.
        default behavior: does nothing.

        Here, we add our actor to the input renderer.
        If the pipeline renderer has changed, the pipeline's ResetDisplay method will be triggered and in turn its
        UpdatePipeline method will be triggered.

        Since we don't control the actual renderer used by the pipeline, this should be used systematically.
        See also: self.GetRenderer()
        """

        if renderer is None or not renderer.HasViewProp(self._actor):
            return
        renderer.RemoveViewProp(self._actor)

    def UpdatePipeline(self):
        """
        Triggered by self.ResetDisplay() calls:
            - Called automatically at pipeline creation / add to the render window
            - Called automatically when switching renderer
        Override to update the representation of the pipeline in the different views.

        See also: self.RequestRender()
        default behavior: does nothing.
        """
        self._UpdateMapperConnection()
        self._UpdateActorVisibility()
        self.RequestRender()

    def OnUpdate(self, obj, eventId, callData):
        """
        Observer update callback.
        Triggered when any object & events observed using UpdateObserver is triggered.

        :param obj: vtkObject instance which triggered the callback
        :param eventId: Event id which triggered the callback
        :param callData: Optional observer call data. Use self.CastCallData(callData, vtkType) to convert to Python
        """

        if obj == self._volumeNode:
            self._ObserveVolumeTransformNode()
            self._ObserveVolumeImageData()

        self.ResetDisplay()

    def SetDisplayNode(self, node):
        """
        Set the display node for the pipeline has changed (initialization).
        default behavior: Stored and display node is observed for vtkCommand::ModifiedEvent.
        See also: self.UpdateObserver(prevObj, newObj, eventIds)
        See also: self.OnUpdate(obj, eventId, callData)

        :param node: The new instance of display node for the pipeline
        """
        super().SetDisplayNode(node)
        self._ObserveVolumeNode()

    def _UpdateMapperConnection(self):
        volumeNode: vtkMRMLVolumeNode = self._GetVolumeNode()
        transform = vtkTransform()
        ijk_to_world_matrix = self._GetVolumeTransformMatrixToWorld()
        if ijk_to_world_matrix:
            transform.SetMatrix(ijk_to_world_matrix)

        self._actor.SetUserTransform(transform)
        self._mapper.SetInputConnection(volumeNode.GetImageDataConnection() if volumeNode else None)

    def _GetVolumeTransformMatrixToWorld(self):
        """
        Converts a volume's IJK coordinates to World coordinates.
        Returns True if successful, False otherwise.
        """
        volume_node = self._GetVolumeNode()
        if not volume_node:
            return None

        # Check if we have a transform node
        transform_node = volume_node.GetParentTransformNode()

        ijk_to_world_matrix = vtkMatrix4x4()
        if not transform_node:
            volume_node.GetIJKToRASMatrix(ijk_to_world_matrix)
            return ijk_to_world_matrix

        # Check if the transform is linear
        if not transform_node.IsTransformToWorldLinear():
            return None

        # IJK to RAS (Local)
        ijk_to_ras_matrix = vtkMatrix4x4()
        volume_node.GetIJKToRASMatrix(ijk_to_ras_matrix)

        # Parent transforms (RAS to World)
        node_to_world_matrix = vtkMatrix4x4()
        transform_node.GetMatrixTransformToWorld(node_to_world_matrix)

        # Multiply: output = node_to_world_matrix * ijk_to_ras_matrix
        vtkMatrix4x4.Multiply4x4(node_to_world_matrix, ijk_to_ras_matrix, ijk_to_world_matrix)
        return ijk_to_world_matrix

    def _GetTransform(self):
        transformNode = self._volumeNode.GetParentTransformNode() if self._volumeNode else None
        if transformNode is None:
            return None
        transform = vtkGeneralTransform()
        transformNode.GetTransformToWorld(transform)
        return transform

    def _UpdateActorVisibility(self):
        self._actor.SetVisibility(self._IsVolumeVisible() and self.GetVisibility(self.GetDisplayNode()))

    @classmethod
    def RemoveVRNode(cls, volumeNode: vtkMRMLVolumeNode, scene: vtkMRMLScene):
        node = cls.GetVRNode(volumeNode, scene)

        if node is not None:
            scene.RemoveNode(node)
        else:
            print("No VR node for: ", volumeNode.GetID())

    @classmethod
    def GetVRNode(cls, volumeNode: vtkMRMLVolumeNode, scene: vtkMRMLScene):
        for node in slicer.util.getNodesByClass("vtkMRMLScriptedModuleNode", scene):
            if cls._GetVolumeNodeID(node) == volumeNode.GetID():
                return node
        return None

    def _IsVolumeVisible(self) -> bool:
        """Convenience method to check if the pipeline's volume node is visible."""
        volumeNode = self._GetVolumeNode()
        if volumeNode is None:
            return False
        return bool(volumeNode.GetDisplayVisibility())

    def _GetVolumeNode(self) -> vtkMRMLVolumeNode | None:
        """
        Convenience method to get the volume node associated with the pipeline's data node.

        Here, we use the following APIs:
            - GetScene: This will return the scene on which the pipeline is attached
            - GetDisplayNode: This will return the pipeline's data node instance
        """
        return self.GetScene().GetNodeByID(self._GetVolumeNodeID(self.GetDisplayNode()))

    def _ObserveVolumeNode(self):
        """
        Convenience method to update the volume and the volume's transform node observers.

        Here, we use the UpdateObserver method:
            - UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) -> bool
            - UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent) -> bool

        This method should be used to add an observer on VTK object.
        By default, the object's modified event will be observed.
        The method also supports lists of events.

        On modify event, the class's self.OnUpdate method will be called.

        Warning: prevObj is not mutated by this call. To update the pointer, a manual set is required after update.

        See also: self.OnUpdate
        """
        if self._volumeNode == self._GetVolumeNode():
            return

        self.UpdateObserver(self._volumeNode, self._GetVolumeNode())
        self._volumeNode = self._GetVolumeNode()
        self._ObserveVolumeTransformNode()
        self._ObserveVolumeImageData()

    def _ObserveVolumeTransformNode(self):
        """
        Convenience method to update the volume's transform node observer.

        Note: Here we explicitly observe the transform modified event as modifying the transform doesn't trigger
        its modified event.

        See also: self._ObserveVolumeNode
        """
        transformNode = self._volumeNode.GetParentTransformNode() if self._volumeNode else None
        if self._volumeTransform == transformNode:
            return

        self.UpdateObserver(
            self._volumeTransform,
            transformNode,
            vtkMRMLTransformNode.TransformModifiedEvent,
        )
        self._volumeTransform = transformNode

    def _ObserveVolumeImageData(self):
        imageData = self._volumeNode.GetImageData() if self._volumeNode else None
        if self._imageData == imageData:
            return

        self.UpdateObserver(self._imageData, imageData)
        self._imageData = imageData

    @classmethod
    def _GetVolumeNodeID(cls, node):
        """Convenience method to get the volume node ID attached to the input MRML node."""
        if cls.IsPipelineNode(node):
            return node.GetAttribute("VolumeNodeID")
        return ""

    @classmethod
    def _HasVRNode(cls, volumeNode):
        return cls._GetVolumeNodeID(volumeNode) != ""


def registerPipeline():
    """
    For the pipeline registration, we will register the pipeline creation mechanism and auto create view nodes when
    a new volume node is added to the scene.
    """
    registerPipelineCreator()
    autoRemoveVRNode()


def registerPipelineCreator():
    def tryCreate(view_node, node):
        pipelines = [CustomVRPipeline]
        for pipeline in pipelines:
            ret = pipeline.TryCreatePipeline(view_node, node)
            if ret is not None:
                return ret
        return None

    pipeline_creator = vtkMRMLLayerDMPipelineScriptedCreator()
    pipeline_creator.SetPythonCallback(tryCreate)
    vtkMRMLLayerDMPipelineFactory.GetInstance().AddPipelineCreator(pipeline_creator)


def autoRemoveVRNode():
    """
    This function is a convenience function to manage the data nodes in the scene.
    We make sure to collect our VR node when the associated volume node is removed.
    """

    @calldata_type(VTK_OBJECT)
    def onNodeRemoved(_caller, _event, node):
        if isinstance(node, vtkMRMLVolumeNode):
            CustomVRPipeline.RemoveVRNode(node, slicer.mrmlScene)

    slicer.mrmlScene.AddObserver(vtkMRMLScene.NodeRemovedEvent, onNodeRemoved)
