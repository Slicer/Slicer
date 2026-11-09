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
#include "vtkSlicerVectorFieldRenderingLogic.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkVectorFieldSampler.h"

// Colors includes
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>

// STD includes
#include <sstream>

namespace
{
/// Name of the color map that a vector field display node of a transform is colored by
const char DefaultTransformColorNodeName[] = "Displacement to color";

//----------------------------------------------------------------------------
/// True if the attributes hold a 3-component array
bool HasVectorArray(vtkDataSetAttributes* attributes)
{
  for (int arrayIndex = 0; attributes && arrayIndex < attributes->GetNumberOfArrays(); ++arrayIndex)
  {
    vtkDataArray* array = attributes->GetArray(arrayIndex);
    if (array && array->GetNumberOfComponents() == 3)
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
/// Number in a legacy visualization attribute. Returns false if the attribute was not read or is not a number.
bool GetLegacyNumber(vtkMRMLTransformDisplayNode* transformDisplayNode, const char* attributeName, double& value)
{
  std::string text = transformDisplayNode->GetLegacyVisualizationAttribute(attributeName);
  if (text.empty())
  {
    return false;
  }
  std::istringstream stream(text);
  return static_cast<bool>(stream >> value);
}
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVectorFieldRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVectorFieldRenderingLogic::vtkSlicerVectorFieldRenderingLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerVectorFieldRenderingLogic::~vtkSlicerVectorFieldRenderingLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes failed: invalid scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLVectorFieldDisplayNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLVectorFieldDisplayNode>::New());
  }
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);
  this->ConvertLegacyTransformDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::OnMRMLSceneEndImport()
{
  this->ConvertLegacyTransformDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::OnMRMLSceneEndRestore()
{
  this->ConvertLegacyTransformDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::ConvertLegacyTransformDisplayNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
  std::vector<vtkMRMLNode*> transformDisplayNodes;
  scene->GetNodesByClass("vtkMRMLTransformDisplayNode", transformDisplayNodes);
  for (vtkMRMLNode* node : transformDisplayNodes)
  {
    vtkMRMLTransformDisplayNode* transformDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node);
    if (transformDisplayNode && transformDisplayNode->HasLegacyVisualization())
    {
      vtkSlicerVectorFieldRenderingLogic::ConvertLegacyTransformDisplayNode(transformDisplayNode);
    }
  }
}

