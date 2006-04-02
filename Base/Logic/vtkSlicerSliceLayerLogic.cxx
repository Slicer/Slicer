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
#include "vtkSlicerSliceLayerLogic.h"

vtkCxxRevisionMacro(vtkSlicerSliceLayerLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLayerLogic);

//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::vtkSlicerSliceLayerLogic()
{
  this->VolumeNode = NULL;
  this->SliceNode = NULL;

  this->XYToIJKTransform = vtkTransform::New();

  this->Reslice = vtkImageReslice::New();
  this->MapToRGBA = vtkImageMapToRGBA::New();
  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();

  this->MapToWindowLevelColors->SetInput( this->Reslice->GetOutput() );
  this->MapToRGBA->SetInput( this->MapToWindowLevelColors->GetOutput() );

}

//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::~vtkSlicerSliceLayerLogic()
{
    this->SetSliceNode(NULL);
    this->SetVolumeNode(NULL);

    this->XYToIJKTransform->Delete();
    this->Reslice->Delete();
    this->MapToRGBA->Delete();
    this->MapToWindowLevelColors->Delete();
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetSliceNode(vtkMRMLSliceNode *SliceNode)
{
  if (this->SliceNode)
    {
    this->SliceNode->Delete();
    }
  this->SliceNode = SliceNode;
  if (this->SliceNode)
    {  
    this->SliceNode->Register(this);
    }

  // Update the reslice transform to move this image into XY
  this->UpdateTransforms();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetVolumeNode(vtkMRMLVolumeNode *VolumeNode)
{
  if (this->VolumeNode)
    {
    this->VolumeNode->Delete();
    }

  if ( VolumeNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    this->VolumeNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (VolumeNode);
    }

  this->VolumeNode->Register(this);

  if (this->VolumeNode)
    {
    this->Reslice->SetInput( this->VolumeNode->GetImageData() ); 
    }
    else
    {
    this->Reslice->SetInput( NULL ); 
    }

  // Update the reslice transform to move this image into XY
  this->UpdateTransforms();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::UpdateTransforms()
{
    unsigned int dimensions[3];
    dimensions[0] = 100;  // dummy values until SliceNode is set
    dimensions[1] = 100;
    dimensions[2] = 100;

    vtkMatrix4x4 *m = this->XYToIJKTransform->GetMatrix();
    m->Identity();

    if (this->SliceNode)
      {
      vtkMatrix4x4::Multiply4x4(this->SliceNode->GetXYToRAS(), m, m);
      this->SliceNode->GetDimensions(dimensions);
      }

    if (this->VolumeNode)
      {
      vtkMatrix4x4 *rasToIJK = vtkMatrix4x4::New();
      this->VolumeNode->GetRASToIJKMatrix(rasToIJK);
      vtkMatrix4x4::Multiply4x4(rasToIJK, m, m); 
      }

    this->Reslice->SetResliceTransform( this->XYToIJKTransform );
    // TODO: why don't these work?
    //this->Reslice->SetOutputOrigin( 0, 0, 0 );
    //this->Reslice->SetOutputSpacing( 1, 1, 1 );
    this->Reslice->SetOutputExtent( 0, dimensions[0]-1,
                                    0, dimensions[1]-1,
                                    0, dimensions[2]-1);
    this->Reslice->SetOutputDimensionality( 2 );
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerSliceLayerLogic:             " << this->GetClassName() << "\n";

  os << indent << "VolumeNode: " <<
    (this->VolumeNode ? this->VolumeNode->GetName() : "(none)") << "\n";
  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetName() : "(none)") << "\n";
  // TODO: fix printing of vtk objects
  os << indent << "Reslice: " <<
    (this->Reslice ? "this->Reslice" : "(none)") << "\n";
  os << indent << "MapToRGBA: " <<
    (this->MapToRGBA ? "this->MapToRGBA" : "(none)") << "\n";
  os << indent << "MapToWindowLevelColors: " <<
    (this->MapToWindowLevelColors ? "this->MapToWindowLevelColors" : "(none)") << "\n";

}

