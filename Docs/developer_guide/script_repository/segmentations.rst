Segmentations
~~~~~~~~~~~~~

Load a 3D image or model file as segmentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   slicer.util.loadSegmentation("c:/tmp/tmp/Segmentation.nrrd")
   slicer.util.loadSegmentation("c:/tmp/tmp/Segmentation.nii")
   slicer.util.loadSegmentation("c:/tmp/Segment_1.stl")

Create a segmentation from a labelmap volume and display in 3D
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   labelmapVolumeNode = getNode("label")
   seg = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
   slicer.modules.segmentations.logic().ImportLabelmapToSegmentationNode(labelmapVolumeNode, seg)
   seg.CreateClosedSurfaceRepresentation()
   slicer.mrmlScene.RemoveNode(labelmapVolumeNode)

The last line is optional. It removes the original labelmap volume so that the same information is not shown twice.

Export labelmap node from segmentation node
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Export labelmap matching reference geometry of the segmentation:

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
   slicer.modules.segmentations.logic().ExportAllSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode, slicer.vtkSegmentation.EXTENT_REFERENCE_GEOMETRY)

Export smallest possible labelmap:

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
   slicer.modules.segmentations.logic().ExportAllSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode)

Export labelmap that matches geometry of a chosen reference volume:

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
   slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode, referenceVolumeNode)

Export a selection of segments (identified by their names):

.. code-block:: python

   segmentNames = ["Prostate", "Urethra"]
   segmentIds = vtk.vtkStringArray()
   for segmentName in segmentNames:
     segmentId = segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(segmentName)
     segmentIds.InsertNextValue(segmentId)
   slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentsToLabelmapNode(segmentationNode, segmentIds, labelmapVolumeNode, referenceVolumeNode)

Export to file by pressing Ctrl+Shift+S key:

.. code-block:: python

   outputPath = "c:/tmp"

   def exportLabelmap():
     segmentationNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLSegmentationNode")
     referenceVolumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
     labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
     slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode, referenceVolumeNode)
     filepath = outputPath + "/" + referenceVolumeNode.GetName() + "-label.nrrd"
     slicer.util.saveNode(labelmapVolumeNode, filepath)
     slicer.mrmlScene.RemoveNode(labelmapVolumeNode.GetDisplayNode().GetColorNode())
     slicer.mrmlScene.RemoveNode(labelmapVolumeNode)
     slicer.util.delayDisplay("Segmentation saved to " + filepath)

   shortcut = qt.QShortcut(slicer.util.mainWindow())
   shortcut.setKey(qt.QKeySequence("Ctrl+Shift+s"))
   shortcut.connect( "activated()", exportLabelmap)

Export model nodes from segmentation node
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
   exportFolderItemId = shNode.CreateFolderItem(shNode.GetSceneItemID(), "Segments")
   slicer.modules.segmentations.logic().ExportAllSegmentsToModels(segmentationNode, exportFolderItemId)

Create a hollow model from boundary of solid segment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In most cases, the most robust and flexible tool for creating empty shell models (e.g., vessel wall model from contrast agent segmentation) is the "Hollow" effect in Segment Editor module. However, for very thin shells, extrusion of the exported surface mesh representation may be just as robust and require less memory and computation time. In this case it may be a better approach to to export the segment to a mesh and extrude it along surface normal direction:

Example using Dynamic Modeler module (allows real-time update of parameters, using GUI in Dynamic Modeler module):

