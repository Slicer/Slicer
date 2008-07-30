/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGlyphableVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkTransformPolyDataFilter.h"

#include "vtkMRMLGlyphableVolumeSliceDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLGlyphableVolumeSliceDisplayNode* vtkMRMLGlyphableVolumeSliceDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGlyphableVolumeSliceDisplayNode");
  if(ret)
    {
    return (vtkMRMLGlyphableVolumeSliceDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGlyphableVolumeSliceDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLGlyphableVolumeSliceDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGlyphableVolumeSliceDisplayNode");
  if(ret)
    {
    return (vtkMRMLGlyphableVolumeSliceDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGlyphableVolumeSliceDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLGlyphableVolumeSliceDisplayNode::vtkMRMLGlyphableVolumeSliceDisplayNode()
{


  this->ColorMode = this->colorModeScalar;
  
  this->SliceToXYTransformer = vtkTransformPolyDataFilter::New();

  this->SliceToXYTransform = vtkTransform::New();
  
  this->SliceToXYMatrix = vtkMatrix4x4::New();
  this->SliceToXYMatrix->Identity();
  this->SliceToXYTransform->PreMultiply();
  this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);

  //this->SliceToXYTransformer->SetInput(this->GlyphGlyphFilter->GetOutput());
  this->SliceToXYTransformer->SetTransform(this->SliceToXYTransform);
}


//----------------------------------------------------------------------------
vtkMRMLGlyphableVolumeSliceDisplayNode::~vtkMRMLGlyphableVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->SliceToXYMatrix->Delete();
  this->SliceToXYTransform->Delete();
  this->SliceToXYTransformer->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{

  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " colorMode =\"" << this->ColorMode << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "colorMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorMode;
      }

    }  


}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLGlyphableVolumeSliceDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLGlyphableVolumeSliceDisplayNode *node = (vtkMRMLGlyphableVolumeSliceDisplayNode *) anode;

  this->SetColorMode(node->ColorMode);
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix)
{
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
//  if (this->GlyphGlyphFilter)
//    {
//    this->GlyphGlyphFilter->SetVolumePositionMatrix(matrix);
//    }
  this->SliceToXYMatrix->DeepCopy(matrix);
  this->SliceToXYMatrix->Invert();
  if (this->SliceToXYTransform)
    {
    this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSliceImage(vtkImageData *image)
{
/*
    if (this->GlyphGlyphFilter)
    {
    this->GlyphGlyphFilter->SetInput(image);
    this->GlyphGlyphFilter->SetDimensions(image->GetDimensions());
    }
*/
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetPolyData(vtkPolyData *glyphPolyData)
{
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLGlyphableVolumeSliceDisplayNode::GetPolyData()
{
    return NULL;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLGlyphableVolumeSliceDisplayNode::GetPolyDataTransformedToSlice()
{
    return NULL;
}
//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdatePolyDataPipeline() 
{
  vtkErrorMacro("Shouldn't be calling this");
}

//---------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}


//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
}




