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

#include "vtkObjectFactory.h"

#include "vtkMRMLTransformDisplayNode.h"

#include <sstream>

namespace
{
const char RegionReferenceRole[] = "region";
const char GlyphPointsReferenceRole[] = "glyphPoints";
/// Attributes of the displacement field visualization that earlier Slicer versions wrote
const char* const LegacyVisualizationAttributeNames[] = {
  "VisualizationMode",     "GlyphSpacingMm",  "GlyphScalePercent",         "GlyphDisplayRangeMaxMm", "GlyphDisplayRangeMinMm", "GlyphType",
  "GlyphTipLengthPercent", "GlyphDiameterMm", "GlyphShaftDiameterPercent", "GlyphResolution",        "GlyphResolution2D",      "GlyphTipLengthPercent2D",
  "GridScalePercent",      "GridSpacingMm",   "GridLineDiameterMm",        "GridResolutionMm",       "GridShowNonWarped",      "ContourResolutionMm",
  "ContourLevelsMm",       "ContourOpacity",
};
} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::vtkMRMLTransformDisplayNode()
  : vtkMRMLDisplayNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLTransformDisplayNode", "Transform Display Node");

  // Don't show transform nodes by default
  // to allow the users to adjust visualization parameters first
  this->Visibility = 0;
  this->Visibility2D = 0;
  // If global visibility is turned on then 3D will show up
  this->Visibility3D = 1;

  this->EditorVisibility = false;
  this->EditorVisibility3D = true;
  this->EditorSliceIntersectionVisibility = true;
  this->EditorTranslationEnabled = true;
  this->EditorTranslationSliceEnabled = true;
  this->EditorTranslationSliceAnywhereEnabled = false;
  this->EditorTranslationSliceAnywhereSensitivity = 0.2;
  this->EditorRotationEnabled = true;
  this->EditorRotationSliceEnabled = true;
  this->EditorScalingEnabled = false;
  this->EditorScalingSliceEnabled = false;

  for (int i = 0; i < 4; ++i)
  {
    this->RotationHandleComponentVisibility3D[i] = true;
    this->ScaleHandleComponentVisibility3D[i] = true;
    this->TranslationHandleComponentVisibility3D[i] = true;

    this->RotationHandleComponentVisibilitySlice[i] = false;
    this->ScaleHandleComponentVisibilitySlice[i] = true;
    this->TranslationHandleComponentVisibilitySlice[i] = false;
  }
  this->RotationHandleComponentVisibility3D[3] = false;
  this->RotationHandleComponentVisibilitySlice[3] = true;
  this->TranslationHandleComponentVisibilitySlice[3] = true;

  // Read and written as attributes under the same names as before, so that they are kept until they are converted
  this->AddNodeReferenceRole(RegionReferenceRole, RegionReferenceRole);
  this->AddNodeReferenceRole(GlyphPointsReferenceRole, GlyphPointsReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::~vtkMRMLTransformDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);

  // As they were read
  for (const auto& nameAndValue : this->LegacyVisualizationAttributes)
  {
    of << " " << nameAndValue.first << "=\"" << this->XMLAttributeEncodeString(nameAndValue.second) << "\"";
  }

  of << " EditorVisibility=\"" << this->EditorVisibility << "\"";
  of << " EditorSliceIntersectionVisibility=\"" << this->EditorSliceIntersectionVisibility << "\"";
  of << " EditorTranslationEnabled=\"" << this->EditorTranslationEnabled << "\"";
  of << " EditorRotationEnabled=\"" << this->EditorRotationEnabled << "\"";
  of << " EditorScalingEnabled=\"" << this->EditorScalingEnabled << "\"";

  vtkMRMLWriteXMLBooleanMacro(EditorVisibility3D, EditorVisibility3D);
  vtkMRMLWriteXMLBooleanMacro(EditorTranslationSliceEnabled, EditorTranslationSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorRotationSliceEnabled, EditorRotationSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorScalingSliceEnabled, EditorScalingSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorTranslationSliceAnywhereEnabled, EditorTranslationSliceAnywhereEnabled);
  vtkMRMLWriteXMLFloatMacro(EditorTranslationSliceAnywhereSensitivity, EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLWriteXMLFloatMacro(InteractionSizeAbsolute, InteractionSizeAbsolute);
  vtkMRMLWriteXMLFloatMacro(InteractionSizeMm, InteractionSizeMm);
  vtkMRMLWriteXMLFloatMacro(InteractionScalePercent, InteractionScalePercent);
  vtkMRMLWriteXMLFloatMacro(InteractionHandleOpacity, InteractionHandleOpacity);
  vtkMRMLWriteXMLVectorMacro(TranslationHandleComponentVisibility3D, TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(RotationHandleComponentVisibility3D, RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(ScaleHandleComponentVisibility3D, ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(TranslationHandleComponentVisibilitySlice, TranslationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLWriteXMLVectorMacro(RotationHandleComponentVisibilitySlice, RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLWriteXMLVectorMacro(ScaleHandleComponentVisibilitySlice, ScaleHandleComponentVisibilitySlice, bool, 4);

  vtkMRMLWriteXMLEndMacro();
}

#define READ_FROM_ATT(varName)           \
  if (!strcmp(xmlReadAttName, #varName)) \
  {                                      \
    std::stringstream ss;                \
    ss << xmlReadAttValue;               \
    ss >> this->varName;                 \
    continue;                            \
  }

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  bool legacyVisualizationAttribute = false;
  for (const char* legacyAttributeName : LegacyVisualizationAttributeNames)
  {
    if (!strcmp(xmlReadAttName, legacyAttributeName))
    {
      this->LegacyVisualizationAttributes[legacyAttributeName] = xmlReadAttValue;
      legacyVisualizationAttribute = true;
      break;
    }
  }
  if (legacyVisualizationAttribute)
  {
    continue;
  }
  READ_FROM_ATT(EditorVisibility);
  READ_FROM_ATT(EditorSliceIntersectionVisibility);
  READ_FROM_ATT(EditorTranslationEnabled);
  READ_FROM_ATT(EditorRotationEnabled);
  READ_FROM_ATT(EditorScalingEnabled);

  vtkMRMLReadXMLBooleanMacro(EditorVisibility3D, EditorVisibility3D);
  vtkMRMLReadXMLBooleanMacro(EditorTranslationSliceEnabled, EditorTranslationSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorRotationSliceEnabled, EditorRotationSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorScalingSliceEnabled, EditorScalingSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorTranslationSliceAnywhereEnabled, EditorTranslationSliceAnywhereEnabled);
  vtkMRMLReadXMLFloatMacro(EditorTranslationSliceAnywhereSensitivity, EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLReadXMLFloatMacro(InteractionSizeAbsolute, InteractionSizeAbsolute);
  vtkMRMLReadXMLFloatMacro(InteractionSizeMm, InteractionSizeMm);
  vtkMRMLReadXMLFloatMacro(InteractionScalePercent, InteractionScalePercent);
  vtkMRMLReadXMLFloatMacro(InteractionHandleOpacity, InteractionHandleOpacity);

  vtkMRMLReadXMLVectorMacro(RotationHandleComponentVisibility3D, RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(ScaleHandleComponentVisibility3D, ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(TranslationHandleComponentVisibility3D, TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(RotationHandleComponentVisibilitySlice, RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLReadXMLVectorMacro(ScaleHandleComponentVisibilitySlice, ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLReadXMLVectorMacro(TranslationHandleComponentVisibilitySlice, TranslationHandleComponentVisibilitySlice, bool, 4);

  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLTransformDisplayNode* node = vtkMRMLTransformDisplayNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);

  if (this->LegacyVisualizationAttributes != node->LegacyVisualizationAttributes)
  {
    this->LegacyVisualizationAttributes = node->LegacyVisualizationAttributes;
    this->Modified();
  }
  vtkMRMLCopyBooleanMacro(EditorVisibility);
  vtkMRMLCopyBooleanMacro(EditorSliceIntersectionVisibility);
  vtkMRMLCopyBooleanMacro(EditorTranslationEnabled);
  vtkMRMLCopyBooleanMacro(EditorRotationEnabled);
  vtkMRMLCopyBooleanMacro(EditorScalingEnabled);
  vtkMRMLCopyBooleanMacro(EditorVisibility3D);
  vtkMRMLCopyBooleanMacro(EditorTranslationSliceAnywhereEnabled);
  vtkMRMLCopyFloatMacro(EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLCopyBooleanMacro(EditorTranslationSliceEnabled);
  vtkMRMLCopyBooleanMacro(EditorRotationSliceEnabled);
  vtkMRMLCopyBooleanMacro(EditorScalingSliceEnabled);
  vtkMRMLCopyBooleanMacro(InteractionSizeAbsolute);
  vtkMRMLCopyFloatMacro(InteractionSizeMm);
  vtkMRMLCopyFloatMacro(InteractionScalePercent);
  vtkMRMLCopyFloatMacro(InteractionHandleOpacity);
  vtkMRMLCopyVectorMacro(RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLCopyVectorMacro(ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLCopyVectorMacro(TranslationHandleComponentVisibilitySlice, bool, 4);

  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);

  for (const auto& nameAndValue : this->LegacyVisualizationAttributes)
  {
    os << indent << "Legacy " << nameAndValue.first << " = " << nameAndValue.second << "\n";
  }

  os << indent << " EditorVisibility=\"" << this->EditorVisibility << "\n";
  os << indent << " EditorSliceIntersectionVisibility=\"" << this->EditorSliceIntersectionVisibility << "\n";
  os << indent << " EditorTranslationEnabled=\"" << this->EditorTranslationEnabled << "\n";
  os << indent << " EditorRotationEnabled=\"" << this->EditorRotationEnabled << "\n";
  os << indent << " EditorScalingEnabled=\"" << this->EditorScalingEnabled << "\n";

  vtkMRMLPrintBooleanMacro(EditorVisibility3D);
  vtkMRMLPrintBooleanMacro(EditorTranslationSliceAnywhereEnabled);
  vtkMRMLPrintFloatMacro(EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLPrintFloatMacro(InteractionHandleOpacity);
  vtkMRMLPrintVectorMacro(RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLPrintVectorMacro(ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLPrintVectorMacro(TranslationHandleComponentVisibilitySlice, bool, 4);

  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::HasLegacyVisualization()
{
  return (!this->LegacyVisualizationAttributes.empty() //
          || this->GetNodeReference(RegionReferenceRole) != nullptr || this->GetNodeReference(GlyphPointsReferenceRole) != nullptr);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::GetLegacyVisualizationAttribute(const std::string& attributeName)
{
  auto nameAndValue = this->LegacyVisualizationAttributes.find(attributeName);
  return (nameAndValue != this->LegacyVisualizationAttributes.end() ? nameAndValue->second : std::string());
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::GetLegacyRegionNodeReferenceRole()
{
  return RegionReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::GetLegacyGlyphPointsNodeReferenceRole()
{
  return GlyphPointsReferenceRole;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ClearLegacyVisualization()
{
  MRMLNodeModifyBlocker blocker(this);
  if (!this->LegacyVisualizationAttributes.empty())
  {
    this->LegacyVisualizationAttributes.clear();
    this->Modified();
  }
  this->RemoveNodeReferenceIDs(RegionReferenceRole);
  this->RemoveNodeReferenceIDs(GlyphPointsReferenceRole);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::UpdateEditorBounds()
{
  this->InvokeEvent(vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent);
}
