/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkAddonMathUtilities.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkStringArray.h>

//----------------------------------------------------------------------------
void AddSliceOrientationPresets(vtkMRMLSliceNode* sliceNode);

//----------------------------------------------------------------------------
int AddSliceOrientationPresetTest();
int RemoveSliceOrientationPresetTest();
int RenameSliceOrientationPresetTest();
int HasSliceOrientationPresetTest();
int GetSliceOrientationPresetTest();
int GetSliceOrientationPresetNameTest();
int SetOrientationTest();
int InitializeDefaultMatrixTest();
int SlabReconstructionEnabledTest();
int SlabReconstructionTypeTest();
int SlabReconstructionThicknessTest();

//----------------------------------------------------------------------------
int vtkMRMLSliceNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLSliceNode> node1;

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(AddSliceOrientationPresetTest());
  CHECK_EXIT_SUCCESS(RemoveSliceOrientationPresetTest());
  CHECK_EXIT_SUCCESS(RenameSliceOrientationPresetTest());
  CHECK_EXIT_SUCCESS(HasSliceOrientationPresetTest());
  CHECK_EXIT_SUCCESS(GetSliceOrientationPresetTest());
  CHECK_EXIT_SUCCESS(GetSliceOrientationPresetNameTest());
  CHECK_EXIT_SUCCESS(SetOrientationTest());
  CHECK_EXIT_SUCCESS(InitializeDefaultMatrixTest());
  CHECK_EXIT_SUCCESS(SlabReconstructionEnabledTest());
  CHECK_EXIT_SUCCESS(SlabReconstructionTypeTest());
  CHECK_EXIT_SUCCESS(SlabReconstructionThicknessTest());

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void AddSliceOrientationPresets(vtkMRMLSliceNode* sliceNode)
{
  {
    vtkNew<vtkMatrix3x3> preset;
    vtkMRMLSliceNode::GetAxialSliceToRASMatrix(preset.GetPointer());

    sliceNode->AddSliceOrientationPreset("Axial", preset.GetPointer());
  }

  {
    vtkNew<vtkMatrix3x3> preset;
    vtkMRMLSliceNode::GetSagittalSliceToRASMatrix(preset.GetPointer());

    sliceNode->AddSliceOrientationPreset("Sagittal", preset.GetPointer());
  }

  {
    vtkNew<vtkMatrix3x3> preset;
    vtkMRMLSliceNode::GetCoronalSliceToRASMatrix(preset.GetPointer());

    sliceNode->AddSliceOrientationPreset("Coronal", preset.GetPointer());
  }
}

//----------------------------------------------------------------------------
int CheckOrientationPresetNames(vtkMRMLSliceNode* sliceNode, std::vector<std::string> names)
{
  vtkNew<vtkStringArray> orientationPresetNames;
  sliceNode->GetSliceOrientationPresetNames(orientationPresetNames.GetPointer());

  CHECK_INT(orientationPresetNames->GetNumberOfValues(), static_cast<int>(names.size()));
  for (int idx = 0; idx < static_cast<int>(names.size()); ++idx)
  {
    CHECK_STD_STRING(orientationPresetNames->GetValue(idx), names.at(idx));
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int AddSliceOrientationPresetTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  AddSliceOrientationPresets(sliceNode.GetPointer());

  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  sliceNode->AddSliceOrientationPreset("Reformat", vtkSmartPointer<vtkMatrix3x3>());
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  CHECK_INT(sliceNode->GetNumberOfSliceOrientationPresets(), 3);

  std::vector<std::string> expectedOrientationNames;
  expectedOrientationNames.emplace_back("Axial");
  expectedOrientationNames.emplace_back("Sagittal");
  expectedOrientationNames.emplace_back("Coronal");

  CHECK_INT(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames), EXIT_SUCCESS);

  return EXIT_SUCCESS;
}

namespace
{
//----------------------------------------------------------------------------
void InitializeMatrix(vtkMatrix3x3* matrix, double value)
{
  for (int ii = 0; ii < 3; ++ii)
  {
    for (int jj = 0; jj < 3; ++jj)
    {
      matrix->SetElement(ii, jj, value);
    }
  }
}
} // namespace

