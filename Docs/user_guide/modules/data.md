# Data

## Overview

Data module shows all data sets loaded into the scene and allows modification of basic properties and perform common operations on all kinds of data, without switching to other modules.

- `Subject Hierarchy` tab shows selected nodes in a freely editable folder structure.
- `Transform Hierarchy` tab shows data organized by what transforms are applied to them.
- `All Nodes` tab shows all nodes in simple list. This is intended for advanced users and troubleshooting.

In Subject Hierarchy, DICOM data is automatically added as patient-study-series hierarchy. Non-DICOM data can be parsed if loaded from a local directory structure, or can be manually organized in tree structure by creating DICOM-like hierarchy or folders.

Subject hierarchy provides features for the underlying data nodes, including cloning, bulk transforming, bulk show/hide, type-specific features, and basic node operations such as delete or rename. Additional plugins provide other type-specific features and general operations, see [Subject hierarchy labs page](https://www.slicer.org/wiki/Documentation/Labs/SubjectHierarchy).

-  Subject hierarchy view
    -  Overview all loaded data objects in the same place, types indicated by icons
    -  Organize data in folders or patient/subject trees
    -  Visualize and bulk-handle lots of data nodes loaded from disk
    -  Easy show/hide of branches of displayable data
    -  Transform whole study (any branch)
    -  Export DICOM data (edit DICOM tags)
    -  Lots of type-specific functionality offered by the plugins
-  Transform hierarchy view
    -  Manage transformation chains/hierarchies
-  All nodes view
    -  Developer tool for debugging problems

## How to

### Create new Subject from scratch

Right-click on the empty area and select 'Create new subject'

### Create new folder

Right-click on an existing item or the empty area and select 'Create new folder'. Folder type hierarchy item can be converted to Subject or Study using the context menu

### Rename item

Right-click on the node and select 'Rename', or double-click the name of a node

### Apply transform on node or branch

Double-click the cell of the node or branch to transform in the transform column (same icon as Transforms module), then set the desired transform. If the column is not visible, check the 'Transforms' checkbox under the tree.  An example can be seen in the top screenshot at Patient 2

## Panels and their use

### Subject hierarchy tab

Contains all the objects in the Subject hierarchy in a tree representation.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_data_subjecthierarchy.png)

Folder structure:

- Nodes can be drag&dropped under other nodes, thus re-arranging the tree
- New folder or subject can be added by right-clicking the `Scene` item at the top
- Data loaded from **DICOM** are automatically added to the tree of patient, study, series
- **Non-DICOM** data also appears automatically in Subject hierarchy. There are multiple ways to organize them in hierarchy:
  - Use `Create hierarchy from loaded directory structure` action in the context menu of the scene (right-click on empty area, see screenshot below). This organizes the nodes according to the local file structure they have been loaded from.
  - Drag&drop manually under a hierarchy node
  - Legacy model and annotation hierarchies from old scenes are imported as subject hierarchy

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_data_create_hierarchy_from_directory.png)

Operations (accessible in the context menu of the nodes by right-clicking them):

- Common for all nodes:
    - **Show/hide** node or branch: Click on the eye icon
    - **Delete**: Delete both data node and SH node
    - **Rename**: Rename both data node and SH node
    - **Clone**: Creates a copy of the selected node that will be identical in every manner. Its name will contain a `_Copy` postfix
    - **Edit properties**: If the role of the node is specified (i.e. its icon is not a question mark), then the corresponding module is opened and the node selected (e.g. Volumes module for volumes)
    - **Create child...**: Create a node with the specified type
    - **Transform node or branch**: Double-click the cell of the node or branch to transform in the `Applied transform` column, then set the desired transform. If the column is not visible, check the 'Transforms' checkbox under the tree. An example can be seen in the top screenshot at 'Day 2' study
- Operations for specific node types:
    - **Volumes**: icon, Edit properties and additional information in tooltip
        - **'Register this...'** action to select fixed image for registration. Right-click the moving image to initiate registration
        - **'Segment this...'** action allows segmenting the volume, for example, in the Segment Editor module
        - **'Toggle labelmap outline display'** for labelmaps
    - **Models**: icon, Edit properties and additional information in tooltip
    - **SceneViews**: icon, Edit properties and Restore scene view
    - **Transforms**: icon, additional tooltip info, Edit properties, Invert, Reset to identity

