/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

// test more general hierarchy uses, with different displayable node types
int vtkMRMLDisplayableHierarchyNodeTest2(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew< vtkMRMLDisplayableHierarchyNode > hnode1;
  scene->AddNode(hnode1.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> hdnode1;
  scene->AddNode(hdnode1.GetPointer());

  if (hdnode1->GetID())
    {
    hnode1->SetAndObserveDisplayNodeID(hdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLDisplayableHierarchyNode> hnode2;
  scene->AddNode(hnode2.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> hdnode2;
  scene->AddNode(hdnode2.GetPointer());

  if (hdnode2->GetID())
    {
    hnode2->SetAndObserveDisplayNodeID(hdnode2->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the second hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLModelNode> mnode1;
  scene->AddNode(mnode1.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> mdnode1;
  scene->AddNode(mdnode1.GetPointer());

  if (mdnode1->GetID())
    {
    mnode1->SetAndObserveDisplayNodeID(mdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first model node\n";
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScalarVolumeNode> vnode1;
  scene->AddNode(vnode1.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> vdnode1;
  scene->AddNode(vdnode1.GetPointer());

  if (vdnode1->GetID())
    {
    vnode1->SetAndObserveDisplayNodeID(vdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first volume node\n";
    return EXIT_FAILURE;
    }

  // now set up a hierarchy
  hnode2->SetDisplayableNodeID(vnode1->GetID());
  hnode2->SetParentNodeID(hnode1->GetID());
  hnode1->SetDisplayableNodeID(mnode1->GetID());

  return EXIT_SUCCESS;
}
