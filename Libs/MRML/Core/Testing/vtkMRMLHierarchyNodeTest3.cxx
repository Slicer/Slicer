/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <sstream>

// helper methods to check children ordering
static void PrintNames(std::vector< vtkMRMLHierarchyNode *> kids)
{
  for (unsigned int i = 0; i < kids.size(); i++)
    {
    std::cout << "\t" << i << " name = " << kids[i]->GetName() << std::endl;
    }
}

// test more ordered node hierarchy uses
int vtkMRMLHierarchyNodeTest3(int , char * [] )
{

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableHierarchyNode> hnode1;
  hnode1->SetName("Level 0");
  scene->AddNode(hnode1.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> hdnode1;
  hdnode1->SetName("Level 0 Display");
  scene->AddNode(hdnode1.GetPointer());

  if (hdnode1->GetID())
    {
    hnode1->SetAndObserveDisplayNodeID(hdnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the first hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLDisplayableHierarchyNode> hnode2;
  scene->AddNode(hnode2.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> hdnode2;
  scene->AddNode(hdnode2.GetPointer());

  if (hdnode2->GetID())
    {
    hnode2->SetName("Level 1");
    hdnode2->SetName("Level 1 Display");
    hnode2->SetAndObserveDisplayNodeID(hdnode2->GetID());
    hnode2->SetParentNodeID(hnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the second hierarchy node: "
              << "id is null on hierarchy display node" << std::endl;
    return EXIT_FAILURE;
    }

  // now add model nodes which will be children of the level 1 hierarchy

  std::vector<vtkSmartPointer<vtkMRMLModelNode> > modelNodes;
  std::vector<vtkSmartPointer<vtkMRMLModelDisplayNode> > modelDisplayNodes;
  std::vector<vtkSmartPointer<vtkMRMLModelHierarchyNode> > modelHierarchyNodes;
  unsigned int numModels = 5;
  for (unsigned int m = 0; m < numModels; m++)
    {
    modelNodes.push_back(vtkSmartPointer<vtkMRMLModelNode>::New());
    modelDisplayNodes.push_back(vtkSmartPointer<vtkMRMLModelDisplayNode>::New());
    scene->AddNode(modelNodes[m]);
    scene->AddNode(modelDisplayNodes[m]);
    if (!modelNodes[m] || !modelDisplayNodes[m] ||
        !modelDisplayNodes[m]->GetID())
      {
      std::cerr << "Error setting up a display node for the " << m << "th model node\n";
      return EXIT_FAILURE;
      }
    std::stringstream ss;
    ss << m;
    ss << "th Model";
    std::string nameString;
    ss >> nameString;
    modelNodes[m]->SetName(nameString.c_str());
    ss << " Display";
    ss >> nameString;
    modelDisplayNodes[m]->SetName(nameString.c_str());

    modelNodes[m]->SetAndObserveDisplayNodeID(modelDisplayNodes[m]->GetID());

    // now set up a hierarchy for this model
    modelHierarchyNodes.push_back(vtkSmartPointer<vtkMRMLModelHierarchyNode>::New());
    std::stringstream ss2;
    ss2 << m;
    ss2 << "th Model Hierarchy";
    nameString = ss2.str();
    modelHierarchyNodes[m]->SetName(nameString.c_str());
    scene->AddNode(modelHierarchyNodes[m]);
    modelHierarchyNodes[m]->SetParentNodeID(hnode2->GetID());
    modelHierarchyNodes[m]->SetDisplayableNodeID(modelNodes[m]->GetID());
    }

  std::cout << "Model nodes size = " << modelNodes.size() << std::endl;
  std::cout << "Model display nodes size = " <<  modelDisplayNodes.size() << std::endl;
  std::cout << "Model hierarchy nodes size = " << modelHierarchyNodes.size() << std::endl;

  // check that the top level hierarchy returns all the children
  std::vector< vtkMRMLHierarchyNode *> allChildren;
  hnode1->GetAllChildrenNodes(allChildren);
  std::cout << "Top level hierarchy children:" << std::endl;
  PrintNames(allChildren);
  if (allChildren.size() != 1 + numModels)
    {
    std::cerr << "ERROR: Top level hierarchy returned  " << allChildren.size() << " total children instead of " << 1 + numModels << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Top level hierarchy has " << allChildren.size() << " total children" << std::endl;
    }

  // check for the immediate children of the top level
  std::vector< vtkMRMLHierarchyNode *> immediateChildren = hnode1->GetChildrenNodes();
  std::cout << "Top level hierarchy immediate children:" << std::endl;
  PrintNames(immediateChildren);
  if (immediateChildren.size() != 1)
    {
    std::cerr << "ERROR: Top level hierarchy returned  " << immediateChildren.size() << " total children instead of " << 1 << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Top level hiearachy has " << immediateChildren.size() << " immediate children" << std::endl;
    }

  // check that the second level hierarchy returns all the children
  std::vector< vtkMRMLHierarchyNode *> allChildren2;
  hnode2->GetAllChildrenNodes(allChildren2);
  std::cout << "Second level hierarchy children:" << std::endl;
  PrintNames(allChildren2);
  if (allChildren2.size() != numModels)
    {
    std::cerr << "ERROR: Second level hierarchy has " << allChildren2.size() << " total children instead of " << numModels << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Second level hierarchy has " << allChildren2.size() << " total children" << std::endl;
    }

  // check for the immediate children of the second level
  std::vector< vtkMRMLHierarchyNode *> immediateChildren2 = hnode2->GetChildrenNodes();
  std::cout << "Second level hierarchy immediate children:" << std::endl;
  PrintNames(immediateChildren2);
  if (immediateChildren2.size() != numModels)
    {
    std::cerr<< "ERROR: Second level hierarchy has " << immediateChildren2.size() << " immediate children instead of " << numModels << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Second level hiearachy has " << immediateChildren2.size() << " immediate children" << std::endl;
    }

  // now check that the children are in the order they were added
  for (unsigned int i = 0; i < numModels; i++)
    {
    int indexInParent = modelHierarchyNodes[i]->GetIndexInParent();
    std::cout << "Model hierarchy node " << i << " is at index " << indexInParent << std::endl;
    if (indexInParent != (int)i)
      {
      std::cerr << "Index mismatch!" << std::endl;
      return EXIT_FAILURE;
      }
    }

  // move the first node down in the hierarchy
  int oldIndexInParent =  modelHierarchyNodes[0]->GetIndexInParent();
  int newIndexInParent = oldIndexInParent + 1;
  modelHierarchyNodes[0]->SetIndexInParent(newIndexInParent);
  int currentIndexInParent =  modelHierarchyNodes[0]->GetIndexInParent();
  if (currentIndexInParent != newIndexInParent)
    {
    std::cerr << "Error moving first hierarchy node in list from index " << oldIndexInParent << " to " << newIndexInParent << ", current index in parent is " << currentIndexInParent << std::endl;
    return EXIT_FAILURE;
    }

  // move the last one up in the hierarchy
  oldIndexInParent = modelHierarchyNodes[numModels-1]->GetIndexInParent();
  newIndexInParent = oldIndexInParent - 1;
  modelHierarchyNodes[numModels-1]->SetIndexInParent(newIndexInParent);
  currentIndexInParent = modelHierarchyNodes[numModels-1]->GetIndexInParent();
  if (currentIndexInParent != newIndexInParent)
    {
    std::cerr << "Error moving last hierarchy node in list from index " << oldIndexInParent << " to " << newIndexInParent << ", current index in parent is " << currentIndexInParent << std::endl;
    return EXIT_FAILURE;
    }

  // get the second hierarchy child and move it to be the first
  vtkMRMLHierarchyNode *node1 = hnode2->GetNthChildNode(1);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->SetIndexInParent(0);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != 0)
      {
      std::cerr << "Error moving second hierarchy child to be the first. Started at index " << oldIndexInParent << ", moved it to 0, now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }
  // get the second last and move it to be the last
  node1 = hnode2->GetNthChildNode(numModels-2);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->SetIndexInParent(numModels-1);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != (int)numModels-1)
      {
      std::cerr << "Error moving second last hierarchy child to be the last. Started at index " << oldIndexInParent << ", moved it to " << numModels-1 << ", now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }

  // move one down
  node1 = hnode2->GetNthChildNode(numModels/2);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->MoveInParent(-1);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != (int)oldIndexInParent-1)
      {
      std::cerr << "Error moving child one down. Started at index " << oldIndexInParent << ", now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }
  // move one up
  node1 = hnode2->GetNthChildNode(numModels/2);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->MoveInParent(1);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != (int)oldIndexInParent+1)
      {
      std::cerr << "Error moving child one up. Started at index " << oldIndexInParent << ", now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }
  // move two down
  node1 = hnode2->GetNthChildNode(numModels/2);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->MoveInParent(-2);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != (int)oldIndexInParent-2)
      {
      std::cerr << "Error moving child two down. Started at index " << oldIndexInParent << ", now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }
  // move two up
  node1 = hnode2->GetNthChildNode(numModels/2);
  if (node1)
    {
    oldIndexInParent = node1->GetIndexInParent();
    node1->MoveInParent(2);
    currentIndexInParent = node1->GetIndexInParent();
    if (currentIndexInParent != (int)oldIndexInParent+2)
      {
      std::cerr << "Error moving child two up. Started at index " << oldIndexInParent << ", now at " << currentIndexInParent << std::endl;
      return EXIT_FAILURE;
      }
    }


  return EXIT_SUCCESS;
}

