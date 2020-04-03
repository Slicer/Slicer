
// MRMLAnnotation includes
#include "vtkMRMLAnnotationPointDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char *vtkMRMLAnnotationPointDisplayNode::GlyphTypesNames[GlyphMax+2] =
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
//  "Diamond3D"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationPointDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationPointDisplayNode::vtkMRMLAnnotationPointDisplayNode()
{
  this->GlyphType = vtkMRMLAnnotationPointDisplayNode::Sphere3D;
  this->GlyphScale = 5.0;
  this->SliceProjection = (vtkMRMLAnnotationDisplayNode::ProjectionOff |
                           vtkMRMLAnnotationPointDisplayNode::ProjectionUseFiducialColor |
                           vtkMRMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";
  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  of << " projectedColor=\"" << this->ProjectedColor[0] << " "
     << this->ProjectedColor[1] << " "
     << this->ProjectedColor[2] << "\"";

  of << " projectedOpacity=\"" << this->ProjectedOpacity << "\"";
 }

//----------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

      if (!strcmp(attName, "glyphType"))
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
      else if (!strcmp(attName, "projectedColor"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->ProjectedColor[0];
        ss >> this->ProjectedColor[1];
        ss >> this->ProjectedColor[2];
        }
      else if (!strcmp(attName, "projectedOpacity"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->ProjectedOpacity;
        }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLAnnotationPointDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationPointDisplayNode *node = (vtkMRMLAnnotationPointDisplayNode *) anode;
  this->SetGlyphType(node->GlyphType);
  this->SetGlyphScale(node->GlyphScale);
  this->SetSliceProjection(node->SliceProjection);
  this->SetProjectedColor(node->GetProjectedColor());
  this->SetProjectedOpacity(node->GetProjectedOpacity());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
const char* vtkMRMLAnnotationPointDisplayNode::GetGlyphTypeAsString()
{
  return this->GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
const char* vtkMRMLAnnotationPointDisplayNode::GetGlyphTypeAsString(int glyphType)
{
  if (glyphType < GlyphMin || (glyphType > GlyphMax))
    {
      return "UNKNOWN";
    }
    return this->GlyphTypesNames[glyphType];
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
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
void vtkMRMLAnnotationPointDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Glyph scale: (";
  os << this->GlyphScale << ")\n";
  os << indent << "Glyph type: ";
  os << this->GetGlyphTypeAsString() << " (" << this->GlyphType << ")\n";
  os << indent << "Slice projection: (";
  os << this->SliceProjection << ")\n";
  os << indent << "Projected Color: (";
  os << this->ProjectedColor[0] << ","
     << this->ProjectedColor[1] << ","
     << this->ProjectedColor[2] << ")" << "\n";
  os << indent << "Projected Opacity: " << this->ProjectedOpacity << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
int  vtkMRMLAnnotationPointDisplayNode::GlyphTypeIs3D(int glyphType)
{
  if (glyphType >= vtkMRMLAnnotationPointDisplayNode::Sphere3D)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void  vtkMRMLAnnotationPointDisplayNode::SetGlyphType(int type)
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
void vtkMRMLAnnotationPointDisplayNode::SetGlyphScale(double scale)
{
  if (this->GlyphScale == scale)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphScale to " << scale);
  this->GlyphScale = scale;
  this->Modified();
}

//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkMRMLAnnotationPointDisplayNode::CreateBackup()
{

  vtkMRMLAnnotationPointDisplayNode * backupNode = vtkMRMLAnnotationPointDisplayNode::New();

  int oldMode = backupNode->GetDisableModifiedEvent();
  backupNode->DisableModifiedEventOn();
  backupNode->Copy(this);
  backupNode->SetDisableModifiedEvent(oldMode);

  this->m_Backup = backupNode;

}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkMRMLAnnotationPointDisplayNode::RestoreBackup()
{

  if (this->m_Backup)
    {
    MRMLNodeModifyBlocker blocker(this);
    this->Copy(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup");
    }

}





