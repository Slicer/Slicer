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
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLViewDisplayableManager.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLAnnotationPointDisplayNode.h>
#include <vtkMRMLAnnotationTextDisplayNode.h>
#include <vtkMRMLAnnotationTextNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>
#include <vtkWindowToImageFilter.h>


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
int vtkMRMLAnnotationTextNodeAndWidgetTest1(int argc, char* argv[])
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


  /*
   *
   * Create the DisplayableManager factory and register the DisplayableManager
   *
   * Perform some tests to ensure registration succeeded.
   *
   */
  // Factory
  vtkMRMLThreeDViewDisplayableManagerFactory * factory = vtkMRMLThreeDViewDisplayableManagerFactory::New();

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  factory->RegisterDisplayableManager("vtkMRMLAnnotationTextDisplayableManager");

  /*
   *
   * Check if factory registered the displayable Manager
   *
   */
  // Check if GetRegisteredDisplayableManagerCount returns 2
  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLDisplayableManagerGroup * displayableManagerGroup =
      factory->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }


  if (displayableManagerGroup->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Check displayableManagerGroup->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "Expected DisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current DisplayableManagerCount:"
      << displayableManagerGroup->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // RenderRequest Callback
  vtkRenderRequestCallback * renderRequestCallback = vtkRenderRequestCallback::New();
  renderRequestCallback->SetRenderer(rr);
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback);

  // Assign ViewNode
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode);

  // Check if RenderWindowInteractor has NOT been changed
  if (displayableManagerGroup->GetInteractor() != ri)
    {
    std::cerr << "Expected RenderWindowInteractor:" << ri << std::endl;
    std::cerr << "Current RenderWindowInteractor:"
        << displayableManagerGroup->GetInteractor() << std::endl;
    return EXIT_FAILURE;
    }

  // Interactor style should be vtkMRMLThreeDViewInteractorStyle
  vtkInteractorObserver * currentInteractoryStyle = ri->GetInteractorStyle();
  if (!vtkMRMLThreeDViewInteractorStyle::SafeDownCast(currentInteractoryStyle))
    {
    std::cerr << "Expected interactorStyle: vtkMRMLThreeDViewInteractorStyle" << std::endl;
    std::cerr << "Current RenderWindowInteractor: "
      << (currentInteractoryStyle ? currentInteractoryStyle->GetClassName() : "Null") << std::endl;
    return EXIT_FAILURE;
    }


  /*
   *
   * Start testing the actual functionality.
   *
   * Testing plan
   * 1. Create text annotation and widget
   *
   */

  //--------------------------------------------------------------------------------------
  // TEST 1:
  // Add one TextNode to scene and see if widget appears!
  //

  // fail if there are already widgets in the RenderView
  if (rr->GetViewProps()->GetNumberOfItems()!=0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  double coordinates[3];
  coordinates[0]=(double)30;
  coordinates[1]=(double)30;
  coordinates[2]=0;

  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
  textNode->GetTextLabel();
  textNode->SetTextLabel("New text");

  scene->AddNode(textNode);

  textNode->CreateAnnotationTextDisplayNode();
  textNode->CreateAnnotationPointDisplayNode();

  textNode->GetAnnotationTextDisplayNode()->SetTextScale(10);

  textNode->SetTextCoordinates(coordinates);

  // need a unique name since the storage node will be named from it
  if (textNode->GetScene())
    {
    textNode->SetName(textNode->GetScene()->GetUniqueNameByString("AnnotationText"));
    }
  else
    {
    textNode->SetName("AnnotationText");
    }

  vtkTextWidget* textWidget = vtkTextWidget::New();
  vtkNew<vtkTextRepresentation> textRep;

  textRep->SetMoving(1);

  if (textNode->GetText(0)!="New text")
    {
    std::cerr << "Expected GetText: New text" << std::endl;
    std::cerr << "Current GetText: " << textNode->GetText(0) << std::endl;
    return EXIT_FAILURE;
    }
  textRep->SetText(textNode->GetText(0));

  textWidget->SetRepresentation(textRep.GetPointer());
  textWidget->SetInteractor(ri);

  if (textNode->GetTextCoordinates()[0]!=30)
    {
    std::cerr << "Expected GetTextCoordinates[0]: 30" << std::endl;
    std::cerr << "Current GetTextCoordinates[0]: " << textNode->GetTextCoordinates()[0]<< std::endl;
    return EXIT_FAILURE;
    }

  if (textNode->GetTextCoordinates()[1]!=30)
    {
    std::cerr << "Expected GetTextCoordinates[1]: 30" << std::endl;
    std::cerr << "Current GetTextCoordinates[1]: " << textNode->GetTextCoordinates()[1] << std::endl;
    return EXIT_FAILURE;
    }

  textRep->SetPosition(textNode->GetTextCoordinates());
  textWidget->On();


  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=1) {
    std::cerr << "Expected number of items in renderer: 1" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  scene->RemoveNode(textNode);

  std::cout << "Test OK!" << std::endl;

  textNode->Delete();
  textWidget->Delete();
  textRep->Delete();

  renderRequestCallback->Delete();
  if (displayableManagerGroup) { displayableManagerGroup->Delete(); }
  factory->Delete();
  applicationLogic->Delete();
  scene->Delete();

  ri->Delete();
  rr->Delete();
  rw->Delete();


  return EXIT_SUCCESS;
}

