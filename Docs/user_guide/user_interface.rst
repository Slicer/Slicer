==============
User Interface
==============

Application overview
-----------------------

Slicer stores all loaded data in a data repository, called the "scene" (or Slicer scene or MRML scene). Each data set, such as an image volume, surface model, or point set, is represented in the scene as a "node".

Slicer provides a large number "modules", each implementing a specific set of functions for creating or manipulating data in the scene. Modules typically do not interact with each other directly: they just all operate on the same data, which is stored in the scene. Slicer package contains over 100 built-in modules and additional modules can be installed by using the Extension Manager.

.. ApplicationMainWindow.png
.. figure:: https://discourse-cloud-file-uploads.s3.dualstack.us-west-2.amazonaws.com/standard17/uploads/slicer/original/2X/c/c1ec70cbdb07560278a650a1478a4d53c4b7eeaf.png

  Slicer application main window

Module Panel
^^^^^^^^^^^^

This panel (located by default on the left side of the application main window) displays all the options and features that the current module offers to the user. Current module can be selected using the **Module Selection** toolbar.

**Data Probe** is located at the bottom of the module panel. It displays information about view content at the position of the mouse pointer.
  
Views
^^^^^

Slicer displays data in various views. The user can choose between a number of predefined layouts, which may contain slice, 3D, chart, and table views.

The Layout Toolbar provides a drop-down menu of layouts useful for many types of studies. When Slicer is exited normally, the selected layout is saved and restored next time the application is started.

Application Menu
^^^^^^^^^^^^^^^^

- **File**: Functions for loading a previouly saved scene or individual datasets of various types, and for downloading sample datasets from the internet. An option for saving scenes and data is also provided here. **Add Data** allows loading data from files. **DICOM** module is recommended to import data from DICOM files and loading of imported DICOM data. **Save** opens the "Save Data" window, which offers a variety of options for saving all data or selected datasets.
- **Edit**: Contains an option for showing Application Settings, which allows users to customize appearance and behavior of Slicer, such as modules displayed in the toolbar, application font size, temporary directory location, location of additional Slicer modules to include.
- **View**: Functions for showing/hiding additional windows and widgets, such as **Extension Manager** for installing extensions from Slicer app store, **Error Log** for checking if the application encountered any potential errors, **Python Interactor** for getting a Python console to interact with the loaded data or modules, **show/hide toolbars**, or **switch view layout**.

Toolbar
^^^^^^^

Toolbar provides quick access to commonly used functions. Individual toolbar panels can be shown/hidden using menu: View / Toolbars section.

**Module Selection** toolbar is used for selecting the currently active "module". The toolbar provides options for searching for module names (``Ctrl`` + ``f`` or click on magnify glass icon) or selecting from a menu. **Module history** shows the list of recently used modules. **Arrow buttons** can be used for going back to/returning from previously used module.

**Favorite modules** toolbar contains a list of most frequently used modules. The list can be customized using menu: Edit / Application settings / Modules / Favorite Modules. Drag-and-drop modules from the Modules list to the Favorite Modules list to add a module.


Status bar
^^^^^^^^^^

This panel may display application status, such as current operation in progress. Clicking the little **X** icons displays the Erro Log window.


Interacting with views
----------------------

View Cross-Reference
^^^^^^^^^^^^^^^^^^^^^^

Holding down the ``Shift`` key while moving the mouse in any slice or 3D view will cause the Crosshair to move to the selected position in all views.
By default, when the Crosshair is moved in any views, all slice views are scrolled to the same RAS position indexed by the mouse. This feature is useful when inspecting.

To show/hide the Crosshair position, click crosshair icon |crosshair|.

.. |crosshair| image:: ../../Libs/MRML/Widgets/Resources/Icons/SlicesCrosshair.png

To customize behavior and appearance of the Crosshair, click the "down arrow" button on the right side of the crosshair icon.

Mouse Modes
^^^^^^^^^^^

Slicer has two mouse modes: **Transform** (which allows interactive rotate, translate and zoom operations), and **Place** (which permits objects to be interactively placed in slice and 3D views).

.. MouseModeToolbar.png
.. image:: https://discourse-cloud-file-uploads.s3.dualstack.us-west-2.amazonaws.com/standard17/uploads/slicer/original/2X/c/c838c6aa419885dcdd249b76248f8d5087c192b6.png

