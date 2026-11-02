/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformInteractionDisplayNode.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>

namespace
{

//----------------------------------------------------------------------------
/// A transform node with its display node, in the scene
vtkMRMLTransformDisplayNode* AddTransformWithDisplay(vtkMRMLScene* scene)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  transformNode->SetMatrixTransformToParent(translation->GetMatrix());
  transformNode->CreateDefaultDisplayNodes();
  return vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
}

//----------------------------------------------------------------------------
/// The displacement that the sampler finds at the origin. When updateSampler is set, the
/// display node configures the sampler first, as a view does when it is updated.
void GetSampledDisplacement(vtkMRMLTransformDisplayNode* displayNode, vtkVectorFieldSampler* sampler, bool updateSampler, double displacement[3])
{
  if (updateSampler)
  {
    displayNode->UpdateSliceFieldSampler(sampler);
  }
  vtkNew<vtkPoints> positions;
  positions->InsertNextPoint(0.0, 0.0, 0.0);
  sampler->SetSamplePositions(positions);
  sampler->Update();
  displacement[0] = displacement[1] = displacement[2] = 0.0;
  vtkDataArray* vectors = sampler->GetOutput()->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName());
  if (vectors && vectors->GetNumberOfTuples() > 0)
  {
    vectors->GetTuple(0, displacement);
  }
}

