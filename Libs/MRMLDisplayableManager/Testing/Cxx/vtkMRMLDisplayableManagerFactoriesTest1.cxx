
// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkThreeDViewInteractorStyle.h>
#include <vtkMRMLTestThreeDViewDisplayableManager.h>
#include <vtkMRMLTestSliceViewDisplayableManager.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLCrosshairNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTesting.h>

// STD includes
#include <cstdlib>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerFactoriesTest1(int argc, char** argv)
{
  VTK_CREATE(vtkTesting, testHelper);
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
  VTK_CREATE(vtkMRMLScene, scene);

  // MRML Application logic (Add Interaction and Selection node)
  VTK_CREATE(vtkMRMLApplicationLogic, mrmlAppLogic);
  mrmlAppLogic->SetMRMLScene(scene);

  // Register displayable manager
  threeDViewFactory->RegisterDisplayableManager("vtkMRMLTestThreeDViewDisplayableManager");
  slicerViewFactory->RegisterDisplayableManager("vtkMRMLTestSliceViewDisplayableManager");

  // Renderer, RenderWindow and Interactor
  VTK_CREATE(vtkRenderer, rr);
  VTK_CREATE(vtkRenderWindow, rw);
  VTK_CREATE(vtkRenderWindowInteractor, ri);
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  rw->AddRenderer(rr);
  rw->SetInteractor(ri);

  // Set Interactor Style
  VTK_CREATE(vtkThreeDViewInteractorStyle, iStyle);
  ri->SetInteractorStyle(iStyle);

  // ThreeD - Instantiate displayable managers
  vtkMRMLDisplayableManagerGroup * threeDViewGroup = threeDViewFactory->InstantiateDisplayableManagers(rr);
  if (!threeDViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with threeDViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Instantiate and add node to the scene
  VTK_CREATE(vtkMRMLViewNode, viewNode);
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode);
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // ThreeD - Associate displayable node to the group
  threeDViewGroup->SetMRMLDisplayableNode(viewNode);

  // Slice - Instantiate displayable managers
  vtkMRMLDisplayableManagerGroup * sliceViewGroup = slicerViewFactory->InstantiateDisplayableManagers(rr);
  if (!sliceViewGroup)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with sliceViewFactory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Instantiate and add node to the scene
  VTK_CREATE(vtkMRMLSliceNode, sliceNode);
  sliceNode->SetLayoutName("Red");
  sliceNode->SetName("Red-Axial");
  nodeAdded = scene->AddNode(sliceNode);
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkSliceViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Slice - Associate displayable node to the group
  sliceViewGroup->SetMRMLDisplayableNode(sliceNode);

  // Add node to the scene
  VTK_CREATE(vtkMRMLCameraNode, cameraNode);
  scene->AddNode(cameraNode);

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

  // Reset
  vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount = 0;
  vtkMRMLTestSliceViewDisplayableManager::NodeAddedCount = 0;


  // Load scene
  std::string mrmlFiletoLoad(testHelper->GetDataRoot());
  mrmlFiletoLoad.append("Data/vtkMRMLDisplayableManagerFactoriesTest1-load.mrml");
  scene->SetURL(mrmlFiletoLoad.c_str());
  bool success = scene->Connect();
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


  // Import scene
  std::string mrmlFiletoImport(testHelper->GetDataRoot());
  mrmlFiletoImport.append("Data/vtkMRMLDisplayableManagerFactoriesTest1-import.mrml");
  scene->SetURL(mrmlFiletoImport.c_str());
  success = scene->Import();
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

  threeDViewGroup->Delete();
  sliceViewGroup->Delete();

  return EXIT_SUCCESS;
}
