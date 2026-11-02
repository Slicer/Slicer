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

// Checks that a scene written by the previous transform display node loads with every setting, including the
// interaction handle settings that moved to vtkMRMLTransformInteractionDisplayNode.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformInteractionDisplayNode.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"

// STD includes
#include <string>
#include <vector>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
// Attributes as the previous transform display node wrote them: vtkMRMLWriteXMLBooleanMacro booleans are
// "true"/"false", stream-written ones "1"/"0", and boolean vectors space separated "1"/"0".
const char* LegacyTransformDisplayNodeXML = "<TransformDisplayNode id=\"vtkMRMLTransformDisplayNode1\" name=\"TransformDisplay\""
                                            " visibility=\"true\" visibility2D=\"true\" visibility3D=\"true\""
                                            " VisualizationMode=\"GRID\""
                                            " GlyphSpacingMm=\"7\" GlyphScalePercent=\"150\" GlyphDisplayRangeMaxMm=\"50\" GlyphDisplayRangeMinMm=\"0.5\""
                                            " GlyphType=\"CONE\" GlyphTipLengthPercent=\"25\" GlyphDiameterMm=\"3\" GlyphShaftDiameterPercent=\"35\""
                                            " GlyphResolution=\"8\" GlyphResolution2D=\"10\" GlyphTipLengthPercent2D=\"20\""
                                            " GridScalePercent=\"80\" GridSpacingMm=\"12\" GridLineDiameterMm=\"0.7\" GridResolutionMm=\"4\" GridShowNonWarped=\"1\""
                                            " ContourResolutionMm=\"3\" ContourLevelsMm=\"1 5 10\" ContourOpacity=\"0.6\""
                                            " EditorVisibility=\"1\" EditorSliceIntersectionVisibility=\"1\" EditorTranslationEnabled=\"0\""
                                            " EditorRotationEnabled=\"1\" EditorScalingEnabled=\"0\""
                                            " EditorVisibility3D=\"true\" EditorTranslationSliceAnywhereEnabled=\"true\" EditorTranslationSliceAnywhereSensitivity=\"0.4\""
                                            " InteractionSizeAbsolute=\"1\" InteractionSizeMm=\"8\" InteractionScalePercent=\"20\" InteractionHandleOpacity=\"0.5\""
                                            " TranslationHandleComponentVisibility3D=\"1 0 1 0\" RotationHandleComponentVisibility3D=\"0 1 0 1\""
                                            " ScaleHandleComponentVisibility3D=\"0 0 1 1\" TranslationHandleComponentVisibilitySlice=\"1 1 0 0\""
                                            " RotationHandleComponentVisibilitySlice=\"0 1 1 0\" ScaleHandleComponentVisibilitySlice=\"1 0 0 1\""
                                            "></TransformDisplayNode>";

const char* LegacyTransformNodeXML = "<LinearTransform id=\"vtkMRMLLinearTransformNode1\" name=\"Transform\""
                                     " displayNodeRef=\"vtkMRMLTransformDisplayNode1\""
                                     " matrixTransformToParent=\"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\"></LinearTransform>";

