/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCameraNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLCameraNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCameraNode");
  if(ret)
    {
    return (vtkMRMLCameraNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCameraNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCameraNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCameraNode");
  if(ret)
    {
    return (vtkMRMLCameraNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCameraNode;
}


//----------------------------------------------------------------------------
vtkMRMLCameraNode::vtkMRMLCameraNode()
{
  this->SingletonTag = "vtkMRMLCameraNode";

  this->HideFromEditors = 1;

  this->Active = 0;
  this->Camera = NULL;
  vtkCamera *camera = vtkCamera::New();

  camera->SetPosition(0, 500, 0);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetViewUp(0, 0, 1);

  this->SetAndObserveCamera(camera); 
  camera->Delete();
 }

//----------------------------------------------------------------------------
vtkMRMLCameraNode::~vtkMRMLCameraNode()
{
  this->SetAndObserveCamera(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  double *position = this->GetPosition();
  of << indent << " position=\"" << position[0] << " "
    << position[1] << " "
    << position[2] << "\"";

  double *focalPoint = this->GetFocalPoint();
  of << indent << " focalPoint=\"" << focalPoint[0] << " "
    << focalPoint[1] << " "
    << focalPoint[2] << "\"";

  double *viewUp = this->GetViewUp();
    of << indent << " viewUp=\"" << viewUp[0] << " "
      << viewUp[1] << " "
      << viewUp[2] << "\"";

  of << indent << " parallelProjection=\"" << (this->GetParallelProjection() ? "true" : "false") << "\"";

  of << indent << " parallelScale=\"" << this->GetParallelScale() << "\"";

  of << indent << " active=\"" << (this->Active ? "true" : "false") << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "position")) 
      {
      std::stringstream ss;
      ss << attValue;
      double Position[3];
      ss >> Position[0];
      ss >> Position[1];
      ss >> Position[2];
      this->SetPosition(Position);
      }
    else if (!strcmp(attName, "focalPoint")) 
      {
      std::stringstream ss;
      ss << attValue;
      double FocalPoint[3];
      ss >> FocalPoint[0];
      ss >> FocalPoint[1];
      ss >> FocalPoint[2];
      this->SetFocalPoint(FocalPoint);
      }
    else if (!strcmp(attName, "viewUp")) 
      {
      std::stringstream ss;
      ss << attValue;
      double ViewUp[3];
      ss >> ViewUp[0];
      ss >> ViewUp[1];
      ss >> ViewUp[2];
      this->SetViewUp(ViewUp);
      }
    else if (!strcmp(attName, "parallelProjection")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->SetParallelProjection(1);
        }
      else
        {
        this->SetParallelProjection(0);
        }
      }
    else if (!strcmp(attName, "parallelScale")) 
      {
      std::stringstream ss;
      ss << attValue;
      double parallelScale;
      ss >> parallelScale;
      this->SetParallelScale(parallelScale);
      }

    else if (!strcmp(attName, "active")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->Active = 1;
        }
      else
        {
        this->Active = 0;
        }
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLCameraNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCameraNode *node = (vtkMRMLCameraNode *) anode;


  this->SetPosition(node->GetPosition());
  this->SetFocalPoint(node->GetFocalPoint());
  this->SetViewUp(node->GetViewUp());
  this->SetParallelProjection(node->GetParallelProjection());
  this->SetParallelScale(node->GetParallelScale());
  this->SetActive(node->GetActive());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

  os << "Position:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << (this->GetPosition())[idx];
    }
  os << "FocalPoint:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << (this->GetFocalPoint())[idx];
    }
  os << "ViewUp:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << (this->GetViewUp())[idx];
    }
  os << indent << "Active:        " << this->Active << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::SetAndObserveCamera(vtkCamera *camera)
{
  if (this->Camera != NULL)
    {
    this->Camera->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    this->SetCamera(NULL);
    }
  this->SetCamera(camera);
  if ( this->Camera )
    {
    this->Camera->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkMRMLCameraNode::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, 
                                            void *callData )
{
  Superclass::ProcessMRMLEvents ( caller, event, callData );

  if (this->Camera != NULL && this->Camera == vtkCamera::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::MakeOthersInActive()
{
  if (this->Scene == NULL)
    {
    return;
    }
  vtkMRMLCameraNode *node = NULL;
  int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLCameraNode::SafeDownCast (
       this->Scene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
    if (node != this)
      {
      node->SetActive(0);
      }
    }
}