.. code-block:: python

   segmentationNode = getNode("Segmentation")

   # Export segments to models
   shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
   exportFolderItemId = shNode.CreateFolderItem(shNode.GetSceneItemID(), "Segments")
   slicer.modules.segmentations.logic().ExportAllSegmentsToModels(segmentationNode, exportFolderItemId)
   segmentModels = vtk.vtkCollection()
   shNode.GetDataNodesInBranch(exportFolderItemId, segmentModels)
   # Get exported model of first segment
   modelNode = segmentModels.GetItemAsObject(0)

   # Set up Hollow tool
   hollowModeler = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLDynamicModelerNode")
   hollowModeler.SetToolName("Hollow")
   hollowModeler.SetNodeReferenceID("Hollow.InputModel", modelNode.GetID())
   hollowedModelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")  # this node will store the hollow model
   hollowModeler.SetNodeReferenceID("Hollow.OutputModel", hollowedModelNode.GetID())
   hollowModeler.SetAttribute("ShellThickness", "2.5")  # grow outside
   hollowModeler.SetContinuousUpdate(True)  # auto-update output model if input parameters are changed

   # Hide inputs, show output
   segmentation.GetDisplayNode().SetVisibility(False)
   modelNode.GetDisplayNode().SetVisibility(False)
   hollowedModelNode.GetDisplayNode().SetOpacity(0.5)

Example using VTK filters:

.. code-block:: python

   # Get closed surface representation of the segment
   shellThickness = 3.0  # mm
   segmentationNode = getNode("Segmentation")
   segmentationNode.CreateClosedSurfaceRepresentation()
   polyData = segmentationNode.GetClosedSurfaceInternalRepresentation("Segment_1")

   # Create shell
   extrude = vtk.vtkLinearExtrusionFilter()
   extrude.SetInputData(polyData)
   extrude.SetExtrusionTypeToNormalExtrusion()
   extrude.SetScaleFactor(shellThickness)

   # Compute consistent surface normals
   triangle_filter = vtk.vtkTriangleFilter()
   triangle_filter.SetInputConnection(extrude.GetOutputPort())
   normals = vtk.vtkPolyDataNormals()
   normals.SetInputConnection(triangle_filter.GetOutputPort())
   normals.FlipNormalsOn()

   # Save result into new model node
   slicer.modules.models.logic().AddModel(normals.GetOutputPort())

Show a segmentation in 3D
^^^^^^^^^^^^^^^^^^^^^^^^^

Segmentation can only be shown in 3D if closed surface representation (or other 3D-displayable representation) is available. To create closed surface representation:

.. code-block:: python

   segmentation.CreateClosedSurfaceRepresentation()

Get a representation of a segment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Access binary labelmap stored in a segmentation node (without exporting it to a volume node) - if it does not exist, it will return None:

.. code-block:: python

   image = slicer.vtkOrientedImageData()
   segmentationNode.GetBinaryLabelmapRepresentation(segmentID, image)

Get closed surface, if it does not exist, it will return None:

.. code-block:: python

   outputPolyData = vtk.vtkPolyData()
   segmentationNode.GetClosedSurfaceRepresentation(segmentID, outputPolyData)

Get binary labelmap representation. If it does not exist then it will be created for that single segment. Applies parent transforms by default (if not desired, another argument needs to be added to the end: false):

.. code-block:: python

   import vtkSegmentationCorePython as vtkSegmentationCore
   outputOrientedImageData = vtkSegmentationCore.vtkOrientedImageData()
   slicer.vtkSlicerSegmentationsModuleLogic.GetSegmentBinaryLabelmapRepresentation(segmentationNode, segmentID, outputOrientedImageData)

Same as above, for closed surface representation:

.. code-block:: python

   outputPolyData = vtk.vtkPolyData()
   slicer.vtkSlicerSegmentationsModuleLogic.GetSegmentClosedSurfaceRepresentation(segmentationNode, segmentID, outputPolyData)

Convert all segments using default path and conversion parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   segmentationNode.CreateBinaryLabelmapRepresentation()

Convert all segments using custom path or conversion parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Change reference image geometry parameter based on an existing referenceImageData image:

.. code-block:: python

   referenceGeometry = slicer.vtkSegmentationConverter.SerializeImageGeometry(referenceImageData)
   segmentation.SetConversionParameter(slicer.vtkSegmentationConverter.GetReferenceImageGeometryParameterName(), referenceGeometry)

Re-convert using a modified conversion parameter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Changing smoothing factor for closed surface generation:

