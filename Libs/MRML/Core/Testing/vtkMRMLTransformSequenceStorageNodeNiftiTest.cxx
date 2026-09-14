/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Tests reading and writing of grid transform sequences in NIfTI file format.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformSequenceStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkOrientedGridTransform.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <string>

namespace
{
const int NUMBER_OF_TRANSFORMS = 5;
const int GRID_DIMENSIONS[3] = { 6, 8, 11 };
const double GRID_ORIGIN[3] = { 10.0, -20.0, 5.0 };
const double GRID_SPACING[3] = { 1.5, 2.0, 2.5 };
// Displacement vector = (i, j, k) * DISPLACEMENT_SCALE * (transformIndex + 1)
const double DISPLACEMENT_SCALE = 0.1;
// NIfTI stores image origin and spacing with float precision
const double NIFTI_GEOMETRY_TOLERANCE = 1e-4;
const double DISPLACEMENT_TOLERANCE = 1e-6;

//---------------------------------------------------------------------------
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLTransformSequenceStorageNodeNiftiTest_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}

//---------------------------------------------------------------------------
double GetExpectedDisplacement(int transformIndex, const int ijk[3], int component)
{
  // Displacement components are positive (except at the grid origin) and different for each component,
  // so that any sign change or component reordering is detected.
  return ijk[component] * DISPLACEMENT_SCALE * (transformIndex + 1);
}

//---------------------------------------------------------------------------
void CreateGridTransformSequence(vtkMRMLSequenceNode* sequenceNode, bool useTransformFromParent)
{
  // NIfTI file format can only store time index (in seconds)
  sequenceNode->SetIndexName("time");
  sequenceNode->SetIndexUnit("s");
  for (int transformIndex = 0; transformIndex < NUMBER_OF_TRANSFORMS; transformIndex++)
  {
    vtkNew<vtkImageData> displacementGrid;
    displacementGrid->SetDimensions(GRID_DIMENSIONS[0], GRID_DIMENSIONS[1], GRID_DIMENSIONS[2]);
    displacementGrid->SetOrigin(GRID_ORIGIN[0], GRID_ORIGIN[1], GRID_ORIGIN[2]);
    displacementGrid->SetSpacing(GRID_SPACING[0], GRID_SPACING[1], GRID_SPACING[2]);
    displacementGrid->AllocateScalars(VTK_DOUBLE, 3);
    for (int k = 0; k < GRID_DIMENSIONS[2]; k++)
    {
      for (int j = 0; j < GRID_DIMENSIONS[1]; j++)
      {
        for (int i = 0; i < GRID_DIMENSIONS[0]; i++)
        {
          const int ijk[3] = { i, j, k };
          for (int component = 0; component < 3; component++)
          {
            displacementGrid->SetScalarComponentFromDouble(i, j, k, component, GetExpectedDisplacement(transformIndex, ijk, component));
          }
        }
      }
    }

    vtkNew<vtkOrientedGridTransform> gridTransform;
    gridTransform->SetDisplacementGridData(displacementGrid);
    vtkNew<vtkMRMLTransformNode> transformNode;
    if (useTransformFromParent)
    {
      transformNode->SetAndObserveTransformFromParent(gridTransform);
    }
    else
    {
      transformNode->SetAndObserveTransformToParent(gridTransform);
    }
    sequenceNode->SetDataNodeAtValue(transformNode, std::to_string(transformIndex));
  }
}
} // namespace

