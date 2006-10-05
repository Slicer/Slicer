/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLColorNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLColorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorNode");
  if(ret)
    {
    return (vtkMRMLColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLColorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorNode");
  if(ret)
    {
    return (vtkMRMLColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorNode;
}


//----------------------------------------------------------------------------
vtkMRMLColorNode::vtkMRMLColorNode()
{

  this->Color = vtkLookupTable::New();
  this->Type = this->Labels;
}

//----------------------------------------------------------------------------
vtkMRMLColorNode::~vtkMRMLColorNode()
{
    if (this->Color)
    {
        this->Color->Delete();
    }
  if (this->Name) {

      delete [] this->Name;
      this->Name = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << " type=\"" << this->GetTypeAsString() << "\"";
  
  of << " color=\"" <<  "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "name"))
      {
          this->SetName(attValue);
      }
      else if (!strcmp(attName, "id"))
      {
          // handled at the vtkMRMLNode level
      }
      else  if (!strcmp(attName, "color")) 
      {
          std::stringstream ss;
          ss << attValue;
          //ss >> this->Color[0];
          //ss >> this->Color[1];
          //ss >> this->Color[2];
      }
      else if (!strcmp(attName, "type")) 
      {
          std::stringstream ss;
          ss << attValue;
          ss >> this->Type;
      }
      else
      {
          std::cerr << "Unknown attribute name " << attName << endl;
      }
  }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLColorNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLColorNode *node = (vtkMRMLColorNode *) anode;

  this->SetName(node->Name);
  this->SetColor(node->Color);
  this->SetType(node->Type);

}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Name: " <<
      (this->Name ? this->Name : "(none)") << "\n";
  

  os << indent << "Type: (";
  os << indent << this->Type << ")\n";

  os << indent << "Color: (";
}

//-----------------------------------------------------------

void vtkMRMLColorNode::UpdateScene(vtkMRMLScene *scene)
{
    Superclass::UpdateScene(scene);
    /*
    if (this->GetStorageNodeID() == NULL) 
    {
        //vtkErrorMacro("No reference StorageNodeID found");
        return;
    }

    vtkMRMLNode* mnode = scene->GetNodeByID(this->StorageNodeID);
    if (mnode) 
    {
        vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(mnode);
        node->ReadData(this);
        //this->SetAndObservePolyData(this->GetPolyData());
    }
    */
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToLabels()
{
    this->SetType(this->Labels);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToRandom()
{
    this->SetType(this->Random);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToOcean()
{
    this->SetType(this->Ocean);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToGrey()
{
    this->SetType(this->Grey);
}

//----------------------------------------------------------------------------
const char* vtkMRMLColorNode::GetTypeAsString()
{
  if (this->Type == this->Labels)
    {
      return "Labels";
    }
  if (this->Type == this->Random)
    {
      return "Random";
    }
  if (this->Type == this->Grey)
    {
      return "Grey";
    }
  if (this->Type == this->Ocean)
    {
      return "Ocean";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLColorDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLColorNode::DisplayModifiedEvent, NULL);
    }
*/
  // check for one of the fiducials being modified, if so, trigger a modified
  // event on the list
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetType(int type)
{
    if (this->Type == type)
    {
        return;
    }
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type);
    this->Type = type;
   
    // invoke a modified event
    this->Modified();
    
    // invoke a  modified event
    this->InvokeEvent(vtkMRMLColorNode::TypeModifiedEvent);
}
