/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceGlyphLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerSliceGlyphLogic.h"

#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkPointData.h"

#include "vtkDiffusionTensorMathematics.h"

vtkCxxRevisionMacro(vtkSlicerSliceGlyphLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceGlyphLogic);


//----------------------------------------------------------------------------
vtkSlicerSliceGlyphLogic::vtkSlicerSliceGlyphLogic()
{
  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;
  this->SliceNode = NULL;

  this->XYToIJKTransform = vtkTransform::New();

  // Create the parts for the scalar layer pipeline
  this->Reslice = vtkImageReslice::New();

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
  this->AssignAttributeTensorsFromScalars= vtkAssignAttribute::New();
  this->AssignAttributeScalarsFromTensors= vtkAssignAttribute::New();
  this->AssignAttributeTensorsFromScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
  this->AssignAttributeScalarsFromTensors->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);


  this->Reslice->SetBackgroundColor(128, 0, 0, 0); // only first two are used
  this->Reslice->AutoCropOutputOff();
  this->Reslice->SetOptimization(1);
  this->Reslice->SetOutputOrigin( 0, 0, 0 );
  this->Reslice->SetOutputSpacing( 1, 1, 1 );
  this->Reslice->SetOutputDimensionality( 2 );

  this->LookupTable = NULL;

  this->SlicerGlyphLogic = vtkSlicerGlyphLogic::New();
  
  if (this->VolumeDisplayNode != NULL
      && this->VolumeDisplayNode->GetColorNode() != NULL)
    {
    // if there's a volume display node which has a valid color node, use it's
    // look up table
    // std::cout << "slicer slice layer logic, getting the colour node to " << this->VolumeDisplayNode->GetColorNode()->GetID() << "\n";
      this->LookupTable = this->VolumeDisplayNode->GetColorNode()->GetLookupTable();
    }
}

