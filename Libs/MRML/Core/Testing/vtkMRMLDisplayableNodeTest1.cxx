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
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
class vtkMRMLDisplayableNodeTestHelper1 : public vtkMRMLDisplayableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayableNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLDisplayableNodeTestHelper1, vtkMRMLDisplayableNode);

  // Return the raw list of display nodes. Elements can be 0 even if the node
  // associated to the node ID exists in the scene.
  const std::vector<vtkMRMLDisplayNode*> GetInternalDisplayNodes()
    {
    std::vector<vtkMRMLDisplayNode*> InternalDisplayNodes;
    int ndnodes = this->GetNumberOfDisplayNodes();
    for (int i=0; i<ndnodes; i++)
      {
      InternalDisplayNodes.push_back(this->GetNthDisplayNode(i));
      }
    return InternalDisplayNodes;
    }
  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLDisplayableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLDisplayableNodeTestHelper1";
    }

  vtkMRMLStorageNode* CreateDefaultStorageNode() override
    {
    // just some random storage node to pass the storage node test of basic MRML node tests
    return vtkMRMLStorageNode::SafeDownCast(vtkMRMLModelStorageNode::New());
    }
};
vtkStandardNewMacro(vtkMRMLDisplayableNodeTestHelper1);

//----------------------------------------------------------------------------
class vtkMRMLDisplayNodeTestHelper : public vtkMRMLDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayNodeTestHelper *New();

  vtkTypeMacro(vtkMRMLDisplayNodeTestHelper, vtkMRMLDisplayNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLDisplayNodeTestHelper::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLDisplayNodeTestHelper";
    }

};
vtkStandardNewMacro(vtkMRMLDisplayNodeTestHelper);

bool TestAddDisplayNodeID();
bool TestAddDisplayNodeIDWithNoScene();
bool TestAddDisplayNodeIDEventsWithNoScene();
bool TestAddDelayedDisplayNode();
bool TestRemoveDisplayNodeID();
bool TestRemoveDisplayNode();
bool TestRemoveDisplayableNode();
bool TestDisplayModifiedEvent();
bool TestReferences();

