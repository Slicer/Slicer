/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLThresholdingFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLThresholdingFilterNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLThresholdingFilterNode* vtkMRMLThresholdingFilterNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLThresholdingFilterNode");
  if(ret)
    {
      return (vtkMRMLThresholdingFilterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLThresholdingFilterNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLThresholdingFilterNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLThresholdingFilterNode");
  if(ret)
    {
      return (vtkMRMLThresholdingFilterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLThresholdingFilterNode;
}

//----------------------------------------------------------------------------
vtkMRMLThresholdingFilterNode::vtkMRMLThresholdingFilterNode()
{
   this->Conductance = 1;
   
   this->Sagittal1 = 1;
   
   this->Position2 = 1;
   
   this->Shrink = 3;
   
   this->Max = 50;
   
   this->Num = 4;
   
   this->Wien = 0.1;
   
   this->Field = 0.15;
   
   this->Con = 0.001;

   
   this->InputVolumeRef = NULL;
   this->OutputVolumeRef = NULL;
   /*this->StorageVolumeRef = NULL;*/
   this->HideFromEditors = true;
}

//----------------------------------------------------------------------------
vtkMRMLThresholdingFilterNode::~vtkMRMLThresholdingFilterNode()
{
   this->SetInputVolumeRef( NULL );
   this->SetOutputVolumeRef( NULL );
   /*this->SetStorageVolumeRef(NULL);*/
}

//----------------------------------------------------------------------------
void vtkMRMLThresholdingFilterNode::WriteXML(ostream& of, int nIndent)
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
    ss << this->Position2;
    of << indent << " Position2=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->Conductance;
    of << indent << " Conductance=\"" << ss.str() << "\"";
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
    /*{
    std::stringstream ss;
    if ( this->StorageVolumeRef )
      {
      ss << this->StorageVolumeRef;
      of << indent << " StorageVolumeRef=\"" << ss.str() << "\"";
      }
  }*/
  {
    std::stringstream ss;
    ss << this->Sagittal1;
    of << indent << " Sagittal1=\"" << ss.str() << "\"";
  }
  
}

//----------------------------------------------------------------------------
void vtkMRMLThresholdingFilterNode::ReadXMLAttributes(const char** atts)
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
    if (!strcmp(attName, "Conductance")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Conductance;
      }

    else if (!strcmp(attName, "InputVolumeRef"))
      {
      this->SetInputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputVolumeRef, this);
      }
    else if (!strcmp(attName, "OutputVolumeRef"))
      {
      this->SetOutputVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->OutputVolumeRef, this);
      }
     /*else if (!strcmp(attName, "StorageVolumeRef"))
      {
      this->SetStorageVolumeRef(attValue);
      this->Scene->AddReferencedNodeID(this->StorageVolumeRef, this);
      }*/
    else if (!strcmp(attName, "Sagittal1"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Sagittal1;
      }
     else if (!strcmp(attName, "Position2"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Position2;
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLThresholdingFilterNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLThresholdingFilterNode *node = (vtkMRMLThresholdingFilterNode *) anode;

  this->SetCon(node->Con);

  this->SetField(node->Field);
  
  this->SetConductance(node->Conductance);

  this->SetShrink(node->Shrink);
  
  this->SetMax(node->Max);
  
  this->SetNum(node->Num);
  
  this->SetWien(node->Wien);
  
  this->SetSagittal1(node->Sagittal1);

  this->SetPosition2(node->Position2);
  
  this->SetInputVolumeRef(node->InputVolumeRef);
  this->SetOutputVolumeRef(node->OutputVolumeRef);
  /*this->SetStorageVolumeRef(node->StorageVolumeRef);*/

}

//----------------------------------------------------------------------------
void vtkMRMLThresholdingFilterNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);
  
  os << indent << "Con:   " << this->Con << "\n";
  
  os << indent << "Field:   " << this->Field << "\n";
  
  os << indent << "Wien:   " << this->Wien << "\n";
  
  os << indent << "Num:   " << this->Num << "\n";
  
  os << indent << "Max:   " << this->Max << "\n";
  
  os << indent << "Shrink:   " << this->Shrink << "\n";

  os << indent << "Conductance:   " << this->Conductance << "\n";

  os << indent << "Sagittal1:   " << this->Sagittal1 << "\n";
  
  os << indent << "Position2:   " << this->Position2 << "\n";

  os << indent << "InputVolumeRef:   " << 
   (this->InputVolumeRef ? this->InputVolumeRef : "(none)") << "\n";
  os << indent << "OutputVolumeRef:   " << 
   (this->OutputVolumeRef ? this->OutputVolumeRef : "(none)") << "\n";
  /*os << indent << "StorageVolumeRef:   " << 
   (this->StorageVolumeRef ? this->StorageVolumeRef : "(none)") << "\n";*/
}

//----------------------------------------------------------------------------
void vtkMRMLThresholdingFilterNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->InputVolumeRef))
    {
    this->SetInputVolumeRef(newID);
    }
  if (!strcmp(oldID, this->OutputVolumeRef))
    {
    this->SetOutputVolumeRef(newID);
    }
  /*if (!strcmp(oldID, this->StorageVolumeRef))
    {
    this->SetStorageVolumeRef(newID);
    }*/
}
