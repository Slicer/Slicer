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

// MRMLMarkups includes
#include "vtkMRMLMarkupsDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

const char *vtkMRMLMarkupsDisplayNode::GlyphTypesNames[GlyphMax+2] =
{
  "GlyphMin",
  "Vertex2D",
  "Dash2D",
  "Cross2D",
  "ThickCross2D",
  "Triangle2D",
  "Square2D",
  "Circle2D",
  "Diamond2D",
  "Arrow2D",
  "ThickArrow2D",
  "HookedArrow2D",
  "StarBurst2D",
  "Sphere3D"
};

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
  this->GlyphScale = 3;

  // projection settings
  this->SliceProjection = (vtkMRMLMarkupsDisplayNode::ProjectionOff |
                           vtkMRMLMarkupsDisplayNode::ProjectionUseFiducialColor |
                           vtkMRMLMarkupsDisplayNode::ProjectionOutlinedBehindSlicePlane);
  this->SliceProjectionColor[0] = 1.0;
  this->SliceProjectionColor[1] = 1.0;
  this->SliceProjectionColor[2] = 1.0;
  this->SliceProjectionOpacity = 0.6;

  this->TextVisibility = true;

  this->ActiveComponentType = ComponentNone;
  this->ActiveComponentIndex = -1;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::~vtkMRMLMarkupsDisplayNode()
= default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " textScale=\"" << this->TextScale << "\"";
  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";

  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  of << " sliceProjectionColor=\"" << this->SliceProjectionColor[0] << " "
     << this->SliceProjectionColor[1] << " "
     << this->SliceProjectionColor[2] << "\"";

  of << " sliceProjectionOpacity=\"" << this->SliceProjectionOpacity << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "textScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextScale;
      }
    else if (!strcmp(attName, "glyphType"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphType;
      }
    else if (!strcmp(attName, "glyphScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphScale;
      }
    else if (!strcmp(attName, "sliceProjection"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceProjection;
      }
    else if (!strcmp(attName, "sliceProjectionColor") ||
         !strcmp(attName, "projectedColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceProjectionColor[0];
      ss >> this->SliceProjectionColor[1];
      ss >> this->SliceProjectionColor[2];
      }
    else if (!strcmp(attName, "sliceProjectionOpacity") ||
         !strcmp(attName, "projectedOpacity"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceProjectionOpacity;
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

  this->SetTextScale(node->TextScale);
  this->SetGlyphType(node->GlyphType);
  this->SetGlyphScale(node->GlyphScale);
  this->SetSliceProjection(node->SliceProjection);
  this->SetSliceProjectionColor(node->GetSliceProjectionColor());
  this->SetSliceProjectionOpacity(node->GetSliceProjectionOpacity());

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString()
{
  return this->GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString(int glyphType)
{
  if (glyphType < GlyphMin || (glyphType > GlyphMax))
    {
    return "UNKNOWN";
    }
  return this->GlyphTypesNames[glyphType];
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
  if (!glyphString)
    {
    vtkErrorMacro("SetGlyphTypeFromString: Null glyph type string!");
    return;
    }
  for (int ID = GlyphMin; ID <= GlyphMax; ID++)
    {
      if (!strcmp(glyphString,GlyphTypesNames[ID]))
      {
      this->SetGlyphType(ID);
      return;
      }
    }
  vtkErrorMacro("Invalid glyph type string: " << glyphString);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Text scale: " << this->TextScale << "\n";
  os << indent << "Glyph scale: (";
  os << this->GlyphScale << ")\n";
  os << indent << "Glyph type: ";
  os << this->GetGlyphTypeAsString() << " (" << this->GlyphType << ")\n";
  os << indent << "Slice projection: ";
  os << this->SliceProjection << "\n";
  os << indent << "Slice projection Color: (";
  os << this->SliceProjectionColor[0] << ","
     << this->SliceProjectionColor[1] << ","
     << this->SliceProjectionColor[2] << ")" << "\n";
  os << indent << "Slice projection Opacity: " << this->SliceProjectionOpacity << "\n";

  os << indent << "Active component type: " << this->ActiveComponentType << "\n";
  os << indent << "Active component index: " << this->ActiveComponentIndex << "\n";
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
void  vtkMRMLMarkupsDisplayNode::SetGlyphType(int type)
{
  if (this->GlyphType == type)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphType to " << type);
  this->GlyphType = type;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetGlyphScale(double scale)
{
  if (this->GlyphScale == scale)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphScale to " << scale);
  this->GlyphScale = scale;
  this->Modified();
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
