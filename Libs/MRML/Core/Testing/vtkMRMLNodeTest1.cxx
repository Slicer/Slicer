/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNodeReference.h"

// VTK includes
#include <vtkNew.h>
#include <vtkCollection.h>

//---------------------------------------------------------------------------
class vtkMRMLNodeTestHelper1 : public vtkMRMLNode
{
public:
  // Provide a concrete New.
  static vtkMRMLNodeTestHelper1 *New(){return new vtkMRMLNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLNodeTestHelper1,vtkMRMLNode);

  NodeReferencesType& GetInternalReferencedNodes()
    {
    return this->NodeReferences;
    }

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLNodeTestHelper1";
    }
};

//---------------------------------------------------------------------------
bool TestAttribute();

bool TestAddReferenceNodeID();
bool TestAddRefrencedNodeIDWithNoScene();
bool TestAddDelayedReferenceNode();
bool TestRemoveReferencedNodeID();
bool TestRemoveReferencedNode();
bool TestRemoveReferencingNode();
bool TestNodeReferences();
bool TestReferenceModifiedEvent();
bool TestAddReferenceedNodeIDEventsWithNoScene();
bool TestReferenceNodeNoObservers();


//---------------------------------------------------------------------------
int vtkMRMLNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLNodeTestHelper1, node1);

  bool res = true;
  res = TestAttribute();

  res = TestAddReferenceNodeID() && res;
  res = TestAddRefrencedNodeIDWithNoScene() && res;
  res = TestAddDelayedReferenceNode() && res;
  res = TestRemoveReferencedNodeID() && res;
  res = TestRemoveReferencedNode() && res;
  res = TestRemoveReferencingNode() && res;
  res = TestNodeReferences() && res;
  res = TestReferenceModifiedEvent() && res;
  res = TestAddReferenceedNodeIDEventsWithNoScene() && res;
  res = TestReferenceNodeNoObservers() && res;

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool TestSetAttribute(const char* attribute, const char* value,
                      const char* expectedValue,
                      size_t expectedSize = 1,
                      int expectedModified = 0)
{
  vtkNew<vtkMRMLNodeTestHelper1> node;
  node->SetAttribute("Attribute0", "Value0");

  vtkNew<vtkMRMLNodeCallback> spy;
  node->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  node->SetAttribute(attribute, value);
  if ((expectedValue == 0 && node->GetAttribute(attribute) != 0) ||
      (expectedValue != 0 && strcmp(expectedValue, node->GetAttribute(attribute)) != 0))
    {
    std::cout << __LINE__ << ": TestSetAttribute failed: "
              << "attribute: " << (attribute ? attribute : "null") << " "
              << "value: " << (value ? value : "null")
              << std::endl;
    return false;
    }
  if (node->GetAttributeNames().size() != expectedSize)
    {
    std::cout << __LINE__ << ": TestSetAttribute failed: "
              << "attribute: " << (attribute ? attribute : "null") << " "
              << "value: " << (value ? value : "null")
              << std::endl;
    return false;
    }
  if (spy->GetTotalNumberOfEvents() != expectedModified ||
    spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != expectedModified)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//---------------------------------------------------------------------------
bool TestAttribute()
{
  vtkNew<vtkMRMLNodeTestHelper1> node;
  // Test defaults and make sure it doesn't crash
  if (node->GetAttribute(0) != 0 ||
      node->GetAttributeNames().size() != 0 ||
      node->GetAttribute("") != 0 ||
      node->GetAttribute("Attribute1") != 0)
    {
    std::cout << "vtkMRMLNode bad default attributes" << std::endl;
    return false;
    }

  // Test sets
  bool res = true;
  res = TestSetAttribute(0,0,0) && res;
  res = TestSetAttribute(0,"",0) && res;
  res = TestSetAttribute(0,"Value1",0) && res;
  res = TestSetAttribute("",0,0) && res;
  res = TestSetAttribute("","",0) && res;
  res = TestSetAttribute("","Value1",0) && res;
  res = TestSetAttribute("Attribute1",0,0) && res;
  res = TestSetAttribute("Attribute1","","",2,1) && res;
  res = TestSetAttribute("Attribute1","Value1","Value1",2,1) && res;
  res = TestSetAttribute("Attribute0",0,0,0,1) && res;
  res = TestSetAttribute("Attribute0","","",1,1) && res;
  res = TestSetAttribute("Attribute0","Value1","Value1",1,1) && res;
  res = TestSetAttribute("Attribute0","Value0","Value0",1,0) && res;
  return res;
}

//----------------------------------------------------------------------------
bool TestAddReferenceNodeID()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;

  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add empty referenced node with empty role
  returnNode = referencingNode->AddAndObserveNodeReferenceID(0, 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode.GetPointer()));
  int referencedNodesCount = referencedNodes->GetNumberOfItems();

  /// Add referenced node ID
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode.GetPointer()));
  int newReferencedNodesCount = referencedNodes->GetNumberOfItems();

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer() ||
      newReferencedNodesCount != (referencedNodesCount + 1) ||
      returnNode != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": AddAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add empty referenced node ID
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 1 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  returnNode = referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer() ||
      returnNode != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer() ||
      returnNode != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add different role

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  returnNode = referencingNode->SetAndObserveNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode22->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 0) != referencedNode22.GetPointer() ||
      returnNode != referencedNode22.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 1), referencedNode23->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 1) != referencedNode23.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode22->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 0) != referencedNode22.GetPointer() ||
      returnNode != referencedNode23.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// make sure it didnt change the first role references
  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// change reference and check that it did
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 1) != referencedNode3.GetPointer() ||
      returnNode != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }



  return true;
}