.. code-block:: python

   import vtkSegmentationCorePython as vtkSegmentationCore
   segmentation = getNode("Segmentation").GetSegmentation()

   # Turn of surface smoothing
   segmentation.SetConversionParameter("Smoothing factor","0.0")

   # Recreate representation using modified parameters (and default conversion path)
   segmentation.RemoveRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())
   segmentation.CreateRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())

Create keyboard shortcut for toggling sphere brush for paint and erase effects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   def toggleSphereBrush():
     segmentEditorWidget = slicer.modules.segmenteditor.widgetRepresentation().self().editor
     paintEffect = segmentEditorWidget.effectByName("Paint")
     isSphere = paintEffect.integerParameter("BrushSphere")
     # BrushSphere is "common" parameter (shared between paint and erase)
     paintEffect.setCommonParameter("BrushSphere", 0 if isSphere else 1)

   shortcut = qt.QShortcut(slicer.util.mainWindow())
   shortcut.setKey(qt.QKeySequence("s"))
   shortcut.connect("activated()", toggleSphereBrush)

Customize list of displayed Segment editor effects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Only show Paint and Erase effects:

.. code-block:: python

   segmentEditorWidget = slicer.modules.segmenteditor.widgetRepresentation().self().editor
   segmentEditorWidget.setEffectNameOrder(["Paint", "Erase"])
   segmentEditorWidget.unorderedEffectsVisible = False

Show list of all available effect names:

.. code-block:: python

   segmentEditorWidget = slicer.modules.segmenteditor.widgetRepresentation().self().editor
   print(segmentEditorWidget.availableEffectNames())

Get centroid of a segment in world (RAS) coordinates
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This example shows how to get centroid of a segment in world coordinates and show that position in all slice views.

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   segmentId = "Segment_1"

   # Get array voxel coordinates
   import numpy as np
   seg=arrayFromSegment(segmentation_node, segmentId)
   # numpy array has voxel coordinates in reverse order (KJI instead of IJK)
   # and the array is cropped to minimum size in the segmentation
   mean_KjiCropped = [coords.mean() for coords in np.nonzero(seg)]

   # Get segmentation voxel coordinates
   segImage = segmentationNode.GetBinaryLabelmapRepresentation(segmentId)
   segImageExtent = segImage.GetExtent()
   # origin of the array in voxel coordinates is determined by the start extent
   mean_Ijk = [mean_KjiCropped[2], mean_KjiCropped[1], mean_KjiCropped[0]] + np.array([segImageExtent[0], segImageExtent[2], segImageExtent[4]])

   # Get segmentation physical coordinates
   ijkToWorld = vtk.vtkMatrix4x4()
   segImage.GetImageToWorldMatrix(ijkToWorld)
   mean_World = [0, 0, 0, 1]
   ijkToRas.MultiplyPoint(np.append(mean_Ijk,1.0), mean_World)
   mean_World = mean_World[0:3]

   # If segmentation node is transformed, apply that transform to get RAS coordinates
   transformWorldToRas = vtk.vtkGeneralTransform()
   slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(segmentationNode.GetParentTransformNode(), None, transformWorldToRas)
   mean_Ras = transformWorldToRas.TransformPoint(mean_World)

   # Show mean position value and jump to it in all slice viewers
   print(mean_Ras)
   slicer.modules.markups.logic().JumpSlicesToLocation(mean_Ras[0], mean_Ras[1], mean_Ras[2], True)