//----------------------------------------------------------------------------
int RemoveSliceOrientationPresetTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(sliceNode->RemoveSliceOrientationPreset(""), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  vtkNew<vtkMatrix3x3> expectedOnes;
  InitializeMatrix(expectedOnes.GetPointer(), 1);
  sliceNode->AddSliceOrientationPreset("Ones", expectedOnes.GetPointer());

  vtkNew<vtkMatrix3x3> expectedTwos;
  InitializeMatrix(expectedTwos.GetPointer(), 2);
  sliceNode->AddSliceOrientationPreset("Twos", expectedTwos.GetPointer());

  vtkNew<vtkMatrix3x3> expectedThrees;
  InitializeMatrix(expectedThrees.GetPointer(), 3);
  sliceNode->AddSliceOrientationPreset("Threes", expectedThrees.GetPointer());

  CHECK_BOOL(sliceNode->RemoveSliceOrientationPreset("Twos"), true);

  std::vector<std::string> expectedOrientationNames;
  expectedOrientationNames.emplace_back("Ones");
  expectedOrientationNames.emplace_back("Threes");

  CHECK_EXIT_SUCCESS(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames));

  CHECK_DOUBLE(sliceNode->GetSliceOrientationPreset("Ones")->GetElement(0, 0), 1.0);
  CHECK_DOUBLE(sliceNode->GetSliceOrientationPreset("Threes")->GetElement(0, 0), 3.0);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int RenameSliceOrientationPresetTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  AddSliceOrientationPresets(sliceNode.GetPointer());

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(sliceNode->RenameSliceOrientationPreset("", "Axial"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  {
    std::vector<std::string> expectedOrientationNames;
    expectedOrientationNames.emplace_back("Axial");
    expectedOrientationNames.emplace_back("Sagittal");
    expectedOrientationNames.emplace_back("Coronal");

    CHECK_EXIT_SUCCESS(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames));
  }

  CHECK_BOOL(sliceNode->RenameSliceOrientationPreset("Axial", "Foo"), true);

  {
    std::vector<std::string> expectedOrientationNames;
    expectedOrientationNames.emplace_back("Foo");
    expectedOrientationNames.emplace_back("Sagittal");
    expectedOrientationNames.emplace_back("Coronal");

    CHECK_EXIT_SUCCESS(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames));
  }

  CHECK_BOOL(sliceNode->RenameSliceOrientationPreset("Coronal", "Bar"), true);

  {
    std::vector<std::string> expectedOrientationNames;
    expectedOrientationNames.emplace_back("Foo");
    expectedOrientationNames.emplace_back("Sagittal");
    expectedOrientationNames.emplace_back("Bar");

    CHECK_EXIT_SUCCESS(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames));
  }

  // "Reformat" is a special value and can NOT be used as a preset.
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(sliceNode->RenameSliceOrientationPreset("Sagittal", "Reformat"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  {
    std::vector<std::string> expectedOrientationNames;
    expectedOrientationNames.emplace_back("Foo");
    expectedOrientationNames.emplace_back("Sagittal");
    expectedOrientationNames.emplace_back("Bar");

    CHECK_EXIT_SUCCESS(CheckOrientationPresetNames(sliceNode.GetPointer(), expectedOrientationNames));
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int HasSliceOrientationPresetTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  AddSliceOrientationPresets(sliceNode.GetPointer());

  CHECK_BOOL(sliceNode->HasSliceOrientationPreset(""), false);
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  CHECK_BOOL(sliceNode->HasSliceOrientationPreset("Reformat"), false);
  TESTING_OUTPUT_ASSERT_WARNINGS_END();
  CHECK_BOOL(sliceNode->HasSliceOrientationPreset("Coronal"), true);
  CHECK_BOOL(sliceNode->HasSliceOrientationPreset("Sagittal"), true);
  CHECK_BOOL(sliceNode->HasSliceOrientationPreset("Axial"), true);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int GetSliceOrientationPresetTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  vtkNew<vtkMatrix3x3> expectedOnes;
  InitializeMatrix(expectedOnes.GetPointer(), 1);

  vtkNew<vtkMatrix3x3> expectedTwos;
  InitializeMatrix(expectedTwos.GetPointer(), 2);

  {
    vtkNew<vtkMatrix3x3> ones;
    ones->DeepCopy(expectedOnes.GetPointer());
    sliceNode->AddSliceOrientationPreset("ones", ones.GetPointer());

    vtkNew<vtkMatrix3x3> twos;
    twos->DeepCopy(expectedTwos.GetPointer());
    sliceNode->AddSliceOrientationPreset("twos", twos.GetPointer());
  }

  {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    vtkMatrix3x3* current = sliceNode->GetSliceOrientationPreset("");
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    CHECK_NULL(current);
  }

  {
    vtkMatrix3x3* current = sliceNode->GetSliceOrientationPreset("ones");
    CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(current, expectedOnes.GetPointer()), true);
  }

  {
    vtkMatrix3x3* current = sliceNode->GetSliceOrientationPreset("twos");
    CHECK_BOOL(vtkAddonMathUtilities::MatrixAreEqual(current, expectedTwos.GetPointer()), true);
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int GetSliceOrientationPresetNameTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  vtkNew<vtkMatrix3x3> originalPreset;
  vtkMRMLSliceNode::GetAxialSliceToRASMatrix(originalPreset.GetPointer());

  vtkNew<vtkMatrix3x3> preset;
  vtkMRMLSliceNode::GetAxialSliceToRASMatrix(preset.GetPointer());
  sliceNode->AddSliceOrientationPreset("Axial", preset.GetPointer());

  originalPreset->SetElement(1, 1, 1.0 + 1e-4);

  CHECK_STD_STRING(sliceNode->GetSliceOrientationPresetName(originalPreset.GetPointer()), std::string("Axial"));

  originalPreset->SetElement(1, 1, 1.0 + 1e-2);

  CHECK_STD_STRING(sliceNode->GetSliceOrientationPresetName(originalPreset.GetPointer()), std::string(""));

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int SetOrientationTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(sliceNode->SetOrientation(nullptr), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(sliceNode->SetOrientation(""), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Reformat"));

  AddSliceOrientationPresets(sliceNode.GetPointer());

  // Set a valid orientation
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    CHECK_BOOL(sliceNode->SetOrientation("Sagittal"), true);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Sagittal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Sagittal");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, true);
  }

  // Orientation are case sensitive
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_BOOL(sliceNode->SetOrientation("axial"), false);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Sagittal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Sagittal");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, false);
  }

  // The sliceToRAS matrix define the current orientation
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_BOOL(sliceNode->SetOrientation("Reformat"), false);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Sagittal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Sagittal");

    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_BOOL(sliceNode->SetOrientation("Foo"), false);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Sagittal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Sagittal");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, false);
  }

  // Check SetOrientationToAxial
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    CHECK_BOOL(sliceNode->SetOrientationToAxial(), true);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Axial"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Axial");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, true);
  }

  // Check SetOrientationToSagittal
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    CHECK_BOOL(sliceNode->SetOrientationToSagittal(), true);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Sagittal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Sagittal");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, true);
  }

  // Check SetOrientationToCoronal
  {
    vtkMTimeType sliceToRASMTime = sliceNode->GetSliceToRAS()->GetMTime();
    CHECK_BOOL(sliceNode->SetOrientationToCoronal(), true);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Coronal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Coronal");
    CHECK_BOOL(sliceNode->GetSliceToRAS()->GetMTime() > sliceToRASMTime, true);
  }

  // Check setting of SliceToRAS transform by modifying existing SliceToRAS
  {
    CHECK_BOOL(sliceNode->SetOrientationToCoronal(), true);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Coronal"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Coronal");
    sliceNode->GetSliceToRAS()->SetElement(0, 0, sliceNode->GetSliceToRAS()->GetElement(0, 0) + 0.5);
    CHECK_STD_STRING(sliceNode->GetOrientation(), std::string("Reformat"));
    CHECK_STRING(sliceNode->GetOrientationString(), "Reformat");
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int InitializeDefaultMatrixTest()
{
  vtkNew<vtkMatrix3x3> axial;
  vtkMRMLSliceNode::GetAxialSliceToRASMatrix(axial.GetPointer());
  CHECK_NOT_NULL(axial.GetPointer());

  vtkNew<vtkMatrix3x3> coronal;
  vtkMRMLSliceNode::GetCoronalSliceToRASMatrix(coronal.GetPointer());
  CHECK_NOT_NULL(coronal.GetPointer());

  vtkNew<vtkMatrix3x3> sagittal;
  vtkMRMLSliceNode::GetSagittalSliceToRASMatrix(sagittal.GetPointer());
  CHECK_NOT_NULL(sagittal.GetPointer());

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int SlabReconstructionEnabledTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  CHECK_BOOL(sliceNode->GetSlabReconstructionEnabled(), false);

  // Set using set macro
  {
    sliceNode->SetSlabReconstructionEnabled(true);
    CHECK_BOOL(sliceNode->GetSlabReconstructionEnabled(), true);
  }

  // Set using on/off macro
  {
    sliceNode->SlabReconstructionEnabledOn();
    CHECK_BOOL(sliceNode->GetSlabReconstructionEnabled(), true);
    sliceNode->SlabReconstructionEnabledOff();
    CHECK_BOOL(sliceNode->GetSlabReconstructionEnabled(), false);
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int SlabReconstructionTypeTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  CHECK_INT(sliceNode->GetSlabReconstructionType(), VTK_IMAGE_SLAB_MAX);

  // Set to min
  {
    sliceNode->SetSlabReconstructionType(VTK_IMAGE_SLAB_MIN);
    CHECK_INT(sliceNode->GetSlabReconstructionType(), VTK_IMAGE_SLAB_MIN);
  }

  // Set to mean
  {
    sliceNode->SetSlabReconstructionType(VTK_IMAGE_SLAB_MEAN);
    CHECK_INT(sliceNode->GetSlabReconstructionType(), VTK_IMAGE_SLAB_MEAN);
  }

  // Set to sum
  {
    sliceNode->SetSlabReconstructionType(VTK_IMAGE_SLAB_SUM);
    CHECK_INT(sliceNode->GetSlabReconstructionType(), VTK_IMAGE_SLAB_SUM);
  }

  // Check GetSlabReconstructionTypeAsString
  {
    CHECK_STRING(sliceNode->GetSlabReconstructionTypeAsString(VTK_IMAGE_SLAB_MAX), "Max");
    CHECK_STRING(sliceNode->GetSlabReconstructionTypeAsString(VTK_IMAGE_SLAB_MIN), "Min");
    CHECK_STRING(sliceNode->GetSlabReconstructionTypeAsString(VTK_IMAGE_SLAB_MEAN), "Mean");
    CHECK_STRING(sliceNode->GetSlabReconstructionTypeAsString(VTK_IMAGE_SLAB_SUM), "Sum");
  }

  // Check GetSlabReconstructionTypeFromString
  {
    CHECK_INT(sliceNode->GetSlabReconstructionTypeFromString("Max"), VTK_IMAGE_SLAB_MAX);
    CHECK_INT(sliceNode->GetSlabReconstructionTypeFromString("Min"), VTK_IMAGE_SLAB_MIN);
    CHECK_INT(sliceNode->GetSlabReconstructionTypeFromString("Mean"), VTK_IMAGE_SLAB_MEAN);
    CHECK_INT(sliceNode->GetSlabReconstructionTypeFromString("Sum"), VTK_IMAGE_SLAB_SUM);
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int SlabReconstructionThicknessTest()
{
  vtkNew<vtkMRMLSliceNode> sliceNode;

  CHECK_DOUBLE(sliceNode->GetSlabReconstructionThickness(), 1.);

  // Set using set macro
  {
    sliceNode->SetSlabReconstructionThickness(99.5);
    CHECK_DOUBLE(sliceNode->GetSlabReconstructionThickness(), 99.5);
  }

  return EXIT_SUCCESS;
}
