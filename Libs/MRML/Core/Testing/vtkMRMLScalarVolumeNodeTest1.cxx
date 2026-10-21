/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkCodedEntry.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

int vtkMRMLScalarVolumeNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLScalarVolumeNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  // Voxel value quantity and units are copied with the node content (needed for sequence items and proxy nodes)
  vtkNew<vtkCodedEntry> quantity;
  quantity->SetValueSchemeMeaning("112031", "DCM", "Attenuation Coefficient");
  node1->SetVoxelValueQuantity(quantity);
  vtkNew<vtkCodedEntry> units;
  units->SetValueSchemeMeaning("[hnsf'U]", "UCUM", "Hounsfield unit");
  node1->SetVoxelValueUnits(units);

  vtkNew<vtkMRMLScalarVolumeNode> node2;
  node2->CopyContent(node1);
  CHECK_NOT_NULL(node2->GetVoxelValueQuantity());
  CHECK_STRING(node2->GetVoxelValueQuantity()->GetCodeValue(), "112031");
  CHECK_STRING(node2->GetVoxelValueQuantity()->GetCodingSchemeDesignator(), "DCM");
  CHECK_NOT_NULL(node2->GetVoxelValueUnits());
  CHECK_STRING(node2->GetVoxelValueUnits()->GetCodeValue(), "[hnsf'U]");
  CHECK_STRING(node2->GetVoxelValueUnits()->GetCodeMeaning(), "Hounsfield unit");
  // The copies are independent of the source entries
  CHECK_BOOL(node2->GetVoxelValueUnits() != node1->GetVoxelValueUnits(), true);
  CHECK_BOOL(node2->GetVoxelValueQuantity() != node1->GetVoxelValueQuantity(), true);

  // Copying from a node without quantity and units clears them
  vtkNew<vtkMRMLScalarVolumeNode> node3;
  node2->CopyContent(node3);
  CHECK_NULL(node2->GetVoxelValueQuantity());
  CHECK_NULL(node2->GetVoxelValueUnits());

  return EXIT_SUCCESS;
}