Get histogram of a segmented region
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Generate input data
   ################################################

   # Load master volume
   import SampleData
   sampleDataLogic = SampleData.SampleDataLogic()
   masterVolumeNode = sampleDataLogic.downloadMRBrainTumor1()

   # Create segmentation
   segmentationNode = slicer.vtkMRMLSegmentationNode()
   slicer.mrmlScene.AddNode(segmentationNode)
   segmentationNode.CreateDefaultDisplayNodes() # only needed for display
   segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(masterVolumeNode)

   # Create segment
   tumorSeed = vtk.vtkSphereSource()
   tumorSeed.SetCenter(-6, 30, 28)
   tumorSeed.SetRadius(25)
   tumorSeed.Update()
   segmentationNode.AddSegmentFromClosedSurfaceRepresentation(tumorSeed.GetOutput(), "Segment A", [1.0,0.0,0.0])

   # Compute histogram
   ################################################

   labelValue = 1  # label value of first segment

   # Get segmentation as labelmap volume node
   labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
   slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode, masterVolumeNode)

   # Extract all voxels of the segment as numpy array
   volumeArray = slicer.util.arrayFromVolume(masterVolumeNode)
   labelArray = slicer.util.arrayFromVolume(labelmapVolumeNode)
   segmentVoxels = volumeArray[labelArray==labelValue]

   # Compute histogram
   import numpy as np
   histogram = np.histogram(segmentVoxels, bins=50)

   # Plot histogram
   ################################################

   slicer.util.plot(histogram, xColumnIndex = 1)

Get segments visible at a selected position
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Show in the console names of segments visible at a markups fiducial position:

.. code-block:: python

   segmentationNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLSegmentationNode")
   markupsFiducialNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLMarkupsFiducialNode")
   sliceViewLabel = "Red"  # any slice view where segmentation node is visible works

   def printSegmentNames(unused1=None, unused2=None):

     sliceViewWidget = slicer.app.layoutManager().sliceWidget(sliceViewLabel)
     segmentationsDisplayableManager = sliceViewWidget.sliceView().displayableManagerByClassName("vtkMRMLSegmentationsDisplayableManager2D")
     ras = [0,0,0]
     markupsFiducialNode.GetNthControlPointPositionWorld(0, ras)
     segmentIds = vtk.vtkStringArray()
     segmentationsDisplayableManager.GetVisibleSegmentsForPosition(ras, segmentationNode.GetDisplayNode(), segmentIds)
     for idIndex in range(segmentIds.GetNumberOfValues()):
       segment = segmentationNode.GetSegmentation().GetSegment(segmentIds.GetValue(idIndex))
       print("Segment found at position {0}: {1}".format(ras, segment.GetName()))

   # Observe markup node changes
   markupsFiducialNode.AddObserver(slicer.vtkMRMLMarkupsPlaneNode.PointModifiedEvent, printSegmentNames)
   printSegmentNames()

Set default segmentation options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allow segments to overlap each other by default:

.. code-block:: python

   defaultSegmentEditorNode = slicer.vtkMRMLSegmentEditorNode()
   defaultSegmentEditorNode.SetOverwriteMode(slicer.vtkMRMLSegmentEditorNode.OverwriteNone)
   slicer.mrmlScene.AddDefaultNode(defaultSegmentEditorNode)

To always make this the default, add the lines above to your `.slicerrc.py file <../user_guide/settings.html#application-startup-file>`__.

How to run segment editor effects from a script
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Editor effects are complex because they need to handle changing master volumes, undo/redo, masking operations, etc. Therefore, it is recommended to use the effect by instantiating a qMRMLSegmentEditorWidget or use/extract processing logic of the effect and use that from a script.

Use Segment editor effects from script (qMRMLSegmentEditorWidget)
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Examples:

-  `brain tumor segmentation using grow from seeds effect <https://gist.github.com/lassoan/2d5a5b73645f65a5eb6f8d5f97abf31b>`__
-  `AI-assisted brain tumor segmentation <https://gist.github.com/lassoan/ef30bc27a22a648ead7f82243f5cc7d5>`__
-  `skin surface extraction using thresholding and smoothing <https://gist.github.com/lassoan/1673b25d8e7913cbc245b4f09ed853f9>`__
-  `mask a volume with segments and compute histogram for each region <https://gist.github.com/lassoan/2f5071c562108dac8efe277c78f2620f>`__
-  `create fat/muscle/bone segment by thresholding and report volume of each segment <https://gist.github.com/lassoan/5ad51c89521d3cd9c5faf65767506b37>`__
-  `segment cranial cavity automatically in dry bone skull CT <https://gist.github.com/lassoan/4d0b94bda52d5b099432e424e03aa2b1>`__
-  `remove patient table from CT image <https://gist.github.com/lassoan/84d1f9a093dbb6a46c0fcc89279d8088>`__

