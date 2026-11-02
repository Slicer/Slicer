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
// MRML includes
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLVectorFieldMask.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkVectorFieldGlyph.h"
#include "vtkVectorFieldSampler.h"
#include "vtkMRMLVolumeNode.h"

// Segmentation includes
#include <vtkSegmentation.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkCellTypes.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

//----------------------------------------------------------------------------
namespace
{
/// Node reference role of the node that the samples are taken from.
const char SamplingRegionReferenceRole[] = "samplingRegion";
/// Node reference role of the node that limits where the field is shown.
const char MaskingRegionReferenceRole[] = "maskingRegion";
/// Node reference role of the display node whose coloring this one follows.
const char ColoringSourceReferenceRole[] = "coloringSource";

//----------------------------------------------------------------------------
/// True if a string is null or empty
bool IsEmpty(const char* text)
{
  return (!text || text[0] == '\0');
}

//----------------------------------------------------------------------------
/// The id, below idCount, whose name asString gives, or -1 if none has that name
int GetIdFromString(const char* name, int idCount, const char* (*asString)(int))
{
  if (!name)
  {
    return -1;
  }
  for (int id = 0; id < idCount; ++id)
  {
    if (strcmp(name, asString(id)) == 0)
    {
      return id;
    }
  }
  return -1;
}
} // namespace

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVectorFieldDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode::vtkMRMLVectorFieldDisplayNode()
{
  // A vector field says most when the length of the vectors is visible in their color too
  this->ScalarVisibility = 1;
  // Glyphs are shown in slice views by default (the base class default is off, because for
  // most display nodes the slice view representation is only an intersection contour).
  this->Visibility2D = 1;
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLVectorFieldDisplayNode", "Vector Field Display");
  // The display node whose coloring is followed is observed however the link is made,
  // including when it is read from a scene
  vtkNew<vtkIntArray> coloringSourceEvents;
  coloringSourceEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ColoringSourceReferenceRole, nullptr, coloringSourceEvents);
  // The same for the region nodes: without registered events a reference read from a scene
  // would not be observed, and the field would not follow changes of the region.
  vtkNew<vtkIntArray> regionEvents;
  vtkMRMLVectorFieldDisplayNode::AddSamplingEvents(regionEvents);
  this->AddNodeReferenceRole(SamplingRegionReferenceRole, nullptr, regionEvents);
  this->AddNodeReferenceRole(MaskingRegionReferenceRole, nullptr, regionEvents);
}

