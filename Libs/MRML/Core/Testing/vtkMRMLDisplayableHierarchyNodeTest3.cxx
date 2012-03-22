/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelHierarchyNode.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

// helper methods to check children ordering
static void PrintNames(std::vector< vtkMRMLHierarchyNode *> kids)
{
  for (unsigned int i = 0; i < kids.size(); i++)
    {
    if (kids[i] != NULL)
      {
      std::cout << "\t" << i << " name = " << (kids[i]->GetName() != NULL ? kids[i]->GetName() : "NULL") << std::endl;
      std::cout << "\t\tID = " << (kids[i]->GetID() ? kids[i]->GetID() : "(no id)") << std::endl;
      std::cout << "\t\tnumber of children nodes = " << kids[i]->GetNumberOfChildrenNodes() << std::endl;
      }
    }
}

// test more ordered node hierachy uses
int vtkMRMLDisplayableHierarchyNodeTest3(int , char * [] )
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
  hnode1->SetName("Level 0");
  hdnode1->SetName("Level 0 Display");
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
    if (hdnode1 == NULL) { std::cerr << "\thierarchy display node is null\n"; }
    if (!hdnode1->GetID()) { std:: cerr << "\nid is null on hierarchy display node\n"; }
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > hnode2 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> hdnode2 = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();

  scene->AddNode(hnode2);
  scene->AddNode(hdnode2);
  if (hnode2 && hdnode2 &&
      hdnode2->GetID())
    {
    hnode2->SetName("Level 1");
    hdnode2->SetName("Level 1 Display");
    hnode2->SetAndObserveDisplayNodeID(hdnode2->GetID());
    hnode2->SetParentNodeID(hnode1->GetID());
    }
  else
    {
    std::cerr << "Error setting up a display node for the second hierarchy node:";
    if (hnode2 == NULL) { std::cerr << "\thierarchy node is null\n"; }
    if (hdnode2 == NULL) { std::cerr << "\thierarchy display node is null\n"; }
    if (!hdnode2->GetID()) { std:: cerr << "\nid is null on hierarchy display node\n"; }
    return EXIT_FAILURE;
    }

  // now add model nodes which will be children of the level 1 hierarchy

  std::vector<vtkSmartPointer<vtkMRMLModelNode> > modelNodes;
  std::vector<vtkSmartPointer<vtkMRMLModelDisplayNode> > modelDisplayNodes;
  std::vector<vtkSmartPointer<vtkMRMLModelHierarchyNode> > modelHierarchyNodes;
  unsigned int numModels = 5;
  for (unsigned int m = 0; m < numModels; m++)
    {
    // first set up a hierarchy for this model
    modelHierarchyNodes.push_back(vtkSmartPointer<vtkMRMLModelHierarchyNode>::New());
    std::stringstream ss;
    ss << m;
    ss << "th Model Hierarchy";
    std::string nameString = ss.str();
    modelHierarchyNodes[m]->SetName(nameString.c_str());
    modelHierarchyNodes[m]->SetParentNodeID(hnode2->GetID());
    
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
    std::stringstream ss2;
    ss2 << m;
    ss2 << "th Model";
    ss2 >> nameString;
    modelNodes[m]->SetName(nameString.c_str());
    ss << " Display";
    nameString = ss.str();
    modelDisplayNodes[m]->SetName(nameString.c_str());

    modelNodes[m]->SetAndObserveDisplayNodeID(modelDisplayNodes[m]->GetID());
    
    scene->AddNode(modelHierarchyNodes[m]);

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
    std::cerr << "ERROR: Top level hiearchy returned  " << allChildren.size() << " total children instead of " << 1 + numModels << std::endl;
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
    std::cerr << "ERROR: Second level hierarchy has " << allChildren2.size() << " total children insted of " << numModels << std::endl;
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
    std::cerr<< "ERROR: Second level hierarachy has " << immediateChildren2.size() << " immediate children instead of " << numModels << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Second level hiearchy has " << immediateChildren2.size() << " immediate children" << std::endl;
    }
  
  // now check that the children are in the order they were added
  for (unsigned int i = 0; i < numModels; i++)
    {
    vtkSmartPointer<vtkMRMLHierarchyNode> childHierarchy = NULL;
    childHierarchy = hnode2->GetNthChildNode(i);
    if (!childHierarchy)
      {
      std::cerr << "ERROR getting " << i << "th child on second level hierarchy " << hnode2->GetName() << std::endl;
      return EXIT_FAILURE;
      }
    else
      {
      std::stringstream ss2;
      ss2 << i;
      ss2 << "th Model Hierarchy";
      std::string nameString = ss2.str();
      if (nameString.compare(childHierarchy->GetName()) != 0)
        {
        std::cerr << "Expected " << i << "th child hierarchy to have the name " << nameString << ", instead have " << childHierarchy->GetName() << std::endl;
        return EXIT_FAILURE;
        }
      }
      
    }
  // now shuffle the nodes so that start with
  // 0 1 2 3 4
  modelHierarchyNodes[4]->SetIndexInParent(0);
  // now have 4 0 1 2 3
  modelHierarchyNodes[3]->SetIndexInParent(1);
  // now have 4 3 0 1 2
  modelHierarchyNodes[2]->SetIndexInParent(2);
  // now have 4 3 2 0 1
  modelHierarchyNodes[0]->SetIndexInParent(4);
  // now have 4 3 2 1 0
  immediateChildren2 = hnode2->GetChildrenNodes();
  std::cout << "Second level hierarchy immediate children after shuffle, expecting reverse order of names:" << std::endl;
  PrintNames(immediateChildren2);
  
  
  // now add some nodes out of order
  vtkSmartPointer<vtkMRMLModelNode> m6 = vtkSmartPointer<vtkMRMLModelNode>::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> md6 = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  vtkSmartPointer<vtkMRMLModelHierarchyNode> mh6 = vtkSmartPointer<vtkMRMLModelHierarchyNode>::New();
  scene->AddNode(mh6);
  mh6->SetName("MH6");
  scene->AddNode(m6);
  m6->SetName("M6");
  scene->AddNode(md6);
  md6->SetName("MD6");
   
  if (m6 && m6->GetID() && mh6 && md6 && md6->GetID())
    {
    m6->SetAndObserveDisplayNodeID(md6->GetID());
    mh6->SetDisplayableNodeID(m6->GetID());
    mh6->SetParentNodeID(hnode2->GetID());
    mh6->SetIndexInParent(3);
    }
  else
    {
    return EXIT_FAILURE;
    }
  immediateChildren2 = hnode2->GetChildrenNodes();
  std::cout << "Second level hierarchy immediate children after inserting node 6 at index 3:" << std::endl;
  PrintNames(immediateChildren2);
  vtkSmartPointer<vtkMRMLHierarchyNode> testInsert = hnode2->GetNthChildNode(3);
  if (!testInsert ||
      strcmp("MH6", testInsert->GetName()) != 0)
    {
    std::cerr << "Error inserting a hierarchy at index 6, got back nth child named " << testInsert->GetName() << " instead of MH6" << std::endl;
    return EXIT_FAILURE;
    }

  // try setting indices out of range
  mh6->SetIndexInParent(100);
  mh6->SetIndexInParent(-1);

  // clean up
  hnode2->RemoveHierarchyChildrenNodes();
  if (hnode2->GetNumberOfChildrenNodes() != 0)
    {
    std::cerr << "Error removing hierarchy children nodes from top level, have " << hnode2->GetNumberOfChildrenNodes() << " children left" << std::endl;
    return EXIT_FAILURE;
    }
  hnode1->RemoveAllHierarchyChildrenNodes();
  if (hnode1->GetNumberOfChildrenNodes() != 0)
    {
    std::cerr << "Error removing all hierarchy children nodes from top level, have " << hnode1->GetNumberOfChildrenNodes() << " children left" << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
