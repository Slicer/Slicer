/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// Slicer includes
#include "vtkSlicerFiducialsLogic.h"

// MRML includes
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <vtksys/SystemTools.hxx>


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
void vtkSlicerFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiducialsLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode *vtkSlicerFiducialsLogic::GetSelectedList()
{
  vtkMRMLFiducialListNode *fList = NULL;
  vtkMRMLSelectionNode *selnode = NULL;

  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  if (selnode != NULL)
    {
    if (selnode->GetActiveFiducialListID() != NULL)
      {
      // get the selected fiducial list
      fList = vtkMRMLFiducialListNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(selnode->GetActiveFiducialListID()));
      }
    else
      {
      vtkDebugMacro("GetSelectedList: selection node doesn't have an active fiducial list right now, making one first before adding a fiducial");
      }
    }
  else
    {
    vtkDebugMacro("GetSelectedList: no selection node on the scene, returning null");
    }
  return fList;
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::AddFiducialListSelected()
{
  vtkMRMLFiducialListNode *node = this->AddFiducialList();

  // make it active
  vtkMRMLSelectionNode *selnode;
  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (selnode && node)
    {
    this->GetMRMLScene()->SaveStateForUndo(selnode);
    selnode->SetActiveFiducialListID(node->GetID());
    }
  else
    {
    vtkErrorMacro("AddFiducialListSelected: unable to add a fiducial list and set it active");
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
  // set up a storage node
  vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(node);
  if (storableNode)
    {
    vtkMRMLStorageNode *snode = vtkMRMLFiducialListNode::SafeDownCast(node)->CreateDefaultStorageNode();
    //    vtkMRMLStorageNode *snode = storableNode->CreateDefaultStorageNode();
    if (snode)
      {
      snode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(snode);
      storableNode->SetAndObserveStorageNodeID(snode->GetID());
      snode->Delete();
      }
    }
  node->SetName(this->GetMRMLScene()->GetUniqueNameByString("L"));
  this->GetMRMLScene()->AddNode(node);
  node->Delete();
  return vtkMRMLFiducialListNode::SafeDownCast(node);
}

//----------------------------------------------------------------------------
int vtkSlicerFiducialsLogic::AddFiducial(float x, float y, float z)
{
  // defaults to selected being false
  return this->AddFiducialSelected(x, y, z, false);
}

//----------------------------------------------------------------------------
int vtkSlicerFiducialsLogic::AddFiducialSelected (float x, float y, float z, int selected)
{
  // get the selected list
  vtkMRMLFiducialListNode *flist = this->GetSelectedList();

  // there wasn't one, so add it
  if (flist == NULL)
    {
    this->AddFiducialListSelected();
    flist = this->GetSelectedList();
    }

  int index = -1;

  // check one more time
  if (flist == NULL)
    {
     vtkErrorMacro("AddFiducialSelected: no selected list to which to add a fiducial, even tried adding one");
     return index;
    }

  // add a fiducial to the selected list
  this->GetMRMLScene()->SaveStateForUndo(flist);
  vtkDebugMacro("AddFiducialSelected: calling add fiducial on list " << flist->GetName());
  index = flist->AddFiducialWithXYZ(x, y, z, selected);
  if (index < 0)
    {
    vtkErrorMacro("AddFiducialSelected: error adding a fiducial at " << x << ", " << y << ", " << z  << " to list " << flist->GetName());
    }

  return index;
}

//----------------------------------------------------------------------------
int vtkSlicerFiducialsLogic::AddFiducialPicked (float x, float y, float z, int selected)
{
  // get the selected list
  vtkMRMLFiducialListNode *flist = this->GetSelectedList();

  // there wasn't one, just call AddFiducialSelected, it will take care of
  // makign a new list, and there won't be a transform on it
  if (flist == NULL)
    {
    vtkDebugMacro("AddFiducialPicked: no selected list, calling AddFiducialSelected");
    return this->AddFiducialSelected(x, y, z, selected);
    }

  // otherwise, we have an exisiting list, check to see if there's a transform
  // on it
  vtkMRMLTransformNode* tnode = flist->GetParentTransformNode();
  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsTransformToWorldLinear())
    {
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    }
  // will convert by the inverted parent transform
  transformToWorld->Invert();
  double xyzw[4];
  xyzw[0] = x;
  xyzw[1] = y;
  xyzw[2] = z;
  xyzw[3] = 1.0;
  double worldxyz[4], *worldp = &worldxyz[0];

  transformToWorld->MultiplyPoint(xyzw, worldp);

  tnode = NULL;

  vtkDebugMacro("AddFiducialPicked: transformed point " << x << ", " << y << ", " << z  << " to " << worldxyz[0] << ", " << worldxyz[1] << ", " << worldxyz[2] << ", calling AddFiducialSelected");
  return this->AddFiducialSelected(worldxyz[0], worldxyz[1], worldxyz[2], selected);
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode *vtkSlicerFiducialsLogic::LoadFiducialList(const char* path)
{
  this->GetMRMLScene()->SaveStateForUndo();

  vtkMRMLNode *node =
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLFiducialListNode");
  if (node == NULL)
    {
    return NULL;
    }

  // the name is set before adding to the scene so that node selectors will be updated
  std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(path);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));
  node->SetName(uname.c_str());

  this->GetMRMLScene()->AddNode(node);
  node->Delete();

  vtkMRMLFiducialListNode *listNode = vtkMRMLFiducialListNode::SafeDownCast(node);

  vtkMRMLStorageNode *snode = listNode->CreateDefaultStorageNode();
  snode->SetFileName(path);
  this->GetMRMLScene()->AddNode(snode);

  listNode->SetAndObserveStorageNodeID(snode->GetID());
  int retval = snode->ReadData(listNode);

  if (retval == 0)
    {
      vtkErrorMacro("LoadFiducialList: error reading fiducial file " << path);
      // remove the nodes
      this->GetMRMLScene()->RemoveNode(snode);
      this->GetMRMLScene()->RemoveNode(listNode);
      snode->Delete();
      return NULL;
    }

  snode->Delete();

  return listNode;
}
