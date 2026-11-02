/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

// MRML includes
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformInteractionDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <set>
#include <map>
#include <sstream>

namespace
{
const char* DEFAULT_COLOR_TABLE_NAME = "Displacement to color";
const char CONTOUR_LEVEL_SEPARATOR = ' ';

/// Attributes of the interactive editing settings that scenes saved by earlier versions
/// stored in this node. They belong to vtkMRMLTransformInteractionDisplayNode now.
const std::set<std::string> LegacyInteractionAttributeNames = {
  "EditorVisibility",
  "EditorVisibility3D",
  "EditorSliceIntersectionVisibility",
  "EditorTranslationEnabled",
  "EditorRotationEnabled",
  "EditorScalingEnabled",
  "EditorTranslationSliceAnywhereEnabled",
  "EditorTranslationSliceAnywhereSensitivity",
  "InteractionSizeAbsolute",
  "InteractionSizeMm",
  "InteractionScalePercent",
  "InteractionHandleOpacity",
  "TranslationHandleComponentVisibility3D",
  "RotationHandleComponentVisibility3D",
  "ScaleHandleComponentVisibility3D",
  "TranslationHandleComponentVisibilitySlice",
  "RotationHandleComponentVisibilitySlice",
  "ScaleHandleComponentVisibilitySlice",
};
} // namespace

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::vtkMRMLTransformDisplayNode()
{
  this->SetScalarVisibility(1);
  // The colors are exactly those of the color map, which goes from a displacement in mm to a
  // color (see UpdateScalarRange)
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseColorNodeScalarRange);
  this->SetActiveScalarName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  this->SetGlyphOrientationArrayName(vtkVectorFieldSampler::GetVectorArrayName());
  this->SetGlyphScaleArrayName(vtkVectorFieldSampler::GetVectorArrayName());

  this->Visibility = 0;
  this->Visibility2D = 1;
  this->Visibility3D = 1;

  // Arrows keep their thickness in mm however long they are, which is what makes a field of
  // displacement vectors readable.
  this->SetGlyphDiameterAbsolute(true);
  this->SetGlyphScaleFactor(1.0);
  this->SetGlyphTipLengthPercent(30.0);
  this->SetGlyphShaftDiameterPercent(40.0);
  this->SetGlyphResolution(6);

  this->SetThresholdEnabled(true);
  this->SetThresholdRange(0.01, 100.0);

  this->SetGridSpacing(15.0);
  this->SetGridLineDiameter(1.0);
  // As the glyphs were placed before every mode shared one spacing: on a lattice, 10 mm apart.
  // A transform is sampled, so the glyphs sit on a lattice rather than on mesh points.
  this->SetSamplingSpacing(10.0);
  this->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  std::vector<double> contourLevels;
  for (int i = 2; i <= 18; i += 2)
  {
    contourLevels.push_back(i);
  }
  this->SetContourLevels(contourLevels.data(), static_cast<int>(contourLevels.size()));

  this->TypeDisplayName = vtkMRMLTr("vtkMRMLTransformDisplayNode", "Transform Display");
}

//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::~vtkMRMLTransformDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // The attribute names are the ones that transform display used before it was built on the
  // shared vector field display node, so that scenes stay readable by both.
  of << " VisualizationMode=\"" << ConvertVisualizationModeToString(this->GetVisualizationMode()) << "\"";
  of << " GlyphSpacingMm=\"" << this->GetSamplingSpacing() << "\"";
  of << " GlyphScalePercent=\"" << this->GetGlyphScalePercent() << "\"";
  of << " GlyphDisplayRangeMaxMm=\"" << this->GetGlyphDisplayRangeMaxMm() << "\"";
  of << " GlyphDisplayRangeMinMm=\"" << this->GetGlyphDisplayRangeMinMm() << "\"";
  of << " GlyphType=\"" << ConvertGlyphTypeToString(this->GetGlyphType()) << "\"";
}