//----------------------------------------------------------------------------
int vtkMRMLDisplayableNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLDisplayableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  bool res = true;
  res = TestAddDisplayNodeID() && res;
  res = TestAddDisplayNodeIDWithNoScene() && res;
  //res = TestAddDisplayNodeIDEventsWithNoScene() && res;
  res = TestAddDelayedDisplayNode() && res;
  res = TestRemoveDisplayNodeID() && res;
  res = TestRemoveDisplayNode() && res;
  res = TestRemoveDisplayableNode() && res;
  res = TestDisplayModifiedEvent() && res;
  res = TestReferences() && res;

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
  displayableNode->AddAndObserveDisplayNodeID(nullptr);
  if (displayableNode->GetNumberOfDisplayNodes() != 0 ||
      displayableNode->GetNthDisplayNodeID(0) != nullptr ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
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

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      newReferencedNodesCount != (referencedNodesCount + 1))
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Add empty display node ID
  displayableNode->AddAndObserveDisplayNodeID(nullptr);
  if (displayableNode->GetNumberOfDisplayNodes() != 1 ||
      displayableNode->GetNthDisplayNodeID(1) != nullptr ||
      displayableNode->GetNthDisplayNode(1) != nullptr)
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Change display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
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

  if (displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer() ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
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

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
    {
    std::cout << __LINE__ << ": AddAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Change display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode2;
  scene->AddNode(displayNode2.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode2->GetID());

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  /// Add display node
  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode3;
  scene->AddNode(displayNode3.GetPointer());

  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());

  if (displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != nullptr ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode failed" << std::endl;
    return false;
    }

  // Finally, add the node into the scene so it can look for the display nodes
  // in the scene.
  scene->AddNode(displayableNode.GetPointer());
  const std::vector<vtkMRMLDisplayNode*> &internalNodes = displayableNode->GetInternalDisplayNodes();
  if (internalNodes.size() != 2 ||
      internalNodes[0] == nullptr ||
      internalNodes[1] == nullptr)
    {
    std::cout << __LINE__ << ": AddNode failed" << std::endl;
    return false;
    }

  // Test the scanning of GetDisplayNode
  vtkMRMLDisplayNode* nthDisplayNode = displayableNode->GetNthDisplayNode(1);

  if (displayableNode->GetInternalDisplayNodes().size() != 2 ||
      nthDisplayNode != displayNode3.GetPointer() ||
      displayableNode->GetInternalDisplayNodes()[1] != displayNode3.GetPointer() ||
      displayableNode->GetInternalDisplayNodes()[0] == nullptr)
    {
    std::cout << __LINE__ << ": GetNthDisplayNode failed" << std::endl;
    return false;
    }

  // Typically called by vtkMRMLScene::Import
  displayableNode->UpdateScene(scene.GetPointer());

  if (displayableNode->GetInternalDisplayNodes().size() != 2 ||
      displayableNode->GetInternalDisplayNodes()[0] != displayNode2.GetPointer() ||
      displayableNode->GetInternalDisplayNodes()[1] != displayNode3.GetPointer() ||
      displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer() ||
      // make sure it didn't change the first display node ID
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": AddNode failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestAddDisplayNodeIDEventsWithNoScene()
{
  // Make sure that the DisplayableModifiedEvent is fired even when the
  // display node is observed when the displayable is not in the scene.
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableNodeTestHelper1> displayableNode;

  vtkNew<vtkMRMLDisplayNodeTestHelper> displayNode;
  scene->AddNode(displayNode.GetPointer());
  displayableNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  displayableNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  scene->AddNode(displayableNode.GetPointer());

  if (!callback->GetErrorString().empty() ||
      // called because added into the scene
      callback->GetNumberOfModified() != 1 ||
      // called because display node pointer is retrieved by scene and is
      // observed by displayable node
      callback->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cerr << "ERROR line " << __LINE__ << ": " << std::endl
              << "vtkMRMLScene::AddNode(displayableNode) failed. "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of DisplayModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent)
              << std::endl;
    return false;
    }
  callback->ResetNumberOfEvents();

  displayNode->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cerr << "ERROR line " << __LINE__ << ": " << std::endl
              << "vtkMRMLDisplayNode::Modified() failed. "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of DisplayModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent)
              << std::endl;
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

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), "vtkMRMLDisplayNodeTestHelper1") ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed" << std::endl;
    return false;
    }

  scene->AddNode(displayNode1.GetPointer());

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), "vtkMRMLDisplayNodeTestHelper1"))
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed" << std::endl;
    return false;
    }

  // Search for the node in the scene.
  vtkMRMLNode* displayNode = displayableNode->GetNthDisplayNode(0);

  if (displayableNode->GetNthDisplayNodeID(0) == nullptr ||
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
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode3.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthDisplayNodeID failed" << std::endl;
    return false;
    }

  displayableNode->SetAndObserveNthDisplayNodeID(1, nullptr);

  if (displayableNode->GetNumberOfDisplayNodes() != 1 ||
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNode(1, 0) failed" << std::endl;
    return false;
    }

  displayableNode->RemoveAllDisplayNodeIDs();

  if (displayableNode->GetNumberOfDisplayNodes() != 0 ||
      displayableNode->GetNthDisplayNodeID(0) != nullptr ||
      displayableNode->GetNthDisplayNode(0) != nullptr)
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
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != displayNode1.GetPointer() ||
      displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthDisplayNodeID failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveDisplayableNode()
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

  scene->RemoveNode(displayableNode.GetPointer());
  // Removing the scene from the displayable node clear the cached display
  // nodes.
  vtkMRMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(0);
  std::vector<vtkMRMLDisplayNode*> displayNodes =
    displayableNode->GetInternalDisplayNodes();

  if (displayableNode->GetNumberOfDisplayNodes() != 3 ||
      displayNode != nullptr ||
      displayNodes.size() != 3 ||
      displayNodes[0] != nullptr ||
      displayNodes[1] != nullptr ||
      displayNodes[2] != nullptr ||
      displayableNode->GetNthDisplayNodeID(0) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(0), displayNode1->GetID()) ||
      displayableNode->GetNthDisplayNode(0) != nullptr ||
      displayableNode->GetNthDisplayNodeID(1) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(1), displayNode2->GetID()) ||
      displayableNode->GetNthDisplayNode(1) != nullptr ||
      displayableNode->GetNthDisplayNodeID(2) == nullptr ||
      strcmp(displayableNode->GetNthDisplayNodeID(2), displayNode3->GetID()) ||
      displayableNode->GetNthDisplayNode(2) != nullptr
      )
    {
    std::cout << __LINE__ << ": RemoveNode failed" << std::endl;
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

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> spy;
  displayableNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode1->GetID());

  if (spy->GetTotalNumberOfEvents() != 3 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << " "
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

  if (spy->GetTotalNumberOfEvents() != 3 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  displayableNode->SetAndObserveDisplayNodeID(nullptr);

  if (spy->GetTotalNumberOfEvents() != 3 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceRemovedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceRemovedEvent) << " "<< std::endl
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

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) != 1 ||
      displayNode != displayNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLDisplayableNode::DisplayModifiedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//----------------------------------------------------------------------------
bool TestReferences()
{
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer<vtkMRMLDisplayNodeTestHelper> displayNode1 =
    vtkSmartPointer<vtkMRMLDisplayNodeTestHelper>::New();
  scene->AddNode(displayNode1);

  vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1> displayableNode =
    vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1>::New();
  scene->AddNode(displayableNode);

  displayableNode->AddAndObserveDisplayNodeID(displayNode1->GetID());

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(
    scene->GetReferencedNodes(displayableNode.GetPointer()));

  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != displayableNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != displayNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Observing a display node not yet in the scene should add the reference in
  // the mrml scene, however GetReferencedNodes can't return the node because
  // it is not yet in the scene.
  vtkSmartPointer<vtkMRMLDisplayNodeTestHelper> displayNode2 =
    vtkSmartPointer<vtkMRMLDisplayNodeTestHelper>::New();
  displayableNode->AddAndObserveDisplayNodeID("vtkMRMLDisplayNodeTestHelper2");

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(displayableNode.GetPointer()));
  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != displayableNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != displayNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  scene->AddNode(displayNode2);
  displayableNode->GetNthDisplayNode(1);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(displayableNode));
  if (referencedNodes->GetNumberOfItems() != 3 ||
      referencedNodes->GetItemAsObject(0) != displayableNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != displayNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Test if the reference removal works
  vtkSmartPointer<vtkMRMLDisplayNodeTestHelper> displayNode3 =
    vtkSmartPointer<vtkMRMLDisplayNodeTestHelper>::New();
  scene->AddNode(displayNode3);
  displayableNode->AddAndObserveDisplayNodeID(displayNode3->GetID());
  displayableNode->RemoveNthDisplayNodeID(2);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(displayableNode));
  if (referencedNodes->GetNumberOfItems() != 3 ||
      referencedNodes->GetItemAsObject(0) != displayableNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != displayNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != displayNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveDisplayNodeID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Simulate scene deletion to see if it crashes or not.
  // When the displayable node is destroyed, it unreferences nodes. Make sure
  // it is ok for nodes already removed/deleted like displayNode1.
  displayNode1 = nullptr;
  displayableNode = nullptr;
  displayNode2 = nullptr;
  displayNode3 = nullptr;
  scene = nullptr;

  return true;
}
