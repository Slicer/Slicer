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

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//---------------------------------------------------------------------------
class vtkMRMLNodeTestHelper1 : public vtkMRMLNode
{
public:
  // Provide a concrete New.
  static vtkMRMLNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLNodeTestHelper1,vtkMRMLNode);

  NodeReferencesType& GetInternalReferencedNodes()
    {
    return this->NodeReferences;
    }

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLNodeTestHelper1::New();
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLNodeTestHelper1";
    }

  int GetNumberOfNodeReferenceObjects(const char* refrole)
    {
    if (!refrole)
      {
      return 0;
      }
    return this->NodeReferences[std::string(refrole)].size();
    }
};
vtkStandardNewMacro(vtkMRMLNodeTestHelper1);

//---------------------------------------------------------------------------
bool TestAttribute();

bool TestSetAndObserveNodeReferenceID();
bool TestAddRefrencedNodeIDWithNoScene();
bool TestAddDelayedReferenceNode();
bool TestRemoveReferencedNodeID();
bool TestRemoveReferencedNode();
bool TestRemoveReferencingNode();
bool TestNodeReferences();
bool TestReferenceModifiedEvent();
bool TestReferencesWithEvent();
bool TestAddReferencedNodeIDEventsWithNoScene();
bool TestSetNodeReferenceID();
bool TestSetNodeReferenceIDToZeroOrEmptyString();
bool TestNodeReferenceSerialization();
bool TestClearScene();


//---------------------------------------------------------------------------
int vtkMRMLNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLNodeTestHelper1> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLNodeTestHelper1, node1.GetPointer());

  bool res = true;
  res = TestAttribute();

  res = TestSetAndObserveNodeReferenceID() && res;
  res = TestAddRefrencedNodeIDWithNoScene() && res;
  res = TestAddDelayedReferenceNode() && res;
  res = TestRemoveReferencedNodeID() && res;
  res = TestRemoveReferencedNode() && res;
  res = TestRemoveReferencingNode() && res;
  res = TestNodeReferences() && res;
  res = TestReferenceModifiedEvent() && res;
  res = TestReferencesWithEvent() && res;
  res = TestAddReferencedNodeIDEventsWithNoScene() && res;
  res = TestSetNodeReferenceID() && res;
  res = TestSetNodeReferenceIDToZeroOrEmptyString() && res;
  res = TestNodeReferenceSerialization() && res;
  res = TestClearScene() && res;

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//----------------------------------------------------------------------------
bool CheckInt(int line, const std::string& function, int current, int expected)
{
  if(current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckInt failed"
              << "\n\tcurrent:" << current
              << "\n\texpected:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckString(int line, const std::string& function, const char* current, const char* expected)
{
  bool different = true;
  if (current == 0 || expected == 0)
    {
    different = !(current == 0 && expected == 0);
    }
  else if(strcmp(current, expected) == 0)
    {
    different = false;
    }
  if(different)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckString failed"
              << "\n\tcurrent:" << (current ? current : "<null>")
              << "\n\texpected:" << (expected ? expected : "<null>")
              << std::endl;
    return false;
    }
  return true;
}

}

//---------------------------------------------------------------------------
bool TestSetAttribute(int line, const char* attribute, const char* value,
                      const char* expectedValue,
                      size_t expectedSize = 1,
                      int expectedModified = 0,
                      int totalNumberOfEvent = -1)
{
  vtkNew<vtkMRMLNodeTestHelper1> node;
  node->SetAttribute("Attribute0", "Value0");

  vtkNew<vtkMRMLNodeCallback> spy;
  node->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  node->SetAttribute(attribute, value);

  if (!CheckString(line, "GetAttribute",
      node->GetAttribute(attribute), expectedValue))
    {
    return false;
    }
  if (!CheckInt(line, "GetAttributeNames",
      node->GetAttributeNames().size(), expectedSize))
    {
    return false;
    }
  if (totalNumberOfEvent == -1)
    {
    totalNumberOfEvent = expectedModified;
    }
  if (!CheckInt(line, "GetTotalNumberOfEvents",
      spy->GetTotalNumberOfEvents(), totalNumberOfEvent))
    {
    return false;
    }
  if (!CheckInt(line, "GetNumberOfEvents(vtkCommand::ModifiedEvent)",
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), expectedModified))
    {
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
    std::cerr << "vtkMRMLNode bad default attributes" << std::endl;
    return false;
    }

  // Test sets
  bool res = true;
  //                                                                 A: expectedSize
  //                                                                 B: expectedModified
  //                                                                 C: totalNumberOfEvent
  //                    (line    , attribute   , value   , expected, A, B, C
  res = TestSetAttribute(__LINE__, 0           , 0       , 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, 0           , ""      , 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, 0           , "Value1", 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , 0       , 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , ""      , 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, ""          , "Value1", 0       , 1, 0, 2) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", 0       , 0) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", ""      , ""      , 2, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute1", "Value1", "Value1", 2, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", 0       , 0       , 0, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", ""      , ""      , 1, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", "Value1", "Value1", 1, 1) && res;
  res = TestSetAttribute(__LINE__, "Attribute0", "Value0", "Value0", 1, 0) && res;
  return res;
}

