/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceLayerLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerSliceLayerLogic.h"

#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLColorNode.h"

vtkCxxRevisionMacro(vtkSlicerSliceLayerLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLayerLogic);


//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::vtkSlicerSliceLayerLogic()
{
  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;
  this->SliceNode = NULL;

  this->XYToIJKTransform = vtkTransform::New();

  // Create the parts for the scalar layer pipeline
  this->Reslice = vtkImageReslice::New();
  this->ResliceThreshold = vtkImageThreshold::New();
  this->ResliceAppendComponents = vtkImageAppendComponents::New();
  this->ResliceExtractLuminance = vtkImageExtractComponents::New();
  this->ResliceExtractAlpha = vtkImageExtractComponents::New();
  this->ResliceAlphaCast = vtkImageCast::New();
  this->AlphaLogic = vtkImageLogic::New();
  this->MapToColors = vtkImageMapToColors::New();
  this->Threshold = vtkImageThreshold::New();
  this->AppendComponents = vtkImageAppendComponents::New();
  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();

  // Create the parts for the DWI layer pipeline
  this->DWIExtractComponent = vtkImageExtractComponents::New();

  // Set parameters that won't change based on input
  this->Reslice->SetBackgroundColor(128, 0, 0, 0); // only first two are used
  this->Reslice->AutoCropOutputOff();
  this->Reslice->SetOptimization(1);
  this->Reslice->SetOutputOrigin( 0, 0, 0 );
  this->Reslice->SetOutputSpacing( 1, 1, 1 );
  this->Reslice->SetOutputDimensionality( 2 );

  this->ResliceThreshold->ThresholdBetween(1, 0); // i.e. everything is Out
  this->ResliceThreshold->ReplaceOutOn();
  this->ResliceThreshold->SetOutValue(255);

  this->ResliceAlphaCast->SetOutputScalarTypeToUnsignedChar();

  this->AlphaLogic->SetOperationToAnd();
  this->AlphaLogic->SetOutputTrueValue(255);

  this->MapToColors->SetOutputFormatToRGB();

  if (this->VolumeDisplayNode != NULL
      && this->VolumeDisplayNode->GetColorNode() != NULL)
    {
    // if there's a volume display node which has a valid color node, use it's
    // look up table
    this->MapToColors->SetLookupTable( this->VolumeDisplayNode->GetColorNode()->GetLookupTable());
    }
}