//----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* vtkSlicerVectorFieldRenderingLogic::ConvertLegacyTransformDisplayNode(vtkMRMLTransformDisplayNode* transformDisplayNode)
{
  if (!transformDisplayNode || !transformDisplayNode->HasLegacyVisualization())
  {
    return nullptr;
  }
  vtkMRMLScene* scene = transformDisplayNode->GetScene();
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(transformDisplayNode->GetDisplayableNode());
  vtkMRMLNode* regionNode = transformDisplayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyRegionNodeReferenceRole());
  vtkMRMLNode* glyphPointsNode = transformDisplayNode->GetNodeReference(vtkMRMLTransformDisplayNode::GetLegacyGlyphPointsNodeReferenceRole());
  // A visualization that was never shown nor placed anywhere had only default settings
  bool inUse = (transformDisplayNode->GetVisibility() || regionNode || glyphPointsNode);
  vtkMRMLVectorFieldDisplayNode* displayNode = nullptr;
  if (scene && transformNode && inUse)
  {
    vtkNew<vtkMRMLVectorFieldDisplayNode> newDisplayNode;
    std::string name = std::string(transformNode->GetName() ? transformNode->GetName() : "") + " vector field";
    newDisplayNode->SetName(scene->GenerateUniqueName(name).c_str());
    displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNode(newDisplayNode));
  }
  if (displayNode)
  {
    MRMLNodeModifyBlocker blocker(displayNode);
    // The color map that the transform was colored by
    displayNode->SetAndObserveColorNodeID(transformDisplayNode->GetColorNodeID());
    vtkSlicerVectorFieldRenderingLogic::SetDefaultTransformDisplayProperties(displayNode);

    std::string mode = transformDisplayNode->GetLegacyVisualizationAttribute("VisualizationMode");
    int visualizationMode = vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph;
    if (mode == "GRID")
    {
      visualizationMode = vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid;
    }
    else if (mode == "CONTOUR")
    {
      visualizationMode = vtkMRMLVectorFieldDisplayNode::VisualizationModeContour;
    }
    displayNode->SetVisualizationMode(visualizationMode);
    std::string glyphType = transformDisplayNode->GetLegacyVisualizationAttribute("GlyphType");
    if (glyphType == "CONE")
    {
      displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeCone);
    }
    else if (glyphType == "SPHERE")
    {
      displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere);
    }
    else
    {
      displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow);
    }

    // Each mode had its own spacing; every mode shares one now, and the one of the mode the scene was saved in is what
    // it showed
    const char* spacingAttributeName = "GlyphSpacingMm";
    if (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid)
    {
      spacingAttributeName = "GridResolutionMm";
    }
    else if (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour)
    {
      spacingAttributeName = "ContourResolutionMm";
    }
    double value = 0.0;
    if (GetLegacyNumber(transformDisplayNode, spacingAttributeName, value))
    {
      displayNode->SetSamplingSpacing(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphScalePercent", value))
    {
      displayNode->SetGlyphScaleFactor(value * 0.01);
    }
    // Glyphs were only drawn where the displacement was in the display range
    double thresholdRange[2] = { displayNode->GetThresholdRange()[0], displayNode->GetThresholdRange()[1] };
    GetLegacyNumber(transformDisplayNode, "GlyphDisplayRangeMinMm", thresholdRange[0]);
    GetLegacyNumber(transformDisplayNode, "GlyphDisplayRangeMaxMm", thresholdRange[1]);
    displayNode->SetThresholdRange(thresholdRange);
    if (GetLegacyNumber(transformDisplayNode, "GlyphTipLengthPercent", value))
    {
      displayNode->SetGlyphTipLengthPercent(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphDiameterMm", value))
    {
      displayNode->SetGlyphDiameter(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphShaftDiameterPercent", value))
    {
      displayNode->SetGlyphShaftDiameterPercent(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphResolution", value))
    {
      displayNode->SetGlyphResolution(static_cast<int>(value));
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphResolution2D", value))
    {
      displayNode->SetGlyphResolution2D(static_cast<int>(value));
    }
    if (GetLegacyNumber(transformDisplayNode, "GlyphTipLengthPercent2D", value))
    {
      displayNode->SetGlyphTipLengthPercent2D(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GridScalePercent", value))
    {
      displayNode->SetGridScaleFactor(value * 0.01);
    }
    if (GetLegacyNumber(transformDisplayNode, "GridSpacingMm", value))
    {
      displayNode->SetGridSpacing(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GridLineDiameterMm", value))
    {
      displayNode->SetGridLineDiameter(value);
    }
    if (GetLegacyNumber(transformDisplayNode, "GridShowNonWarped", value))
    {
      displayNode->SetGridShowNonWarped(value != 0.0);
    }
    if (GetLegacyNumber(transformDisplayNode, "ContourOpacity", value))
    {
      displayNode->SetContourOpacity3D(value);
    }
    std::string contourLevels = transformDisplayNode->GetLegacyVisualizationAttribute("ContourLevelsMm");
    if (!contourLevels.empty())
    {
      displayNode->SetContourLevelsAsString(contourLevels);
    }

    // Glyphs were drawn at the glyph points if there were some, everything else in the region
    bool glyphMode = (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
    displayNode->SetAndObserveSamplingRegionNode(glyphMode && glyphPointsNode ? glyphPointsNode : regionNode);

    // Shown as the transform was, and with the transform by its eye icon
    vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(displayNode, true);
    displayNode->SetVisibility(transformDisplayNode->GetVisibility());
    displayNode->SetVisibility2D(transformDisplayNode->GetVisibility2D());
    displayNode->SetVisibility3D(transformDisplayNode->GetVisibility3D());
    displayNode->SetSliceIntersectionThickness(transformDisplayNode->GetSliceIntersectionThickness());
    for (const std::string& viewNodeID : transformDisplayNode->GetViewNodeIDs())
    {
      displayNode->AddViewNodeID(viewNodeID.c_str());
    }
  }
  transformDisplayNode->ClearLegacyVisualization();
  if (displayNode)
  {
    transformNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    // Adding a display node does not tell the displayable managers that the node has a new display node to draw
    transformNode->Modified();
  }
  return displayNode;
}

//----------------------------------------------------------------------------
bool vtkSlicerVectorFieldRenderingLogic::HasVectorField(vtkMRMLNode* node)
{
  if (vtkMRMLTransformNode::SafeDownCast(node))
  {
    return true;
  }
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if (volumeNode)
  {
    vtkImageData* image = volumeNode->GetImageData();
    return (image && image->GetNumberOfScalarComponents() >= 3);
  }
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  vtkPointSet* mesh = (modelNode ? modelNode->GetMesh() : nullptr);
  return (mesh && (HasVectorArray(mesh->GetPointData()) || HasVectorArray(mesh->GetCellData())));
}

//----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(vtkMRMLDisplayableNode* node)
{
  vtkMRMLScene* scene = (node ? node->GetScene() : nullptr);
  if (!scene || !vtkSlicerVectorFieldRenderingLogic::HasVectorField(node))
  {
    return nullptr;
  }
  // The default display node has to be the first one: modules, views and the subject hierarchy use the first display
  // node of a node as its display node
  node->CreateDefaultDisplayNodes();

  vtkNew<vtkMRMLVectorFieldDisplayNode> displayNode;
  std::string name = std::string(node->GetName() ? node->GetName() : "") + " vector field";
  displayNode->SetName(scene->GenerateUniqueName(name).c_str());
  if (!scene->AddNode(displayNode))
  {
    return nullptr;
  }
  if (vtkMRMLTransformNode::SafeDownCast(node))
  {
    vtkSlicerVectorFieldRenderingLogic::SetDefaultTransformDisplayProperties(displayNode);
  }
  else if (vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node))
  {
    // The color table of the model, so that the field is colored the same way as the surface
    vtkMRMLModelDisplayNode* modelDisplayNode = modelNode->GetModelDisplayNode();
    if (modelDisplayNode && modelDisplayNode->GetColorNodeID())
    {
      displayNode->SetAndObserveColorNodeID(modelDisplayNode->GetColorNodeID());
    }
  }
  else
  {
    displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFilePlasma.txt");
  }
  vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(displayNode, true);
  node->AddAndObserveDisplayNodeID(displayNode->GetID());
  // Adding a display node does not tell the displayable managers that the node has a new display node to draw
  node->Modified();
  return displayNode;
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::RemoveVectorFieldDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  vtkMRMLScene* scene = (displayNode ? displayNode->GetScene() : nullptr);
  if (!scene)
  {
    return;
  }
  vtkMRMLDisplayableNode* node = displayNode->GetDisplayableNode();
  if (node)
  {
    // Collected first, as removing a legend removes it from the display nodes of the node
    std::vector<vtkMRMLColorLegendDisplayNode*> colorLegendNodes;
    for (int legendIndex = 0; legendIndex < vtkSlicerColorLogic::GetNumberOfColorLegendDisplayNodes(node); ++legendIndex)
    {
      vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetNthColorLegendDisplayNode(node, legendIndex);
      if (colorLegendNode && colorLegendNode->GetPrimaryDisplayNode() == displayNode)
      {
        colorLegendNodes.push_back(colorLegendNode);
      }
    }
    for (vtkMRMLColorLegendDisplayNode* colorLegendNode : colorLegendNodes)
    {
      scene->RemoveNode(colorLegendNode);
    }
  }
  scene->RemoveNode(displayNode);
}

//----------------------------------------------------------------------------
std::vector<vtkMRMLVectorFieldDisplayNode*> vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(vtkMRMLDisplayableNode* node)
{
  std::vector<vtkMRMLVectorFieldDisplayNode*> displayNodes;
  for (int displayNodeIndex = 0; node && displayNodeIndex < node->GetNumberOfDisplayNodes(); ++displayNodeIndex)
  {
    vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(node->GetNthDisplayNode(displayNodeIndex));
    if (displayNode)
    {
      displayNodes.push_back(displayNode);
    }
  }
  return displayNodes;
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(vtkMRMLVectorFieldDisplayNode* displayNode, bool shown)
{
  if (!displayNode)
  {
    return;
  }
  MRMLNodeModifyBlocker blocker(displayNode);
  // A hidden vector field is left out when the eye icon shows the node again
  displayNode->SetShowMode(shown ? vtkMRMLDisplayNode::ShowDefault : vtkMRMLDisplayNode::ShowIgnore);
  displayNode->SetVisibility(shown);
}

//----------------------------------------------------------------------------
bool vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPolyData* mesh)
{
  if (!displayNode || !mesh)
  {
    vtkGenericWarningMacro("vtkSlicerVectorFieldRenderingLogic::GetVisualizationMesh failed: invalid display node or mesh");
    return false;
  }
  // The pipeline of the views if one shows the node, otherwise one that only lives for this call
  vtkSmartPointer<vtkMRMLVectorFieldPipeline> pipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode);
  return pipeline->GetVisualizationMesh(mesh);
}

//----------------------------------------------------------------------------
void vtkSlicerVectorFieldRenderingLogic::SetDefaultTransformDisplayProperties(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return;
  }
  MRMLNodeModifyBlocker blocker(displayNode);
  displayNode->SetGlyphOrientationArrayName(vtkVectorFieldSampler::GetVectorArrayName());
  displayNode->SetGlyphScaleArrayName(vtkVectorFieldSampler::GetVectorArrayName());

  // Arrows keep their thickness in mm however long they are, which is what makes a field of displacement vectors
  // readable
  displayNode->SetGlyphDiameterAbsolute(true);
  displayNode->SetGlyphScaleFactor(1.0);
  displayNode->SetGlyphTipLengthPercent(30.0);
  displayNode->SetGlyphShaftDiameterPercent(40.0);
  displayNode->SetGlyphResolution(6);

  // Displacements below 0.01 mm are not drawn
  displayNode->SetThresholdEnabled(true);
  displayNode->SetThresholdRange(0.01, 100.0);

  displayNode->SetGridSpacing(15.0);
  displayNode->SetGridLineDiameter(1.0);
  // A transform is sampled, so the glyphs sit on a lattice, 10 mm apart
  displayNode->SetSamplingSpacing(10.0);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  std::vector<double> contourLevels;
  for (int level = 2; level <= 18; level += 2)
  {
    contourLevels.push_back(level);
  }
  displayNode->SetContourLevels(contourLevels.data(), static_cast<int>(contourLevels.size()));

  // The colors are exactly those of the color map, which goes from a displacement in mm to a color
  displayNode->SetScalarVisibility(1);
  displayNode->SetActiveScalarName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  displayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseColorNodeScalarRange);
  vtkMRMLScene* scene = displayNode->GetScene();
  if (!scene || displayNode->GetColorNode())
  {
    return;
  }
  vtkNew<vtkMRMLProceduralColorNode> colorNode;
  colorNode->SetName(scene->GenerateUniqueName(DefaultTransformColorNodeName).c_str());
  colorNode->SetAttribute("Category", "Transform display");
  // A procedural color node is saved using a storage node, and hidden nodes are not saved if they use a storage node,
  // therefore the color node must be visible.
  colorNode->SetHideFromEditors(false);
  vtkColorTransferFunction* colorMap = colorNode->GetColorTransferFunction();
  // Map: mm -> RGB
  colorMap->AddRGBPoint(1.0, 0.2, 0.2, 0.2);
  colorMap->AddRGBPoint(2.0, 0.0, 1.0, 0.0);
  colorMap->AddRGBPoint(5.0, 1.0, 1.0, 0.0);
  colorMap->AddRGBPoint(10.0, 1.0, 0.0, 0.0);
  scene->AddNode(colorNode);
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}
