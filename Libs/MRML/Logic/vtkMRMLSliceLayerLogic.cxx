/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLayerLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTrivialProducer.h>
#include <vtkTransform.h>
#include <vtkVersion.h>
#include <vtkAddonMathUtilities.h>

//
#include "vtkImageLabelOutline.h"

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceLayerLogic);

bool AreMatricesEqual(const vtkMatrix4x4* first, const vtkMatrix4x4* second)
{
  return vtkAddonMathUtilities::MatrixAreEqual(first, second);
}

// Convert a linear transform that is almost exactly a permute transform
// to an exact permute transform.
// vtkImageReslice works about 10-20% faster if it reslices along an axis
// (transformation is just a permutation). However, vtkImageReslice
// checks for strict (floatValue!=0) to consider a matrix element zero.
// Here we set a small floatValue to 0 if it is several magnitudes
// (controlled by SUPPRESSION_FACTOR parameter) smaller than the
// maximum norm of the axis.
//----------------------------------------------------------------------------
void SnapToPermuteMatrix(vtkTransform* transform)
{
  const double SUPPRESSION_FACTOR = 1e-3;
  vtkHomogeneousTransform* linearTransform = vtkHomogeneousTransform::SafeDownCast(transform);
  if (!linearTransform)
    {
    // it is not a simple linear transform, so it cannot be snapped to a permute matrix
    return;
    }
  bool modified = false;
  vtkNew<vtkMatrix4x4> transformMatrix;
  linearTransform->GetMatrix(transformMatrix.GetPointer());
  for (int c=0; c<3; c++)
    {
    double absValues[3] = {fabs(transformMatrix->Element[0][c]), fabs(transformMatrix->Element[1][c]), fabs(transformMatrix->Element[2][c])};
    double maxValue = std::max(absValues[0], std::max(absValues[1], absValues[2]));
    double zeroThreshold = SUPPRESSION_FACTOR * maxValue;
    for (int r=0; r<3; r++)
      {
      if (absValues[r]!=0 && absValues[r]<zeroThreshold)
        {
        transformMatrix->Element[r][c]=0;
        modified = true;
        }
      }
    }
  if (modified)
  {
    transform->SetMatrix(transformMatrix.GetPointer());
  }
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic::vtkMRMLSliceLayerLogic()
{
  this->VolumeNode = nullptr;
  this->VolumeDisplayNode = nullptr;
  this->VolumeDisplayNodeUVW = nullptr;
  this->VolumeDisplayNodeObserved = nullptr;
  this->SliceNode = nullptr;

  this->XYToIJKTransform = vtkGeneralTransform ::New();
  this->UVWToIJKTransform = vtkGeneralTransform ::New();

  this->IsLabelLayer = 0;

  this->AssignAttributeTensorsToScalars= vtkAssignAttribute::New();
  this->AssignAttributeScalarsToTensors= vtkAssignAttribute::New();
  this->AssignAttributeScalarsToTensorsUVW= vtkAssignAttribute::New();
  this->AssignAttributeTensorsToScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  this->AssignAttributeScalarsToTensors->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);
  this->AssignAttributeScalarsToTensorsUVW->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);

  // Create the parts for the scalar layer pipeline
  this->Reslice = vtkImageReslice::New();
  this->ResliceUVW = vtkImageReslice::New();
  this->LabelOutline = vtkImageLabelOutline::New();
  this->LabelOutlineUVW = vtkImageLabelOutline::New();

  //
  // Set parameters that won't change based on input
  //
  this->Reslice->SetBackgroundColor(0, 0, 0, 0); // only first two are used
  this->Reslice->AutoCropOutputOff();
  this->Reslice->SetOptimization(1);
  this->Reslice->SetOutputOrigin( 0, 0, 0 );
  this->Reslice->SetOutputSpacing( 1, 1, 1 );
  this->Reslice->SetOutputDimensionality( 3 );
  this->Reslice->GenerateStencilOutputOn();

  this->ResliceUVW->SetBackgroundColor(0, 0, 0, 0); // only first two are used
  this->ResliceUVW->AutoCropOutputOff();
  this->ResliceUVW->SetOptimization(1);
  this->ResliceUVW->SetOutputOrigin( 0, 0, 0 );
  this->ResliceUVW->SetOutputSpacing( 1, 1, 1 );
  this->ResliceUVW->SetOutputDimensionality( 3 );
  this->ResliceUVW->GenerateStencilOutputOn();

  this->UpdatingTransforms = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic::~vtkMRMLSliceLayerLogic()
{
  if ( this->SliceNode )
    {
    vtkSetAndObserveMRMLNodeMacro(this->SliceNode, 0 );
    }
  if ( this->VolumeNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeNode, 0 );
    }
  if ( this->VolumeDisplayNodeObserved )
    {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeDisplayNodeObserved , 0 );
    }

  this->SetSliceNode(nullptr);
  this->SetVolumeNode(nullptr);
  this->XYToIJKTransform->Delete();
  this->UVWToIJKTransform->Delete();

  this->Reslice->SetInputConnection( nullptr );
  this->ResliceUVW->SetInputConnection( nullptr );
  this->LabelOutline->SetInputConnection( nullptr );
  this->LabelOutlineUVW->SetInputConnection( nullptr );

  this->Reslice->Delete();
  this->ResliceUVW->Delete();

  this->LabelOutline->Delete();
  this->LabelOutlineUVW->Delete();

  this->AssignAttributeTensorsToScalars->Delete();
  this->AssignAttributeScalarsToTensors->Delete();
  this->AssignAttributeScalarsToTensorsUVW->Delete();

  if ( this->VolumeDisplayNode )
    {
    this->VolumeDisplayNode->Delete();
    }

  if ( this->VolumeDisplayNodeUVW )
    {
    this->VolumeDisplayNodeUVW->Delete();
    }

}

