/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNonlinearTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkGeneralTransform.h"
#include "vtkGridTransform.h"
#include "vtkWarpTransform.h"


#include "vtkMRMLNonlinearTransformNode.h"

vtkCxxSetObjectMacro(vtkMRMLNonlinearTransformNode,WarpTransformToParent,vtkWarpTransform);
vtkCxxSetObjectMacro(vtkMRMLNonlinearTransformNode,WarpTransformFromParent,vtkWarpTransform);


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNonlinearTransformNode);

//----------------------------------------------------------------------------
vtkMRMLNonlinearTransformNode::vtkMRMLNonlinearTransformNode()
{
  this->WarpTransformToParent = NULL;
  this->WarpTransformFromParent = NULL;
  this->ReadWriteAsTransformToParent = 0;

}

//----------------------------------------------------------------------------
vtkMRMLNonlinearTransformNode::~vtkMRMLNonlinearTransformNode()
{
  if (this->WarpTransformToParent)
    {
    this->SetAndObserveWarpTransformToParent(NULL);
    }
  if (this->WarpTransformFromParent)
    {
    this->SetAndObserveWarpTransformFromParent(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // The different derived nonlinear transformation classes are going
  // to be so different that it doesn't make sense to write anything
  // here.  Let the derived classes do the work.
}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  // The different derived nonlinear transformation classes are going
  // to be so different that it doesn't make sense to read anything
  // here.  Let the derived classes do the work.
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLNonlinearTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->WarpTransformToParent != NULL)
    {
    os << indent << "WarpTransformToParent: " << "\n";
    this->WarpTransformToParent->PrintSelf( os, indent.GetNextIndent() );
    }
}

//----------------------------------------------------------------------------
vtkGeneralTransform* vtkMRMLNonlinearTransformNode::GetTransformToParent()
{
  vtkWarpTransform *warp = this->GetWarpTransformToParent();

  if (this->TransformToParent == 0)
    {
    this->TransformToParent = vtkGeneralTransform::New();
    }
  this->TransformToParent->Identity();
  this->TransformToParent->Concatenate(warp);
  return this->TransformToParent;
}

//----------------------------------------------------------------------------
vtkGeneralTransform* vtkMRMLNonlinearTransformNode::GetTransformFromParent()
{
  vtkWarpTransform *warp = this->GetWarpTransformFromParent();

  if (this->TransformFromParent == 0)
    {
    this->TransformFromParent = vtkGeneralTransform::New();
    }
  this->TransformFromParent->Identity();
  this->TransformFromParent->Concatenate(warp);
  return this->TransformFromParent;

}



//----------------------------------------------------------------------------
int  vtkMRMLNonlinearTransformNode::GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld)
{
  if (this->IsTransformToWorldLinear() != 1) 
    {
    transformToWorld->Identity();
    return 0;
    }

  // TODO: what does this return code mean?
  return 1;
}

//----------------------------------------------------------------------------
int  vtkMRMLNonlinearTransformNode::GetMatrixTransformToNode(vtkMRMLTransformNode* node,
                                                          vtkMatrix4x4* transformToNode)
{
  if (this->IsTransformToNodeLinear(node) != 1) 
    {
    transformToNode->Identity();
    return 0;
    }
  
  
  // TODO: what does this return code mean?
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::SetAndObserveWarpTransformToParent(vtkWarpTransform *warp)
{
  if (warp == this->WarpTransformToParent)
    {
    // We return for 2 reasons:
    //   - there is nothing to do
    //   - the remaining of the function could uninstantiate warp (when calling
    //     this->SetWarpTransformToParent(NULL)) but try to register it after
    //     in this->SetWarpTransformToParent. One must use Register carefully
    return;
    }
  if (this->WarpTransformToParent != NULL)
    {
    this->WarpTransformToParent->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    this->SetWarpTransformToParent(NULL);
    }
  this->SetWarpTransformToParent(warp);
  if ( this->WarpTransformToParent )
    {
    this->WarpTransformToParent->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  if (warp && this->WarpTransformFromParent)
    {
    vtkWarpTransform *warpInv = this->GetWarpTransformFromParent();
    warpInv->DeepCopy(warp);
    warpInv->Inverse();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::SetAndObserveWarpTransformFromParent(vtkWarpTransform *warp)
{
  if (warp == this->WarpTransformFromParent)
    {
    // We return for 2 reasons:
    //   - there is nothing to do
    //   - the remaining of the function could uninstantiate warp (when calling
    //     this->SetWarpTransformFromParent(NULL)) but try to register it after
    //     in this->SetWarpTransformFromParent. One must use Register carefully
    return;
    }
  if (this->WarpTransformFromParent != NULL)
    {
    this->WarpTransformFromParent->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    this->SetWarpTransformFromParent(NULL);
    }
  this->SetWarpTransformFromParent(warp);
  if ( this->WarpTransformFromParent )
    {
    this->WarpTransformFromParent->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  if (warp && this->WarpTransformToParent)
    {
    vtkWarpTransform *warpInv = this->GetWarpTransformToParent();
    warpInv->DeepCopy(warp);
    warpInv->Inverse();
    }
}


//---------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  Superclass::ProcessMRMLEvents ( caller, event, callData );

  if (this->WarpTransformToParent != NULL && this->WarpTransformToParent == vtkWarpTransform::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
    }
  else if (this->WarpTransformFromParent != NULL && this->WarpTransformFromParent == vtkWarpTransform::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
    }
}

// End
