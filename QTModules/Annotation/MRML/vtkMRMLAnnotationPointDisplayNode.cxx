#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLScene.h"

const char *vtkMRMLAnnotationPointDisplayNode::GlyphTypesNames[GlyphMax+1] = 
{ 
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
  "Sphere3D",
  "Diamond3D"
};



//------------------------------------------------------------------------------
vtkMRMLAnnotationPointDisplayNode* vtkMRMLAnnotationPointDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationPointDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationPointDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationPointDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationPointDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationPointDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationPointDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationPointDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationPointDisplayNode::vtkMRMLAnnotationPointDisplayNode()
{
  this->GlyphType = this->StarBurst2D;
  this->GlyphScale = 10.0;
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";
 }



//----------------------------------------------------------------------------
void vtkMRMLAnnotationPointDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
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
      this->ModifiedSinceReadOn();
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
  
  if (!this->GetDisableModifiedEvent())
    {
    // invoke a display modified event
    // this->InvokeEvent(vtkMRMLAnnotationPointDisplayNode::DisplayModifiedEvent);
    }
  this->ModifiedSinceReadOn();
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
   
    if (!this->GetDisableModifiedEvent())
      {
      // invoke a display modified event
      // KP this->InvokeEvent(vtkMRMLAnnotationPointDisplayNode::DisplayModifiedEvent);
      }
    this->ModifiedSinceReadOn();
}