#define READ_FROM_ATT_INTO(attributeName, setter) \
  if (!strcmp(xmlReadAttName, #attributeName))    \
  {                                               \
    double attributeValue = 0.0;                  \
    std::stringstream ss;                         \
    ss << xmlReadAttValue;                        \
    ss >> attributeValue;                         \
    this->setter(attributeValue);                 \
    continue;                                     \
  }

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  // The nodes that earlier versions drew the transform in (region) and placed glyphs at
  // (glyphPoints), by ID
  std::string legacyRegionNodeID;
  std::string legacyGlyphPointsNodeID;
  // and the spacing that each mode was sampled at
  std::map<int, double> legacySpacings;
  auto readLegacySpacing = [&legacySpacings](int mode, const char* value)
  {
    std::stringstream ss;
    ss << value;
    double spacing = 0.0;
    if (ss >> spacing)
    {
      legacySpacings[mode] = spacing;
    }
  };
  // One bound of the threshold range. Whether it applies is thresholdEnabled (read by the superclass); older scenes
  // without it keep the constructor default (on), as those versions always applied the range.
  auto readLegacyThresholdBound = [this](int index, const char* value)
  {
    std::stringstream ss;
    ss << value;
    double bound = 0.0;
    if (ss >> bound)
    {
      double range[2] = { this->ThresholdRange[0], this->ThresholdRange[1] };
      range[index] = bound;
      this->SetThresholdRange(range);
    }
  };

  vtkMRMLReadXMLBeginMacro(atts);

  if (!strcmp(xmlReadAttName, "VisualizationMode"))
  {
    int mode = ConvertVisualizationModeFromString(xmlReadAttValue);
    if (mode >= 0)
    {
      this->SetVisualizationMode(mode);
    }
    continue;
  }
  if (!strcmp(xmlReadAttName, "GlyphType"))
  {
    int glyphType = ConvertGlyphTypeFromString(xmlReadAttValue);
    if (glyphType >= 0)
    {
      this->SetGlyphType(glyphType);
    }
    continue;
  }
  if (!strcmp(xmlReadAttName, "GlyphSpacingMm"))
  {
    readLegacySpacing(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH, xmlReadAttValue);
    continue;
  }
  if (!strcmp(xmlReadAttName, "GridResolutionMm"))
  {
    readLegacySpacing(vtkMRMLTransformDisplayNode::VIS_MODE_GRID, xmlReadAttValue);
    continue;
  }
  if (!strcmp(xmlReadAttName, "ContourResolutionMm"))
  {
    readLegacySpacing(vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR, xmlReadAttValue);
    continue;
  }
  READ_FROM_ATT_INTO(GlyphScalePercent, SetGlyphScalePercent);
  if (!strcmp(xmlReadAttName, "GlyphDisplayRangeMaxMm"))
  {
    readLegacyThresholdBound(1, xmlReadAttValue);
    continue;
  }
  if (!strcmp(xmlReadAttName, "GlyphDisplayRangeMinMm"))
  {
    readLegacyThresholdBound(0, xmlReadAttValue);
    continue;
  }
  // Properties that moved to the shared node but kept their old attribute name
  READ_FROM_ATT_INTO(GlyphTipLengthPercent, SetGlyphTipLengthPercent);
  READ_FROM_ATT_INTO(GlyphDiameterMm, SetGlyphDiameter);
  READ_FROM_ATT_INTO(GlyphShaftDiameterPercent, SetGlyphShaftDiameterPercent);
  READ_FROM_ATT_INTO(GlyphResolution, SetGlyphResolution);
  READ_FROM_ATT_INTO(GlyphResolution2D, SetGlyphResolution2D);
  READ_FROM_ATT_INTO(GlyphTipLengthPercent2D, SetGlyphTipLengthPercent2D);
  READ_FROM_ATT_INTO(GridScalePercent, SetGridScalePercent);
  READ_FROM_ATT_INTO(GridSpacingMm, SetGridSpacing);
  READ_FROM_ATT_INTO(GridLineDiameterMm, SetGridLineDiameter);
  READ_FROM_ATT_INTO(GridShowNonWarped, SetGridShowNonWarped);
  READ_FROM_ATT_INTO(ContourOpacity, SetContourOpacity3D);
  if (!strcmp(xmlReadAttName, "ContourLevelsMm"))
  {
    this->SetContourLevelsAsString(xmlReadAttValue);
    continue;
  }
  if (!strcmp(xmlReadAttName, "region"))
  {
    legacyRegionNodeID = xmlReadAttValue;
    continue;
  }
  if (!strcmp(xmlReadAttName, "glyphPoints"))
  {
    legacyGlyphPointsNodeID = xmlReadAttValue;
    continue;
  }
  // Editor properties of scenes that were saved before they moved into their own node. They
  // are kept as they were written and applied once the transform node is known.
  if (LegacyInteractionAttributeNames.count(xmlReadAttName))
  {
    this->LegacyInteractionAttributes[xmlReadAttName] = xmlReadAttValue;
    continue;
  }

  vtkMRMLReadXMLEndMacro();

  // Earlier versions kept a spacing for each mode; every mode shares one now, and the one of
  // the mode the scene was saved in is what it showed
  if (!legacySpacings.empty())
  {
    auto modeSpacing = legacySpacings.find(this->GetVisualizationMode());
    this->SetSamplingSpacing(modeSpacing != legacySpacings.end() ? modeSpacing->second : legacySpacings.begin()->second);
  }

  // Earlier versions drew glyphs at the glyph points when there were some, and everything
  // else in the region. Both are the sampling region now.
  if (!legacyGlyphPointsNodeID.empty() && this->GetVisualizationMode() == vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH)
  {
    this->SetAndObserveSamplingRegionNodeID(legacyGlyphPointsNodeID.c_str());
  }
  else if (!legacyRegionNodeID.empty())
  {
    this->SetAndObserveSamplingRegionNodeID(legacyRegionNodeID.c_str());
  }

  // When the node is already in the scene (for example its attributes are set again) the
  // interaction display node can be found right away
  this->ApplyLegacyInteractionAttributes();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::UpdateScene(vtkMRMLScene* scene)
{
  this->Superclass::UpdateScene(scene);
  // All the nodes of the scene that is read are in the scene now, so the transform node
  // that refers to this node can be found
  this->ApplyLegacyInteractionAttributes();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ApplyLegacyInteractionAttributes()
{
  if (this->LegacyInteractionAttributes.empty() || !this->GetScene() || !this->GetDisplayableNode())
  {
    return;
  }
  vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode(true);
  if (!interactionDisplayNode)
  {
    return;
  }
  // Booleans were written both as "1"/"0" and as "true"/"false", depending on the version
  auto toBool = [](const std::string& value) { return (value == "true" || value == "1"); };
  auto toDouble = [](const std::string& value)
  {
    double number = 0.0;
    std::stringstream ss(value);
    ss >> number;
    return number;
  };
  auto toBoolVector = [&toBool](const std::string& value, bool vector[4])
  {
    std::stringstream ss(value);
    for (int i = 0; i < 4; ++i)
    {
      std::string component;
      ss >> component;
      vector[i] = toBool(component);
    }
  };

  MRMLNodeModifyBlocker blocker(interactionDisplayNode);
  for (const auto& nameAndValue : this->LegacyInteractionAttributes)
  {
    const std::string& name = nameAndValue.first;
    const std::string& value = nameAndValue.second;
    bool vector[4] = { false, false, false, false };
    if (name == "EditorVisibility")
    {
      interactionDisplayNode->SetEditorVisibility(toBool(value));
    }
    else if (name == "EditorVisibility3D")
    {
      interactionDisplayNode->SetEditorVisibility3D(toBool(value));
    }
    else if (name == "EditorSliceIntersectionVisibility")
    {
      interactionDisplayNode->SetEditorSliceIntersectionVisibility(toBool(value));
    }
    else if (name == "EditorTranslationEnabled")
    {
      interactionDisplayNode->SetEditorTranslationEnabled(toBool(value));
    }
    else if (name == "EditorRotationEnabled")
    {
      interactionDisplayNode->SetEditorRotationEnabled(toBool(value));
    }
    else if (name == "EditorScalingEnabled")
    {
      interactionDisplayNode->SetEditorScalingEnabled(toBool(value));
    }
    else if (name == "EditorTranslationSliceAnywhereEnabled")
    {
      interactionDisplayNode->SetEditorTranslationSliceAnywhereEnabled(toBool(value));
    }
    else if (name == "EditorTranslationSliceAnywhereSensitivity")
    {
      interactionDisplayNode->SetEditorTranslationSliceAnywhereSensitivity(toDouble(value));
    }
    else if (name == "InteractionSizeAbsolute")
    {
      interactionDisplayNode->SetInteractionSizeAbsolute(toBool(value));
    }
    else if (name == "InteractionSizeMm")
    {
      interactionDisplayNode->SetInteractionSizeMm(toDouble(value));
    }
    else if (name == "InteractionScalePercent")
    {
      interactionDisplayNode->SetInteractionScalePercent(toDouble(value));
    }
    else if (name == "InteractionHandleOpacity")
    {
      interactionDisplayNode->SetInteractionHandleOpacity(toDouble(value));
    }
    else if (name == "TranslationHandleComponentVisibility3D")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetTranslationHandleComponentVisibility3D(vector);
    }
    else if (name == "RotationHandleComponentVisibility3D")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetRotationHandleComponentVisibility3D(vector);
    }
    else if (name == "ScaleHandleComponentVisibility3D")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetScaleHandleComponentVisibility3D(vector);
    }
    else if (name == "TranslationHandleComponentVisibilitySlice")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetTranslationHandleComponentVisibilitySlice(vector);
    }
    else if (name == "RotationHandleComponentVisibilitySlice")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetRotationHandleComponentVisibilitySlice(vector);
    }
    else if (name == "ScaleHandleComponentVisibilitySlice")
    {
      toBoolVector(value, vector);
      interactionDisplayNode->SetScaleHandleComponentVisibilitySlice(vector);
    }
  }
  this->LegacyInteractionAttributes.clear();
}