//----------------------------------------------------------------------------
vtkSlicerSliceGlyphLogic::~vtkSlicerSliceGlyphLogic()
{
  if ( this->SliceNode ) 
    {
    vtkSetAndObserveMRMLNodeMacro(this->SliceNode, NULL );
    }
  if ( this->VolumeNode ) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeNode, NULL );
    }
  if ( this->VolumeDisplayNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeDisplayNode , NULL );
    }
  
  this->SetSliceNode(NULL);
  this->SetVolumeNode(NULL);
  if (this->XYToIJKTransform)
    {
    this->XYToIJKTransform->Delete();
    }

  this->Reslice->SetInput( NULL );

  if (this->Reslice)
    {
    this->Reslice->Delete();
    }
  if ( this->DTIReslice)
    {
    this->DTIReslice->Delete();
    }
  if (this->DWIExtractComponent)
    {
    this->DWIExtractComponent->Delete();
    }
  if (this->DTIMathematics)
    {
    this->DTIMathematics->Delete();
    }
  if (this->AssignAttributeTensorsFromScalars)
    {
    this->AssignAttributeTensorsFromScalars->Delete();
    }
  if (this->AssignAttributeScalarsFromTensors)
    {
    this->AssignAttributeScalarsFromTensors->Delete();
    }
  if (this->SlicerGlyphLogic)
    {
    this->SlicerGlyphLogic->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void *callData)
{
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node =  reinterpret_cast<vtkMRMLNode*> (callData);
    if (node == NULL || !(node->IsA("vtkMRMLVolumeNode") || node->IsA("vtkMRMLSliceNode")))
      {
      return;
      }
    }

  if (this->VolumeDisplayNode == vtkMRMLVolumeDisplayNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    // reset the colour look up table
    if (this->VolumeDisplayNode != NULL
      && this->VolumeDisplayNode->GetColorNode() != NULL)
      {
      vtkDebugMacro("vtkSlicerSliceGlyphLogic::ProcessMRMLEvents: got a volume display node modified event, updating the map to colors!\n");
      }
    vtkMRMLDiffusionTensorVolumeDisplayNode *dtiVDN = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(caller);
    if (this->VolumeDisplayNode == dtiVDN && dtiVDN != NULL)
      {
      if (dtiVDN->GetDiffusionTensorDisplayPropertiesNode())
        {
        this->DTIMathematics->SetOperation(dtiVDN->GetDiffusionTensorDisplayPropertiesNode()->GetScalarInvariant());
        }
      }
    else
      {      
      vtkDebugMacro("vtkSlicerSliceGlyphLogic::ProcessMRMLEvents: volume display node " << (this->VolumeDisplayNode == NULL ? " is null" : "is set, but") << ", not updating map to colors (color node may be null)\n");
      if (this->VolumeDisplayNode)
        {
        this->LookupTable = this->VolumeDisplayNode->GetColorNode()->GetLookupTable();
        }
      }
    }
  this->UpdateTransforms();
  
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::SetSliceNode(vtkMRMLSliceNode *sliceNode)
{
  if ( sliceNode != this->SliceNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->SliceNode, sliceNode );

    // Update the reslice transform to move this image into XY
    this->UpdateTransforms();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::SetVolumeNode(vtkMRMLVolumeNode *volumeNode)
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeNode, volumeNode, events );
  events->Delete();
  if ( volumeNode )
    {
    this->LookupTable = volumeNode->GetVolumeDisplayNode()->GetColorNode()->GetLookupTable();
    // Update the reslice transform to move this image into XY
    this->UpdateTransforms();
    this->UpdatePipeline();    
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::UpdateNodeReferences ()
{
  // if there's a display node, observe it
  vtkMRMLVolumeDisplayNode *displayNode = NULL;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = NULL;
  if ( this->VolumeNode )
    {
    const char *id = this->VolumeNode->GetDisplayNodeID();
    if (id)
      {
      displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(id));
      }
    else
      {
      // TODO: this is a hack
      vtkErrorMacro("UpdateNodeReferences: Volume Node " << this->VolumeNode->GetID() << " doesn't have a display node, adding one.");
      int isLabelMap =0;
      if (vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode = vtkMRMLVolumeDisplayNode::New();
        isLabelMap = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNode)->GetLabelMap();
        }
      else if (vtkMRMLVectorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode = vtkMRMLVectorVolumeDisplayNode::New();
        }
      else if (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
        }
      else if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeNode))
        {
        displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
        propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
        }
      displayNode->SetScene(this->MRMLScene);
      this->MRMLScene->AddNode(displayNode);

      if (propNode)
        {
        propNode->SetScene(this->MRMLScene);
        this->MRMLScene->AddNode(propNode);
        displayNode->SetAndObserveColorNodeID(propNode->GetID());
        }

      displayNode->SetDefaultColorMap();
        
      this->VolumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      displayNode->Delete();
      }
    }

    if ( displayNode == this->VolumeDisplayNode )
      {
      return;
      }
    vtkDebugMacro("vtkSlicerSliceGlyphLogic::UpdateNodeReferences: new display node = " << (displayNode == NULL ? "null" : "valid") << endl);
    if ( displayNode )
      {
      vtkSetAndObserveMRMLNodeMacro( this->VolumeDisplayNode, displayNode );
      }
    else if (this->VolumeDisplayNode)
      {
      vtkSetMRMLNodeMacro( this->VolumeDisplayNode, NULL );
      }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::UpdatePipeline()
{

  vtkErrorMacro("About to reslice");
  this->Reslice->SetInterpolationModeToLinear();

  if ( this->GetVolumeNode() && this->GetVolumeNode()->GetImageData() )
  {
    vtkImageData* imageData = this->GetVolumeNode()->GetImageData(); 
    vtkIndent indent;
    

    if ( this->GetVolumeNode()->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
      // TODO: return for now since it crashes
        return;

      imageData->PrintSelf(std::cout,indent);

      vtkErrorMacro("Starting interchange and reslice pipeline");
  
  
      this->AssignAttributeTensorsFromScalars->SetInput(imageData);
      this->AssignAttributeTensorsFromScalars->Update();
      vtkImageData* InterchangedImage =  vtkImageData::SafeDownCast(this->AssignAttributeTensorsFromScalars->GetOutput());
      InterchangedImage->SetNumberOfScalarComponents(9);

      this->Reslice->SetInput( InterchangedImage );
      this->Reslice->Update();

      //Fixing horrible bug of the vtkSetAttributes Filter it doesn't copy attributes without name
      this->Reslice->GetOutput()->GetPointData()->GetScalars()->SetName(imageData->GetPointData()->GetTensors()->GetName());

      this->AssignAttributeScalarsFromTensors->SetInput(this->Reslice->GetOutput() );
      this->AssignAttributeScalarsFromTensors->Update();
      imageData = vtkImageData::SafeDownCast(this->AssignAttributeScalarsFromTensors->GetOutput());
      this->Reslice->GetOutput()->PrintSelf(std::cout,indent);

      vtkErrorMacro("Interchange and reslice pipeline done");
    } else {
      this->Reslice->SetInput( this->GetVolumeNode()->GetImageData());
      this->Reslice->Update();
      imageData = this->Reslice->GetOutput();
    }

    this->SlicerGlyphLogic->SetImageData( imageData ); 
    vtkErrorMacro("Setting the volumeDisplayNode to the GlyphLogic: "<<this->GetVolumeNode()->GetDisplayNode());
    this->SlicerGlyphLogic->SetVolumeDisplayNode( this->GetVolumeNode()->GetVolumeDisplayNode() );
  }
 

}
//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::UpdateTransforms()
{
  //int labelMap = 0; UNUSED // keep track so label maps don't get interpolated

  // Ensure display node matches the one we are observing
  this->UpdateNodeReferences();
  
  unsigned int dimensions[3];
  dimensions[0] = 100;  // dummy values until SliceNode is set
  dimensions[1] = 100;
  dimensions[2] = 100;

  vtkMatrix4x4 *xyToIJK = vtkMatrix4x4::New();
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
    if ( transformNode != NULL ) 
      {
      if ( !transformNode->IsTransformToWorldLinear() )
        {
        vtkErrorMacro ("non linear transforms not yet supported");
        }
      else
        {
        vtkMatrix4x4 *rasToRAS = vtkMatrix4x4::New();
        transformNode->GetMatrixTransformToWorld( rasToRAS );
        rasToRAS->Invert();
        vtkMatrix4x4::Multiply4x4(rasToRAS, xyToIJK, xyToIJK); 
        rasToRAS->Delete();
        }
      }

    vtkMatrix4x4 *rasToIJK = vtkMatrix4x4::New();
    this->VolumeNode->GetRASToIJKMatrix(rasToIJK);
    vtkMatrix4x4::Multiply4x4(rasToIJK, xyToIJK, xyToIJK); 
    rasToIJK->Delete();

    if (this->VolumeNode->IsA("vtkMRMLScalarVolumeNode"))
      {
      this->ScalarVolumeNodeUpdateTransforms();
      }
    else if (this->VolumeNode->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
      {
      this->DiffusionWeightedVolumeNodeUpdateTransforms();
      }
    else if (this->VolumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode"))
      {
      this->DiffusionTensorVolumeNodeUpdateTransforms();
      }
    else if (this->VolumeNode->IsA("vtkMRMLVectorVolumeNode"))
      {
      this->VectorVolumeNodeUpdateTransforms();
      }
  }

  this->XYToIJKTransform->SetMatrix( xyToIJK );
  xyToIJK->Delete();

  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);
  this->DTIReslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);
  this->Modified();
}


