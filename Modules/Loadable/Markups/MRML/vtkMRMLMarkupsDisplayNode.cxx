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

==============================================================================*/

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkTextProperty.h>
#include <vtksys/SystemTools.hxx>

// STL includes
#include <sstream>

const char* vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceRole = "lineColor";
const char* vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceMRMLAttributeName = "lineColorNodeRef";

static const int SHADOW_H_OFFSET_INDEX = 0;
static const int SHADOW_V_OFFSET_INDEX = 1;
static const int SHADOW_BLUR_RADIUS_INDEX = 2;
static const int SHADOW_COLOR_INDEX = 3;

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::vtkMRMLMarkupsDisplayNode()
{
  // Markups display node settings
  this->Visibility = 1;
  this->Visibility2D = 1;
  this->VectorVisibility = 0;
  this->ScalarVisibility = 0;
  this->TensorVisibility = 0;

  this->Color[0] = 0.4;
  this->Color[1] = 1.0;
  this->Color[2] = 1.0;

  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.5;
  this->SelectedColor[2] = 0.5;

  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

  // markup display node settings
  this->TextScale = 3;
  this->GlyphType = vtkMRMLMarkupsDisplayNode::Sphere3D;
  this->GlyphScale = 1.0; // size as percent in screen size
  this->GlyphSize = 5.0;  // size in world coordinate system (mm)
  this->UseGlyphScale = true; // relative size by default

  this->SnapMode = vtkMRMLMarkupsDisplayNode::SnapModeToVisibleSurface;

  // projection settings
  this->SliceProjection = false;
  this->SliceProjectionUseFiducialColor = true;
  this->SliceProjectionOutlinedBehindSlicePlane = false;
  this->SliceProjectionColor[0] = 1.0;
  this->SliceProjectionColor[1] = 1.0;
  this->SliceProjectionColor[2] = 1.0;
  this->SliceProjectionOpacity = 0.6;

  this->PropertiesLabelVisibility = true;
  this->PointLabelsVisibility = false;
  this->FillVisibility = true;
  this->OutlineVisibility = true;
  this->FillOpacity = 0.5;
  this->OutlineOpacity = 1.0;

  // Set active component defaults for mouse (identified by empty string)
  this->ActiveComponents[GetDefaultContextName()] = ComponentInfo();

  this->CurveLineSizeMode = vtkMRMLMarkupsDisplayNode::UseLineThickness;
  this->LineThickness = 0.2;
  this->LineDiameter = 1.0;

  // Line color variables
  this->LineColorFadingStart = 1.;
  this->LineColorFadingEnd = 10.;
  this->LineColorFadingSaturation = 1.;
  this->LineColorFadingHueOffset = 0.;

  this->OccludedVisibility = false;
  this->OccludedOpacity = 0.3;

  // Text apperarance
  this->TextProperty = nullptr;
  vtkNew<vtkTextProperty> textProperty;
  textProperty->SetBackgroundOpacity(0.0);
  textProperty->SetFontSize(5);
  vtkSetAndObserveMRMLObjectMacro(this->TextProperty, textProperty);

  this->HandlesInteractive = false;

  // Line color node
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  this->AddNodeReferenceRole(this->GetLineColorNodeReferenceRole(),
                             this->GetLineColorNodeReferenceMRMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::~vtkMRMLMarkupsDisplayNode()
{
  vtkSetAndObserveMRMLObjectMacro(this->TextProperty, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(propertiesLabelVisibility, PropertiesLabelVisibility);
  vtkMRMLWriteXMLBooleanMacro(pointLabelsVisibility, PointLabelsVisibility);
  vtkMRMLWriteXMLFloatMacro(textScale, TextScale);
  vtkMRMLWriteXMLFloatMacro(glyphScale, GlyphScale);
  vtkMRMLWriteXMLFloatMacro(glyphSize, GlyphSize);
  vtkMRMLWriteXMLBooleanMacro(useGlyphScale, UseGlyphScale);
  vtkMRMLWriteXMLEnumMacro(glyphType, GlyphType);
  vtkMRMLWriteXMLEnumMacro(snapMode, SnapMode);
  vtkMRMLWriteXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkMRMLWriteXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkMRMLWriteXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLWriteXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkMRMLWriteXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkMRMLWriteXMLEnumMacro(curveLineSizeMode, CurveLineSizeMode);
  vtkMRMLWriteXMLFloatMacro(lineThickness, LineThickness);
  vtkMRMLWriteXMLFloatMacro(lineDiameter, LineDiameter);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
  vtkMRMLWriteXMLBooleanMacro(handlesInteractive, HandlesInteractive);
  vtkMRMLWriteXMLBooleanMacro(fillVisibility, FillVisibility);
  vtkMRMLWriteXMLBooleanMacro(outlineVisibility, OutlineVisibility);
  vtkMRMLWriteXMLFloatMacro(fillOpacity, FillOpacity);
  vtkMRMLWriteXMLFloatMacro(outlineOpacity, OutlineOpacity);
  vtkMRMLWriteXMLBooleanMacro(occludedVisibility, OccludedVisibility);
  vtkMRMLWriteXMLFloatMacro(occludedOpacity, OccludedOpacity);
  if (this->TextProperty)
    {
    vtkMRMLWriteXMLStdStringMacro(textProperty, TextPropertyAsString);
    }
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(propertiesLabelVisibility, PropertiesLabelVisibility);
  vtkMRMLReadXMLBooleanMacro(pointLabelsVisibility, PointLabelsVisibility);
  vtkMRMLReadXMLFloatMacro(textScale, TextScale);
  vtkMRMLReadXMLFloatMacro(glyphScale, GlyphScale);
  vtkMRMLReadXMLFloatMacro(glyphSize, GlyphSize);
  vtkMRMLReadXMLBooleanMacro(useGlyphScale, UseGlyphScale);
  vtkMRMLReadXMLEnumMacro(glyphType, GlyphType);
  vtkMRMLReadXMLEnumMacro(snapMode, SnapMode);
  vtkMRMLReadXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkMRMLReadXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkMRMLReadXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLReadXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkMRMLReadXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkMRMLReadXMLEnumMacro(curveLineSizeMode, CurveLineSizeMode);
  vtkMRMLReadXMLFloatMacro(lineThickness, LineThickness);
  vtkMRMLReadXMLFloatMacro(lineDiameter, LineDiameter);
  vtkMRMLReadXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkMRMLReadXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkMRMLReadXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkMRMLReadXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
  vtkMRMLReadXMLBooleanMacro(handlesInteractive, HandlesInteractive);
  vtkMRMLReadXMLBooleanMacro(fillVisibility, FillVisibility);
  vtkMRMLReadXMLBooleanMacro(outlineVisibility, OutlineVisibility);
  vtkMRMLReadXMLFloatMacro(fillOpacity, FillOpacity);
  vtkMRMLReadXMLFloatMacro(outlineOpacity, OutlineOpacity);
  vtkMRMLReadXMLBooleanMacro(occludedVisibility, OccludedVisibility);
  vtkMRMLReadXMLFloatMacro(occludedOpacity, OccludedOpacity);
  vtkMRMLReadXMLStdStringMacro(textProperty, TextPropertyFromString);
  vtkMRMLReadXMLEndMacro();

  // Fix up legacy markups fiducial nodes
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    // Glyph type used to be saved as an integer (not as a string enum as it is done now),
    // therefore we can use it to detect legacy scenes.
    if (!strcmp(attName, "glyphType"))
      {
      std::stringstream ss;
      int val = 0;
      ss << attValue;
      ss >> val;
      if (val > 0)
        {
        // Se glyph type from integer
        this->SetGlyphType(val);
        // Point label visibility attribute was not present in legacy scenes,
        // therefore we need to set it here.
        this->SetPointLabelsVisibility(true);
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(PropertiesLabelVisibility);
  vtkMRMLCopyBooleanMacro(PointLabelsVisibility);
  vtkMRMLCopyFloatMacro(TextScale);
  vtkMRMLCopyFloatMacro(GlyphScale);
  vtkMRMLCopyFloatMacro(GlyphSize);
  vtkMRMLCopyBooleanMacro(UseGlyphScale);
  vtkMRMLCopyEnumMacro(GlyphType);
  vtkMRMLCopyEnumMacro(SnapMode);
  vtkMRMLCopyBooleanMacro(SliceProjection);
  vtkMRMLCopyBooleanMacro(SliceProjectionUseFiducialColor);
  vtkMRMLCopyBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLCopyVectorMacro(SliceProjectionColor, double, 3);
  vtkMRMLCopyFloatMacro(SliceProjectionOpacity);
  vtkMRMLCopyEnumMacro(CurveLineSizeMode);
  vtkMRMLCopyFloatMacro(LineThickness);
  vtkMRMLCopyFloatMacro(LineDiameter);
  vtkMRMLCopyFloatMacro(LineColorFadingStart);
  vtkMRMLCopyFloatMacro(LineColorFadingEnd);
  vtkMRMLCopyFloatMacro(LineColorFadingSaturation);
  vtkMRMLCopyFloatMacro(LineColorFadingHueOffset);
  vtkMRMLCopyBooleanMacro(HandlesInteractive);
  vtkMRMLCopyBooleanMacro(FillVisibility);
  vtkMRMLCopyBooleanMacro(OutlineVisibility);
  vtkMRMLCopyFloatMacro(FillOpacity);
  vtkMRMLCopyFloatMacro(OutlineOpacity);
  vtkMRMLCopyBooleanMacro(OccludedVisibility);
  vtkMRMLCopyFloatMacro(OccludedOpacity);
  this->TextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetTextProperty());
  vtkMRMLCopyEndMacro();
}


//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString()
{
  return vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
  this->SetGlyphType(vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(glyphString));
}

//-----------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return 0;
    }
  for (int ii = 0; ii < GlyphType_Last; ii++)
    {
    if (strcmp(name, GetGlyphTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return GlyphTypeInvalid;
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString(int id)
{
  switch (id)
  {
  case Vertex2D: return "Vertex2D";
  case Dash2D: return "Dash2D";
  case Cross2D: return "Cross2D";
  case CrossDot2D: return "CrossDot2D";
  case ThickCross2D: return "ThickCross2D";
  case Triangle2D: return "Triangle2D";
  case Square2D: return "Square2D";
  case Circle2D: return "Circle2D";
  case Diamond2D: return "Diamond2D";
  case Arrow2D: return "Arrow2D";
  case ThickArrow2D: return "ThickArrow2D";
  case HookedArrow2D: return "HookedArrow2D";
  case StarBurst2D: return "StarBurst2D";
  case Sphere3D: return "Sphere3D";
  case GlyphTypeInvalid:
  default:
    // invalid id
    return "Invalid";
  }
}

//-----------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetSnapModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < SnapMode_Last; ii++)
    {
    if (strcmp(name, GetSnapModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetSnapModeAsString(int id)
{
  switch (id)
  {
  case SnapModeUnconstrained: return "unconstrained";
  case SnapModeToVisibleSurface: return "toVisibleSurface";
  default:
    // invalid id
    return "invalid";
  }
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeAsString()
{
  return vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(this->CurveLineSizeMode);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetCurveLineSizeModeFromString(const char* modeString)
{
  this->SetCurveLineSizeMode(vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(modeString));
}

//-----------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return 0;
    }
  for (int ii = 0; ii < CurveLineSizeMode_Last; ii++)
    {
    if (strcmp(name, GetCurveLineSizeModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(int id)
{
  switch (id)
  {
  case UseLineThickness: return "UseLineThickness";
  case UseLineDiameter: return "UseLineDiameter";
  default:
    // invalid id
    return "Invalid";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os,indent);
  vtkMRMLPrintBooleanMacro(PropertiesLabelVisibility);
  vtkMRMLPrintBooleanMacro(PointLabelsVisibility);
  vtkMRMLPrintFloatMacro(TextScale);
  vtkMRMLPrintFloatMacro(GlyphScale);
  vtkMRMLPrintFloatMacro(GlyphSize);
  vtkMRMLPrintBooleanMacro(UseGlyphScale);
  vtkMRMLPrintEnumMacro(GlyphType);
  vtkMRMLPrintEnumMacro(SnapMode);
  vtkMRMLPrintBooleanMacro(SliceProjection);
  vtkMRMLPrintBooleanMacro(SliceProjectionUseFiducialColor);
  vtkMRMLPrintBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLPrintVectorMacro(SliceProjectionColor, double, 3);
  vtkMRMLPrintFloatMacro(SliceProjectionOpacity);
  {
  os << indent << "ActiveComponents:   ";
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    os << it->first << ": " << it->second.Type << ", " << it->second.Index;
    }
  os << "\n";
  }
  vtkMRMLPrintEnumMacro(CurveLineSizeMode);
  vtkMRMLPrintFloatMacro(LineThickness);
  vtkMRMLPrintFloatMacro(LineDiameter);
  vtkMRMLPrintFloatMacro(LineColorFadingStart);
  vtkMRMLPrintFloatMacro(LineColorFadingEnd);
  vtkMRMLPrintFloatMacro(LineColorFadingSaturation);
  vtkMRMLPrintFloatMacro(LineColorFadingHueOffset);
  vtkMRMLPrintBooleanMacro(HandlesInteractive);
  vtkMRMLPrintBooleanMacro(FillVisibility);
  vtkMRMLPrintBooleanMacro(OutlineVisibility);
  vtkMRMLPrintFloatMacro(FillOpacity);
  vtkMRMLPrintFloatMacro(OutlineOpacity);
  vtkMRMLPrintBooleanMacro(OccludedVisibility);
  vtkMRMLPrintFloatMacro(OccludedOpacity);
  vtkMRMLPrintStdStringMacro(TextPropertyAsString);
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                  unsigned long event,
                                                  void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  if (caller == this->TextProperty)
    {
    switch (event)
      {
      case vtkCommand::ModifiedEvent:
        this->InvokeEvent(event);
      default:
        break;
      }
    }
  return;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
int  vtkMRMLMarkupsDisplayNode::GlyphTypeIs3D(int glyphType)
{
  if (glyphType == vtkMRMLMarkupsDisplayNode::Sphere3D)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetLineColorNodeID(const char *lineColorNodeID)
{
  this->SetNodeReferenceID(this->GetLineColorNodeReferenceRole(), lineColorNodeID);
}

//---------------------------------------------------------------------------
const char *vtkMRMLMarkupsDisplayNode::GetLineColorNodeID()
{
  return this->GetNodeReferenceID(this->GetLineColorNodeReferenceRole());
}

//---------------------------------------------------------------------------
vtkMRMLProceduralColorNode *vtkMRMLMarkupsDisplayNode::GetLineColorNode()
{
  return vtkMRMLProceduralColorNode::SafeDownCast(this->GetNodeReference(this->GetLineColorNodeReferenceRole()));
}

//---------------------------------------------------------------------------
const char *vtkMRMLMarkupsDisplayNode::GetLineColorNodeReferenceRole()
{
  return vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char *vtkMRMLMarkupsDisplayNode::GetLineColorNodeReferenceMRMLAttributeName()
{
  return vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceMRMLAttributeName;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetActiveComponentType(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentType: No interaction context with identifier '" << context << "' was found");
    return ComponentNone;
    }

  return this->ActiveComponents[context].Type;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetActiveComponentIndex(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentIndex: No interaction context with identifier '" << context << "' was found");
    return -1;
    }

  return this->ActiveComponents[context].Index;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetActiveComponent(int componentType, int componentIndex, std::string context/*=GetDefaultContextName()*/)
{
  if ( this->ActiveComponents.find(context) != this->ActiveComponents.end()
    && this->ActiveComponents[context].Type == componentType
    && this->ActiveComponents[context].Index == componentIndex )
    {
    // no change
    return;
    }
  this->ActiveComponents[context].Index = componentIndex;
  this->ActiveComponents[context].Type = componentType;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayNode::HasActiveComponent()
{
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkMRMLMarkupsDisplayNode::GetActiveComponentInteractionContexts()
{
  std::vector<std::string> interactionContextVector;
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      interactionContextVector.push_back(it->first);
      }
    }
  return interactionContextVector;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetActiveControlPoint(int controlPointIndex)
{
  this->SetActiveComponent(ComponentControlPoint, controlPointIndex);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::UpdateActiveControlPointWorld(
  int controlPointIndex, vtkMRMLInteractionEventData* eventData,
  double orientationMatrixWorld[9], const char* viewNodeID,
  const char* associatedNodeID, int positionStatus)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !eventData)
    {
    return -1;
    }

  bool addNewControlPoint = false;
  if (controlPointIndex < 0 || controlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    // Determine new control point index. Now we assume that new point is added to the end,
    // but in the future we may place existing points.
    controlPointIndex = markupsNode->GetNumberOfControlPoints();
    addNewControlPoint = true;
    }

  // Update active component but not yet fire modified event because the control
  // point is not created/updated yet in the markups node.
  //TODO: Allow other interaction contexts to place markups
  bool activeComponentChanged = false;
  std::string interactionContext = eventData->GetInteractionContextName();
  if ( this->ActiveComponents[interactionContext].Index != controlPointIndex
    || this->ActiveComponents[interactionContext].Type != ComponentControlPoint )
    {
    this->ActiveComponents[interactionContext].Type = ComponentControlPoint;
    this->ActiveComponents[interactionContext].Index = controlPointIndex;
    activeComponentChanged = true;
    }

  // AddControlPoint will fire modified events anyway, so we temporarily disable events
  // to add a new point with a minimum number of events.
  bool wasDisabled = markupsNode->GetDisableModifiedEvent();
  markupsNode->DisableModifiedEventOn();
  if (positionStatus == vtkMRMLMarkupsNode::PositionPreview)
    {
    const char* layoutName = nullptr;
    if (this->GetScene())
      {
      vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->GetScene()->GetNodeByID(viewNodeID));
      if (viewNode)
        {
        layoutName = viewNode->GetLayoutName();
        }
      }
    markupsNode->SetAttribute("Markups.MovingInSliceView", layoutName ? layoutName : "");
    std::ostringstream controlPointIndexStr;
    controlPointIndexStr << controlPointIndex;
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", controlPointIndexStr.str().c_str());
    }
  else
    {
    markupsNode->SetAttribute("Markups.MovingInSliceView", "");
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", "");
    }
  markupsNode->SetDisableModifiedEvent(wasDisabled);

  double pointWorld[3] = { 0.0 };
  eventData->GetWorldPosition(pointWorld);

  if (addNewControlPoint)
    {
    // Add new control point
    vtkMRMLMarkupsNode::ControlPoint* controlPoint = new vtkMRMLMarkupsNode::ControlPoint;
    markupsNode->TransformPointFromWorld(pointWorld, controlPoint->Position);
    // TODO: transform orientation to world before copying
    std::copy_n(orientationMatrixWorld, 9, controlPoint->OrientationMatrix);
    if (associatedNodeID)
      {
      controlPoint->AssociatedNodeID = associatedNodeID;
      }
    controlPoint->PositionStatus = positionStatus;

    markupsNode->AddControlPoint(controlPoint);
  }
  else
    {
    // Update existing control point
    markupsNode->SetNthControlPointPositionOrientationWorldFromArray(controlPointIndex,
      pointWorld, orientationMatrixWorld, associatedNodeID, positionStatus);
    }

  if (activeComponentChanged)
    {
    this->Modified();
    }

  return controlPointIndex;
}


//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::GetActiveControlPoints(std::vector<int>& controlPointIndices)
{
  controlPointIndices.clear();
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type == ComponentControlPoint)
      {
      controlPointIndices.push_back(it->second.Index);
      }
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::GetActiveControlPoint(std::string context)
{
  if ( this->ActiveComponents.find(context) != this->ActiveComponents.end()
    && this->ActiveComponents[context].Type == ComponentControlPoint )
    {
    return this->ActiveComponents[context].Index;
    }
  else
    {
    return -1;
    }
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLMarkupsDisplayNode::GetMarkupsNode()
{
  return vtkMRMLMarkupsNode::SafeDownCast(this->GetDisplayableNode());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = this->GetTextPropertyAsString(this->TextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->TextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsDisplayNode::GetTextPropertyAsString()
{
  return this->GetTextPropertyAsString(this->GetTextProperty());
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsDisplayNode::GetTextPropertyAsString(vtkTextProperty* textProperty)
{
  if (!textProperty)
    {
    return "";
    }

  std::stringstream ss;
  ss << "font-family:" << textProperty->GetFontFamilyAsString() << ";";
  if (textProperty->GetFontFile())
    {
    ss << "font-file:" << textProperty->GetFontFile() << ";";
    }
  ss << "font-size:" << textProperty->GetFontSize() << "px;";
  ss << "font-style:" << (textProperty->GetItalic() ? "italic" : "normal") << ";";
  ss << "font-weight:" << (textProperty->GetBold() ? "bold" : "normal") << ";";
  ss << "color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetColor()[i] * 255)) << ",";
    }
  ss << textProperty->GetOpacity() << ");";
  ss << "background-color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetBackgroundColor()[i] * 255))  << ",";
    }
  ss << textProperty->GetBackgroundOpacity() << ");";
  ss << "border-width:" << textProperty->GetFrameWidth() << "px;";
  ss << "border-color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetFrameColor()[i] * 255)) << ",";
    }
  ss << (textProperty->GetFrame() ? "1.0" : "0.0") << ");";
  ss << "text-shadow:";
  ss << textProperty->GetShadowOffset()[0] << "px " << textProperty->GetShadowOffset()[0] << "px ";
  ss << "0px "; // blur radius
  ss << "rgba(0,0,0," << (textProperty->GetShadow() ? "1.0" : "0.0") << ");";
  return ss.str();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::UpdateTextPropertyFromString(std::string inputString, vtkTextProperty* textProperty)
{
  if (!textProperty)
    {
    return ;
    }

  std::vector<vtksys::String> textProperties = vtksys::SystemTools::SplitString(inputString, ';');
  for (vtksys::String textPropertyString : textProperties)
    {
    std::vector<vtksys::String> keyValue = vtksys::SystemTools::SplitString(textPropertyString, ':');
    if (keyValue.empty())
      {
      continue;
      }
    std::string key = keyValue[0];

    if (keyValue.size() < 2)
      {
      continue;
      }

    std::string value = keyValue[1];
    if (key == "font-family")
      {
      textProperty->SetFontFamilyAsString(value.c_str());
      }
    else if (key == "font-file")
      {
      textProperty->SetFontFile(value.c_str());
      }
    else if (key == "font-size")
      {
      size_t pos = value.find("px");
      std::stringstream ss;
      vtkVariant size = vtkVariant(value.substr(0, pos));
      textProperty->SetFontSize(size.ToInt());
      }
    else if (key == "font-style")
      {
      textProperty->SetItalic(value == "italic");
      }
    else if (key == "font-weight")
      {
      textProperty->SetBold(value == "bold");
      }
    else if (key == "color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0};
      vtkMRMLMarkupsDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetColor(colorF);
      textProperty->SetOpacity(colorF[3]);
      }
    else if (key == "background-color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0 };
      vtkMRMLMarkupsDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetBackgroundColor(colorF);
      textProperty->SetBackgroundOpacity(colorF[3]);
      }
    else if (key == "border-color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0 };
      vtkMRMLMarkupsDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetFrameColor(colorF);
      textProperty->SetFrame(colorF[3] > 0.0);
      }
    else if (key == "border-width")
      {
      size_t pos = value.find("px");
      std::stringstream ss;
      vtkVariant size = vtkVariant(value.substr(0, pos));
      textProperty->SetFrameWidth(size.ToInt());
      }
    else if (key == "text-shadow")
      {
      std::vector<vtksys::String> shadowProperties = vtksys::SystemTools::SplitString(textPropertyString, ' ');
      int shadowOffset[2] = { 0,0 };
      int shadowPropertyIndex = 0;
      for (vtksys::String shadowProperty : shadowProperties)
        {
        if (shadowPropertyIndex == SHADOW_H_OFFSET_INDEX || shadowPropertyIndex == SHADOW_V_OFFSET_INDEX)
          {
          size_t pos = shadowProperty.find("px");
          std::stringstream ss;
          vtkVariant offset = vtkVariant(shadowProperty.substr(0, pos));
          if (shadowPropertyIndex == SHADOW_H_OFFSET_INDEX)
            {
            shadowOffset[0] = offset.ToInt();
            }
          else
            {
            shadowOffset[1] = offset.ToInt();
            }
          }
        else if (shadowPropertyIndex == SHADOW_COLOR_INDEX)
          {
          // The shadow color in vtkTextProperty is actually just calculated from the text color (either black or white)
          // and cannot be changed.
          // We use the opacity channel to determine if the shadow should be displayed.
          double shadowColorF[4] = { 0.0, 0.0, 0.0, 0.0 };
          vtkMRMLMarkupsDisplayNode::GetColorFromString(shadowProperty, shadowColorF);
          textProperty->SetShadow(shadowColorF[3] > 0.0);
          }
        ++shadowPropertyIndex;
        }
        textProperty->SetShadowOffset(shadowOffset);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::GetColorFromString(const std::string& inputString, double colorF[4])
{
  std::string colorString = inputString;
  std::string prefixString = "rgba(";

  size_t pos = colorString.find(prefixString);
  if (pos != std::string::npos)
    {
    colorString = colorString.substr(pos + prefixString.size(), colorString.size());
    }

  pos = colorString.find(")");
  if (pos != std::string::npos)
    {
    colorString = colorString.substr(0, pos);
    }

  std::vector<vtksys::String> componentStrings = vtksys::SystemTools::SplitString(colorString, ',');
  for (int i = 0; i < 4 && i <static_cast<int>(componentStrings.size()); ++i)
    {
    double componentF = vtkVariant(componentStrings[i]).ToDouble();
    if (i != 3)
      {
      componentF /= 255.0;
      }
    colorF[i] = componentF;
    }
}
