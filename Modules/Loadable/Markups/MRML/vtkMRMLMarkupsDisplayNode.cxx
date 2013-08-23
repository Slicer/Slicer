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
    // model display node settings
  this->SetVisibility(1);
  this->SetVectorVisibility(0);
  this->SetScalarVisibility(0);
  this->SetTensorVisibility(0);

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
  this->TextScale = 3.4;
  this->GlyphType = vtkMRMLMarkupsDisplayNode::Sphere3D;
  this->GlyphScale = 2.1;

  // projection settings
  this->SliceProjection = (vtkMRMLMarkupsDisplayNode::ProjectionOff |
                           vtkMRMLMarkupsDisplayNode::ProjectionUseFiducialColor |
                           vtkMRMLMarkupsDisplayNode::ProjectionOutlinedBehindSlicePlane);
  this->SliceProjectionColor[0] = 1.0;
  this->SliceProjectionColor[1] = 1.0;
  this->SliceProjectionColor[2] = 1.0;
  this->SliceProjectionOpacity = 0.6;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::~vtkMRMLMarkupsDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " textScale=\"" << this->TextScale << "\"";
  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";

  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  if (this->SliceProjectionColor)
    {
    of << " sliceProjectionColor=\"" << this->SliceProjectionColor[0] << " "
       << this->SliceProjectionColor[1] << " "
       << this->SliceProjectionColor[2] << "\"";
    }
  of << " sliceProjectionOpacity=\"" << this->SliceProjectionOpacity << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
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

  vtkMRMLMarkupsDisplayNode *node = (vtkMRMLMarkupsDisplayNode *)anode;

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
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
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
