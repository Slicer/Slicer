"""
This example shows how to create a glow around a model node using the SlicerLayerDisplayableManager module.

It goes over the following concepts:
    - Creates a unique display pipeline to set attach a VTK glow pass to a renderer
    - Creates one glow display pipeline per 3D view for each created model nodes in the scene
    - Register the pipeline creation mechanism

Usage:
    This example is implemented as a scripted module and can be added as such to Slicer.
    Once added, loading new model nodes will set their glow pass automatically.
"""

import sys
import qt
import random

import slicer
from slicer import (
    vtkMRMLAbstractViewNode,
    vtkMRMLInteractionEventData,
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLModelNode,
    vtkMRMLNode,
    vtkMRMLScene,
    vtkMRMLScriptedModuleNode,
    vtkMRMLTransformNode,
    vtkMRMLViewNode,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModule, ScriptedLoadableModuleWidget
from vtk import (
    VTK_OBJECT,
    calldata_type,
    vtkActor,
    vtkCommand,
    vtkGeneralTransform,
    vtkMath,
    vtkNamedColors,
    vtkOutlineGlowPass,
    vtkPolyDataMapper,
    vtkRenderStepsPass,
    vtkRenderer,
    vtkSphereSource,
    vtkTransformPolyDataFilter,
)

from LayerDMLib import vtkMRMLLayerDMScriptedPipeline


class ModelGlowDM(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = " Model Glow Pipeline Example"
        self.parent.categories = ["qSlicerAbstractCoreModule", "Examples"]
        self.parent.dependencies = []
        self.parent.contributors = []
        self.parent.helpText = ""
        self.parent.acknowledgementText = ""

        # At startup connected, the pipeline registration is called
        # This allows the pipeline registration to be done automatically at loading time
        slicer.app.connect("startupCompleted()", registerPipeline)


class _Pipeline(vtkMRMLLayerDMScriptedPipeline):
    """
    This class is a convenience abstract class for the glow pass pipelines.
    It provides the following static methods:
        - _CreatePipelineNode: Creates a new scripted node with a PipelineType string property containing the class type
            This value is used to check if the pipeline should be created when a node is added to the scene.
        - IsPipelineNode: Checks if a node is a scripted node and it contains the right PipelineType string property
        - TryCreatePipeline: Creates new pipeline instances for views matching 3D views and PipelineType nodes.
    """

    @classmethod
    def _CreatePipelineNode(cls) -> vtkMRMLScriptedModuleNode:
        """
        Creates a new scripted node with a PipelineType string property containing the class type.
        This value is used to check if the pipeline should be created when a node is added to the scene.

        In Python, inheritance of vtkMRMLDisplayNode is not possible to create new display node types.
        To bypass this limitation and create display data that will be used in the pipelines, we use
        vtkMRMLScriptedModuleNode instead.

        vtkMRMLScriptedModuleNode can contain any pairs of string keys and string values.
        We use this mechanism to store the type of pipeline we would like to create for the given node.

        The actual choice of attribute is arbitrary in this example and the creation logic can be adapted in actual
        application code.

        Warning: Pipeline creation in the views is triggered by adding the node to the scene.
        vtkMRMLScriptedModuleNode properties should be initialized prior to adding the nodes to the scene.
        """
        node = vtkMRMLScriptedModuleNode()
        node.SetAttribute("PipelineType", cls._GetClassName())
        return node

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


class GlowDMPassPipeline(_Pipeline):
    """
    The GlowDMPassPipeline is responsible for setting a glow render pass to the input renderer.
    Since the pass only need to be set once, the following are used in this class:
        - The data node is set to be a singleton data. This will make the data node persist in between scene clear.
        - Setting and removing the render pass is done on renderer added / removed API calls.
        - We set the pipeline to its own renderer (GetRenderOrder != 0) and we make it easy for other classes to know
            the pipeline's render order by adding a convenience static method.

    Note: This logic doesn't need to be split but is an example of possible implementation decoupling.
    """

    def __init__(self):
        """
        At creation, we call the super class initialization and create the VTK passes.
        The VTK passes are then set during the API on renderer added / removed calls.
        """
        super().__init__()
        self._basicPasses = vtkRenderStepsPass()
        self._glowPass = vtkOutlineGlowPass()
        self._glowPass.SetDelegatePass(self._basicPasses)

    def OnRendererAdded(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is displayed on a new renderer.

        When the renderer is added, we attach our glow pass.
        Since we don't control the actual renderer used by the pipeline, this should be used systematically.
        See also: self.GetRenderer()
        """

        if renderer is None:
            return
        renderer.SetPass(self._glowPass)

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is removed from its previous renderer.

        When the renderer is removed, we remove our glow pass.
        Since we don't control the actual renderer used by the pipeline, this should be used systematically.
        See also: self.GetRenderer()
        """

        if renderer is None:
            return
        renderer.SetPass(None)

    def GetRenderOrder(self) -> int:
        """
        Arbitrary render order number where the pipeline wants to be displayed.
        Return 0 to be at the default order (main 3D Slicer pipelines)
        Return larger values to be rendered on top of pipelines with lower render orders.

        :return: default = 0. Here we use a helper static method to return the Glow pass render order and allow
            pipelines that want to be rendered in this renderer to use it as well.
        """
        return self.GetGlowPassRenderOrder()

    @classmethod
    def GetGlowPassRenderOrder(cls):
        """Convenience static method to return the glow pass render order and be used by other classes."""
        return 1

    @classmethod
    def EnsureGlowPass(cls, scene: vtkMRMLScene):
        """
        Convenience static method to add a singleton render glow pass data node to the scene.
        The scene is passed in argument to avoid using the singleton slicer.mrmlScene which is not available in
        trame-slicer.
        """
        if cls.SceneHasGlowPass(scene):
            return

        node = cls._CreatePipelineNode()
        node.SetSingletonOn()
        node.SetSingletonTag("RenderGlowPassPipeline")
        scene.AddNode(node)

    @classmethod
    def SceneHasGlowPass(cls, scene: vtkMRMLScene) -> bool:
        """
        Convenience static method to check if the singleton glow pass data node is present in the scene.
        The scene is passed in argument to avoid using the singleton slicer.mrmlScene which is not available in
        trame-slicer.
        """
        return scene.GetNodeByID("RenderGlowPassPipeline") is not None


class ModelGlowDMPipeline(_Pipeline):
    """
    The ModelGlowDMPipeline is responsible for the actual creation of actors (and mappers) that will be rendered
    in the glow pass renderer.

    In this pipeline, we do three things:
        - Configure the rendering pipeline
        - Connect the pipeline reactivity to the scene observers
        - Connect the interaction events to make our model glow when the interaction is within the model's bounding box

    Creation of the pipeline will be handled by our _Pipeline.TryCreatePipeline base methods and connected to the
    factory in the registerPipeline method that we connected to the application load event.
    """

    def __init__(self):
        """
        In the pipeline creation, we create the different VTK objects.
        Here, the mapper properties are static, but they could be set in the data node and be reactive.
        """
        super().__init__()

        colors = vtkNamedColors()
        self._glowMapper = vtkPolyDataMapper()
        self._glowActor = vtkActor()
        self._glowActor.SetMapper(self._glowMapper)
        self._glowActor.GetProperty().SetColor(colors.GetColor3d("Magenta"))
        self._glowActor.GetProperty().LightingOff()

        # The two attributes below are used to connect observers on the modelNode and the modelTransform ModifiedEvent
        # The vtkMRMLLayerDMScriptedPipeline base class provides convenience methods to simply observers
        # See also: OnUpdate
        # See also: UpdateObserver
        self._modelNode = None
        self._modelTransform = None

        # The attribute below is used to store the transformed polydata.
        # Its bounding boxes will be used for user interaction.
        self._polyData = None

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

        if renderer is None or renderer.HasViewProp(self._glowActor):
            return
        renderer.AddViewProp(self._glowActor)

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

        if renderer is None or not renderer.HasViewProp(self._glowActor):
            return
        renderer.RemoveViewProp(self._glowActor)

    def GetRenderOrder(self) -> int:
        """
        Arbitrary render order number where the pipeline wants to be displayed.
        Here, we use the glow pass's render order to be in the same renderer (although we don't know which it will be)
        """
        return GlowDMPassPipeline.GetGlowPassRenderOrder()

    def UpdatePipeline(self):
        """
        Triggered by self.ResetDisplay() calls:
            - Called automatically at pipeline creation / add to the render window
            - Called automatically when switching renderer
        Override to update the representation of the pipeline in the different views.

        See also: self.RequestRender()
        default behavior: does nothing.

        Here, we update the mapper connection to our modelNode PolyData and update the actor visibility to follow
        the model's visibility.
        Finally we ask for a rendering refresh.
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

        Here, we want to update our model transform node observer if it has changed and trigger the pipeline's display
        when either the view has changed (observed by default), the data node has changed (observed by default), or
        our modelNode / transform nodes have changed (manually observed).
        """

        if obj == self._modelNode:
            self._ObserveModelTransformNode()

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
        self._ObserveModelNode()

    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        """
        Should return true + distance2 to interaction if the pipeline can process the input event data.
        :param eventData: The MRML event needing to be processed
        :return: (bool, distance2) default = False, float_max

        Here, we check if the event interaction is within the glow pass actor bounds and return the distance to it.

        To avoid blocking camera interaction, we will also process mouse move events.
        Of course we could also check for left click to go further with this widget.

        Note: This is not an efficient way to check for interactions. A better way would be to delegate to the model
            display node for picking events.
        """
        # Only process mouse move events to avoid blocking camera interaction on click / drag
        isMouseMoveEvent = eventData.GetType() == vtkCommand.MouseMoveEvent

        if not isMouseMoveEvent or not self._IsModelVisible() or self._polyData is None:
            return False, sys.float_info.max

        pos = eventData.GetWorldPosition()
        glowActorBounds = self._polyData.GetBounds()
        isInBounds = (
                glowActorBounds[0] < pos[0] < glowActorBounds[1]
                and glowActorBounds[2] < pos[1] < glowActorBounds[3]
                and glowActorBounds[4] < pos[2] < glowActorBounds[5]
        )
        distance2 = vtkMath.Distance2BetweenPoints(pos, self._polyData.GetCenter())
        return isInBounds, distance2

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        """
        Triggered when the pipeline can process the interaction and is at the top of the priority list.
        default behavior: does nothing and returns false.

        :param eventData: The MRML event needing to be processed
        :return: True if event was processed. False otherwise (default = false)

        Here, the pipeline is the closest to the interaction. We modify our display property which will trigger the
        rendering update.
        """
        if not self.GetDisplayNode():
            return False

        self.GetDisplayNode().SetAttribute("IsSelected", str(1))
        return True

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData | None) -> None:
        """
        Triggered when the pipeline had focus (processed an interaction) and loses the focus (other pipeline
        handled the new interaction or window leave event).
        default behavior: does nothing.
        :param eventData: Optional event data which triggered the lose focus

        Here, the pipeline lost the previous interaction.
        We make sure to restore the selection state.
        """
        super().LoseFocus(eventData)
        if not self.GetDisplayNode():
            return
        self.GetDisplayNode().SetAttribute("IsSelected", str(0))

    def _UpdateMapperConnection(self):
        """
        Convenience method to update the mapper connection.
        Note: We apply the polydata transform manually here, but we could configure a transform pipeline and use
            the model's polydata connection instead for a cleaner VTK implementation.
        """
        modelNode: vtkMRMLModelNode = self._GetModelNode()
        self._polyData = self._TransformPolyData(modelNode.GetPolyData() if modelNode else None)
        self._glowMapper.SetInputData(self._polyData)

    def _TransformPolyData(self, polyData):
        """Convenience method to update the transformed displayed polydata based on the current transform node."""
        transformNode = self._modelNode.GetParentTransformNode() if self._modelNode else None
        if transformNode is None:
            return polyData
        transformFilter = vtkTransformPolyDataFilter()
        transform = vtkGeneralTransform()
        transformNode.GetTransformToWorld(transform)
        transformFilter.SetTransform(transform)
        transformFilter.SetInputData(polyData)
        transformFilter.Update()
        return transformFilter.GetOutput()

    def _UpdateActorVisibility(self):
        """Convenience method to update the actor based on the model's visibility and the selection."""
        isSelected = bool(self.GetDisplayNode() and int(self.GetDisplayNode().GetAttribute("IsSelected")))
        self._glowActor.SetVisibility(self._IsModelVisible() and isSelected)

    @classmethod
    def CreateGlowNode(cls, modelNode: vtkMRMLModelNode, scene: vtkMRMLScene):
        """
        Convenience static method to create and add a new glow data node pointing to a model node and add it to the
        scene.

        Note: Here, we could add our new data node as a reference node for the modelNode instead of keeping each
            separate. This would allow to iterate over the modelNode's references and find it instead of having
            to iterate on the scene.
        """

        # Since our nodes can store key / value strings, we set a new key for the model node ID for the model this
        # glow effect will be attached to.
        # We also store a selection value to be able to update the model glow on user interaction.
        # We could store other display properties here as well (or point to a dedicated display node storing more
        # information)
        node = cls._CreatePipelineNode()
        node.SetAttribute("ModelNodeID", modelNode.GetID())
        node.SetAttribute("IsSelected", str(0))
        return scene.AddNode(node)

    @classmethod
    def RemoveGlowNode(cls, modelNode: vtkMRMLModelNode, scene: vtkMRMLScene):
        """
        Convenience static method to remove a glow node set on a given modelNode.
        See also: autoCreateGlowNode

        Note: This logic can be simplified if we attach our pipeline to the model node directly.
            We would then iterate over node references to check if we have our pipeline node.
        """
        for node in slicer.util.getNodesByClass("vtkMRMLScriptedModuleNode", scene):
            if cls._GetModelNodeID(node) == modelNode.GetID():
                scene.RemoveNode(node)

    def _IsModelVisible(self) -> bool:
        """Convenience method to check if the pipeline's model node is visible."""
        modelNode = self._GetModelNode()
        if modelNode is None:
            return False
        return bool(modelNode.GetDisplayVisibility())

    def _GetModelNode(self) -> vtkMRMLModelNode | None:
        """
        Convenience method to get the model node associated with the pipeline's data node.

        Here, we use the following APIs:
            - GetScene: This will return the scene on which the pipeline is attached
            - GetDisplayNode: This will return the pipeline's data node instance
        """
        return self.GetScene().GetNodeByID(self._GetModelNodeID(self.GetDisplayNode()))

    def _ObserveModelNode(self):
        """
        Convenience method to update the model and the model's transform node observers.

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
        if self._modelNode == self._GetModelNode():
            return

        self.UpdateObserver(self._modelNode, self._GetModelNode())
        self._modelNode = self._GetModelNode()
        self._ObserveModelTransformNode()

    def _ObserveModelTransformNode(self):
        """
        Convenience method to update the model's transform node observer.

        Note: Here we explicitly observe the transform modified event as modifying the transform doesn't trigger
        its modified event.

        See also: self._ObserveModelNode
        """
        transformNode = self._modelNode.GetParentTransformNode() if self._modelNode else None
        if self._modelTransform == transformNode:
            return

        self.UpdateObserver(self._modelTransform, transformNode, vtkMRMLTransformNode.TransformModifiedEvent)
        self._modelTransform = transformNode

    @classmethod
    def _GetModelNodeID(cls, node):
        """Convenience method to get the model node ID attached to the input MRML node."""
        if cls.IsPipelineNode(node):
            return node.GetAttribute("ModelNodeID")
        return ""


def registerPipeline():
    """
    For the pipeline registration, we will register the pipeline creation mechanism and auto create view nodes when
    a new model node is added to the scene.
    """
    registerPipelineCreator()
    autoCreateGlowNode()


def registerPipelineCreator():
    """
    For pipelines to be created in our views, we need to use the pipeline factory to register pipeline creator
    instances.

    When a node is added to the scene, the LayerDM orchestration will query the vtkMRMLLayerDMPipelineFactory singleton
    instance to check if a pipeline can be created.

    The factory will receive two information: The view node on which it is attached and the newly created node.

    To add a new creator to the factory, we use a vtkMRMLLayerDMPipelineScriptedCreator instance and set a callback
    to our custom tryCreate function.

    This function will iterate on the pipelines we want to create and create it when applicable.

    Note: Scene lifecycle are managed by the LayerDM library. If the view is newly created, its pipeline manager will
        iterate over all the nodes in the scene to check if pipelines need to be created.

        Similarly, when loading a scene, clearing a scene, the pipelines will be handled accordingly.
    """

    def tryCreate(view_node, node):
        pipelines = [GlowDMPassPipeline, ModelGlowDMPipeline]
        for pipeline in pipelines:
            ret = pipeline.TryCreatePipeline(view_node, node)
            if ret is not None:
                return ret
        return None

    pipeline_creator = vtkMRMLLayerDMPipelineScriptedCreator()
    pipeline_creator.SetPythonCallback(tryCreate)
    vtkMRMLLayerDMPipelineFactory.GetInstance().AddPipelineCreator(pipeline_creator)


def autoCreateGlowNode():
    """
    This function is a convenience function to manage the data nodes in the scene.

    Here, we attach two observers to the scene for node added / removed.
    We then check to add our glow pipeline to model nodes when they are added and garbage collect them on removal.

    We also create our glow pass data node so that the glow pass pipeline is created.
    """

    @calldata_type(VTK_OBJECT)
    def onNodeAdded(_caller, _event, node):
        if isinstance(node, vtkMRMLModelNode):
            ModelGlowDMPipeline.CreateGlowNode(node, slicer.mrmlScene)

    @calldata_type(VTK_OBJECT)
    def onNodeRemoved(_caller, _event, node):
        if isinstance(node, vtkMRMLModelNode):
            ModelGlowDMPipeline.RemoveGlowNode(node, slicer.mrmlScene)

    GlowDMPassPipeline.EnsureGlowPass(slicer.mrmlScene)
    slicer.mrmlScene.AddObserver(vtkMRMLScene.NodeAddedEvent, onNodeAdded)
    slicer.mrmlScene.AddObserver(vtkMRMLScene.NodeRemovedEvent, onNodeRemoved)


class ModelGlowDMWidget(ScriptedLoadableModuleWidget):
    """In this example, the module's widget will allow us to create random sphere model nodes in the scene."""

    def setup(self) -> None:
        """
        In the setup method, we create a widget with only two buttons:
            - A "create sphere" button to create a random sphere in the scene
            - A "Reset 3D views" button to reset the 3D view on the created spheres
        """
        ScriptedLoadableModuleWidget.setup(self)

        widget = qt.QWidget()
        layout = qt.QVBoxLayout(widget)

        createSphereButton = qt.QPushButton("Create sphere")
        createSphereButton.clicked.connect(self._onCreateSphereClicked)
        layout.addWidget(createSphereButton)

        reset3DView = qt.QPushButton("Reset 3D views")
        reset3DView.clicked.connect(slicer.util.resetThreeDViews)
        layout.addWidget(reset3DView)
        layout.addStretch()

        self.layout.addWidget(widget)

    @classmethod
    def _onCreateSphereClicked(cls, *_):
        """
        Here we create the sphere models at random.

        Note: Attaching the glow data to the model could be done in this type of methods if we wanted more control
            on the creation logic.
        """
        # Create a sphere positioned at a random position
        sphereSource = vtkSphereSource()
        sphereSource.SetCenter(random.uniform(0, 10),
                               random.uniform(0, 10),
                               random.uniform(0, 10))
        sphereSource.SetRadius(random.uniform(0.1, 1.0))
        sphereSource.Update()

        # Create the model node and set its polydata
        modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        modelNode.SetAndObservePolyData(sphereSource.GetOutput())
        modelNode.CreateDefaultDisplayNodes()

        # Set random color
        displayNode = modelNode.GetDisplayNode()
        modelNode.SetAndObserveDisplayNodeID(displayNode.GetID())
        displayNode.SetColor(random.random(), random.random(), random.random())
