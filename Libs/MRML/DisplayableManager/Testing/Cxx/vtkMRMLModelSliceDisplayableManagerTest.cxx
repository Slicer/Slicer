/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelSliceDisplayableManager.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkActor2DCollection.h>
#include <vtkCamera.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRTAnalyticSource.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWindowToImageFilter.h>

// STD includes
#include <iostream>

bool TestBatchRemoveDisplayNode();
bool TestVolumetricMeshIntersectionIsFilled();

//----------------------------------------------------------------------------
int vtkMRMLModelSliceDisplayableManagerTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  bool res = true;
  res = TestBatchRemoveDisplayNode() && res;
  res = TestVolumetricMeshIntersectionIsFilled() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderWindow> CreateRenderWindow()
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());
  return vtkSmartPointer<vtkRenderWindow>(renderWindow.GetPointer());
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLDisplayableManagerGroup> CreateDisplayableManager(vtkMRMLScene* scene, vtkRenderer* renderer)
{
  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  // Add ViewNode
  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  scene->AddNode(sliceNode.GetPointer());

  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer);
  displayableManagerGroup->SetMRMLDisplayableNode(sliceNode.GetPointer());

  vtkNew<vtkMRMLModelSliceDisplayableManager> displayableManager;
  displayableManager->SetMRMLApplicationLogic(applicationLogic.GetPointer());
  displayableManagerGroup->AddDisplayableManager(displayableManager.GetPointer());
  displayableManagerGroup->GetInteractor()->Initialize();
  return displayableManagerGroup.GetPointer();
}

//----------------------------------------------------------------------------
bool TestBatchRemoveDisplayNode()
{
  vtkSmartPointer<vtkRenderWindow> renderWindow = CreateRenderWindow();
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> displayableManagerGroup = CreateDisplayableManager(scene.GetPointer(), renderWindow->GetRenderers()->GetFirstRenderer());

  // We observe the model display nodes and we make sure only the mrmlscene
  // has a reference on them (ref count is 1)
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::New();
  scene->AddNode(modelDisplayNode);
  modelDisplayNode->Delete();

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::New();
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(10.);
  sphereSource->Update();

  modelNode->SetPolyDataConnection(sphereSource->GetOutputPort());
  modelNode->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());
  scene->AddNode(modelNode);
  modelNode->Delete();

  // and we delete them
  scene->StartState(vtkMRMLScene::BatchProcessState);
  scene->RemoveNode(modelNode);
  scene->RemoveNode(modelDisplayNode);
  scene->EndState(vtkMRMLScene::BatchProcessState);

  // We trigger an update here (UpdateSliceNode) to make sure the displayable
  // manager doesn't use the deleted nodes.
  vtkMRMLNode* sliceNode = scene->GetNodeByID("vtkMRMLSliceNodeRed");
  sliceNode->Modified();
  return true;
}

//----------------------------------------------------------------------------
/// Number of polygons in the intersection of the model shown in the slice view
vtkIdType GetNumberOfIntersectionPolygons(vtkRenderer* renderer)
{
  vtkActor2DCollection* actors = renderer->GetActors2D();
  actors->InitTraversal();
  while (vtkActor2D* actor = actors->GetNextActor2D())
  {
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());
    if (mapper && actor->GetVisibility() && mapper->GetInputAlgorithm())
    {
      mapper->GetInputAlgorithm()->Update();
      return mapper->GetInput()->GetNumberOfPolys();
    }
  }
  return 0;
}

//----------------------------------------------------------------------------
/// The intersection of a volumetric mesh stays filled when it is updated without the mesh having
/// changed, which is when only the cells that the slice plane crosses are cut.
bool TestVolumetricMeshIntersectionIsFilled()
{
  vtkSmartPointer<vtkRenderWindow> renderWindow = CreateRenderWindow();
  vtkRenderer* renderer = renderWindow->GetRenderers()->GetFirstRenderer();
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> displayableManagerGroup = CreateDisplayableManager(scene.GetPointer(), renderer);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetNodeByID("vtkMRMLSliceNodeRed"));
  sliceNode->SetDimensions(600, 600, 1);
  sliceNode->SetFieldOfView(50.0, 50.0, 1.0);
  sliceNode->UpdateMatrices();
  sliceNode->SetSliceOffset(0.5);

  // A tetrahedral mesh that has enough cells for a cell locator to be used
  vtkNew<vtkRTAnalyticSource> image;
  image->SetWholeExtent(-12, 12, -12, 12, -12, 12);
  vtkNew<vtkDataSetTriangleFilter> tetrahedra;
  tetrahedra->SetInputConnection(image->GetOutputPort());
  tetrahedra->Update();

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(tetrahedra->GetOutput());
  modelNode->CreateDefaultDisplayNodes();
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  displayNode->SetVisibility(true);
  displayNode->SetVisibility2D(true);

  // Updates that do not change the mesh, and then a slice that the locator has not seen yet
  const vtkIdType polygonsOfWholeMeshCut = GetNumberOfIntersectionPolygons(renderer);
  displayNode->SetOpacity(0.8);
  displayNode->SetOpacity(0.5);
  const vtkIdType polygonsAfterDisplayChange = GetNumberOfIntersectionPolygons(renderer);
  sliceNode->SetSliceOffset(1.5);
  const vtkIdType polygonsAfterSliceMove = GetNumberOfIntersectionPolygons(renderer);

  if (polygonsOfWholeMeshCut == 0 || polygonsAfterDisplayChange != polygonsOfWholeMeshCut || polygonsAfterSliceMove == 0)
  {
    std::cerr << "Line " << __LINE__ << ": intersection of a volumetric mesh is not filled: " //
              << polygonsOfWholeMeshCut << ", " << polygonsAfterDisplayChange << ", " << polygonsAfterSliceMove << " polygons" << std::endl;
    return false;
  }
  return true;
}
