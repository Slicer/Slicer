/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

#include "vtkMRMLCoreTestingMacros.h"

//------------------------------------------------------------------------------
class vtkMRMLCustomNode
  : public vtkMRMLNode
{
public:
  static vtkMRMLCustomNode *New();
  vtkTypeMacro(vtkMRMLCustomNode, vtkMRMLNode);

  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "Custom"; }

  void Reset(vtkMRMLNode* defaultNode) override
    {
    ++this->ResetCount;
    this->vtkMRMLNode::Reset(defaultNode);
    }

  int ResetCount{0};

protected:
  vtkMRMLCustomNode() = default;
  ~vtkMRMLCustomNode() override = default;
  vtkMRMLCustomNode(const vtkMRMLCustomNode&);
  void operator=(const vtkMRMLCustomNode&);
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCustomNode);

//------------------------------------------------------------------------------
class vtkMRMLAnotherCustomNode
  : public vtkMRMLNode
{
public:
  static vtkMRMLAnotherCustomNode *New();
  vtkTypeMacro(vtkMRMLAnotherCustomNode, vtkMRMLNode);

  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "AnotherCustom"; }

protected:
  vtkMRMLAnotherCustomNode() = default;
  ~vtkMRMLAnotherCustomNode() override = default;
  vtkMRMLAnotherCustomNode(const vtkMRMLAnotherCustomNode&);
  void operator=(const vtkMRMLAnotherCustomNode&);
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnotherCustomNode);

