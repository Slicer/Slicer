
// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTestThreeDViewDisplayableManager.h>
#include <vtkMRMLTestSliceViewDisplayableManager.h>
#include <vtkMRMLTestCustomDisplayableManager.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTesting.h>

// STD includes

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerFactoriesTest1(int argc, char* argv[])
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc, const_cast<const char **>(argv));

  vtkMRMLThreeDViewDisplayableManagerFactory * threeDViewFactory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  if (!threeDViewFactory)
    {
    std::cerr << "Line " << __LINE__
      << " - Problem with vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance() method"
      << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLSliceViewDisplayableManagerFactory * slicerViewFactory = vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();
  if (!slicerViewFactory)
    {
    std::cerr << "Line " << __LINE__
      << " - Problem with vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance() method"
      << std::endl;
    return EXIT_FAILURE;
    }

  // MRML Scene
  vtkNew<vtkMRMLScene> scene;

  // MRML Application logic (Add Interaction and Selection node)
  vtkNew<vtkMRMLApplicationLogic> mrmlAppLogic;
  mrmlAppLogic->SetMRMLScene(scene.GetPointer());

  int currentCount = threeDViewFactory->GetRegisteredDisplayableManagerCount();
  if (currentCount != 0)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetRegisteredDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 0" << std::endl;
    return EXIT_FAILURE;
    }

  if (threeDViewFactory->GetRegisteredDisplayableManagerName(-1) != "" ||
      threeDViewFactory->GetRegisteredDisplayableManagerName(0) != "")
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetNthRegisteredDisplayableManagerName() method"
        << std::endl;
    }

  // Register displayable manager
  threeDViewFactory->RegisterDisplayableManager("vtkMRMLTestThreeDViewDisplayableManager");
  threeDViewFactory->RegisterDisplayableManager("vtkMRMLTestCustomDisplayableManager");

  slicerViewFactory->RegisterDisplayableManager("vtkMRMLTestSliceViewDisplayableManager");
  slicerViewFactory->RegisterDisplayableManager("vtkMRMLTestCustomDisplayableManager");

  if (threeDViewFactory->GetRegisteredDisplayableManagerName(0) != "vtkMRMLTestThreeDViewDisplayableManager" ||
      threeDViewFactory->GetRegisteredDisplayableManagerName(1) != "vtkMRMLTestCustomDisplayableManager")
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->GetNthRegisteredDisplayableManagerName() method"
        << std::endl;
    }

  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  rw->AddRenderer(rr.GetPointer());
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // ThreeD - Instantiate displayable managers
  vtkMRMLDisplayableManagerGroup * threeDViewGroup = threeDViewFactory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!threeDViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  currentCount = threeDViewGroup->GetDisplayableManagerCount();
  if (currentCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewGroup->GetDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Instantiate and add node to the scene
  vtkNew<vtkMRMLViewNode> viewNode;
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Associate displayable node to the group
  threeDViewGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  // Slice - Instantiate displayable managers
  vtkMRMLDisplayableManagerGroup * sliceViewGroup =
      slicerViewFactory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!sliceViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with sliceViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  currentCount = sliceViewGroup->GetDisplayableManagerCount();
  if (currentCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with sliceViewGroup->GetDisplayableManagerCount() method"
        << std::endl;
    std::cerr << "\tcurrentCount:" << currentCount << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Instantiate and add node to the scene
  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  sliceNode->SetName("Red-Axial");
  nodeAdded = scene->AddNode(sliceNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkSliceViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Associate displayable node to the group
  sliceViewGroup->SetMRMLDisplayableNode(sliceNode.GetPointer());

  // Add node to the scene
  vtkNew<vtkMRMLCameraNode> cameraNode;
  scene->AddNode(cameraNode.GetPointer());

  // Check if both displayable manager cought the event
  if (vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestThreeDViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestSliceViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }

  // Reset
  vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount = 0;
  vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount = 0;
  vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView = 0;
  vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView = 0;


  // Load scene
  std::string dataRoot = testHelper->GetDataRoot();
  std::string mrmlFiletoLoad = dataRoot + "/Data/vtkMRMLDisplayableManagerFactoriesTest1-load.mrml";
  scene->SetURL(mrmlFiletoLoad.c_str());
  bool success = scene->Connect() != 0;
  if (!success)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkMRMLScene::Connect" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if scene contains the expected node
  std::vector<vtkMRMLNode*> cameraNodes;
  scene->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  if (cameraNodes.size() != 1 || !vtkMRMLCameraNode::SafeDownCast(cameraNodes.at(0)))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkMRMLScene::Import"
              << std::endl << "\tScene CameraNode count - current:"
              << cameraNodes.size() << " - expected: 1" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if both displayable manager cought the event
  if (vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestThreeDViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestSliceViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 1"<< std::endl;
    return EXIT_FAILURE;
    }


  // Import scene
  std::string mrmlFiletoImport = dataRoot + "/Data/vtkMRMLDisplayableManagerFactoriesTest1-import.mrml";
  scene->SetURL(mrmlFiletoImport.c_str());
  success = scene->Import() != 0;
  if (!success)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkMRMLScene::Import" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if scene contains the expected node
  cameraNodes.clear();
  scene->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  if (cameraNodes.size() != 2 || !vtkMRMLCameraNode::SafeDownCast(cameraNodes.at(0)))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with vtkMRMLScene::Import"
              << std::endl << "\tScene CameraNode count - current:"
              << cameraNodes.size() << " - expected: 2" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if both displayable manager cought the event
  if (vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestThreeDViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestSliceViewDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent method"
        << std::endl;
    std::cerr << "\tNodeAddedCount - current:" <<
              vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView
              << "- expected: 2"<< std::endl;
    return EXIT_FAILURE;
    }

  threeDViewGroup->Delete();
  sliceViewGroup->Delete();

  return EXIT_SUCCESS;
}
