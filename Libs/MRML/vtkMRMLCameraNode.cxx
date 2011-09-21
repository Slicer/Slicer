/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCameraNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkEventBroker.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLScene.h"

#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>

vtkCxxSetObjectMacro(vtkMRMLCameraNode, Camera, vtkCamera);
vtkCxxSetObjectMacro(vtkMRMLCameraNode, AppliedTransform, vtkMatrix4x4);
vtkCxxSetReferenceStringMacro(vtkMRMLCameraNode, InternalActiveTag);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCameraNode);

//----------------------------------------------------------------------------
vtkMRMLCameraNode::vtkMRMLCameraNode()
{
  //this->SingletonTag = const_cast<char *>("vtkMRMLCameraNode");

  this->HideFromEditors = 0;

  this->InternalActiveTag = NULL;
  this->Camera = NULL;
  vtkCamera *camera = vtkCamera::New();

  camera->SetPosition(0, 500, 0);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetViewUp(0, 0, 1);

  this->SetAndObserveCamera(camera); 
  camera->Delete();

  this->AppliedTransform = vtkMatrix4x4::New();
 }

//----------------------------------------------------------------------------
vtkMRMLCameraNode::~vtkMRMLCameraNode()
{
  this->SetAndObserveCamera(NULL);
  delete [] this->InternalActiveTag;

  if (this->AppliedTransform)
    {
    this->AppliedTransform->Delete();
    }
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

  if (this->GetActiveTag())
    {
    of << indent << " activetag=\"" << this->GetActiveTag() << "\"";
    }

  if (this->GetAppliedTransform()) 
    {
    std::stringstream ss;
    for (int row=0; row<4; row++) 
      {
      for (int col=0; col<4; col++) 
        {
        ss << this->AppliedTransform->GetElement(row, col);
        if (!(row==3 && col==3)) 
          {
          ss << " ";
          }
        }
      if ( row != 3 )
        {
        ss << " ";
        }
      }
    of << indent << " appliedTransform=\"" << ss.str() << "\"";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

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
    else if (!strcmp(attName, "activetag")) 
      {
      this->SetActiveTag(attValue);
      }
    else if (!strcmp(attName, "active")) 
      {
      // Legacy, was replaced by active tag, try to set ActiveTag instead
      // to link to the main viewer
      if (!this->GetActiveTag() && this->Scene)
        {
        vtkMRMLViewNode *vnode = vtkMRMLViewNode::SafeDownCast(
          this->Scene->GetNthNodeByClass(0, "vtkMRMLViewNode")); 
        if (vnode)
        {
          this->SetActiveTag(vnode->GetID());
        }
        }
      }
    else if (!strcmp(attName, "appliedTransform")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int row=0; row<4; row++) 
        {
        for (int col=0; col<4; col++) 
          {
          ss >> val;
          this->GetAppliedTransform()->SetElement(row, col, val);
          }
        }
      }
    }  
    this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLCameraNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLCameraNode *node = (vtkMRMLCameraNode *) anode;


  this->SetPosition(node->GetPosition());
  this->SetFocalPoint(node->GetFocalPoint());
  this->SetViewUp(node->GetViewUp());
  this->SetParallelProjection(node->GetParallelProjection());
  this->SetParallelScale(node->GetParallelScale());
  this->AppliedTransform->DeepCopy(node->GetAppliedTransform());
  // Important, do not call SetActiveTag() or the owner of the current tag
  // (node) will lose its tag, and the active camera will be untagged, and
  // a the active camera of the current view will be reset to NULL, and a 
  // new camera will be created on the fly by VTK the next time an active
  // camera is need, one completely disconnected from Slicer3's MRML/internals
  this->SetInternalActiveTag(node->GetActiveTag()); 

  this->EndModify(disabledModify);

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
  os << indent << "ActiveTag: " <<
    (this->GetActiveTag() ? this->GetActiveTag() : "(none)") << "\n";
  os << indent << "AppliedTransform: " ;
  this->GetAppliedTransform()->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::SetAndObserveCamera(vtkCamera *camera)
{
  if (this->Camera != NULL)
    {
    this->SetCamera(NULL);
    }
  this->SetCamera(camera);
  if ( this->Camera )
    {
    vtkEventBroker::GetInstance()->AddObservation (
      this->Camera, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetParallelProjection(int parallelProjection) 
{
  this->Camera->SetParallelProjection(parallelProjection);
}
  
//---------------------------------------------------------------------------
int vtkMRMLCameraNode::GetParallelProjection()
{
  return this->Camera->GetParallelProjection();
};

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetParallelScale(double scale) 
{
  this->Camera->SetParallelScale(scale);
}
  
//---------------------------------------------------------------------------
double vtkMRMLCameraNode::GetParallelScale()
{
  return this->Camera->GetParallelScale();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetPosition(double position[3]) 
{
  this->Camera->SetPosition(position);
}
  
//---------------------------------------------------------------------------
double *vtkMRMLCameraNode::GetPosition()
{
  return this->Camera->GetPosition();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetFocalPoint(double focalPoint[3]) 
{
  this->Camera->SetFocalPoint(focalPoint);
}
  
//---------------------------------------------------------------------------
double *vtkMRMLCameraNode::GetFocalPoint()
{
  return this->Camera->GetFocalPoint();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetViewUp(double viewUp[3]) 
{
  this->Camera->SetViewUp(viewUp);
}
  
//---------------------------------------------------------------------------
double *vtkMRMLCameraNode::GetViewUp()
{
  return this->Camera->GetViewUp();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, 
                                            void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->Camera != NULL && 
      this->Camera == vtkCamera::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }

  vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
  if (this->Camera != NULL &&
      tnode == vtkMRMLTransformNode::SafeDownCast(caller) && 
      event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {

    /*
     * calculate the delta transform Td, which is the incremental transform
     * that has not yet been applied to the current camera paramters.
     * 
     * We started with Po (original parameter)
     * We have Pa = param with Ta (AppliedTransform applied)
     * we want Pn = param with new transform applied
     * Since Pn = Tn * Po
     * and Tn = Td * Ta
     * then
     * Td = Ta-1 * Tn
     * and
     * Pn = Td * Pa
     * then we save Tn as Ta for next time
     */
    vtkSmartPointer<vtkMatrix4x4> deltaTransform = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    transformToWorld->Identity();
    tnode->GetMatrixTransformToWorld(transformToWorld);

    this->AppliedTransform->Invert();
    vtkMatrix4x4::Multiply4x4(transformToWorld, this->AppliedTransform, deltaTransform);

    // transform the points and the vector through delta and store back to camera
    double v[4];
    // position is point - include translation with 1 in homogeneous coordinate
    v[0] = this->Camera->GetPosition()[0];
    v[1] = this->Camera->GetPosition()[1];
    v[2] = this->Camera->GetPosition()[2];
    v[3] = 1;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetPosition(v[0],v[1],v[2]);
    // focal point is point - include translation with 1 in homogeneous coordinate
    v[0] = this->Camera->GetFocalPoint()[0];
    v[1] = this->Camera->GetFocalPoint()[1];
    v[2] = this->Camera->GetFocalPoint()[2];
    v[3] = 1;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetFocalPoint(v[0],v[1],v[2]);
    // view up is vector - exclude translation with 0 in homogeneous coordinate
    v[0] = this->Camera->GetViewUp()[0];
    v[1] = this->Camera->GetViewUp()[1];
    v[2] = this->Camera->GetViewUp()[2];
    v[3] = 0;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetViewUp(v[0],v[1],v[2]);

    this->GetAppliedTransform()->DeepCopy(transformToWorld);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
}

//-----------------------------------------------------------
void vtkMRMLCameraNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();

  if (this->GetActiveTag() != NULL && 
      this->Scene->GetNodeByID(this->GetActiveTag()) == NULL)
    {
    this->SetActiveTag(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->GetActiveTag() && !strcmp(oldID, this->GetActiveTag()))
    {
    this->SetActiveTag(newID);
    }
}

//---------------------------------------------------------------------------
const char* vtkMRMLCameraNode::GetActiveTag() 
{
  return this->InternalActiveTag;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetActiveTag(const char *_arg) 
{
  if (this->GetActiveTag() == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->GetActiveTag() && _arg && 
      (!strcmp(this->GetActiveTag(), _arg)))
    {
    return;
    }
  // set this node's active tag first, then loop through and unset anyone
  // else's
  // do this first because the viewer widget will get an event when we set
  // other node's active tags to null and it will regrab an unassigned camera
  // node so as not to be without one.
  this->SetInternalActiveTag(_arg);
  
  // If any camera is already using that new tag, let's find them and set
  // their tags to null
  if (this->Scene != NULL && _arg != NULL)
    {
    vtkMRMLCameraNode *node = NULL;
    int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLCameraNode::SafeDownCast (
                this->Scene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
      if (node &&
          node != this && 
          node->GetActiveTag() && 
          !strcmp(node->GetActiveTag(), _arg))
        {
        vtkWarningMacro("SetActiveTag: " << (this->GetID() ? this->GetID() : "NULL ID") << " found another node " << node->GetID() << " with the tag " << _arg);
        node->SetActiveTag(NULL);
        }
      }
    }
  else
    {
    vtkDebugMacro("SetActiveTag: null scene or tag, not checking for duplicates on camera " << (this->GetName() ? this->GetName() : "no name")
                    << ", input arg = " << (_arg == NULL ? "NULL" : _arg));
    }  
  this->InvokeEvent(vtkMRMLCameraNode::ActiveTagModifiedEvent, NULL);
}

//----------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraNode::FindActiveTagInScene(const char *tag)
{
  if (this->Scene == NULL || tag == NULL)
    {
    return NULL;
    }

  vtkMRMLCameraNode *node = NULL;
  int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLCameraNode::SafeDownCast (
       this->Scene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
    if (node != this && 
        node->GetActiveTag() && 
        !strcmp(node->GetActiveTag(), tag))
      {
      return node;
      }
    }

  return NULL;
}