//---------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::ProcessMRMLSceneEvents(vtkObject * caller,
                                                    unsigned long event,
                                                    void *callData)
{
  // ignore node events that aren't the observed volume or slice node
  if ( vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene()
    && (event == vtkMRMLScene::NodeAddedEvent ||
        event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node =  reinterpret_cast<vtkMRMLNode*> (callData);
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    if (node == nullptr ||
        // Care only about volume and slice nodes
        (!volumeNode && !sliceNode) ||
        // Care only if the node is the observed volume node
        (volumeNode && volumeNode != this->VolumeNode) ||
        // Care only if the node is the observed slice node
        (sliceNode && sliceNode != this->SliceNode))
      {
      return;
      }
    }
  this->UpdateLogic();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateLogic()
{
  // TBD: make sure UpdateTransforms() is not called for not a good reason as it
  // is expensive.
  int wasModifying = this->StartModify();
  this->UpdateTransforms();
  this->UpdateImageDisplay();
  this->UpdateGlyphs();
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::ProcessMRMLNodesEvents(vtkObject * caller,
                                                    unsigned long event,
                                                    void *callData)
{
  switch (event)
    {
    case vtkMRMLTransformableNode::TransformModifiedEvent:
      if (caller == this->VolumeNode)
        {// TBD: Needed ?
        this->UpdateLogic();
        }
      break;
    default:
      this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::OnMRMLNodeModified(vtkMRMLNode *node)
{
  if (node == this->VolumeDisplayNodeObserved)
    {
    this->UpdateVolumeDisplayNode();
    int wasModifying = this->StartModify();
    this->UpdateImageDisplay();
    // Maybe the pipeline hasn't changed, but we know that the display node has changed
    // so the output has changed.
    this->Modified();
    this->EndModify(wasModifying);
    }
  else if (node == this->SliceNode ||
           node == this->VolumeNode)
    {
    this->UpdateLogic();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::SetSliceNode(vtkMRMLSliceNode *sliceNode)
{
  if ( sliceNode == this->SliceNode )
    {
    return;
    }
  bool wasModifying = this->StartModify();
  vtkSetAndObserveMRMLNodeMacro( this->SliceNode, sliceNode );

  // Update the reslice transform to move this image into XY
  this->UpdateTransforms();
  this->UpdateImageDisplay();
  this->UpdateGlyphs();

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::SetVolumeNode(vtkMRMLVolumeNode *volumeNode)
{
  if (this->VolumeNode == volumeNode)
    {
    return;
    }
  int wasModifying = this->StartModify();

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeNode, volumeNode, events.GetPointer());

  // Update the reslice transform to move this image into XY
  this->UpdateTransforms();
  this->UpdateImageDisplay();
  this->UpdateGlyphs();

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateNodeReferences ()
{
  // if there's a display node, observe it
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode;
  vtkSmartPointer<vtkMRMLDiffusionTensorDisplayPropertiesNode> dtPropNode;

  if ( this->VolumeNode )
    {
    const char *id = this->VolumeNode->GetDisplayNodeID();
    if (id)
      {
      if (this->GetMRMLScene())
        {
        displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID(id));
        }
      }
    else
      {
      // TODO: this is a hack
      vtkDebugMacro("UpdateNodeReferences: Volume Node " << this->VolumeNode->GetID() << " doesn't have a display node, adding one.");
      if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode.TakeReference(vtkMRMLDiffusionTensorVolumeDisplayNode::New());
        dtPropNode.TakeReference(vtkMRMLDiffusionTensorDisplayPropertiesNode::New());
        }
      else if (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode.TakeReference(vtkMRMLDiffusionWeightedVolumeDisplayNode::New());
        }
      else if (vtkMRMLVectorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode.TakeReference(vtkMRMLVectorVolumeDisplayNode::New());
        }
      else if (vtkMRMLLabelMapVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode.TakeReference(vtkMRMLLabelMapVolumeDisplayNode::New());
        }
      else if (vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode.TakeReference(vtkMRMLScalarVolumeDisplayNode::New());
        }

      displayNode->SetScene(this->GetMRMLScene());
      if (this->GetMRMLScene())
        {
        this->GetMRMLScene()->AddNode(displayNode);
        }

      if (dtPropNode)
        {
        dtPropNode->SetScene(this->GetMRMLScene());
        if (this->GetMRMLScene())
          {
          this->GetMRMLScene()->AddNode(dtPropNode);
          }
        displayNode->SetAndObserveColorNodeID(dtPropNode->GetID());
        }

      displayNode->SetDefaultColorMap();

      this->VolumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      }
    }

    if ( displayNode != this->VolumeDisplayNodeObserved &&
         this->VolumeDisplayNode != nullptr)
      {
      vtkDebugMacro("vtkMRMLSliceLayerLogic::UpdateNodeReferences: new display node = " << (displayNode == nullptr ? "null" : "valid") << endl);
      this->VolumeDisplayNode->Delete();
      this->VolumeDisplayNode = nullptr;
      }

    if ( displayNode != this->VolumeDisplayNodeObserved &&
         this->VolumeDisplayNodeUVW != nullptr)
      {
      vtkDebugMacro("vtkMRMLSliceLayerLogic::UpdateNodeReferences: new display node = " << (displayNode == nullptr ? "null" : "valid") << endl);
      this->VolumeDisplayNodeUVW->Delete();
      this->VolumeDisplayNodeUVW = nullptr;
      }
    // vtkSetAndObserveMRMLNodeMacro could fire an event but we want to wait
    // after UpdateVolumeDisplayNode is called to fire it.
    if (this->VolumeDisplayNodeObserved != displayNode)
      {
      bool wasModifying = this->StartModify();
      vtkSetAndObserveMRMLNodeMacro(this->VolumeDisplayNodeObserved, displayNode);
      this->UpdateVolumeDisplayNode();
      this->EndModify(wasModifying);
      }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateVolumeDisplayNode()
{
  if (this->VolumeDisplayNode == nullptr &&
      this->VolumeDisplayNodeObserved != nullptr)
    {
    this->VolumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(
      this->VolumeDisplayNodeObserved->CreateNodeInstance());
    }
  if (this->VolumeDisplayNodeUVW == nullptr &&
      this->VolumeDisplayNodeObserved != nullptr)
    {
    this->VolumeDisplayNodeUVW = vtkMRMLVolumeDisplayNode::SafeDownCast(
      this->VolumeDisplayNodeObserved->CreateNodeInstance());
    }
  if (this->VolumeDisplayNode == nullptr ||
      this->VolumeDisplayNodeUVW == nullptr ||
      this->VolumeDisplayNodeObserved == nullptr)
    {
    return;
    }

  int wasDisabling = this->VolumeDisplayNode->StartModify();
  // copy the scene first because Copy() might need the scene
  this->VolumeDisplayNode->SetScene(this->VolumeDisplayNodeObserved->GetScene());
  this->VolumeDisplayNode->Copy(this->VolumeDisplayNodeObserved);
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode))
    {
    // Disable auto computation of CalculateScalarsWindowLevel()
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)->SetAutoWindowLevel(0);
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)->SetAutoThreshold(0);
    }
  this->VolumeDisplayNode->EndModify(wasDisabling);

  int wasDisablingUVW = this->VolumeDisplayNodeUVW->StartModify();
  // copy the scene first because Copy() might need the scene
  this->VolumeDisplayNodeUVW->SetScene(this->VolumeDisplayNodeObserved->GetScene());
  this->VolumeDisplayNodeUVW->Copy(this->VolumeDisplayNodeObserved);
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNodeUVW))
    {
    // Disable auto computation of CalculateScalarsWindowLevel()
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNodeUVW)->SetAutoWindowLevel(0);
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNodeUVW)->SetAutoThreshold(0);
    }
  this->VolumeDisplayNodeUVW->EndModify(wasDisablingUVW);

}