namespace
{

//----------------------------------------------------------------------------
bool CheckNthNodeReferenceID(int line, const char* function,
                             vtkMRMLNode* referencingNode, const char* role, int n,
                             const char* expectedNodeReferenceID,
                             bool referencingNodeAddedToScene = true,
                             vtkMRMLNode* expectedNodeReference = 0)
{
  const char* currentNodeReferenceID = referencingNode->GetNthNodeReferenceID(role, n);
  bool different = true;
  if (currentNodeReferenceID == 0 || expectedNodeReferenceID == 0)
    {
    different = !(currentNodeReferenceID == 0 && expectedNodeReferenceID == 0);
    }
  else if(strcmp(currentNodeReferenceID, expectedNodeReferenceID) == 0)
    {
    different = false;
    }
  if (different)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNthNodeReferenceID failed"
              << "\n\tcurrent " << n << "th NodeReferenceID:"
              << (currentNodeReferenceID ? currentNodeReferenceID : "<null>")
              << "\n\texpected " << n << "th NodeReferenceID:"
              << (expectedNodeReferenceID ? expectedNodeReferenceID : "<null>")
              << std::endl;
    return false;
    }

  if (!referencingNodeAddedToScene)
    {
    expectedNodeReference = 0;
    }
  vtkMRMLNode* currentNodeReference = referencingNode->GetNthNodeReference(role, n);
  if (currentNodeReference!= expectedNodeReference)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNthNodeReferenceID failed"
              << "\n\tcurrent " << n << "th NodeReference:" << currentNodeReference
              << "\n\texpected " << n << "th NodeReference:" << expectedNodeReference
              << std::endl;
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
int GetReferencedNodeCount(vtkMRMLScene* scene, vtkMRMLNode * referencingNode)
{
  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(referencingNode));
  return referencedNodes->GetNumberOfItems();
}

