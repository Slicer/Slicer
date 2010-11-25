/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLayerLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkDiffusionTensorMathematics.h"

//
#include "vtkImageLabelOutline.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLSliceLayerLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLSliceLayerLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic::vtkMRMLSliceLayerLogic()
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
  this->VolumeDisplayNodeObserved = 0;
  this->SliceNode = 0;
   
  this->XYToIJKTransform = vtkTransform::New();

  this->IsLabelLayer = 0;
  
  this->AssignAttributeTensorsFromScalars= vtkAssignAttribute::New();
  this->AssignAttributeScalarsFromTensors= vtkAssignAttribute::New();
  this->AssignAttributeTensorsFromScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
  this->AssignAttributeScalarsFromTensors->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);

  // Create the parts for the scalar layer pipeline
  this->Slice = vtkImageSlice::New();
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
  this->DTIReslice = vtkImageReslice::New();
  this->DTIMathematics = vtkDiffusionTensorMathematics::New();
  // Set parameters that won't change based on input
  this->DTIReslice->SetBackgroundColor(128, 0, 0, 0); // only first two are used
  this->DTIReslice->AutoCropOutputOff();
  this->DTIReslice->SetOptimization(1);
  this->DTIReslice->SetOutputOrigin( 0, 0, 0 );
  this->DTIReslice->SetOutputSpacing( 1, 1, 1 );
  this->DTIReslice->SetOutputDimensionality( 2 );


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
  this->DTIReslice->Delete();
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

  this->AssignAttributeTensorsFromScalars->Delete();
  this->AssignAttributeScalarsFromTensors->Delete();
   
  if ( this->VolumeDisplayNode )
    {
    this->VolumeDisplayNode->Delete();
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void *callData)
{
  // ignore node events that aren't volumes or slice nodes
  if ( vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene()
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node =  reinterpret_cast<vtkMRMLNode*> (callData);
    if (node == 0 || !(node->IsA("vtkMRMLVolumeNode") || node->IsA("vtkMRMLSliceNode")))
      {
      return;
      }
    }

  // ignore unimportant scene events
  if ( vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene()
    && (event == vtkMRMLScene::NewSceneEvent) )
    {
    return;
    }

  if (this->VolumeDisplayNodeObserved != 0 &&
      this->VolumeDisplayNodeObserved == vtkMRMLVolumeDisplayNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
      if (this->VolumeDisplayNode && this->VolumeDisplayNodeObserved)
        {
        this->VolumeDisplayNode->CopyWithoutModifiedEvent(this->VolumeDisplayNodeObserved);
        }
      this->UpdateImageDisplay();
      this->Modified();
      return;
    }
  this->UpdateTransforms();
  
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::SetSliceNode(vtkMRMLSliceNode *sliceNode)
{
  if ( sliceNode != this->SliceNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->SliceNode, sliceNode );

    // Update the reslice transform to move this image into XY
    if (this->SliceNode) 
      {
      this->UpdateTransforms();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::SetVolumeNode(vtkMRMLVolumeNode *volumeNode)
{
  if (this->VolumeNode != volumeNode)
    {
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeNode, volumeNode, events );
    events->Delete();

    // Update the reslice transform to move this image into XY
    if (this->VolumeNode)
      {
      this->UpdateTransforms();
      }
    }
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

    if ( displayNode == this->VolumeDisplayNodeObserved )
      {
      if (this->VolumeDisplayNode && displayNode)
        {
        this->VolumeDisplayNode->CopyWithoutModifiedEvent(displayNode);
        }
      return;
      }
    vtkDebugMacro("vtkMRMLSliceLayerLogic::UpdateNodeReferences: new display node = " << (displayNode == 0 ? "null" : "valid") << endl);
    if ( displayNode )
      {
      if (this->VolumeDisplayNode != 0)
        {
        this->VolumeDisplayNode->Delete();
        }
      this->VolumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(displayNode->CreateNodeInstance());
      // copy the scene first, as CopyWithoutModifiedEvent might need the scene
      this->VolumeDisplayNode->SetScene(displayNode->GetScene());
      this->VolumeDisplayNode->CopyWithoutModifiedEvent(displayNode);
      vtkSetAndObserveMRMLNodeMacro(this->VolumeDisplayNodeObserved, displayNode);
      }
    else 
      {
      if (this->VolumeDisplayNodeObserved)
        {
        vtkSetAndObserveNoModifyMRMLNodeMacro(this->VolumeDisplayNodeObserved, 0);
        }
      if (this->VolumeDisplayNode)
        {
        this->VolumeDisplayNode->Delete();
        this->VolumeDisplayNode = 0;
        }
      }
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
  xyToIJK->Identity();

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

  this->XYToIJKTransform->SetMatrix( xyToIJK );

  this->Slice->SetOutputDimensions( dimensions[0], dimensions[1], dimensions[2]);
  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);
  this->DTIReslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);

  this->UpdateImageDisplay();

  this->UpdatingTransforms = 0; 

  this->Modified();


}

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
  if ( this->VolumeNode->GetImageData() && labelMapVolumeDisplayNode )
    {
    this->Slice->SetInterpolationModeToNearestNeighbor();
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    }
  else
    {
    this->Slice->SetInterpolationModeToLinear();
    this->Reslice->SetInterpolationModeToLinear();
    }

   vtkImageData *slicedImageData = 0;

  // for tensors reassign scalar data
  if ( volumeNode && volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    this->AssignAttributeTensorsFromScalars->SetInput(volumeNode->GetImageData());
    this->AssignAttributeTensorsFromScalars->Update();
    
    vtkImageData* InterchangedImage =  vtkImageData::SafeDownCast(this->AssignAttributeTensorsFromScalars->GetOutput());
    if (InterchangedImage)
      {
      InterchangedImage->SetNumberOfScalarComponents(9);
      }
    this->Reslice->SetInput( InterchangedImage );
    this->Reslice->Update();

    //Fixing horrible bug of the vtkSetAttributes Filter it doesn't copy attributes without name
    if ( this->Reslice->GetOutput() && 
         this->Reslice->GetOutput()->GetPointData() && 
         this->Reslice->GetOutput()->GetPointData()->GetScalars() &&
         volumeNode->GetImageData() &&
         volumeNode->GetImageData()->GetPointData() && 
         volumeNode->GetImageData()->GetPointData()->GetTensors())
      {
      this->Reslice->GetOutput()->GetPointData()->GetScalars()->SetName(volumeNode->GetImageData()->GetPointData()->GetTensors()->GetName());
      }
    this->AssignAttributeScalarsFromTensors->SetInput(this->Reslice->GetOutput() );
    this->AssignAttributeScalarsFromTensors->Update();
    slicedImageData = vtkImageData::SafeDownCast(this->AssignAttributeScalarsFromTensors->GetOutput());
    
    } 
  else if (volumeNode) 
    {
    this->Reslice->SetInput( volumeNode->GetImageData());
    // use the label outline if we have a label map volume, this is the label
    // layer (turned on in slice logic when the label layer is instantiated)
    // and the slice node is set to use it.
    if (labelMapVolumeDisplayNode && this->GetIsLabelLayer() &&
        (this->GetSliceNode() != 0) &&
        this->GetSliceNode()->GetUseLabelOutline() )
      {
      vtkDebugMacro("UpdateImageDisplay: volume node (not diff tensor), using label outline");
      this->LabelOutline->SetInput( this->Reslice->GetOutput() );
      slicedImageData = this->LabelOutline->GetOutput();
      }
    else
      {
      slicedImageData = this->Reslice->GetOutput();
      }
    }

  if (volumeDisplayNode)
    {
    if (volumeNode != 0 && volumeNode->GetImageData() != 0)
      {
      volumeDisplayNode->SetImageData(slicedImageData);
      volumeDisplayNode->SetBackgroundImageData(this->Reslice->GetBackgroundMask());
      }
    }

  if (scalarVolumeDisplayNode && scalarVolumeDisplayNode->GetInterpolate() == 0  )
    {
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    }

  this->Slice->SetSliceTransform( this->XYToIJKTransform ); 
  this->Reslice->SetResliceTransform( this->XYToIJKTransform ); 
  
  this->UpdateGlyphs(slicedImageData);

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLayerLogic::UpdateGlyphs(vtkImageData *sliceImage)
{
  if ( this->VolumeNode )
  {
    vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (this->VolumeNode);
    vtkMRMLGlyphableVolumeDisplayNode *displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( this->VolumeNode->GetDisplayNode() );
    if (displayNode)
      {
      std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes( volumeNode );
      for (unsigned int n=0; n<dnodes.size(); n++)
        {
        vtkMRMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[n];
        if (this->GetSliceNode() != 0 &&
            !strcmp(this->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
          {
          dnode->SetSliceImage(sliceImage);

          vtkMRMLTransformNode* tnode = volumeNode->GetParentTransformNode();
          vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
          transformToWorld->Identity();
          if (tnode != 0 && tnode->IsLinear())
            {
            vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
            lnode->GetMatrixTransformToWorld(transformToWorld);
            }
          transformToWorld->Invert();

          vtkMatrix4x4* xyToRas = this->SliceNode->GetXYToRAS();

          vtkMatrix4x4::Multiply4x4(transformToWorld, xyToRas, transformToWorld); 

          dnode->SetSlicePositionMatrix(transformToWorld);
          double dirs[3][3];
          volumeNode->GetIJKToRASDirections(dirs);
          vtkSmartPointer<vtkMatrix4x4> trot = vtkSmartPointer<vtkMatrix4x4>::New();
          trot->Identity();
          for (int i=0; i<3; i++) 
            {
            for (int j=0; j<3; j++)
              {
              trot->SetElement(i, j, dirs[i][j]);
              }
            }
          dnode->SetSliceGlyphRotationMatrix(trot);
          }
        }
      }
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
