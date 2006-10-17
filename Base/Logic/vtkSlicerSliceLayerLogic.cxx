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
#include "vtkMRMLTransformNode.h"


vtkCxxRevisionMacro(vtkSlicerSliceLayerLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLayerLogic);


//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::vtkSlicerSliceLayerLogic()
{
  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;
  this->SliceNode = NULL;

  this->XYToIJKTransform = vtkTransform::New();

  // Create the parts for the layer pipeline
  this->Reslice = vtkImageReslice::New();
  this->ResliceThreshold = vtkImageThreshold::New();
  this->ResliceAppendComponents = vtkImageAppendComponents::New();
  this->ResliceExtractLuminance = vtkImageExtractComponents::New();
  this->ResliceExtractAlpha = vtkImageExtractComponents::New();
  this->ResliceAlphaCast = vtkImageCast::New();
  this->AlphaLogic = vtkImageLogic::New();
  this->MapToColors = vtkImageMapToColors::New();
  this->LookupTable = vtkLookupTable::New();
  this->Threshold = vtkImageThreshold::New();
  this->AppendComponents = vtkImageAppendComponents::New();
  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();

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
  this->MapToColors->SetLookupTable( this->LookupTable );
  this->LookupTable->SetRampToLinear();
  this->LookupTable->SetTableRange(0, 255);
  this->LookupTable->SetHueRange(0, 0);
  this->LookupTable->SetSaturationRange(0, 0);
  this->LookupTable->SetValueRange(0, 1);
  this->LookupTable->SetAlphaRange(1, 1); // not used
  this->LookupTable->Build();

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
  this->LookupTable->Delete();
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
void vtkSlicerSliceLayerLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
{
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
    this->ResliceAppendComponents->SetInput(0, this->VolumeNode->GetImageData() ); 
    this->ResliceAppendComponents->SetInput(1, this->ResliceThreshold->GetOutput() );
    this->Reslice->SetInput( this->ResliceAppendComponents->GetOutput() ); 

    }
  else
    {

    this->ResliceAppendComponents->RemoveAllInputs();
    this->ResliceAppendComponents->SetInput( 0, NULL ); 
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

    this->AppendComponents->RemoveAllInputs();
    this->AppendComponents->SetInput(0, this->MapToColors->GetOutput() );

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

    this->AppendComponents->SetInput(1, this->AlphaLogic->GetOutput() );

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
void vtkSlicerSliceLayerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerSliceLayerLogic:             " << this->GetClassName() << "\n";

  os << indent << "VolumeNode: " <<
    (this->VolumeNode ? this->VolumeNode->GetID() : "(none)") << "\n";
  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetID() : "(none)") << "\n";
  // TODO: fix printing of vtk objects
  os << indent << "Reslice: " <<
    (this->Reslice ? "this->Reslice" : "(none)") << "\n";
  os << indent << "MapToColors: " <<
    (this->MapToColors ? "this->MapToColors" : "(none)") << "\n";
  os << indent << "MapToWindowLevelColors: " <<
    (this->MapToWindowLevelColors ? "this->MapToWindowLevelColors" : "(none)") << "\n";
}