void vtkSlicerSliceGlyphLogic::ScalarVolumeNodeUpdateTransforms()
{
  int labelMap = 0;
  double window = 0;
  double level = 0;
  int interpolate = 0;
  vtkMRMLScalarVolumeNode *scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast (this->VolumeNode);
  if ( scalarVolumeNode && scalarVolumeNode->GetLabelMap() )
    {
    labelMap = 1;
    }
  else
    {
    labelMap = 0;
    }

  vtkMRMLScalarVolumeDisplayNode *scalarVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);

  if (scalarVolumeDisplayNode)
    {
    interpolate = scalarVolumeDisplayNode->GetInterpolate();
    if (scalarVolumeDisplayNode->GetColorNode())
      {
      this->LookupTable = scalarVolumeDisplayNode->GetColorNode()->GetLookupTable();
      }
    window = scalarVolumeDisplayNode->GetWindow();
    level = scalarVolumeDisplayNode->GetLevel();
    }

//  this->ScalarSlicePipeline(scalarVolumeNode->GetImageData(), labelMap, window, level, interpolate, lookupTable, applyThreshold, lowerThreshold, upperThreshold);

  this->Reslice->SetResliceTransform( this->XYToIJKTransform ); 
}
//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::VectorVolumeNodeUpdateTransforms()
{
  // int interpolate = 0; UNUSED
  // vtkMRMLVectorVolumeNode *vectorVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast (this->VolumeNode); UNUSED

  vtkMRMLVectorVolumeDisplayNode *vectorVolumeDisplayNode = vtkMRMLVectorVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);

  if (vectorVolumeDisplayNode)
    {
    //interpolate = vectorVolumeDisplayNode->GetInterpolate();
    }

