/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Checks that the vector field displayable managers draw every mode a source offers in 3D and slice views (vector volume,
// volumetric mesh, surface mesh on and between points, cell data), by mapper inputs rather than images (GPU independent).

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLVectorFieldDisplayableManager.h>
#include <vtkMRMLVectorFieldSliceDisplayableManager.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldTestingHelpers.h>
#include <vtkVectorFieldSampler.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVectorVolumeNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAssignAttribute.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProp.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkScalarsToColors.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkVector.h>

// STD includes
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
void Field(const double position[3], double vector[3])
{
  vector[0] = 1.0 + 0.02 * position[0];
  vector[1] = std::sin(position[1] / 30.0);
  vector[2] = 0.03 * position[2];
}

//----------------------------------------------------------------------------
/// The visible props of a renderer
std::vector<vtkProp*> GetVisibleProps(vtkRenderer* renderer)
{
  std::vector<vtkProp*> visibleProps;
  vtkPropCollection* props = renderer->GetViewProps();
  vtkCollectionSimpleIterator it;
  props->InitTraversal(it);
  while (vtkProp* prop = props->GetNextProp(it))
  {
    if (prop->GetVisibility())
    {
      visibleProps.push_back(prop);
    }
  }
  return visibleProps;
}

//----------------------------------------------------------------------------
/// Whether a visible actor draws solid lines in the first component of the color, in 3D or in 2D
bool HasVisibleSolidActor(vtkRenderer* renderer, double red)
{
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    vtkActor* actor = vtkActor::SafeDownCast(prop);
    vtkActor2D* actor2D = vtkActor2D::SafeDownCast(prop);
    double* color = actor ? actor->GetProperty()->GetColor() : (actor2D ? actor2D->GetProperty()->GetColor() : nullptr);
    vtkAbstractMapper* mapper = actor ? static_cast<vtkAbstractMapper*>(actor->GetMapper()) : (actor2D ? static_cast<vtkAbstractMapper*>(actor2D->GetMapper()) : nullptr);
    vtkMapper* mapper3D = vtkMapper::SafeDownCast(mapper);
    vtkPolyDataMapper2D* mapper2D = vtkPolyDataMapper2D::SafeDownCast(mapper);
    bool solid = (mapper3D && !mapper3D->GetScalarVisibility()) || (mapper2D && !mapper2D->GetScalarVisibility());
    if (color && solid && std::abs(color[0] - red) < 1e-9)
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
/// The mapper of an actor, in 3D or in 2D; nullptr for any other prop
vtkAbstractMapper* GetMapper(vtkProp* prop)
{
  if (vtkActor* actor = vtkActor::SafeDownCast(prop))
  {
    return actor->GetMapper();
  }
  if (vtkActor2D* actor2D = vtkActor2D::SafeDownCast(prop))
  {
    return actor2D->GetMapper();
  }
  return nullptr;
}

//----------------------------------------------------------------------------
/// What a mapper draws, computed; nullptr if nothing is connected to it
vtkDataSet* GetUpdatedMapperInput(vtkAbstractMapper* mapper)
{
  if (!mapper || !mapper->GetInputAlgorithm())
  {
    return nullptr;
  }
  mapper->GetInputAlgorithm()->Update();
  return vtkDataSet::SafeDownCast(mapper->GetInputDataObject(0, 0));
}

//----------------------------------------------------------------------------
/// Number of points that reach the mappers of the visible actors of a renderer: the glyph
/// positions, or the points of the grid, contour or streamline geometry.
vtkIdType CountDrawnPoints(vtkRenderer* renderer)
{
  vtkIdType numberOfPoints = 0;
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    vtkDataSet* input = GetUpdatedMapperInput(GetMapper(prop));
    numberOfPoints += (input ? input->GetNumberOfPoints() : 0);
  }
  return numberOfPoints;
}

//----------------------------------------------------------------------------
/// The visible actor of a 3D view, or nullptr if there is none
vtkActor* GetVisibleActor(vtkRenderer* renderer)
{
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    if (vtkActor* actor = vtkActor::SafeDownCast(prop))
    {
      return actor;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
/// Opacity of the first visible actor of a 3D renderer, or -1 if there is none.
double GetVisibleActorOpacity(vtkRenderer* renderer)
{
  vtkActor* actor = GetVisibleActor(renderer);
  return (actor ? actor->GetProperty()->GetOpacity() : -1.0);
}

//----------------------------------------------------------------------------
/// Color of the first visible actor of a 3D renderer. Returns false if there is none.
bool GetVisibleActorColor(vtkRenderer* renderer, double color[3])
{
  vtkActor* actor = GetVisibleActor(renderer);
  if (!actor)
  {
    return false;
  }
  actor->GetProperty()->GetColor(color);
  return true;
}

//----------------------------------------------------------------------------
/// What the visible actor of a slice view draws, in slice XY coordinates, or nullptr
vtkPolyData* GetVisibleSliceGeometry(vtkRenderer* renderer)
{
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    vtkDataSet* input = (vtkActor2D::SafeDownCast(prop) ? GetUpdatedMapperInput(GetMapper(prop)) : nullptr);
    if (input)
    {
      return vtkPolyData::SafeDownCast(input);
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
/// Range of the alpha of the per-point colors that reach the mappers of a slice view.
/// Returns false if there are none.
bool GetSliceColorAlphaRange(vtkRenderer* renderer, double alphaRange[2])
{
  alphaRange[0] = VTK_DOUBLE_MAX;
  alphaRange[1] = VTK_DOUBLE_MIN;
  bool found = false;
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    vtkPolyData* input = (vtkActor2D::SafeDownCast(prop) ? vtkPolyData::SafeDownCast(GetUpdatedMapperInput(GetMapper(prop))) : nullptr);
    vtkDataArray* colors = (input ? input->GetPointData()->GetScalars() : nullptr);
    if (!colors || colors->GetNumberOfComponents() != 4)
    {
      continue;
    }
    // Only the points that are on the streamlines that are drawn
    vtkCellArray* lines = input->GetLines();
    vtkNew<vtkIdList> pointIds;
    lines->InitTraversal();
    while (lines->GetNextCell(pointIds))
    {
      for (vtkIdType i = 0; i < pointIds->GetNumberOfIds(); ++i)
      {
        double alpha = colors->GetComponent(pointIds->GetId(i), 3);
        alphaRange[0] = std::min(alphaRange[0], alpha);
        alphaRange[1] = std::max(alphaRange[1], alpha);
        found = true;
      }
    }
  }
  return found;
}

//----------------------------------------------------------------------------
/// Range of the values that the mappers of the visible actors of a renderer color by, over
/// all of them. Returns false if none colors by an array.
bool GetColoredValueRange(vtkRenderer* renderer, double valueRange[2])
{
  valueRange[0] = VTK_DOUBLE_MAX;
  valueRange[1] = VTK_DOUBLE_MIN;
  bool found = false;
  for (vtkProp* prop : GetVisibleProps(renderer))
  {
    // Whether and by which array it colors is asked of the mapper type: 3D and 2D mappers
    // have no common base for that
    vtkAbstractMapper* mapper = GetMapper(prop);
    vtkMapper* mapper3D = vtkMapper::SafeDownCast(mapper);
    vtkPolyDataMapper2D* mapper2D = vtkPolyDataMapper2D::SafeDownCast(mapper);
    bool scalarVisibility = (mapper3D ? mapper3D->GetScalarVisibility() : (mapper2D ? mapper2D->GetScalarVisibility() : false));
    const char* arrayName = (mapper3D ? mapper3D->GetArrayName() : (mapper2D ? mapper2D->GetArrayName() : nullptr));
    vtkDataSet* input = (scalarVisibility && arrayName && arrayName[0] != '\0' ? GetUpdatedMapperInput(mapper) : nullptr);
    vtkDataArray* values = (input ? input->GetPointData()->GetArray(arrayName) : nullptr);
    if (!values || values->GetNumberOfTuples() == 0)
    {
      continue;
    }
    double range[2] = { 0.0, -1.0 };
    values->GetRange(range, values->GetNumberOfComponents() > 1 ? -1 : 0);
    valueRange[0] = std::min(valueRange[0], range[0]);
    valueRange[1] = std::max(valueRange[1], range[1]);
    found = true;
  }
  return found;
}

//----------------------------------------------------------------------------
/// A 3D view and an axial slice view through the origin, each with its vector field
/// displayable manager.
struct Views
{
  vtkNew<vtkRenderer> Renderer3D;
  vtkNew<vtkRenderWindow> RenderWindow3D;
  vtkNew<vtkRenderWindowInteractor> Interactor3D;
  vtkNew<vtkMRMLDisplayableManagerGroup> Group3D;
  vtkNew<vtkMRMLVectorFieldDisplayableManager> Manager3D;

  vtkNew<vtkRenderer> RendererSlice;
  vtkNew<vtkRenderWindow> RenderWindowSlice;
  vtkNew<vtkRenderWindowInteractor> InteractorSlice;
  vtkNew<vtkMRMLDisplayableManagerGroup> GroupSlice;
  vtkNew<vtkMRMLVectorFieldSliceDisplayableManager> ManagerSlice;

  /// A second 3D view, which shares the filters of the display node with the first one
  vtkNew<vtkRenderer> Renderer3DSecond;
  vtkNew<vtkRenderWindow> RenderWindow3DSecond;
  vtkNew<vtkRenderWindowInteractor> Interactor3DSecond;
  vtkNew<vtkMRMLDisplayableManagerGroup> Group3DSecond;
  vtkNew<vtkMRMLVectorFieldDisplayableManager> Manager3DSecond;

  vtkNew<vtkMRMLApplicationLogic> ApplicationLogic;

  /// An off-screen view of a view node, drawn by one displayable manager
  void SetupView(vtkRenderWindow* renderWindow,
                 vtkRenderer* renderer,
                 vtkRenderWindowInteractor* interactor,
                 vtkMRMLDisplayableManagerGroup* group,
                 vtkMRMLAbstractDisplayableManager* manager,
                 vtkMRMLNode* viewNode)
  {
    renderWindow->SetOffScreenRendering(1);
    renderWindow->SetSize(300, 300);
    renderWindow->AddRenderer(renderer);
    renderWindow->SetInteractor(interactor);
    group->SetRenderer(renderer);
    group->SetMRMLDisplayableNode(viewNode);
    manager->SetMRMLApplicationLogic(this->ApplicationLogic);
    group->AddDisplayableManager(manager);
  }

  void Setup(vtkMRMLScene* scene)
  {
    this->ApplicationLogic->SetMRMLScene(scene);
    this->SetupView(this->RenderWindow3D, this->Renderer3D, this->Interactor3D, this->Group3D, this->Manager3D, scene->AddNewNodeByClass("vtkMRMLViewNode"));
    this->SetupView(
      this->RenderWindow3DSecond, this->Renderer3DSecond, this->Interactor3DSecond, this->Group3DSecond, this->Manager3DSecond, scene->AddNewNodeByClass("vtkMRMLViewNode"));

    vtkNew<vtkMRMLSliceNode> sliceNode;
    sliceNode->SetLayoutName("Red");
    scene->AddNode(sliceNode);
    sliceNode->SetDimensions(300, 300, 1);
    // A thick slice, so that it holds the centers of the cells next to the equator of the
    // test sphere as well as its points
    sliceNode->SetFieldOfView(150.0, 150.0, 5.0);
    sliceNode->UpdateMatrices();
    this->SetupView(this->RenderWindowSlice, this->RendererSlice, this->InteractorSlice, this->GroupSlice, this->ManagerSlice, sliceNode);
  }

  void TearDown()
  {
    this->Manager3D->SetMRMLApplicationLogic(nullptr);
    this->Manager3DSecond->SetMRMLApplicationLogic(nullptr);
    this->ManagerSlice->SetMRMLApplicationLogic(nullptr);
  }

  void Render()
  {
    this->RenderWindow3D->Render();
    this->RenderWindow3DSecond->Render();
    this->RenderWindowSlice->Render();
  }
};

//----------------------------------------------------------------------------
/// Both 3D views draw the display node from the same filters, which the display node owns:
/// the mappers of the two views have the same input.
int CheckShared3DFilters(Views& views, const std::string& description)
{
  vtkActor* actor = GetVisibleActor(views.Renderer3D);
  vtkActor* secondActor = GetVisibleActor(views.Renderer3DSecond);
  CHECK_NOT_NULL(actor);
  CHECK_NOT_NULL(secondActor);
  CHECK_BOOL(actor != secondActor, true);
  CHECK_BOOL(actor->GetMapper() != secondActor->GetMapper(), true);
  vtkAlgorithm* input = actor->GetMapper()->GetInputAlgorithm();
  CHECK_NOT_NULL(input);
  CHECK_POINTER(secondActor->GetMapper()->GetInputAlgorithm(), input);
  vtkIdType drawnPoints = CountDrawnPoints(views.Renderer3D);
  std::cout << description << ": " << drawnPoints << " points in both 3D views, drawn by " << input->GetClassName() << std::endl;
  CHECK_BOOL(drawnPoints > 0, true);
  CHECK_INT(CountDrawnPoints(views.Renderer3DSecond), drawnPoints);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The display node draws something in both views in every mode it supports, and the
/// modes that the source does not support are not offered.
int CheckDrawnInModes(Views& views, vtkMRMLVectorFieldDisplayNode* displayNode, const std::string& description, int expectedNumberOfModes)
{
  int numberOfModes = 0;
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    if (!displayNode->IsVisualizationModeSupported(mode))
    {
      continue;
    }
    ++numberOfModes;
    displayNode->SetVisualizationMode(mode);
    views.Render();
    vtkIdType drawnPoints3D = CountDrawnPoints(views.Renderer3D);
    vtkIdType drawnPointsSlice = CountDrawnPoints(views.RendererSlice);
    std::cout << description << ", " << displayNode->GetVisualizationModeAsString() << ": " //
              << drawnPoints3D << " points in 3D, " << drawnPointsSlice << " in the slice" << std::endl;
    CHECK_BOOL(drawnPoints3D > 0, true);
    CHECK_BOOL(drawnPointsSlice > 0, true);
  }
  CHECK_INT(numberOfModes, expectedNumberOfModes);

  // Nothing is drawn when the display node is hidden
  displayNode->SetVisibility(false);
  views.Render();
  CHECK_INT(CountDrawnPoints(views.Renderer3D), 0);
  CHECK_INT(CountDrawnPoints(views.RendererSlice), 0);
  displayNode->SetVisibility(true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// How the test field is shown by every display node here
void SetUpTestField(vtkMRMLScene* scene, vtkMRMLVectorFieldDisplayNode* displayNode)
{
  // Samples are taken at the origin and at a point on the surface of the test sphere: glyphs
  // are drawn there, and streamlines start from there
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(50.0, 0.0, 0.0) }));
  displayNode->SetGridSpacing(20.0);
  displayNode->SetSamplingSpacing(10.0);
  // The magnitude of the test field is between about 0 and 2.5
  double levels[2] = { 1.0, 1.2 };
  displayNode->SetContourLevels(levels, 2);
}

//----------------------------------------------------------------------------
int TestVectorVolume()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(Field));
  SetUpTestField(scene, displayNode);
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Vector volume", 4));

  // Every 3D view draws from the filters of the display node, also under a non-linear
  // transform, which the display node applies to the points rather than each view
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    displayNode->SetVisualizationMode(mode);
    views.Render();
    CHECK_EXIT_SUCCESS(CheckShared3DFilters(views, std::string("Vector volume, ") + displayNode->GetVisualizationModeAsString()));
  }
  {
    // Not linear, but it moves everything by the same offset
    volumeNode->SetAndObserveTransformNodeID(AddNonLinearTranslation(scene, 200.0)->GetID());
    // The whole volume, wherever it is moved: a region node stays where it is in the world
    vtkMRMLNode* samplingRegionNode = displayNode->GetSamplingRegionNode();
    displayNode->SetAndObserveSamplingRegionNode(nullptr);
    for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
    {
      displayNode->SetVisualizationMode(mode);
      views.Render();
      CHECK_EXIT_SUCCESS(CheckShared3DFilters(views, std::string("Vector volume under a non-linear transform, ") + displayNode->GetVisualizationModeAsString()));
      // Moved into the world by the display node, not by the actor
      vtkActor* actor = GetVisibleActor(views.Renderer3D);
      CHECK_BOOL(!actor->GetUserMatrix() || actor->GetUserMatrix()->IsIdentity(), true);
      vtkDataSet* drawn = actor->GetMapper()->GetInput();
      double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
      drawn->GetBounds(bounds);
      CHECK_BOOL(bounds[0] > 100.0, true);
    }
    volumeNode->SetAndObserveTransformNodeID(nullptr);
    displayNode->SetAndObserveSamplingRegionNode(samplingRegionNode);
  }

  // Colored by one component of the vectors: the 3D view maps that component of the sampled
  // vectors, the slice view colors the glyphs by it as well
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetScalarVisibility(true);
  displayNode->SetActiveScalarComponent(1);
  views.Render();
  {
    vtkActor* actor = GetVisibleActor(views.Renderer3D);
    CHECK_NOT_NULL(actor);
    vtkMapper* mapper = actor->GetMapper();
    CHECK_STRING(mapper->GetArrayName(), vtkVectorFieldSampler::GetVectorArrayName());
    CHECK_INT(mapper->GetArrayComponent(), 1);
    CHECK_NOT_NULL(mapper->GetLookupTable());
    CHECK_INT(mapper->GetLookupTable()->GetVectorMode(), vtkScalarsToColors::COMPONENT);
    CHECK_INT(mapper->GetLookupTable()->GetVectorComponent(), 1);
    CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > 0, true);
    // Back to the magnitude
    displayNode->SetActiveScalarComponent(-1);
    views.Render();
    actor = GetVisibleActor(views.Renderer3D);
    CHECK_STRING(actor->GetMapper()->GetArrayName(), vtkVectorFieldSampler::GetMagnitudeArrayName());
  }

  // Contours are drawn semi-transparent in 3D views, on top of the opacity of the display node
  displayNode->SetOpacity(0.5);
  displayNode->SetContourOpacity3D(0.8);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  views.Render();
  CHECK_DOUBLE_TOLERANCE(GetVisibleActorOpacity(views.Renderer3D), 0.4, 1e-9);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  views.Render();
  CHECK_DOUBLE_TOLERANCE(GetVisibleActorOpacity(views.Renderer3D), 0.5, 1e-9);
  displayNode->SetOpacity(1.0);

  // Slice views show the streamlines that cross the slice, the same ones as the 3D view,
  // fading out as they get farther from the slice
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  displayNode->SetStreamlineSliceFadeDistance(20.0);
  // The test field has no component across the axial slice, so streamlines seeded in it stay in it. A slice tilted by 30
  // degrees around A crosses them at the seed, and they get farther from it as they go.
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLSliceNode"));
  vtkNew<vtkMatrix4x4> axialSliceToRAS;
  axialSliceToRAS->DeepCopy(sliceNode->GetSliceToRAS());
  const double tilt = vtkMath::RadiansFromDegrees(30.0);
  sliceNode->SetSliceToRASByNTP(std::sin(tilt), 0.0, std::cos(tilt), std::cos(tilt), 0.0, -std::sin(tilt), 0.0, 0.0, 0.0, 0);
  sliceNode->UpdateMatrices();
  views.Render();
  double alphaRange[2] = { 0.0, -1.0 };
  CHECK_BOOL(GetSliceColorAlphaRange(views.RendererSlice, alphaRange), true);
  std::cout << "Streamline alpha in the slice view: " << alphaRange[0] << " to " << alphaRange[1] << std::endl;
  CHECK_BOOL(alphaRange[1] > 200.0, true); // nearly opaque where they cross the slice
  CHECK_BOOL(alphaRange[0] < 100.0, true); // faint farther away
  // Without fading, only what is within the slice is shown
  displayNode->SetStreamlineSliceFadeDistance(0.0);
  views.Render();
  CHECK_BOOL(GetSliceColorAlphaRange(views.RendererSlice, alphaRange), true);
  CHECK_DOUBLE(alphaRange[0], 0.0);
  CHECK_BOOL(alphaRange[1] > 200.0, true);
  displayNode->SetStreamlineSliceFadeDistance(20.0);
  sliceNode->GetSliceToRAS()->DeepCopy(axialSliceToRAS);
  sliceNode->UpdateMatrices();

  // Under a transform that moves the volume far from the slice, the slice shows nothing in any mode, the 3D view still
  // shows it. The volume itself is sampled: the seed points stay where they are in the world, away from the moved volume.
  vtkMRMLNode* samplingRegionNode = displayNode->GetSamplingRegionNode();
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  vtkMRMLLinearTransformNode* transformNode = AddTranslation(scene, 0.0, 0.0, 300.0);
  volumeNode->SetAndObserveTransformNodeID(transformNode->GetID());
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    displayNode->SetVisualizationMode(mode);
    views.Render();
    std::cout << "Vector volume moved away from the slice, " << displayNode->GetVisualizationModeAsString() << ": " //
              << CountDrawnPoints(views.Renderer3D) << " points in 3D, " << CountDrawnPoints(views.RendererSlice) << " in the slice" << std::endl;
    CHECK_INT(CountDrawnPoints(views.RendererSlice), 0);
    CHECK_BOOL(CountDrawnPoints(views.Renderer3D) > 0, true);
  }
  // Moved so that the slice crosses it again, it is drawn there again
  vtkNew<vtkTransform> nearby;
  nearby->Translate(0.0, 0.0, 20.0);
  transformNode->SetMatrixTransformToParent(nearby->GetMatrix());
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  views.Render();
  CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > 0, true);
  volumeNode->SetAndObserveTransformNodeID(nullptr);
  displayNode->SetAndObserveSamplingRegionNode(samplingRegionNode);

  // Random volume samples are not used in slice views (few would be within the slice thickness):
  // the slice is sampled in its plane, as with the regular pattern
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  displayNode->SetSamplingSpacing(2.0);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  views.Render();
  vtkIdType regularGlyphsSlice = CountDrawnPoints(views.RendererSlice);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  views.Render();
  vtkIdType randomGlyphsSlice = CountDrawnPoints(views.RendererSlice);
  std::cout << "Vector volume glyphs in the slice: " << regularGlyphsSlice << " regular, " << randomGlyphsSlice << " random" << std::endl;
  CHECK_BOOL(regularGlyphsSlice > 0, true);
  CHECK_INT(randomGlyphsSlice, regularGlyphsSlice);
  // The 3D view does show the random samples
  CHECK_BOOL(CountDrawnPoints(views.Renderer3D) > 0, true);
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestVolumetricMesh()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "Field", Field);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  SetUpTestField(scene, displayNode);
  displayNode->SetFieldArrayName("Field");
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Volumetric mesh", 4));

  // Colored by other arrays of the mesh: a vector array of constant length 3, and a scalar
  // between 0 and 100 (the mesh is 100 mm wide)
  vtkNew<vtkDoubleArray> other;
  other->SetName("Other");
  other->SetNumberOfComponents(3);
  other->SetNumberOfTuples(mesh->GetNumberOfPoints());
  vtkNew<vtkDoubleArray> pressure;
  pressure->SetName("Pressure");
  pressure->SetNumberOfTuples(mesh->GetNumberOfPoints());
  for (vtkIdType pointIndex = 0; pointIndex < mesh->GetNumberOfPoints(); ++pointIndex)
  {
    other->SetTuple3(pointIndex, 0.0, 0.0, 3.0);
    pressure->SetValue(pointIndex, 50.0 + mesh->GetPoint(pointIndex)[0]);
  }
  mesh->GetPointData()->AddArray(other);
  mesh->GetPointData()->AddArray(pressure);
  mesh->Modified();
  double coloredRange[2] = { 0.0, -1.0 };
  // Isolines in a slice view, cut from the cells of the mesh, are colored by the magnitude of
  // the vector array they are colored by, not by the magnitude of the field they show
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  displayNode->SetActiveScalar("Other", vtkAssignAttribute::POINT_DATA);
  views.Render();
  CHECK_BOOL(GetColoredValueRange(views.RendererSlice, coloredRange), true);
  CHECK_DOUBLE_TOLERANCE(coloredRange[0], 3.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(coloredRange[1], 3.0, 1e-9);
  CHECK_BOOL(GetColoredValueRange(views.Renderer3D, coloredRange), true);
  CHECK_DOUBLE_TOLERANCE(coloredRange[0], 3.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(coloredRange[1], 3.0, 1e-9);
  // A sampled grid carries the scalar the field is colored by, in 3D and in slice views
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  displayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  views.Render();
  for (vtkRenderer* renderer : { views.Renderer3D.GetPointer(), views.RendererSlice.GetPointer() })
  {
    CHECK_BOOL(GetColoredValueRange(renderer, coloredRange), true);
    std::cout << "Grid colored by pressure: " << coloredRange[0] << " to " << coloredRange[1] << std::endl;
    // The grid spans the mesh, so the values span most of the range of the array
    CHECK_BOOL(coloredRange[0] >= 0.0 && coloredRange[0] < 20.0, true);
    CHECK_BOOL(coloredRange[1] <= 100.0 && coloredRange[1] > 80.0, true);
  }
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestSurfaceMesh()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", Field);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  SetUpTestField(scene, displayNode);
  displayNode->SetFieldArrayName("Field");
  // On the points of the mesh: glyphs only, and in the slice view the points that the slice
  // holds
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Surface mesh", 1));

  // Interpolated between the points: every mode, and glyphs at the points of another node
  displayNode->SetInterpolateBetweenPoints(true);
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Surface mesh, interpolated", 4));

  // Sampled inside the mesh, the slice view samples a lattice in the plane of the slice, so
  // there are more glyphs than the mesh has points in the slice
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetSamplingSpacing(2.0);
  views.Render();
  vtkIdType interpolatedGlyphs = CountDrawnPoints(views.RendererSlice);
  displayNode->SetInterpolateBetweenPoints(false);
  views.Render();
  vtkIdType meshPointGlyphs = CountDrawnPoints(views.RendererSlice);
  std::cout << "Glyphs in the slice: " << interpolatedGlyphs << " interpolated, " << meshPointGlyphs << " on mesh points" << std::endl;
  CHECK_BOOL(interpolatedGlyphs > meshPointGlyphs, true);
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Glyphs that are oriented by one array and scaled by another are scaled by it in slice
/// views too, as in 3D views
int TestSliceGlyphScaleArray()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  // One point in the slice, pointing along R, sized by an array of its own that points along A
  vtkNew<vtkPolyData> pointCloud;
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(20.0, 0.0, 0.0);
  pointCloud->SetPoints(points);
  vtkNew<vtkCellArray> vertices;
  vtkIdType pointId = 0;
  vertices->InsertNextCell(1, &pointId);
  pointCloud->SetVerts(vertices);
  vtkNew<vtkDoubleArray> direction;
  direction->SetName("Direction");
  direction->SetNumberOfComponents(3);
  direction->InsertNextTuple3(1.0, 0.0, 0.0);
  pointCloud->GetPointData()->AddArray(direction);
  vtkNew<vtkDoubleArray> size;
  size->SetName("Size");
  size->SetNumberOfComponents(3);
  size->InsertNextTuple3(0.0, 2.0, 0.0);
  pointCloud->GetPointData()->AddArray(size);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, pointCloud);
  SetUpTestField(scene, displayNode);
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  displayNode->SetFieldArrayName("Direction");
  displayNode->SetGlyphScaleArrayName("Size");
  displayNode->SetGlyphScaleFactor(1.0);

  // The length of the glyph along R, in slice coordinates, for a size of 2 and of 8
  double glyphLength_XY[2] = { 0.0, 0.0 };
  const double sizes[2] = { 2.0, 8.0 };
  for (int index = 0; index < 2; ++index)
  {
    size->SetTuple3(0, 0.0, sizes[index], 0.0);
    size->Modified();
    pointCloud->Modified();
    views.Render();
    vtkPolyData* glyphs = GetVisibleSliceGeometry(views.RendererSlice);
    CHECK_NOT_NULL(glyphs);
    CHECK_BOOL(glyphs->GetNumberOfPoints() > 0, true);
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    glyphs->GetBounds(bounds);
    glyphLength_XY[index] = bounds[1] - bounds[0];
  }
  std::cout << "Slice glyph lengths for sizes 2 and 8: " << glyphLength_XY[0] << " and " << glyphLength_XY[1] << std::endl;
  CHECK_DOUBLE_TOLERANCE(glyphLength_XY[1] / glyphLength_XY[0], 4.0, 0.01);
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestCellData()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddCellField(surface, "CellField", Field);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  SetUpTestField(scene, displayNode);
  displayNode->SetFieldArrayName("CellField");
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Cell data", 1));
  // One glyph per cell
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  views.Render();
  CHECK_INT(CountDrawnPoints(views.Renderer3D), surface->GetNumberOfCells());

  // A part that cannot be sampled for cell data falls back to one that can, instead of
  // drawing nothing
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  views.Render();
  CHECK_BOOL(CountDrawnPoints(views.Renderer3D) > 0, true);
  CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > 0, true);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  displayNode->SetInterpolateBetweenPoints(true);
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Cell data, interpolated", 4));
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestTransform()
{
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  // A rotation and a translation, so that the displacement varies from place to place and
  // has contours
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkNew<vtkTransform> transform;
  transform->Translate(3.0, 0.0, 0.0);
  transform->RotateZ(10.0);
  transformNode->SetMatrixTransformToParent(transform->GetMatrix());
  transformNode->CreateDefaultDisplayNodes();
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  CHECK_NOT_NULL(displayNode);
  displayNode->SetVisibility(true);
  displayNode->SetSamplingSpacing(10.0);
  displayNode->SetGridSpacing(20.0);

  // Without a region a transform is drawn in slice views, wherever the slice is, but not in
  // 3D views: it is defined everywhere, so the 3D view has to be told where to draw it
  for (int mode = 0; mode < vtkMRMLTransformDisplayNode::VIS_MODE_LAST; ++mode)
  {
    displayNode->SetVisualizationMode(mode);
    views.Render();
    vtkIdType drawnPointsSlice = CountDrawnPoints(views.RendererSlice);
    std::cout << "Transform without region, " << displayNode->GetVisualizationModeAsString() << ": " << drawnPointsSlice << " points in the slice" << std::endl;
    CHECK_BOOL(drawnPointsSlice > 0, true);
    CHECK_INT(CountDrawnPoints(views.Renderer3D), 0);
  }

  // With a region, in both views
  vtkMRMLMarkupsROINode* roiNode = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(50.0, 50.0, 50.0));
  displayNode->SetAndObserveSamplingRegionNode(roiNode);
  CHECK_EXIT_SUCCESS(CheckDrawnInModes(views, displayNode, "Transform with region", vtkMRMLTransformDisplayNode::VIS_MODE_LAST));

  // A transform is evaluated in the world, with its parent transforms, so a 3D view does not
  // move it by its parent transform again
  {
    transformNode->SetAndObserveTransformNodeID(AddTranslation(scene, 0.0, 0.0, 300.0)->GetID());
    displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH);
    views.Render();
    vtkActor* actor = GetVisibleActor(views.Renderer3D);
    CHECK_NOT_NULL(actor);
    CHECK_BOOL(!actor->GetUserMatrix() || actor->GetUserMatrix()->IsIdentity(), true);
    transformNode->SetAndObserveTransformNodeID(nullptr);
  }

  // Glyphs at the points of a node. A grid and contours need an inside or a plane to be built
  // in, so in 3D views they have nowhere to be drawn; slice views draw them in their plane.
  vtkMRMLMarkupsFiducialNode* pointsNode = AddPointList(scene, { vtkVector3d(20.0, 0.0, 0.0), vtkVector3d(-20.0, 10.0, 0.0), vtkVector3d(0.0, 30.0, 0.0) });
  displayNode->SetAndObserveSamplingRegionNode(pointsNode);
  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH);
  views.Render();
  CHECK_INT(CountDrawnPoints(views.Renderer3D), 3);
  // A slice view draws the outline of each glyph, a few points per glyph
  vtkIdType drawnPointsSlice = CountDrawnPoints(views.RendererSlice);
  CHECK_BOOL(drawnPointsSlice > 0 && drawnPointsSlice <= 3 * 10, true);

  // A negative scale factor points the glyph the other way. The slice view draws the glyph geometry itself, in slice XY
  // coordinates (the 3D glyph mapper only reports bounds that hold the glyph in any direction).
  {
    displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(20.0, 0.0, 0.0) }));
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLSliceNode"));
    CHECK_NOT_NULL(sliceNode);
    vtkNew<vtkMatrix4x4> rasToXY;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToXY);
    const double point_RAS[4] = { 20.0, 0.0, 0.0, 1.0 };
    double point_XY[4] = { 0.0, 0.0, 0.0, 1.0 };
    rasToXY->MultiplyPoint(point_RAS, point_XY);
    double glyphCenter_XY[2][2] = { { 0.0, 0.0 }, { 0.0, 0.0 } };
    const double scaleFactors[2] = { 5.0, -5.0 };
    for (int index = 0; index < 2; ++index)
    {
      displayNode->SetGlyphScaleFactor(scaleFactors[index]);
      views.Render();
      CHECK_INT(CountDrawnPoints(views.Renderer3D), 1);
      vtkPolyData* glyphs = GetVisibleSliceGeometry(views.RendererSlice);
      CHECK_NOT_NULL(glyphs);
      CHECK_BOOL(glyphs->GetNumberOfPoints() > 0, true);
      double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
      glyphs->GetBounds(bounds);
      glyphCenter_XY[index][0] = 0.5 * (bounds[0] + bounds[1]);
      glyphCenter_XY[index][1] = 0.5 * (bounds[2] + bounds[3]);
    }
    std::cout << "Glyph centers in the slice with a scale factor of 5 and -5: " << glyphCenter_XY[0][0] << ", " << glyphCenter_XY[0][1] << " and " << glyphCenter_XY[1][0] << ", "
              << glyphCenter_XY[1][1] << "; the point is at " << point_XY[0] << ", " << point_XY[1] << std::endl;
    // On either side of the point, as far from it
    CHECK_BOOL(std::abs(glyphCenter_XY[0][0] - point_XY[0]) + std::abs(glyphCenter_XY[0][1] - point_XY[1]) > 1.0, true);
    for (int axis = 0; axis < 2; ++axis)
    {
      CHECK_DOUBLE_TOLERANCE(glyphCenter_XY[0][axis] + glyphCenter_XY[1][axis], 2.0 * point_XY[axis], 0.5);
    }
    displayNode->SetGlyphScaleFactor(1.0);
    displayNode->SetAndObserveSamplingRegionNode(pointsNode);
  }

  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
  views.Render();
  CHECK_INT(CountDrawnPoints(views.Renderer3D), 0);
  CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > 0, true);

  // Masked by a region of interest: only what is inside it is drawn, in both views
  displayNode->SetAndObserveSamplingRegionNode(roiNode);
  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH);
  views.Render();
  vtkIdType unmaskedGlyphs3D = CountDrawnPoints(views.Renderer3D);
  vtkIdType unmaskedSlice = CountDrawnPoints(views.RendererSlice);
  vtkMRMLMarkupsROINode* maskNode = AddROI(scene, vtkVector3d(25.0, 0.0, 0.0), vtkVector3d(25.0, 50.0, 50.0));
  displayNode->SetAndObserveMaskingRegionNode(maskNode);
  views.Render();
  vtkIdType maskedGlyphs3D = CountDrawnPoints(views.Renderer3D);
  vtkIdType maskedSlice = CountDrawnPoints(views.RendererSlice);
  std::cout << "Transform glyphs masked to half of the region: " << maskedGlyphs3D << " of " << unmaskedGlyphs3D << " in 3D, " << maskedSlice << " of " << unmaskedSlice
            << " points in the slice" << std::endl;
  CHECK_BOOL(maskedGlyphs3D > 0 && maskedGlyphs3D < unmaskedGlyphs3D, true);
  CHECK_BOOL(maskedSlice > 0 && maskedSlice < unmaskedSlice, true);
  // Grid lines are cut at the edge of the mask
  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
  displayNode->SetAndObserveMaskingRegionNode(nullptr);
  views.Render();
  vtkIdType unmaskedGrid3D = CountDrawnPoints(views.Renderer3D);
  displayNode->SetAndObserveMaskingRegionNode(maskNode);
  views.Render();
  vtkIdType maskedGrid3D = CountDrawnPoints(views.Renderer3D);
  CHECK_BOOL(maskedGrid3D > 0 && maskedGrid3D < unmaskedGrid3D, true);
  // A grid deformed the opposite way is drawn as well
  displayNode->SetGridScaleFactor(-1.0);
  views.Render();
  CHECK_BOOL(CountDrawnPoints(views.Renderer3D) > 0, true);
  CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > 0, true);
  displayNode->SetGridScaleFactor(1.0);
  // The undeformed grid is one more actor in both views, in a color of its own however the field is colored
  displayNode->SetScalarVisibility(true);
  size_t visibleProps3D = GetVisibleProps(views.Renderer3D).size();
  size_t visiblePropsSlice = GetVisibleProps(views.RendererSlice).size();
  vtkIdType gridPoints3D = CountDrawnPoints(views.Renderer3D);
  vtkIdType gridPointsSlice = CountDrawnPoints(views.RendererSlice);
  displayNode->SetGridNonWarpedColor(0.125, 0.25, 0.5);
  displayNode->SetGridShowNonWarped(true);
  views.Render();
  CHECK_INT(static_cast<int>(GetVisibleProps(views.Renderer3D).size()), static_cast<int>(visibleProps3D + 1));
  CHECK_INT(static_cast<int>(GetVisibleProps(views.RendererSlice).size()), static_cast<int>(visiblePropsSlice + 1));
  CHECK_BOOL(HasVisibleSolidActor(views.Renderer3D, 0.125), true);
  CHECK_BOOL(HasVisibleSolidActor(views.RendererSlice, 0.125), true);
  CHECK_BOOL(CountDrawnPoints(views.Renderer3D) > gridPoints3D, true);
  CHECK_BOOL(CountDrawnPoints(views.RendererSlice) > gridPointsSlice, true);
  // It follows the color, and goes away with the option and in other modes
  displayNode->SetGridNonWarpedColor(0.75, 0.25, 0.5);
  views.Render();
  CHECK_BOOL(HasVisibleSolidActor(views.Renderer3D, 0.75), true);
  CHECK_BOOL(HasVisibleSolidActor(views.RendererSlice, 0.75), true);
  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH);
  views.Render();
  CHECK_BOOL(HasVisibleSolidActor(views.Renderer3D, 0.75), false);
  CHECK_BOOL(HasVisibleSolidActor(views.RendererSlice, 0.75), false);
  displayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
  displayNode->SetGridShowNonWarped(false);
  views.Render();
  CHECK_INT(static_cast<int>(GetVisibleProps(views.Renderer3D).size()), static_cast<int>(visibleProps3D));
  CHECK_INT(static_cast<int>(GetVisibleProps(views.RendererSlice).size()), static_cast<int>(visiblePropsSlice));
  views.TearDown();
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
/// A folder that applies its display properties to its branch colors the field as it colors
/// the models in it: solid color and opacity, in 3D views
int TestSliceReparent()
{
  // A volume moved from one parent transform to another: the slice view samples it where it is now
  vtkNew<vtkMRMLScene> scene;
  Views views;
  views.Setup(scene);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(Field));
  displayNode->SetSamplingSpacing(10.0);
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
  auto sliceGlyphCenterX = [&views]()
  {
    views.Render();
    vtkPolyData* glyphs = GetVisibleSliceGeometry(views.RendererSlice);
    if (!glyphs || glyphs->GetNumberOfPoints() == 0)
    {
      return 0.0;
    }
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    glyphs->GetBounds(bounds);
    return 0.5 * (bounds[0] + bounds[1]);
  };
  volumeNode->SetAndObserveTransformNodeID(AddTranslation(scene, 60.0, 0.0, 0.0)->GetID());
  double centerRight = sliceGlyphCenterX();
  volumeNode->SetAndObserveTransformNodeID(AddTranslation(scene, -60.0, 0.0, 0.0)->GetID());
  double centerLeft = sliceGlyphCenterX();
  // Slice geometry is in pixels: the 300 pixel wide view shows 150 mm, x = 0 mm is at pixel 150
  std::cout << "Slice glyph center x (pixels) under +60 mm: " << centerRight << ", under -60 mm: " << centerLeft << std::endl;
  CHECK_BOOL(centerRight > 180.0, true);
  CHECK_BOOL(centerLeft < 120.0, true);
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestFolderDisplayOverride()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  CHECK_NOT_NULL(shNode);
  Views views;
  views.Setup(scene);
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "Field", Field);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  SetUpTestField(scene, displayNode);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
  displayNode->SetFieldArrayName("Field");
  displayNode->SetScalarVisibility(false);
  displayNode->SetColor(0.0, 1.0, 0.0);
  views.Render();
  double color[3] = { 0.0, 0.0, 0.0 };
  CHECK_BOOL(GetVisibleActorColor(views.Renderer3D, color), true);
  CHECK_DOUBLE(color[0], 0.0);
  CHECK_DOUBLE(color[1], 1.0);
  CHECK_DOUBLE(color[2], 0.0);
  CHECK_DOUBLE(GetVisibleActorOpacity(views.Renderer3D), 1.0);

  vtkIdType folderItemID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "Folder");
  vtkMRMLFolderDisplayNode* folderDisplayNode = vtkMRMLFolderDisplayNode::AddDisplayNodeForItem(shNode, folderItemID);
  CHECK_NOT_NULL(folderDisplayNode);
  folderDisplayNode->SetColor(1.0, 0.0, 0.0);
  folderDisplayNode->SetOpacity(0.5);
  folderDisplayNode->SetApplyDisplayPropertiesOnBranch(true);
  // The application creates the items of data nodes; without it, the model gets one here
  shNode->SetItemParent(shNode->CreateItem(shNode->GetSceneItemID(), modelNode), folderItemID);
  // The views pick the folder up when the display node is updated next
  displayNode->Modified();
  views.Render();
  CHECK_BOOL(GetVisibleActorColor(views.Renderer3D, color), true);
  CHECK_DOUBLE(color[0], 1.0);
  CHECK_DOUBLE(color[1], 0.0);
  CHECK_DOUBLE(color[2], 0.0);
  CHECK_BOOL(GetVisibleActorOpacity(views.Renderer3D) < 1.0, true);
  views.TearDown();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldDisplayableManagerTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(TestVectorVolume());
  CHECK_EXIT_SUCCESS(TestVolumetricMesh());
  CHECK_EXIT_SUCCESS(TestSurfaceMesh());
  CHECK_EXIT_SUCCESS(TestSliceGlyphScaleArray());
  CHECK_EXIT_SUCCESS(TestCellData());
  CHECK_EXIT_SUCCESS(TestTransform());
  CHECK_EXIT_SUCCESS(TestFolderDisplayOverride());
  CHECK_EXIT_SUCCESS(TestSliceReparent());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
