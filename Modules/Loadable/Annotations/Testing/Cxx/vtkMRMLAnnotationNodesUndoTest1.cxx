// MRMLDisplayableManager includes
#include <vtkMRMLAnnotationFiducialDisplayableManager.h>
#include <vtkMRMLAnnotationRulerDisplayableManager.h>
#include <vtkMRMLAnnotationBidimensionalDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <sstream>

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
  void Execute(vtkObject*, unsigned long , void* ) override
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback() = default;
  vtkRenderer * Renderer{nullptr};
  int           RenderRequestCount{0};
};

int vtkMRMLAnnotationNodesUndoTest1(int , char * [] )
{

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
  vtkMRMLThreeDViewDisplayableManagerFactory * factory = vtkMRMLThreeDViewDisplayableManagerFactory::New();

  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLViewDisplayableManager");
//  factory->RegisterDisplayableManager("vtkMRMLAnnotationDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLAnnotationFiducialDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLAnnotationRulerDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLAnnotationBidimensionalDisplayableManager");
  // need a model displayable manager to test for superclass conflicts
  factory->RegisterDisplayableManager("vtkMRMLModelDisplayableManager");

  vtkMRMLDisplayableManagerGroup * displayableManagerGroup =
      factory->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  // RenderRequest Callback
  vtkRenderRequestCallback * renderRequestCallback = vtkRenderRequestCallback::New();
  renderRequestCallback->SetRenderer(rr);
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback);

  // Assign ViewNode
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode);


  // test undo/redo on the scene with a fiducial node
  vtkNew<vtkMRMLAnnotationFiducialNode> node1;
  scene->RegisterNodeClass(node1.GetPointer());
  double f1[3];
  f1[0] = 0.0;
  f1[1] = 1.0;
  f1[2] = -1.0;
  node1->SetFiducialWorldCoordinates(f1);
  scene->SaveStateForUndo();
  node1->Initialize(scene);
  scene->Undo();
  std::cout << "After undo for fiducial" << std::endl;

  // test undo/redo on the scene with a ruler node
  vtkNew<vtkMRMLAnnotationRulerNode> node2;
  scene->RegisterNodeClass(node2.GetPointer());
  std::cout << "Ruler node class registered: " << node2->GetClassName() << std::endl;
  double p1[3], p2[3];
  p1[0] = p1[1] = p1[2] = 0.0;
  p2[0] = p2[1] = p1[2] = 1.0;
  node2->SetPositionWorldCoordinates1(p1);
  node2->SetPositionWorldCoordinates2(p2);
  std::cout << "Ruler node positioned" << std::endl;
  scene->SaveStateForUndo();
  std::cout << "Scene state saved for undo" << std::endl;
  // this adds the node to the scene, along with display nodes
  node2->Initialize(scene);
  std::cout << "Ruler node initialized/added to scene" << std::endl;
  if (node2->GetID())
    {
    std::cout << "Node2 added to scene, id = " << node2->GetID() << ", number of display nodes = " << node2->GetNumberOfDisplayNodes() << std::endl;

    }
  std::cout << "Calling Scene Undo" << std::endl;
  scene->Undo();
  std::cout << "After undo for ruler" << std::endl;

  // test undo/redo on the scene with a bidimensional node
  vtkNew<vtkMRMLAnnotationBidimensionalNode> node3;
  scene->RegisterNodeClass(node3.GetPointer());
  double b1[3];
  b1[0] = 0.0;
  b1[1] = 1.0;
  b1[2] = -1.0;
  node3->SetControlPoint(b1, 0);
  b1[0] = 1.0;
  node3->SetControlPoint(b1, 1);
  b1[0] = -1.0;
  node3->SetControlPoint(b1, 2);
  b1[0] = 0.5;
  node3->SetControlPoint(b1, 3);
  scene->SaveStateForUndo();
  node3->Initialize(scene);
  scene->Undo();
  std::cout << "After undo for bidimensional" << std::endl;

  // and one more test: add a fiducial after all the undo's are done
  vtkNew<vtkMRMLAnnotationFiducialNode> doFid;
  doFid->SetFiducialWorldCoordinates(f1);
  scene->SaveStateForUndo();
  doFid->Initialize(scene);
  std::cout << "After adding a new fiducial" << std::endl;
  f1[2] = -5.4;
  doFid->SetFiducialWorldCoordinates(f1);
  std::cout << "After resetting the world position of the new fiducial" << std::endl;

  renderRequestCallback->Delete();
  if (displayableManagerGroup) { displayableManagerGroup->Delete(); }
  factory->Delete();
  applicationLogic->Delete();
  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();

  return EXIT_SUCCESS;

}


