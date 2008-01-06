/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiducialsLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerFiducialsLogic.h"

#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLSelectionNode.h"

vtkCxxRevisionMacro(vtkSlicerFiducialsLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiducialsLogic);

//----------------------------------------------------------------------------
vtkSlicerFiducialsLogic::vtkSlicerFiducialsLogic()
{
 
}

//----------------------------------------------------------------------------
vtkSlicerFiducialsLogic::~vtkSlicerFiducialsLogic()
{
    
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::ProcessMRMLEvents()
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiducialsLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::AddFiducialListSelected()
{
  vtkMRMLFiducialListNode *node = this->AddFiducialList();

  // make it active
  vtkMRMLSelectionNode *selnode;
  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (selnode && node)
    {
    selnode->SetActiveFiducialListID(node->GetID());
    }
  else
    {
    vtkErrorMacro("AddFiducialListSelected: unable to add a fiducial list and set it active");
    }
  if (node)
    {
    node->Delete();
    }
}
//----------------------------------------------------------------------------
vtkMRMLFiducialListNode *vtkSlicerFiducialsLogic::AddFiducialList()
{
  this->GetMRMLScene()->SaveStateForUndo();
  
  vtkMRMLNode *node = 
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLFiducialListNode");
  if (node == NULL)
    {
    return NULL;
    }
  const char *name;
  name = this->MRMLScene->GetTagByClassName("vtkMRMLFiducialListNode");
//  node->SetName(this->MRMLScene->GetUniqueNameByString(name));
  node->SetName(this->MRMLScene->GetUniqueNameByString("L"));
  this->GetMRMLScene()->AddNode(node); 
  return vtkMRMLFiducialListNode::SafeDownCast(node);
}

//----------------------------------------------------------------------------
int vtkSlicerFiducialsLogic::AddFiducialSelected (float x, float y, float z, int selected)
{
  // get the selection node
  vtkMRMLSelectionNode *selnode;
  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  int index = -1;
  if (selnode != NULL)
    {
    this->GetMRMLScene()->SaveStateForUndo(selnode);
    
    if (selnode->GetActiveFiducialListID() == NULL)
      {
      vtkDebugMacro("FiducialsLogic: selection node doesn't have an active fiducial list right now, making one first before adding a fiducial");
      this->AddFiducialListSelected();
      }
    // get the selected fiducial list
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
    if (flist == NULL)
      {
      vtkDebugMacro("FiducialsLogic: selected fiducial list " << selnode->GetActiveFiducialListID() << " is null, making a new one");
      this->AddFiducialListSelected();
      flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
      }

    // add a fiducial
    this->MRMLScene->SaveStateForUndo(flist);
    vtkDebugMacro("Fids Logic: calling add fiducial on list " << flist->GetName());
    index = flist->AddFiducialWithXYZ(x, y, z, selected);
    if (index < 0)
      {
      vtkErrorMacro("AddFiducial: error adding a blank fiducial to list " << flist->GetName());
      }
    return index;
    }
  else
    {
    vtkErrorMacro("FiducialsLogic: no selection node to pick which list to which to add a fiducial\n");
    return -1;
    }

  // get the selection node
  if (selnode == NULL)
    {
    vtkDebugMacro("Selection node is null, returning.");
    return index;
    }
  if (selnode->GetActiveFiducialListID() == NULL)
    {
    vtkDebugMacro("FiducialsLogic: selection node doesn't have an active fiducial list right now, returning");
    return index;
    }

  vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
  if (flist == NULL)
    {
    vtkErrorMacro("FiducialsLogic: selected fiducial list is null");
    return -1;
    }
  return index;
}

//----------------------------------------------------------------------------
int vtkSlicerFiducialsLogic::AddFiducial(float x, float y, float z)
{
  // get the selection node
  vtkMRMLSelectionNode *selnode;
  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  if (selnode != NULL)
    {
    
    if (selnode->GetActiveFiducialListID() == NULL)
      {
      vtkDebugMacro("FiducialsLogic: selection node doesn't have an active fiducial list right now, making one first before adding a fiducial");
      this->AddFiducialListSelected();
      }
    // get the selected fiducial list
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
    if (flist == NULL)
      {
      vtkErrorMacro("FiducialsLogic: selected fiducial list " << selnode->GetActiveFiducialListID() << " is null, making a new one");
      this->AddFiducialListSelected();
      flist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
      }

    // add a fiducial
    this->MRMLScene->SaveStateForUndo(flist);
    vtkDebugMacro("Fids Logic: calling add fiducial on list " << flist->GetName());
    int index = flist->AddFiducialWithXYZ(x, y, z, false);
    if (index < 0)
      {
      vtkErrorMacro("AddFiducial: error adding a blank fiducial to list " << flist->GetName());
      }
    return index;
    }
  else
    {
    vtkErrorMacro("FiducialsLogic: no selection node to pick which list to which to add a fiducial\n");
    return -1;
    }
}