//----------------------------------------------------------------------------
int CheckNumberOfNodeReferences(int line, const char* function,
                                const char* role, vtkMRMLNode * referencingNode, int expected)
{
  int current = referencingNode->GetNumberOfNodeReferences(role);
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckNumberOfNodeReferences failed"
              << "\n\tcurrent NumberOfNodeReferences:" << current
              << "\n\texpected NumberOfNodeReferences:" << expected
              << std::endl;
    return false;
    }
  vtkMRMLNodeTestHelper1 * referencingNodeTest = vtkMRMLNodeTestHelper1::SafeDownCast(referencingNode);
  if (referencingNodeTest)
    {
    current = referencingNodeTest->GetNumberOfNodeReferenceObjects(role);
    if (current != expected)
      {
      std::cerr << "Line " << line << " - " << function << " : CheckNumberOfNodeReferenceObjects failed"
                << "\n\tcurrent NumberOfNodeReferences:" << current
                << "\n\texpected NumberOfNodeReferences:" << expected
                << std::endl;
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
int CheckReferencedNodeCount(int line, const char* function,
                             vtkMRMLScene* scene, vtkMRMLNode * referencingNode, int expected)
{
  int current = GetReferencedNodeCount(scene, referencingNode);
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckReferencedNodeCount failed"
              << "\n\tcurrent ReferencedNodesCount:" << current
              << "\n\texpected ReferencedNodesCount:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckReturnNode(int line, const char* function, vtkMRMLNode* current, vtkMRMLNode* expected)
{
  if (current != expected)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckReturnNode failed"
              << "\n\tcurrent returnNode:" << current
              << "\n\texpected returnNode:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeReferences(int line, const char* function, vtkMRMLScene* scene,
                         vtkMRMLNode * referencingNode, const char* role, int n,
                         vtkMRMLNode* expectedNodeReference,
                         int expectedNumberOfNodeReferences,
                         int expectedReferencedNodesCount,
                         vtkMRMLNode* currentReturnNode)
{
  if (!CheckNthNodeReferenceID(line, function, referencingNode, role,
                               /* n = */ n,
                               /* expectedNodeReferenceID = */
                                 (expectedNodeReference ? expectedNodeReference->GetID() : 0),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ expectedNodeReference
                               ))
    {
    return false;
    }
  if (!CheckNumberOfNodeReferences(line, function, role, referencingNode,
                                   expectedNumberOfNodeReferences))
    {
    return false;
    }
  if (!CheckReferencedNodeCount(line, function, scene, referencingNode,
                                expectedReferencedNodesCount))
    {
    return false;
    }
  if (!CheckReturnNode(line, function, currentReturnNode, expectedNodeReference))
    {
    return false;
    }
  return true;
}
}

//----------------------------------------------------------------------------
bool TestSetAndObserveNodeReferenceID()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;
  int referencedNodesCount = -1;

  std::string role1("refrole1");
  std::string role2("refrole2");
  std::string role3("refrole3");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add empty referenced node with empty role
  returnNode = referencingNode->AddAndObserveNodeReferenceID(0, 0);
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), 0,
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), 0);
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode1.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add empty referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), 0);
  if (!CheckNodeReferences(__LINE__, "AddAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode2.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// Add different role
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode22.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode23.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode22->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode22.GetPointer()))
    {
    return false;
    }

  // make sure it didnt change the first role references
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }
  if (!CheckNumberOfNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }
  // make sure it didn't change the first referenced node ID associated with the first role
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// change reference and check that it did
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount - 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }
  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// (1) set first reference, (2) set first reference to null and (3) set second reference
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode31;
  scene->AddNode(referencedNode31.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 0, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 0, 0);

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ referencedNodesCount -1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), 1, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Set Nth reference to 0
  std::vector<int> referenceIndices;
  referenceIndices.push_back(20);
  referenceIndices.push_back(30);
  referenceIndices.push_back(31);
  referenceIndices.push_back(32);
  referenceIndices.push_back(21);
  referenceIndices.push_back(10);
  referenceIndices.push_back(3);
  referenceIndices.push_back(-1);
  for (std::vector<int>::iterator it = referenceIndices.begin();
       it != referenceIndices.end();
       ++it)
    {
    int nth = *it;
    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    returnNode = referencingNode->SetAndObserveNthNodeReferenceID(role3.c_str(), nth, 0);

    if (!CheckNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role3.c_str(),
                             /* n = */ nth,
                             /* expectedNodeReference = */ 0,
                             /* expectedNumberOfNodeReferences = */ 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount,
                             /* currentReturnNode = */ returnNode))
      {
      return false;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestAddRefrencedNodeIDWithNoScene()
{
  vtkNew<vtkMRMLScene> scene;

  std::string role1("refrole1");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add referenced node
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()))
    {
    return false;
    }

  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode3->GetID());

  if (!CheckNthNodeReferenceID(__LINE__, "AddAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "AddAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  // Finally, add the node into the scene so it can look for the referenced nodes
  // in the scene.
  scene->AddNode(referencingNode.GetPointer());

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::AddNode",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::AddNode",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
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

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ "vtkMRMLNodeTestHelper12",
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ 0))
    {
    return false;
    }

  scene->AddNode(referencedNode1.GetPointer());

  if (referencingNode->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNode->GetNthNodeReferenceID(role1.c_str(), 0), "vtkMRMLNodeTestHelper12") ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode != 0)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed" << std::endl;
    return false;
    }

  // Search for the node in the scene.
  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()))
    {
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

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, 0);

  if (!CheckNumberOfNodeReferences(__LINE__, "SetAndObserveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "SetAndObserveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  // add second role refs
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode2->GetID());
  referencingNode->AddAndObserveNodeReferenceID(role2.c_str(), referencedNode3->GetID());

  referencingNode->RemoveNthNodeReferenceID(role2.c_str(), 1);

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveNthNodeReferenceID",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveNthNodeReferenceID",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  referencingNode->RemoveAllNodeReferenceIDs(role1.c_str());

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ 0,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role2.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 1))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  referencingNode->RemoveAllNodeReferenceIDs(0);

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ 0,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  if (!CheckNumberOfNodeReferences(__LINE__, "RemoveAllNodeReferenceIDs",
                                   role2.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 0))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "RemoveAllNodeReferenceIDs",
                               referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ 0,
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencedNode()
{
  std::string role1("refrole1");

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

  if (!CheckNumberOfNodeReferences(__LINE__, "vtkMRMLScene::RemoveNode(referencedNode)",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::RemoveNode(referencedNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode1.GetPointer()
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::RemoveNode(referencedNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestRemoveReferencingNode()
{
  std::string role1("refrole1");

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

  if (!CheckNumberOfNodeReferences(__LINE__, "vtkMRMLScene::RemoveNode(referencingNode)",
                                   role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 3))
    {
    return false;
    }

  int expectedReferencedNodeVectorSize = 3;
  int currentReferencedNodeVectorSize = referencedNodes.size();
  if (currentReferencedNodeVectorSize != expectedReferencedNodeVectorSize)
    {
    std::cerr << "Line " << __LINE__ << " - " << "GetNodeReferences" << " failed"
              << "\n\tcurrent ReferencedNodeVectorSize:" << currentReferencedNodeVectorSize
              << "\n\texpected ReferencedNodeVectorSize:" << expectedReferencedNodeVectorSize
              << std::endl;
    }

  if (referencedNode != 0 ||
      referencedNodes[0] != 0 ||
      referencedNodes[1] != 0 ||
      referencedNodes[2] != 0
      )
    {
    std::cerr << __LINE__ << ": RemoveNode failed" << std::endl;
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode1->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  if (!CheckNthNodeReferenceID(__LINE__, "vtkMRMLScene::RemoveNode(referencingNode)",
                               referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 2,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ false,
                               /* expectedNodeReference = */ 0
                               ))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeReferences()
{
  std::string role1("refrole1");

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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
  int expectedNumberOfItems = 3;
  if (referencedNodes->GetNumberOfItems() != expectedNumberOfItems ||
      referencedNodes->GetItemAsObject(0) != referencingNode.GetPointer() ||
      referencedNodes->GetItemAsObject(1) != referencedNode1.GetPointer() ||
      referencedNodes->GetItemAsObject(2) != referencedNode2.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << " : SetAndObserveNodeReferenceID failed"
              << "\n\tcurrent NumberOfItems:" << referencedNodes->GetNumberOfItems()
              << "\n\texpected NumberOfItems:" << expectedNumberOfItems
              << std::endl;
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//----------------------------------------------------------------------------
bool TestReferencesWithEvent()
{
  std::string role1("refrole1");

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(777);
  events->InsertNextValue(888);

  vtkNew<vtkMRMLNodeCallback> spy;
  referencingNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID(), events.GetPointer());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) != 1 )
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode == 0 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(1) != 888)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }

  spy->ResetNumberOfEvents();

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());
  referencingNode->SetAndObserveNodeReferenceID(role1.c_str(), referencedNode2->GetID(), events.GetPointer());

  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      spy->GetNumberOfEvents(vtkMRMLNode::ReferenceModifiedEvent) != 1)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  if (referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode == 0 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(1) != 888)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
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
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed:" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();


  referencingNode->AddAndObserveNodeReferenceID(role1.c_str(), referencedNode1->GetID(), events.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode == 0 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(1) != 888)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }


  spy->ResetNumberOfEvents();

  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 0, referencedNode1->GetID(), events.GetPointer());

  if (referencingNode->GetInternalReferencedNodes()[role1][0]->ReferencedNode == 0 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetNumberOfTuples() != 2 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(0) != 777 ||
      referencingNode->GetInternalReferencedNodes()[role1][0]->Events->GetValue(1) != 888)
    {
    std::cerr << __LINE__ << ": SetAndObserveNodeReferenceID failed: events are incorrect" << std::endl
              << spy->GetTotalNumberOfEvents() << " "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << spy->GetNumberOfEvents(vtkMRMLNode::ReferenceAddedEvent) << std::endl;
    return false;
    }


  spy->ResetNumberOfEvents();

  return true;
}

