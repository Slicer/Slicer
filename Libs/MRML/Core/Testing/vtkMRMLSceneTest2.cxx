/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkXMLDataParser.h>

// STD includes
#include <algorithm>
#include <iterator>
#include <set>
#include <sstream>

namespace
{

//---------------------------------------------------------------------------
std::vector<std::string> vector_diff(const std::vector<std::string>& v1,
                                     const std::vector<std::string>& v2)
{
  std::set<std::string> s_v1(v1.begin(), v1.end());
  std::set<std::string> s_v2(v2.begin(), v2.end());
  std::vector<std::string> result;

  std::set_difference(s_v1.begin(), s_v1.end(),
                      s_v2.begin(), s_v2.end(),
                      std::back_inserter(result));
  return result;
}

//---------------------------------------------------------------------------
class vtkMRMLSceneCallback : public vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback
{
public:
  static vtkMRMLSceneCallback *New() {return new vtkMRMLSceneCallback;}

  int NumberOfSingletonNodes;

  // List of node that should be updated when NodeAddedEvent is caught
  std::vector<std::string> NodeAddedClassNames;

  void ResetNumberOfEvents() override
    {
    vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback::ResetNumberOfEvents();
    this->NumberOfSingletonNodes = 0;
    this->NodeAddedClassNames.clear();
    }

