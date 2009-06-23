/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMRIBiasFieldCorrectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLMRIBiasFieldCorrectionNode.h"
#include "vtkMRMLScene.h"

//-------------------------------------------------------------------
vtkMRMLMRIBiasFieldCorrectionNode* vtkMRMLMRIBiasFieldCorrectionNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLMRIBiasFieldCorrectionNode");

  if(ret)
    {
      return (vtkMRMLMRIBiasFieldCorrectionNode*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMRIBiasFieldCorrectionNode;
}

//-------------------------------------------------------------------
vtkMRMLNode* vtkMRMLMRIBiasFieldCorrectionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLMRIBiasFieldCorrectionNode");

  if(ret)
    {
      return (vtkMRMLMRIBiasFieldCorrectionNode*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMRIBiasFieldCorrectionNode;
}

//-------------------------------------------------------------------
vtkMRMLMRIBiasFieldCorrectionNode::vtkMRMLMRIBiasFieldCorrectionNode()
{
  this->Shrink = 3;
  this->Max    = 50;
  this->Num    = 4;
  this->Wien   = 0.1;
  this->Field  = 0.15;
  this->Con    = 0.001;

  this->InputVolumeRef   = NULL;
  this->OutputVolumeRef  = NULL;
  this->StorageVolumeRef = NULL;
  this->MaskVolumeRef    = NULL;

  this->HideFromEditors = true;
}

//-------------------------------------------------------------------
vtkMRMLMRIBiasFieldCorrectionNode::
~vtkMRMLMRIBiasFieldCorrectionNode()
{
  this->SetInputVolumeRef( NULL );
  this->SetOutputVolumeRef( NULL );
  this->SetStorageVolumeRef(NULL);
  this->SetMaskVolumeRef(NULL);
}

//-------------------------------------------------------------------
void vtkMRMLMRIBiasFieldCorrectionNode::WriteXML(ostream& of,
  int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  {
  std::stringstream ss;
  ss << this->Con;
  of << indent << " Con=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->Field;
  of << indent << " Field=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->Wien;
  of << indent << " Wien=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->Num;
  of << indent << " Num=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->Max;
  of << indent << " Max=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  ss << this->Shrink;
  of << indent << " Shrink=\"" << ss.str() << "\"";
  }

  {
  std::stringstream ss;
  if ( this->InputVolumeRef )
    {
    ss << this->InputVolumeRef;
    of << indent << " InputVolumeRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if ( this->OutputVolumeRef )
    {
    ss << this->OutputVolumeRef;
    of << indent << " OutputVolumeRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if ( this->StorageVolumeRef )
    {
    ss << this->StorageVolumeRef;
    of << indent << " StorageVolumeRef=\"" << ss.str() << "\"";
    }
  }

  {
  std::stringstream ss;
  if ( this->MaskVolumeRef )
    {
    ss << this->MaskVolumeRef;
    of << indent << " MaskVolumeRef=\"" << ss.str() << "\"";
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLMRIBiasFieldCorrectionNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    
        if (!strcmp(attName, "Con")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Con;
      }
    if (!strcmp(attName, "Field")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Field;
      }
       if (!strcmp(attName, "Wien")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Wien;
      }
    if (!strcmp(attName, "Num")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Num;
      }
    if (!strcmp(attName, "Max")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Max;
      }
    if (!strcmp(attName, "Shrink")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Shrink;
      }

    if (!strcmp(attName, "InputVolumeRef"))
      {
      this->SetInputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputVolumeRef, this);
      }
    if (!strcmp(attName, "OutputVolumeRef"))
      {
      this->SetOutputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->OutputVolumeRef, this);
      }
    if (!strcmp(attName, "StorageVolumeRef"))
      {
      this->SetStorageVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->StorageVolumeRef, this);
      }
     if (!strcmp(attName, "MaskVolumeRef"))
      {
      this->SetMaskVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->MaskVolumeRef, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMRIBiasFieldCorrectionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLMRIBiasFieldCorrectionNode *node = (vtkMRMLMRIBiasFieldCorrectionNode *) anode;

  this->SetCon(node->Con);

  this->SetField(node->Field);

  this->SetShrink(node->Shrink);
  
  this->SetMax(node->Max);
  
  this->SetNum(node->Num);
  
  this->SetWien(node->Wien);
  
  this->SetInputVolumeRef(node->InputVolumeRef);
  this->SetOutputVolumeRef(node->OutputVolumeRef);
  this->SetStorageVolumeRef(node->StorageVolumeRef);
  this->SetMaskVolumeRef(node->MaskVolumeRef);

}

//----------------------------------------------------------------------------
void vtkMRMLMRIBiasFieldCorrectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);
  
  os << indent << "Con:   " << this->Con << "\n";
  
  os << indent << "Field:   " << this->Field << "\n";
  
  os << indent << "Wien:   " << this->Wien << "\n";
  
  os << indent << "Num:   " << this->Num << "\n";
  
  os << indent << "Max:   " << this->Max << "\n";
  
  os << indent << "Shrink:   " << this->Shrink << "\n";

  os << indent << "InputVolumeRef:   " << 
   (this->InputVolumeRef ? this->InputVolumeRef : "(none)") << "\n";
  os << indent << "OutputVolumeRef:   " << 
   (this->OutputVolumeRef ? this->OutputVolumeRef : "(none)") << "\n";
  os << indent << "StorageVolumeRef:   " << 
   (this->StorageVolumeRef ? this->StorageVolumeRef : "(none)") << "\n";
   os << indent << "MaskVolumeRef:   " << 
   (this->MaskVolumeRef ? this->MaskVolumeRef : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLMRIBiasFieldCorrectionNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->InputVolumeRef))
    {
    this->SetInputVolumeRef(newID);
    }
  if (!strcmp(oldID, this->OutputVolumeRef))
    {
    this->SetOutputVolumeRef(newID);
    }
  if (!strcmp(oldID, this->StorageVolumeRef))
    {
    this->SetStorageVolumeRef(newID);
    }
  if (!strcmp(oldID, this->MaskVolumeRef))
    {
    this->SetMaskVolumeRef(newID);
    }
}