//----------------------------------------------------------------------------
bool CheckVector(const bool actual[4], const bool expected[4], const char* name)
{
  for (int i = 0; i < 4; ++i)
  {
    if (actual[i] != expected[i])
    {
      std::cerr << "Line " << __LINE__ << ": " << name << " component " << i << " is " << actual[i] << ", expected " << expected[i] << std::endl;
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
int TestLegacyScene(bool displayNodeFirst)
{
  std::cout << "Display node " << (displayNodeFirst ? "before" : "after") << " the transform node in the scene file" << std::endl;
  std::string sceneXML = "<MRML version=\"Slicer4.4.0\">";
  sceneXML += (displayNodeFirst ? LegacyTransformDisplayNodeXML : LegacyTransformNodeXML);
  sceneXML += (displayNodeFirst ? LegacyTransformNodeXML : LegacyTransformDisplayNodeXML);
  sceneXML += "</MRML>";

  vtkNew<vtkMRMLScene> scene;
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  CHECK_BOOL(scene->Import() != 0, true);

  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLLinearTransformNode"));
  CHECK_NOT_NULL(transformNode);
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLTransformDisplayNode"));
  CHECK_NOT_NULL(displayNode);
  CHECK_POINTER(displayNode->GetDisplayableNode(), transformNode);

  // Vector field display settings
  CHECK_INT(displayNode->GetVisualizationMode(), vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
  CHECK_INT(displayNode->GetGlyphType(), vtkMRMLVectorFieldDisplayNode::GlyphTypeCone);
  // Each mode had a spacing of its own; the one of the mode the scene was saved in (grid) is
  // what every mode uses now
  CHECK_DOUBLE(displayNode->GetSamplingSpacing(), 4.0);
  CHECK_DOUBLE(displayNode->GetGlyphSpacingMm(), 4.0);
  CHECK_DOUBLE(displayNode->GetGlyphScalePercent(), 150.0);
  CHECK_DOUBLE(displayNode->GetGlyphDisplayRangeMaxMm(), 50.0);
  CHECK_DOUBLE(displayNode->GetGlyphDisplayRangeMinMm(), 0.5);
  CHECK_DOUBLE(displayNode->GetGlyphTipLengthPercent(), 25.0);
  CHECK_DOUBLE(displayNode->GetGlyphDiameter(), 3.0);
  CHECK_DOUBLE(displayNode->GetGlyphShaftDiameterPercent(), 35.0);
  CHECK_INT(displayNode->GetGlyphResolution(), 8);
  CHECK_INT(displayNode->GetGlyphResolution2D(), 10);
  CHECK_DOUBLE(displayNode->GetGlyphTipLengthPercent2D(), 20.0);
  CHECK_DOUBLE(displayNode->GetGridScaleFactor(), 0.8);
  CHECK_DOUBLE(displayNode->GetGridSpacing(), 12.0);
  CHECK_DOUBLE(displayNode->GetGridLineDiameter(), 0.7);
  CHECK_BOOL(displayNode->GetGridShowNonWarped(), true);
  CHECK_STD_STRING(displayNode->GetContourLevelsAsString(), "1 5 10");
  CHECK_DOUBLE(displayNode->GetContourOpacity3D(), 0.6);

  // The accessors that were named for their units still work, for code written against them
  CHECK_DOUBLE(displayNode->GetGlyphDiameterMm(), 3.0);
  CHECK_DOUBLE(displayNode->GetGridScalePercent(), 80.0);
  CHECK_DOUBLE(displayNode->GetGridSpacingMm(), 12.0);
  CHECK_DOUBLE(displayNode->GetGridLineDiameterMm(), 0.7);
  CHECK_DOUBLE(displayNode->GetGridResolutionMm(), 4.0);
  CHECK_DOUBLE(displayNode->GetContourResolutionMm(), 4.0);
  CHECK_STD_STRING(displayNode->GetContourLevelsMmAsString(), "1 5 10");
  displayNode->SetGridScalePercent(50.0);
  CHECK_DOUBLE(displayNode->GetGridScaleFactor(), 0.5);
  displayNode->SetGridScalePercent(80.0);

  // Interactive editing settings, which now belong to the interaction display node
  vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = displayNode->GetInteractionDisplayNode();
  CHECK_NOT_NULL(interactionDisplayNode);
  CHECK_BOOL(displayNode->GetEditorVisibility(), true);
  CHECK_BOOL(displayNode->GetEditorSliceIntersectionVisibility(), true);
  CHECK_BOOL(displayNode->GetEditorTranslationEnabled(), false);
  CHECK_BOOL(displayNode->GetEditorRotationEnabled(), true);
  CHECK_BOOL(displayNode->GetEditorScalingEnabled(), false);
  CHECK_BOOL(displayNode->GetEditorVisibility3D(), true);
  CHECK_BOOL(displayNode->GetEditorTranslationSliceAnywhereEnabled(), true);
  CHECK_DOUBLE(displayNode->GetEditorTranslationSliceAnywhereSensitivity(), 0.4);
  CHECK_BOOL(displayNode->GetInteractionSizeAbsolute(), true);
  CHECK_DOUBLE(displayNode->GetInteractionSizeMm(), 8.0);
  CHECK_DOUBLE(displayNode->GetInteractionScalePercent(), 20.0);
  CHECK_DOUBLE(displayNode->GetInteractionHandleOpacity(), 0.5);

  bool visibility[4] = { false, false, false, false };
  const bool expectedTranslation3D[4] = { true, false, true, false };
  interactionDisplayNode->GetTranslationHandleComponentVisibility3D(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedTranslation3D, "TranslationHandleComponentVisibility3D"), true);
  const bool expectedRotation3D[4] = { false, true, false, true };
  interactionDisplayNode->GetRotationHandleComponentVisibility3D(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedRotation3D, "RotationHandleComponentVisibility3D"), true);
  const bool expectedScale3D[4] = { false, false, true, true };
  interactionDisplayNode->GetScaleHandleComponentVisibility3D(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedScale3D, "ScaleHandleComponentVisibility3D"), true);
  const bool expectedTranslationSlice[4] = { true, true, false, false };
  interactionDisplayNode->GetTranslationHandleComponentVisibilitySlice(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedTranslationSlice, "TranslationHandleComponentVisibilitySlice"), true);
  const bool expectedRotationSlice[4] = { false, true, true, false };
  interactionDisplayNode->GetRotationHandleComponentVisibilitySlice(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedRotationSlice, "RotationHandleComponentVisibilitySlice"), true);
  const bool expectedScaleSlice[4] = { true, false, false, true };
  interactionDisplayNode->GetScaleHandleComponentVisibilitySlice(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedScaleSlice, "ScaleHandleComponentVisibilitySlice"), true);

  // Saving and loading the scene again keeps the settings, now in the new format
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  vtkMRMLTransformDisplayNode* reloadedDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLTransformDisplayNode"));
  CHECK_NOT_NULL(reloadedDisplayNode);
  CHECK_INT(reloadedDisplayNode->GetVisualizationMode(), vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
  CHECK_DOUBLE(reloadedDisplayNode->GetGridSpacing(), 12.0);
  CHECK_DOUBLE(reloadedDisplayNode->GetGridScaleFactor(), 0.8);
  CHECK_BOOL(reloadedDisplayNode->GetEditorVisibility(), true);
  CHECK_BOOL(reloadedDisplayNode->GetEditorVisibility3D(), true);
  CHECK_DOUBLE(reloadedDisplayNode->GetInteractionSizeMm(), 8.0);
  vtkMRMLTransformInteractionDisplayNode* reloadedInteractionDisplayNode = reloadedDisplayNode->GetInteractionDisplayNode();
  CHECK_NOT_NULL(reloadedInteractionDisplayNode);
  reloadedInteractionDisplayNode->GetTranslationHandleComponentVisibility3D(visibility);
  CHECK_BOOL(CheckVector(visibility, expectedTranslation3D, "reloaded TranslationHandleComponentVisibility3D"), true);
  // One interaction display node, not one more for every time the scene is loaded
  CHECK_INT(reloadedScene->GetNumberOfNodesByClass("vtkMRMLTransformInteractionDisplayNode"), 1);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The legacy "region" and "glyphPoints" references both become the sampling region
/// (the glyph points in glyph mode, the region otherwise).
int TestLegacyRegionAndGlyphPoints()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLMarkupsROINode> roiNode;
  scene->AddNode(roiNode);
  vtkNew<vtkMRMLMarkupsFiducialNode> pointsNode;
  scene->AddNode(pointsNode);
  const std::string legacyNodesXML = std::string("<TransformDisplayNode id=\"vtkMRMLTransformDisplayNode1\" name=\"TransformDisplay\"") //
                                     + " region=\"" + roiNode->GetID() + "\" glyphPoints=\"" + pointsNode->GetID() + "\" VisualizationMode=\"";
  for (const char* mode : { "GLYPH", "GRID" })
  {
    vtkNew<vtkMRMLScene> legacyScene;
    legacyScene->SetLoadFromXMLString(1);
    std::string sceneXML = std::string("<MRML version=\"Slicer4.4.0\">") //
                           + "<MarkupsROI id=\"" + roiNode->GetID() + "\" name=\"R\"></MarkupsROI>" + "<MarkupsFiducial id=\"" + pointsNode->GetID()
                           + "\" name=\"F\"></MarkupsFiducial>" //
                           + legacyNodesXML + mode + "\"></TransformDisplayNode>"
                           + "<LinearTransform id=\"vtkMRMLLinearTransformNode1\" name=\"Transform\" displayNodeRef=\"vtkMRMLTransformDisplayNode1\""
                           + " matrixTransformToParent=\"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\"></LinearTransform></MRML>";
    legacyScene->SetSceneXMLString(sceneXML);
    CHECK_BOOL(legacyScene->Import() != 0, true);
    vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(legacyScene->GetFirstNodeByClass("vtkMRMLTransformDisplayNode"));
    CHECK_NOT_NULL(displayNode);
    vtkMRMLNode* samplingRegionNode = displayNode->GetSamplingRegionNode();
    CHECK_NOT_NULL(samplingRegionNode);
    bool glyphMode = (std::string(mode) == "GLYPH");
    CHECK_STRING(samplingRegionNode->GetClassName(), glyphMode ? "vtkMRMLMarkupsFiducialNode" : "vtkMRMLMarkupsROINode");
    // The legacy accessors say the same
    CHECK_POINTER(displayNode->GetRegionNode(), samplingRegionNode);
    if (glyphMode)
    {
      CHECK_POINTER(displayNode->GetGlyphPointsNode(), samplingRegionNode);
    }
    else
    {
      CHECK_NULL(displayNode->GetGlyphPointsNode());
    }
  }

  // Setting the glyph points replaces the region, and clearing them clears it
  vtkNew<vtkMRMLTransformDisplayNode> displayNode;
  scene->AddNode(displayNode);
  displayNode->SetAndObserveRegionNode(roiNode);
  CHECK_NULL(displayNode->GetGlyphPointsNode());
  displayNode->SetAndObserveGlyphPointsNode(pointsNode);
  CHECK_POINTER(displayNode->GetSamplingRegionNode(), pointsNode.GetPointer());
  displayNode->SetAndObserveGlyphPointsNode(nullptr);
  CHECK_NULL(displayNode->GetSamplingRegionNode());
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNodeLegacySceneTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestLegacyScene(true));
  CHECK_EXIT_SUCCESS(TestLegacyScene(false));
  CHECK_EXIT_SUCCESS(TestLegacyRegionAndGlyphPoints());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