//----------------------------------------------------------------------------
// Vector field source
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::CanSampleAtArbitraryPositions()
{
  return vtkMRMLTransformNode::SafeDownCast(this->GetDisplayableNode()) != nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::HasFixedFieldArrays()
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::IsVisualizationModeSupported(int visualizationMode)
{
  return (visualizationMode >= 0 && visualizationMode < vtkMRMLTransformDisplayNode::VIS_MODE_LAST);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::UpdateScalarRange()
{
  // The color map maps displacement in mm to color, so its range is the scalar range, even before the field is sampled.
  // A manual range or one taken from the data is handled by the superclass.
  if (this->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseColorNodeScalarRange)
  {
    this->Superclass::UpdateScalarRange();
    return;
  }
  vtkMRMLColorNode* colorNode = this->GetColorNode();
  if (colorNode && colorNode->GetLookupTable())
  {
    double* range = colorNode->GetLookupTable()->GetRange();
    if (range && range[1] >= range[0])
    {
      this->SetScalarRange(range[0], range[1]);
      return;
    }
  }
  this->Superclass::UpdateScalarRange();
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformDisplayNode::GetSampledTransform()
{
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(this->GetDisplayableNode());
  if (!transformNode)
  {
    this->SampledTransform = nullptr;
    this->SampledTransformNode = nullptr;
    return nullptr;
  }
  // Inverting or reparenting modifies no transform, so node modified times are included to rebuild the transform to world.
  // Transform edits need no rebuild: it concatenates the node transforms themselves, so it is modified with them.
  vtkMTimeType buildTime = transformNode->GetTransformToWorldMTime();
  for (vtkMRMLTransformNode* node = transformNode; node; node = node->GetParentTransformNode())
  {
    buildTime = std::max(buildTime, node->GetMTime());
  }
  if (!this->SampledTransform || this->SampledTransformNode != transformNode || buildTime > this->SampledTransformBuildTime)
  {
    // A new object rather than the same one rebuilt, so that a sampler that holds it is
    // modified even if the concatenated transforms are older than the last sampling
    this->SampledTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    transformNode->GetTransformToWorld(this->SampledTransform);
    this->SampledTransformNode = transformNode;
    this->SampledTransformBuildTime = buildTime;
  }
  return this->SampledTransform;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ConfigureTransformSampler(vtkVectorFieldSampler* sampler)
{
  sampler->SetFieldSourceToTransform();
  sampler->SetTransform(this->GetSampledTransform());
  this->GetPipeline()->UpdateSamplerRegion(sampler);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLTransformDisplayNode::GetFieldConnection()
{
  if (!this->CanSampleAtArbitraryPositions())
  {
    return nullptr;
  }
  // A transform is defined everywhere, so there is nothing to draw until the user says
  // which region to draw it in, or which points to place glyphs at.
  if (!this->GetSamplingRegionNodeOtherThanSource())
  {
    return nullptr;
  }
  vtkVectorFieldSampler* sampler = this->GetPipeline()->GetFieldSampler();
  this->ConfigureTransformSampler(sampler);
  return sampler->GetOutputPort();
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::UpdateSliceFieldSampler(vtkVectorFieldSampler* sampler)
{
  // A transform is defined everywhere, so a slice view always samples it in its own plane,
  // with or without a region. The base class only knows how to sample meshes and volumes.
  if (!sampler || !this->CanSampleAtArbitraryPositions())
  {
    return false;
  }
  this->ConfigureTransformSampler(sampler);
  return true;
}

//----------------------------------------------------------------------------
// Display options
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTransformDisplayNode::GetRegionNode()
{
  return this->GetSamplingRegionNode();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetAndObserveRegionNode(vtkMRMLNode* node)
{
  this->SetAndObserveSamplingRegionNode(node);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTransformDisplayNode::GetGlyphPointsNode()
{
  // The glyph points of earlier versions are a sampling region whose points are sampled
  vtkMRMLNode* samplingRegionNode = this->GetSamplingRegionNode();
  if (samplingRegionNode && this->GetEffectiveSamplingRegionPart() == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints)
  {
    return samplingRegionNode;
  }
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetAndObserveGlyphPointsNode(vtkMRMLNode* node)
{
  // The glyph points are the sampling region now, so they replace the region the transform
  // was drawn in; clearing them clears the sampling region if they were it
  if (node)
  {
    this->SetAndObserveSamplingRegionNode(node);
  }
  else if (this->GetGlyphPointsNode())
  {
    this->SetAndObserveSamplingRegionNode(nullptr);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetGlyphScalePercent(double scalePercent)
{
  this->SetGlyphScaleFactor(scalePercent * 0.01);
}

//----------------------------------------------------------------------------
double vtkMRMLTransformDisplayNode::GetGlyphScalePercent()
{
  return this->GetGlyphScaleFactor() * 100.0;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetGlyphDisplayRangeMinMm(double minMm)
{
  this->SetThresholdRange(minMm, this->ThresholdRange[1]);
  this->SetThresholdEnabled(true);
}

//----------------------------------------------------------------------------
double vtkMRMLTransformDisplayNode::GetGlyphDisplayRangeMinMm()
{
  return this->GetThresholdRange()[0];
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetGlyphDisplayRangeMaxMm(double maxMm)
{
  this->SetThresholdRange(this->ThresholdRange[0], maxMm);
  this->SetThresholdEnabled(true);
}

//----------------------------------------------------------------------------
double vtkMRMLTransformDisplayNode::GetGlyphDisplayRangeMaxMm()
{
  return this->GetThresholdRange()[1];
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::ConvertVisualizationModeToString(int modeIndex)
{
  switch (modeIndex)
  {
    case VIS_MODE_GLYPH: return "GLYPH";
    case VIS_MODE_GRID: return "GRID";
    case VIS_MODE_CONTOUR: return "CONTOUR";
    default: return "";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNode::ConvertVisualizationModeFromString(const char* modeString)
{
  if (!modeString)
  {
    return -1;
  }
  for (int modeIndex = 0; modeIndex < VIS_MODE_LAST; ++modeIndex)
  {
    if (strcmp(modeString, ConvertVisualizationModeToString(modeIndex)) == 0)
    {
      return modeIndex;
    }
  }
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::ConvertGlyphTypeToString(int typeIndex)
{
  // The shared node knows more glyph types than transform display offered; the ones it
  // offered keep their old names so that saved scenes keep working.
  switch (typeIndex)
  {
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow: return "ARROW";
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeCone: return "CONE";
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere: return "SPHERE";
    default: return vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString(typeIndex);
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNode::ConvertGlyphTypeFromString(const char* typeString)
{
  if (!typeString)
  {
    return -1;
  }
  if (strcmp(typeString, "ARROW") == 0)
  {
    return vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow;
  }
  if (strcmp(typeString, "CONE") == 0)
  {
    return vtkMRMLVectorFieldDisplayNode::GlyphTypeCone;
  }
  if (strcmp(typeString, "SPHERE") == 0)
  {
    return vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere;
  }
  return vtkMRMLVectorFieldDisplayNode::GetGlyphTypeFromString(typeString);
}

//----------------------------------------------------------------------------
// Contour levels
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetContourLevelsMm(double* levels, int size)
{
  this->SetContourLevels(levels, size);
}

//----------------------------------------------------------------------------
double* vtkMRMLTransformDisplayNode::GetContourLevelsMm()
{
  return this->GetContourLevels();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::GetContourLevelsMm(std::vector<double>& levels)
{
  this->GetContourLevels(levels);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::GetContourLevelsMmAsString()
{
  return this->GetContourLevelsAsString();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetContourLevelsMmFromString(const char* str)
{
  this->SetContourLevelsAsString(str ? str : "");
}

//----------------------------------------------------------------------------
std::vector<double> vtkMRMLTransformDisplayNode::ConvertContourLevelsFromString(const char* str)
{
  return StringToDoubleVector(str);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::ConvertContourLevelsToString(const std::vector<double>& levels)
{
  return DoubleVectorToString(levels.data(), static_cast<int>(levels.size()));
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::IsContourLevelEqual(const std::vector<double>& levels1, const std::vector<double>& levels2)
{
  if (levels1.size() != levels2.size())
  {
    return false;
  }
  const double COMPARISON_TOLERANCE = 0.01;
  for (unsigned int i = 0; i < levels1.size(); i++)
  {
    if (fabs(levels1[i] - levels2[i]) > COMPARISON_TOLERANCE)
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
std::vector<double> vtkMRMLTransformDisplayNode::StringToDoubleVector(const char* sourceStr)
{
  std::vector<double> values;
  if (!sourceStr)
  {
    return values;
  }
  std::stringstream ss(sourceStr);
  std::string itemString;
  while (std::getline(ss, itemString, CONTOUR_LEVEL_SEPARATOR))
  {
    if (itemString.empty())
    {
      continue;
    }
    std::stringstream itemStream(itemString);
    double value = 0.0;
    itemStream >> value;
    if (!itemStream.fail())
    {
      values.push_back(value);
    }
  }
  return values;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::DoubleVectorToString(const double* values, int numberOfValues)
{
  std::stringstream ss;
  for (int i = 0; i < numberOfValues; i++)
  {
    if (i > 0)
    {
      ss << CONTOUR_LEVEL_SEPARATOR;
    }
    ss << values[i];
  }
  return ss.str();
}

//----------------------------------------------------------------------------
// Interaction parameters, forwarded to the interaction display node
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkMRMLTransformInteractionDisplayNode* vtkMRMLTransformDisplayNode::GetInteractionDisplayNode(bool createIfMissing /*=false*/)
{
  vtkMRMLDisplayableNode* displayableNode = this->GetDisplayableNode();
  if (!displayableNode)
  {
    return nullptr;
  }
  int numberOfDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; ++i)
  {
    vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = vtkMRMLTransformInteractionDisplayNode::SafeDownCast(displayableNode->GetNthDisplayNode(i));
    if (interactionDisplayNode)
    {
      return interactionDisplayNode;
    }
  }
  if (!createIfMissing || !this->GetScene())
  {
    return nullptr;
  }
  vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode =
    vtkMRMLTransformInteractionDisplayNode::SafeDownCast(this->GetScene()->AddNewNodeByClass("vtkMRMLTransformInteractionDisplayNode"));
  if (interactionDisplayNode)
  {
    displayableNode->AddAndObserveDisplayNodeID(interactionDisplayNode->GetID());
  }
  return interactionDisplayNode;
}

/// Forwards a property to the interaction display node. The node is only created when a
/// property is set, so that a transform that is never edited does not get one.
#define INTERACTION_PROPERTY_FORWARDER(type, name, defaultValue)                                            \
  type vtkMRMLTransformDisplayNode::Get##name()                                                             \
  {                                                                                                         \
    vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode();     \
    return interactionDisplayNode ? interactionDisplayNode->Get##name() : defaultValue;                     \
  }                                                                                                         \
  void vtkMRMLTransformDisplayNode::Set##name(type value)                                                   \
  {                                                                                                         \
    vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode(true); \
    if (interactionDisplayNode)                                                                             \
    {                                                                                                       \
      interactionDisplayNode->Set##name(value);                                                             \
    }                                                                                                       \
    /* Observers of this node expect to hear about the change, wherever it is stored now */                 \
    this->Modified();                                                                                       \
  }

INTERACTION_PROPERTY_FORWARDER(bool, EditorVisibility, false);
INTERACTION_PROPERTY_FORWARDER(bool, EditorVisibility3D, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorSliceIntersectionVisibility, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorTranslationEnabled, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorTranslationSliceEnabled, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorTranslationSliceAnywhereEnabled, false);
INTERACTION_PROPERTY_FORWARDER(double, EditorTranslationSliceAnywhereSensitivity, 0.2);
INTERACTION_PROPERTY_FORWARDER(bool, EditorRotationEnabled, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorRotationSliceEnabled, true);
INTERACTION_PROPERTY_FORWARDER(bool, EditorScalingEnabled, false);
INTERACTION_PROPERTY_FORWARDER(bool, EditorScalingSliceEnabled, false);
INTERACTION_PROPERTY_FORWARDER(double, InteractionSizeMm, 5.0);
INTERACTION_PROPERTY_FORWARDER(double, InteractionScalePercent, 15.0);
INTERACTION_PROPERTY_FORWARDER(bool, InteractionSizeAbsolute, false);
INTERACTION_PROPERTY_FORWARDER(double, InteractionHandleOpacity, 1.0);
INTERACTION_PROPERTY_FORWARDER(int, ActiveInteractionType, -1);
INTERACTION_PROPERTY_FORWARDER(int, ActiveInteractionIndex, -1);

/// Forwards a handle visibility vector to the interaction display node.
#define INTERACTION_VISIBILITY_FORWARDER(name)                                                              \
  void vtkMRMLTransformDisplayNode::Get##name(bool visibility[4])                                           \
  {                                                                                                         \
    vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode();     \
    if (interactionDisplayNode)                                                                             \
    {                                                                                                       \
      interactionDisplayNode->Get##name(visibility);                                                        \
      return;                                                                                               \
    }                                                                                                       \
    for (int i = 0; i < 4; ++i)                                                                             \
    {                                                                                                       \
      visibility[i] = true;                                                                                 \
    }                                                                                                       \
  }                                                                                                         \
  void vtkMRMLTransformDisplayNode::Set##name(bool visibility[4])                                           \
  {                                                                                                         \
    vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode(true); \
    if (interactionDisplayNode)                                                                             \
    {                                                                                                       \
      interactionDisplayNode->Set##name(visibility);                                                        \
    }                                                                                                       \
    this->Modified();                                                                                       \
  }

INTERACTION_VISIBILITY_FORWARDER(RotationHandleComponentVisibility3D);
INTERACTION_VISIBILITY_FORWARDER(ScaleHandleComponentVisibility3D);
INTERACTION_VISIBILITY_FORWARDER(TranslationHandleComponentVisibility3D);
INTERACTION_VISIBILITY_FORWARDER(RotationHandleComponentVisibilitySlice);
INTERACTION_VISIBILITY_FORWARDER(ScaleHandleComponentVisibilitySlice);
INTERACTION_VISIBILITY_FORWARDER(TranslationHandleComponentVisibilitySlice);

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::UpdateEditorBounds()
{
  vtkMRMLTransformInteractionDisplayNode* interactionDisplayNode = this->GetInteractionDisplayNode();
  if (interactionDisplayNode)
  {
    interactionDisplayNode->UpdateEditorBounds();
  }
  this->InvokeEvent(vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent);
}

//----------------------------------------------------------------------------
// Color map
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetDefaultColors()
{
  if (!this->GetScene())
  {
    vtkErrorMacro("vtkMRMLTransformDisplayNode::SetDefaultColors failed: scene is not set");
    return;
  }

  // Create and set a new color table node
  vtkNew<vtkMRMLProceduralColorNode> colorNode;
  colorNode->SetName(this->GetScene()->GenerateUniqueName(DEFAULT_COLOR_TABLE_NAME).c_str());
  colorNode->SetAttribute("Category", "Transform display");
  // The color node is a procedural color node, which is saved using a storage node.
  // Hidden nodes are not saved if they use a storage node, therefore
  // the color node must be visible.
  colorNode->SetHideFromEditors(false);

  vtkColorTransferFunction* colorMap = colorNode->GetColorTransferFunction();
  // Map: mm -> RGB
  colorMap->AddRGBPoint(1.0, 0.2, 0.2, 0.2);
  colorMap->AddRGBPoint(2.0, 0.0, 1.0, 0.0);
  colorMap->AddRGBPoint(5.0, 1.0, 1.0, 0.0);
  colorMap->AddRGBPoint(10.0, 1.0, 0.0, 0.0);

  this->GetScene()->AddNode(colorNode.GetPointer());
  this->SetAndObserveColorNodeID(colorNode->GetID());
  // The color map is what defines the scalar range, so the range is only meaningful once
  // the map exists
  this->UpdateScalarRange();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(caller);
  if (colorNode && colorNode == this->GetColorNode() && event == vtkCommand::ModifiedEvent)
  {
    // Editing the color map moves the range that the displacements are mapped to
    this->UpdateScalarRange();
  }
}

//----------------------------------------------------------------------------
vtkColorTransferFunction* vtkMRMLTransformDisplayNode::GetColorMap()
{
  vtkMRMLProceduralColorNode* colorNode = vtkMRMLProceduralColorNode::SafeDownCast(GetColorNode());
  if (colorNode == nullptr                                //
      || colorNode->GetColorTransferFunction() == nullptr //
      || colorNode->GetColorTransferFunction()->GetSize() == 0)
  {
    // We don't have a color node or it is not the right type
    this->SetDefaultColors();
    colorNode = vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
    if (colorNode == nullptr)
    {
      vtkErrorMacro("vtkMRMLTransformDisplayNode::GetColorMap failed: could not create default color node");
      return nullptr;
    }
  }
  vtkColorTransferFunction* colorMap = colorNode->GetColorTransferFunction();
  return colorMap;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetColorMap(vtkColorTransferFunction* newColorMap)
{
  int oldModified = this->StartModify();
  vtkMRMLProceduralColorNode* colorNode = vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
  if (colorNode == nullptr)
  {
    // We don't have a color node or it is not the right type
    this->SetDefaultColors();
    colorNode = vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
  }
  if (colorNode != nullptr)
  {
    if (colorNode->GetColorTransferFunction() == nullptr)
    {
      vtkNew<vtkColorTransferFunction> ctf;
      colorNode->SetAndObserveColorTransferFunction(ctf.GetPointer());
    }
    if (!vtkMRMLProceduralColorNode::IsColorMapEqual(colorNode->GetColorTransferFunction(), newColorMap))
    {
      colorNode->GetColorTransferFunction()->DeepCopy(newColorMap);
    }
  }
  else
  {
    vtkErrorMacro("vtkMRMLTransformDisplayNode::SetColorMap failed: could not create default color node");
  }
  this->EndModify(oldModified);
}