//----------------------------------------------------------------------------
bool TestAddReferencedNodeIDEventsWithNoScene()
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
bool TestSetNodeReferenceID()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;
  int referencedNodesCount = -1;

  std::string role1("refrole1");
  std::string role2("refrole2");
  std::string role3("refrole3");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode1;
  scene->AddNode(referencedNode1.GetPointer());

  /// Add empty referenced node with empty role
  returnNode = referencingNode->AddNodeReferenceID(0, 0);
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), 0,
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add empty referenced node with a role
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), 0);
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode1->GetID());
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode1.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add empty referenced node ID
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), 0);
  if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Change referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode2;
  scene->AddNode(referencedNode2.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNodeReferenceID(role1.c_str(), referencedNode2->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode2.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode3;
  scene->AddNode(referencedNode3.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role1.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// Add different role
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode22.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode22.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Add referenced node
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode23;
  scene->AddNode(referencedNode23.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode23->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode23.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role2.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode22->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode22.GetPointer()))
    {
    return false;
    }

  // make sure it didnt change the first role references
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 1,
                               /* expectedNodeReferenceID = */ referencedNode3->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode3.GetPointer()))
    {
    return false;
    }
  if (!CheckNumberOfNodeReferences(__LINE__, "SetNthNodeReferenceID", role1.c_str(),
                                   referencingNode.GetPointer(),
                                   /* expectedNumberOfNodeReferences = */ 2))
    {
    return false;
    }
  // make sure it didn't change the first referenced node ID associated with the first role
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// change reference and check that it did
  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role2.c_str(), 1, referencedNode3->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role2.c_str(),
                           /* n = */ 1,
                           /* expectedNodeReference = */ referencedNode3.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 2,
                           /* expectedReferencedNodesCount = */ referencedNodesCount - 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }
  // make sure it didn't change the first referenced node ID
  if (!CheckNthNodeReferenceID(__LINE__, "SetNthNodeReferenceID", referencingNode.GetPointer(),
                               role1.c_str(),
                               /* n = */ 0,
                               /* expectedNodeReferenceID = */ referencedNode2->GetID(),
                               /* referencingNodeAddedToScene = */ true,
                               /* expectedNodeReference = */ referencedNode2.GetPointer()))
    {
    return false;
    }

  /// (1) set first reference, (2) set first reference to null and (3) set second reference
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode31;
  scene->AddNode(referencedNode31.GetPointer());

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 0, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 0, 0);

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ 0,
                           /* expectedNumberOfNodeReferences = */ 0,
                           /* expectedReferencedNodesCount = */ referencedNodesCount -1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
  returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), 1, referencedNode31->GetID());

  if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                           referencingNode.GetPointer(), role3.c_str(),
                           /* n = */ 0,
                           /* expectedNodeReference = */ referencedNode31.GetPointer(),
                           /* expectedNumberOfNodeReferences = */ 1,
                           /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                           /* currentReturnNode = */ returnNode))
    {
    return false;
    }

  /// Set Nth reference to 0
  std::vector<int> referenceIndices;
  referenceIndices.push_back(20);
  referenceIndices.push_back(30);
  referenceIndices.push_back(31);
  referenceIndices.push_back(32);
  referenceIndices.push_back(21);
  referenceIndices.push_back(10);
  referenceIndices.push_back(3);
  referenceIndices.push_back(-1);
  for (std::vector<int>::iterator it = referenceIndices.begin();
       it != referenceIndices.end();
       ++it)
    {
    int nth = *it;
    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    returnNode = referencingNode->SetNthNodeReferenceID(role3.c_str(), nth, 0);

    if (!CheckNodeReferences(__LINE__, "SetNthNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role3.c_str(),
                             /* n = */ nth,
                             /* expectedNodeReference = */ 0,
                             /* expectedNumberOfNodeReferences = */ 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount,
                             /* currentReturnNode = */ returnNode))
      {
      return false;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestSetNodeReferenceIDToZeroOrEmptyString()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLNode *returnNode = 0;
  int referencedNodesCount = -1;

  std::string role1("refrole1");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  // The following code adds 8 referenced nodes

  std::vector< vtkWeakPointer<vtkMRMLNodeTestHelper1> > referencingNodes;

  int referencingNodeCount = 8;
  for (int idx = 0; idx < referencingNodeCount; idx++)
    {
    vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
    scene->AddNode(referencingNode.GetPointer());
    referencingNodes.push_back(referencingNode.GetPointer());
    }

  for (int idx = 0; idx < referencingNodeCount; ++idx)
    {
    vtkMRMLNodeTestHelper1 * referencedNode = referencingNodes.at(idx);

    referencedNodesCount = GetReferencedNodeCount(scene.GetPointer(), referencingNode.GetPointer());
    returnNode = referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode->GetID());
    if (!CheckNodeReferences(__LINE__, "AddNodeReferenceID", scene.GetPointer(),
                             referencingNode.GetPointer(), role1.c_str(),
                             /* n = */ idx,
                             /* expectedNodeReference = */ referencedNode,
                             /* expectedNumberOfNodeReferences = */ idx + 1,
                             /* expectedReferencedNodesCount = */ referencedNodesCount + 1,
                             /* currentReturnNode = */ returnNode))
      {
      return false;
      }
    }

  int expectedReferencedNodesCount = referencedNodesCount;

  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  // The code below checks that setting a reference to either 0 or an empty string removes
  // the reference from the underlying vector.

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, "");
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetNthNodeReferenceID(role1.c_str(), 1, 0);
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, "");
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  expectedReferencedNodesCount = expectedReferencedNodesCount - 1;
  referencingNode->SetAndObserveNthNodeReferenceID(role1.c_str(), 1, 0);
  if (!CheckNumberOfNodeReferences(__LINE__, "TestSetNodeReferenceIDToZeroOrEmptyString", role1.c_str(),
                                   referencingNode.GetPointer(), expectedReferencedNodesCount))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeReferenceSerialization()
{
  std::string role1("refrole1");
  std::string role2("refrole2");

  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLNodeTestHelper1>::New());

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode11;
  scene->AddNode(referencedNode11.GetPointer());
  referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode11->GetID());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode21;
  vtkNew<vtkMRMLNodeTestHelper1> referencedNode22;
  scene->AddNode(referencedNode21.GetPointer());
  scene->AddNode(referencedNode22.GetPointer());
  referencingNode->AddNodeReferenceID(role2.c_str(), referencedNode21->GetID());
  referencingNode->AddNodeReferenceID(role2.c_str(), referencedNode22->GetID());

  std::stringstream ss;

  // Write scene to XML string
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string sceneXMLString = scene->GetSceneXMLString();

  vtkNew<vtkMRMLScene> scene2;
  scene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLNodeTestHelper1>::New());
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(sceneXMLString);
  scene2->Import();

  vtkMRMLNode* referencingNodeImported = NULL;
  if (scene2->GetNumberOfNodes() != 4 ||
      (referencingNodeImported = scene2->GetNodeByID(referencingNode->GetID())) == 0 ||
      referencingNodeImported->GetNumberOfNodeReferences(role1.c_str()) != 1 ||
      referencingNodeImported->GetNumberOfNodeReferences(role2.c_str()) != 2 ||
      referencingNodeImported->GetNthNodeReferenceID(role1.c_str(), 0) == 0 ||
      strcmp(referencingNodeImported->GetNthNodeReferenceID(role1.c_str(), 0),
             referencedNode11->GetID()) != 0 ||
      referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 0) == 0 ||
      strcmp(referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 0),
             referencedNode21->GetID()) != 0 ||
      referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 1) == 0 ||
      strcmp(referencingNodeImported->GetNthNodeReferenceID(role2.c_str(), 1),
             referencedNode22->GetID()) != 0)
    {
    std::cerr << __LINE__ << ": TestNodeReferenceSerialization failed" << std::endl
      << "Number of nodes: " << scene2->GetNumberOfNodes()
      << "Number of role1 references: "
        << referencingNodeImported->GetNumberOfNodeReferences(role1.c_str())
      << "Number of role2 references: "
        << referencingNodeImported->GetNumberOfNodeReferences(role2.c_str());
    return false;
    }
  return true;
}