//---------------------------------------------------------------------------
int TestNiftiGridTransformSequenceReadWrite(const std::string& tempDir)
{
  // Test if a resampling type (transform from parent) grid transform sequence can be written to NIfTI file and read back.
  std::cout << "TestNiftiGridTransformSequenceReadWrite" << std::endl;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);
  CreateGridTransformSequence(sequenceNode, true /*useTransformFromParent*/);

  // Write
  vtkNew<vtkMRMLTransformSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  std::string fileName = tempFilename(tempDir, "FromParent", "nii.gz", true);
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);
  // No information is lost, therefore no warnings are expected
  CHECK_INT(storageNode->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 0);
  // The written file is recognized as a transform sequence (displacement field, intent code 1006)
  CHECK_INT(storageNode->SupportedFileType(fileName.c_str()), 1);

  // Read
  vtkNew<vtkMRMLSequenceNode> readSequenceNode;
  scene->AddNode(readSequenceNode);
  CHECK_BOOL(storageNode->ReadData(readSequenceNode), true);
  CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), NUMBER_OF_TRANSFORMS);
  CHECK_STD_STRING(readSequenceNode->GetIndexName(), "time");
  CHECK_STD_STRING(readSequenceNode->GetIndexUnit(), "s");

  for (int transformIndex = 0; transformIndex < NUMBER_OF_TRANSFORMS; transformIndex++)
  {
    CHECK_STD_STRING(readSequenceNode->GetNthIndexValue(transformIndex), std::to_string(transformIndex));

    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(readSequenceNode->GetNthDataNode(transformIndex));
    CHECK_NOT_NULL(transformNode);
    vtkOrientedGridTransform* gridTransform = vtkOrientedGridTransform::SafeDownCast(transformNode->GetTransformFromParent());
    CHECK_NOT_NULL(gridTransform);
    vtkImageData* displacementGrid = gridTransform->GetDisplacementGrid();
    CHECK_NOT_NULL(displacementGrid);

    // Geometry
    int dimensions[3] = { 0, 0, 0 };
    displacementGrid->GetDimensions(dimensions);
    double origin[3] = { 0.0, 0.0, 0.0 };
    displacementGrid->GetOrigin(origin);
    double spacing[3] = { 0.0, 0.0, 0.0 };
    displacementGrid->GetSpacing(spacing);
    for (int axis = 0; axis < 3; axis++)
    {
      CHECK_INT(dimensions[axis], GRID_DIMENSIONS[axis]);
      CHECK_DOUBLE_TOLERANCE(origin[axis], GRID_ORIGIN[axis], NIFTI_GEOMETRY_TOLERANCE);
      CHECK_DOUBLE_TOLERANCE(spacing[axis], GRID_SPACING[axis], NIFTI_GEOMETRY_TOLERANCE);
    }

    // Displacement vectors (NIfTI stores them in RAS, ITK converts them to LPS, Slicer converts them back to RAS)
    for (int k = 0; k < GRID_DIMENSIONS[2]; k++)
    {
      for (int j = 0; j < GRID_DIMENSIONS[1]; j++)
      {
        for (int i = 0; i < GRID_DIMENSIONS[0]; i++)
        {
          const int ijk[3] = { i, j, k };
          for (int component = 0; component < 3; component++)
          {
            CHECK_DOUBLE_TOLERANCE(
              displacementGrid->GetScalarComponentAsDouble(i, j, k, component), GetExpectedDisplacement(transformIndex, ijk, component), DISPLACEMENT_TOLERANCE);
          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestNiftiModelingGridTransformSequenceWrite(const std::string& tempDir)
{
  // Modeling type (transform to parent) grid transforms cannot be stored in NIfTI file,
  // because NIfTI displacement fields are always resampling type (transform from parent).
  std::cout << "TestNiftiModelingGridTransformSequenceWrite" << std::endl;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);
  CreateGridTransformSequence(sequenceNode, false /*useTransformFromParent*/);

  vtkNew<vtkMRMLTransformSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  std::string fileName = tempFilename(tempDir, "ToParent", "nii", true);
  storageNode->SetFileName(fileName.c_str());

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(storageNode->WriteData(sequenceNode), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int vtkMRMLTransformSequenceStorageNodeNiftiTest(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  CHECK_EXIT_SUCCESS(TestNiftiGridTransformSequenceReadWrite(tempDir));
  CHECK_EXIT_SUCCESS(TestNiftiModelingGridTransformSequenceWrite(tempDir));

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
