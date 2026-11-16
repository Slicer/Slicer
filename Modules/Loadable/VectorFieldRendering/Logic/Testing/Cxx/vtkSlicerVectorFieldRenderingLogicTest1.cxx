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

// VectorFieldRendering includes
#include "vtkMRMLVectorFieldTestingHelpers.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// Colors includes
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
/// Models, vector volumes and transforms have a vector field; other nodes do not
int TestHasVectorField()
{
  vtkNew<vtkMRMLScene> scene;
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(nullptr), false);

  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(surface);
  AddPointScalar(surface, "Scalar", LinearScalar);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(modelNode), false);
  AddCellField(surface, "CellVectors", LinearField);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(modelNode), true);

  vtkMRMLVectorVolumeNode* vectorVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  vectorVolumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(vectorVolumeNode), true);

  vtkMRMLScalarVolumeNode* scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
  vtkNew<vtkImageData> scalarImage;
  scalarImage->SetDimensions(3, 3, 3);
  scalarImage->AllocateScalars(VTK_DOUBLE, 1);
  scalarVolumeNode->SetAndObserveImageData(scalarImage);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(scalarVolumeNode), false);

  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(scene->AddNewNodeByClass("vtkMRMLTransformNode")), true);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::HasVectorField(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")), false);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Vector field display nodes are added after the default display node, with defaults for the kind of node, any number
/// of them
int TestAddDisplayNodes()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVectorFieldRenderingLogic> logic;
  logic->SetMRMLScene(scene);
  CHECK_BOOL(scene->IsNodeClassRegistered("vtkMRMLVectorFieldDisplayNode"), true);

  // Model: colored with the color table of its surface
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  AddPointField(surface, "Velocity", LinearField);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(surface);
  modelNode->CreateDefaultDisplayNodes();
  modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID("SomeColorTable");
  vtkMRMLVectorFieldDisplayNode* modelFieldNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(modelNode);
  CHECK_NOT_NULL(modelFieldNode);
  CHECK_NOT_NULL(modelNode->GetModelDisplayNode());
  CHECK_POINTER(modelNode->GetNthDisplayNode(1), modelFieldNode);
  CHECK_STRING(modelFieldNode->GetColorNodeID(), "SomeColorTable");
  CHECK_BOOL(modelFieldNode->GetVisibility() != 0, true);
  CHECK_INT(modelFieldNode->GetShowMode(), vtkMRMLDisplayNode::ShowDefault);

  // A second display of the same model is a new node
  vtkMRMLVectorFieldDisplayNode* secondModelFieldNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(modelNode);
  CHECK_NOT_NULL(secondModelFieldNode);
  CHECK_BOOL(secondModelFieldNode != modelFieldNode, true);
  CHECK_INT(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(modelNode).size()), 2);

  // Vector volume: its volume display node is created first, so that it is still shown in slice views
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
  vtkMRMLVectorFieldDisplayNode* volumeFieldNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(volumeNode);
  CHECK_NOT_NULL(volumeFieldNode);
  CHECK_NOT_NULL(vtkMRMLVectorVolumeDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(0)));
  CHECK_POINTER(volumeNode->GetNthDisplayNode(1), volumeFieldNode);
  CHECK_STRING(volumeFieldNode->GetColorNodeID(), "vtkMRMLColorTableNodeFilePlasma.txt");
  CHECK_NOT_NULL(vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(volumeNode));
  CHECK_INT(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(volumeNode).size()), 2);

  // Transform: displacement in mm, colored by a color map from displacement to color
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkMRMLVectorFieldDisplayNode* transformFieldNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(transformNode);
  CHECK_NOT_NULL(transformFieldNode);
  CHECK_NOT_NULL(vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetNthDisplayNode(0)));
  CHECK_POINTER(transformNode->GetNthDisplayNode(1), transformFieldNode);
  CHECK_BOOL(transformFieldNode->GetGlyphDiameterAbsolute(), true);
  CHECK_INT(transformFieldNode->GetScalarRangeFlag(), vtkMRMLDisplayNode::UseColorNodeScalarRange);
  vtkMRMLProceduralColorNode* colorNode = vtkMRMLProceduralColorNode::SafeDownCast(transformFieldNode->GetColorNode());
  CHECK_NOT_NULL(colorNode);
  CHECK_STRING(colorNode->GetName(), "Displacement to color");
  CHECK_DOUBLE_TOLERANCE(transformFieldNode->GetScalarRange()[0], 1.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(transformFieldNode->GetScalarRange()[1], 10.0, 1e-6);
  vtkMRMLVectorFieldDisplayNode* secondTransformFieldNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(transformNode);
  CHECK_NOT_NULL(secondTransformFieldNode);
  CHECK_INT(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(transformNode).size()), 2);

  // Nothing is added to a node without a vector field
  CHECK_NULL(vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(vtkMRMLDisplayableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"))));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A shown vector field is hidden and shown with its node by the eye icon; a hidden one stays hidden
int TestShowAndHide()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVectorFieldRenderingLogic> logic;
  logic->SetMRMLScene(scene);
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkMRMLVectorFieldDisplayNode* shownNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(transformNode);
  vtkMRMLVectorFieldDisplayNode* hiddenNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(transformNode);
  vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(hiddenNode, false);
  CHECK_INT(hiddenNode->GetShowMode(), vtkMRMLDisplayNode::ShowIgnore);
  CHECK_BOOL(hiddenNode->GetVisibility() != 0, false);

  // The eye icon sets the visibility of the display nodes that it shows and hides
  transformNode->SetDisplayVisibility(0);
  CHECK_BOOL(shownNode->GetVisibility() != 0, false);
  transformNode->SetDisplayVisibility(1);
  CHECK_BOOL(shownNode->GetVisibility() != 0, true);
  CHECK_BOOL(hiddenNode->GetVisibility() != 0, false);

  // Shown again, it follows the eye icon again
  vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(hiddenNode, true);
  CHECK_BOOL(hiddenNode->GetVisibility() != 0, true);
  transformNode->SetDisplayVisibility(0);
  CHECK_BOOL(hiddenNode->GetVisibility() != 0, false);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Removing a vector field display node removes its color legend, and leaves the other displays of the node
int TestRemove()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVectorFieldRenderingLogic> logic;
  logic->SetMRMLScene(scene);
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLColorLegendDisplayNode>::New());
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
  vtkMRMLVectorFieldDisplayNode* removedNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(volumeNode);
  vtkMRMLVectorFieldDisplayNode* keptNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(volumeNode);
  vtkMRMLColorLegendDisplayNode* removedLegend = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(removedNode);
  vtkMRMLColorLegendDisplayNode* keptLegend = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(keptNode);
  CHECK_NOT_NULL(removedLegend);
  CHECK_NOT_NULL(keptLegend);
  std::string removedNodeID = removedNode->GetID();
  std::string removedLegendID = removedLegend->GetID();

  vtkSlicerVectorFieldRenderingLogic::RemoveVectorFieldDisplayNode(removedNode);
  CHECK_NULL(scene->GetNodeByID(removedNodeID));
  CHECK_NULL(scene->GetNodeByID(removedLegendID));
  CHECK_POINTER(vtkSlicerColorLogic::GetColorLegendDisplayNode(keptNode), keptLegend);
  std::vector<vtkMRMLVectorFieldDisplayNode*> remainingNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(volumeNode);
  CHECK_INT(static_cast<int>(remainingNodes.size()), 1);
  CHECK_POINTER(remainingNodes[0], keptNode);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// What the views would show is exported without any view: the logic sets up the filters for the call, and uses
/// those of the views when they hold them.
int TestVisualizationMesh()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkPolyData> mesh;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  displayNode->SetSamplingSpacing(20.0);
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(nullptr, mesh), false);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(displayNode, nullptr), false);
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  // Nothing holds the filters of the display node
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(displayNode, mesh), true);
  vtkIdType glyphPoints = mesh->GetNumberOfPoints();
  CHECK_BOOL(glyphPoints > 0, true);

  // The filters that a view holds give the same mesh, and are not replaced
  vtkSmartPointer<vtkMRMLVectorFieldPipeline> viewPipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode);
  vtkVectorFieldGlyph* viewGlyph = viewPipeline->GetGlyphFilter();
  vtkNew<vtkPolyData> meshWithView;
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(displayNode, meshWithView), true);
  CHECK_INT(static_cast<int>(meshWithView->GetNumberOfPoints()), static_cast<int>(glyphPoints));
  CHECK_POINTER(vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode), viewPipeline);
  CHECK_POINTER(viewPipeline->GetGlyphFilter(), viewGlyph);

  // Grid mode
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(displayNode, mesh), true);
  // The grid lines are tubes in 3D views
  CHECK_BOOL(mesh->GetNumberOfStrips() > 0, true);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkSlicerVectorFieldRenderingLogicTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestHasVectorField());
  CHECK_EXIT_SUCCESS(TestAddDisplayNodes());
  CHECK_EXIT_SUCCESS(TestShowAndHide());
  CHECK_EXIT_SUCCESS(TestRemove());
  CHECK_EXIT_SUCCESS(TestVisualizationMesh());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
