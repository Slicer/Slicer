// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLCameraWidget.h>
#include <vtkMRMLCameraDisplayableManager.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkTransform.h>
#include <vtkCamera.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"


namespace {


/// Imitate translation of the screen (SHIFT+M1+DRAG)
void doTranslate(vtkRenderer* renderer, vtkMRMLViewNode* viewNode, vtkMRMLCameraWidget* cameraWidget){
// Process CLICK ON (click mouse 1 btn)
  vtkNew<vtkMRMLInteractionEventData> ev;
  ev->SetRenderer(renderer);
  ev->SetViewNode(viewNode);
  ev->SetType(12);
  ev->SetKeySym("Shift_L");
  ev->SetModifiers(1);  // Shift btn
  ev->SetMouseMovedSinceButtonDown(false);
  double wp[3];
  wp[0] = 0;
  wp[1] = 0;
  wp[2] = 0;
  int dp[2];
  dp[0] = 0;
  dp[1] = 0;
  ev->SetWorldPosition(wp);
  ev->SetDisplayPosition(dp);
  cameraWidget->ProcessInteractionEvent(ev);

  // Process DRAG
  ev->SetType(26);
  ev->SetMouseMovedSinceButtonDown(true);
  wp[0] = 10;
  wp[1] = 0;
  wp[2] = 0;
  dp[0] = 100;
  dp[1] = 0;
  ev->SetWorldPosition(wp);
  ev->SetDisplayPosition(dp);
  cameraWidget->ProcessInteractionEvent(ev);

  // Process CLICK OFF (release the mouse 1 btn)
  ev->SetType(13);
  ev->SetMouseMovedSinceButtonDown(true);
  cameraWidget->ProcessInteractionEvent(ev);
}

}

//----------------------------------------------------------------------------
int vtkMRMLCameraWidgetTest1(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr);
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // MRML scene
  vtkNew<vtkMRMLScene> scene;

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());

  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  viewNode->SetLayoutName("1");
  viewNode->SetLayoutLabel("1");
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode.GetPointer());
  if (!nodeAdded)
  {
    std::cerr << "Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
  }

  // Factory
  vtkNew<vtkMRMLThreeDViewDisplayableManagerFactory> factory;

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factory->GetRegisteredDisplayableManagerCount() != 0)
  {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
  }

  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLViewDisplayableManager");

  // Check if GetRegisteredDisplayableManagerCount returns 2
  if (factory->GetRegisteredDisplayableManagerCount() != 2)
  {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 2" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
  }

  // Instantiate DisplayableManagerGroup
  // Without `vtkSmartPointer<>::Take` we'll get memory leak (or we need to use `group->Delete()` at the end)
  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> group =
      vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(factory->InstantiateDisplayableManagers(rr.GetPointer()));
  if (!group)
  {
    std::cerr << "Line " << __LINE__
              << " - Problem with factory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
  }

  vtkMRMLCameraDisplayableManager * cameraDM2 =
      vtkMRMLCameraDisplayableManager::SafeDownCast(
        group->GetDisplayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
  if (!cameraDM2)
  {
    std::cerr << "Line " << __LINE__
              << " - Problem with cameraDM2 = group->GetDisplayableManagerByClassName(\"vtkMRMLCameraDisplayableManager\") method"
        << std::endl;
    std::cerr << "\tcameraDM2 should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
  }

  vtkMRMLCameraWidget* cameraWidget = cameraDM2->GetCameraWidget();
  if (!cameraWidget)
  {
    std::cerr << "Line " << __LINE__
              << " - Problem with cameraWidget = cameraDM2->GetCameraWidget() method"
        << std::endl;
    std::cerr << "\tcameraWidget should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLCameraNode> cameraNode;
  cameraNode->SetPosition(0, 50, 0);
  cameraWidget->SetRenderer(rr);
  cameraWidget->SetCameraNode(cameraNode);
  rr->SetActiveCamera(cameraWidget->GetCameraNode()->GetCamera());

  // Calculate `deltaCamPos` before changing aspect ratio
  double camPosOld[3], camPosNew[3], deltaCamPos[3];
  cameraWidget->GetCameraNode()->GetPosition(camPosOld);
  doTranslate(rr, viewNode, cameraWidget);
  cameraWidget->GetCameraNode()->GetPosition(camPosNew);
  deltaCamPos[0] = camPosNew[0] - camPosOld[0];
  deltaCamPos[1] = camPosNew[1] - camPosOld[1];
  deltaCamPos[2] = camPosNew[2] - camPosOld[2];

  // Change aspect ratio
  vtkNew<vtkTransform> transform;
  transform->Scale(2, 1, 1);
  cameraWidget->GetCameraNode()->GetCamera()->SetModelTransformMatrix(transform->GetMatrix());

  // Calculate `deltaCamPos` befafterore changing aspect ratio
  double translated_camPosOld[3], translated_camPosNew[3], translated_deltaCamPos[3];
  cameraWidget->GetCameraNode()->GetPosition(translated_camPosOld);
  doTranslate(rr, viewNode, cameraDM2->GetCameraWidget());
  cameraWidget->GetCameraNode()->GetPosition(translated_camPosNew);
  translated_deltaCamPos[0] = translated_camPosNew[0] - translated_camPosOld[0];
  translated_deltaCamPos[1] = translated_camPosNew[1] - translated_camPosOld[1];
  translated_deltaCamPos[2] = translated_camPosNew[2] - translated_camPosOld[2];

  bool isDeltaCamPosEqual = true;
  for (int i = 0; i < 3; i++)
    isDeltaCamPosEqual &= std::abs(translated_deltaCamPos[i] - deltaCamPos[i]) < 1.0e-5;

  return !isDeltaCamPosEqual;
}
