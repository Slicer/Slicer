/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkMRMLHierarchyStorageNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScene.h"

#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLHierarchyStorageNode);

//----------------------------------------------------------------------------
vtkMRMLHierarchyStorageNode::vtkMRMLHierarchyStorageNode()
{
  this->DefaultWriteFileExtension = "txt";
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyStorageNode::~vtkMRMLHierarchyStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLHierarchyStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLHierarchyNode");
}

//----------------------------------------------------------------------------
int vtkMRMLHierarchyStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLHierarchyStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLHierarchyNode *hierarchyNode = nullptr;
  if ( refNode->IsA("vtkMRMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkMRMLHierarchyNode *> (refNode);
    }

  if (hierarchyNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a hierarchy node");
    return 0;
    }

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    //turn off modified events
    int modFlag = hierarchyNode->GetDisableModifiedEvent();
    hierarchyNode->DisableModifiedEventOn();

    // do the reading here if necessary, but it's not right now

    hierarchyNode->SetDisableModifiedEvent(modFlag);
    hierarchyNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, hierarchyNode);
    fstr.close();
    }
  else
    {
    vtkErrorMacro("ERROR opening file " << this->FileName << endl);
    return 0;
    }

  // make sure that the list node points to this storage node
  //-------------------------> hierarchyNode->SetAndObserveStorageNodeID(this->GetID());

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLHierarchyStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLHierarchyStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLHierarchyNode *hierarchyNode = nullptr;
  if ( refNode->IsA("vtkMRMLHierarchyNode") )
    {
    hierarchyNode = dynamic_cast <vtkMRMLHierarchyNode *> (refNode);
    }

  if (hierarchyNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known hierarchy node");
    return 0;
    }

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
  {
  vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
  return 0;
  }

  // put down a header
  of << "# hierarchy file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;

  of.close();

  this->StageWriteData(refNode);

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
