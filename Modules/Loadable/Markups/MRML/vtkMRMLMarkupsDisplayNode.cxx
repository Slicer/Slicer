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
#include "vtkMRMLMarkupsDisplayNode.h"
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>

// STL includes
#include <sstream>

const char* vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceRole = "lineColor";
const char* vtkMRMLMarkupsDisplayNode::LineColorNodeReferenceMRMLAttributeName = "lineColorNodeRef";

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

  this->ActiveComponentType = ComponentNone;
  this->ActiveComponentIndex = -1;

  this->LineThickness = 0.2;

  // Line color variables
  this->LineColorFadingStart = 1.;
  this->LineColorFadingEnd = 10.;
  this->LineColorFadingSaturation = 1.;
  this->LineColorFadingHueOffset = 0.;

  // Line color node
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  this->AddNodeReferenceRole(this->GetLineColorNodeReferenceRole(),
                             this->GetLineColorNodeReferenceMRMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::~vtkMRMLMarkupsDisplayNode()
= default;

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
  vtkMRMLWriteXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkMRMLWriteXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkMRMLWriteXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLWriteXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkMRMLWriteXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkMRMLWriteXMLFloatMacro(lineThickness, LineThickness);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkMRMLWriteXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(propertiesLabelVisibility, PropertiesLabelVisibility);
  vtkMRMLReadXMLBooleanMacro(pointLabelsVisibility, PointLabelsVisibility);
  vtkMRMLReadXMLFloatMacro(textScale, TextScale);
  vtkMRMLReadXMLFloatMacro(glyphScale, GlyphScale);
  vtkMRMLReadXMLFloatMacro(glyphSize, GlyphSize);
  vtkMRMLReadXMLBooleanMacro(useGlyphScale, UseGlyphScale);
  vtkMRMLReadXMLEnumMacro(glyphType, GlyphType);
  vtkMRMLReadXMLBooleanMacro(sliceProjection, SliceProjection);
  vtkMRMLReadXMLBooleanMacro(sliceProjectionUseFiducialColor, SliceProjectionUseFiducialColor);
  vtkMRMLReadXMLBooleanMacro(sliceProjectionOutlinedBehindSlicePlane, SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLReadXMLVectorMacro(sliceProjectionColor, SliceProjectionColor, double, 3);
  vtkMRMLReadXMLFloatMacro(sliceProjectionOpacity, SliceProjectionOpacity);
  vtkMRMLReadXMLFloatMacro(lineThickness, LineThickness);
  vtkMRMLReadXMLFloatMacro(lineColorFadingStart, LineColorFadingStart);
  vtkMRMLReadXMLFloatMacro(lineColorFadingEnd, LineColorFadingEnd);
  vtkMRMLReadXMLFloatMacro(lineColorFadingSaturation, LineColorFadingSaturation);
  vtkMRMLReadXMLFloatMacro(lineColorFadingHueOffset, LineColorFadingHueOffset);
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

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLMarkupsDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLMarkupsDisplayNode *node = vtkMRMLMarkupsDisplayNode::SafeDownCast(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(PropertiesLabelVisibility);
  vtkMRMLCopyBooleanMacro(PointLabelsVisibility);
  vtkMRMLCopyFloatMacro(TextScale);
  vtkMRMLCopyFloatMacro(GlyphScale);
  vtkMRMLCopyFloatMacro(GlyphSize);
  vtkMRMLCopyBooleanMacro(UseGlyphScale);
  vtkMRMLCopyEnumMacro(GlyphType);
  vtkMRMLCopyBooleanMacro(SliceProjection);
  vtkMRMLCopyBooleanMacro(SliceProjectionUseFiducialColor);
  vtkMRMLCopyBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLCopyVectorMacro(SliceProjectionColor, double, 3);
  vtkMRMLCopyFloatMacro(SliceProjectionOpacity);
  vtkMRMLCopyFloatMacro(LineThickness);
  vtkMRMLCopyFloatMacro(LineColorFadingStart);
  vtkMRMLCopyFloatMacro(LineColorFadingEnd);
  vtkMRMLCopyFloatMacro(LineColorFadingSaturation);
  vtkMRMLCopyFloatMacro(LineColorFadingHueOffset);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
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
  case Diamond3D: return "Diamond3D";
  case GlyphTypeInvalid:
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
  vtkMRMLPrintBooleanMacro(SliceProjection);
  vtkMRMLPrintBooleanMacro(SliceProjectionUseFiducialColor);
  vtkMRMLPrintBooleanMacro(SliceProjectionOutlinedBehindSlicePlane);
  vtkMRMLPrintVectorMacro(SliceProjectionColor, double, 3);
  vtkMRMLPrintFloatMacro(SliceProjectionOpacity);
  vtkMRMLPrintFloatMacro(ActiveComponentType);
  vtkMRMLPrintFloatMacro(ActiveComponentIndex);
  vtkMRMLPrintFloatMacro(LineThickness);
  vtkMRMLPrintFloatMacro(LineColorFadingStart);
  vtkMRMLPrintFloatMacro(LineColorFadingEnd);
  vtkMRMLPrintFloatMacro(LineColorFadingSaturation);
  vtkMRMLPrintFloatMacro(LineColorFadingHueOffset);
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                  unsigned long event,
                                                  void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
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
  if (glyphType >= vtkMRMLMarkupsDisplayNode::Sphere3D)
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
void vtkMRMLMarkupsDisplayNode::SetActiveComponent(int componentType, int componentIndex)
{
  if (this->ActiveComponentIndex == componentIndex
    && this->ActiveComponentType == componentType)
    {
    // no change
    return;
    }
  this->ActiveComponentIndex = componentIndex;
  this->ActiveComponentType = componentType;
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetActiveControlPoint(int controlPointIndex)
{
  this->SetActiveComponent(ComponentControlPoint, controlPointIndex);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayNode::UpdateActiveControlPointWorld(
  int controlPointIndex, double pointWorld[3],
  double orientationMatrixWorld[9], const char* viewNodeID,
  const char* associatedNodeID, int positionStatus)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return -1;
    }

  bool addNewControlPoint = false;
  if (controlPointIndex < 0 || controlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    // Determine new control point index. Now we assuem that new point is added to the end,
    // but in the future we may place existing points.
    controlPointIndex = markupsNode->GetNumberOfControlPoints();
    addNewControlPoint = true;
    }

  // Update active component but not yet fire modified event
  // because the control point is not created/updated yet
  // in the markups node.
  bool activeComponentChanged = false;
  if (this->ActiveComponentIndex != controlPointIndex
    || this->ActiveComponentType != ComponentControlPoint)
    {
    this->ActiveComponentType = ComponentControlPoint;
    this->ActiveComponentIndex = controlPointIndex;
    activeComponentChanged = true;
    }

  // AddControlPoint will fire modified events anyway, so we temporarily disable events
  // to add a new point with a minimum number of events.
  bool wasDisabled = markupsNode->GetDisableModifiedEvent();
  markupsNode->DisableModifiedEventOn();
  if (positionStatus == vtkMRMLMarkupsNode::PositionPreview)
    {
    markupsNode->SetAttribute("Markups.MovingInSliceView", viewNodeID);
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
int vtkMRMLMarkupsDisplayNode::GetActiveControlPoint()
{
  if (this->ActiveComponentType == ComponentControlPoint)
    {
    return this->ActiveComponentIndex;
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
