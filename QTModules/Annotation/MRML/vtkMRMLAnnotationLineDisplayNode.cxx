#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationLineDisplayNode* vtkMRMLAnnotationLineDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationLineDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationLineDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationLineDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationLineDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationLineDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationLineDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationLineDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationLineDisplayNode::vtkMRMLAnnotationLineDisplayNode()
{
  this->LineThickness = 1.0;
  this->LabelPosition = 0.2;
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << " lineThickness =\"" << this->LineThickness << "\"";
  of << " labelPosition =\"" << this->LabelPosition << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

      if (!strcmp(attName, "lineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LineThickness;
      }
      else if (!strcmp(attName, "labelPosition"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->LabelPosition;
        }
    }
  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLAnnotationLineDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationLineDisplayNode *node = (vtkMRMLAnnotationLineDisplayNode *) anode;
  this->SetLineThickness(node->LineThickness);
  this->SetLabelPosition(node->LabelPosition);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Line Thickness : " << this->LineThickness << "\n";
  os << indent << "Label Position : " << this->LabelPosition << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::SetLineThickness(double thickness)
{
    if (this->LineThickness == thickness)
    {
        return;
    }
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting LineThickness to " << thickness);
    this->LineThickness = thickness;
   
    if (!this->GetDisableModifiedEvent())
      {
      // invoke a display modified event
      // KP this->InvokeEvent(vtkMRMLAnnotationLineDisplayNode::DisplayModifiedEvent);
      this->Modified();
      }
    this->ModifiedSinceReadOn();
}





