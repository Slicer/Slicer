/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLRobotNode.h"
#include "vtkMRMLScene.h"

#include "vtkSmartPointer.h"

//------------------------------------------------------------------------------
vtkMRMLRobotNode* vtkMRMLRobotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLRobotNode"); if(ret)
    {
      return (vtkMRMLRobotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLRobotNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLRobotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLRobotNode");
  if(ret)
    {
      return (vtkMRMLRobotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLRobotNode;
}

//----------------------------------------------------------------------------
vtkMRMLRobotNode::vtkMRMLRobotNode()
{
  this->TargetTransformNodeID = NULL;
  this->TargetTransformNode = NULL;

  this->HideFromEditors=0;
}

//----------------------------------------------------------------------------
vtkMRMLRobotNode::~vtkMRMLRobotNode()
{
  if (this->TargetTransformNodeID) 
  {
    SetAndObserveTargetTransformNodeID(NULL);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLRobotNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->TargetTransformNodeID != NULL) 
    {
    of << indent << " TargetTransformNodeRef=\"" << this->TargetTransformNodeID << "\"";
    }

  //switch (this->Type)
  //  {
  //  case TYPE_SERVER:
  //    of << " connectorType=\"" << "SERVER" << "\" ";
  //    break;
  //  case TYPE_CLIENT:
  //    of << " connectorType=\"" << "CLIENT" << "\" ";
  //    of << " serverHostname=\"" << this->ServerHostname << "\" ";
  //    break;
  //  default:
  //    of << " connectorType=\"" << "NOT_DEFINED" << "\" ";
  //    break;
  //  }
  //
  //of << " serverPort=\"" << this->ServerPort << "\" ";
  //of << " restrictDeviceName=\"" << this->RestrictDeviceName << "\" ";

}


//----------------------------------------------------------------------------
void vtkMRMLRobotNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName  = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "TargetTransformNodeRef")) 
      {
      this->SetAndObserveTargetTransformNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->TransformNodeID, this);
      }

    }
 

/*  const char* attName;
  const char* attValue;

  const char* serverHostname = "";
  int port = 0;
  int type = -1;
  int restrictDeviceName = 0;
*/
  /*
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "connectorType"))
      {
      if (!strcmp(attValue, "SERVER"))
        {
        type = TYPE_SERVER;
        }
      else if (!strcmp(attValue, "CLIENT"))
        {
        type = TYPE_CLIENT;
        }
      else
        {
        type = TYPE_NOT_DEFINED;
        }
      }
    if (!strcmp(attName, "serverHostname"))
      {
      serverHostname = attValue;
      }
    if (!strcmp(attName, "serverPort"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> port;
      }
    if (!strcmp(attName, "restrictDeviceName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> restrictDeviceName;;
      }
    }

  switch(type)
    {
    case TYPE_SERVER:
      this->SetTypeServer(port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    case TYPE_CLIENT:
      this->SetTypeClient(serverHostname, port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    default: // not defined
      // do nothing
      break;
    }
  */

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLRobotNode::Copy(vtkMRMLNode *anode)
{  
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLRobotNode *node = (vtkMRMLRobotNode *) anode;
  this->SetTargetTransformNodeID(node->TargetTransformNodeID);

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLRobotNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->TargetTransformNodeID != NULL && this->Scene->GetNodeByID(this->TargetTransformNodeID) == NULL)
    {
    this->SetAndObserveTargetTransformNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLRobotNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->TargetTransformNodeID && !strcmp(oldID, this->TargetTransformNodeID))
    {
    this->SetAndObserveTargetTransformNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLRobotNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
   this->SetAndObserveTargetTransformNodeID(this->GetTargetTransformNodeID());
}

//-----------------------------------------------------------
void vtkMRMLRobotNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}


//----------------------------------------------------------------------------
void vtkMRMLRobotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "TargetTransformNodeID: " <<
    (this->TargetTransformNodeID ? this->TargetTransformNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLRobotNode::SetAndObserveTargetTransformNodeID(const char *transformNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->TargetTransformNode, NULL);

  this->SetTargetTransformNodeID(transformNodeID);

  vtkMRMLTransformNode *tnode = this->GetTargetTransformNode();

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->TargetTransformNode, tnode, events);
  events->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLRobotNode::GetTargetTransformNode()
{
  vtkMRMLTransformNode* node = NULL;
  if (this->GetScene() && this->TargetTransformNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->TargetTransformNodeID);
    node = vtkMRMLTransformNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
int vtkMRMLRobotNode::Init(vtkSlicerApplication* app)
{
  if (GetTargetTransformNode()==NULL)
  {
    // Target node
    vtkSmartPointer<vtkMRMLLinearTransformNode> tnode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    tnode->SetName("ProstateNavRobotTarget");

    vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
    transform->Identity();
    //transformNode->SetAndObserveImageData(transform);
    tnode->ApplyTransform(transform);
    tnode->SetScene(this->GetScene());

    this->GetScene()->AddNode(tnode);
    SetAndObserveTargetTransformNodeID(tnode->GetID());
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLRobotNode::GetStatusDescriptorCount()
{
  return this->StatusDescriptors.size();
}

//----------------------------------------------------------------------------
int vtkMRMLRobotNode::GetStatusDescriptor(unsigned int index, std::string &text, STATUS_ID &indicator)
{
  if (index>=this->StatusDescriptors.size())
  {
    return 0;
  }
  text=this->StatusDescriptors[index].text;
  indicator=this->StatusDescriptors[index].indicator;
  return 1;
}
