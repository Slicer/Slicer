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

// Checks that the transform visualizations of a scene saved by an earlier Slicer version are shown the same way by
// vector field display nodes when the scene is loaded.

// VectorFieldRendering includes
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <string>
#include <vector>

namespace
{

//----------------------------------------------------------------------------
/// A scene as an earlier Slicer version saved it: a transform shown as a grid in a region, one shown as glyphs at points
/// but hidden, and one whose visualization was never used
const char LegacyScene[] = //
  "<MRML version=\"Slicer4.4.0\" userTags=\"\">\n"
  " <MarkupsROI id=\"vtkMRMLMarkupsROINode1\" name=\"Region\" />\n"
  " <MarkupsFiducial id=\"vtkMRMLMarkupsFiducialNode1\" name=\"GlyphPoints\" />\n"
  " <ProceduralColor id=\"vtkMRMLProceduralColorNode1\" name=\"Displacement to color\" />\n"
  " <Transform id=\"vtkMRMLTransformNode1\" name=\"Shown\" displayNodeRef=\"vtkMRMLTransformDisplayNode1\" />\n"
  " <TransformDisplayNode id=\"vtkMRMLTransformDisplayNode1\" name=\"ShownDisplay\" visibility=\"true\" visibility2D=\"true\""
  " visibility3D=\"false\" colorNodeID=\"vtkMRMLProceduralColorNode1\" region=\"vtkMRMLMarkupsROINode1\""
  " VisualizationMode=\"GRID\" GlyphSpacingMm=\"12\" GridResolutionMm=\"3\" ContourResolutionMm=\"4\" GridScalePercent=\"200\""
  " GridSpacingMm=\"18\" GridLineDiameterMm=\"1.5\" GridShowNonWarped=\"1\" GlyphScalePercent=\"150\" GlyphDisplayRangeMinMm=\"0.5\""
  " GlyphDisplayRangeMaxMm=\"80\" GlyphType=\"CONE\" ContourLevelsMm=\"1 3 5\" ContourOpacity=\"0.6\" EditorVisibility=\"1\" />\n"
  " <Transform id=\"vtkMRMLTransformNode2\" name=\"Points\" displayNodeRef=\"vtkMRMLTransformDisplayNode2\" />\n"
  " <TransformDisplayNode id=\"vtkMRMLTransformDisplayNode2\" name=\"PointsDisplay\" visibility=\"false\""
  " region=\"vtkMRMLMarkupsROINode1\" glyphPoints=\"vtkMRMLMarkupsFiducialNode1\" VisualizationMode=\"GLYPH\" GlyphSpacingMm=\"7\" />\n"
  " <Transform id=\"vtkMRMLTransformNode3\" name=\"Unused\" displayNodeRef=\"vtkMRMLTransformDisplayNode3\" />\n"
  " <TransformDisplayNode id=\"vtkMRMLTransformDisplayNode3\" name=\"UnusedDisplay\" visibility=\"false\""
  " VisualizationMode=\"GLYPH\" GlyphSpacingMm=\"10\" />\n"
  "</MRML>\n";

//----------------------------------------------------------------------------
bool ImportScene(vtkMRMLScene* scene, const std::string& sceneXML)
{
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  return (scene->Import() != 0);
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* GetTransformNode(vtkMRMLScene* scene, const char* name)
{
  return vtkMRMLTransformNode::SafeDownCast(scene->GetFirstNodeByName(name));
}

//----------------------------------------------------------------------------
int CheckConvertedScene(vtkMRMLScene* scene)
{
  // Shown as a grid in the region
  vtkMRMLTransformNode* shownNode = GetTransformNode(scene, "Shown");
  CHECK_NOT_NULL(shownNode);
  std::vector<vtkMRMLVectorFieldDisplayNode*> shownDisplayNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(shownNode);
  CHECK_INT(static_cast<int>(shownDisplayNodes.size()), 1);
  vtkMRMLVectorFieldDisplayNode* gridNode = shownDisplayNodes[0];
  CHECK_INT(gridNode->GetVisualizationMode(), vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(gridNode->GetVisibility() != 0, true);
  CHECK_BOOL(gridNode->GetVisibility2D() != 0, true);
  CHECK_BOOL(gridNode->GetVisibility3D() != 0, false);
  CHECK_INT(gridNode->GetShowMode(), vtkMRMLDisplayNode::ShowDefault);
  // The spacing of the mode it was shown in
  CHECK_DOUBLE(gridNode->GetSamplingSpacing(), 3.0);
  CHECK_DOUBLE(gridNode->GetGridScaleFactor(), 2.0);
  CHECK_DOUBLE(gridNode->GetGridSpacing(), 18.0);
  CHECK_DOUBLE(gridNode->GetGridLineDiameter(), 1.5);
  CHECK_BOOL(gridNode->GetGridShowNonWarped(), true);
  CHECK_DOUBLE(gridNode->GetGlyphScaleFactor(), 1.5);
  CHECK_INT(gridNode->GetGlyphType(), vtkMRMLVectorFieldDisplayNode::GlyphTypeCone);
  CHECK_BOOL(gridNode->GetThresholdEnabled(), true);
  CHECK_DOUBLE(gridNode->GetThresholdRange()[0], 0.5);
  CHECK_DOUBLE(gridNode->GetThresholdRange()[1], 80.0);
  CHECK_DOUBLE(gridNode->GetContourOpacity3D(), 0.6);
  CHECK_STD_STRING(gridNode->GetContourLevelsAsString(), "1 3 5");
  CHECK_NOT_NULL(vtkMRMLMarkupsROINode::SafeDownCast(gridNode->GetSamplingRegionNode()));
  // Colored by the same color map
  CHECK_NOT_NULL(vtkMRMLProceduralColorNode::SafeDownCast(gridNode->GetColorNode()));
  CHECK_STD_STRING(std::string(gridNode->GetColorNode()->GetID()), "vtkMRMLProceduralColorNode1");
  // The transform display node only has its interaction settings left
  vtkMRMLTransformDisplayNode* shownTransformDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(shownNode->GetDisplayNode());
  CHECK_NOT_NULL(shownTransformDisplayNode);
  CHECK_BOOL(shownTransformDisplayNode->HasLegacyVisualization(), false);
  CHECK_BOOL(shownTransformDisplayNode->GetEditorVisibility(), true);

  // Glyphs at the points, hidden as they were
  vtkMRMLTransformNode* pointsNode = GetTransformNode(scene, "Points");
  std::vector<vtkMRMLVectorFieldDisplayNode*> pointsDisplayNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(pointsNode);
  CHECK_INT(static_cast<int>(pointsDisplayNodes.size()), 1);
  CHECK_INT(pointsDisplayNodes[0]->GetVisualizationMode(), vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  CHECK_BOOL(pointsDisplayNodes[0]->GetVisibility() != 0, false);
  CHECK_DOUBLE(pointsDisplayNodes[0]->GetSamplingSpacing(), 7.0);
  CHECK_NOT_NULL(vtkMRMLMarkupsFiducialNode::SafeDownCast(pointsDisplayNodes[0]->GetSamplingRegionNode()));
  // It had no color map, so it gets the default one, as it did when it was shown
  CHECK_NOT_NULL(vtkMRMLProceduralColorNode::SafeDownCast(pointsDisplayNodes[0]->GetColorNode()));
  CHECK_BOOL(pointsDisplayNodes[0]->GetColorNode() != gridNode->GetColorNode(), true);
  // No other color map is added
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLProceduralColorNode"), 2);

  // Never used: nothing is added, and the settings are removed
  vtkMRMLTransformNode* unusedNode = GetTransformNode(scene, "Unused");
  CHECK_INT(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(unusedNode).size()), 0);
  CHECK_BOOL(vtkMRMLTransformDisplayNode::SafeDownCast(unusedNode->GetDisplayNode())->HasLegacyVisualization(), false);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkSlicerVectorFieldRenderingLogicLegacySceneTest1(int, char*[])
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVectorFieldRenderingLogic> logic;
  logic->SetMRMLScene(scene);
  CHECK_BOOL(ImportScene(scene, LegacyScene), true);
  CHECK_EXIT_SUCCESS(CheckConvertedScene(scene));

  // Saved and loaded again: the vector field display nodes are loaded, nothing is converted again
  scene->SetSaveToXMLString(1);
  CHECK_BOOL(scene->Commit() != 0, true);
  std::string savedScene = scene->GetSceneXMLString();
  CHECK_BOOL(savedScene.find("VisualizationMode") == std::string::npos, true);
  vtkNew<vtkMRMLScene> reloadedScene;
  vtkNew<vtkSlicerVectorFieldRenderingLogic> reloadedLogic;
  reloadedLogic->SetMRMLScene(reloadedScene);
  CHECK_BOOL(ImportScene(reloadedScene, savedScene), true);
  CHECK_EXIT_SUCCESS(CheckConvertedScene(reloadedScene));

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
