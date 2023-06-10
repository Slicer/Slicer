
## Batch processing

### Iterate through dicom series

This examples shows how to perform an operation on each
series in the dicom database.

```python
db = slicer.dicomDatabase
patients = db.patients()
patientCount = 0
for patient in patients:
  patientCount += 1
  print(f"Patient {patient} ({patientCount} of {len(patients)})")
  for study in db.studiesForPatient(patient):
    print(f"Study {study}")
    for series in db.seriesForStudy(study):
      print(f"Series {series}")
      temporaryDir = qt.QTemporaryDir()
      for instanceUID in db.instancesForSeries(series):
        qt.QFile.copy(db.fileForInstance(instanceUID), f"{temporaryDir.path()}/{instanceUID}.dcm")
      patientID = slicer.dicomDatabase.instanceValue(instanceUID, '0010,0020')
      outputPath = os.path.join(convertedPath, patientID, study, series, "BatchResult")
      if not os.path.exists(outputPath):
        os.makedirs(outputPath)
      # do an operation here that processes the series into the outputPath
```

:::{note}

It can be helpful for debugging to include a comment with python commands
that can be pasted into the console to run the script.  With this approach
any global variables, such as vtk class instances, defined in the script will exist after the script runs
and you can easily inspect them or call methods on them.

```python
"""
filePath = "/data/myscript.py"

exec(open(filePath).read())

"""
```

:::

### Extracting volume patches around segments

For machine learning apps, such as [MONAI Label](https://docs.monai.io/projects/label/en/latest/index.html)
it can be helpful to reduce the size of the problem by
extracting out subsets of data.  This example shows how to iterate through
a directory of segmentations, compute their bounding boxes, and save out
new volumes and segmentations centered around the segmentation.

Here the ROI is aligned with the volume.  See [Segmentations](/developer_guide/script_repository.md#segmentations) for
examples using oriented bounding boxes and other options.

```python
import glob
import os

def segROI(segmentationNode):
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
    roi=slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode")
    roi.SetName(segment.GetName() + " OBB")
    roi.GetDisplayNode().SetHandlesInteractive(False)  # do not let the user resize the box
    roi.SetSize(obb_diameter_mm * 2) # make the ROI twice the size of the segmentation
    # Position and orient ROI using a transform
    obb_center_ras = obb_origin_ras+0.5*(obb_diameter_mm[0] * obb_direction_ras_x + obb_diameter_mm[1] * obb_direction_ras_y + obb_diameter_mm[2] * obb_direction_ras_z)
    boundingBoxToRasTransform = np.row_stack((np.column_stack(((1,0,0), (0,1,0), (0,0,1), obb_center_ras)), (0, 0, 0, 1)))
    boundingBoxToRasTransformMatrix = slicer.util.vtkMatrixFromArray(boundingBoxToRasTransform)
    roi.SetAndObserveObjectToNodeMatrix(boundingBoxToRasTransformMatrix)
    return roi

labelFiles = glob.glob("/data/imagesTr/labels/final/*.nii.gz")

for labelFile in labelFiles:
  slicer.mrmlScene.Clear()
  print(labelFile)
  baseName = os.path.basename(labelFile)
  ctFile = os.path.join("/data/imagesTr", baseName)
  print(ctFile)
  ct = slicer.util.loadVolume(ctFile)
  seg = slicer.util.loadSegmentation(labelFile)
  roi = segROI(seg)
  cropVolumeParameters = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLCropVolumeParametersNode")
  cropVolumeParameters.SetInputVolumeNodeID(ct.GetID())
  cropVolumeParameters.SetROINodeID(roi.GetID())
  slicer.modules.cropvolume.logic().Apply(cropVolumeParameters)
  croppedCT = cropVolumeParameters.GetOutputVolumeNode()
  seg.SetReferenceImageGeometryParameterFromVolumeNode(croppedCT)
  segLogic = slicer.modules.segmentations.logic()
  labelmap = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
  segLogic.ExportAllSegmentsToLabelmapNode(seg, labelmap, slicer.vtkSegmentation.EXTENT_REFERENCE_GEOMETRY)
  slicer.util.saveNode(croppedCT, f"/data/crops/{baseName}")
  slicer.util.saveNode(labelmap, f"/data/crops/labels/final/{baseName}")
  slicer.app.processEvents() # to watch progress
```
