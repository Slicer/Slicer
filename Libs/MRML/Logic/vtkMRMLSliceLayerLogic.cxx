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
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageLinearReslice.h>
#include <vtkImageResliceMask.h>
#include <vtkImageReslice.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

//
#include "vtkImageLabelOutline.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLSliceLayerLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLSliceLayerLogic);

bool AreMatricesEqual(const vtkMatrix4x4* first, const vtkMatrix4x4* second)
{
  return first->GetElement(0,0) == second->GetElement(0,0) &&
         first->GetElement(0,1) == second->GetElement(0,1) &&
         first->GetElement(0,2) == second->GetElement(0,2) &&
         first->GetElement(0,3) == second->GetElement(0,3) &&
         first->GetElement(1,0) == second->GetElement(1,0) &&
         first->GetElement(1,1) == second->GetElement(1,1) &&
         first->GetElement(1,2) == second->GetElement(1,2) &&
         first->GetElement(1,3) == second->GetElement(1,3) &&
         first->GetElement(2,0) == second->GetElement(2,0) &&
         first->GetElement(2,1) == second->GetElement(2,1) &&
         first->GetElement(2,2) == second->GetElement(2,2) &&
         first->GetElement(2,3) == second->GetElement(2,3) &&
         first->GetElement(3,0) == second->GetElement(3,0) &&
         first->GetElement(3,1) == second->GetElement(3,1) &&
         first->GetElement(3,2) == second->GetElement(3,2) &&
         first->GetElement(3,3) == second->GetElement(3,3);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic::vtkMRMLSliceLayerLogic()
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
  this->VolumeDisplayNodeObserved = 0;
  this->SliceNode = 0;
   
  this->XYToIJKTransform = vtkTransform::New();

  this->IsLabelLayer = 0;
  
  this->AssignAttributeTensorsToScalars= vtkAssignAttribute::New();
  this->AssignAttributeScalarsToTensors= vtkAssignAttribute::New();
  this->AssignAttributeTensorsToScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
  this->AssignAttributeScalarsToTensors->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);

  // Create the parts for the scalar layer pipeline
  this->Slice = vtkImageLinearReslice::New();
  this->Reslice = vtkImageResliceMask::New();
  this->ResliceThreshold = vtkImageThreshold::New();
  this->ResliceAppendComponents = vtkImageAppendComponents::New();
  this->ResliceExtractLuminance = vtkImageExtractComponents::New();
  this->ResliceExtractAlpha = vtkImageExtractComponents::New();
  this->ResliceAlphaCast = vtkImageCast::New();
  this->AlphaLogic = vtkImageLogic::New();
  this->Threshold = vtkImageThreshold::New();
  this->LabelOutline = vtkImageLabelOutline::New();
  this->AppendComponents = vtkImageAppendComponents::New();

  // Create the parts for the DWI layer pipeline
  this->DWIExtractComponent = vtkImageExtractComponents::New();

  // Create the components for the DTI layer pipeline
  //this->DTIReslice = vtkImageReslice::New();
  this->DTIMathematics = vtkDiffusionTensorMathematics::New();
  // Set parameters that won't change based on input
  //this->DTIReslice->SetBackgroundColor(128, 0, 0, 0); // only first two are used
  //this->DTIReslice->AutoCropOutputOff();
  //this->DTIReslice->SetOptimization(1);
  //this->DTIReslice->SetOutputOrigin( 0, 0, 0 );
  //this->DTIReslice->SetOutputSpacing( 1, 1, 1 );
  //this->DTIReslice->SetOutputDimensionality( 2 );


  //
  // Set parameters that won't change based on input
  //
  this->Slice->SetOutputOrigin( 0, 0, 0 );
  this->Slice->SetOutputSpacing( 1, 1, 1 );

  this->Reslice->SetBackgroundColor(0, 0, 0, 0); // only first two are used
  this->Reslice->AutoCropOutputOff();
  this->Reslice->SetOptimization(1);
  this->Reslice->SetOutputOrigin( 0, 0, 0 );
  this->Reslice->SetOutputSpacing( 1, 1, 1 );
  this->Reslice->SetOutputDimensionality( 3 );
  

  this->ResliceThreshold->ThresholdBetween(1, 0); // i.e. everything is Out
  this->ResliceThreshold->ReplaceOutOn();
  this->ResliceThreshold->SetOutValue(255);

  this->ResliceAlphaCast->SetOutputScalarTypeToUnsignedChar();

  this->AlphaLogic->SetOperationToAnd();
  this->AlphaLogic->SetOutputTrueValue(255);
  
  // Only the transform matrix can change, not the transform itself
  this->Slice->SetSliceTransform( this->XYToIJKTransform ); 
  this->Reslice->SetResliceTransform( this->XYToIJKTransform ); 

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

  this->SetSliceNode(0);
  this->SetVolumeNode(0);
  this->XYToIJKTransform->Delete();

  this->Slice->SetInput( 0 );
  this->Reslice->SetInput( 0 );
  this->Threshold->SetInput( 0 );
  this->LabelOutline->SetInput( 0 );
  this->AppendComponents->SetInput( 0 );

  this->Slice->Delete();
  this->Reslice->Delete();
  //this->DTIReslice->Delete();
  this->DWIExtractComponent->Delete();
  this->DTIMathematics->Delete();
  this->Threshold->Delete();
  this->LabelOutline->Delete();
  this->AppendComponents->Delete();
  this->ResliceThreshold->Delete();
  this->ResliceAppendComponents->Delete();
  this->ResliceExtractLuminance->Delete();
  this->ResliceExtractAlpha->Delete();
  this->ResliceAlphaCast->Delete();
  this->AlphaLogic->Delete();

  this->AssignAttributeTensorsToScalars->Delete();
  this->AssignAttributeScalarsToTensors->Delete();
   
  if ( this->VolumeDisplayNode )
    {
    this->VolumeDisplayNode->Delete();
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
    if (node == 0 ||
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

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeNode, volumeNode, events );
  events->Delete();

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
  vtkMRMLVolumeDisplayNode *displayNode = 0;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = 0;

  vtkMRMLDiffusionTensorVolumeDisplayNode *dtdisplayNode = 0;
  vtkMRMLDiffusionWeightedVolumeDisplayNode *dwdisplayNode = 0;
  vtkMRMLVectorVolumeDisplayNode *vdisplayNode = 0;
  vtkMRMLScalarVolumeDisplayNode *sdisplayNode = 0;

  if ( this->VolumeNode )
    {
    const char *id = this->VolumeNode->GetDisplayNodeID();
    if (id)
      {
      displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID(id));
      }
    else
      {
      // TODO: this is a hack
      vtkErrorMacro("UpdateNodeReferences: Volume Node " << this->VolumeNode->GetID() << " doesn't have a display node, adding one.");
      int isLabelMap =0;
      if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        dtdisplayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
        displayNode= dtdisplayNode;
        propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
        }
      else if (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(this->VolumeNode))
        {
        dwdisplayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
        displayNode= dwdisplayNode;
        }
      else if (vtkMRMLVectorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        vdisplayNode = vtkMRMLVectorVolumeDisplayNode::New();
        displayNode= vdisplayNode;
        }
      else if (vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNode))
        {
        isLabelMap = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNode)->GetLabelMap();
        if (isLabelMap) 
          {
          displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
          }
        else
         {
         sdisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
         displayNode = sdisplayNode;
         }

        }
      displayNode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(displayNode);

      if (propNode)
        {
        propNode->SetScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(propNode);
        displayNode->SetAndObserveColorNodeID(propNode->GetID());
        }

      displayNode->SetDefaultColorMap();
        
      this->VolumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      displayNode->Delete();
      }
    }
    
    if ( displayNode != this->VolumeDisplayNodeObserved &&
         this->VolumeDisplayNode != 0)
      {
      vtkDebugMacro("vtkMRMLSliceLayerLogic::UpdateNodeReferences: new display node = " << (displayNode == 0 ? "null" : "valid") << endl);
      this->VolumeDisplayNode->Delete();
      this->VolumeDisplayNode = 0;
      }
    // vtkSetAndObserveMRMLNodeMacro could fire an event but we want to wait
    // after UpdateVolumeDisplayNode is called to fire it.
    bool wasModifying = this->StartModify();
    vtkSetAndObserveMRMLNodeMacro(this->VolumeDisplayNodeObserved, displayNode);
    this->UpdateVolumeDisplayNode();
    this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateVolumeDisplayNode()
{
  if (this->VolumeDisplayNode == 0 &&
      this->VolumeDisplayNodeObserved != 0)
    {
    this->VolumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(
      this->VolumeDisplayNodeObserved->CreateNodeInstance());
    }
  if (this->VolumeDisplayNode == 0 ||
      this->VolumeDisplayNodeObserved == 0)
    {
    return;
    }
  int wasDisabling = this->VolumeDisplayNode->GetDisableModifiedEvent();
  this->VolumeDisplayNode->SetDisableModifiedEvent(1);
  // copy the scene first because Copy() might need the scene
  this->VolumeDisplayNode->SetScene(this->VolumeDisplayNodeObserved->GetScene());
  this->VolumeDisplayNode->Copy(this->VolumeDisplayNodeObserved);
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode))
    {
    // Disable auto computation of CalculateScalarsWindowLevel()
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)->SetAutoWindowLevel(0);
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)->SetAutoThreshold(0);
    }
  this->VolumeDisplayNode->SetDisableModifiedEvent(wasDisabling);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateTransforms()
{
  static bool reportedNonlinearTransformSupport = false;
  
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

  vtkSmartPointer<vtkMatrix4x4> xyToIJK = vtkSmartPointer<vtkMatrix4x4>::New();

  if (this->SliceNode)
    {
    vtkMatrix4x4::Multiply4x4(this->SliceNode->GetXYToRAS(), xyToIJK, xyToIJK);
    this->SliceNode->GetDimensions(dimensions);
    }

  if (this->VolumeNode && this->VolumeNode->GetImageData())
    {
    // Apply the transform, if it exists
    vtkMRMLTransformNode *transformNode = this->VolumeNode->GetParentTransformNode();
    if ( transformNode != 0 )
      {
      if ( !transformNode->IsTransformToWorldLinear() )
        {
        if (!reportedNonlinearTransformSupport)
          {
          vtkErrorMacro ("Nonlinear transforms are not yet supported in slice viewers.");
          reportedNonlinearTransformSupport = true;
          }
        }
      else
        {
        vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
        transformNode->GetMatrixTransformToWorld( rasToRAS );
        rasToRAS->Invert();
        vtkMatrix4x4::Multiply4x4(rasToRAS, xyToIJK, xyToIJK); 
        }
      }

    vtkSmartPointer<vtkMatrix4x4> rasToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
    this->VolumeNode->GetRASToIJKMatrix(rasToIJK);
    vtkMatrix4x4::Multiply4x4(rasToIJK, xyToIJK, xyToIJK); 
  }

  // Optimisation: If there is no volume, calling or not Modified() won't
  // have any visual impact. the transform has no sense if there is no volume
  bool transformModified = this->VolumeNode &&
    !AreMatricesEqual(this->XYToIJKTransform->GetMatrix(), xyToIJK);
  if (transformModified)
    {
    this->XYToIJKTransform->SetMatrix( xyToIJK );
    }

  this->Slice->SetOutputDimensions( dimensions[0], dimensions[1], dimensions[2]);
  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);
  //this->DTIReslice->SetOutputExtent( 0, dimensions[0]-1,
  //                                0, dimensions[1]-1,
  //                                0, dimensions[2]-1);

  this->UpdatingTransforms = 0; 

  if (transformModified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLayerLogic::GetImageData()
{
  if ( this->GetVolumeNode() == NULL || this->GetVolumeDisplayNode() == NULL)
    {
    return NULL;
    }
  return this->GetVolumeDisplayNode()->GetImageData();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateImageDisplay()
{
  vtkMRMLVolumeDisplayNode *volumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLLabelMapVolumeDisplayNode *labelMapVolumeDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLScalarVolumeDisplayNode *scalarVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (this->VolumeNode);

  if (this->VolumeNode == 0)
    {
    return;
    }
  
  unsigned long oldSliceMTime = this->Slice->GetMTime();
  unsigned long oldReSliceMTime = this->Reslice->GetMTime();
  unsigned long oldAssign = this->AssignAttributeTensorsToScalars->GetMTime();
  unsigned long oldLabel = this->LabelOutline->GetMTime();
  
  if ( (this->VolumeNode->GetImageData() && labelMapVolumeDisplayNode) ||
       (scalarVolumeDisplayNode && scalarVolumeDisplayNode->GetInterpolate() == 0))
    {
    this->Slice->SetInterpolationModeToNearestNeighbor();
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    }
  else
    {
    this->Slice->SetInterpolationModeToLinear();
    this->Reslice->SetInterpolationModeToLinear();
    }

  // for tensors reassign scalar data
  if ( volumeNode && volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
      vtkImageData* image = volumeNode->GetImageData();
      vtkDataArray* tensors = image ? image->GetPointData()->GetTensors() : 0;
      /*
      vtkImageData* image = vtkImageData::New();
      image->SetDimensions(2,1,1);

      vtkNew<vtkFloatArray> tensors;
      tensors->SetName("tensors");
      tensors->SetNumberOfComponents(9);
      // 2 tuples, identity matrices
      tensors->InsertNextTuple9(1.,0.,0.,0.,1.,0.,0.,0.,1.);
      tensors->InsertNextTuple9(1.,0.,0.,0.,1.,0.,0.,0.,1.);

      image->GetPointData()->SetTensors(tensors.GetPointer());
      */
      /// HACK !
      /// vtkAssignAttribute is not able to set these values automatically,
      /// we do it manually instead.
      if (image)
        {
        image->SetScalarType(tensors ? tensors->GetDataType() : VTK_FLOAT);
        image->SetNumberOfScalarComponents(tensors ? tensors->GetNumberOfComponents() : 1);
        }
      /// END of HACK
/*      {
      vtkNew<vtkAssignAttribute> assign;
      assign->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
      assign->SetInput(image);
      
      vtkDataObject::SetActiveAttributeInfo(image->GetPipelineInformation(), 
                                            vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                            vtkDataSetAttributes::TENSORS,
                                            "tensors",-1,9,-1);
      vtkNew<vtkImageReslice> cast;
      cast->SetInputConnection(assign->GetOutputPort());

      vtkNew<vtkAssignAttribute> assignBack;
      assignBack->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);
      assignBack->SetInputConnection(cast->GetOutputPort());

      assignBack->Update();

      vtkImageData* imageOut = vtkImageData::SafeDownCast(assign->GetOutput());
      vtkImageData* imageCasted = vtkImageData::SafeDownCast(cast->GetOutput());
      vtkImageData* imageBack = vtkImageData::SafeDownCast(assignBack->GetOutput());

    //if (imageBack->GetPointData()->GetNumberOfComponents() != 9)
      {
      cerr << "Input: \n";
      image->GetPointData()->Print(cerr);
      cerr << "Intermediate: \n";
      imageOut->GetPointData()->Print(cerr);
      cerr << "Casted: \n";
      imageCasted->GetPointData()->Print(cerr);
      cerr << "Back: \n";
      imageBack->GetPointData()->Print(cerr);
      }
      }*/
    if (image)
      {
      this->AssignAttributeTensorsToScalars->SetInput(image);
      /// HACK !
      /// vtkAssignAttribute is not able to set these values automatically,
      /// we do it manually instead.
      vtkDataObject::SetActiveAttributeInfo(
        image->GetPipelineInformation(),
        vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::TENSORS,
        tensors->GetName(), tensors->GetDataType(),
        tensors->GetNumberOfComponents(), tensors->GetNumberOfTuples());
      /// End of HACK !
      }
    this->Reslice->SetInput( this->AssignAttributeTensorsToScalars->GetImageDataOutput() );

    this->AssignAttributeScalarsToTensors->SetInput(this->Reslice->GetOutput() );
    
    bool verbose = false;
    if (image && verbose)
      {
      this->AssignAttributeScalarsToTensors->Update();
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
    this->Reslice->SetInput( volumeNode->GetImageData());
    // use the label outline if we have a label map volume, this is the label
    // layer (turned on in slice logic when the label layer is instantiated)
    // and the slice node is set to use it.
    if (this->GetIsLabelLayer() &&
        labelMapVolumeDisplayNode && 
        this->SliceNode && this->SliceNode->GetUseLabelOutline() )
      {
      vtkDebugMacro("UpdateImageDisplay: volume node (not diff tensor), using label outline");
      this->LabelOutline->SetInput( this->Reslice->GetOutput() );
      }
    else
      {
      this->LabelOutline->SetInput(0);
      }
    }

  if (volumeDisplayNode)
    {
    if (volumeNode != 0 && volumeNode->GetImageData() != 0)
      {
      //int wasModifying = volumeDisplayNode->StartModify();
      volumeDisplayNode->SetInputImageData(this->GetSliceImageData());
      volumeDisplayNode->SetBackgroundImageData(this->Reslice->GetBackgroundMask());
      // If the background mask is not used, make sure the update extent of the
      // background mask is set to the whole extent so the reslice filter can write
      // into the entire extent instead of trying to access an update extent that won't
      // be up-to-date because not connected to a pipeline.
      if (volumeDisplayNode->GetBackgroundImageData() == 0 &&
          this->Reslice->GetBackgroundMask() != 0)
        {
        this->Reslice->GetBackgroundMask()->SetUpdateExtentToWholeExtent();
        }
      //volumeDisplayNode->EndModify(wasModifying);
      }
    }

  if ( oldSliceMTime != this->Slice->GetMTime() ||
       oldReSliceMTime != this->Reslice->GetMTime() ||
       oldAssign != this->AssignAttributeTensorsToScalars->GetMTime() ||
       oldLabel != this->LabelOutline->GetMTime() ||
       (volumeNode != 0 && (volumeNode->GetMTime() > oldReSliceMTime)) ||
       (volumeDisplayNode != 0 && (volumeDisplayNode->GetMTime() > oldReSliceMTime))
       )
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLayerLogic::GetSliceImageData()
{
  if (this->GetIsLabelLayer() &&
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode)&& 
      this->SliceNode && this->SliceNode->GetUseLabelOutline() )
    {
    return this->LabelOutline->GetOutput();
    }
  if (this->VolumeNode && this->VolumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    return this->AssignAttributeScalarsToTensors->GetImageDataOutput();
    }
  return this->Reslice->GetOutput();
}
    
//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateGlyphs()
{
  if ( !this->VolumeNode )
    {
    return;
    }
  vtkImageData *sliceImage = this->GetSliceImageData();

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
    if (this->GetSliceNode() != 0 &&
        !strcmp(this->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
      {
      vtkMRMLTransformNode* tnode = this->VolumeNode->GetParentTransformNode();
      vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
      //transformToWorld->Identity();unnecessary, transformToWorld is already identiy
      if (tnode != 0 && tnode->IsLinear())
        {
        vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
        lnode->GetMatrixTransformToWorld(transformToWorld);
        transformToWorld->Invert();
        }

      vtkMatrix4x4* xyToRas = this->SliceNode->GetXYToRAS();

      vtkMatrix4x4::Multiply4x4(transformToWorld, xyToRas, transformToWorld);
      double dirs[3][3];
      this->VolumeNode->GetIJKToRASDirections(dirs);
      vtkSmartPointer<vtkMatrix4x4> trot = vtkSmartPointer<vtkMatrix4x4>::New();
      //trot->Identity(); unnecessary, trot is already identiy
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
      dnode->SetSliceImage(sliceImage);
      dnode->SetSlicePositionMatrix(transformToWorld);
      dnode->SetSliceGlyphRotationMatrix(trot);
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

  os << indent << "Slice:\n";
  if (this->Slice)
    {
    this->Slice->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (0)\n";
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
}
