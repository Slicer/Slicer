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
#include <vtkCallbackCommand.h>
#include <vtkXMLDataParser.h>

// STD includes
#include <algorithm>
#include <set>
#include <sstream>

// Convenient macro
#define SCENE_CHECK_NUMBER_OF_EVENT(MSG, VARNAME, CURRENTCOUNT, OP, EXPECTEDCOUNT) \
  if (EXPECTEDCOUNT OP CURRENTCOUNT)                                               \
    {                                                                              \
    std::cerr << "line " << __LINE__                                               \
    << " - " << #MSG << std::endl;                                                 \
    std::cerr << "Expected " << #VARNAME << ":" << EXPECTEDCOUNT << std::endl;     \
    std::cerr << "Current " << #VARNAME << ":" << CURRENTCOUNT << std::endl;       \
    return EXIT_FAILURE;                                                           \
    }

namespace
{
// List of node that should be added to the scene
std::vector<std::string> expectedNodeAddedClassNames;

// List of node that should be updated when NodeAddedEvent is catched
std::vector<std::string> nodeAddedClassNames;

vtkMRMLScene*  scene = 0;
std::string    errorString;
int            numberOfAboutToBeAddedNodes = 0;
int            numberOfAddedNodes = 0;
int            numberOfAboutToBeRemovedNodes = 0;
int            numberOfRemovedNodes = 0;
int            numberOfSingletonNodes = 0;
int            numberOfSceneAboutToBeClosedEvents = 0;
int            numberOfSceneClosedEvents = 0;
int            numberOfSceneAboutToBeImportedEvents = 0;
int            numberOfSceneImportedEvents = 0;
bool           calledConnectMethod = false;

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
void resetNumberOfEventsVariables()
{
  nodeAddedClassNames.clear();
  numberOfAboutToBeAddedNodes = 0;
  numberOfAddedNodes = 0;
  numberOfAboutToBeRemovedNodes = 0;
  numberOfRemovedNodes = 0;
  numberOfSingletonNodes = 0;
  numberOfSceneAboutToBeClosedEvents = 0;
  numberOfSceneClosedEvents = 0;
  numberOfSceneAboutToBeImportedEvents = 0;
  numberOfSceneImportedEvents = 0;
  calledConnectMethod = false;
}

//---------------------------------------------------------------------------
void setErrorString(int line, const std::string& msg)
{
  std::stringstream ss;
  ss << "line " << line << " - " << msg;
  errorString = ss.str();
}

//---------------------------------------------------------------------------
void mrmlEventCallback(vtkObject *vtkcaller, unsigned long eid,
                         void *vtkNotUsed(clientdata), void *calldata)
{

  // Let's return if an error already occured
  if (errorString.size() > 0)
    {
    return;
    }
  if (scene != vtkMRMLScene::SafeDownCast(vtkcaller))
    {
    setErrorString(__LINE__, "mrmlEventCallback - scene != vtkMRMLScene::SafeDownCast(vtkcaller)");
    return;
    }

  if (eid == vtkMRMLScene::NodeAboutToBeAddedEvent)
    {
    vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
    if (!node)
      {
      setErrorString(__LINE__, "mrmlEventCallback - NodeAboutToBeAddedEvent - node is NULL");
      return;
      }
    ++numberOfAboutToBeAddedNodes;
    }
  else if (eid == vtkMRMLScene::NodeAddedEvent)
    {
    vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
    if (!node)
      {
      setErrorString(__LINE__, "mrmlEventCallback - NodeAddedEvent - node is NULL");
      return;
      }
    ++numberOfAddedNodes;
    if (node->GetSingletonTag())
      {
      ++numberOfSingletonNodes;
      }

    nodeAddedClassNames.push_back(node->GetClassName());
    }
  else if (eid == vtkMRMLScene::NodeAboutToBeRemovedEvent)
    {
    vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
    if (!node)
      {
      setErrorString(__LINE__, "mrmlEventCallback - NodeAboutToBeRemovedEvent - node is NULL");
      return;
      }
    ++numberOfAboutToBeRemovedNodes;
    }
  else if (eid == vtkMRMLScene::NodeRemovedEvent)
    {
    vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(calldata);
    if (!node)
      {
      setErrorString(__LINE__, "mrmlEventCallback - NodeRemovedEvent - node is NULL");
      return;
      }
    ++numberOfRemovedNodes;
    }
  else if (eid == vtkMRMLScene::StartCloseEvent)
    {
    if (scene->IsImporting())
      {
      setErrorString(__LINE__, "StartCloseEvent - IsImporting is expected to be 0");
      return;
      }
    if (scene->IsClosing())
      {
      setErrorString(__LINE__, "StartCloseEvent - IsClosing is expected to be 1");
      return;
      }
    if (scene->IsBatchProcessing())
      {
      setErrorString(__LINE__, "StartCloseEvent - IsUpdating is expected to be 1");
      return;
      }
    ++numberOfSceneAboutToBeClosedEvents;
    }
  else if (eid == vtkMRMLScene::EndCloseEvent)
    {
    if (scene->IsImporting())
      {
      setErrorString(__LINE__, "EndCloseEvent - IsImporting is expected to be 0");
      return;
      }
    if (scene->IsClosing())
      {
      setErrorString(__LINE__, errorString = "EndCloseEvent - IsClosing is expected to be 0");
      return;
      }
    ++numberOfSceneClosedEvents;
    }
  else if (eid == vtkMRMLScene::StartImportEvent)
    {
    if (!scene->IsImporting())
      {
      setErrorString(__LINE__, "StartImport - ImportState is expected");
      return;
      }
    if (!scene->IsBatchProcessing())
      {
      setErrorString(__LINE__, "StartImport - IsBatchProcessing is expected");
      return;
      }
    ++numberOfSceneAboutToBeImportedEvents;
    }
  else if (eid == vtkMRMLScene::EndImportEvent)
    {
    if (scene->IsImporting())
      {
      setErrorString(__LINE__, "EndImportEvent - ImportState is not expected");
      return;
      }
    if (scene->IsBatchProcessing())
      {
      setErrorString(__LINE__, "EndImportEvent - IsBatchProcessing is not expected");
      return;
      }
    ++numberOfSceneImportedEvents;
    }
}

//---------------------------------------------------------------------------
bool checkErrorString()
{
  if (errorString.size() > 0)
    {
    std::cerr << errorString << std::endl;
    return false;
    }
  return true;
}

}

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

  // Instanciate scene
  scene = vtkMRMLScene::New();
  EXERCISE_BASIC_OBJECT_METHODS( scene );

  if (scene->GetNumberOfNodes() != 0)
    {
    std::cerr << "line " << __LINE__ << " - Problem with vtkMRMLScene::GetNumberOfNodes() - "
        "Scene should contains 0 nodes" << std::endl;
    return EXIT_FAILURE;
    }

  // Configure mrml event callback
  vtkNew<vtkCallbackCommand> callback;
  callback->SetCallback(mrmlEventCallback.GetPointer());
  scene->AddObserver(vtkMRMLScene::NodeAboutToBeAddedEvent, callback);
  scene->AddObserver(vtkMRMLScene::NodeAddedEvent, callback);
  scene->AddObserver(vtkMRMLScene::NodeAboutToBeRemovedEvent, callback);
  scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, callback);
  scene->AddObserver(vtkMRMLScene::StartCloseEvent, callback);
  scene->AddObserver(vtkMRMLScene::EndCloseEvent, callback);
  scene->AddObserver(vtkMRMLScene::StartImportEvent, callback);
  scene->AddObserver(vtkMRMLScene::EndImportEvent, callback);

  //---------------------------------------------------------------------------
  // Make sure IsClosing, IsImporting, IsBatchProcessing default values are correct
  //---------------------------------------------------------------------------
  if (scene->IsClosing())
    {
    std::cerr << "line " << __LINE__ <<
        " - Problem with IsClosing - 0 expected" << std::endl;
    return EXIT_FAILURE;
    }
  if (scene->IsImporting())
    {
    std::cerr << "line " << __LINE__ <<
        " - Problem with GetStates - 0 expected" << std::endl;
    return EXIT_FAILURE;
    }
  if (scene->IsBatchProcessing())
    {
    std::cerr << "line " << __LINE__ <<
        " - Problem with IsBatchProcessing - 0 expected" << std::endl;
    return EXIT_FAILURE;
    }

  //---------------------------------------------------------------------------
  // Make sure SetIsImporting invoke events properly
  //---------------------------------------------------------------------------
  resetNumberOfEventsVariables();
  scene->SetIsImporting(false);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneAboutToBeImportedEvents",
                              numberOfSceneAboutToBeImportedEvents, !=, 0);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneImportedEvents",
                              numberOfSceneImportedEvents, !=, 0);

  resetNumberOfEventsVariables();
  scene->SetIsImporting(true);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneAboutToBeImportedEvents",
                              numberOfSceneAboutToBeImportedEvents, !=, 1);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneImportedEvents",
                              numberOfSceneImportedEvents, !=, 0);

  resetNumberOfEventsVariables();
  scene->SetIsImporting(false);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneAboutToBeImportedEvents",
                              numberOfSceneAboutToBeImportedEvents, !=, 0);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::SetIsImporting()",
                              "SceneImportedEvents",
                              numberOfSceneImportedEvents, !=, 1);

  //---------------------------------------------------------------------------
  // Extract number of nodes
  //---------------------------------------------------------------------------
  vtkNew<vtkXMLDataParser> xmlParser;
  xmlParser->SetFileName(argv[1]);
  if (!xmlParser->Parse())
    {
    std::cerr << "line " << __LINE__
        << " - Failed to extract number of node using vtkXMLDataParser" <<std::endl;
    return EXIT_FAILURE;
    }
  int expectedNumberOfNode = xmlParser->GetRootElement()->GetNumberOfNestedElements();
  if (expectedNumberOfNode <= 0)
    {
    std::cerr << "line " << __LINE__
        << " - " << argv[1] << " file do NOT contains any node !" <<std::endl;
    return EXIT_FAILURE;
    }
  // Loop though all exepcted node and populate expectedNodeAddedNames vector
  // Note that node that can't be instantiated using CreateNodeByClass are not expected
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
  resetNumberOfEventsVariables();

  // Load the scene
  scene->SetURL( argv[1] );

  calledConnectMethod = true;
  scene->Connect();
  calledConnectMethod = false;

  if (!checkErrorString())
    {
    return EXIT_FAILURE;
    }

  std::vector<std::string> unexpectedAddedNodeNames =
      vector_diff(expectedNodeAddedClassNames, nodeAddedClassNames);
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
  if (static_cast<int>(nodeAddedClassNames.size()) > expectedNumberOfNode)
    {
    expectedNumberOfNode = static_cast<int>(nodeAddedClassNames.size());
    }

  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::Connect()", "NumberOfNode",
                              scene->GetNumberOfNodes(), !=, expectedNumberOfNode);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeAddedEvent",
                              "numberOfAddedNodes", numberOfAddedNodes, !=, expectedNumberOfNode);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeAboutToAddedEvent",
                              "numberOfAboutToBeAddedNodes", numberOfAboutToBeAddedNodes, !=,
                              numberOfAddedNodes);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeRemovedEvent",
                              "numberOfRemovedNodes", numberOfRemovedNodes, >, 0);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeAboutToBeRemovedEvent",
                              "numberOfAboutToBeRemovedNodes", numberOfAboutToBeRemovedNodes, >, 0);

  // Since Connect() means Clean (or close) first then import, let's check if one event of each
  //  as been triggered.
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with SceneAboutToBeClosedEvent",
                              "numberOfSceneAboutToBeClosedEvents",
                              numberOfSceneAboutToBeClosedEvents, !=, 1);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with SceneClosedEvent",
                              "numberOfSceneClosedEvents",
                              numberOfSceneClosedEvents, !=, 1);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with SceneAboutToBeImportedEvent",
                              "numberOfSceneAboutToBeImportedEvents",
                              numberOfSceneAboutToBeImportedEvents, !=, 1);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with SceneImportedEvent",
                              "numberOfSceneImportedEvents",
                              numberOfSceneImportedEvents, !=, 1);

  if (scene->GetIsUpdating())
    {
    std::cout << "Problem with vtkMRMLScene::Connect() - IsUpdating should return 0" << std::endl;
    return EXIT_FAILURE;
    }

  int currentNumberOfSingletonTag = numberOfSingletonNodes;

  //---------------------------------------------------------------------------
  // Clear all node except Singleton ones
  //---------------------------------------------------------------------------
  resetNumberOfEventsVariables();

  // Clear scene expect singletons
  scene->Clear(0);
  if (!checkErrorString())
    {
    return EXIT_FAILURE;
    }

  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::Clear(0)",
                              "NumberOfNode",
                              numberOfRemovedNodes + currentNumberOfSingletonTag, !=,
                              expectedNumberOfNode);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::Clear(0)", "NumberOfNode",
                              scene->GetNumberOfNodes(), !=, currentNumberOfSingletonTag);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::Clear(0)",
                              "numberOfRemovedNodes",
                              numberOfAboutToBeRemovedNodes, !=, numberOfRemovedNodes);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeAddedEvent",
                              "numberOfAddedNodes", numberOfAddedNodes, >, 0);
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with NodeAboutToAddedEvent",
                              "numberOfAboutToBeAddedNodes", numberOfAboutToBeAddedNodes, >, 0);

  //---------------------------------------------------------------------------
  // Clear all node including Singleton ones
  //---------------------------------------------------------------------------
  resetNumberOfEventsVariables();

  // Clear scene including singletons - Note that the scene should contain only singletons
  scene->Clear(1);
  if (!checkErrorString())
    {
    return EXIT_FAILURE;
    }
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Connect() - Problem with vtkMRMLScene::Clear(1)",
                              "numberOfRemovedNodes",
                              numberOfRemovedNodes, !=, currentNumberOfSingletonTag);
  SCENE_CHECK_NUMBER_OF_EVENT("Problem with vtkMRMLScene::Clear(1) - Scene should contains 0 nodes",
                              "NumberOfNodes",
                              scene->GetNumberOfNodes(), !=, 0);