//-----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode::~vtkMRMLVectorFieldDisplayNode()
{
  if (this->Pipeline)
  {
    this->Pipeline->SetDisplayNode(nullptr);
  }
  this->SetGlyphOrientationArrayName(nullptr);
  this->SetGlyphScaleArrayName(nullptr);
  // Not through SetFieldArrayName(): it updates other arrays, calls SetActiveScalar(), recomputes the scale factor
  // and invokes events, none of which is safe once the subclass parts are destroyed.
  delete[] this->FieldArrayName;
  this->FieldArrayName = nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(GlyphType);
  vtkMRMLPrintStringMacro(GlyphOrientationArrayName);
  vtkMRMLPrintStringMacro(GlyphScaleArrayName);
  vtkMRMLPrintStringMacro(FieldArrayName);
  vtkMRMLPrintBooleanMacro(GlyphScaleByComponents);
  vtkMRMLPrintFloatMacro(GlyphScaleFactor);
  vtkMRMLPrintEnumMacro(SamplingRegionPart);
  vtkMRMLPrintEnumMacro(SamplingPattern);
  vtkMRMLPrintIntMacro(SamplingPointsNumber);
  vtkMRMLPrintBooleanMacro(MaskingShowInside);
  vtkMRMLPrintStdStringVectorMacro(MaskingSegmentIDs, std::vector);
  vtkMRMLPrintEnumMacro(VisualizationMode);
  vtkMRMLPrintBooleanMacro(GlyphScaleDirectional);
  vtkMRMLPrintFloatMacro(GlyphDiameter);
  vtkMRMLPrintBooleanMacro(GlyphDiameterAbsolute);
  vtkMRMLPrintFloatMacro(GlyphDiameterPercent);
  vtkMRMLPrintFloatMacro(GridSpacing);
  vtkMRMLPrintBooleanMacro(GridShowNonWarped);
  vtkMRMLPrintVectorMacro(GridNonWarpedColor, double, 3);
  vtkMRMLPrintBooleanMacro(GridFill);
  vtkMRMLPrintBooleanMacro(GridLinesVisibility);
  vtkMRMLPrintFloatMacro(ContourOpacity3D);
  vtkMRMLPrintFloatMacro(GridScaleFactor);
  vtkMRMLPrintFloatMacro(GridLineDiameter);
  vtkMRMLPrintStdStringMacro(ContourLevelsAsString);
  vtkMRMLPrintFloatMacro(StreamlineMaximumPropagation);
  vtkMRMLPrintBooleanMacro(StreamlineBidirectional);
  vtkMRMLPrintFloatMacro(StreamlineInitialIntegrationStep);
  vtkMRMLPrintFloatMacro(StreamlineTubeDiameter);
  vtkMRMLPrintFloatMacro(StreamlineSliceFadeDistance);
  vtkMRMLPrintFloatMacro(SamplingSpacing);
  vtkMRMLPrintBooleanMacro(GlyphSliceProjectionEnabled);
  vtkMRMLPrintIntMacro(GlyphResolution);
  vtkMRMLPrintFloatMacro(GlyphTipLengthPercent);
  vtkMRMLPrintFloatMacro(GlyphShaftDiameterPercent);
  vtkMRMLPrintIntMacro(GlyphResolution2D);
  vtkMRMLPrintFloatMacro(GlyphTipLengthPercent2D);
  vtkMRMLPrintBooleanMacro(ThresholdEnabled);
  vtkMRMLPrintVectorMacro(ThresholdRange, double, 2);
  vtkMRMLPrintBooleanMacro(InterpolateBetweenPoints);
  vtkMRMLPrintFloatMacro(InterpolationMaximumDistance);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(glyphType, GlyphType);
  vtkMRMLWriteXMLStringMacro(glyphOrientationArrayName, GlyphOrientationArrayName);
  vtkMRMLWriteXMLStringMacro(glyphScaleArrayName, GlyphScaleArrayName);
  vtkMRMLWriteXMLStringMacro(fieldArrayName, FieldArrayName);
  vtkMRMLWriteXMLBooleanMacro(glyphScaleByComponents, GlyphScaleByComponents);
  vtkMRMLWriteXMLFloatMacro(glyphScaleFactor, GlyphScaleFactor);
  vtkMRMLWriteXMLEnumMacro(samplingRegionPart, SamplingRegionPart);
  vtkMRMLWriteXMLEnumMacro(samplingPattern, SamplingPattern);
  vtkMRMLWriteXMLIntMacro(samplingPointsNumber, SamplingPointsNumber);
  vtkMRMLWriteXMLBooleanMacro(maskingShowInside, MaskingShowInside);
  vtkMRMLWriteXMLStdStringVectorMacro(maskingSegmentIDs, MaskingSegmentIDs, std::vector);
  vtkMRMLWriteXMLEnumMacro(visualizationMode, VisualizationMode);
  vtkMRMLWriteXMLBooleanMacro(glyphScaleDirectional, GlyphScaleDirectional);
  vtkMRMLWriteXMLFloatMacro(glyphDiameter, GlyphDiameter);
  vtkMRMLWriteXMLBooleanMacro(glyphDiameterAbsolute, GlyphDiameterAbsolute);
  vtkMRMLWriteXMLFloatMacro(glyphDiameterPercent, GlyphDiameterPercent);
  vtkMRMLWriteXMLFloatMacro(gridSpacing, GridSpacing);
  vtkMRMLWriteXMLBooleanMacro(gridShowNonWarped, GridShowNonWarped);
  vtkMRMLWriteXMLVectorMacro(gridNonWarpedColor, GridNonWarpedColor, double, 3);
  vtkMRMLWriteXMLBooleanMacro(gridFill, GridFill);
  vtkMRMLWriteXMLBooleanMacro(gridLinesVisibility, GridLinesVisibility);
  vtkMRMLWriteXMLFloatMacro(contourOpacity3D, ContourOpacity3D);
  vtkMRMLWriteXMLFloatMacro(gridScaleFactor, GridScaleFactor);
  vtkMRMLWriteXMLFloatMacro(gridLineDiameter, GridLineDiameter);
  vtkMRMLWriteXMLStdStringMacro(contourLevels, ContourLevelsAsString);
  vtkMRMLWriteXMLFloatMacro(streamlineMaximumPropagation, StreamlineMaximumPropagation);
  vtkMRMLWriteXMLBooleanMacro(streamlineBidirectional, StreamlineBidirectional);
  vtkMRMLWriteXMLFloatMacro(streamlineInitialIntegrationStep, StreamlineInitialIntegrationStep);
  vtkMRMLWriteXMLFloatMacro(streamlineTubeDiameter, StreamlineTubeDiameter);
  vtkMRMLWriteXMLFloatMacro(streamlineSliceFadeDistance, StreamlineSliceFadeDistance);
  vtkMRMLWriteXMLFloatMacro(samplingSpacing, SamplingSpacing);
  vtkMRMLWriteXMLBooleanMacro(glyphSliceProjectionEnabled, GlyphSliceProjectionEnabled);
  vtkMRMLWriteXMLIntMacro(glyphResolution, GlyphResolution);
  vtkMRMLWriteXMLFloatMacro(glyphTipLengthPercent, GlyphTipLengthPercent);
  vtkMRMLWriteXMLFloatMacro(glyphShaftDiameterPercent, GlyphShaftDiameterPercent);
  vtkMRMLWriteXMLIntMacro(glyphResolution2D, GlyphResolution2D);
  vtkMRMLWriteXMLFloatMacro(glyphTipLengthPercent2D, GlyphTipLengthPercent2D);
  vtkMRMLWriteXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkMRMLWriteXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkMRMLWriteXMLBooleanMacro(interpolateBetweenPoints, InterpolateBetweenPoints);
  vtkMRMLWriteXMLFloatMacro(interpolationMaximumDistance, InterpolationMaximumDistance);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(glyphType, GlyphType);
  vtkMRMLReadXMLStringMacro(glyphOrientationArrayName, GlyphOrientationArrayName);
  vtkMRMLReadXMLStringMacro(glyphScaleArrayName, GlyphScaleArrayName);
  vtkMRMLReadXMLStringMacro(fieldArrayName, FieldArrayName);
  vtkMRMLReadXMLBooleanMacro(glyphScaleByComponents, GlyphScaleByComponents);
  vtkMRMLReadXMLFloatMacro(glyphScaleFactor, GlyphScaleFactor);
  vtkMRMLReadXMLEnumMacro(samplingRegionPart, SamplingRegionPart);
  vtkMRMLReadXMLEnumMacro(samplingPattern, SamplingPattern);
  vtkMRMLReadXMLIntMacro(samplingPointsNumber, SamplingPointsNumber);
  vtkMRMLReadXMLBooleanMacro(maskingShowInside, MaskingShowInside);
  vtkMRMLReadXMLStdStringVectorMacro(maskingSegmentIDs, MaskingSegmentIDs, std::vector);
  vtkMRMLReadXMLEnumMacro(visualizationMode, VisualizationMode);
  vtkMRMLReadXMLBooleanMacro(glyphScaleDirectional, GlyphScaleDirectional);
  vtkMRMLReadXMLFloatMacro(glyphDiameter, GlyphDiameter);
  vtkMRMLReadXMLBooleanMacro(glyphDiameterAbsolute, GlyphDiameterAbsolute);
  vtkMRMLReadXMLFloatMacro(glyphDiameterPercent, GlyphDiameterPercent);
  vtkMRMLReadXMLFloatMacro(gridSpacing, GridSpacing);
  vtkMRMLReadXMLBooleanMacro(gridShowNonWarped, GridShowNonWarped);
  vtkMRMLReadXMLVectorMacro(gridNonWarpedColor, GridNonWarpedColor, double, 3);
  vtkMRMLReadXMLBooleanMacro(gridFill, GridFill);
  vtkMRMLReadXMLBooleanMacro(gridLinesVisibility, GridLinesVisibility);
  vtkMRMLReadXMLFloatMacro(contourOpacity3D, ContourOpacity3D);
  // Scenes saved before the opacity was named for the 3D views it applies to
  vtkMRMLReadXMLFloatMacro(contourOpacity, ContourOpacity3D);
  vtkMRMLReadXMLFloatMacro(gridScaleFactor, GridScaleFactor);
  vtkMRMLReadXMLFloatMacro(gridLineDiameter, GridLineDiameter);
  vtkMRMLReadXMLStdStringMacro(contourLevels, ContourLevelsAsString);
  vtkMRMLReadXMLFloatMacro(streamlineMaximumPropagation, StreamlineMaximumPropagation);
  vtkMRMLReadXMLBooleanMacro(streamlineBidirectional, StreamlineBidirectional);
  vtkMRMLReadXMLFloatMacro(streamlineInitialIntegrationStep, StreamlineInitialIntegrationStep);
  vtkMRMLReadXMLFloatMacro(streamlineTubeDiameter, StreamlineTubeDiameter);
  vtkMRMLReadXMLFloatMacro(streamlineSliceFadeDistance, StreamlineSliceFadeDistance);
  vtkMRMLReadXMLFloatMacro(samplingSpacing, SamplingSpacing);
  vtkMRMLReadXMLBooleanMacro(glyphSliceProjectionEnabled, GlyphSliceProjectionEnabled);
  vtkMRMLReadXMLIntMacro(glyphResolution, GlyphResolution);
  vtkMRMLReadXMLFloatMacro(glyphTipLengthPercent, GlyphTipLengthPercent);
  vtkMRMLReadXMLFloatMacro(glyphShaftDiameterPercent, GlyphShaftDiameterPercent);
  vtkMRMLReadXMLIntMacro(glyphResolution2D, GlyphResolution2D);
  vtkMRMLReadXMLFloatMacro(glyphTipLengthPercent2D, GlyphTipLengthPercent2D);
  vtkMRMLReadXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkMRMLReadXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkMRMLReadXMLBooleanMacro(interpolateBetweenPoints, InterpolateBetweenPoints);
  vtkMRMLReadXMLFloatMacro(interpolationMaximumDistance, InterpolationMaximumDistance);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLVectorFieldDisplayNode* node = vtkMRMLVectorFieldDisplayNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(GlyphType);
  vtkMRMLCopyStringMacro(GlyphOrientationArrayName);
  vtkMRMLCopyStringMacro(GlyphScaleArrayName);
  vtkMRMLCopyStringMacro(FieldArrayName);
  vtkMRMLCopyBooleanMacro(GlyphScaleByComponents);
  vtkMRMLCopyFloatMacro(GlyphScaleFactor);
  vtkMRMLCopyEnumMacro(SamplingRegionPart);
  vtkMRMLCopyEnumMacro(SamplingPattern);
  vtkMRMLCopyIntMacro(SamplingPointsNumber);
  vtkMRMLCopyBooleanMacro(MaskingShowInside);
  this->SetMaskingSegmentIDs(node->GetMaskingSegmentIDs());
  vtkMRMLCopyEnumMacro(VisualizationMode);
  vtkMRMLCopyBooleanMacro(GlyphScaleDirectional);
  vtkMRMLCopyFloatMacro(GlyphDiameter);
  vtkMRMLCopyBooleanMacro(GlyphDiameterAbsolute);
  vtkMRMLCopyFloatMacro(GlyphDiameterPercent);
  vtkMRMLCopyFloatMacro(GridSpacing);
  vtkMRMLCopyBooleanMacro(GridShowNonWarped);
  vtkMRMLCopyVectorMacro(GridNonWarpedColor, double, 3);
  vtkMRMLCopyBooleanMacro(GridFill);
  vtkMRMLCopyBooleanMacro(GridLinesVisibility);
  vtkMRMLCopyFloatMacro(ContourOpacity3D);
  vtkMRMLCopyFloatMacro(GridScaleFactor);
  vtkMRMLCopyFloatMacro(GridLineDiameter);
  vtkMRMLCopyStdStringMacro(ContourLevelsAsString);
  vtkMRMLCopyFloatMacro(StreamlineMaximumPropagation);
  vtkMRMLCopyBooleanMacro(StreamlineBidirectional);
  vtkMRMLCopyFloatMacro(StreamlineInitialIntegrationStep);
  vtkMRMLCopyFloatMacro(StreamlineTubeDiameter);
  vtkMRMLCopyFloatMacro(StreamlineSliceFadeDistance);
  vtkMRMLCopyFloatMacro(SamplingSpacing);
  vtkMRMLCopyBooleanMacro(GlyphSliceProjectionEnabled);
  vtkMRMLCopyIntMacro(GlyphResolution);
  vtkMRMLCopyFloatMacro(GlyphTipLengthPercent);
  vtkMRMLCopyFloatMacro(GlyphShaftDiameterPercent);
  vtkMRMLCopyIntMacro(GlyphResolution2D);
  vtkMRMLCopyFloatMacro(GlyphTipLengthPercent2D);
  vtkMRMLCopyBooleanMacro(ThresholdEnabled);
  vtkMRMLCopyVectorMacro(ThresholdRange, double, 2);
  vtkMRMLCopyBooleanMacro(InterpolateBetweenPoints);
  vtkMRMLCopyFloatMacro(InterpolationMaximumDistance);
  vtkMRMLCopyEndMacro();
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString(int id)
{
  switch (id)
  {
    case GlyphTypeArrow: return "arrow";
    case GlyphTypeCone: return "cone";
    case GlyphTypeBox: return "box";
    case GlyphTypeCylinder: return "cylinder";
    case GlyphTypeLine: return "line";
    case GlyphTypeSphere: return "sphere";
    default: return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetGlyphTypeFromString(const char* name)
{
  return GetIdFromString(name, GlyphType_Last, vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString()
{
  return vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString(this->GlyphType);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetGlyphTypeFromString(const char* name)
{
  int id = vtkMRMLVectorFieldDisplayNode::GetGlyphTypeFromString(name);
  if (id < 0)
  {
    vtkWarningMacro("SetGlyphTypeFromString: invalid glyph type " << (name ? name : "(none)"));
    return;
  }
  this->SetGlyphType(id);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetVisualizationModeAsString(int id)
{
  switch (id)
  {
    case VisualizationModeGlyph: return "glyph";
    case VisualizationModeGrid: return "grid";
    case VisualizationModeContour: return "contour";
    case VisualizationModeStreamline: return "streamline";
    default: return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetVisualizationModeFromString(const char* name)
{
  return GetIdFromString(name, VisualizationMode_Last, vtkMRMLVectorFieldDisplayNode::GetVisualizationModeAsString);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetVisualizationModeAsString()
{
  return vtkMRMLVectorFieldDisplayNode::GetVisualizationModeAsString(this->VisualizationMode);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetVisualizationModeFromString(const char* name)
{
  int id = vtkMRMLVectorFieldDisplayNode::GetVisualizationModeFromString(name);
  if (id < 0)
  {
    vtkWarningMacro("SetVisualizationModeFromString: invalid visualization mode " << (name ? name : "(none)"));
    return;
  }
  this->SetVisualizationMode(id);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartAsString(int id)
{
  switch (id)
  {
    case SamplingRegionPartDefault: return "default";
    case SamplingRegionPartPoints: return "points";
    case SamplingRegionPartSurface: return "surface";
    case SamplingRegionPartVolume: return "volume";
    default: return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartFromString(const char* name)
{
  return GetIdFromString(name, SamplingRegionPart_Last, vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartAsString);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartAsString()
{
  return vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartAsString(this->SamplingRegionPart);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetSamplingRegionPartFromString(const char* name)
{
  int id = vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartFromString(name);
  if (id < 0)
  {
    vtkWarningMacro("SetSamplingRegionPartFromString: invalid sampling region part " << (name ? name : "(none)"));
    return;
  }
  this->SetSamplingRegionPart(id);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetSamplingPatternAsString(int id)
{
  switch (id)
  {
    case SamplingPatternRegular: return "regular";
    case SamplingPatternRandom: return "random";
    default: return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetSamplingPatternFromString(const char* name)
{
  return GetIdFromString(name, SamplingPattern_Last, vtkMRMLVectorFieldDisplayNode::GetSamplingPatternAsString);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetSamplingPatternAsString()
{
  return vtkMRMLVectorFieldDisplayNode::GetSamplingPatternAsString(this->SamplingPattern);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetSamplingPatternFromString(const char* name)
{
  int id = vtkMRMLVectorFieldDisplayNode::GetSamplingPatternFromString(name);
  if (id < 0)
  {
    vtkWarningMacro("SetSamplingPatternFromString: invalid sampling pattern " << (name ? name : "(none)"));
    return;
  }
  this->SetSamplingPattern(id);
}

//-----------------------------------------------------------
vtkMRMLNode* vtkMRMLVectorFieldDisplayNode::GetSamplingRegionNode()
{
  return this->GetNodeReference(SamplingRegionReferenceRole);
}
//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetAndObserveSamplingRegionNode(vtkMRMLNode* node)
{
  this->SetAndObserveSamplingRegionNodeID(node ? node->GetID() : nullptr);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetAndObserveSamplingRegionNodeID(const char* nodeID)
{
  // The events to observe come from the role, which was registered with them
  this->SetAndObserveNodeReferenceID(SamplingRegionReferenceRole, nodeID);
}
//-----------------------------------------------------------
vtkMRMLNode* vtkMRMLVectorFieldDisplayNode::GetMaskingRegionNode()
{
  return this->GetNodeReference(MaskingRegionReferenceRole);
}
//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetAndObserveMaskingRegionNode(vtkMRMLNode* node)
{
  this->SetAndObserveMaskingRegionNodeID(node ? node->GetID() : nullptr);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetAndObserveMaskingRegionNodeID(const char* nodeID)
{
  this->SetAndObserveNodeReferenceID(MaskingRegionReferenceRole, nodeID);
}

//-----------------------------------------------------------
std::vector<std::string> vtkMRMLVectorFieldDisplayNode::GetMaskingSegmentIDs()
{
  return this->MaskingSegmentIDs;
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetMaskingSegmentIDs(const std::vector<std::string>& segmentIDs)
{
  if (this->MaskingSegmentIDs == segmentIDs)
  {
    return;
  }
  this->MaskingSegmentIDs = segmentIDs;
  this->Modified();
}
//-----------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLVectorFieldDisplayNode::GetColoringSourceDisplayNode()
{
  return vtkMRMLDisplayNode::SafeDownCast(this->GetNodeReference(ColoringSourceReferenceRole));
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetAndObserveColoringSourceDisplayNode(vtkMRMLDisplayNode* node)
{
  if (node == this)
  {
    vtkErrorMacro("SetAndObserveColoringSourceDisplayNode: a display node cannot follow its own coloring");
    return;
  }
  // The events to observe come from the role, which was registered with them
  this->SetNodeReferenceID(ColoringSourceReferenceRole, node ? node->GetID() : nullptr);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::OnNodeReferenceAdded(vtkMRMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (reference && reference->GetReferenceRole() && strcmp(reference->GetReferenceRole(), ColoringSourceReferenceRole) == 0)
  {
    this->UpdateColoringFromSourceDisplayNode();
  }
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::OnNodeReferenceModified(vtkMRMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (reference && reference->GetReferenceRole() && strcmp(reference->GetReferenceRole(), ColoringSourceReferenceRole) == 0)
  {
    this->UpdateColoringFromSourceDisplayNode();
  }
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::UpdateColoringFromSourceDisplayNode()
{
  vtkMRMLDisplayNode* source = this->GetColoringSourceDisplayNode();
  if (!source || source == this)
  {
    return;
  }
  MRMLNodeModifyBlocker blocker(this);
  this->SetScalarVisibility(source->GetScalarVisibility());
  this->SetAndObserveColorNodeID(source->GetColorNodeID());
  this->SetScalarRangeFlag(source->GetScalarRangeFlag());
  const char* sourceArrayName = (source->GetActiveScalarName() ? source->GetActiveScalarName() : "");
  const char* arrayName = (this->GetActiveScalarName() ? this->GetActiveScalarName() : "");
  if (strcmp(sourceArrayName, arrayName) != 0 || source->GetActiveAttributeLocation() != this->GetActiveAttributeLocation())
  {
    this->SetActiveScalar(source->GetActiveScalarName(), source->GetActiveAttributeLocation());
  }
  // The range the other node colors with, whatever it was worked out from
  this->SetScalarRange(source->GetScalarRange());
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::AddSamplingEvents(vtkIntArray* events)
{
  if (!events)
  {
    return;
  }
  // Markups do not invoke ModifiedEvent when control points change (PointModified/Added/RemovedEvent) or when
  // transformed (TransformModifiedEvent), and all of these move where the field is sampled or seeded.
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);
  events->InsertNextValue(vtkMRMLMarkupsNode::PointAddedEvent);
  events->InsertNextValue(vtkMRMLMarkupsNode::PointRemovedEvent);
  events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionDefinedEvent);
  events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionUndefinedEvent);
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  // A model node used as a region does not invoke ModifiedEvent when only its mesh changes,
  // nor does a segmentation when its segments are edited.
  events->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);
  events->InsertNextValue(vtkSegmentation::SourceRepresentationModified);
  events->InsertNextValue(vtkSegmentation::RepresentationModified);
  events->InsertNextValue(vtkSegmentation::SegmentAdded);
  events->InsertNextValue(vtkSegmentation::SegmentRemoved);
  events->InsertNextValue(vtkSegmentation::SegmentModified);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  if (caller != nullptr && caller == this->GetColoringSourceDisplayNode())
  {
    this->UpdateColoringFromSourceDisplayNode();
    return;
  }
  // Moving, resizing or editing the sampling or masking region changes where the field is
  // sampled or shown
  if (caller != nullptr && (caller == this->GetSamplingRegionNode() || caller == this->GetMaskingRegionNode()))
  {
    this->Modified();
  }
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsSamplingRegionUsed()
{
  // Streamlines are traced on the cells of a volumetric mesh too, but from seeds that are
  // sampled in the region
  return (this->VisualizationMode != VisualizationModeContour || vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling(this));
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsSamplingSpacingUsed()
{
  if (!this->IsSamplingRegionUsed())
  {
    return false;
  }
  if (this->VisualizationMode == VisualizationModeGrid //
      || this->VisualizationMode == VisualizationModeContour)
  {
    return true;
  }
  // Glyphs and seeds: the regular pattern is spaced, except where it takes every point
  if (this->SamplingPattern == SamplingPatternRandom)
  {
    return false;
  }
  if (vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(this))
  {
    return this->IsSamplingOwnSurfaceSpaced();
  }
  vtkMRMLNode* regionNode = this->GetSamplingRegionNodeOtherThanSource();
  if (this->GetEffectiveSamplingRegionPart() == SamplingRegionPartPoints)
  {
    // Only a curve is sampled in steps; other nodes give their points as they are
    return (regionNode && regionNode->IsA("vtkMRMLMarkupsCurveNode"));
  }
  return true;
}
//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsSamplingOwnSurfaceSpaced()
{
  return (this->VisualizationMode == VisualizationModeGlyph                 //
          && vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(this) //
          && !this->GetSamplingRegionNodeOtherThanSource()                  //
          && this->GetEffectiveSamplingRegionPart() == SamplingRegionPartSurface);
}

//-----------------------------------------------------------
double vtkMRMLVectorFieldDisplayNode::GetEffectiveScaleFactor()
{
  if (this->VisualizationMode == VisualizationModeGrid)
  {
    return this->GridScaleFactor;
  }
  return this->GlyphScaleFactor;
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetEffectiveScaleFactor(double scaleFactor)
{
  if (this->VisualizationMode == VisualizationModeGrid)
  {
    this->SetGridScaleFactor(scaleFactor);
    return;
  }
  this->SetGlyphScaleFactor(scaleFactor);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsScaleFactorUsed()
{
  return (this->VisualizationMode == VisualizationModeGlyph || //
          this->VisualizationMode == VisualizationModeGrid);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsThresholdUsed()
{
  // Contour levels already choose the magnitudes, and a grid lattice is not cut by value.
  return (this->VisualizationMode == VisualizationModeGlyph //
          || this->VisualizationMode == VisualizationModeStreamline);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetFieldArrayName(const char* arrayName)
{
  std::string previousName = (this->FieldArrayName ? this->FieldArrayName : "");
  std::string newName = (arrayName ? arrayName : "");
  if (previousName == newName)
  {
    return;
  }
  MRMLNodeModifyBlocker blocker(this);

  // The arrays that were following this one keep following it. An array that was set to
  // something else stays where the user put it.
  auto follows = [&previousName](const char* name) { return (IsEmpty(name) || previousName == name); };
  bool orientationFollows = follows(this->GlyphOrientationArrayName);
  bool scaleFollows = follows(this->GlyphScaleArrayName);
  bool colorFollows = follows(this->GetActiveScalarName());

  vtkSetStringBodyMacro(FieldArrayName, arrayName);

  if (orientationFollows)
  {
    this->SetGlyphOrientationArrayName(arrayName);
  }
  if (scaleFollows)
  {
    this->SetGlyphScaleArrayName(arrayName);
  }
  if (colorFollows)
  {
    // Through SetActiveScalar, not the string setter, because that is what tells the node
    // that the array the colors are read from has changed
    this->SetActiveScalar(arrayName, this->GetActiveAttributeLocation());
  }

  // A field that has just been chosen should be visible at a sensible size straight away
  double defaultScaleFactor = this->ComputeDefaultGlyphScaleFactor();
  if (defaultScaleFactor > 0.0)
  {
    this->SetGlyphScaleFactor(defaultScaleFactor);
  }
}

//-----------------------------------------------------------
double vtkMRMLVectorFieldDisplayNode::ComputeDefaultGlyphScaleFactor()
{
  vtkDataSet* dataSet = this->GetScalarDataSet();
  vtkDataArray* vectorArray = this->GetGlyphOrientationArray();
  if (!dataSet || !vectorArray || vectorArray->GetNumberOfComponents() < 2)
  {
    return 0.0;
  }
  double magnitudeRange[2] = { 0.0, -1.0 };
  vectorArray->GetRange(magnitudeRange, -1); // -1: L2 norm over all components
  if (magnitudeRange[1] <= 0.0)
  {
    return 0.0;
  }
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  dataSet->GetBounds(bounds);
  double diagonal = vtkVectorFieldSampler::GetBoundsDiagonal(bounds);
  if (diagonal <= 0.0)
  {
    return 0.0;
  }
  // The longest vector reaches about a twentieth of the way across what is shown
  return 0.05 * diagonal / magnitudeRange[1];
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetEffectiveGlyphScaleArrayName()
{
  return (IsEmpty(this->GlyphScaleArrayName) ? this->GlyphOrientationArrayName : this->GlyphScaleArrayName);
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetEffectiveGlyphScaleArray()
{
  return this->GetFieldDataSetArray(this->GetEffectiveGlyphScaleArrayName());
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetModeVectorArrayName()
{
  // Glyphs can be oriented by any array, but a grid is warped, an isosurface is a level of
  // and a streamline follows the field itself
  bool glyphMode = (this->VisualizationMode == VisualizationModeGlyph);
  const char* preferred = (glyphMode ? this->GlyphOrientationArrayName : this->FieldArrayName);
  const char* fallback = (glyphMode ? this->FieldArrayName : this->GlyphOrientationArrayName);
  return (IsEmpty(preferred) ? fallback : preferred);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetSampledColorArrayName()
{
  // Only a mesh has arrays other than the field to color by
  if (!vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode()))
  {
    return nullptr;
  }
  const char* colorArrayName = this->GetActiveScalarName();
  if (IsEmpty(colorArrayName) || !this->GetFieldDataSetArray(colorArrayName))
  {
    return nullptr;
  }
  // Coloring by the vectors that are drawn is coloring by their magnitude, which the samplers
  // produce anyway
  const char* vectorArrayName = this->GetModeVectorArrayName();
  if (vectorArrayName && strcmp(colorArrayName, vectorArrayName) == 0)
  {
    return nullptr;
  }
  return colorArrayName;
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::GetEffectiveContourLevels(std::vector<double>& levels)
{
  this->GetContourLevels(levels);
  if (!levels.empty())
  {
    return;
  }
  // Magnitudes of an arbitrary field are not known in advance (unlike transform displacements in mm),
  // so the levels are spread over the color mapping range.
  double scalarRange[2] = { 0.0, -1.0 };
  this->GetScalarRange(scalarRange);
  if (scalarRange[1] <= scalarRange[0])
  {
    return;
  }
  const double levelPercentages[] = { 10.0, 25.0, 50.0, 75.0, 90.0 };
  for (double percentage : levelPercentages)
  {
    levels.push_back(scalarRange[0] + (scalarRange[1] - scalarRange[0]) * percentage / 100.0);
  }
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsFieldSampled()
{
  vtkAlgorithmOutput* fieldConnection = this->GetFieldConnection();
  return vtkVectorFieldSampler::ProducesSampledField(fieldConnection ? fieldConnection->GetProducer() : nullptr);
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetRenderedGlyphOrientationArrayName()
{
  if (this->IsFieldSampled())
  {
    return vtkVectorFieldSampler::GetVectorArrayName();
  }
  return this->GlyphOrientationArrayName;
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetRenderedGlyphScaleArrayName()
{
  if (this->IsFieldSampled())
  {
    // A sampled field carries the vectors and their magnitude, and nothing else, so the
    // glyphs are scaled by the vectors themselves
    return vtkVectorFieldSampler::GetVectorArrayName();
  }
  return this->GetEffectiveGlyphScaleArrayName();
}

//-----------------------------------------------------------
const char* vtkMRMLVectorFieldDisplayNode::GetRenderedActiveScalarName()
{
  if (this->IsFieldSampled())
  {
    // The sampler carries the array the field is colored by, or the field is colored by the
    // magnitude of its vectors, or by one of their components
    if (this->GetSampledColorArrayName())
    {
      return vtkVectorFieldSampler::GetColorArrayName();
    }
    return (this->IsSampledVectorComponentColored() ? vtkVectorFieldSampler::GetVectorArrayName() //
                                                    : vtkVectorFieldSampler::GetMagnitudeArrayName());
  }
  return this->GetActiveScalarName();
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsSampledVectorComponentColored()
{
  // The sampled vectors are in RAS and have 3 components
  return (this->GetActiveScalarComponent() >= 0 && this->GetActiveScalarComponent() < 3);
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetRenderedActiveScalarComponent()
{
  if (this->IsFieldSampled())
  {
    // The color array and the magnitude that the sampler produces have one component
    bool vectorsColored = (!this->GetSampledColorArrayName() && this->IsSampledVectorComponentColored());
    return (vectorsColored ? this->GetActiveScalarComponent() : 0);
  }
  return this->GetEffectiveActiveScalarComponent(this->GetActiveScalarArray());
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetActiveScalarNumberOfComponents()
{
  if (this->IsFieldSampled() && !this->GetSampledColorArrayName())
  {
    return 3;
  }
  return this->Superclass::GetActiveScalarNumberOfComponents();
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsRenderedActiveScalarMultiComponent()
{
  if (this->IsFieldSampled())
  {
    // Only the sampled vectors have more than one component
    return (!this->GetSampledColorArrayName() && this->IsSampledVectorComponentColored());
  }
  vtkDataArray* activeScalarArray = this->GetActiveScalarArray();
  return (activeScalarArray && activeScalarArray->GetNumberOfComponents() > 1);
}

//-----------------------------------------------------------
vtkDataSet* vtkMRMLVectorFieldDisplayNode::GetSampledFieldDataSet()
{
  // Without executing the pipeline: what a render has already produced, or nothing
  return vtkDataSet::SafeDownCast(vtkMRMLVectorFieldPipeline::GetConnectionOutput(this->GetFieldConnection(), false));
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetSampledFieldArray(const char* arrayName)
{
  vtkDataSet* sampledField = this->GetSampledFieldDataSet();
  return (sampledField && arrayName ? sampledField->GetPointData()->GetArray(arrayName) : nullptr);
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetRenderedActiveScalarArray()
{
  if (!this->IsFieldSampled())
  {
    return this->GetActiveScalarArray();
  }
  return this->GetSampledFieldArray(this->GetRenderedActiveScalarName());
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetRenderedGlyphScaleArray()
{
  if (!this->IsFieldSampled())
  {
    return this->GetEffectiveGlyphScaleArray();
  }
  return this->GetSampledFieldArray(this->GetRenderedGlyphScaleArrayName());
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsSamplingRegionPartSupported(int part)
{
  return vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported(this, part);
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetEffectiveSamplingRegionPart()
{
  return vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart(this);
}

//-----------------------------------------------------------
vtkMRMLNode* vtkMRMLVectorFieldDisplayNode::GetSamplingRegionNodeOtherThanSource()
{
  vtkMRMLNode* regionNode = this->GetSamplingRegionNode();
  return (regionNode == this->GetDisplayableNode() ? nullptr : regionNode);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsVisualizationModeSupported(int visualizationMode)
{
  if (visualizationMode < 0 || visualizationMode >= VisualizationMode_Last)
  {
    return false;
  }
  if (visualizationMode == VisualizationModeGlyph)
  {
    return true;
  }
  // Grid, contour and streamline modes need a field sampled through a volume, which surface mesh point data
  // cannot provide unless interpolated between its points.
  return this->CanSampleAtArbitraryPositions();
}

//-----------------------------------------------------------
int vtkMRMLVectorFieldDisplayNode::GetGridSubdivision()
{
  if (this->VisualizationMode != VisualizationModeGrid || this->GridSpacing <= 0.0)
  {
    return 1;
  }
  double requestedSpacingMm = this->GetSamplingSpacing();
  if (requestedSpacingMm <= 0.0)
  {
    // No sampling spacing requested: sample at the grid lines themselves
    return 1;
  }
  // Round up, so that the lattice is never coarser than what was asked for
  return std::max(1, static_cast<int>(std::ceil(this->GridSpacing / requestedSpacingMm - 1e-6)));
}

//-----------------------------------------------------------
double vtkMRMLVectorFieldDisplayNode::GetSamplingSpacingForField()
{
  if (this->VisualizationMode == VisualizationModeGrid && this->GridSpacing > 0.0)
  {
    return this->GridSpacing / this->GetGridSubdivision();
  }
  return this->GetSamplingSpacing();
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetDefaultSampledFieldArrayNames()
{
  // A sampled field always provides the same two arrays, so there is nothing for the user
  // to pick: orient and scale the glyphs by the vector, color them by its magnitude.
  if (IsEmpty(this->GlyphOrientationArrayName))
  {
    this->SetGlyphOrientationArrayName(vtkVectorFieldSampler::GetVectorArrayName());
  }
  if (IsEmpty(this->GlyphScaleArrayName))
  {
    this->SetGlyphScaleArrayName(vtkVectorFieldSampler::GetVectorArrayName());
  }
  if (IsEmpty(this->ActiveScalarName))
  {
    this->SetActiveScalarName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  }
}

//-----------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldDisplayNode::GetFieldConnection()
{
  return this->GetPipeline()->GetFieldConnection();
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::HasRegionTest()
{
  if (vtkMRMLVectorFieldMask::CanBeRegion(this->GetMaskingRegionNode()))
  {
    return true;
  }
  // The inside of a node is sampled over its bounding box, so samples outside a segmentation or model are dropped.
  // Slice views sample their own plane and are limited to the inside of any node.
  vtkMRMLNode* regionNode = this->GetSamplingRegionNodeOtherThanSource();
  return (regionNode && this->GetEffectiveSamplingRegionPart() == SamplingRegionPartVolume //
          && vtkMRMLVectorFieldMask::CanBeRegion(regionNode));
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsPolyDataVisualizationMode()
{
  return this->VisualizationMode != VisualizationModeGlyph;
}

//-----------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldDisplayNode::GetGlyphPointsConnection()
{
  if (this->VisualizationMode != VisualizationModeGlyph)
  {
    return nullptr;
  }
  vtkAlgorithmOutput* fieldConnection = this->GetPipeline()->GetWorldFieldConnection();
  if (!fieldConnection)
  {
    return nullptr;
  }
  vtkVectorFieldGlyph* glyph = this->GetPipeline()->GetGlyphFilter();
  return (this->GetPipeline()->UpdateGlyphFilter(glyph, fieldConnection) ? glyph->GetOutputPort(vtkVectorFieldGlyph::GlyphPointsPort) : nullptr);
}

//-----------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldDisplayNode::GetGlyphSourceConnection()
{
  vtkVectorFieldGlyph* glyph = this->GetPipeline()->GetGlyphFilter();
  this->GetPipeline()->UpdateGlyphShape(glyph);
  return glyph->GetOutputPort(vtkVectorFieldGlyph::GlyphShapePort);
}

//-----------------------------------------------------------
vtkVectorFieldGlyph* vtkMRMLVectorFieldDisplayNode::GetGlyphFilter()
{
  return this->GetPipeline()->GetGlyphFilter();
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::GetVisualizationMesh(vtkPolyData* mesh)
{
  if (!mesh)
  {
    vtkErrorMacro("GetVisualizationMesh: invalid mesh");
    return false;
  }
  mesh->Initialize();
  vtkSmartPointer<vtkPolyData> geometry;
  if (this->VisualizationMode == VisualizationModeGlyph)
  {
    if (!this->GetGlyphPointsConnection())
    {
      return false;
    }
    // Own glyph filter with the 3D view settings, so the views are not made to compute the geometry.
    // It selects the same points because the random subset is seeded the same way.
    vtkVectorFieldGlyph* viewGlyph = this->GetGlyphFilter();
    vtkNew<vtkVectorFieldGlyph> glyph;
    glyph->CopySettings(viewGlyph);
    glyph->GenerateGeometryOn();
    glyph->SetInputConnection(viewGlyph->GetInputConnection(0, 0));
    glyph->Update();
    geometry = vtkPolyData::SafeDownCast(glyph->GetOutputDataObject(vtkVectorFieldGlyph::GlyphGeometryPort));
  }
  else
  {
    // The grid, isosurfaces and streamlines are geometry already
    vtkAlgorithmOutput* geometryConnection = this->GetPipeline()->GetOutputPolyDataConnection();
    if (!geometryConnection)
    {
      return false;
    }
    geometry = vtkPolyData::SafeDownCast(vtkMRMLVectorFieldPipeline::GetConnectionOutput(geometryConnection, true));
    // The views draw the undeformed grid separately, the mesh has all that is shown
    vtkAlgorithmOutput* nonWarpedConnection = this->GetPipeline()->GetNonWarpedGridConnection();
    if (geometry && nonWarpedConnection)
    {
      vtkNew<vtkAppendPolyData> appender;
      appender->AddInputData(geometry);
      appender->AddInputConnection(nonWarpedConnection);
      appender->Update();
      geometry = appender->GetOutput();
    }
  }
  if (!geometry || geometry->GetNumberOfPoints() == 0)
  {
    return false;
  }
  // A non-linear parent transform has been applied to the geometry. A linear one is left to
  // the views, which apply it to the actor; the mesh is moved into the world here instead.
  vtkAbstractTransform* fieldToWorld = this->GetPipeline()->GetFieldToWorldTransform();
  if (fieldToWorld && !this->GetPipeline()->GetNonLinearFieldToWorldTransform())
  {
    vtkNew<vtkTransformPolyDataFilter> toWorld;
    toWorld->SetInputData(geometry);
    toWorld->SetTransform(fieldToWorld);
    toWorld->Update();
    mesh->DeepCopy(toWorld->GetOutput());
    return true;
  }
  mesh->DeepCopy(geometry);
  return true;
}

//-----------------------------------------------------------
vtkMRMLVectorFieldPipeline* vtkMRMLVectorFieldDisplayNode::GetPipeline()
{
  if (!this->Pipeline)
  {
    this->Pipeline = vtkSmartPointer<vtkMRMLVectorFieldPipeline>::New();
    this->Pipeline->SetDisplayNode(this);
  }
  return this->Pipeline;
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::HasVolumetricMesh()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode());
  vtkUnstructuredGrid* grid = (modelNode ? vtkUnstructuredGrid::SafeDownCast(modelNode->GetMesh()) : nullptr);
  if (!grid || grid->GetNumberOfCells() == 0)
  {
    return false;
  }
  vtkNew<vtkCellTypes> cellTypes;
  grid->GetCellTypes(cellTypes);
  for (vtkIdType typeIndex = 0; typeIndex < cellTypes->GetNumberOfTypes(); ++typeIndex)
  {
    if (vtkCellTypes::GetDimension(cellTypes->GetCellType(typeIndex)) == 3)
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsFieldArrayCellData()
{
  return this->IsMeshCellDataArray(this->GetModeVectorArrayName());
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsMeshCellDataArray(const char* arrayName)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode());
  vtkPointSet* mesh = (modelNode ? modelNode->GetMesh() : nullptr);
  if (!mesh || IsEmpty(arrayName))
  {
    return false;
  }
  return (mesh->GetPointData()->GetArray(arrayName) == nullptr //
          && mesh->GetCellData()->GetArray(arrayName) != nullptr);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::CanInterpolateBetweenPoints()
{
  // A volumetric mesh is interpolated exactly inside its cells without a fit, but only there:
  // a fit to its points is what extends the field beyond the mesh, up to the maximum distance
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode());
  vtkPointSet* mesh = (modelNode ? modelNode->GetMesh() : nullptr);
  return (mesh && mesh->GetNumberOfPoints() > 0);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::IsInterpolationUsed()
{
  return (this->InterpolateBetweenPoints && this->CanInterpolateBetweenPoints());
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::CanSampleAtArbitraryPositions()
{
  // A vector volume, or a mesh whose cells enclose a volume, can be interpolated anywhere; a surface mesh
  // only has values at its points unless a field is fitted to them.
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
  if (volumeNode && volumeNode->GetImageData() && volumeNode->GetImageData()->GetNumberOfScalarComponents() >= 3)
  {
    return true;
  }
  return (this->HasVolumetricMesh() || this->IsInterpolationUsed());
}

//-----------------------------------------------------------

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::UpdateSliceFieldSampler(vtkVectorFieldSampler* sampler)
{
  return this->GetPipeline()->UpdateSliceFieldSampler(sampler);
}

//-----------------------------------------------------------
bool vtkMRMLVectorFieldDisplayNode::HasFixedFieldArrays()
{
  // A sampled image always gives the same two arrays; the point data of a mesh can hold several vector arrays,
  // so the user has to choose there.
  return (vtkMRMLVolumeNode::SafeDownCast(this->GetDisplayableNode()) != nullptr);
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::GetFieldArrayInfo(std::vector<std::string>& arrayNames, std::vector<int>& numberOfComponents)
{
  arrayNames.clear();
  numberOfComponents.clear();

  // A sampled field always produces the same two arrays; report them without executing the sampler, so the GUI
  // does not sample the field. A volumetric mesh is read through a sampler, but its own arrays are offered.
  bool readsFromMesh = (vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode()) != nullptr);
  if (!readsFromMesh && this->IsFieldSampled())
  {
    arrayNames.emplace_back(vtkVectorFieldSampler::GetVectorArrayName());
    numberOfComponents.push_back(3);
    arrayNames.emplace_back(vtkVectorFieldSampler::GetMagnitudeArrayName());
    numberOfComponents.push_back(1);
    return;
  }

  vtkDataSet* fieldDataSet = this->GetScalarDataSet();
  if (!fieldDataSet)
  {
    return;
  }
  vtkPointData* pointData = fieldDataSet->GetPointData();
  if (!pointData)
  {
    return;
  }
  for (int arrayIndex = 0; arrayIndex < pointData->GetNumberOfArrays(); ++arrayIndex)
  {
    vtkDataArray* array = pointData->GetArray(arrayIndex);
    if (!array || !array->GetName())
    {
      continue;
    }
    arrayNames.emplace_back(array->GetName());
    numberOfComponents.push_back(array->GetNumberOfComponents());
  }
  // Also the cell data of a mesh, drawn at the cell centers. A cell array named like a point array cannot be
  // chosen, because arrays are found by name and point data comes first.
  vtkCellData* cellData = (readsFromMesh ? fieldDataSet->GetCellData() : nullptr);
  for (int arrayIndex = 0; cellData && arrayIndex < cellData->GetNumberOfArrays(); ++arrayIndex)
  {
    vtkDataArray* array = cellData->GetArray(arrayIndex);
    if (!array || !array->GetName() || pointData->GetArray(array->GetName()))
    {
      continue;
    }
    arrayNames.emplace_back(array->GetName());
    numberOfComponents.push_back(array->GetNumberOfComponents());
  }
}

//-----------------------------------------------------------
vtkDataSet* vtkMRMLVectorFieldDisplayNode::GetScalarDataSet()
{
  // A mesh's arrays are its own and exist whether or not the field has been sampled,
  // also for a volumetric mesh that is read through a sampler.
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode());
  if (modelNode)
  {
    return modelNode->GetMesh();
  }
  // Without updating the pipeline: sources that need sampling (transforms, images) only
  // return data that a render has already produced
  return this->GetSampledFieldDataSet();
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetFieldDataSetArray(const char* arrayName)
{
  if (IsEmpty(arrayName))
  {
    return nullptr;
  }
  vtkDataSet* dataSet = this->GetScalarDataSet();
  if (!dataSet)
  {
    return nullptr;
  }
  vtkDataArray* array = dataSet->GetPointData()->GetArray(arrayName);
  if (!array && vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode()))
  {
    // A cell data array of a mesh, which is drawn at the centers of its cells
    array = dataSet->GetCellData()->GetArray(arrayName);
  }
  return array;
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetGlyphOrientationArray()
{
  return this->GetFieldDataSetArray(this->GlyphOrientationArrayName);
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetGlyphScaleArray()
{
  return this->GetFieldDataSetArray(this->GlyphScaleArrayName);
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLVectorFieldDisplayNode::GetActiveScalarArray()
{
  return this->GetFieldDataSetArray(this->ActiveScalarName);
}

//-----------------------------------------------------------
unsigned int vtkMRMLVectorFieldDisplayNode::GetNumberOfContourLevels()
{
  return static_cast<unsigned int>(this->ContourLevels.size());
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetContourLevels(double* levels, int size)
{
  if (size < 0 || (size > 0 && !levels))
  {
    return;
  }
  std::vector<double> newLevels(levels, levels + size);
  if (newLevels == this->ContourLevels)
  {
    return;
  }
  this->ContourLevels = newLevels;
  this->Modified();
}

//-----------------------------------------------------------
double* vtkMRMLVectorFieldDisplayNode::GetContourLevels()
{
  return this->ContourLevels.empty() ? nullptr : this->ContourLevels.data();
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::GetContourLevels(std::vector<double>& levels)
{
  levels = this->ContourLevels;
}

//-----------------------------------------------------------
std::string vtkMRMLVectorFieldDisplayNode::GetContourLevelsAsString()
{
  std::stringstream levelsStream;
  for (size_t levelIndex = 0; levelIndex < this->ContourLevels.size(); ++levelIndex)
  {
    if (levelIndex > 0)
    {
      levelsStream << " ";
    }
    levelsStream << this->ContourLevels[levelIndex];
  }
  return levelsStream.str();
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::SetContourLevelsAsString(const std::string& levelsString)
{
  std::vector<double> levels;
  std::stringstream levelsStream(levelsString);
  while (!levelsStream.eof())
  {
    double level = 0.0;
    levelsStream >> level;
    if (levelsStream.fail())
    {
      break;
    }
    levels.push_back(level);
  }
  this->SetContourLevels(levels.data(), static_cast<int>(levels.size()));
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::UpdateAssignedAttribute()
{
  this->Superclass::UpdateAssignedAttribute();
  this->UpdateScalarRange();
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::UpdateScalarRange()
{
  if (this->IsUpdatingScalarRange)
  {
    return;
  }
  this->IsUpdatingScalarRange = true;
  this->UpdateScalarRangeInternal();
  this->IsUpdatingScalarRange = false;
}

//-----------------------------------------------------------
void vtkMRMLVectorFieldDisplayNode::UpdateScalarRangeInternal()
{
  // The range of the node whose coloring is followed, which may differ from what this node
  // would work out (it can be set by hand there, or come from the color table)
  vtkMRMLDisplayNode* coloringSource = this->GetColoringSourceDisplayNode();
  if (coloringSource && coloringSource != this)
  {
    this->SetScalarRange(coloringSource->GetScalarRange());
    return;
  }
  if (this->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDataScalarRange)
  {
    // A vector volume is sampled only during rendering. Interpolating voxels and rotating vectors into RAS cannot
    // exceed the voxel vector magnitudes, so their range is used before anything is drawn.
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
    vtkImageData* image = (volumeNode ? volumeNode->GetImageData() : nullptr);
    vtkDataArray* voxelVectors = (image ? image->GetPointData()->GetScalars() : nullptr);
    const char* activeScalarName = this->GetActiveScalarName();
    bool colorsBySampledField = (activeScalarName                                                                  //
                                 && (strcmp(activeScalarName, vtkVectorFieldSampler::GetMagnitudeArrayName()) == 0 //
                                     || strcmp(activeScalarName, vtkVectorFieldSampler::GetVectorArrayName()) == 0));
    // The same holds for a component, because the voxel vectors are RAS directions already.
    if (colorsBySampledField && voxelVectors && voxelVectors->GetNumberOfComponents() >= 3)
    {
      double range[2] = { 0.0, -1.0 };
      // -1: L2 norm over all components
      voxelVectors->GetRange(range, this->IsSampledVectorComponentColored() ? this->GetActiveScalarComponent() : -1);
      this->SetScalarRange(range);
      return;
    }

    vtkDataArray* activeScalarArray = this->GetActiveScalarArray();
    if (!activeScalarArray)
    {
      // Nothing to compute a range from yet, for example a field that has not been sampled.
      // An empty range would make every mapper that uses it report a bad lookup table range.
      return;
    }
    if (activeScalarArray->GetNumberOfComponents() > 1)
    {
      // Multi-component arrays are displayed by magnitude or by the chosen component (see the
      // glyph display pipelines), so the automatic range is the range of those values.
      double range[2] = { 0.0, -1.0 };
      activeScalarArray->GetRange(range, this->GetEffectiveActiveScalarComponent(activeScalarArray));
      this->SetScalarRange(range);
      return;
    }
  }
  this->Superclass::UpdateScalarRange();
}
