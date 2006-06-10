/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

#include "vtkPointData.h"
#include "vtkDataArray.h"


//------------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLScalarVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLScalarVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::vtkMRMLScalarVolumeNode()
{
  this->LabelMap = 0;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::~vtkMRMLScalarVolumeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << indent << "labelMap=\"" << this->LabelMap << "\" ";
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "labelMap")) 
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      this->SetLabelMap(val);
      }
    }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScalarVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLScalarVolumeNode *node = (vtkMRMLScalarVolumeNode *) anode;

  this->SetLabelMap(node->LabelMap);
}

//-----------------------------------------------------------
void vtkMRMLScalarVolumeNode::CreateNoneNode(vtkMRMLScene *scene)
{
  vtkMRMLScalarVolumeNode *n = vtkMRMLScalarVolumeNode::New();
  n->SetName("None");
  n->SetID("None");

  // Create a None volume RGBA of 0, 0, 0 so the filters won't complain
  // about missing input
  vtkImageData *id;
  id = vtkImageData::New();
  id->SetDimensions(10, 10, 10);
  id->AllocateScalars();
  id->GetPointData()->GetScalars()->FillComponent(0, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(1, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(2, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(3, 0.0);

  n->SetImageData(id);
  scene->AddNode(n);

  n->Delete();
  id->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  os << indent << "LabelMap:          " << this->LabelMap << "\n";
}


 