Highlights: when an item is selected, the related items are highlighted. Meaning of colors:

  - Green: Items referencing the current item directly via DICOM or node references
  - Yellow: Items referenced by the current item directly via DICOM or node references
  - Light yellow: Items referenced by the current item recursively via node references

Subject hierarchy item information section: Displays detailed information about the selected subject hierarchy item.

### Transform hierarchy tab

- **Nodes**: The view lists all transformable nodes of the scene as a hierarchical tree that describes the relationships between nodes. Nodes are graphical objects such as volumes or models that control the displays in the different views (2D, 3D). To rename an item, double click with the left button on any item (but the scene) in the list. A right click pops up a menu containing different actions: "Insert Transform" creates an identity linear transform node and applies it on the selected node. "Edit properties" opens the module of the node (e.g. "Volumes" for volume nodes, "Models" for model nodes, etc.). "Rename" opens a dialog to rename the node. "Delete" removes the node from the scene. Internal drag-and-drops are supported in the view, while moving a node position within the same parent has no effect, changing the parent of a node has a different meaning depending on the current scene model.
- **Show MRML ID's**: Show/hide in the tree view a second column containing the node ID of the nodes. Hidden by default
- **Show hidden nodes**: Show/hide the hidden nodes. By default, only the main nodes are shown

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_data_transform_hierarchy.png)

### All nodes tab

List of all nodes in the scene. Supports Edit properties, Rename, Delete.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_data_allnodes.png)

### Common section for all tabs

- **Filter**: Hide all the nodes not matching the typed string. This can be useful to quickly search for a specific node. Please note that the search is case sensitive
- **MRML node information**: Attribute list of the currently selected node. Attributes can be edited (double click in the "Attribute value" cell), added (with the "Add" button) or removed (with the "Remove" button).

## Tutorials

- 2016: [This tutorial](https://www.na-mic.org/Wiki/index.php/Winter2016TutorialContest) demonstrates the basic usage and potential of Slicer’s data manager module Subject Hierarchy using a two-timepoint radiotherapy phantom dataset.
- 2015: Tutorial about [loading and viewing data](https://www.slicer.org/wiki/Documentation/4.5/Training).

## Information for developers

- Code snippets accessing and manipulating subject hierarchy items can be found in the [script repository](../../developer_guide/script_repository.md#subject-hierarchy).
- **Implementing new plugins**: Plugins are the real power of subject hierarchy, as they provide support for data node types, and add functionality to the context menu items.
To create a C++ plugin, implement a child class of qSlicerSubjectHierarchyAbstractPlugin, for Python plugin see below. Many examples can be found in Slicer core and in the SlicerRT extension, look for folders named SubjectHierarchyPlugins.
    - Writing plugins in **Python**:
        - Child class of AbstractScriptedSubjectHierarchyPlugin which is a Python adaptor of the C++ qSlicerSubjectHierarchyScriptedPlugin class
        - Example: [role plugin](https://github.com/SlicerHeart/SlicerHeart/blob/master/ValveAnnulusAnalysis/HeartValveLib/HeartValvesSubjectHierarchyPlugin.py) in SlicerHeart extension, [function plugin](https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/SegmentEditor/SubjectHierarchyPlugins/SegmentEditorSubjectHierarchyPlugin.py) in Segment Editor module
    - **Role** plugins: add support for new data node types
        - Defines: ownership, icon, tooltip, edit properties, help text (in the yellow question mark popup), visibility icon, set/get display visibility, displayed node name (if different than name of the node object)
        - Existing plugins in Slicer core: Markups, Models, SceneViews, Charts, Folder, Tables, Transforms, LabelMaps, Volumes
    -  **Function** plugins: add feature in right-click context menu for certain types of nodes
        - Defines: list of context menu actions for nodes and the scene, types of nodes for which the action shows up, functions handling the defined action
        - Existing plugins in Slicer core: CloneNode, ParseLocalData, Register, Segment, DICOM, Volumes, Markups, Models, Annotations, Segmentations, Segments, etc.

## References

- Additional information on [Subject hierarchy labs page](https://www.slicer.org/wiki/Documentation/Labs/SubjectHierarchy)
- Manual editing of segmentations can be done in the [Segment Editor module](segmenteditor.md)

## Contributors

End-user advocate: Ron Kikinis (SPL, NA-MIC)

Authors:
- Csaba Pinter (PerkLab, Queen's University)
- Julien Finet (Kitware)
- Alex Yarmarkovich (Isomics)
- Nicole Aucoin (SPL, BWH)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. This work was also funded by An Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health, Canada

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_cco.png)