The toolbar icons that switch between these mouse modes are shown from left to right above, respectively. Place Fiducial is the default place option as shown above; options to place both Ruler and Region of Interest Widgets are also available from the drop-down Place Mode menu. 

.. note::

  Transform mode is the default interaction mode. By default, Place mode persists for one "place" operation after the 
  Place Mode icon is selected, and then the mode switches back to Transform. Place mode can be made persistent (useful 
  for creating multiple fiducial points, rulers, etc.) by checking the Persistent checkbox shown rightmost in the Mouse 
  Mode Toolbar.


3D View
^^^^^^^

Displays a rendered 3D view of the scene along with visual references to specify orientation and scale.

Default orientation axes: A = anterior, P = posterior, R = right, L = left, S = superior and I = inferior.

.. ViewControls3D.png
.. image:: https://aws1.discourse-cdn.com/standard17/uploads/slicer/original/2X/4/4785bc866024231f3dc8865aefb6935da115ef72.png

3D View Controls: The blue bar across any 3D View shows a pushpin icon on its left. When the mouse rolls over this icon, a panel for configuring the 3D View is displayed. The panel is hidden when the mouse moves away. For persistent display of this panel, just click the pushpin icon. 

Slice View
^^^^^^^^^^

Three default slice views are provided (with Red, Yellow and Green colored bars) in which Axial, Saggital, Coronal or Oblique 2D slices of volume images can be displayed. Additional generic slice views have a grey colored bar and an identifying number in their upper left corner.

.. ViewControlsSlice.png
.. image:: https://discourse-cloud-file-uploads.s3.dualstack.us-west-2.amazonaws.com/standard17/uploads/slicer/original/2X/f/f39d6106526fa3b090d12258c9387ab2ab12735c.png

Slice View Controls: The colored bar across any Slice View shows a pushpin icon on its left. When the mouse rolls over this icon, a panel for configuring the slice view is displayed. The panel is hidden when the mouse moves away. For persistent display of this panel, just click the pushpin icon. For more options, click the double-arrow icon.

View Controllers module provides an alternate way of displaying these controllers in the Module Panel.


Chart View
^^^^^^^^^^

Table View
^^^^^^^^^^



Mouse & Keyboard Shortcuts
---------------------------------

Generic shortcuts
^^^^^^^^^^^^^^^^^

+------------------------------------------+-----------------------------------------+
| Shortcut                                 | Operation                               |
+==========================================+=========================================+
| ``Ctrl`` + ``f``                         | find module by name                     |
|                                          | (hit ``Enter`` to select)               |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``a``                         | add data from file                      |
+------------------------------------------+                                         |
| ``Ctrl`` + ``o``                         |                                         |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``s``                         | save data to files                      |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``w``                         | close scene                             |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``0``                         | show Error Log                          |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``1``                         | show Application Help                   |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``2``                         | show Application Settings               |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``3``                         | show/hide Python Interactor             |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``4``                         | show Extension Manager                  |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``5``                         | show/hide Module Panel                  |
+------------------------------------------+-----------------------------------------+
| ``Ctrl`` + ``h``                         | open default startup module             |
|                                          | (configurable in Application Settings)  |
+------------------------------------------+-----------------------------------------+

Slice views
^^^^^^^^^^^

The following shortcuts are available when a slice view is active.
To activate a view, click inside the view: if you do not want to change anything
in the view, just activate it then do ``right-click`` without moving the mouse.
Note that simply hovering over the mouse over a slice view will not activate the view.

