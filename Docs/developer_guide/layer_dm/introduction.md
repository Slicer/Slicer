# Introduction

The LayerDM (Layer Displayable Manager) module simplifies the creation of custom rendering and interaction behavior
in slice and 3D views. Its displayable manager is automatically registered in all default slice and 3D views,
so custom display pipelines can be registered from C++ or Python without any additional setup.

## What it provides

Implementing a displayable manager requires creating and connecting a significant number of components: a MRML
node to store display information, a displayable manager class that builds VTK pipelines for each view, potentially
additional renderers for overlay display, VTK widgets and representations for event handling, and the observer logic that keeps
all of these synchronized with the scene. This has traditionally restricted displayable manager development to expert
C++ developers.

The LayerDM module removes most of this boilerplate. Developers implement a single *pipeline* class per display
behavior, and the module takes care of:

* [Pipeline lifecycle management](module_architecture.md#pipeline-lifecycle)
  * Automatic creation / removal of pipeline instances (one per view) driven by nodes added to / removed from the scene
  * Automatic handling of scene import, scene close, and view creation / destruction
* Rendering infrastructure
  * [Renderer layer creation / removal](module_architecture.md#render-layer-handling), with
    [explicit render ordering](getting_started.md#rendering-on-top-of-other-actors) to display content on top of
    (or below) other actors
  * [Camera synchronization](getting_started.md#monitoring-camera-changes) between renderer layers and slice / 3D views
  * [Render requests driven by observed object events](getting_started.md#refreshing-the-display)
* [Interaction handling](getting_started.md#processing-interactions)
  * Interaction events are delivered directly to the pipeline, with focus handling and priorities between competing
    widgets [already arbitrated](module_architecture.md#interaction-handling)
  * Mouse and keyboard events can be mapped to widget actions using
    [event translation nodes](getting_started.md#defining-custom-event-translation-across-pipelines), without writing
    custom event parsing code
* Pipeline registration and implementation
  * A pipeline is [registered with a single callback](getting_started.md#registration-logic) that decides, from the
    view node and display node types, whether the pipeline applies
  * [Pipelines](getting_started.md#pipeline) can be written in C++ or entirely in Python
  * Convenience methods for [observing node and object events](getting_started.md#monitoring-node-changes), and access
    to sibling pipelines for coordinated behaviors

## When to use it

LayerDM is the recommended way to add custom per-node rendering or custom interactive widgets to Slicer views.
Use it when:

* You want to display custom VTK actors in slice or 3D views, driven by nodes in the scene (created / removed
  automatically as nodes are added / removed, and shown consistently in every view).
* You want to render content as an overlay, on top of or below other actors, without managing renderers and cameras
  yourself.
* You want to implement custom mouse / keyboard interactions that cooperate correctly with the built-in widgets
  (markups, segmentation tools, camera manipulation), including focus handling and event priorities.
* You want to implement any of the above in Python, which is not possible with the classic displayable manager
  architecture.

Simpler alternatives remain appropriate for simpler needs:

* If existing node types already provide the display and interaction you need, compose them instead: for example,
  [use markups control points for interactivity](../script_repository/markups.md#specify-a-sphere-by-multiple-control-points).
  This requires no custom rendering code at all.
* For quick experiments, you can
  [access the VTK render window directly and inject actors](../script_repository/gui.md#access-vtk-rendering-classes).
  This is fine for prototyping, but the injected actors are not managed: they are not synchronized with the scene, do
  not participate in interaction handling, and must be manually added to each view. Prefer LayerDM for anything
  intended to be maintained or distributed.
* Implementing a classic displayable manager from scratch in C++ is rarely necessary for new code; it remains relevant
  mainly when modifying Slicer's existing core displayable managers.

To get started with writing your first pipeline, continue to the [getting started section](getting_started.md).
