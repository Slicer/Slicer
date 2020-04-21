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
vtkSlicerFiducialsLogic::vtkSlicerFiducialsLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerFiducialsLogic::~vtkSlicerFiducialsLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiducialsLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode *vtkSlicerFiducialsLogic::GetSelectedList()
{
  vtkMRMLFiducialListNode *fList = nullptr;
  vtkMRMLSelectionNode *selnode = nullptr;

  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));

  if (selnode != nullptr)
    {
    if (selnode->GetActiveFiducialListID() != nullptr)
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
            this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  if (selnode && node)
    {
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
  vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take(
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLFiducialListNode"));
  vtkMRMLFiducialListNode* fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
  if (fiducialListNode == nullptr)
    {
    return nullptr;
    }
  this->GetMRMLScene()->AddNode(fiducialListNode);
  fiducialListNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("L"));
  fiducialListNode->AddDefaultStorageNode();
  return fiducialListNode;
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
  if (flist == nullptr)
    {
    this->AddFiducialListSelected();
    flist = this->GetSelectedList();
    }

  int index = -1;

  // check one more time
  if (flist == nullptr)
    {
     vtkErrorMacro("AddFiducialSelected: no selected list to which to add a fiducial, even tried adding one");
     return index;
    }

  // add a fiducial to the selected list
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
  if (flist == nullptr)
    {
    vtkDebugMacro("AddFiducialPicked: no selected list, calling AddFiducialSelected");
    return this->AddFiducialSelected(x, y, z, selected);
    }

  // otherwise, we have an existing list, check to see if there's a transform
  // on it
  vtkMRMLTransformNode* tnode = flist->GetParentTransformNode();
  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();
  if (tnode != nullptr && tnode->IsTransformToWorldLinear())
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

  tnode = nullptr;

  vtkDebugMacro("AddFiducialPicked: transformed point " << x << ", " << y << ", " << z  << " to " << worldxyz[0] << ", " << worldxyz[1] << ", " << worldxyz[2] << ", calling AddFiducialSelected");
  return this->AddFiducialSelected(worldxyz[0], worldxyz[1], worldxyz[2], selected);
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode *vtkSlicerFiducialsLogic::LoadFiducialList(const char* path)
{
  vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take(
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLFiducialListNode"));
  vtkMRMLFiducialListNode *listNode = vtkMRMLFiducialListNode::SafeDownCast(node);
  if (listNode == nullptr)
    {
    vtkErrorMacro("vtkSlicerFiducialsLogic::LoadFiducialList: failed to create vtkMRMLFiducialListNode");
    return nullptr;
    }

  // the name is set before adding to the scene so that node selectors will be updated
  std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(path);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));
  node->SetName(uname.c_str());

  this->GetMRMLScene()->AddNode(listNode);

  bool success = listNode->AddDefaultStorageNode(path);
  vtkMRMLStorageNode *snode = listNode->GetStorageNode();
  if (!success || !snode)
    {
    vtkErrorMacro("vtkSlicerFiducialsLogic::LoadFiducialList: failed to add storage node");
    this->GetMRMLScene()->RemoveNode(listNode);
    return nullptr;
    }

  int retval = snode->ReadData(listNode);
  if (retval == 0)
    {
      vtkErrorMacro("LoadFiducialList: error reading fiducial file " << path);
      // remove the nodes
      this->GetMRMLScene()->RemoveNode(snode);
      this->GetMRMLScene()->RemoveNode(listNode);
      return nullptr;
    }

  return listNode;
}
