/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModelHierarchyLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerModelHierarchyLogic.h"

vtkCxxRevisionMacro(vtkSlicerModelHierarchyLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerModelHierarchyLogic);

//----------------------------------------------------------------------------
vtkSlicerModelHierarchyLogic::vtkSlicerModelHierarchyLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerModelHierarchyLogic::~vtkSlicerModelHierarchyLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerModelHierarchyLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
int vtkSlicerModelHierarchyLogic::CreateModelToHierarchyMap()
{
  this->ModeHierarchylNodes.clear();
  if (this->MRMLScene == NULL)
    {
    return 0;
    }

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
  for (int i=0; i<nnodes; i++)
    {
    vtkMRMLModelHierarchyNode *node =  vtkMRMLModelHierarchyNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(i, "vtkMRMLModelHierarchyNode"));
    if (node)
      {
      vtkMRMLModelNode *mnode = node->GetModelNode();
      if (mnode)
        {
        this->ModeHierarchylNodes[std::string(mnode->GetID())] = node;
        }
      }
    }
  return nnodes;
}

//---------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkSlicerModelHierarchyLogic::GetModelHierarchyNode(const char *modelNodeID)
{
  if (modelNodeID == NULL)
    {
    return NULL;
    }
  std::map<std::string, vtkMRMLModelHierarchyNode *>::iterator iter;
  
  iter = this->ModeHierarchylNodes.find(modelNodeID);
  if (iter != this->ModeHierarchylNodes.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }
  
}