//------------------------------------------------------------------------------
int vtkMRMLSceneTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene1;

  EXERCISE_BASIC_OBJECT_METHODS(scene1.GetPointer());

  std::cout << "GetNumberOfRegisteredNodeClasses() = ";
  std::cout << scene1->GetNumberOfRegisteredNodeClasses() << std::endl;

  TEST_SET_GET_STRING(scene1.GetPointer(), URL);
  TEST_SET_GET_STRING(scene1.GetPointer(), RootDirectory);

  //---------------------------------------------------------------------------
  // Test IsNodeClassRegistered
  //---------------------------------------------------------------------------

  {
  CHECK_BOOL(scene1->IsNodeClassRegistered(""), false);
  CHECK_BOOL(scene1->IsNodeClassRegistered("vtkMRMLScalarVolumeNode"), true);
  CHECK_BOOL(scene1->IsNodeClassRegistered("vtkMRMLInvalidNode"), false);
  }

  //---------------------------------------------------------------------------
  // Test ResetNodes
  //---------------------------------------------------------------------------

  {
    vtkNew<vtkMRMLCustomNode> node1;
    CHECK_INT(node1->ResetCount, 0);

    scene1->AddNode(node1.GetPointer());
    CHECK_INT(node1->ResetCount, 0);

    scene1->ResetNodes();

    CHECK_INT(node1->ResetCount, 1);

    scene1->Clear(/* removeSingletons= */ 1);
  }

  //---------------------------------------------------------------------------
  // Test GetFirstNode
  //---------------------------------------------------------------------------

  vtkMRMLNode* node1 =
    scene1->AddNode(vtkSmartPointer<vtkMRMLCustomNode>::New());
  node1->SetName("Node");
  node1->SetHideFromEditors(0);

  vtkMRMLNode* node2 =
    scene1->AddNode(vtkSmartPointer<vtkMRMLAnotherCustomNode>::New());
  node2->SetName("NodeWithSuffix");
  node2->SetHideFromEditors(0);

  vtkMRMLNode* node3 =
    scene1->AddNode(vtkSmartPointer<vtkMRMLAnotherCustomNode>::New());
  node3->SetName("Node");
  node3->SetHideFromEditors(1);

  vtkMRMLNode* node4 =
      scene1->AddNode(vtkSmartPointer<vtkMRMLCustomNode>::New());
  node4->SetName("NodeWithSuffix");
  node4->SetHideFromEditors(1);

  // Check if transform nodes have been added
  vtkSmartPointer<vtkCollection> transformNodes;
  transformNodes.TakeReference(scene1->GetNodesByClass("vtkMRMLCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = transformNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node1;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(transformNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node4;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(transformNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check if selection nodes have been added in the expected order
  vtkSmartPointer<vtkCollection> selectionNodes;
  selectionNodes.TakeReference(scene1->GetNodesByClass("vtkMRMLAnotherCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = selectionNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node2;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(selectionNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node3;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(selectionNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ "vtkMRMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkMRMLNode* expectedFirstNodeByClass = node2;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ "vtkMRMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName + [exactNameMatch=false] works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node2;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ nullptr,
          /* byHideFromEditors= */ nullptr,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ nullptr, /* byClass= */ nullptr, /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkMRMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkMRMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + [exactNameMatch=false] works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node4;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ "vtkMRMLCustomNode",
          /* byHideFromEditors= */ nullptr,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node4;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "NodeWithSuffix",
          /* byClass= */ "vtkMRMLCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node",
          /* byClass= */ "vtkMRMLAnotherCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Verify content of ReferencedIDChanges map
  {
    // Make sure IDs of nodes coming from private scenes are not stored in
    // the referenced ID changes map. It caused problems with AddArchetypeVoume,
    // because the volume (and related) nodes are tested in private scenes, they
    // have IDs when adding to the main scene, and the last node ID was stored
    // incorrectly as a changed ID
    vtkNew<vtkMRMLScene> privateScene;
    vtkMRMLCustomNode* nodeFromPrivateScene = vtkMRMLCustomNode::New();
    privateScene->AddNode(nodeFromPrivateScene);
    nodeFromPrivateScene->SetName("NodeFromPrivateScene");
    // Copy to std::string because pointer becomes invalid when adding to other scene
    std::string nodeInPrivateSceneID(nodeFromPrivateScene->GetID());
    privateScene->Clear(0);

    scene1->AddNode(nodeFromPrivateScene);
    const char* nodeAddedFromPrivateSceneID = nodeFromPrivateScene->GetID();
    const char* changedIDFromPrivateScene =
      scene1->GetChangedID(nodeInPrivateSceneID.c_str());
    if ( changedIDFromPrivateScene
      || nodeInPrivateSceneID.empty() || !nodeAddedFromPrivateSceneID ||
      !nodeInPrivateSceneID.compare(nodeAddedFromPrivateSceneID) )
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetChangedID()\n"
                << "  nodeFromPrivateSceneID: " << nodeInPrivateSceneID << "\n"
                << "  changedIDFromPrivateScene: " <<
                (changedIDFromPrivateScene?changedIDFromPrivateScene:"NULL") << "\n"
                << "  nodeAddedFromPrivateSceneID: " <<
                (nodeAddedFromPrivateSceneID?nodeAddedFromPrivateSceneID:"NULL")
                << std::endl;
      return EXIT_FAILURE;
      }

    // Check that IDs from imported scenes are indeed stored as changed if in
    // conflict with the main scene
    vtkNew<vtkMRMLScene> importedScene;
    vtkMRMLNode* importedNode =
      importedScene->AddNode(vtkSmartPointer<vtkMRMLCustomNode>::New());
    importedNode->SetName("ImportedNode");
    importedScene->SetSaveToXMLString(1);
    importedScene->Commit();
    std::string sceneXMLString = importedScene->GetSceneXMLString();
    const char* importedNodeID = importedNode->GetID();

    scene1->RegisterNodeClass(vtkSmartPointer<vtkMRMLCustomNode>::New());
    scene1->SetLoadFromXMLString(1);
    scene1->SetSceneXMLString(sceneXMLString);
    scene1->Import();
    const char* changedIDFromImportedScene =
      scene1->GetChangedID(importedNodeID);
    if ( !importedNodeID || !changedIDFromImportedScene ||
      !strcmp(changedIDFromImportedScene, importedNodeID) )
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetChangedID()\n"
                << "  importedNodeID: " <<
                (importedNodeID?importedNodeID:"NULL") << "\n"
                << "  changedIDFromImportedScene: " <<
                (changedIDFromImportedScene?changedIDFromImportedScene:"NULL")
                << std::endl;
      return EXIT_FAILURE;
      }

    // Needed to make sure the node is present after clearing the private scene
    nodeFromPrivateScene->Delete();
  }

  return EXIT_SUCCESS;
}