//----------------------------------------------------------------------------
bool TestAddRefrencedNodeIDWithNoScene()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;

  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add referenced node
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != 0 ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0)
    {
    std::cout << __LINE__ << ": AddAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  // Finally, add the node into the scene so it can look for the referenced nodes
  // in the scene.
  scene->AddNode(referencingNode.GetPointer());

  //std::map< std::string, std::vector< vtkMRMLNodeReference *> > refs = referencingNode->GetInternalReferencedNodes();

  //refs = referencingNode->GetInternalReferencedNodes();
  /**
  if (referencingNode->GetInternalReferencedNodes()[role1].size() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode != referencedNode2.GetPointer() ||
      referencingNode->GetInternalReferencedNodes()[role1][1]->ReferencedNode != referencedNode3.GetPointer() ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": AddNode failed" << std::endl;
    return false;
    }
   **/
  if (
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": AddNode failed" << std::endl;
    return false;
    }
  return true;
}


//----------------------------------------------------------------------------
bool TestAddDelayedReferenceNode()
{
  vtkNew<vtkMRMLScene> scene;
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  // Set a node ID that doesn't exist but will exist.
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(),"vtkMRMLNodeTestHelper12");

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), "vtkMRMLNodeTestHelper12") ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  scene->AddNode(referencedNode1.GetPointer());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), "vtkMRMLNodeTestHelper12") ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode != 0)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  // Search for the node in the scene.
  vtkMRMLNode* referencedNode = referencingNode->GetNthNodeReference(role1.c_str(), 0);

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencedNode != referencedNode1.GetPointer() ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }
  return true;
}


//----------------------------------------------------------------------------
bool TestRemoveReferencedNodeID()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  referencingNode->RemoveNthNodeReferenceID(role1.c_str(), 1);

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 2 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer() ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthNodeReferenceID failed" << std::endl;
    return false;
    }

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, 0);

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 1 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID(1, 0) failed" << std::endl;
    return false;
    }

  // add second role refs
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode3->GetID());

  referencingNode->RemoveNthNodeReferenceID(role2.c_str(), 1);

  if (referencingNode->GetNumberOfNodeReferences(role2.c_str()) != 1 ||
      referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 0) != referencedNode2.GetPointer() ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) )
    {
    std::cout << __LINE__ << ": RemoveNthNodeReferenceID failed" << std::endl;
    return false;
    }


  referencingNode->RemoveAllNodeReferenceIDs(role1.c_str());

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 || 
      referencingNode->GetNumberOfNodeReferences(role2.c_str()) != 1 ||
      referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode2->GetID()))
    {
    std::cout << __LINE__ << ": RemoveAllNodeReferenceIDs failed" << std::endl;
    return false;
    }

  referencingNode->RemoveAllNodeReferenceIDs(0);

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0)
    {
    std::cout << __LINE__ << ": RemoveAllNodeReferenceIDs failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencedNode()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  scene->RemoveNode(referencedNode3.GetPointer());

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 2 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer() ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": RemoveNthReferenceNodeID failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencingNode()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  scene->RemoveNode(referencingNode.GetPointer());
  // Removing the scene from the  node clear the cached referenced
  // nodes.
  vtkMRMLNode* referencedNode = referencingNode->GetNthNodeReference(role1.c_str(), 0);
  std::vector<vtkMRMLNode*> referencedNodes;
  referencingNode->GetNodeReferences(role1.c_str(), referencedNodes);

  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 3 ||
      referencedNode != 0 ||
      referencedNodes.size() != 3 ||
      referencedNodes[0] != 0 ||
      referencedNodes[1] != 0 ||
      referencedNodes[2] != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(),  0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(),  1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(),  1), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(),  2) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(),  2), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 2) != 0
      )
    {
    std::cout << __LINE__ << ": RemoveNode failed" << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeReferences()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer<vtkMRMLNodeTestHelper1> referencedNode1 =
    vtkSmartPointer<vtkMRMLNodeTestHelper1>::New();
  scene->AddNode(referencedNode1);

  vtkSmartPointer<vtkMRMLNodeTestHelper1> referencingNode =
    vtkSmartPointer<vtkMRMLNodeTestHelper1>::New();
  scene->AddNode(referencingNode);

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode.GetPointer()));

  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Observing a referenced node not yet in the scene should add the reference in
  // the mrml scene, however GetReferencedNodes can't return the node because
  // it is not yet in the scene.
  vtkSmartPointer<vtkMRMLNodeTestHelper1> referencedNode2 =
    vtkSmartPointer<vtkMRMLNodeTestHelper1>::New();
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(),"vtkMRMLNodeTestHelper13");

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode.GetPointer()));
  if (referencedNodes->GetNumberOfItems() != 2 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  scene->AddNode(referencedNode2);
  //referencingNode->GetNthNodeReference(role1.c_str(), 1);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode));
  if (referencedNodes->GetNumberOfItems() != 3 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Test if the reference removal works
  vtkSmartPointer<vtkMRMLNodeTestHelper1> referencedNode3 =
    vtkSmartPointer<vtkMRMLNodeTestHelper1>::New();
  scene->AddNode(referencedNode3);
  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());
  referencingNode->RemoveNthNodeReferenceID(role1.c_str(), 2);

  referencedNodes.TakeReference(
    scene->GetReferencedNodes(referencingNode));
  if (referencedNodes->GetNumberOfItems() != 3 ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << referencedNodes->GetNumberOfItems() << std::endl;
    return false;
    }

  // Simulate scene deletion to see if it crashes or not.
  // When the  node is destroyed, it unreferences nodes. Make sure
  // it is ok for nodes already removed/deleted like referencedNode1.
  referencedNode1 = 0;
  referencingNode = 0;
  referencedNode2 = 0;
  referencedNode3 = 0;
  scene = 0;

  return true;
}

