/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceCompositeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSliceCompositeNode.h"

#include "vtkMatrix4x4.h"

//------------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceCompositeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceCompositeNode");
  if(ret)
    {
    return (vtkMRMLSliceCompositeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceCompositeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSliceCompositeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceCompositeNode");
  if(ret)
    {
    return (vtkMRMLSliceCompositeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceCompositeNode;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->BackgroundVolumeID = NULL;
  this->ForegroundVolumeID = NULL;
  this->LabelVolumeID = NULL;
  this->ForegroundOpacity = 0.0; // start by showing only the background volume
  // TODO: LabelOpacity should be 1.0 by default once the 'None' volume is fixed
  this->LabelOpacity = 0.0; // Show the label if there is one
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << "backgroundVolumeID=\"" << this->BackgroundVolumeID << "\" ";
  of << indent << "foregroundVolumeID=\"" << this->ForegroundVolumeID << "\" ";
  of << indent << "forgroundOpacity=\"" << this->ForegroundOpacity << "\" ";
  of << indent << "labelOpacity=\"" << this->LabelOpacity << "\" ";

}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "backgroundVolumeID")) 
      {
      this->SetBackgroundVolumeID(attValue);
      }
    if (!strcmp(attName, "foregroundVolumeID")) 
      {
      this->SetForegroundVolumeID(attValue);
      }
    if (!strcmp(attName, "foregroundOpacity")) 
      {
      this->SetForegroundOpacity( atof(attValue) );
      }
    if (!strcmp(attName, "labelOpacity")) 
      {
      this->SetLabelOpacity( atof(attValue) );
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSliceCompositeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSliceCompositeNode *node = vtkMRMLSliceCompositeNode::SafeDownCast(anode);

  this->SetBackgroundVolumeID(node->GetBackgroundVolumeID());
  this->SetForegroundVolumeID(node->GetForegroundVolumeID());
  this->SetForegroundOpacity(node->GetForegroundOpacity());
  this->SetLabelOpacity(node->GetLabelOpacity());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "BackgroundVolumeID: " << 
   (this->BackgroundVolumeID ? this->BackgroundVolumeID : "(none)") << "\n";
  os << indent << "ForegroundVolumeID: " << 
   (this->ForegroundVolumeID ? this->ForegroundVolumeID : "(none)") << "\n";
  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";
  os << indent << "LabelOpacity: " << this->LabelOpacity << "\n";
}


// End
