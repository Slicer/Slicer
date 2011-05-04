#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode*
vtkMRMLAnnotationTextDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode*
vtkMRMLAnnotationTextDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode::vtkMRMLAnnotationTextDisplayNode()
{
  this->TextScale = 4.5;
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  // of << " symbolScale=\"" << this->GlyphScale << "\"";
  of << " textScale=\"" << this->TextScale << "\"";
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ReadXMLAttributes(const char** atts)
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
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void
vtkMRMLAnnotationTextDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationTextDisplayNode *node =
      (vtkMRMLAnnotationTextDisplayNode *) anode;

  this->SetTextScale(node->TextScale);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Text scale: (";
  os << this->TextScale << ")\n";
}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ProcessMRMLEvents(vtkObject *caller,
    unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::SetTextScale(double scale)
{

  //vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting TextScale to " << scale);
  this->TextScale = scale;

  if (!this->GetDisableModifiedEvent())
    {
      // invoke a display modified event
      // this->InvokeEvent(vtkMRMLAnnotationTextDisplayNode::DisplayModifiedEvent);
    }
  //this->InvokeEvent(vtkCommand::ModifiedEvent);
  this->ModifiedSinceReadOn();
}