  void Execute(vtkObject* caller, unsigned long eid, void *calldata) override
    {
    vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback::Execute(caller, eid, calldata);

    // Let's return if an error already occurred
    if (this->CheckStatus() == EXIT_FAILURE)
      {
      return;
      }

    vtkMRMLScene* callerScene = vtkMRMLScene::SafeDownCast(caller);

    if (eid == vtkMRMLScene::NodeAboutToBeAddedEvent)
      {
      vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
      if (!node)
        {
        SetErrorString(__LINE__, "mrmlEventCallback - NodeAboutToBeAddedEvent - node is NULL");
        return;
        }
      }
    else if (eid == vtkMRMLScene::NodeAddedEvent)
      {
      vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
      if (!node)
        {
        SetErrorString(__LINE__, "mrmlEventCallback - NodeAddedEvent - node is NULL");
        return;
        }
      if (node->GetSingletonTag())
        {
        ++this->NumberOfSingletonNodes;
        }

      this->NodeAddedClassNames.emplace_back(node->GetClassName());
      }
    else if (eid == vtkMRMLScene::NodeAboutToBeRemovedEvent)
      {
      vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
      if (!node)
        {
        SetErrorString(__LINE__, "mrmlEventCallback - NodeAboutToBeRemovedEvent - node is NULL");
        return;
        }
      }
    else if (eid == vtkMRMLScene::NodeRemovedEvent)
      {
      vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
      if (!node)
        {
        SetErrorString(__LINE__, "mrmlEventCallback - NodeRemovedEvent - node is NULL");
        return;
        }
      }
    else if (eid == vtkMRMLScene::StartCloseEvent)
      {
      if (callerScene->IsImporting())
        {
        SetErrorString(__LINE__, "StartCloseEvent - IsImporting is expected to be 0");
        return;
        }
      if (!callerScene->IsClosing())
        {
        SetErrorString(__LINE__, "StartCloseEvent - IsClosing is expected to be 1");
        return;
        }
      if (!callerScene->IsBatchProcessing())
        {
        SetErrorString(__LINE__, "StartCloseEvent - IsUpdating is expected to be 1");
        return;
        }
      }
    else if (eid == vtkMRMLScene::EndCloseEvent)
      {
      if (callerScene->IsImporting())
        {
        SetErrorString(__LINE__, "EndCloseEvent - IsImporting is expected to be 0");
        return;
        }
      if (callerScene->IsClosing())
        {
        SetErrorString(__LINE__, "EndCloseEvent - IsClosing is expected to be 0");
        return;
        }
      }
    else if (eid == vtkMRMLScene::StartImportEvent)
      {
      if (!callerScene->IsImporting())
        {
        SetErrorString(__LINE__, "StartImport - ImportState is expected");
        return;
        }
      if (!callerScene->IsBatchProcessing())
        {
        SetErrorString(__LINE__, "StartImport - IsBatchProcessing is expected");
        return;
        }
      }
    else if (eid == vtkMRMLScene::EndImportEvent)
      {
      if (callerScene->IsImporting())
        {
        SetErrorString(__LINE__, "EndImportEvent - ImportState is not expected");
        return;
        }
      }
    }

protected:
  vtkMRMLSceneCallback()
    {
    this->NumberOfSingletonNodes = 0;
    }
  ~vtkMRMLSceneCallback() override  = default;

}; // class vtkMRMLSceneCallback

}; //namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneTest2(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  // Instantiate scene
  vtkSmartPointer<vtkMRMLScene>  scene = vtkSmartPointer<vtkMRMLScene>::New(); // vtkSmartPointer instead of vtkNew to allow SetPointer
  EXERCISE_BASIC_OBJECT_METHODS(scene.GetPointer());
  CHECK_INT(scene->GetNumberOfNodes(), 0);

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene);

  // Configure mrml event observer
  vtkNew<vtkMRMLSceneCallback> callback;
  scene->AddObserver(vtkMRMLScene::NodeAboutToBeAddedEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::NodeAddedEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::NodeAboutToBeRemovedEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::StartCloseEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::EndCloseEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::StartImportEvent, callback.GetPointer());
  scene->AddObserver(vtkMRMLScene::EndImportEvent, callback.GetPointer());


  //---------------------------------------------------------------------------
  // Make sure IsClosing, IsImporting, IsBatchProcessing default values are correct
  //---------------------------------------------------------------------------
  CHECK_BOOL(scene->IsClosing(), false);
  CHECK_BOOL(scene->IsImporting(), false);
  CHECK_BOOL(scene->IsBatchProcessing(), false);

  //---------------------------------------------------------------------------
  // Make sure SetIsImporting invoke events properly
  //---------------------------------------------------------------------------
  callback->ResetNumberOfEvents();
  // it is invalid operation to end a state before starting it, should report error
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->EndState(vtkMRMLScene::ImportState);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::StartImportEvent), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::EndImportEvent), 0);

  callback->ResetNumberOfEvents();
  scene->StartState(vtkMRMLScene::ImportState);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::StartImportEvent), 1);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::EndImportEvent), 0);

  callback->ResetNumberOfEvents();
  scene->EndState(vtkMRMLScene::ImportState);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::StartImportEvent), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::EndImportEvent), 1);

  //---------------------------------------------------------------------------
  // Extract number of nodes
  //---------------------------------------------------------------------------
  vtkNew<vtkXMLDataParser> xmlParser;
  xmlParser->SetFileName(argv[1]);
  CHECK_BOOL(xmlParser->Parse()!=0, true);
  int expectedNumberOfNode = xmlParser->GetRootElement()->GetNumberOfNestedElements();
  CHECK_BOOL(expectedNumberOfNode>0, true);

  // Loop though all exepcted node and populate expectedNodeAddedNames vector
  // Note that node that can't be instantiated using CreateNodeByClass are not expected
  std::vector<std::string> expectedNodeAddedClassNames; // List of node that should be added to the scene
  for(int i=0; i < xmlParser->GetRootElement()->GetNumberOfNestedElements(); ++i)
    {
    std::string className = "vtkMRML";
    className += xmlParser->GetRootElement()->GetNestedElement(i)->GetName();
    // Append 'Node' prefix only if required
    if (className.find("Node") != className.size() - 4)
      {
      className += "Node";
      }
    vtkSmartPointer<vtkMRMLNode> nodeSmartPointer;
    nodeSmartPointer.TakeReference(scene->CreateNodeByClass(className.c_str()));
    if (!nodeSmartPointer)
      {
      std::cout << "className:" << className << std::endl;
      --expectedNumberOfNode;
      }
    else
      {
      expectedNodeAddedClassNames.push_back(className);
      }
    }

  //---------------------------------------------------------------------------
  // Check if the correct number of Events are sent - Also Keep track # of Singleton node
  //---------------------------------------------------------------------------
  callback->ResetNumberOfEvents();

  // Load the scene
  scene->SetURL( argv[1] );

  scene->Connect();

  CHECK_EXIT_SUCCESS(callback->CheckStatus());

  std::vector<std::string> unexpectedAddedNodeNames =
      vector_diff(expectedNodeAddedClassNames, callback->NodeAddedClassNames);
  if (!unexpectedAddedNodeNames.empty())
    {
    std::cerr << "line " << __LINE__ << " - unexpectedAddedNodeNames: ";
    for(size_t i = 0; i < unexpectedAddedNodeNames.size(); ++i)
      {
      std::cerr << unexpectedAddedNodeNames[i] << " ";
      }
    std::cerr << std::endl;
    return EXIT_FAILURE;
    }

  // If additional nodes have been instantiated, let's update expectedNumberOfNode
  if (static_cast<int>(callback->NodeAddedClassNames.size()) > expectedNumberOfNode)
    {
    expectedNumberOfNode = static_cast<int>(callback->NodeAddedClassNames.size());
    }
  CHECK_INT(scene->GetNumberOfNodes(), expectedNumberOfNode);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAddedEvent), expectedNumberOfNode);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAboutToBeAddedEvent), expectedNumberOfNode);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeRemovedEvent), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAboutToBeRemovedEvent), 0);

  // Since Connect() means Clean (or close) first then import,
  // let's check if one event of each has been triggered.
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::StartCloseEvent), 1);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::EndCloseEvent), 1);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::StartImportEvent), 1);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::EndImportEvent), 1);

  CHECK_BOOL(scene->IsBatchProcessing(), false);

  int currentNumberOfSingletonTag = callback->NumberOfSingletonNodes;

  //---------------------------------------------------------------------------
  // Clear all node except Singleton ones
  //---------------------------------------------------------------------------
  callback->ResetNumberOfEvents();

  // Clear scene expect singletons
  scene->Clear(0);
  CHECK_EXIT_SUCCESS(callback->CheckStatus());

  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeRemovedEvent) + currentNumberOfSingletonTag, expectedNumberOfNode);
  CHECK_INT(scene->GetNumberOfNodes(), currentNumberOfSingletonTag);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAboutToBeRemovedEvent), callback->GetNumberOfEvents(vtkMRMLScene::NodeRemovedEvent));
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAboutToBeAddedEvent), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAddedEvent), 0);

  //---------------------------------------------------------------------------
  // Clear all node including Singleton ones
  //---------------------------------------------------------------------------
  callback->ResetNumberOfEvents();

  // Clear scene including singletons - Note that by now the scene should contain only singletons
  scene->Clear(1);
  CHECK_EXIT_SUCCESS(callback->CheckStatus());
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeAboutToBeRemovedEvent), currentNumberOfSingletonTag);
  CHECK_INT(scene->GetNumberOfNodes(), 0);

#if 0
  //---------------------------------------------------------------------------
  // Print content
  //---------------------------------------------------------------------------
  vtkCollection * collection = scene->GetNodes();
  std::cout << "Collection GetNumberOfItems() = ";
  std::cout << collection->GetNumberOfItems() << std::endl;

  std::cout << "List of Node Names in this Scene" << std::endl;
  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node = nullptr;
  vtkCollection *nodes = scene->GetNodes();
  for (nodes->InitTraversal(it);
    (node = vtkMRMLNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
    {
    std::cout << " " << node->GetName() << std::endl;
    }
#endif

  //---------------------------------------------------------------------------
  // Check if the destructor remove all nodes by checking number of NodeRemovedEvent sent
  //---------------------------------------------------------------------------
  // Expected number of nodes that should removed
  int numberOfNodes = scene->GetNumberOfNodes();
  callback->ResetNumberOfEvents();
  scene = nullptr;
  CHECK_EXIT_SUCCESS(callback->CheckStatus());
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLScene::NodeRemovedEvent), numberOfNodes);

  std::cout << "Test completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
