/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLModelSliceDisplayableManager.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkActor2DCollection.h>
#include <vtkActorCollection.h>
#include <vtkDoubleArray.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkUnsignedCharArray.h>

// STD includes
#include <algorithm>

namespace
{

//----------------------------------------------------------------------------
/// Lowest and highest grey levels of mapped colors of a grey color table
void GetGreyRange(vtkUnsignedCharArray* colors, double greyRange[2])
{
  greyRange[0] = 255.0;
  greyRange[1] = 0.0;
  for (vtkIdType i = 0; colors && i < colors->GetNumberOfTuples(); ++i)
  {
    greyRange[0] = std::min(greyRange[0], colors->GetComponent(i, 0));
    greyRange[1] = std::max(greyRange[1], colors->GetComponent(i, 0));
  }
}

//----------------------------------------------------------------------------
vtkUnsignedCharArray* GetColors3D(vtkRenderer* renderer)
{
  vtkActorCollection* actors = renderer->GetActors();
  actors->InitTraversal();
  while (vtkActor* actor = actors->GetNextActor())
  {
    vtkMapper* mapper = actor->GetMapper();
    if (mapper && mapper->GetScalarVisibility() && mapper->GetInput() && mapper->GetInput()->GetNumberOfPoints() > 0)
    {
      return mapper->MapScalars(1.0);
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkUnsignedCharArray* GetColorsSlice(vtkRenderer* renderer)
{
  vtkActor2DCollection* actors = renderer->GetActors2D();
  actors->InitTraversal();
  while (vtkActor2D* actor = actors->GetNextActor2D())
  {
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());
    if (mapper && mapper->GetScalarVisibility() && mapper->GetInput() && mapper->GetInput()->GetNumberOfPoints() > 0)
    {
      return mapper->MapScalars(1.0);
    }
  }
  return nullptr;
}

} // namespace

//----------------------------------------------------------------------------
/// The colors that the mappers of 3D and slice views produce follow the active scalar component.
int vtkMRMLModelScalarComponentDisplayableManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  vtkNew<vtkRenderer> renderer3D;
  vtkNew<vtkRenderWindow> renderWindow3D;
  vtkNew<vtkRenderWindowInteractor> interactor3D;
  renderWindow3D->SetOffScreenRendering(1);
  renderWindow3D->SetSize(300, 300);
  renderWindow3D->AddRenderer(renderer3D);
  renderWindow3D->SetInteractor(interactor3D);
  vtkNew<vtkMRMLDisplayableManagerGroup> group3D;
  group3D->SetRenderer(renderer3D);
  group3D->SetMRMLDisplayableNode(scene->AddNewNodeByClass("vtkMRMLViewNode"));
  vtkNew<vtkMRMLModelDisplayableManager> manager3D;
  manager3D->SetMRMLApplicationLogic(applicationLogic);
  group3D->AddDisplayableManager(manager3D);

  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  scene->AddNode(sliceNode);
  sliceNode->SetDimensions(300, 300, 1);
  sliceNode->SetFieldOfView(150.0, 150.0, 5.0);
  sliceNode->UpdateMatrices();
  vtkNew<vtkRenderer> rendererSlice;
  vtkNew<vtkRenderWindow> renderWindowSlice;
  vtkNew<vtkRenderWindowInteractor> interactorSlice;
  renderWindowSlice->SetOffScreenRendering(1);
  renderWindowSlice->SetSize(300, 300);
  renderWindowSlice->AddRenderer(rendererSlice);
  renderWindowSlice->SetInteractor(interactorSlice);
  vtkNew<vtkMRMLDisplayableManagerGroup> groupSlice;
  groupSlice->SetRenderer(rendererSlice);
  groupSlice->SetMRMLDisplayableNode(sliceNode);
  vtkNew<vtkMRMLModelSliceDisplayableManager> managerSlice;
  managerSlice->SetMRMLApplicationLogic(applicationLogic);
  groupSlice->AddDisplayableManager(managerSlice);

  // A plane that the axial slice view cuts, not through its points. The vectors are (1, t, 0),
  // with t growing from 0 to 10 along x, so the first component is the same everywhere and
  // does not tell the points apart, while the magnitude and the second component do.
  vtkNew<vtkPlaneSource> plane;
  plane->SetOrigin(-50.0, 0.0, -50.0);
  plane->SetPoint1(50.0, 0.0, -50.0);
  plane->SetPoint2(-50.0, 0.0, 53.0);
  plane->SetResolution(10, 10);
  plane->Update();
  vtkPolyData* mesh = plane->GetOutput();
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName("Vectors");
  vectors->SetNumberOfComponents(3);
  for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
  {
    vectors->InsertNextTuple3(1.0, (mesh->GetPoint(i)[0] + 50.0) / 10.0, 0.0);
  }
  mesh->GetPointData()->AddArray(vectors);

  vtkNew<vtkMRMLColorTableNode> colorNode;
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObservePolyData(mesh);
  modelNode->CreateDefaultDisplayNodes();
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  CHECK_NOT_NULL(displayNode);
  displayNode->SetVisibility(true);
  displayNode->SetVisibility2D(true);
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
  displayNode->SetActiveScalarName("Vectors");
  displayNode->SetScalarVisibility(true);

  for (int view = 0; view < 2; ++view)
  {
    vtkRenderWindow* renderWindow = (view == 0 ? renderWindow3D.GetPointer() : renderWindowSlice.GetPointer());
    vtkRenderer* renderer = (view == 0 ? renderer3D.GetPointer() : rendererSlice.GetPointer());
    double greyRange[2] = { 0.0, 0.0 };

    // The magnitude by default: the longest vector is at the top of the color table
    displayNode->SetActiveScalarComponent(-1);
    renderWindow->Render();
    vtkUnsignedCharArray* colors = (view == 0 ? GetColors3D(renderer) : GetColorsSlice(renderer));
    CHECK_NOT_NULL(colors);
    GetGreyRange(colors, greyRange);
    CHECK_BOOL(greyRange[1] > 250.0, true);
    CHECK_BOOL(greyRange[0] < 5.0, true);

    // The second component spans the color table as well
    displayNode->SetActiveScalarComponent(1);
    renderWindow->Render();
    GetGreyRange(view == 0 ? GetColors3D(renderer) : GetColorsSlice(renderer), greyRange);
    CHECK_BOOL(greyRange[1] > 250.0, true);

    // The first component is the same everywhere, so all points have the same color
    displayNode->SetActiveScalarComponent(0);
    renderWindow->Render();
    GetGreyRange(view == 0 ? GetColors3D(renderer) : GetColorsSlice(renderer), greyRange);
    CHECK_DOUBLE(greyRange[0], greyRange[1]);
  }

  manager3D->SetMRMLApplicationLogic(nullptr);
  managerSlice->SetMRMLApplicationLogic(nullptr);
  return EXIT_SUCCESS;
}
