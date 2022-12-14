# Cameras

## Overview

Bring core support for multiple cameras and 3D views in Slicer.

The multi views and cameras framework is available from the "Camera" modules. This module displays two pull-down menus. The first one, "View", lists all available views and lets the user create new 3D views. The second one, "Camera", lists all available cameras, and lets the user create new cameras.

Only one camera is used by a view at a time. When a view is selected from the pull-down, the camera it is currently using is automatically selected in the second pull-down.

- **Create a new camera:** In the "Camera" pull-down menu, select "Create New Camera": a new camera node is created (most likely named "Camera1", as opposed to the default "Camera" node), and automatically assigned to the currently selected view node (named "View" by default). Try interacting with the 3D view.
- **Assign a camera to a view:** Select any camera node from the "Camera" pull-down menu: the camera is assigned to the currently selected view. For example, try selecting back the "Camera" node if you have created a "Camera1" node in the previous step, and you should notice the 3D view on the right update itself to reflect the different point of view. Note: a camera can not be shared between two views: selecting a camera already used by a view will effectively swap the cameras between the two views.
- **Create a new view:** Select the "Tabbed 3D Layout" from the layout button in the toolbar. In the "View" pull-down menu, select "Create New View": a new view node is created (most likely named "View1", as opposed to the default "View" node), and displayed on the right under a new tab. You can select which view to display by clicking on the corresponding tab in the "Tabbed 3D Layout". Interacting in that view will automatically mark it as "active"; there can only be one "active" view at a time in Slicer, and it always feature a thin dark blue border. Since a view can not exist without a camera, a new camera node is automatically created and assigned to this new view (most likely named "Camera2" if you have created "Camera1" in the previous steps). At this point, you can assign any of the previously created cameras to this new view (say, the "Camera" or "Camera1" nodes).

## Panels and their use

- **Cameras**: List the 3D views and cameras in the scene.
  - **View**: Select a 3D view. "View" is the main 3D view, additional 3D views are named "View_1", "View_2" etc. On selection, the view active camera is automatically selected in the Camera pull-down.
  - **Camera**: Active camera of the current view.

## Contributors

Julien Finet (Kitware), Sebastien Barr&eacute; (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