//----------------------------------------------------------------------------
bool TestReferenceModifiedEvent()
{
  std::string role1("refrole1");

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  vtkNew<vtkMRMLNodeCallback> spy;
  referencingNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(),  0);

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceRemovedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), "vtkMRMLNodeTestHelper14");

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  scene->AddNode(referencedNode3.GetPointer());
  // update the reference of the node
  vtkMRMLNode* referencedNode = referencingNode->GetNodeReference(role1.c_str());

  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) != 1 ||
      referencedNode != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}



//----------------------------------------------------------------------------
bool TestAddReferenceedNodeIDEventsWithNoScene()
{
  std::string role1("refrole1");

  // Make sure that the ReferenceAddedEvent is fired even when the
  // referenced node is observed when the referencing is not in the scene.
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode;
  scene->AddNode(referencedNode.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode->GetID());

  vtkNew<vtkMRMLNodeCallback> callback;
  referencingNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  scene->AddNode(referencingNode.GetPointer());

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 )
    {
    std::cerr << "ERROR line " << __LINE__ << ": " << std::endl
              << "vtkMRMLScene::AddNode(referencingNode) failed. "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of ReferenceAddedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent)
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestReferenceNodeNoObservers()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;

  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add empty referenced node with empty role
  returnNode = referencingNode->AddNodeReferenceID(0, 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 0 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddNodeReferenceID failed" << std::endl;
    return false;
    }

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode.GetPointer()));
  int referencedNodesCount = referencedNodes->GetNumberOfItems();

  /// Add referenced node ID
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode.GetPointer()));
  int newReferencedNodesCount = referencedNodes->GetNumberOfItems();

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode1->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode1.GetPointer() ||
      newReferencedNodesCount != (referencedNodesCount + 1) ||
      returnNode != referencedNode1.GetPointer())
    {
    std::cout << __LINE__ << ": AddNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add empty referenced node ID
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), 0);
  if (referencingNode->GetNumberOfNodeReferences(role1.c_str()) != 1 ||
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) != 0 ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != 0 || 
      returnNode != 0)
    {
    std::cout << __LINE__ << ": AddNodeReferenceID failed" << std::endl;
    return false;
    }


  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  returnNode = referencingNode->SetNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer() ||
      returnNode != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  returnNode = referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer() ||
      returnNode != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add different role

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  returnNode = referencingNode->SetNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode22->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 0) != referencedNode22.GetPointer() ||
      returnNode != referencedNode22.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 1), referencedNode23->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 1) != referencedNode23.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 0), referencedNode22->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 0) != referencedNode22.GetPointer() ||
      returnNode != referencedNode23.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }

  /// make sure it didnt change the first role references
  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 1) != referencedNode3.GetPointer() ||
      // make sure it didn't change the first referenced node ID
      referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), referencedNode2->GetID()) ||
      referencingNode->GetNthNodeReference(role1.c_str(), 0) != referencedNode2.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }

  /// change reference and check that it did
  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (referencingNode->GetNthNodeReferenceID(role2.c_str(), 1) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role2.c_str(), 1), referencedNode3->GetID()) ||
      referencingNode->GetNthNodeReference(role2.c_str(), 1) != referencedNode3.GetPointer() ||
      returnNode != referencedNode3.GetPointer())
    {
    std::cout << __LINE__ << ": SetNodeReferenceID failed" << std::endl;
    return false;
    }


  return true;
}