//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::~vtkSlicerSliceLayerLogic()
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
  this->XYToIJKTransform->Delete();

  this->Reslice->SetInput( NULL );
  this->Threshold->SetInput( NULL );
  this->AppendComponents->SetInput( NULL );
  this->MapToWindowLevelColors->SetInput( NULL );
  this->MapToColors->SetInput( NULL );

  this->Reslice->Delete();
  this->MapToColors->Delete();
  this->Threshold->Delete();
  this->AppendComponents->Delete();
  this->MapToWindowLevelColors->Delete();
  this->ResliceThreshold->Delete();
  this->ResliceAppendComponents->Delete();
  this->ResliceExtractLuminance->Delete();
  this->ResliceExtractAlpha->Delete();
  this->ResliceAlphaCast->Delete();
  this->AlphaLogic->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void * /*callData*/)
{
  
  if (this->VolumeDisplayNode == vtkMRMLVolumeDisplayNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    // reset the colour look up table
    if (this->VolumeDisplayNode != NULL
      && this->VolumeDisplayNode->GetColorNode() != NULL)
      {
      vtkDebugMacro("vtkSlicerSliceLayerLogic::ProcessMRMLEvents: got a volume display node modified event, updating the map to colors!\n");
      this->MapToColors->SetLookupTable( this->VolumeDisplayNode->GetColorNode()->GetLookupTable());
      }
    else
      {      
      vtkDebugMacro("vtkSlicerSliceLayerLogic::ProcessMRMLEvents: volume display node " << (this->VolumeDisplayNode == NULL ? " is null" : "is set, but") << ", not updating map to colors (color node may be null)\n");
      }
    }
  this->UpdateTransforms();
  
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetSliceNode(vtkMRMLSliceNode *sliceNode)
{
  if ( sliceNode != this->SliceNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->SliceNode, sliceNode );

    // Update the reslice transform to move this image into XY
    this->UpdateTransforms();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetVolumeNode(vtkMRMLVolumeNode *volumeNode)
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeNode, volumeNode, events );
  events->Delete();

  // Update the reslice transform to move this image into XY
  this->UpdateTransforms();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::UpdateNodeReferences ()
{
  // if there's a display node, observe it
  vtkMRMLVolumeDisplayNode *displayNode = NULL;
  if ( this->VolumeNode )
    {
    const char *id = this->VolumeNode->GetDisplayNodeID();
    if (id)
      {
      displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(id));
      }
    }

    if ( displayNode == this->VolumeDisplayNode )
      {
      return;
      }
    vtkDebugMacro("vtkSlicerSliceLayerLogic::UpdateNodeReferences: new display node = " << (displayNode == NULL ? "null" : "valid") << endl);
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
void vtkSlicerSliceLayerLogic::UpdateTransforms()
{
  int labelMap = 0;  // keep track so label maps don't get interpolated

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
  this->Reslice->SetResliceTransform( this->XYToIJKTransform );

  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);

  this->Modified();


}


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::UpdateTransformsOLD()
{
  int labelMap = 0;  // keep track so label maps don't get interpolated

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

  if (this->VolumeNode)
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


    vtkMRMLScalarVolumeNode *scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast (this->VolumeNode);
    if ( scalarVolumeNode && scalarVolumeNode->GetLabelMap() )
      {
      labelMap = 1;
      this->Reslice->SetInterpolationModeToNearestNeighbor();
      }
    else
      {
      this->Reslice->SetInterpolationModeToLinear();
      }

    // Prime the imaging pipeline
    // - add an alpha channel to the input data
    this->ResliceThreshold->SetInput( this->VolumeNode->GetImageData() ); 
    this->ResliceAppendComponents->RemoveAllInputs();
      
    this->ResliceAppendComponents->SetInputConnection(0, this->VolumeNode->GetImageData()->GetProducerPort());
    this->ResliceAppendComponents->AddInputConnection(0, this->ResliceThreshold->GetOutput()->GetProducerPort());

    this->Reslice->SetInput( this->ResliceAppendComponents->GetOutput() ); 

    }
  else
    {

    this->ResliceAppendComponents->RemoveAllInputs();
    //this->ResliceAppendComponents->SetInput( 0, NULL ); 
    this->ResliceThreshold->SetInput( NULL ); 
    this->Reslice->SetInput( NULL ); 

    }


  if (this->VolumeDisplayNode)
    {
    //
    // Configure the imaging pipeline
    //
    // - make an alpha channel for the image data from the node
    // - perform the reslice 
    // - extract the luminance and alpha for individual processing
    // -- use the luminance to get a second alpha channel
    // -- or the two alpha channels
    // - run the luminance through the window level and color maps
    // - append the alpha channel to the final RGB image
    //

    this->ResliceExtractLuminance->SetInput(this->Reslice->GetOutput() );
    this->ResliceExtractLuminance->SetComponents(0);

    this->ResliceExtractAlpha->SetInput(this->Reslice->GetOutput() );
    this->ResliceExtractAlpha->SetComponents(1);
    this->ResliceAlphaCast->SetInput( this->ResliceExtractAlpha->GetOutput() );
        
    if ( this->VolumeDisplayNode->GetInterpolate() && !labelMap )
      {
      this->Reslice->SetInterpolationModeToLinear();
      }
    else
      {
      this->Reslice->SetInterpolationModeToNearestNeighbor();
      }

    // update the lookup table
    if (this->VolumeDisplayNode->GetColorNode())
      {
      if (this->VolumeDisplayNode->GetColorNode()->GetLookupTable() != this->MapToColors->GetLookupTable())
        {
        vtkDebugMacro("vtkSlicerSliceLayerLogic::UpdateTransforms: volume display node lut isn't the same as the map to colours lut, resetting the map to cols\n");
        this->MapToColors->SetLookupTable(this->VolumeDisplayNode->GetColorNode()->GetLookupTable());
        }
      }
    else
      {
      vtkDebugMacro("vtkSlicerSliceLayerLogic::UpdateTransforms: volume display node doesn't have a color node, not updating the map to colours\n");
      }
    if ( labelMap ) 
      {
      // Don't put label maps through the window/level filter,
      // because this will map them to unsigned char
      this->MapToColors->SetInput( this->ResliceExtractLuminance->GetOutput() );
      } 
    else
      {
      // a non-label map is windowed first, then mapped through lookup table
      this->MapToWindowLevelColors->SetWindow(this->VolumeDisplayNode->GetWindow());
      this->MapToWindowLevelColors->SetLevel(this->VolumeDisplayNode->GetLevel());

      this->MapToWindowLevelColors->SetInput( this->ResliceExtractLuminance->GetOutput() );
      this->MapToWindowLevelColors->SetOutputFormatToLuminance();
      this->MapToColors->SetInput( this->MapToWindowLevelColors->GetOutput() );
      }

    this->Threshold->SetInput( this->ResliceExtractLuminance->GetOutput() );
    this->Threshold->SetOutputScalarTypeToUnsignedChar();

    if ( this->VolumeDisplayNode->GetApplyThreshold() )
      {
      this->Threshold->ReplaceInOn();
      this->Threshold->SetInValue(255);
      this->Threshold->ReplaceOutOn();
      this->Threshold->SetOutValue(0);
      this->Threshold->ThresholdBetween( this->VolumeDisplayNode->GetLowerThreshold(), 
                                         this->VolumeDisplayNode->GetUpperThreshold() );
      }
    else
      {
      vtkMRMLScalarVolumeNode *scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast (this->VolumeNode);
      if ( scalarVolumeNode->GetLabelMap() )
        {
        // don't apply threshold - let it come from the label map
        this->Threshold->ReplaceInOff();
        this->Threshold->ReplaceOutOff();
        } 
      else
        {
        // don't apply threshold - alpha channel becomes 255 everywhere
        this->Threshold->ThresholdBetween( 1, 0 ); 
        this->Threshold->ReplaceInOn();
        this->Threshold->SetInValue(255);
        this->Threshold->ReplaceOutOn();
        this->Threshold->SetOutValue(255);
        }
      }

    this->AlphaLogic->SetInput1( this->ResliceAlphaCast->GetOutput() );
    this->AlphaLogic->SetInput2( this->Threshold->GetOutput() );

    this->AppendComponents->RemoveAllInputs();
    this->AppendComponents->SetInputConnection(0, this->MapToColors->GetOutput()->GetProducerPort() );
    this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
    }

  this->XYToIJKTransform->SetMatrix( xyToIJK );
  xyToIJK->Delete();
  this->Reslice->SetResliceTransform( this->XYToIJKTransform );

  this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                  0, dimensions[1]-1,
                                  0, dimensions[2]-1);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::ScalarVolumeNodeUpdateTransforms()
{
  int labelMap = 0;
  double window = 0;
  double level = 0;
  int interpolate = 0;
  int applyThreshold = 0;
  double lowerThreshold = 0;
  double upperThreshold = 0;
  vtkLookupTable *lookupTable = NULL;
  vtkMRMLScalarVolumeNode *scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast (this->VolumeNode);
  if ( scalarVolumeNode && scalarVolumeNode->GetLabelMap() )
    {
    labelMap = 1;
    }
  else
    {
    labelMap = 0;
    }

  vtkMRMLVolumeDisplayNode *scalarVolumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(this->VolumeDisplayNode);

  if (scalarVolumeDisplayNode)
    {
    interpolate = scalarVolumeDisplayNode->GetInterpolate();
    if (scalarVolumeDisplayNode->GetColorNode())
      {
      lookupTable = scalarVolumeDisplayNode->GetColorNode()->GetLookupTable();
      }
    window = scalarVolumeDisplayNode->GetWindow();
    level = scalarVolumeDisplayNode->GetLevel();
    applyThreshold = scalarVolumeDisplayNode->GetApplyThreshold();
    lowerThreshold = scalarVolumeDisplayNode->GetLowerThreshold();
    upperThreshold = scalarVolumeDisplayNode->GetUpperThreshold();
    }

  this->ScalarSlicePipeline(scalarVolumeNode->GetImageData(), labelMap, window, level, interpolate, lookupTable, applyThreshold, lowerThreshold, upperThreshold);

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::VectorVolumeNodeUpdateTransforms()
{

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::DiffusionWeightedVolumeNodeUpdateTransforms()
{
  double window = 0;
  double level = 0;
  int interpolate = 0;
  int applyThreshold = 0;
  double lowerThreshold = 0;
  double upperThreshold = 0;
  vtkLookupTable *lookupTable = NULL;
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

  //cout<<"display node: "<<dwiVolumeDisplayNode->GetClassName()<<endl;
  if (dwiVolumeDisplayNode)
    {
    this->DWIExtractComponent->SetComponents(dwiVolumeDisplayNode->GetDiffusionComponent());
    interpolate = dwiVolumeDisplayNode->GetInterpolate();
    if (dwiVolumeDisplayNode->GetColorNode())
      {
      lookupTable = dwiVolumeDisplayNode->GetColorNode()->GetLookupTable();
      }
    window = dwiVolumeDisplayNode->GetWindow();
    level = dwiVolumeDisplayNode->GetLevel();
    applyThreshold = dwiVolumeDisplayNode->GetApplyThreshold();
    lowerThreshold = dwiVolumeDisplayNode->GetLowerThreshold();
    upperThreshold = dwiVolumeDisplayNode->GetUpperThreshold();
    }

 

  this->ScalarSlicePipeline(this->DWIExtractComponent->GetOutput(),0,window,level,interpolate, lookupTable, applyThreshold,lowerThreshold,upperThreshold);

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::DiffusionTensorVolumeNodeUpdateTransforms()
{

}


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::ScalarSlicePipeline(vtkImageData *imageData, int labelMap, double window, double level, int interpolate, vtkLookupTable *lookupTable, int applyThreshold, double lowerThreshold, double upperThreshold)
{

  if ( imageData && labelMap )
    {
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    }
  else
    {
    this->Reslice->SetInterpolationModeToLinear();
    }

  // Prime the imaging pipeline
  // - add an alpha channel to the input data
  this->ResliceThreshold->SetInput( imageData ); 
  this->ResliceAppendComponents->RemoveAllInputs();

  this->ResliceAppendComponents->SetInputConnection(0, imageData->GetProducerPort());
  this->ResliceAppendComponents->AddInputConnection(0, this->ResliceThreshold->GetOutput()->GetProducerPort());

  this->Reslice->SetInput( this->ResliceAppendComponents->GetOutput() ); 

    //
    // Configure the imaging pipeline
    //
    // - make an alpha channel for the image data from the node
    // - perform the reslice 
    // - extract the luminance and alpha for individual processing
    // -- use the luminance to get a second alpha channel
    // -- or the two alpha channels
    // - run the luminance through the window level and color maps
    // - append the alpha channel to the final RGB image
    //

  this->ResliceExtractLuminance->SetInput(this->Reslice->GetOutput() );
  this->ResliceExtractLuminance->SetComponents(0);

  this->ResliceExtractAlpha->SetInput(this->Reslice->GetOutput() );
  this->ResliceExtractAlpha->SetComponents(1);
  this->ResliceAlphaCast->SetInput( this->ResliceExtractAlpha->GetOutput() );

  if ( interpolate && !labelMap )
    {
    this->Reslice->SetInterpolationModeToLinear();
    }
  else
    {
    this->Reslice->SetInterpolationModeToNearestNeighbor();
    }

  // update the lookup table
  if (lookupTable)
    {
    if (lookupTable != this->MapToColors->GetLookupTable())
      {
      vtkDebugMacro("vtkSlicerSliceLayerLogic::UpdateTransforms: volume display node lut isn't the same as the map to colours lut, resetting the map to cols\n");
      this->MapToColors->SetLookupTable(lookupTable);
      }
    }
  else
    {
    vtkDebugMacro("vtkSlicerSliceLayerLogic::UpdateTransforms: volume display node doesn't have a color node, not updating the map to colours\n");
    }
  if ( labelMap ) 
    {
    // Don't put label maps through the window/level filter,
    // because this will map them to unsigned char
    this->MapToColors->SetInput( this->ResliceExtractLuminance->GetOutput() );
    } 
  else
    {
    // a non-label map is windowed first, then mapped through lookup table
    this->MapToWindowLevelColors->SetWindow(window);
    this->MapToWindowLevelColors->SetLevel(level);

    this->MapToWindowLevelColors->SetInput( this->ResliceExtractLuminance->GetOutput() );
    this->MapToWindowLevelColors->SetOutputFormatToLuminance();
    this->MapToColors->SetInput( this->MapToWindowLevelColors->GetOutput() );
    }

  this->Threshold->SetInput( this->ResliceExtractLuminance->GetOutput() );
  this->Threshold->SetOutputScalarTypeToUnsignedChar();

  if ( applyThreshold )
    {
    this->Threshold->ReplaceInOn();
    this->Threshold->SetInValue(255);
    this->Threshold->ReplaceOutOn();
    this->Threshold->SetOutValue(0);
    this->Threshold->ThresholdBetween( lowerThreshold, 
                                         upperThreshold );
    }
  else
    {
    if ( labelMap )
      {
      // don't apply threshold - let it come from the label map
      this->Threshold->ReplaceInOff();
      this->Threshold->ReplaceOutOff();
      } 
    else
      {
      // don't apply threshold - alpha channel becomes 255 everywhere
      this->Threshold->ThresholdBetween( 1, 0 ); 
      this->Threshold->ReplaceInOn();
      this->Threshold->SetInValue(255);
      this->Threshold->ReplaceOutOn();
      this->Threshold->SetOutValue(255);
      }
    }

  this->AlphaLogic->SetInput1( this->ResliceAlphaCast->GetOutput() );
  this->AlphaLogic->SetInput2( this->Threshold->GetOutput() );

  this->AppendComponents->RemoveAllInputs();
  this->AppendComponents->SetInputConnection(0, this->MapToColors->GetOutput()->GetProducerPort() );
  this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
  }


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
  
  os << indent << "SlicerSliceLayerLogic:             " << this->GetClassName() << "\n";

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
  os << indent << "MapToColors:\n";
  if (this->MapToColors)
    {
    this->MapToColors->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (NULL)\n";
    }
  os << indent << "MapToWindowLevelColors:\n";
  if (this->MapToWindowLevelColors)
    {
    this->MapToWindowLevelColors->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << " (NULL)\n";
    }
}