+------------------------------------------+-----------------------------------------+
| Shortcut                                 | Operation                               |
+==========================================+=========================================+
| ``right-click`` + ``drag up/down``       | zoom image in/out                       |
+------------------------------------------+                                         |
| ``Ctrl`` + ``mouse wheel``               |                                         |
+------------------------------------------+-----------------------------------------+
| ``middle-click`` + ``drag``              | pan (translate) view                    |
+------------------------------------------+                                         |
| ``Shift`` + ``left-click`` + ``drag``    |                                         |
+------------------------------------------+-----------------------------------------+
| ``left arrow`` / ``right arrow``         | move to previous/next slice             |
+------------------------------------------+                                         |
| ``b`` / ``f``                            |                                         |
+------------------------------------------+-----------------------------------------+
| ``Shift`` + ``mouse move``               | move crosshair in all views             |
+------------------------------------------+-----------------------------------------+
| ``v``                                    | toggle slice visibility in 3D view      |
+------------------------------------------+-----------------------------------------+
| ``r``                                    | reset zoom and pan to default           |
+------------------------------------------+-----------------------------------------+
| ``g``                                    | toggle segmentation or labelmap volume  |
|                                          | visibility                              |
+------------------------------------------+-----------------------------------------+
| ``t``                                    | toggle foreground volume visibility     |
+------------------------------------------+-----------------------------------------+
| ``[`` / ``]``                            | use previous/next volume as background  |
+------------------------------------------+-----------------------------------------+
| ``{`` / ``}``                            | use previous/next volume as foreeround  |
+------------------------------------------+-----------------------------------------+

3D views
^^^^^^^^

The following shortcuts are available when a 3D view is active.
To activate a view, click inside the view: if you do not want to change anything
in the view, just activate it then do ``right-click`` without moving the mouse.
Note that simply hovering over the mouse over a slice view will not activate the view.

+------------------------------------------+-----------------------------------------+
| Shortcut                                 | Operation                               |
+==========================================+=========================================+
| ``Shift`` + ``mouse move``               | move crosshair in all views             |
+------------------------------------------+-----------------------------------------+
| ``left-click`` + ``drag``                | rotate view                             |
+------------------------------------------+                                         |
| ``left arrow`` / ``right arrow``         |                                         |
+------------------------------------------+                                         |
| ``up arrow`` / ``down arrow``            |                                         |
+------------------------------------------+-----------------------------------------+
| ``End`` or ``Keypad 1``                  | rotate to view from anterior            |
+------------------------------------------+-----------------------------------------+
| ``Shift`` + ``End`` or                   | rotate to view from posterior           |
| ``Shift`` + ``Keypad 1``                 |                                         |
+------------------------------------------+-----------------------------------------+
| ``Page Down`` or ``Keypad 3``            | rotate to view from left side           |
+------------------------------------------+-----------------------------------------+
| ``Shift`` + ``Page Down``                | rotate to view from right side          |
| ``Shift`` + ``Keypad 3`` or              |                                         |
+------------------------------------------+-----------------------------------------+
| ``Home`` or ``Keypad 7``                 | rotate to view from superior            |
+------------------------------------------+-----------------------------------------+
| ``Shift`` + ``Home``                     | rotate to view from inferior            |
| ``Shift`` + ``Keypad 7`` or              |                                         |
+------------------------------------------+-----------------------------------------+
| ``right-click`` + ``drag up/down``       | zoom view in/out                        |
+------------------------------------------+                                         |
| ``Ctrl`` + ``mouse wheel``               |                                         |
+------------------------------------------+                                         |
| ``+`` / ``-``                            |                                         |
+------------------------------------------+-----------------------------------------+
| ``middle-click`` + ``drag``              | pan (translate) view                    |
+------------------------------------------+                                         |
| ``Shift`` + ``left-click`` + ``drag``    |                                         |
+------------------------------------------+                                         |
| ``Shift`` + ``left arrow`` /             |                                         |
| ``Shift`` + ``right arrow``              |                                         |
+------------------------------------------+                                         |
| ``Shift`` + ``up arrow`` /               |                                         |
| ``Shift`` + ``down arrow``               |                                         |
+------------------------------------------+                                         |
| ``Shift`` + ``Keypad 2`` /               |                                         |
| ``Shift`` + ``Keypad 4``                 |                                         |
+------------------------------------------+                                         |
| ``Shift`` + ``Keypad 6`` /               |                                         |
| ``Shift`` + ``Keypad 8``                 |                                         |
+------------------------------------------+-----------------------------------------+
| ``Keypad 0`` or ``Insert``               | reset zoom and pan, rotate to nearest   |
|                                          | standard view                           |
+------------------------------------------+-----------------------------------------+

.. note::

  Simulation of shortcuts not available on your device:

  - One-button mouse: instead of ``right-click`` do ``Ctrl`` + ``click``
  - Trackpad: instead of ``right-click`` do ``two-finger click``
