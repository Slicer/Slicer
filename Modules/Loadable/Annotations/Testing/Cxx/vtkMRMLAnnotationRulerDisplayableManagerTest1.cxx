/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLViewDisplayableManager.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLAnnotationRulerNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.h"

// STD includes
#include <cstdlib>
#include <iterator>

#include "vtkMRMLCoreTestingMacros.h"

//----------------------------------------------------------------------------
class vtkRenderRequestCallback : public vtkCommand
{
public:
  static vtkRenderRequestCallback *New()
    { return new vtkRenderRequestCallback; }
  void SetRenderer(vtkRenderer *renderer)
    { this->Renderer =  renderer; }
  int GetRenderRequestCount()
    { return this->RenderRequestCount; }
  virtual void Execute(vtkObject*, unsigned long , void* )
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback():Renderer(0), RenderRequestCount(0){}
  vtkRenderer * Renderer;
  int           RenderRequestCount;
};

//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerDisplayableManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{


  /*
   *
   * Setup the test scenario.
   *
   */
  // Renderer, RenderWindow and Interactor
   vtkRenderer* rr = vtkRenderer::New();
   vtkRenderWindow* rw = vtkRenderWindow::New();
   vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
   rw->SetSize(600, 600);

   rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

   rw->AddRenderer(rr);
   rw->SetInteractor(ri);

   // Set Interactor Style
   vtkMRMLThreeDViewInteractorStyle * iStyle = vtkMRMLThreeDViewInteractorStyle::New();
   ri->SetInteractorStyle(iStyle);
   iStyle->Delete();

   // MRML scene
   vtkMRMLScene* scene = vtkMRMLScene::New();

   // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
   vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
   applicationLogic->SetMRMLScene(scene);

   // Add ViewNode
   vtkMRMLViewNode * viewNode = vtkMRMLViewNode::New();
   vtkMRMLNode * nodeAdded = scene->AddNode(viewNode);
   viewNode->Delete();
   if (!nodeAdded)
     {
     std::cerr << "Failed to add vtkMRMLViewNode" << std::endl;
     return EXIT_FAILURE;
     }
  // Factory
  vtkMRMLThreeDViewDisplayableManagerFactory * factoryThreeDView = vtkMRMLThreeDViewDisplayableManagerFactory::New();
  vtkMRMLSliceViewDisplayableManagerFactory * factorySliceView = vtkMRMLSliceViewDisplayableManagerFactory::New();

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factoryThreeDView->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factoryThreeDView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  if (factorySliceView->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factorySliceView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }


  factoryThreeDView->RegisterDisplayableManager(
      "vtkMRMLAnnotationRulerDisplayableManager");
  factorySliceView->RegisterDisplayableManager(
      "vtkMRMLAnnotationRulerDisplayableManager");


  /*
   *
   * Check if factory registered the displayable Managers
   *
   */
  // Check if GetRegisteredDisplayableManagerCount returns 1
  if (factoryThreeDView->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factoryThreeDView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }
  // Check if GetRegisteredDisplayableManagerCount returns 1
  if (factorySliceView->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factorySliceView->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }


  vtkMRMLDisplayableManagerGroup * displayableManagerGroupThreeDView =
      factoryThreeDView->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroupThreeDView)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  //vtkMRMLDisplayableManagerGroup * displayableManagerGroupSliceView =
   //   factorySliceView->InstantiateDisplayableManagers(rr);

  /*
   *
   * Start testing the actual functionality.
   *
   * Testing plan
   * 1. Create ruler annotation and fire vtkMRMLScene::NodeAddedEvent events through MRMLScene->AddNode
   * 2. Delete ruler annotations and fire vtkMRMLScene::NodeRemovedEvent events through MRMLScene->RemoveNode
   *
   */

  //--------------------------------------------------------------------------------------
  // TEST 1:
  // Add one TextNode to scene and see if widget appears!
  //

  // fail if widgets are in Renderwindow
  if (rr->GetViewProps()->GetNumberOfItems()>0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  double worldCoordinates1[4];
  double worldCoordinates2[4];

  worldCoordinates1[0] = 10;
  worldCoordinates1[1] = 10;
  worldCoordinates1[2] = 0;
  worldCoordinates1[3] = 1;

  worldCoordinates1[0] = 30;
  worldCoordinates1[1] = 30;
  worldCoordinates1[2] = 0;
  worldCoordinates1[3] = 1;



  // create the MRML node
  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);
  rulerNode->SetDistanceMeasurement(distance);

  rulerNode->Initialize(scene);

  rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));

  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=1) {
    std::cerr << "Expected number of items in renderer: 1" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  //--------------------------------------------------------------------------------------
  // TEST 2:
  // Delete ruler annotations and fire vtkMRMLScene::NodeRemovedEvent events through MRMLScene->RemoveNode
  //
  scene->RemoveNode(rulerNode);

  // fail if widget did not disappear
  if (rr->GetViewProps()->GetNumberOfItems()!=0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  // cleanup
  rulerNode->Delete();

  applicationLogic->Delete();
  scene->RemoveNode(viewNode);
  viewNode->Delete();

  factoryThreeDView->Delete();
  factorySliceView->Delete();

  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();


  return EXIT_SUCCESS;
}