//  this->VectorSlicePipeline(vectorVolumeNode->GetImageData(), interpolate);

  this->Reslice->SetResliceTransform( this->XYToIJKTransform ); 

}

//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::DiffusionWeightedVolumeNodeUpdateTransforms()
{
  int interpolate = 0;
  vtkMRMLDiffusionWeightedVolumeNode *dwiVolumeNode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast (this->VolumeNode);
  if (dwiVolumeNode)
    {
     this->DWIExtractComponent->SetInput(dwiVolumeNode->GetImageData());
    }
  else
    {
    this->DWIExtractComponent->SetInput(NULL);
    }

  vtkMRMLDiffusionWeightedVolumeDisplayNode *dwiVolumeDisplayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);

  if (dwiVolumeDisplayNode)
    {
    this->DWIExtractComponent->SetComponents(dwiVolumeDisplayNode->GetDiffusionComponent());
    interpolate = dwiVolumeDisplayNode->GetInterpolate();
    if (dwiVolumeDisplayNode->GetColorNode())
      {
      this->LookupTable = dwiVolumeDisplayNode->GetColorNode()->GetLookupTable();
      }
    }

//  this->ScalarSlicePipeline(this->DWIExtractComponent->GetOutput(),0,window,level,interpolate, lookupTable, applyThreshold,lowerThreshold,upperThreshold);

  this->Reslice->SetResliceTransform( this->XYToIJKTransform ); 
}


//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::DiffusionTensorVolumeNodeUpdateTransforms()
{

  // double window = 0; UNUSED
  // double level = 0; UNUSED
  int interpolate = 0;
  // int applyThreshold = 0; UNUSED
  // double lowerThreshold = 0; UNUSED
  // double upperThreshold = 0; UNUSED
  vtkImageData *inVol;

  vtkMRMLDiffusionTensorVolumeNode *dtiVolumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast (this->VolumeNode);

  if (dtiVolumeNode)
    {
     inVol = this->VolumeNode->GetImageData();
     this->DTIMathematics->SetInput(0,inVol);
    }
  else
    {
    this->DTIMathematics->SetInput(0,NULL);
    }

  vtkMRMLDiffusionTensorVolumeDisplayNode *dtiVolumeDisplayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);

  if (dtiVolumeDisplayNode)
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(dtiVolumeDisplayNode->GetDiffusionTensorDisplayPropertiesNode());
      if (propNode)
        {
        this->DTIMathematics->SetOperation(propNode->GetScalarInvariant());
        }
    interpolate = dtiVolumeDisplayNode->GetInterpolate();
    if (dtiVolumeDisplayNode->GetColorNode())
      {
      this->LookupTable = dtiVolumeDisplayNode->GetColorNode()->GetLookupTable();
      }
    }

    this->DTIMathematics->Update();
//  this->ScalarSlicePipeline(this->DTIMathematics->GetOutput(),0,window,level,interpolate, lookupTable, applyThreshold,lowerThreshold,upperThreshold);

  //Set the right transformations
  this->DTIReslice->SetResliceTransform(this->XYToIJKTransform );
  this->Reslice->SetResliceTransform(this->XYToIJKTransform); 

}
//----------------------------------------------------------------------------
vtkPolyData* vtkSlicerSliceGlyphLogic::GetPolyData()
{
  return (this->SlicerGlyphLogic->GetPolyData());
/*
  if ( this->VolumeDisplayNode && this->VolumeDisplayNode->IsA("vtkMRMLVolumeGlyphDisplayNode") )
  {
    return vtkMRMLVolumeGlyphDisplayNode::SafeDownCast(this->VolumeNode)->GetPolyData();
  } else {
    return NULL;
  }
*/
}
//----------------------------------------------------------------------------
void vtkSlicerSliceGlyphLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
  
  os << indent << "SlicerSliceGlyphLogic:             " << this->GetClassName() << "\n";

  os << indent << "VolumeNode: " <<
    (this->VolumeNode ? this->VolumeNode->GetID() : "(none)") << "\n";
  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetID() : "(none)") << "\n";

  os << indent << "VolumeDisplayNode: " <<
    (this->VolumeDisplayNode ? this->VolumeDisplayNode->GetID() : "(none)") << "\n";
  if (this->VolumeDisplayNode)
    {
    this->VolumeDisplayNode->PrintSelf(os, nextIndent);
  }
 os << indent << "Reslice:\n";
  if (this->Reslice)
    {
    this->Reslice->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (NULL)\n";
    }
}

