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

// Checks that the transform visualization settings of a scene saved by an earlier Slicer version are kept as they were
// read, and saved again, when nothing converts them (for example when the Vector Field Rendering module is not loaded).

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformNode.h"

// STD includes
#include <string>
#include <vector>

namespace
{

/// The visualization attributes of a transform display node as an earlier Slicer version wrote them
const std::vector<std::pair<std::string, std::string>> LegacyAttributes = {
  { "VisualizationMode", "GRID" },       { "GlyphSpacingMm", "12" },     { "GlyphScalePercent", "150" },    { "GlyphDisplayRangeMaxMm", "80" },
  { "GlyphDisplayRangeMinMm", "0.5" },   { "GlyphType", "CONE" },        { "GlyphTipLengthPercent", "25" }, { "GlyphDiameterMm", "4" },
  { "GlyphShaftDiameterPercent", "35" }, { "GlyphResolution", "8" },     { "GlyphResolution2D", "10" },     { "GlyphTipLengthPercent2D", "20" },
  { "GridScalePercent", "200" },         { "GridSpacingMm", "18" },      { "GridLineDiameterMm", "1.5" },   { "GridResolutionMm", "3" },
  { "GridShowNonWarped", "1" },          { "ContourResolutionMm", "4" }, { "ContourLevelsMm", "1 3 5" },    { "ContourOpacity", "0.6" },
};

//----------------------------------------------------------------------------
std::string GetLegacyScene()
{
  std::string displayNodeAttributes;
  for (const auto& nameAndValue : LegacyAttributes)
  {
    displayNodeAttributes += " " + nameAndValue.first + "=\"" + nameAndValue.second + "\"";
  }
  return "<MRML version=\"Slicer4.4.0\" userTags=\"\">\n"
         " <MarkupsROI id=\"vtkMRMLMarkupsROINode1\" name=\"Region\" />\n"
         " <MarkupsFiducial id=\"vtkMRMLMarkupsFiducialNode1\" name=\"Points\" />\n"
         " <Transform id=\"vtkMRMLTransformNode1\" name=\"Transform\" displayNodeRef=\"vtkMRMLTransformDisplayNode1\" />\n"
         " <TransformDisplayNode id=\"vtkMRMLTransformDisplayNode1\" name=\"TransformDisplay\" visibility=\"true\""
         " region=\"vtkMRMLMarkupsROINode1\" glyphPoints=\"vtkMRMLMarkupsFiducialNode1\""
         + displayNodeAttributes
         + " EditorVisibility=\"1\" EditorTranslationEnabled=\"0\" />\n"
           "</MRML>\n";
}

//----------------------------------------------------------------------------
bool ImportScene(vtkMRMLScene* scene, const std::string& sceneXML)
{
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  return (scene->Import() != 0);
}

//----------------------------------------------------------------------------
std::string SaveScene(vtkMRMLScene* scene)
{
  scene->SetSaveToXMLString(1);
  scene->Commit();
  return scene->GetSceneXMLString();
}

//----------------------------------------------------------------------------
/// The XML element of the transform display node in a saved scene
std::string GetDisplayNodeElement(const std::string& sceneXML)
{
  size_t start = sceneXML.find("<TransformDisplayNode");
  if (start == std::string::npos)
  {
    return std::string();
  }
  return sceneXML.substr(start, sceneXML.find(">", start) - start);
}

//----------------------------------------------------------------------------
/// The visualization attributes and references are read and kept, the interaction settings are read as before
int TestRead(vtkMRMLTransformDisplayNode* displayNode)
{
  CHECK_NOT_NULL(displayNode);
  CHECK_BOOL(displayNode->HasLegacyVisualization(), true);
  for (const auto& nameAndValue : LegacyAttributes)
  {
    CHECK_STD_STRING(displayNode->GetLegacyVisualizationAttribute(nameAndValue.first), nameAndValue.second);
  }
  CHECK_STD_STRING(displayNode->GetLegacyVisualizationAttribute("NoSuchAttribute"), "");
  CHECK_NOT_NULL(vtkMRMLMarkupsROINode::SafeDownCast(displayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyRegionNodeReferenceRole())));
  CHECK_NOT_NULL(vtkMRMLMarkupsFiducialNode::SafeDownCast(displayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyGlyphPointsNodeReferenceRole())));
  CHECK_BOOL(displayNode->GetEditorVisibility(), true);
  CHECK_BOOL(displayNode->GetEditorTranslationEnabled(), false);
  CHECK_BOOL(displayNode->GetVisibility() != 0, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Saved again, the scene has the same visualization attributes and references
int CheckSavedElement(const std::string& element, const std::string& regionNodeID, const std::string& glyphPointsNodeID)
{
  for (const auto& nameAndValue : LegacyAttributes)
  {
    std::string attribute = " " + nameAndValue.first + "=\"" + nameAndValue.second + "\"";
    if (element.find(attribute) == std::string::npos)
    {
      std::cerr << "Attribute" << attribute << " is not saved in: " << element << std::endl;
      return EXIT_FAILURE;
    }
  }
  // Written in the references attribute, as earlier Slicer versions wrote them
  CHECK_BOOL(element.find("region:" + regionNodeID + ";") != std::string::npos, true);
  CHECK_BOOL(element.find("glyphPoints:" + glyphPointsNodeID + ";") != std::string::npos, true);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNodeLegacySceneTest1(int, char*[])
{
  // Read, and saved again unchanged
  vtkNew<vtkMRMLScene> scene;
  CHECK_BOOL(ImportScene(scene, GetLegacyScene()), true);
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(scene->GetNodeByID("vtkMRMLTransformDisplayNode1"));
  CHECK_EXIT_SUCCESS(TestRead(displayNode));
  std::string savedScene = SaveScene(scene);
  CHECK_EXIT_SUCCESS(CheckSavedElement(GetDisplayNodeElement(savedScene), "vtkMRMLMarkupsROINode1", "vtkMRMLMarkupsFiducialNode1"));

  // The saved scene can be read again, by this or an earlier Slicer version
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ImportScene(reloadedScene, savedScene), true);
  CHECK_EXIT_SUCCESS(TestRead(vtkMRMLTransformDisplayNode::SafeDownCast(reloadedScene->GetNodeByID("vtkMRMLTransformDisplayNode1"))));

  // Imported into a scene where the IDs are taken, the references follow the nodes to their new IDs
  vtkNew<vtkMRMLScene> otherScene;
  otherScene->AddNewNodeByClass("vtkMRMLMarkupsROINode");
  otherScene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode");
  otherScene->AddNewNodeByClass("vtkMRMLTransformNode");
  vtkNew<vtkMRMLTransformDisplayNode> existingDisplayNode;
  otherScene->AddNode(existingDisplayNode);
  CHECK_BOOL(ImportScene(otherScene, GetLegacyScene()), true);
  vtkMRMLTransformDisplayNode* importedDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(otherScene->GetFirstNodeByName("TransformDisplay"));
  CHECK_NOT_NULL(importedDisplayNode);
  vtkMRMLNode* importedRegionNode = importedDisplayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyRegionNodeReferenceRole());
  vtkMRMLNode* importedPointsNode = importedDisplayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyGlyphPointsNodeReferenceRole());
  CHECK_NOT_NULL(importedRegionNode);
  CHECK_NOT_NULL(importedPointsNode);
  CHECK_STRING(importedRegionNode->GetName(), "Region");
  CHECK_STRING(importedPointsNode->GetName(), "Points");
  CHECK_BOOL(std::string(importedRegionNode->GetID()) != "vtkMRMLMarkupsROINode1", true);

  // A copy has the same settings, for example in a scene view or a sequence
  vtkNew<vtkMRMLTransformDisplayNode> copiedNode;
  copiedNode->CopyContent(displayNode);
  for (const auto& nameAndValue : LegacyAttributes)
  {
    CHECK_STD_STRING(copiedNode->GetLegacyVisualizationAttribute(nameAndValue.first), nameAndValue.second);
  }

  // Once converted, they are cleared, and not saved any more
  displayNode->ClearLegacyVisualization();
  CHECK_BOOL(displayNode->HasLegacyVisualization(), false);
  std::string element = GetDisplayNodeElement(SaveScene(scene));
  CHECK_BOOL(element.find("VisualizationMode") == std::string::npos, true);
  CHECK_BOOL(element.find("GlyphSpacingMm") == std::string::npos, true);
  CHECK_BOOL(element.find("region:") == std::string::npos, true);
  CHECK_BOOL(element.find("glyphPoints:") == std::string::npos, true);
  // The interaction settings are still saved
  CHECK_BOOL(element.find("EditorVisibility=\"1\"") != std::string::npos, true);

  // A new display node has no visualization settings to convert
  vtkNew<vtkMRMLTransformDisplayNode> newDisplayNode;
  CHECK_BOOL(newDisplayNode->HasLegacyVisualization(), false);

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