//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateTransforms()
{
  if (this->UpdatingTransforms)
    {
    return;
    }

  this->UpdatingTransforms = 1;

  // Ensure display node matches the one we are observing
  this->UpdateNodeReferences();

  int dimensions[3];
  dimensions[0] = 100;  // dummy values until SliceNode is set
  dimensions[1] = 100;
  dimensions[2] = 100;

  int dimensionsUVW[3];
  dimensionsUVW[0] = 100;  // dummy values until SliceNode is set
  dimensionsUVW[1] = 100;
  dimensionsUVW[2] = 100;

  vtkNew<vtkMatrix4x4> xyToIJK;
  xyToIJK->Identity();

  vtkNew<vtkMatrix4x4> uvwToIJK;
  uvwToIJK->Identity();

  this->XYToIJKTransform->Identity();
  this->UVWToIJKTransform->Identity();

  this->XYToIJKTransform->PostMultiply();
  this->UVWToIJKTransform->PostMultiply();

  if (this->SliceNode)
    {
    this->SliceNode->GetDimensions(dimensions);
    vtkMatrix4x4::Multiply4x4(this->SliceNode->GetXYToRAS(), xyToIJK.GetPointer(), xyToIJK.GetPointer());

    vtkMatrix4x4::Multiply4x4(this->SliceNode->GetUVWToRAS(), uvwToIJK.GetPointer(), uvwToIJK.GetPointer());
    this->SliceNode->GetUVWDimensions(dimensionsUVW);

    this->XYToIJKTransform->Concatenate(xyToIJK.GetPointer());
    this->UVWToIJKTransform->Concatenate(uvwToIJK.GetPointer());
    }

  if (this->VolumeNode && this->VolumeNode->GetImageData())
    {
    // Apply the transform, if it exists
    vtkMRMLTransformNode *transformNode = this->VolumeNode->GetParentTransformNode();
    if ( transformNode != nullptr )
      {
      vtkNew<vtkGeneralTransform> worldTransform;
      worldTransform->Identity();
      transformNode->GetTransformFromWorld(worldTransform.GetPointer());
      //worldTransform->Inverse();

      this->XYToIJKTransform->Concatenate(worldTransform.GetPointer());
      this->UVWToIJKTransform->Concatenate(worldTransform.GetPointer());
      }

    vtkNew<vtkMatrix4x4> rasToIJK;
    this->VolumeNode->GetRASToIJKMatrix(rasToIJK.GetPointer());

    this->XYToIJKTransform->Concatenate(rasToIJK.GetPointer());
    this->UVWToIJKTransform->Concatenate(rasToIJK.GetPointer());

    // vtkImageReslice works faster if the input is a linear transform, so try to convert it
    // to a linear transform.
    // Also attempt to make it a permute transform, as it makes reslicing even faster.
    vtkSmartPointer<vtkTransform> linearXYToIJKTransform = vtkSmartPointer<vtkTransform>::New();
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(this->XYToIJKTransform, linearXYToIJKTransform))
      {
      SnapToPermuteMatrix(linearXYToIJKTransform);
      this->Reslice->SetResliceTransform(linearXYToIJKTransform);
      }
    else
      {
      this->Reslice->SetResliceTransform(this->XYToIJKTransform);
      }
    vtkSmartPointer<vtkTransform> linearUVWToIJKTransform = vtkSmartPointer<vtkTransform>::New();
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(this->UVWToIJKTransform, linearUVWToIJKTransform))
      {
      SnapToPermuteMatrix(linearUVWToIJKTransform);
      this->ResliceUVW->SetResliceTransform( linearUVWToIJKTransform );
      }
    else
      {
      this->ResliceUVW->SetResliceTransform( this->UVWToIJKTransform );
      }

  }

  /***
  // Optimisation: If there is no volume, calling or not Modified() won't
  // have any visual impact. the transform has no sense if there is no volume
  bool transformModified = this->VolumeNode &&
    !AreMatricesEqual(this->XYToIJKTransform->GetMatrix(), xyToIJK.GetPointer());
  if (transformModified)
    {
    this->XYToIJKTransform->SetMatrix(xyToIJK.GetPointer());
    }

  bool transformModifiedUVW = this->VolumeNode &&
    !AreMatricesEqual(this->UVWToIJKTransform->GetMatrix(), uvwToIJK.GetPointer());
  if (transformModifiedUVW)
    {
    this->UVWToIJKTransform->SetMatrix(uvwToIJK.GetPointer());
    }
  ***/

  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);

  this->ResliceUVW->SetOutputExtent( 0, dimensionsUVW[0]-1,
                                     0, dimensionsUVW[1]-1,
                                     0, dimensionsUVW[2]-1);

  this->UpdatingTransforms = 0;

  //if (transformModified || transformModifiedUVW)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLayerLogic::GetImageData()
{
  if ( this->GetVolumeNode() == nullptr || this->GetVolumeDisplayNode() == nullptr)
    {
    return nullptr;
    }
  return this->GetVolumeDisplayNode()->GetOutputImageData();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLayerLogic::GetImageDataConnection()
{
  if ( this->GetVolumeNode() == nullptr || this->GetVolumeDisplayNode() == nullptr)
    {
    return nullptr;
    }
  return this->GetVolumeDisplayNode()->GetOutputImageDataConnection();
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLayerLogic::GetImageDataUVW()
{
  if ( this->GetVolumeNode() == nullptr || this->GetVolumeDisplayNodeUVW() == nullptr)
    {
    return nullptr;
    }
  return this->GetVolumeDisplayNodeUVW()->GetOutputImageData();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLayerLogic::GetImageDataConnectionUVW()
{
  if ( this->GetVolumeNode() == nullptr || this->GetVolumeDisplayNodeUVW() == nullptr)
    {
    return nullptr;
    }
  return this->GetVolumeDisplayNodeUVW()->GetOutputImageDataConnection();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateImageDisplay()
{
  vtkMRMLVolumeDisplayNode *volumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLVolumeDisplayNode *volumeDisplayNodeUVW = vtkMRMLVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNodeUVW);
  vtkMRMLLabelMapVolumeDisplayNode *labelMapVolumeDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLScalarVolumeDisplayNode *scalarVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (this->VolumeNode);

  if (this->VolumeNode == nullptr)
    {
    return;
    }

  vtkImageData *imageData = this->VolumeNode->GetImageData();
  if (imageData != nullptr &&
      (imageData->GetScalarType() == VTK_LONG ||
       imageData->GetScalarType() == VTK_UNSIGNED_LONG ||
       imageData->GetScalarType() == VTK_LONG_LONG ||
       imageData->GetScalarType() == VTK_UNSIGNED_LONG_LONG))
    {
    vtkErrorMacro("Reslicing can only be done on types representable as double.  Node " <<
      this->VolumeNode->GetName() << " has image data of type " << imageData->GetScalarTypeAsString());
    return;
    }

  vtkMTimeType oldReSliceMTime = this->Reslice->GetMTime();
  vtkMTimeType oldReSliceUVWMTime = this->ResliceUVW->GetMTime();
  vtkMTimeType oldAssign = this->AssignAttributeTensorsToScalars->GetMTime();
  vtkMTimeType oldLabel = this->LabelOutline->GetMTime();
  vtkMTimeType oldLabelUVW = this->LabelOutlineUVW->GetMTime();

  if ( (this->VolumeNode->GetImageData() && labelMapVolumeDisplayNode) ||
       (scalarVolumeDisplayNode && scalarVolumeDisplayNode->GetInterpolate() == 0))
    {
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    this->ResliceUVW->SetInterpolationModeToNearestNeighbor();
    }
  else
    {
    this->Reslice->SetInterpolationModeToLinear();
    this->ResliceUVW->SetInterpolationModeToLinear();
    }

  // for tensors reassign scalar data
  if ( volumeNode && volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    vtkImageData* image = nullptr;
      vtkAlgorithmOutput* imageDataConnection = volumeNode->GetImageDataConnection();
      if (imageDataConnection)
        {
        imageDataConnection->GetProducer()->UpdateInformation();
        image = vtkImageData::SafeDownCast(
          imageDataConnection->GetProducer()->GetOutputDataObject(imageDataConnection->GetIndex()));
        vtkDataArray* tensors = image ? image->GetPointData()->GetTensors() : nullptr;

        // HACK: vtkAssignAttribute fails to propagate the tensor array scalar to its
        // output image data scalar type. It reuses what scalar type was
        // previously set on the SCALARS array. See VTK#14692
        vtkDataObject::SetPointDataActiveScalarInfo(
          imageDataConnection->GetProducer()->GetOutputInformation(0),
          tensors ? tensors->GetDataType() : VTK_FLOAT,
          tensors ? tensors->GetNumberOfComponents() : 9);
        // HACK: vtkAssignAttribute needs the tensor array to "have a name"/"be active".
        // It seems it is already the case, no need for the hack. See VTK#14693
        // vtkDataObject::SetActiveAttributeInfo(imageDataConnection->GetProducer()->GetOutputInformation(0),
        //                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
        //                                       vtkDataSetAttributes::TENSORS,
        //                                       "tensors",-1,9,-1);
        this->AssignAttributeTensorsToScalars->SetInputConnection(imageDataConnection);
        }
      else
        {
        this->AssignAttributeTensorsToScalars->SetInputConnection(imageDataConnection);
        }
      this->Reslice->SetInputConnection( this->AssignAttributeTensorsToScalars->GetOutputPort() );
      this->ResliceUVW->SetInputConnection( this->AssignAttributeTensorsToScalars->GetOutputPort() );

      this->AssignAttributeScalarsToTensors->SetInputConnection(this->Reslice->GetOutputPort() );
      // don't activate 3D UVW reslice pipeline if we use single 2D reslice pipeline
      if (this->SliceNode && this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
        {
          this->AssignAttributeScalarsToTensorsUVW->SetInputConnection(this->ResliceUVW->GetOutputPort() );
        }
      else
        {
          this->AssignAttributeScalarsToTensorsUVW->SetInputConnection(nullptr);
        }
    bool verbose = false;
    if (image && verbose)
      {
      this->AssignAttributeScalarsToTensors->UpdateInformation();
      std::cerr << "Image\n";
      std::cerr << " typ: " << image->GetScalarType() << std::endl;
      image->GetPointData()->Print(std::cerr);
      vtkImageData* assignTensorToScalarsOutput = this->AssignAttributeTensorsToScalars->GetImageDataOutput();
      std::cerr << "\nAssignTensorToScalar output: \n";
      std::cerr << "type: " << assignTensorToScalarsOutput->GetScalarType() << std::endl;
      assignTensorToScalarsOutput->GetPointData()->Print(std::cerr);
      vtkPointData* reslicePointData = this->Reslice->GetOutput()->GetPointData();
      std::cerr << "\nReslice output: \n";
      std::cerr << "type: " << this->Reslice->GetOutput()->GetScalarType() << std::endl;
      reslicePointData->Print(std::cerr);
      vtkImageData* assignScalarsToTensorOutput = this->AssignAttributeScalarsToTensors->GetImageDataOutput();
      std::cerr << "\nAssignScalarToTensor output: \n";
      std::cerr << " typ: " << assignScalarsToTensorOutput->GetScalarType() << std::endl;
      assignScalarsToTensorOutput->GetPointData()->Print(std::cerr);
      }
    }
  else if (volumeNode)
    {
    //std::cout << "volumeNode->GetImageData()" << volumeNode->GetImageData() << std::endl;
//    if (volumeNode->GetImageData())
//      {
//      volumeNode->GetImageData()->Print(std::cout);
//      }
    this->Reslice->SetInputData(volumeNode->GetImageData());
    this->ResliceUVW->SetInputData(volumeNode->GetImageData());
    // use the label outline if we have a label map volume, this is the label
    // layer (turned on in slice logic when the label layer is instantiated)
    // and the slice node is set to use it.
    if (this->GetIsLabelLayer() &&
        labelMapVolumeDisplayNode &&
        this->SliceNode && this->SliceNode->GetUseLabelOutline() )
      {
      vtkDebugMacro("UpdateImageDisplay: volume node (not diff tensor), using label outline");
      this->LabelOutline->SetInputConnection( this->Reslice->GetOutputPort() );
      int outlineThickness = labelMapVolumeDisplayNode->GetSliceIntersectionThickness();
      this->LabelOutline->SetOutline(outlineThickness);
      // don't activate 3D UVW reslice pipeline if we use single 2D reslice pipeline
      if (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
        {
        this->LabelOutlineUVW->SetInputConnection( this->ResliceUVW->GetOutputPort() );
        }
      else
        {
        this->LabelOutlineUVW->SetInputConnection( nullptr );
        }
      }
    else
      {
        this->LabelOutline->SetInputConnection(nullptr);
        this->LabelOutlineUVW->SetInputConnection(nullptr);
      }
    }

  if (volumeDisplayNode)
    {
    if (volumeNode != nullptr && volumeNode->GetImageData() != nullptr)
      {
      volumeDisplayNode->SetInputImageDataConnection(this->GetSliceImageDataConnection());
      volumeDisplayNode->SetBackgroundImageStencilDataConnection(this->Reslice->GetOutputPort(1));
      }
    }
  if (volumeDisplayNodeUVW)
    {
    if (volumeNode != nullptr && volumeNode->GetImageData() != nullptr)
      {
      //int wasModifying = volumeDisplayNode->StartModify();
      volumeDisplayNodeUVW->SetInputImageDataConnection(this->GetSliceImageDataConnectionUVW());
      volumeDisplayNodeUVW->SetBackgroundImageStencilDataConnection(this->ResliceUVW->GetOutputPort(1));
      //volumeDisplayNode->EndModify(wasModifying);
      }
    }

  if ( oldReSliceMTime != this->Reslice->GetMTime() ||
       oldReSliceUVWMTime != this->ResliceUVW->GetMTime() ||
       oldAssign != this->AssignAttributeTensorsToScalars->GetMTime() ||
       oldLabel != this->LabelOutline->GetMTime() ||
       oldLabelUVW != this->LabelOutlineUVW->GetMTime() ||
       (volumeNode != nullptr && (volumeNode->GetMTime() > oldReSliceMTime)) ||
       (volumeDisplayNode != nullptr && (volumeDisplayNode->GetMTime() > oldReSliceMTime)) ||
       (volumeDisplayNodeUVW != nullptr && (volumeDisplayNodeUVW->GetMTime() > oldReSliceUVWMTime))
       )
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLayerLogic::GetSliceImageDataConnection()
{
  if (this->GetIsLabelLayer() &&
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)&&
      this->SliceNode && this->SliceNode->GetUseLabelOutline() )
    {
    return this->LabelOutline->GetOutputPort();
    }
  if (this->VolumeNode && this->VolumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    return this->AssignAttributeScalarsToTensors->GetOutputPort();
    }
  return this->Reslice->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLayerLogic::GetSliceImageDataConnectionUVW()
{
  // don't activate 3D UVW reslice pipeline if we use single 2D reslice pipeline
  if (this->SliceNode == nullptr || this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    return nullptr;
    }

  if (this->GetIsLabelLayer() &&
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNodeUVW)&&
      this->SliceNode && this->SliceNode->GetUseLabelOutline() )
    {
    return this->LabelOutlineUVW->GetOutputPort();
    }
  if (this->VolumeNode && this->VolumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    return this->AssignAttributeScalarsToTensorsUVW->GetOutputPort();
    }
  return this->ResliceUVW->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateGlyphs()
{
  if ( !this->VolumeNode )
    {
    return;
    }
  vtkAlgorithmOutput *sliceImagePort = this->GetSliceImageDataConnection();

  vtkMRMLGlyphableVolumeDisplayNode *displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( this->VolumeNode->GetDisplayNode() );
  if ( !displayNode )
    {
    return;
    }
  int displayNodesModified = 0;
  std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes( this->VolumeNode );
  for (unsigned int n=0; n<dnodes.size(); n++)
    {
    vtkMRMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[n];
    if (this->GetSliceNode() != nullptr &&
        !strcmp(this->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
      {
      vtkMRMLTransformNode* tnode = this->VolumeNode->GetParentTransformNode();
      vtkNew<vtkMatrix4x4> transformToWorld;
      //transformToWorld->Identity();unnecessary, transformToWorld is already identity
      if (tnode != nullptr && tnode->IsTransformToWorldLinear())
        {
        tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
        transformToWorld->Invert();
        }

      vtkMatrix4x4* xyToRas = this->SliceNode->GetXYToRAS();

      vtkMatrix4x4::Multiply4x4(transformToWorld.GetPointer(), xyToRas, transformToWorld.GetPointer());
      double dirs[3][3];
      this->VolumeNode->GetIJKToRASDirections(dirs);
      vtkNew<vtkMatrix4x4> trot;
      //trot->Identity(); unnecessary, trot is already identity
      for (int i=0; i<3; i++)
        {
        for (int j=0; j<3; j++)
          {
          trot->SetElement(i, j, dirs[i][j]);
          }
        }
      // Calling SetSlicePositionMatrix() and SetSliceGlyphRotationMatrix()
      // would update the glyph filter twice. Fire a modified() event only
      // once
      int blocked = dnode->StartModify();
      dnode->SetSliceImagePort(sliceImagePort);
      dnode->SetSlicePositionMatrix(transformToWorld.GetPointer());
      dnode->SetSliceGlyphRotationMatrix(trot.GetPointer());
      displayNodesModified += dnode->EndModify(blocked);
      }
    }
  if (displayNodesModified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "SlicerSliceLayerLogic:             " << this->GetClassName() << "\n";

  if (this->VolumeNode)
    {
    os << indent << "VolumeNode: ";
    os << (this->VolumeNode->GetID() ? this->VolumeNode->GetID() : "(null ID)") << "\n";
    this->VolumeNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "VolumeNode: (none)\n";
    }

  if (this->SliceNode)
    {
    os << indent << "SliceNode: ";
    os << (this->SliceNode->GetID() ? this->SliceNode->GetID() : "(null ID)") << "\n";
    this->SliceNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceNode: (none)\n";
    }

  if (this->VolumeDisplayNode)
    {
    os << indent << "VolumeDisplayNode: ";
    os << (this->VolumeDisplayNode->GetID() ? this->VolumeDisplayNode->GetID() : "(null ID)") << "\n";
    this->VolumeDisplayNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "VolumeDisplayNode: (none)\n";
    }

  if (this->VolumeDisplayNodeUVW)
    {
    os << indent << "VolumeDisplayNodeUVW: ";
    os << (this->VolumeDisplayNodeUVW->GetID() ? this->VolumeDisplayNodeUVW->GetID() : "(null ID)") << "\n";
    this->VolumeDisplayNodeUVW->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "VolumeDisplayNodeUVW: (none)\n";
    }

  os << indent << "Reslice:\n";
  if (this->Reslice)
    {
    this->Reslice->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (0)\n";
    }

  os << indent << "ResliceUVW:\n";
  if (this->ResliceUVW)
    {
    this->ResliceUVW->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (0)\n";
    }

  os << indent << "IsLabelLayer: " << this->GetIsLabelLayer() << "\n";
  os << indent << "LabelOutline:\n";
  if (this->LabelOutline)
    {
    this->LabelOutline->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (0)\n";
    }

  os << indent << "LabelOutlineUVW:\n";
  if (this->LabelOutlineUVW)
    {
    this->LabelOutlineUVW->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (0)\n";
    }
}
