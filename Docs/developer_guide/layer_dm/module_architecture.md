# Module architecture

## Layer DM classes

The LayerDisplayableManager is the main entry point for the classes in this module.
It's automatically registered in the default 3D Slicer SliceViews and ThreeDViews as a normal displayable manager when
the module is first setup using conventional 3D Slicer mechanisms.

Internally, the Layer displayable manager delegates to the following objects.

* pipeline manager: used for the lifetime of the created pipelines.
* layer manager: used for the creation / deletion of the renderers added to the render window and managed by the layer
* camera synchronizer: used for camera synchronization and callback mechanism linked to the default view camera.

```{tip}
From a user standpoint, only the following classes need to be manipulated directly:

* The pipeline: either implemented in Python or in C++
* The pipeline creator: Responsible for creating the pipeline through a user set callback
* The pipeline factory: Accessible through a singleton and used automatically by the displayable manager
```

The diagram below summarizes the relationship between the different classes.

```{eval-rst}
.. mermaid:: diagrams/layer_dm_collaboration.mmd
   :align: center
```

The main classes of the library and their responsibilities are summarized below :

| Class                                    | Description                                                                                  |
|------------------------------------------|----------------------------------------------------------------------------------------------|
| vtkMRMLLayerDMPipelineI                  | Interface for display pipelines. Handles interaction, rendering, camera, and observer logic. |
| vtkMRMLLayerDisplayableManager           | Main displayable manager. Initializes pipeline manager and delegates scene updates.          |
| vtkMRMLLayerDMCameraSynchronizer         | Synchronizes default camera with renderer or slice node state.                               |
| vtkMRMLLayerDMLayerManager               | Manages renderer layers based on pipeline layer/camera pairs.                                |
| vtkMRMLLayerDMPipelineCreatorI           | Interface for pipeline creation. Supports custom instantiation logic.                        |
| vtkMRMLLayerDMPipelineCallbackCreator    | Callback-based implementation of pipeline creator.                                           |
| vtkMRMLLayerDMPipelineScriptedCreator    | Python lambda-based pipeline creator.                                                        |
| vtkMRMLLayerDMPipelineFactory            | Singleton factory for pipeline instantiation and registration.                               |
| vtkMRMLLayerDMPipelineManager            | Manages pipeline lifecycle, layer manager, and camera sync.                                  |
| vtkMRMLLayerDMScriptedPipelineBridge     | Python bridge for virtual method delegation.                                                 |
| vtkMRMLLayerDMScriptedPipeline           | Python abstract class for scripted pipelines.                                                |
| vtkMRMLLayerDMWidgetEventTranslationNode | MRML node providing interactions to widget event map.                                        |
| vtkMRMLLayerDMNodeReferenceObserver      | Monitors scene for reference changes to trigger pipeline update.                             |
| vtkSlicerLayerDMLogic                    | Module's logic class providing helper functionalities to manage display / TL nodes.          |

## Pipeline lifecycle

The sequence diagram below shows a typical pipeline lifecycle as handled by the library:

```{eval-rst}
.. mermaid:: diagrams/layer_dm_pipeline_lifecycle_sequence.mmd
   :align: center
```

## Interaction handling

The diagram below shows how the LayerDM handles interactions :

```{eval-rst}
.. mermaid:: diagrams/layer_dm_interaction_sequence.mmd
   :align: center
```

During interactions, the can process / process and lose focus events are forwarded to the interaction logic class
responsible for handling the events.

The interaction logic will forward the events to the underlying pipelines.

Pipelines that can process the interaction will be sorted using a tuple of three values:

* Pipeline widget state: if its value is greater than hovered indicating and active user interaction with the pipeline
* Pipeline render order: greater values indicate that pipelines are rendered over other pipelines
* Pipeline distance to interaction

This strategy allows for intuitive user interactions depending on the type of widgets they are manipulating.

## Render layer handling

In VTK, a render window can contain an arbitrary number of renderers. For each renderer, developers define their render
layer, which will control the rendering order of the different renderers. Top most renderer layers drawn elements will
be drawn at the top of other elements.

In 3D Slicer's displayable managers, some displayable managers such as
the [vtkMRMLRulerDisplayableManager](https://github.com/Slicer/Slicer/blob/main/Libs/MRML/DisplayableManager/vtkMRMLRulerDisplayableManager.cxx#L177)
use this strategy to render on top of other elements.

In the module, the renderers are not manipulated directly by developers in their pipelines but automatically managed
by the LayerManager class.

From a developer standpoint, only the `GetRenderOrder` value needs to be returned. This value is a static value read
when new pipelines are added / removed from the pipeline manager.

Pipelines with the same GetRenderOrder and the same GetCustomCamera will be grouped in the same renderer layer. If the
value is set to 0, the pipelines will be set to the default renderer layer.

The grouping logic is summarized below:

```{eval-rst}
.. mermaid:: diagrams/layer_dm_layer_manager_logic.mmd
   :align: center
```

## Camera Synchronizer

Most displayable managers follow the main camera present in the render window. This allows actors / widgets to be
updated at the right position.

If a pipeline doesn't use a specific camera, its renderer layer will be set the default camera (i.e., the camera set on
renderer 0). The default synchronization behavior is set to copy the default camera and notify the pipelines of camera
update using the `OnDefaultCameraModified` method calls.

For SliceViews, the camera synchronizer monitors modified events to set the default camera aligned with the Slice view
properties. This allows pipelines to define and update actors directly in 3D and avoid having to do manual conversions
between screen and RAS space.

Another added value is that 3D actors can benefit from all VTK features, including antialiasing, which is not the
case when using 2D actors which is currently used in SliceViews.

## Node reference updates

vtkMRML nodes provide a referencing mechanism. This mechanism is, for instance, used to register nodes as display nodes
to other nodes.

Pipelines are associated to a given `Display Node` (note: the node doesn't need to inherit from vtkMRMLDisplayNode) when
nodes in the scene add references to the display node, the pipeline's `OnReferenceToDisplayNodeAdded` method will be
triggered.

Similarly, when a reference is removed, the pipeline's `OnReferenceToDisplayNodeRemoved` method will be triggered.

By default, these calls will be dispatched to the `OnUpdate` method with the display node as the target vtkObject and
`vtkMRMLNode::ReferenceAddedEvent` / `vtkMRMLNode::ReferenceRemovedEvent` event Ids.

## Event translation

Although not used internally, the library provides the `vtkMRMLLayerDMWidgetEventTranslationNode` MRML node to help in
converting processing event data to significant widget events.

Although with a different API, its usage follows the `vtkMRMLAbstractWidget` event translation mechanism.

The implementation was split to provide:

* Direct access from Python
* Possible access from the Scene to customize the interaction events
