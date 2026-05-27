# Displayable Manager architecture reminder(s)

The library is integrated in the displayable manager system. To properly understand its behavior and connection
to the existing architecture, a summary of the displayable manager architecture and behavior is done in this section.

Developers already familiar with this system can skip to the next documentation section.

Additional information can be found in the [MRML Overview section](../mrml_overview.md#the-displayable-manager).

## Main components

3D Slicer's rendering is handled by:

* Layout manager: Responsible for creating the application Qt layout based on an XML description.
* (Qt) views: Views are based on the CTK abstract view and populate the layout manager
* Displayable manager group: DM groups are set to the views and handle
* Displayable managers: A collection of displayable managers living in the DM group
* (VTK) Render Window: VTK handles the low-level rendering. Interactions / actor creation / etc. are dispatched
  from / to Slicer by the displayable manager system.

## Rendering initialization

The diagram below presents the view initialization sequence:

```{eval-rst}
.. mermaid:: diagrams/dm_initialization_sequence.mmd
   :align: center
```

The layout manager is responsible for creating new view data nodes when switching from one layout to another.
View data nodes are singleton nodes with unique IDs. When the view nodes are not yet present in the scene, the layout
manager will create them. For each created node type, it will use the associated factory responsible for instantiating
the underlying Qt widgets.

During the creation of the Qt widgets, each widget will create a new instance of displayable manager group and one
instance of each displayable manager registered in their factory singleton instance.

Thanks to this mechanism, the displayable managers present in the views are independent from one-another and each widget
will have their instance of each displayable manager.

This initialization implies that little connection of the displayable manager is done with other application components.
The displayable managers will have access to:

* The Scene
* The application logic
* The parent widget's default vtkRenderer instance (and by extension its render window)
* The parent widget's view node instance

By convention, one displayable manager will be responsible with one type of node type although some displayable managers
may contain other linked responsibilities to ease implementation.

By convention, most displayable managers are reactive to display node types which contain only representation
information of data nodes to which they are attached.

## Relationship between data nodes / display nodes / representation

The diagram below shows the relationship between a data node, its display node and the other components described
before.

```{eval-rst}
.. mermaid:: diagrams/dm_model_node_colaboration.mmd
   :align: center
```

In this example, changes to the either the source data (polydata / connectivity) or its display properties (colors /
opacity / ...) will trigger an update in the VTK pipeline.

## Reactivity handling

To operate normally, the displayable manager needs to properly handle changes happening both in the scene, the
view and the display node it is currently displaying in its VTK renderer.

The scene events can be handled thanks to the following abstract methods present in the abstract displayable manager
class :

* OnMRMLSceneNodeAdded(vtkMRMLNode* node): Watch to add a widget / representation for given nodes
* OnMRMLSceneNodeRemoved(vtkMRMLNode* node): Watch to remove a widget / representation for a given node on delete
* OnMRMLSceneEndImport(): Watch for an MRB file that has finished importing.
* OnMRMLSceneEndClose(): Watch to clean up after a scene close.

In addition to these events, the following data events need to be manually observed at the pipeline or displayable
level:

* Camera changes
* View node changes
* Display node changes
* Data node changes

The type of event to observe and the reaction to the events will depend on the specifics of the displayable manager.

## Interaction handling

The previous sections detailed how a representation is created and displayed in the views with the displayable manager
system. In this section, we will have a rapid look at the interaction system.

At low level, the user interactions are intercepted by the VTK layer. To simplify the customization and complex handling
of user interactions, the interactions are forwarded to the displayable managers through the vtkMRMLViewInteractorStyle.
The interaction logic is summarized in the diagram below:

```{eval-rst}
.. mermaid:: diagrams/dm_interaction_sequence.mmd
   :align: center
```

The displayable manager provides the following methods to handle interactions :

* bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2): Return true + distance to
  the event if the displayable manager can process the interaction event present at given coordinates.
* bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData): Return true / false and process the actual
  event if the displayable manager was selected (more on that below).
* void LoseFocus(vtkMRMLInteractionEventData* eventData): To handle a switch to another displayable manager if the
  displayable manager was handling prior events and another displayable manager was chosen for newer events.
