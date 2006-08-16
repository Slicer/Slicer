/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiducialListNode* vtkMRMLFiducialListNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListNode");
  if(ret)
    {
    return (vtkMRMLFiducialListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiducialListNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListNode");
  if(ret)
    {
    return (vtkMRMLFiducialListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiducialListNode::vtkMRMLFiducialListNode()
{

  this->FiducialList = vtkCollection::New();
  this->Indent = 1;
  this->SymbolSize = 6.0;
  this->TextSize = 4.5;
  this->Visibility = 1;
  this->Color[0]=0.4; this->Color[1]=1.0; this->Color[2]=1.0;
  this->Name = "NewList";
  this->DisplayNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode::~vtkMRMLFiducialListNode()
{
  this->FiducialList->Delete();

  if (this->Name) {
      delete [] this->Name;
      this->Name = NULL;
  }

  this->SetAndObserveDisplayNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  of << indent << "name=\"" << (this->Name ? "(none)" : this->Name) << "\"";
      
  if (this->DisplayNodeID != NULL) 
  {
      of << indent << "displayNodeRef=\"" << this->DisplayNodeID << "\" ";
  }
  
  of << " symbolSize=\"" << this->SymbolSize << "\"";
  of << " textSize=\"" << this->TextSize << "\"";
  of << " visibility=\"" << this->Visibility << "\"";
  
  of << " color=\"" << this->Color[0] << " " << 
                    this->Color[1] << " " <<
                    this->Color[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::ReadXMLAttributes(const char** atts)
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
        this->SetName(attName);
    }
    else  if (!strcmp(attName, "color")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Color[0];
      ss >> this->Color[1];
      ss >> this->Color[2];
      }
    else if (!strcmp(attName, "symbolSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SymbolSize;
      }
    else if (!strcmp(attName, "textSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextSize;
      }
    else if (!strcmp(attName, "visibility")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Visibility;
      }
    else if (!strcmp(attName, "displayNodeRef")) 
    {
        this->SetDisplayNodeID(attValue);
    }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiducialListNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLFiducialListNode *node = (vtkMRMLFiducialListNode *) anode;

  this->SetName(node->Name);
  this->SetColor(node->Color);
  this->SetSymbolSize(node->SymbolSize);
  this->SetTextSize(node->TextSize);
  this->SetVisibility(node->Visibility);

  this->SetDisplayNodeID(node->DisplayNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Name: " <<
      (this->Name ? this->Name : "(none)") << "\n";
  
  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";
  
  os << indent << "Symbol size: (";
  os << indent << this->SymbolSize << ") \n ";

  os << indent << "Text size: (";
  os << indent << this->TextSize << ") \n ";

  os << indent << "Visibility:        " << this->Visibility << "\n";

  os << "Color:\n";
  for (idx = 0; idx < 3; ++idx)
    {
    os << indent << ", " << this->Color[idx];
    }
}

//-----------------------------------------------------------

void vtkMRMLFiducialListNode::UpdateScene(vtkMRMLScene *scene)
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
        this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());
    }
    */
}

//-----------------------------------------------------------
vtkMRMLFiducialNode* vtkMRMLFiducialListNode::GetNthFiducialNode(int n)
{

  if(n < 0 || n >= this->FiducialList->GetNumberOfItems()) 
    {
    return NULL;
    }
  else 
    {
    return (vtkMRMLFiducialNode*)this->FiducialList->GetItemAsObject(n);
    }
}

//-----------------------------------------------------------
void vtkMRMLFiducialListNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListDisplayNode* vtkMRMLFiducialListNode::GetDisplayNode()
{
  vtkMRMLFiducialListDisplayNode* node = NULL;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkMRMLFiducialListDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  if (this->DisplayNodeID != NULL)
    {
    vtkMRMLFiducialListDisplayNode *dnode = this->GetDisplayNode();
    if (dnode != NULL)
      {
      dnode->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
      }
    }
  this->SetDisplayNodeID(displayNodeID);
  vtkMRMLFiducialListDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL) 
    {
    dnode->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLFiducialListNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLFiducialListDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLFiducialListDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLFiducialListNode::DisplayModifiedEvent, NULL);
    }
  return;
}