namespace
{

//----------------------------------------------------------------------------
bool TestClearScene_CheckNumberOfEvents(const char* description,
                                        int removeSingleton,
                                        bool referencingNodeIsSingleton,
                                        bool referencedNodeIsSingleton,
                                        int expectedTotalNumberOfEventsForReferencingNode,
                                        int expectedNumberOfReferenceRemovedEventsForReferencingNode,
                                        int expectedTotalNumberOfEventsForReferencedNode,
                                        int expectedNumberOfReferenceRemovedEventsForReferencedNode)
{
  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLNodeTestHelper1>::New());

  std::string role1("refrole1");

  vtkNew<vtkMRMLNodeTestHelper1> referencingNode;
  if (referencingNodeIsSingleton)
    {
    std::string tag("ReferencingNodeSingleton-");
    tag += description;
    referencingNode->SetSingletonTag(tag.c_str());
    }
  scene->AddNode(referencingNode.GetPointer());

  vtkNew<vtkMRMLNodeTestHelper1> referencedNode;
  if (referencedNodeIsSingleton)
    {
    std::string tag("ReferencedNodeSingleton-");
    tag += description;
    referencedNode->SetSingletonTag(tag.c_str());
    }
  scene->AddNode(referencedNode.GetPointer());
  referencingNode->AddNodeReferenceID(role1.c_str(), referencedNode->GetID());

