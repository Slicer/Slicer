/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDisplayableHierarchyNode.h"


#include "vtkMRMLCoreTestingMacros.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// test more general hierachy uses, with different displayable node types
int vtkMRMLDisplayableHierarchyNodeTest2(int , char * [] )
{

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > hnode1 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  if (hnode1 == NULL)
    {
    std::cerr << "Error making a new hierarchy node.\n";
    return EXIT_FAILURE;
    }
  vtkSmartPointer<vtkMRMLModelDisplayNode> hdnode1 = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  if (hdnode1 == NULL)
    {
    std::cerr << "Error making a new hierarchy display node.\n";
    return EXIT_FAILURE;
    }
  scene->AddNode(hnode1);
  scene->AddNode(hdnode1);
  if (hnode1 && hdnode1 &&
      hdnode1->GetID())
    {
    hnode1->SetAndObserveDisplayNodeID(hdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first hierarchy node:";
    if (hnode1 == NULL) { std::cerr << "\thierarchy node is null\n"; }
    if (hdnode1 == NULL) { std::cerr << "\thierarhcy display node is null\n"; }
    if (!hdnode1->GetID()) { std:: cerr << "\nid is null on hierarchy display node\n"; }
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > hnode2 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> hdnode2 = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();

  scene->AddNode(hnode2);
  scene->AddNode(hdnode2);
  if (hnode2 && hdnode2 &&
      hdnode2->GetID())
    {
    hnode2->SetAndObserveDisplayNodeID(hdnode2->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the second hierarchy node:";
    if (hnode2 == NULL) { std::cerr << "\thierarchy node is null\n"; }
    if (hdnode2 == NULL) { std::cerr << "\thierarhcy display node is null\n"; }
    if (!hdnode2->GetID()) { std:: cerr << "\nid is null on hierarchy display node\n"; }
    return EXIT_FAILURE;
    }
  
  vtkSmartPointer<vtkMRMLModelNode> mnode1 = vtkSmartPointer<vtkMRMLModelNode>::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> mdnode1 = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();

  scene->AddNode(mnode1);
  scene->AddNode(mdnode1);
  if (mnode1 && mdnode1 &&
      mdnode1->GetID())
    {
    mnode1->SetAndObserveDisplayNodeID(mdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first model node\n";
    return EXIT_FAILURE;
    }
  
  vtkSmartPointer<vtkMRMLScalarVolumeNode> vnode1 = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> vdnode1 = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  
  scene->AddNode(vnode1);
  scene->AddNode(vdnode1);

  if (vnode1 && vdnode1 &&
      vdnode1->GetID())
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
