/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

//MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

class vtkMRMLDisplayableNodeTestHelper1 : public vtkMRMLDisplayableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayableNodeTestHelper1 *New(){return new vtkMRMLDisplayableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLDisplayableNodeTestHelper1, vtkMRMLDisplayableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLDisplayableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLDisplayableNodeTestHelper1";
    }
};

class vtkMRMLDisplayNodeTestHelper : public vtkMRMLDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayNodeTestHelper *New(){return new vtkMRMLDisplayNodeTestHelper;};

  vtkTypeMacro( vtkMRMLDisplayNodeTestHelper, vtkMRMLDisplayNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLDisplayNodeTestHelper;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLDisplayNodeTestHelper";
    }
};

bool TestAddDisplayNodeID();
bool TestAddDisplayNodeIDWithNoScene();
bool TestAddDelayedDisplayNode();
bool TestRemoveDisplayNodeID();
bool TestRemoveDisplayNode();
bool TestDisplayModifiedEvent();

//----------------------------------------------------------------------------
int vtkMRMLDisplayableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDisplayableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLDisplayableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayableNodeTestHelper1, node1);

  bool res = true;
  res = TestAddDisplayNodeID() && res;
  res = TestAddDisplayNodeIDWithNoScene() && res;
  res = TestAddDelayedDisplayNode() && res;
  res = TestRemoveDisplayNodeID() && res;
  res = TestRemoveDisplayNode() && res;
  res = TestDisplayModifiedEvent() && res;

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestAddDisplayNodeID()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1.GetPointer());

  /// Add empty display node
  displayableNode->AddAndObserveDisplayNodeID(0);
  if (displayableNode->GetNumberOfDisplayNodes() != 0 ||
      displayableNode->GetNthDisplayNodeID(0) != 0 ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode.GetPointer()));
  int referencedNodesCount = referencedNodes->GetNumberOfItems();

  /// Add display node ID
  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());

  referencedNodes.TakeReference(scene->GetReferencedNodes(displayableNode.GetPointer()));
  int newReferencedNodesCount = referencedNodes->GetNumberOfItems();

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      newReferencedNodesCount != (referencedNodesCount + 1))
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Add empty display node ID
  displayableNode->AddAndObserveDisplayNodeID(0);
  if (displayableNode->GetNumberOfDisplayNodes() != 1 ||
      displayableNode->GetNthDisplayNodeID(1) != 0 ||
      displayableNode->GetNthDisplayNode(1) != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Change display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Add display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3.GetPointer());

  displayableNode->SetAndObserveNthDisplayNodeID(1, displayNode3->GetID());

  if (displayableNode->GetNthDisplayNodeID(1) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer() ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestAddDisplayNodeIDWithNoScene()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1.GetPointer());

  /// Add display node
  displayableNode->SetAndObserveDisplayNodeID(displayNode1->GetID());

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Change display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Add display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3.GetPointer());

  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  if (displayableNode->GetNthDisplayNodeID(1) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != 0 ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  // Finally, add the node into the scene so it can look for the display nodes
  // in the scene.
  scene->AddNode(displayableNode.GetPointer());

  if (displayableNode->GetDisplayNodes().size() != 2 ||
      displayableNode->GetDisplayNodes()[0] != 0 ||
      displayableNode->GetDisplayNodes()[1] != 0)
    {
    std::cout << __LINE__ << ": AddNode failed" << std::endl;
    return false;
    }

  // Test the scanning of GetDisplayNode
  vtkMRMLDisplayNode* nthDisplayNode = displayableNode->GetNthDisplayNode(1);

  if (displayableNode->GetDisplayNodes().size() != 2 ||
      nthDisplayNode != displayNode3.GetPointer() ||
      displayableNode->GetDisplayNodes()[1] != displayNode3.GetPointer() ||
      displayableNode->GetDisplayNodes()[0] != 0)
    {
    std::cout << __LINE__ << ": GetNthDisplayNode failed" << std::endl;
    return false;
    }

  // Typically called by vtkMRMLScene::Import
  displayableNode->UpdateScene(scene.GetPointer());

  if (displayableNode->GetNthDisplayNodeID(1) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer() ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetScene failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestAddDelayedDisplayNode()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  // Set a node ID that doesn't exist but will exist.
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  displayableNode->SetAndObserveDisplayNodeID("vtkMRMLDisplayNodeTestHelper1");

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), "vtkMRMLDisplayNodeTestHelper1") ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed" << std::endl;
    return false;
    }

  scene->AddNode(displayNode1.GetPointer());

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), "vtkMRMLDisplayNodeTestHelper1") ||
      displayableNode->GetDisplayNodes()[0] != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed" << std::endl;
    return false;
    }

  // Search for the node in the scene.
  vtkMRMLNode* displayNode = displayableNode->GetNthDisplayNode(0);

  if (displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayNode != displayNode1.GetPointer() ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveDisplayNodeID()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1.GetPointer());
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3.GetPointer());

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  displayableNode->RemoveNthDisplayNodeID(1);

  if (displayableNode->GetNumberOfDisplayNodes() != 2 ||
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      displayableNode->GetNthDisplayNodeID(1) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthDisplayNodeID failed" << std::endl;
    return false;
    }

  displayableNode->SetAndObserveNthDisplayNodeID(1, 0);

  if (displayableNode->GetNumberOfDisplayNodes() != 1 ||
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode(1, 0) failed" << std::endl;
    return false;
    }

  displayableNode->RemoveAllDisplayNodeIDs();

  if (displayableNode->GetNumberOfDisplayNodes() != 0 ||
      displayableNode->GetNthDisplayNodeID(0) != 0 ||
      displayableNode->GetNthDisplayNode(0) != 0)
    {
    std::cout << __LINE__ << ": RemoveAllDisplayNodeIDs failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveDisplayNode()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1.GetPointer());
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3.GetPointer());

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode2->GetID());
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  scene->RemoveNode(displayNode3.GetPointer());

  if (displayableNode->GetNumberOfDisplayNodes() != 2 ||
      displayableNode->GetNthDisplayNodeID(0) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      displayableNode->GetNthDisplayNodeID(1) == 0 ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthDisplayNodeID failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestDisplayModifiedEvent()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode1;
  scene->AddNode(displayNode1.GetPointer());

  vtkNew<vtkMRMLNodeCallback> spy;
  displayableNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode1->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  displayNode1->Modified();

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());
  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  displayableNode->SetAndObserveDisplayNodeID(0);

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  displayableNode->SetAndObserveDisplayNodeID("vtkMRMLDisplayNodeTestHelper3");

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  scene->AddNode(displayNode3.GetPointer());
  // update the reference of the node
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}
