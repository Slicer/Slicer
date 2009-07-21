/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAtlasCreatorNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include <iostream>
#include <sstream>

#include "vtkMRMLAtlasCreatorNode.h"
#include "vtkMRMLScene.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkMRMLAtlasCreatorNode* vtkMRMLAtlasCreatorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAtlasCreatorNode");

  if (ret)
    {
    return (vtkMRMLAtlasCreatorNode*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAtlasCreatorNode;
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAtlasCreatorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAtlasCreatorNode");

  if (ret)
    {
    return (vtkMRMLAtlasCreatorNode*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAtlasCreatorNode;
}

//---------------------------------------------------------------------------
vtkMRMLAtlasCreatorNode::vtkMRMLAtlasCreatorNode()
{
  this->SecondLabelMapThreshold = 0.0;
  this->OutputSize              = 1.0;

  this->FirstLabelMapRef        = NULL;
  this->OutputVolumeRef         = NULL;
  this->ThirdLabelMapVolumeRef  = NULL;
  this->SecondLabelMapVolumeRef = NULL;

  this->HideFromEditors         = true;
}

//---------------------------------------------------------------------------
vtkMRMLAtlasCreatorNode::~vtkMRMLAtlasCreatorNode()
{
  this->SetFirstLabelMapRef(NULL);
  this->SetOutputVolumeRef(NULL);
  this->SetThirdLabelMapVolumeRef(NULL);
  this->SetSecondLabelMapVolumeRef(NULL);
}

//---------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all MRML node attributes into output stream
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  {
  std::stringstream ss;
  ss << this->SecondLabelMapThreshold;
  of << indent << " SecondLabelMapThreshold=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->OutputSize;
  of << indent << " OutputSize=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  if (this->FirstLabelMapRef)
    {
    ss << this->FirstLabelMapRef;
    of << indent << " FirstLabelMapRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if (this->OutputVolumeRef)
    {
    ss << this->OutputVolumeRef;
    of << indent << " OutputVolumeRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if (this->ThirdLabelMapVolumeRef)
    {
    ss << this->ThirdLabelMapVolumeRef;
    of << indent << " ThirdLabelMapVolumeRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if (this->SecondLabelMapVolumeRef)
    {
    ss << this->SecondLabelMapVolumeRef;
    of << indent << " SecondLabelMapVolumeRef=\"" << ss.str() << "\"";
    }
  }
  /*
  {
  std::stringstream ss;
  if (this->DemoImage)
    {
    ss << this->DemoImage;
    of << indent << " DemoImage=\"" << ss.str() << "\"";
    }
  }
  */
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName  = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName,"SecondLabelMapThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SecondLabelMapThreshold;
      }

    if (!strcmp(attName,"OutputSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->OutputSize;
      }

    if (!strcmp(attName,"FirstLabelMapRef"))
      {
      this->SetFirstLabelMapRef(attValue);
      this->Scene->AddReferencedNodeID(this->FirstLabelMapRef,this);
      }

    if (!strcmp(attName,"OutputVolumeRef"))
      {
      this->SetOutputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->OutputVolumeRef,this);
      }

    if (!strcmp(attName,"ThirdLabelMapVolumeRef"))
      {
      this->SetThirdLabelMapVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->ThirdLabelMapVolumeRef,this);
      }

     if (!strcmp(attName,"SecondLabelMapVolumeRef"))
      {
      this->SetSecondLabelMapVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->SecondLabelMapVolumeRef,this);
      }
    }

  // DemoImage
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLAtlasCreatorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLAtlasCreatorNode *node =
    (vtkMRMLAtlasCreatorNode *) anode;

  this->SetSecondLabelMapThreshold(node->SecondLabelMapThreshold);
  this->SetOutputSize(node->OutputSize);

  this->SetFirstLabelMapRef(node->FirstLabelMapRef);
  this->SetOutputVolumeRef(node->OutputVolumeRef);
  this->SetThirdLabelMapVolumeRef(node->ThirdLabelMapVolumeRef);
  this->SetSecondLabelMapVolumeRef(node->SecondLabelMapVolumeRef);
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::PrintSelf(ostream& os, vtkIndent
    indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "SecondLabelMapThreshold: " << this->SecondLabelMapThreshold
    <<"\n";
  os << indent << "OutputSize:            " << this->OutputSize        <<"\n";
  os << indent << "FirstLabelMapRef:   " <<
   (this->FirstLabelMapRef ? this->FirstLabelMapRef     : "(none)") << "\n";
  os << indent << "OutputVolumeRef:   " <<
   (this->OutputVolumeRef ? this->OutputVolumeRef   : "(none)") << "\n";
  os << indent << "ThirdLabelMapVolumeRef:   " <<
   (this->ThirdLabelMapVolumeRef ? this->ThirdLabelMapVolumeRef : "(none)") <<
   "\n";
   os << indent << "SecondLabelMapVolumeRef:   " <<
   (this->SecondLabelMapVolumeRef ? this->SecondLabelMapVolumeRef       :
    "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::UpdateReferenceID(const char *oldID, const char
    *newID)
{
  if (strcmp(oldID, this->FirstLabelMapRef) == 0)
    {
    this->SetFirstLabelMapRef(newID);
    }
  if (strcmp(oldID, this->OutputVolumeRef) == 0)
    {
    this->SetOutputVolumeRef(newID);
    }
  if (strcmp(oldID, this->ThirdLabelMapVolumeRef) == 0)
    {
    this->SetThirdLabelMapVolumeRef(newID);
    }
  if (strcmp(oldID, this->SecondLabelMapVolumeRef) == 0)
    {
    this->SetSecondLabelMapVolumeRef(newID);
    }
}