Description of effect parameters are available `here <https://slicer.readthedocs.io/en/latest/developer_guide/modules/segmenteditor.html#effect-parameters>`__.

Use logic of effect from a script
'''''''''''''''''''''''''''''''''

This example shows how to perform operations on segmentations using VTK filters *extracted* from an effect:

-  `brain tumor segmentation using grow from seeds effect <https://gist.github.com/lassoan/7c94c334653010696b2bf96abc0ac8e7>`__

Process segment using a VTK filter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This example shows how to apply a VTK filter to a segment that dilates the image by a specified margin.

.. code-block:: python

   segmentationNode = getNode("Segmentation")
   segmentId = "Segment_1"
   kernelSize = [3,1,5]

   # Export segment as vtkImageData (via temporary labelmap volume node)
   segmentIds = vtk.vtkStringArray()
   segmentIds.InsertNextValue(segmentId)
   labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
   slicer.modules.segmentations.logic().ExportSegmentsToLabelmapNode(segmentationNode, segmentIds, labelmapVolumeNode)

   # Process segmentation
   segmentImageData = labelmapVolumeNode.GetImageData()
   erodeDilate = vtk.vtkImageDilateErode3D()
   erodeDilate.SetInputData(segmentImageData)
   erodeDilate.SetDilateValue(1)
   erodeDilate.SetErodeValue(0)
   erodeDilate.SetKernelSize(*kernelSize)
   erodeDilate.Update()
   segmentImageData.DeepCopy(erodeDilate.GetOutput())

   # Import segment from vtkImageData
   slicer.modules.segmentations.logic().ImportLabelmapToSegmentationNode(labelmapVolumeNode, segmentationNode, segmentIds)

   # Cleanup temporary nodes
   slicer.mrmlScene.RemoveNode(labelmapVolumeNode.GetDisplayNode().GetColorNode())
   slicer.mrmlScene.RemoveNode(labelmapVolumeNode)

Get information from segmentation nrrd file header
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can use this code snippet to get information from segmentation (.seg.nrrd), for example when creating numpy arrays for generating training data for deep learning networks. This script can be used in any Python environment, not just inside Slicer.

.. code-block:: python

   # pip_install("pynrrd")

   def read_segmentation_info(filename):
     import nrrd
     header = nrrd.read_header(filename)
     segmentation_info = {}
     segments = []
     segment_index = 0
     while True:
       prefix = "Segment{0}_".format(segment_index)
       if not prefix + "ID" in header.keys():
         break
       segment = {}
       segment["index"] = segment_index
       segment["color"] = [float(i) for i in header[prefix + "Color"].split(" ")]  # Segment0_Color:=0.501961 0.682353 0.501961
       segment["colorAutoGenerated"] = int(header[prefix + "ColorAutoGenerated"]) != 0  # Segment0_ColorAutoGenerated:=1
       segment["extent"] = [int(i) for i in header[prefix + "Extent"].split(" ")]  # Segment0_Extent:=68 203 53 211 24 118
       segment["id"] = header[prefix + "ID"]  # Segment0_ID:=Segment_1
       segment["labelValue"] = int(header[prefix + "LabelValue"])  # Segment0_LabelValue:=1
       segment["layer"] = int(header[prefix + "Layer"])  # Segment0_Layer:=0
       segment["name"] = header[prefix + "Name"]  # Segment0_Name:=Segment_1
       segment["nameAutoGenerated"] = int(header[prefix + "NameAutoGenerated"]) != 0  # Segment0_NameAutoGenerated:=1
       # Segment0_Tags:=Segmentation.Status:inprogress|TerminologyEntry:Segmentation category and type - 3D Slicer General Anatomy list
       # ~SCT^85756007^Tissue~SCT^85756007^Tissue~^^~Anatomic codes - DICOM master list~^^~^^|
       tags = {}
       tags_str = header[prefix + "Tags"].split("|")
       for tag_str in tags_str:
         tag_str = tag_str.strip()
         if not tag_str:
           continue
         key, value = tag_str.split(":", maxsplit=1)
         tags[key] = value
       segment["tags"] = tags
       segments.append(segment)
       segment_index += 1
     segmentation_info["segments"] = segments
     return segmentation_info

   def segment_from_name(segmentation_info, segment_name):
     for segment in segmentation_info["segments"]:
       if segment_name == segment["name"]:
         return segment
     raise KeyError("segment not found by name " + segment_name)

   def segment_names(segmentation_info):
     names = []
     for segment in segmentation_info["segments"]:
       names.append(segment["name"])
     return names

   def extract_segments(voxels, header, segmentation_info, segment_names_to_label_values):
     import numpy as np
     # Create empty array from last 3 dimensions (output will be flattened to a 3D array)
     output_voxels = np.zeros(voxels.shape[-3:])
     # Copy non-segmentation fields to the extracted header
     output_header = {}
     for key in header.keys():
       if not re.match("^Segment[0-9]+_.+", key):
         output_header[key] = header[key]
     # Copy extracted segments
     dims = len(voxels.shape)
     for output_segment_index, segment_name_to_label_value in enumerate(segment_names_to_label_values):
       # Copy relabeled voxel data
       segment = segment_from_name(segmentation_info, segment_name_to_label_value[0])
       input_label_value = segment["labelValue"]
       output_label_value = segment_name_to_label_value[1]
       if dims == 3:
         output_voxels[voxels == input_label_value] = output_label_value
       elif dims == 4:
           inputLayer = segment["layer"]
         output_voxels[voxels[inputLayer,:,:,:] == input_label_value] = output_label_value
       else:
         raise ValueError("Voxel array dimension is invalid")
       # Copy all segment fields corresponding to this segment
       for key in header.keys():
         prefix = "Segment{0}_".format(segment["index"])
         matched = re.match("^" + prefix + "(.+)", key)
         if matched:
           field_name = matched.groups()[0]
           if field_name == "LabelValue":
             value = output_label_value
           elif field_name == "Layer":
             # Output is a single layer (3D volume)
             value = 0
           else:
             value = header[key]
           output_header["Segment{0}_".format(output_segment_index) + field_name] = value
     # Remove unnecessary 4th dimension (volume is collapsed into 3D)
     if dims == 4:
       # Remove "none" from "none (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)"
       output_header["space directions"] = output_header["space directions"][-3:,:]
       # Remove "list" from "list domain domain domain"
       output_header["kinds"] = output_header["kinds"][-3:]
     return output_voxels, output_header

   # Read segmentation and show some information about segments
   filename = "c:/Users/andra/OneDrive/Projects/SegmentationPynrrd/SegmentationOverlapping.seg.nrrd"
   segmentation_info = read_segmentation_info(filename)
   number_of_segments = len(segmentation_info["segments"])
   names = segment_names(segmentation_info)
   label0 = segment_from_name(segmentation_info, names[0])["labelValue"]
   print("Number of segments: " + str())
   print("Segment names: " + str(names))
   print("Label value of {0}: {1}".format(names[0], label0))

   # Extract selected segments with chosen label values
   extracted_filename = "c:/Users/andra/OneDrive/Projects/SegmentationPynrrd/SegmentationExtracted.seg.nrrd"
   voxels, header = nrrd.read(filename)
   segment_list = [("Segment_1", 10), ("Segment_3", 12), ("Segment_4", 6)]
   extracted_voxels, extracted_header = extract_segments(voxels, header, segmentation_info, segment_list)
   nrrd.write(extracted_filename, extracted_voxels, extracted_header)

Quantifying segments
^^^^^^^^^^^^^^^^^^^^

Get centroid of each segment
''''''''''''''''''''''''''''