#if 0
  //---------------------------------------------------------------------------
  // Print content
  //---------------------------------------------------------------------------
  vtkCollection * collection = scene->GetNodes();
  std::cout << "Collection GetNumberOfItems() = ";
  std::cout << collection->GetNumberOfItems() << std::endl;

  std::cout << "List of Node Names in this Scene" << std::endl;
  scene->InitTraversal();
  vtkMRMLNode * nodePtr = scene->GetNextNode();
  while( nodePtr != 0 )
    {
    std::cout << " " << nodePtr->GetName() << std::endl;
    nodePtr = scene->GetNextNode();
    }
#endif

  //---------------------------------------------------------------------------
  // Check if the destructor remove all nodes by checking number of NodeRemovedEvent sent
  //---------------------------------------------------------------------------
  // Expected number of nodes that should removed
  int numberOfNodes = scene->GetNumberOfNodes();

  resetNumberOfEventsVariables();

  // Make sure scene destructor work properly
  scene->Delete();
  if (!checkErrorString())
    {
    return EXIT_FAILURE;
    }
  SCENE_CHECK_NUMBER_OF_EVENT("vtkMRMLScene::Delete() has not fired enough NodeRemovedEvent",
                              "NumberOfNodes",
                              numberOfRemovedNodes, !=, numberOfNodes);

  return EXIT_SUCCESS;
}
