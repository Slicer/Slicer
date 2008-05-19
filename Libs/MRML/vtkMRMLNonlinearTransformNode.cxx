/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNonlinearTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkGridTransform.h"

#include "vtkMRMLNonlinearTransformNode.h"
#include "vtkMRMLScene.h"

vtkCxxSetObjectMacro(vtkMRMLNonlinearTransformNode,WarpTransformToParent,vtkWarpTransform);


//------------------------------------------------------------------------------
vtkMRMLNonlinearTransformNode* vtkMRMLNonlinearTransformNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNonlinearTransformNode");
  if(ret)
    {
    return (vtkMRMLNonlinearTransformNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNonlinearTransformNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLNonlinearTransformNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNonlinearTransformNode");
  if(ret)
    {
    return (vtkMRMLNonlinearTransformNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNonlinearTransformNode;
}

//----------------------------------------------------------------------------
vtkMRMLNonlinearTransformNode::vtkMRMLNonlinearTransformNode()
{
  this->WarpTransformToParent = NULL;

  // default to a grid transform? null transform? or should this be an
  // abstract class?
  vtkGridTransform *grid = vtkGridTransform::New();
  this->SetAndObserveWarpTransformToParent(grid);
  grid->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLNonlinearTransformNode::~vtkMRMLNonlinearTransformNode()
{
  if (this->WarpTransformToParent) 
    {
    this->SetAndObserveWarpTransformToParent(NULL);
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
  vtkMRMLNonlinearTransformNode *node = (vtkMRMLNonlinearTransformNode *) anode;

  this->WarpTransformToParent->DeepCopy( node->GetWarpTransformToParent() );
}

//----------------------------------------------------------------------------
void vtkMRMLNonlinearTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->WarpTransformToParent != NULL) 
    {
    os << indent << "WarpTransformToParent: " << "\n";
    this->WarpTransformToParent->PrintSelf( os, indent.GetNextIndent() );
//     for (int row=0; row<4; row++) 
//       {
//       for (int col=0; col<4; col++) 
//         {
//         os << this->MatrixTransformToParent->GetElement(row, col);
//         if (!(row==3 && col==3)) 
//           {
//           os << " ";
//           }
//         else 
//           {
//           os << "\n";
//           }
//         } // for (int col
//       } // for (int row
    }
}

//----------------------------------------------------------------------------
vtkGeneralTransform* vtkMRMLNonlinearTransformNode::GetTransformToParent()
{
  this->TransformToParent->Identity();
  this->TransformToParent->Concatenate(this->WarpTransformToParent);
  return this->TransformToParent;
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
}

// End