//----------------------------------------------------------------------------
/// The sampled field follows the transform node: edits of the transform, its inversion,
/// and the transforms it is placed under.
int TestSampledTransform()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode);
  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  transformNode->SetMatrixTransformToParent(translation->GetMatrix());
  transformNode->CreateDefaultDisplayNodes();
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  CHECK_NOT_NULL(displayNode);

  vtkNew<vtkVectorFieldSampler> sampler;
  CHECK_BOOL(displayNode->UpdateSliceFieldSampler(sampler), true);
  CHECK_NOT_NULL(sampler);
  CHECK_INT(sampler->GetFieldSource(), vtkVectorFieldSampler::FieldSourceTransform);
  double displacement[3] = { 0.0, 0.0, 0.0 };
  GetSampledDisplacement(displayNode, sampler, false, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[0], 1.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(displacement[1], 2.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(displacement[2], 3.0, 1e-9);

  // An edit of the transform reaches the sampler without the display node updating it,
  // through the transform that the sampler holds
  translation->Translate(1.0, 0.0, 0.0);
  transformNode->SetMatrixTransformToParent(translation->GetMatrix());
  GetSampledDisplacement(displayNode, sampler, false, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[0], 2.0, 1e-9);

  // Inverting the transform swaps its to parent and from parent transforms without
  // modifying either of them; the display node gives the sampler the inverted transform
  transformNode->Inverse();
  GetSampledDisplacement(displayNode, sampler, true, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[0], -2.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(displacement[1], -2.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(displacement[2], -3.0, 1e-9);
  transformNode->Inverse();
  GetSampledDisplacement(displayNode, sampler, true, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[0], 2.0, 1e-9);

  // Placing it under another transform, and inverting that one
  vtkNew<vtkMRMLLinearTransformNode> parentTransformNode;
  scene->AddNode(parentTransformNode);
  vtkNew<vtkTransform> parentTranslation;
  parentTranslation->Translate(0.0, 0.0, 10.0);
  parentTransformNode->SetMatrixTransformToParent(parentTranslation->GetMatrix());
  transformNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
  GetSampledDisplacement(displayNode, sampler, true, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[2], 13.0, 1e-9);
  // An edit of the parent reaches the sampler without the display node updating it too
  parentTranslation->Translate(0.0, 0.0, 10.0);
  parentTransformNode->SetMatrixTransformToParent(parentTranslation->GetMatrix());
  GetSampledDisplacement(displayNode, sampler, false, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[2], 23.0, 1e-9);
  parentTransformNode->Inverse();
  GetSampledDisplacement(displayNode, sampler, true, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[2], -17.0, 1e-9);

  // Taken out from under it
  transformNode->SetAndObserveTransformNodeID(nullptr);
  GetSampledDisplacement(displayNode, sampler, true, displacement);
  CHECK_DOUBLE_TOLERANCE(displacement[2], 3.0, 1e-9);

  // The sampler of the 3D views and those of the slice views share the transform
  vtkNew<vtkVectorFieldSampler> otherSampler;
  CHECK_BOOL(displayNode->UpdateSliceFieldSampler(otherSampler), true);
  CHECK_POINTER(otherSampler->GetTransform(), sampler->GetTransform());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Interaction display node defaults match the former transform display ones (3D and slice intersection handles, no
/// scaling, only in-plane translation in slice views); the display node reports them before that node exists.
int TestInteractionDefaults()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformDisplayNode* displayNode = AddTransformWithDisplay(scene);
  CHECK_NOT_NULL(displayNode);
  CHECK_NULL(displayNode->GetInteractionDisplayNode());
  CHECK_BOOL(displayNode->GetEditorVisibility(), false);
  CHECK_BOOL(displayNode->GetEditorVisibility3D(), true);
  CHECK_BOOL(displayNode->GetEditorSliceIntersectionVisibility(), true);
  CHECK_BOOL(displayNode->GetEditorScalingEnabled(), false);
  CHECK_BOOL(displayNode->GetEditorScalingSliceEnabled(), false);

  // Turning the editor on creates the node, with handles that show
  displayNode->SetEditorVisibility(true);
  vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = displayNode->GetInteractionDisplayNode();
  CHECK_NOT_NULL(interactionDisplayNode);
  CHECK_BOOL(interactionDisplayNode->GetEditorVisibility(), true);
  CHECK_BOOL(interactionDisplayNode->GetEditorVisibility3D(), true);
  CHECK_BOOL(interactionDisplayNode->GetEditorSliceIntersectionVisibility(), true);
  CHECK_BOOL(interactionDisplayNode->GetEditorTranslationEnabled(), true);
  CHECK_BOOL(interactionDisplayNode->GetEditorRotationEnabled(), true);
  CHECK_BOOL(interactionDisplayNode->GetEditorScalingEnabled(), false);
  CHECK_BOOL(interactionDisplayNode->GetEditorScalingSliceEnabled(), false);
  bool visibility[4] = { true, true, true, true };
  interactionDisplayNode->GetTranslationHandleComponentVisibilitySlice(visibility);
  CHECK_BOOL(visibility[0], false);
  CHECK_BOOL(visibility[1], false);
  CHECK_BOOL(visibility[2], false);
  CHECK_BOOL(visibility[3], true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Settings survive saving and loading: the ones stored in the format of the shared node,
/// next to the attributes that older versions read, and the ones of the interaction node.
int TestSaveAndLoad()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformDisplayNode* displayNode = AddTransformWithDisplay(scene);
  CHECK_NOT_NULL(displayNode);
  // Thresholding off, where the older attributes always meant on
  CHECK_BOOL(displayNode->GetThresholdEnabled(), true);
  displayNode->SetThresholdEnabled(false);
  displayNode->SetThresholdRange(2.0, 30.0);
  // The size of the handles given in mm
  displayNode->SetInteractionSizeAbsolute(true);
  displayNode->SetInteractionSizeMm(8.0);

  scene->SetSaveToXMLString(1);
  CHECK_BOOL(scene->Commit() != 0, true);
  vtkNew<vtkMRMLScene> reloadedScene;
  reloadedScene->SetLoadFromXMLString(1);
  reloadedScene->SetSceneXMLString(scene->GetSceneXMLString());
  CHECK_BOOL(reloadedScene->Import() != 0, true);
  vtkMRMLTransformDisplayNode* reloadedDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLTransformDisplayNode"));
  CHECK_NOT_NULL(reloadedDisplayNode);
  CHECK_BOOL(reloadedDisplayNode->GetThresholdEnabled(), false);
  CHECK_DOUBLE(reloadedDisplayNode->GetThresholdRange()[0], 2.0);
  CHECK_DOUBLE(reloadedDisplayNode->GetThresholdRange()[1], 30.0);
  CHECK_INT(reloadedScene->GetNumberOfNodesByClass("vtkMRMLTransformInteractionDisplayNode"), 1);
  CHECK_BOOL(reloadedDisplayNode->GetInteractionSizeAbsolute(), true);
  CHECK_DOUBLE(reloadedDisplayNode->GetInteractionSizeMm(), 8.0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The colors span the range of the color map by default, and a range chosen by hand is kept
int TestScalarRange()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformDisplayNode* displayNode = AddTransformWithDisplay(scene);
  CHECK_NOT_NULL(displayNode);
  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLColorTableNode"));
  colorNode->SetTypeToGrey();
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
  CHECK_INT(displayNode->GetScalarRangeFlag(), vtkMRMLDisplayNode::UseColorNodeScalarRange);
  displayNode->UpdateScalarRange();
  double* lookupTableRange = colorNode->GetLookupTable()->GetRange();
  CHECK_DOUBLE(displayNode->GetScalarRange()[0], lookupTableRange[0]);
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], lookupTableRange[1]);

  displayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
  displayNode->SetScalarRange(3.0, 7.0);
  displayNode->UpdateScalarRange();
  CHECK_DOUBLE(displayNode->GetScalarRange()[0], 3.0);
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 7.0);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLTransformDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  CHECK_EXIT_SUCCESS(TestSampledTransform());
  CHECK_EXIT_SUCCESS(TestInteractionDefaults());
  CHECK_EXIT_SUCCESS(TestSaveAndLoad());
  CHECK_EXIT_SUCCESS(TestScalarRange());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
