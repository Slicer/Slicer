# Sequences

## Overview

The Sequences module can create and visualize sequences of nodes, for example time 4D CT, cine-MRI, 4D ultrasound, or navigated 2D ultrasound. The module is not limited to just image sequences as it works for sequences of any other nodes (transforms, markups, ...) including their display properties. Multiple sequences can be replayed in real-time, optionally synchronized, to visualize the contents in 2D and 3D.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_sequences_cardiac_ct.gif)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_sequences_ultrasound_tool_guidance.gif)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_sequences_shape_model.gif)

## Use cases

### Explore Sequences module using sample data

Option A. Load sample data sets using "Sample Data" module
- Go to the Sample Data module and click on one of these data sets:
  - CTPCardioSeq: cardiac 4D perfusion CT
  - CTCardioSeq: cardiac 4D coronary CT
- Use the toolbar to start replay or browse time points

Option B. Download scene files and load them into the application
- Download one of these scene files:
  - [Deformation of a 3D model](https://github.com/SlicerRt/SequencesData/raw/master/SampleSceneModelDeformation.mrb)
  - [Ultrasound-guided needle insertion: moving tracked ultrasound image and tools, synchronized replay of image ad transforms](https://github.com/SlicerRt/SequencesData/raw/master/SampleSceneUsGuidedNeedleInsertion.mrb)
- Load the downloaded .mrb scene file into Slicer by drag-and-dropping the file to the application window then clicking OK
- Use the toolbar to start replay or browse time points

### Recording node changes into a sequence node

- Go to Sequences module.
- If a node is not selected in the *Sequence browser* selector then click on it and choose *Create new SequenceBrowser*.
- Click the green `+` button next to *(new sequence)*. This creates a new sequence that will store the segmentation for each timepoint.
- In the *Proxy node* column and in the last row of the table, choose the node that you want to record changes. This indicates that this sequence will store states of the chosen segmentation node.
- Check the *Save changes* checkbox to allow modifying the sequence by editing the segmentation node.- Click the record button (red dot) in the sequence browser toolbar and start modifying the node to record changes.
- Click the record button (red dot) in the sequence browser toolbar and start modifying the node to record changes.
- Click the stop button in the sequence browser toolbar to stop recording.
- Move the slider or click the play button in the sequence browser toolbar to review recorded data.

### Creating sequences from a set of nodes

- Load all your nodes (volumes, models, etc.) into Slicer - these will be referred to as *data nodes*
- Open the Sequences module
- Switch to the Sequences tab
- Click *Select a Sequence*, choose *Create new Sequence*
- In the *Add/remove data nodes* section select your first data node in the list and click the left arrow button
- Repeat this for all data nodes: select the next data node and click the left arrow button (Slicer will automatically jump to the next data node of the same type, so you may need to keep clicking the arrow button)
- To replay the sequence that you have just created: switch to *Sequence browser* tab
- Click on *Select a Sequence* in the Master node list and select your sequence node (it is called *Sequence* by default)
- Press the play button to start replay of the data
- Go to the Data module and select all input *data nodes*. Right-click and choose to delete them, to prevent them from occluding the view.
- To visualize a volume in 2D: drag-and-drop the *Sequence [time=...]* node into a slice view
- To visualize a volume in 3D viewer using volume rendering: drag-and-drop the *Sequence [time=...]* node into a 3D view

### Create a segmentation node sequence

To allow segmenting each time point of the image, you need to create a segmentation sequence:
- Create a new Segmentation node (e.g., by segmenting the image at one timepoint)
- Go to the Sequences module
- Click the green `+` button next to *(new sequence)*. This creates a new sequence that will store the segmentation for each timepoint.
- Choose your segmentation node in the *Proxy node* column and in the last row of the table. This indicates that this sequence will store states of the chosen segmentation node.
- Check the *Save changes* checkbox to allow modifying the sequence by editing the segmentation node.
### Convert MultiVolume node to Sequence node

If your data is in a *MultiVolume* node, you can convert it to a *Sequence* node by following these steps:
- Save your 4D volume (to a .nrrd file)
- Load the saved .nrrd file as a sequence node: in the Add data dialog, select *Volume Sequence* in the Description column

### Load DICOM file as Sequence node

In Application settings / DICOM / MultiVolumeImporterPlugin / Preferred multi-volume import format, select "volume sequence". After this, volume sequences will be loaded as Sequence nodes by default.

It is also possible to choose the import format for each loaded DICOM data set, by following these steps:
- Open the DICOM browser, select data set to load
- Check the "Advanced" checkbox
- Click "Examine"
- In the populated table, check the checkbox in the row that contains "... frames Volume Sequence by ..." (to load data set as multi-volume node, select row "... frames Multivolume by ...")

## Definitions

- Sequence: Contains an ordered array of data nodes, each data node is tagged with an index value.
- Data node: A regular MRML node, one item in the sequence. Data nodes are stored privately inside the sequence, therefore they are not visible in the main scene (where the sequence node is located). Singleton nodes are not allowed to be stored as data nodes. Sequence nodes can be data nodes, therefore a sequence of sequence nodes can be used to represent higher-dimensional data sets.
- Sequence index: The index describes the dimension of the data node sequence. The index name (such as "time"), unit (such as "s"), and type (such as "numeric" or "text") is the same for the whole sequence. The index value is specified for each data node. The index type information is used for sorting (numerical or string sorting) and for matching the index values (in the case of a numerical index we can find the closest data node even if there is no perfectly matching index value).
- Sequence browsing: A sequence node only contains the data nodes, but does not store any node relationships, such as parent transform, display properties, etc. These relationships can be only defined for the virtual output nodes that are generated by the Sequence browser module. Several browser nodes can be created to visualize data from the same sequence to support comparing multiple different time points from the same sequence.
- Proxy node: The sequence browser node creates a copy of the selected privately stored data node in the main scene. This copy is the proxy node (formerly *virtual output node*).

## Contributors

Authors:
- Andras Lasso (PerkLab, Queen's University)
- Matthew Holden (PerkLab, Queen's University)
- Kyle Sunderland (PerkLab, Queen's University)
- Kevin Wang (Princess Margaret Cancer Centre)
- Gabor Fichtinger (PerkLab, Queen's University)

## Acknowledgements

This work is funded in part by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide a free, open-source toolset for radiotherapy and related image-guided interventions.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_cco.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ocairo.png)
