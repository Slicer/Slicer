==============
User Interface
==============

Application Menu
----------------

Slicer's Application Menu contains many useful features. A brief overview of some of these is given below.

The **File Menu** contains options for loading a MRML scene, for downloading sample datasets from the internet, or individual datasets of various types. An option for saving scenes and data is also provided here.

The **Edit menu** contains an option to raise the Application Settings Interface. This interface allows users to customize their installation of Slicer. 'Edit->Application Settings can be used to include or suppress various Slicer modules, to customize the appearance of your Python Interactor, to select a custom font, to specify a temporary directory, and to accomplish other customizations as well.

The **View menu** contains options to raise the Python Interactor and the Error Log, both useful developer tools. Menu options to switch layouts, and to toggle the display of individual toolbars and the module GUI panels are also provided under the View Menu.

.. figure:: https://www.slicer.org/w/images/c/c6/Slicer4AppMenu.jpg

  File, Edit and View menus


Toolbar
-------

The Toolbar provides shortcuts to many useful sets of functionality. The following toolbars appear here by default. Customization is possible from the Slicer View Menu.

Load & Save Options
^^^^^^^^^^^^^^^^^^^

The **Load Scene** option will raise a browser to select and load a collection of data as a MRML scene. This option clears the existing scene and resets the application state.

The **Save** option will raise the "Save Data" Interface which offers a variety of options for saving MRML scenes and individual datasets. 

Module Selection & Navigation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Module Selection & Navigation toolbar provides options for searching for module names, selecting from a module menu, and moving backward and forward through the module selection history for convenience.

Core Modules
^^^^^^^^^^^^

The Core Module Toolbar provides shortcuts to your "Home" module, and some of Slicer's core modules including:

* Data
* Volumes
* Models
* Transforms
* Markups
* Segment Editor

Layouts
^^^^^^^

The Layout Toolbar provides a drop-down menu of layouts useful for many types of studies. When Slicer is exited normally, the selected layout is saved and restored as part of the Application state. 

.. image:: https://www.slicer.org/w/images/8/82/Slicer4Layouts.jpg


Mouse Modes
^^^^^^^^^^^

Slicer has two mouse modes: **Transform** (which allows interactive rotate, translate and zoom operations), and **Place** (which permits objects to be interactively placed in 3D or 2D).

.. image:: https://www.slicer.org/w/images/7/7b/Slicer4MouseModeToolbar.jpg

The toolbar icons that switch between these mouse modes are shown from left to right above, respectively. Place Fiducial is the default place option as shown above; options to place both Ruler and Region of Interest Widgets are also available from the drop-down Place Mode menu. 

.. note::

  Transform mode is the default interaction mode. By default, Place mode persists for one "place" operation after the 
  Place Mode icon is selected, and then the mode switches back to Transform. Place mode can be made persistent (useful 
  for creating multiple fiducial points, rulers, etc.) by checking the Persistent checkbox shown rightmost in the Mouse 
  Mode Toolbar. 

Screenshots & Scene Views
^^^^^^^^^^^^^^^^^^^^^^^^^

Crosshair Options
^^^^^^^^^^^^^^^^^

Module Panel
------------

Data Probe
----------

3D Viewer
---------

Slice Viewers
-------------

Chart Viewers
-------------

Mouse & Keyboard Across Platforms
---------------------------------

Adjusting Window & Level on All Platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Selecting & Manipulating on All Platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Viewer Cross^Reference on All Platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Zoom In & Out in Slice & 3D Viewers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Pan (Translate) in Slicer & 3D Viewers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Rotate in 3D Viewers
^^^^^^^^^^^^^^^^^^^^

