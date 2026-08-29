# How to implement a displayable manager with Layer DM module

To implement a displayable manager with the Layer DM module, developers need to implement the following classes / logic:

* The [Pipeline object](#pipeline) responsible for adding actors and handling interaction events
* The [creation logic of the pipeline](#registration-logic), often limited to checking the input view node and display node types

## Pipeline

Pipeline objects derive from the vtkMRMLLayerDMPipeline base class in C++ and the vtkMRMLLayerDMScriptedPipeline in
python.

Pipeline implementation can be split in two:

* The display logic of new VTK actors / mappers
* The user interaction handling

For full examples, please check out the [examples section of the documentation](examples.md).

### Displaying actors

Actors can be added / removed from the renderer through the `OnRendererAdded` and `OnRendererRemoved` methods:

* `virtual void OnRendererAdded(vtkRenderer* renderer)`: Called when attached to a new renderer.
* `virtual void OnRendererRemoved(vtkRenderer* renderer)`: Called when removed from a renderer.

After initialization, the pipeline can access its renderer if needed using:

* `vtkRenderer* GetRenderer() const`: Get current renderer.

After the first display, actor visibility properties should be made reactive depending on the scene state.

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def OnRendererAdded(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is displayed on a new renderer.
        default behavior: does nothing.
        """
        if renderer is None or renderer.HasViewProp(self._myActor):
            return
        renderer.AddViewProp(self._myActor)

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        """
        Triggered when the pipeline is removed from its previous renderer.
        default behavior: does nothing.
        """

        if renderer is None or not renderer.HasViewProp(self._myActor):
            return
        renderer.RemoveViewProp(self._myActor)
```

### Updating actors

By default, the display is refreshed when the pipeline is created. Afterwards, refreshing its display should be
connected to the object events and the `UpdateDisplay` method should be called.

Calling this method will call the `UpdateFromMRML` method and will request a render from the widget.

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def UpdateFromMRML(self):
        """
        Triggered by self.UpdateDisplay() calls:
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

        # Calling UpdateDisplay will trigger the UpdateFromMRML method call
        self.UpdateDisplay()
```

### Rendering on top of other actors

Pipelines can define their rendering order through the `GetRenderOrder` or `GetRenderOrders` method(s).

* `virtual unsigned int GetRenderOrder() const`: Return render order priority (default = 0).
* `virtual std::vector<unsigned int> GetRenderOrders() const`: Return render order priorities for each renderer layer
  the pipeline wants to use. This method is recommended when the pipelines wants to manage both background / foreground
  representations. The pipeline can return an arbitrary long list of RenderOrders (default = {GetRenderOrder()}).

If the value 0 is used, then the objects will be rendered in the default view renderer along builtin 3D Slicer objects.

If the grouping configuration (render order or custom cameras) changes dynamically, the pipeline should emit the
`RenderGroupingModified` event.

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def GetRenderOrder(self) -> int:
        """
        Arbitrary render order number where the pipeline wants to be displayed.
        default= 0
        """
        return 42
```

### Monitoring node changes

Most pipelines will react to node changes to update their display. The pipeline API automatically monitors the view node
and the display node modified events (i.e., the node which triggered the pipeline's creation).

When the modified events are triggered, the pipeline's `OnUpdate` method will be called with the object which was
modified, the event ID and call data.

* `virtual void OnUpdate(vtkObject* obj, unsigned long eventId, void* callData)`: Callback when observed event triggers.

To observe other object events, the `UpdateObserver` can be used.

* `bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) const`: Update
  observer with multiple events.
* `bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent) const`:
  Update observer with single event.
* `void RemoveObserver(vtkObject* prevObj) const`: Remove all observed events.

````{warning}
`UpdateObserver` cannot update the variable passed as `prevObj` to point to the newly observed object (passing
pointers by reference is not supported by the VTK Python wrapping). After the call, the variable passed as `prevObj`
should be updated manually to the new object, so that the next `UpdateObserver` call removes the observers from the
correct object:

```python
self.UpdateObserver(self._modelTransform, transformNode, vtkMRMLTransformNode.TransformModifiedEvent)
self._modelTransform = transformNode
```

```cpp
this->UpdateObserver(this->ModelTransform, transformNode, vtkMRMLTransformNode::TransformModifiedEvent);
this->ModelTransform = transformNode;
```

If the variable is not updated, the next `UpdateObserver` call is passed a stale `prevObj`: the observers added on the
current object are never removed, leading to leaked observers and duplicate `OnUpdate` callbacks.
````

The `SetDisplayNode` and `SetViewNode` should be overridden to observe other events aside for the default modified
event.

* `virtual void SetViewNode(vtkMRMLAbstractViewNode* viewNode)`: Called at initialization.
* `virtual void SetDisplayNode(vtkMRMLNode* displayNode)`: Called at initialization.

During the `OnUpdate` call if the display should be update, then the `UpdateDisplay` method should be called. This method
will trigger the `UpdateFromMRML` method which can be overridden to update display properties and will also trigger a
request render.

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def __init__(self):
        """
        In the pipeline creation, we create the different VTK objects.
        """
        super().__init__()

        self._myMapper = vtkPolyDataMapper()
        self._myActor = vtkActor()
        self._myActor.SetMapper(self._myMapper)

        # The two attributes below are used to connect observers on the modelNode and the modelTransform ModifiedEvent
        # The vtkMRMLLayerDMScriptedPipeline base class provides convenience methods to simply observers
        # See also: OnUpdate
        # See also: UpdateObserver
        self._modelNode = None
        self._modelTransform = None

    def OnUpdate(self, obj, eventId, callData):
        """
        Observer update callback.
        Triggered when any object & events observed using UpdateObserver is triggered.

        :param obj: vtkObject instance which triggered the callback
        :param eventId: Event id which triggered the callback
        :param callData: Optional observer call data. Use self.CastCallData(callData, vtkType) to convert to Python
        """

        if obj == self._modelNode:
            self._ObserveModelTransformNode()

        self.UpdateDisplay()

    def _ObserveModelTransformNode(self):
        """
        Convenience method to update the model's transform node observer.

        Note: Here we explicitly observe the transform modified event as modifying the transform doesn't trigger
        its modified event.
        """
        transformNode = self._modelNode.GetParentTransformNode() if self._modelNode else None
        if self._modelTransform == transformNode:
            return

        self.UpdateObserver(self._modelTransform, transformNode, vtkMRMLTransformNode.TransformModifiedEvent)
        self._modelTransform = transformNode
```

### Monitoring camera changes

The main camera is observed by default and will trigger the `OnDefaultCameraModified` method call. This method has no
implementation by default but can be used to update the display if needed.

* `virtual void OnDefaultCameraModified(vtkCamera* camera)`: React to default camera changes.

### Defining a custom camera

If another camera should be used and not the default camera, then the `GetCustomCamera` method can be used to return the
camera instance the pipeline should be used.

* `virtual vtkCamera* GetCustomCamera() const`: Return custom camera or nullptr for default behavior.

Pipelines sharing the same camera instance and the same render order will be set to the same renderer.~~~~

### Processing interactions

Interaction processing is completely delegated to the pipelines thanks to the following methods:

* `virtual bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)`: Check if
  pipeline can process an interaction event.
* `virtual bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)`: Handle an interaction event if
  pipeline has focus.
* `virtual void LoseFocus(vtkMRMLInteractionEventData* eventData)`: Called when pipeline loses interaction focus.

The state of the widget and its current mouse cursor can be returned using the following methods:

* `virtual int GetMouseCursor() const`: Return custom mouse cursor VTK enum.
* `virtual int GetWidgetState() const`: Return current widget state enum (default idle).

It is advised for complex interactions to use
either [vtkMRMLLayerDMWidgetEventTranslationNode objects](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/LayerDM/MRML/vtkMRMLLayerDMWidgetEventTranslationNode.h)
or [vtkMRMLAbstractWidget objects](https://github.com/Slicer/Slicer/blob/main/Libs/MRML/DisplayableManager/vtkMRMLAbstractWidget.h).

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        """
        Should return true + distance2 to interaction if the pipeline can process the input event data.
        :param eventData: The MRML event needing to be processed
        :return: (bool, distance2) default = False, float_max

        Here, we check if the event interaction is within the actor bounds and return the distance to it.
        """
        if not self._IsModelVisible() or self._polyData is None:
            return False, sys.float_info.max

        pos = eventData.GetWorldPosition()
        actorBounds = self._polyData.GetBounds()
        isInBounds = (
                actorBounds[0] < pos[0] < actorBounds[1]
                and actorBounds[2] < pos[1] < actorBounds[3]
                and actorBounds[4] < pos[2] < actorBounds[5]
        )
        distance2 = vtkMath.Distance2BetweenPoints(pos, self._polyData.GetCenter())
        return isInBounds, distance2

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        """
        Triggered when the pipeline can process the interaction and is at the top of the priority list.
        default behavior: does nothing and returns false.

        :param eventData: The MRML event needing to be processed
        :return: True if event was processed. False otherwise (default = false)
        """

        self.GetDisplayNode().SetAttribute("IsSelected", str(1))
        return True

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData | None) -> None:
        """
        Triggered when the pipeline had focus (processed an interaction) and loses the focus (other pipeline
        handled the new interaction or window leave event).
        default behavior: does nothing.
        :param eventData: Optional event data which triggered the lose focus
        """
        super().LoseFocus(eventData)
        self.GetDisplayNode().SetAttribute("IsSelected", str(0))
```

### Defining custom interaction event translation across pipelines

For complex interactions, it is recommended to use
either [vtkMRMLLayerDMWidgetEventTranslationNode objects](https://github.com/KitwareMedical/SlicerLayerDisplayableManager/blob/main/LayerDM/MRML/vtkMRMLLayerDMWidgetEventTranslationNode.h)
or [vtkMRMLAbstractWidget objects](https://github.com/Slicer/Slicer/blob/main/Libs/MRML/DisplayableManager/vtkMRMLAbstractWidget.h).

The vtkMRMLLayerDMWidgetEventTranslationNode are compatible with scene exchange and can be used as an easy way to define
and customize interactions for given pipelines.

The easiest way to register a TL node is using the LayerDM logic class.
The logic class can register singleton TL nodes which will not be saved by the scene to provide the default expected TL
behavior.

The TL node is attached to each display node as a node reference. The recommended pattern is to let the pipeline attach
the shared TL node singleton to its display node at initialization, unless the display node already has one (so that a
per-node customization is never overwritten):

```python
import slicer
from slicer import vtkSlicerLayerDMLogic


def configureTLNode(tl_node):
    """Define the default event translations of the TL node (see the available translation methods below)."""
    tl_node.SetTranslation(
        vtkMRMLAbstractWidget.WidgetStateAny,
        vtkCommand.LeftButtonReleaseEvent,
        vtkMRMLAbstractWidget.WidgetEventUser,
    )


class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def SetDisplayNode(self, displayNode):
        super().SetDisplayNode(displayNode)
        self._createDefaultEventTranslation(displayNode)

    @staticmethod
    def _createDefaultEventTranslation(displayNode):
        """Attach the shared TL node singleton to the display node, unless it already has a TL node attached."""
        if displayNode is None or vtkSlicerLayerDMLogic.GetWidgetEventTranslationNode(displayNode) is not None:
            return

        # Get or lazily create and configure the TL node singleton shared by every pipeline instance
        tl_node = vtkSlicerLayerDMLogic.GetWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton")
        if tl_node is None:
            tl_node = vtkSlicerLayerDMLogic.CreateWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton")
            configureTLNode(tl_node)

        vtkSlicerLayerDMLogic.SetWidgetEventTranslationNode(displayNode, tl_node)
```

With this pattern, every display node automatically gets the shared TL node when its pipeline is created. To customize
the interactions of one specific display node, attach a different TL node to it with `SetWidgetEventTranslationNode`
before the pipeline is created (or at any time afterwards): the pipeline will leave the existing reference untouched.
The TL node attached to a display node can be retrieved with `GetWidgetEventTranslationNode(displayNode)`.

```{note}
In C++, the `vtkSlicerLayerDMLogic::CreateDefaultEventTranslation(displayNode, singletonId, configureF)` helper
performs the attach-if-missing and get-or-create steps above in a single call. It is not available in Python because
its configure callback parameter (`std::function`) cannot be wrapped.
```

The TL node provides the following event translation methods:

```python
# Click event translation
tl_node.SetTranslation(
    vtkMRMLAbstractWidget.WidgetStateAny,
    vtkCommand.LeftButtonReleaseEvent,
    vtkMRMLAbstractWidget.WidgetEventUser,
)

# Click drag event translation
tl_node.SetTranslationClickAndDrag(
    vtkMRMLAbstractWidget.WidgetStateOnWidget,
    vtkCommand.LeftButtonPressEvent,
    dragging_state,
    start_event,
    end_event,
)

# Keyboard events
tl_node.SetTranslationKeyboard(
    vtkMRMLAbstractWidget.WidgetStateIdle,
    "Delete",
    vtkMRMLAbstractWidget.WidgetEventReset,
)
```

In the pipeline, during the can process and process interaction methods, the TL node can be used to translate the
incoming event data.

```{note}
The pipelines don't have builtin widget state. The widget should be managed internally by the pipeline if needed.
State values should reuse the vtkMRMLAbstractWidget enum for compatibility with the other displayable managers.
```

```python
class MyPipeline(vtkMRMLLayerDMScriptedPipeline):
    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        widgetEvent = self.tl_node.Translate(self.widgetState, eventData)
        if widgetEvent == vtkMRMLAbstractWidget.WidgetEventNone:
            return False, sys.float_info.max

        # Compute representative distance to event
        return True, my_distance

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        widgetEvent = self.tl_node.Translate(self.widgetState, eventData)
        if widgetEvent == vtkMRMLAbstractWidget.WidgetEventTranslateStart:
            self.widgetState = vtkMRMLAbstractWidget.WidgetStateTranslate
            return self.StartTranslate(eventData)

        # ...
        return True
```

## Registration logic

To register our pipeline, we need two objects:

* The pipeline creator: Responsible for creating the pipeline depending on input view node / display node pair
* The pipeline factory singleton: Used by de display manager and responsible for storing pipeline creator instances

The pipeline creator `SetCallback` method can be used to define the callback which should be called when a new node is
added to the scene.

The callback should then:

* Create a new pipeline instance if the pipeline can handle the new view node / node input pair
* Return null if the creator cannot create a new pipeline

The factory will iterate over the different creator instances until it can find one that can create the pipeline. It
case it finds nothing, no new pipeline will be created for the new node.

```python
def tryCreate(viewNode, node):
    if not isinstance(node, MyNodeType) or not isinstance(viewNode, vtkMRMLViewNode):
        return None

    return MyPipeline()


pipeline_creator = vtkMRMLLayerDMPipelineScriptedCreator()
pipeline_creator.SetPythonCallback(MyPipeline.TryCreate)
vtkMRMLLayerDMPipelineFactory.GetInstance().AddPipelineCreator(pipeline_creator)
```

```{note}
A convenience template is provided in C++ to simplify instantiating new pipelines based on a view, node type checking.

If the view matches a given type and node matches a given type, then a pipeline of given type is created.

This method supports variadic templating and makes for easy registration.
```

Example of registration for one type :

```cpp
creator = vtkMRMLLayerDMPipelineFactory::GetInstance()->AddPipelineCreator(
      [](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* displayNode)
      {
        return layer_dm::TryCreateForView<vtkMRMLViewNode, MyDisplayNode, MyPipelineNode();
      });
```

Example of registration for one view type with multiple display node / display pipeline pairs :

```cpp
#include "vtkMRMLLayerDMPipelineCreateHelper.h"

void vtkMRMLCPRPipelineCreatorLogic::RegisterPipelines()
{
  static vtkSmartPointer<vtkMRMLLayerDMPipelineCreator> creator{ nullptr };

  if (!creator)
  {
    creator = vtkMRMLLayerDMPipelineFactory::GetInstance()->AddPipelineCreator(
      [](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* displayNode)
      {
        return layer_dm::TryCreateForView< //
          vtkMRMLCPRViewNode,
          vtkMRMLVolumeCPRViewDisplayNode,
          vtkMRMLVolumeCPRViewPipeline,
          vtkMRMLModelCPRViewDisplayNode,
          vtkMRMLModelCPRViewPipeline,
          vtkMRMLSliceCPRViewDisplayNode,
          vtkMRMLSliceCPRViewPipeline,
          vtkMRMLCPRCameraNode,
          vtkMRMLCPRCameraPipeline>(viewNode, displayNode);
      });
  }
}
```

### Sharing logic objects between pipelines

When pipelines are created, one pipeline instance is created per view as per the displayable manager architecture. If
pipelines need to share common logic instances, this instance can easily be set at creation using the pipeline creator.

The code snippet below shows an example of such logic in Python / C++:

```python
my_logic = MyLogic()


def tryCreate(viewNode, node):
    if not isinstance(node, MyNodeType) or not isinstance(viewNode, vtkMRMLViewNode):
        return None

    pipeline = MyPipeline()
    pipeline.SetLogic(my_logic)
    return pipeline


pipeline_creator = vtkMRMLLayerDMPipelineScriptedCreator()
pipeline_creator.SetPythonCallback(tryCreate)
vtkMRMLLayerDMPipelineFactory.GetInstance().AddPipelineCreator(pipeline_creator)
```

```cpp
  static vtkSmartPointer<vtkMRMLLayerDMPipelineCreator> creator{ nullptr };

  if (!creator)
  {
    creator = vtkMRMLLayerDMPipelineFactory::GetInstance()->AddPipelineCreator(
      [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* displayNode)
      {
        auto pipeline = layer_dm::TryCreateForView<vtkMRMLViewNode, MyDisplayNode, MyPipelineNode>(viewNode, displayNode));
        if(!pipeline)
        {
          return nullptr;
        }

        pipeline->SetLogic(this->m_myLogic);
        return pipeline;
      });
  }
```