  vtkNew<vtkMRMLNodeCallback> referencingNodeSpy;
  vtkNew<vtkMRMLNodeCallback> referencedNodeSpy;

  referencingNode->AddObserver(vtkCommand::AnyEvent, referencingNodeSpy.GetPointer());
  referencedNode->AddObserver(vtkCommand::AnyEvent, referencedNodeSpy.GetPointer());

  if (!CheckInt(__LINE__,
                std::string("TestClearScene_AddNodes-TotalNumberOfEvents-ReferencingNode_") + description,
                referencingNodeSpy->GetTotalNumberOfEvents(), 0))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene_AddNodes-TotalNumberOfEvents-ReferencedNode_") + description,
                referencedNodeSpy->GetTotalNumberOfEvents(), 0))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  scene->Clear(removeSingleton);

  // ReferencingNode

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-TotalNumberOfEvents-for-ReferencingNode_") + description,
                referencingNodeSpy->GetTotalNumberOfEvents(),
                expectedTotalNumberOfEventsForReferencingNode))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-NumberOfReferenceRemovedEvents-for-ReferencingNode_") + description,
                referencingNodeSpy->GetNumberOfEvents(vtkMRMLNode::ReferenceRemovedEvent),
                expectedNumberOfReferenceRemovedEventsForReferencingNode))
    {
    referencingNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,

  // ReferencedNode

                std::string("TestClearScene-TotalNumberOfEvents-for-ReferencedNode_") + description,
                referencedNodeSpy->GetTotalNumberOfEvents(),
                expectedTotalNumberOfEventsForReferencedNode))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  if (!CheckInt(__LINE__,
                std::string("TestClearScene-NumberOfReferenceRemovedEvents-for-ReferencedNode_") + description,
                referencedNodeSpy->GetNumberOfEvents(vtkMRMLNode::ReferenceRemovedEvent),
                expectedNumberOfReferenceRemovedEventsForReferencedNode))
    {
    referencedNodeSpy->Print(std::cerr);
    return false;
    }

  referencingNodeSpy->ResetNumberOfEvents();
  referencedNodeSpy->ResetNumberOfEvents();

  return true;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
bool TestClearScene()
{

  // removeSingleton OFF
  int removeSingleton = 0;

  // "referencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "referencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }


  // "singletonReferencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 2,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 1,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "singletonReferencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 2,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 1,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // removeSingleton ON
  removeSingleton = 1;

  // "referencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "referencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[referencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ false,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }


  // "singletonReferencingNode" references "referencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-referencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ false,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  // "singletonReferencingNode" references "singletonReferencedNode"
  if (!TestClearScene_CheckNumberOfEvents(
        "[singletonReferencingNode-singletonReferencedNode]",
        /* removeSingleton = */ removeSingleton,
        /* referencingNodeIsSingleton = */ true,
        /* referencedNodeIsSingleton = */ true,
        /* expectedTotalNumberOfEventsForReferencingNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencingNode= */ 0,
        /* expectedTotalNumberOfEventsForReferencedNode= */ 0,
        /* expectedNumberOfReferenceRemovedEventsForReferencedNode= */ 0
        ))
    {
    return false;
    }

  return true;
}
