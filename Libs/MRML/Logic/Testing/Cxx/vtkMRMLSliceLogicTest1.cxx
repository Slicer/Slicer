/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkImageBlend.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>

#include "vtkMRMLCoreTestingMacros.h"

// STD includes
#include <iostream>

int vtkMRMLSliceLogicTest1(int, char*[])
{
  vtkNew<vtkMRMLSliceLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  logic->SetMRMLScene(scene.GetPointer());
  vtkMRMLSliceNode* sceneSliceNode = logic->AddSliceNode("Green");
  CHECK_NOT_NULL(sceneSliceNode);

  vtkNew<vtkMRMLSliceNode> SliceNode;
  TEST_SET_GET_VALUE(logic, SliceNode, SliceNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> LabelLayer;
  TEST_SET_GET_VALUE(logic, LabelLayer, LabelLayer.GetPointer());

  vtkNew<vtkMRMLSliceCompositeNode> SliceCompositeNode;
  TEST_SET_GET_VALUE(logic, SliceCompositeNode, SliceCompositeNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> ForegroundLayer;
  TEST_SET_GET_VALUE(logic, ForegroundLayer, ForegroundLayer.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> BackgroundLayer;
  TEST_SET_GET_VALUE(logic, BackgroundLayer, BackgroundLayer.GetPointer());

  logic->SetSliceNode(sceneSliceNode);
  sceneSliceNode->SetSliceSpacingModeToPrescribed();
  sceneSliceNode->SetPrescribedSliceSpacing(1.0, 1.0, 1.0);
  sceneSliceNode->SetSlabReconstructionEnabled(true);
  sceneSliceNode->SetSlabReconstructionThickness(3.0);
  sceneSliceNode->SetSlabReconstructionOversamplingFactor(2.0);

  vtkMRMLSliceLogic::UpdateReconstructionSlab(logic.GetPointer(), BackgroundLayer.GetPointer());

  vtkImageReslice* reslice = BackgroundLayer->GetReslice();
  CHECK_NOT_NULL(reslice);
  CHECK_INT(reslice->GetSlabNumberOfSlices(), 6);
  CHECK_DOUBLE_TOLERANCE(reslice->GetSlabSliceSpacingFraction(), 0.5, 1e-6);

  sceneSliceNode->SetPrescribedSliceSpacing(1.0, 1.0, 2.5);
  sceneSliceNode->SetSlabReconstructionThickness(8.1);
  vtkMRMLSliceLogic::UpdateReconstructionSlab(logic.GetPointer(), BackgroundLayer.GetPointer());
  CHECK_INT(reslice->GetSlabNumberOfSlices(), 9);
  vtkMatrix4x4* xyToRAS = sceneSliceNode->GetXYToRAS();
  double outputStepRAS[3] = { xyToRAS->GetElement(0, 2), xyToRAS->GetElement(1, 2), xyToRAS->GetElement(2, 2) };
  CHECK_DOUBLE_TOLERANCE(vtkMath::Norm(outputStepRAS), 1.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(reslice->GetSlabSliceSpacingFraction(), 1.0, 1e-6);
  CHECK_BOOL(reslice->GetSlabSliceSpacingFraction() < 1.0, true);

  vtkNew<vtkImageData> slabInput;
  slabInput->SetExtent(0, 0, 0, 0, -5, 5);
  slabInput->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  for (int z = -5; z <= 5; ++z)
  {
    slabInput->SetScalarComponentFromDouble(0, 0, z, 0, z == 4 ? 10 : 0);
  }
  vtkNew<vtkImageReslice> slabProbe;
  slabProbe->SetInputData(slabInput.GetPointer());
  slabProbe->SetOutputExtent(0, 0, 0, 0, 0, 0);
  slabProbe->SetOutputSpacing(1.0, 1.0, 1.0);
  slabProbe->SetOutputOrigin(0.0, 0.0, 0.0);
  slabProbe->SetSlabModeToMax();
  slabProbe->SetSlabNumberOfSlices(reslice->GetSlabNumberOfSlices());
  slabProbe->SetSlabSliceSpacingFraction(reslice->GetSlabSliceSpacingFraction());
  slabProbe->Update();
  CHECK_DOUBLE_TOLERANCE(slabProbe->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 0), 10.0, 1e-6);

  vtkNew<vtkImageData> rampInput;
  rampInput->SetExtent(0, 0, 0, 0, -5, 5);
  rampInput->AllocateScalars(VTK_DOUBLE, 1);
  for (int z = -5; z <= 5; ++z)
  {
    rampInput->SetScalarComponentFromDouble(0, 0, z, 0, z + 10.0);
  }
  vtkNew<vtkImageReslice> alignedProbe;
  alignedProbe->SetInputData(rampInput.GetPointer());
  alignedProbe->SetOutputExtent(0, 0, 0, 0, 0, 0);
  alignedProbe->SetOutputSpacing(1.0, 1.0, 1.0);
  alignedProbe->SetOutputOrigin(0.0, 0.0, 0.25);
  alignedProbe->SetInterpolationModeToLinear();
  alignedProbe->SetSlabModeToMax();
  alignedProbe->SetSlabNumberOfSlices(reslice->GetSlabNumberOfSlices());
  alignedProbe->SetSlabSliceSpacingFraction(reslice->GetSlabSliceSpacingFraction());
  alignedProbe->Update();
  CHECK_DOUBLE_TOLERANCE(alignedProbe->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 0), 14.25, 1e-6);

  sceneSliceNode->SetSlabReconstructionThickness(2.0);
  vtkMRMLSliceLogic::UpdateReconstructionSlab(logic.GetPointer(), BackgroundLayer.GetPointer());
  CHECK_INT(reslice->GetSlabNumberOfSlices(), 2);

  sceneSliceNode->SetSlabReconstructionThickness(8.1);
  double* fieldOfView = sceneSliceNode->GetFieldOfView();
  sceneSliceNode->SetFieldOfView(fieldOfView[0], fieldOfView[1], 2.5);
  vtkMRMLSliceLogic::UpdateReconstructionSlab(logic.GetPointer(), BackgroundLayer.GetPointer());
  CHECK_INT(reslice->GetSlabNumberOfSlices(), 7);
  CHECK_DOUBLE_TOLERANCE(reslice->GetSlabSliceSpacingFraction(), 0.5, 1e-6);
  vtkNew<vtkTransform> slabProbeTransform;
  slabProbeTransform->Scale(1.0, 1.0, 2.5);
  slabProbe->SetResliceTransform(slabProbeTransform.GetPointer());
  slabProbe->SetSlabNumberOfSlices(reslice->GetSlabNumberOfSlices());
  slabProbe->SetSlabSliceSpacingFraction(reslice->GetSlabSliceSpacingFraction());
  slabProbe->Update();
  CHECK_DOUBLE_TOLERANCE(slabProbe->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 0), 10.0, 1e-6);

  sceneSliceNode->SetSlabReconstructionThickness(7500.0);
  vtkMRMLSliceLogic::UpdateReconstructionSlab(logic.GetPointer(), BackgroundLayer.GetPointer());
  CHECK_INT(reslice->GetSlabNumberOfSlices(), 6000);

  // TODO: need to fix the test.
  // The problem here is that the current node of the logic is wrong
  // it hasn't been added to the mrml scene. So when modified,
  // the logic realizes it and create a new node (losing the props).
  // TEST_SET_GET_VALUE(logic, SliceOffset, 1);

  logic->DeleteSliceModel();
  logic->CreateSliceModel();
  TEST_GET_OBJECT(logic, SliceModelNode);
  TEST_GET_OBJECT(logic, SliceModelDisplayNode);
  TEST_GET_OBJECT(logic, SliceModelTransformNode);
  TEST_GET_OBJECT(logic, Blend);

  logic->Print(std::cout);
  return EXIT_SUCCESS;
}