Place a markups fiducial point at the centroid of each segment.

.. code-block:: python

   segmentationNode = getNode("Segmentation")

   # Compute centroids
   import SegmentStatistics
   segStatLogic = SegmentStatistics.SegmentStatisticsLogic()
   segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.centroid_ras.enabled", str(True))
   segStatLogic.computeStatistics()
   stats = segStatLogic.getStatistics()

   # Place a markup point in each centroid
   markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
   markupsNode.CreateDefaultDisplayNodes()
   for segmentId in stats["SegmentIDs"]:
     centroid_ras = stats[segmentId,"LabelmapSegmentStatisticsPlugin.centroid_ras"]
     segmentName = segmentationNode.GetSegmentation().GetSegment(segmentId).GetName()
     markupsNode.AddFiducialFromArray(centroid_ras, segmentName)

Get size, position, and orientation of each segment
'''''''''''''''''''''''''''''''''''''''''''''''''''

This example computes oriented bounding box for each segment and displays them using annotation ROI.

.. code-block:: python

   segmentationNode = getNode("Segmentation")

   # Compute bounding boxes
   import SegmentStatistics
   segStatLogic = SegmentStatistics.SegmentStatisticsLogic()
   segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.obb_origin_ras.enabled",str(True))
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.obb_diameter_mm.enabled",str(True))
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.obb_direction_ras_x.enabled",str(True))
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.obb_direction_ras_y.enabled",str(True))
   segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.obb_direction_ras_z.enabled",str(True))
   segStatLogic.computeStatistics()
   stats = segStatLogic.getStatistics()

   # Draw ROI for each oriented bounding box
   import numpy as np
   for segmentId in stats["SegmentIDs"]:
     # Get bounding box
     obb_origin_ras = np.array(stats[segmentId,"LabelmapSegmentStatisticsPlugin.obb_origin_ras"])
     obb_diameter_mm = np.array(stats[segmentId,"LabelmapSegmentStatisticsPlugin.obb_diameter_mm"])
     obb_direction_ras_x = np.array(stats[segmentId,"LabelmapSegmentStatisticsPlugin.obb_direction_ras_x"])
     obb_direction_ras_y = np.array(stats[segmentId,"LabelmapSegmentStatisticsPlugin.obb_direction_ras_y"])
     obb_direction_ras_z = np.array(stats[segmentId,"LabelmapSegmentStatisticsPlugin.obb_direction_ras_z"])
     # Create ROI
     segment = segmentationNode.GetSegmentation().GetSegment(segmentId)
     roi=slicer.mrmlScene.AddNewNodeByClass("vtkMRMLAnnotationROINode")
     roi.SetName(segment.GetName() + " bounding box")
     roi.SetXYZ(0.0, 0.0, 0.0)
     roi.SetRadiusXYZ(*(0.5*obb_diameter_mm))
     # Position and orient ROI using a transform
     obb_center_ras = obb_origin_ras+0.5*(obb_diameter_mm[0] * obb_direction_ras_x + obb_diameter_mm[1] * obb_direction_ras_y + obb_diameter_mm[2] * obb_direction_ras_z)
     boundingBoxToRasTransform = np.row_stack((np.column_stack((obb_direction_ras_x, obb_direction_ras_y, obb_direction_ras_z, obb_center_ras)), (0, 0, 0, 1)))
     boundingBoxToRasTransformMatrix = slicer.util.vtkMatrixFromArray(boundingBoxToRasTransform)
     transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode")
     transformNode.SetAndObserveMatrixTransformToParent(boundingBoxToRasTransformMatrix)
     roi.SetAndObserveTransformNodeID(transformNode.GetID())

Complete list of available parameters can be obtained by running ``segStatLogic.getParameterNode().GetParameterNames()``.
