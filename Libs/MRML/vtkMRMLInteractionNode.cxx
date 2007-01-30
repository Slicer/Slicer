#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLInteractionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLInteractionNode");
  if(ret)
    {
    return (vtkMRMLInteractionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLInteractionNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLInteractionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLInteractionNode");
  if(ret)
    {
    return (vtkMRMLInteractionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLInteractionNode;
}


//----------------------------------------------------------------------------
vtkMRMLInteractionNode::vtkMRMLInteractionNode()
{
  this->SingletonTag = "vtkMRMLInteractionNode";

  this->CurrentMouseMode = vtkMRMLInteractionNode::MouseTransform;
  this->LastMouseMode = vtkMRMLInteractionNode::MouseTransform;
 }

//----------------------------------------------------------------------------
vtkMRMLInteractionNode::~vtkMRMLInteractionNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetCurrentMouseMode ( int mode )
{
  switch (mode)
    {
    case vtkMRMLInteractionNode::MouseSelect:
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MouseSelectRegion:
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MousePut:
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MousePan:      
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MouseZoom:
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MouseRotate:      
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::MouseTransform:      
      this->CurrentMouseMode = mode;
      this->InvokeEvent(this->MouseModeChangedEvent, NULL);
      break;
    default:
      break;
    }
}



//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  // configure stereo
  if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MouseSelect )
    {
    of << indent << " currentMouseMode=\"" << "MouseSelect" << "\"";    
    }
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MouseSelectRegion )
    {
    of << indent << " currentMouseMode=\"" << "MouseSelectRegion" << "\"";    
    }
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MousePut )
    {
    of << indent << " currentMouseMode=\"" << "MousePut" << "\"";    
    }
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MousePan )
    {
    of << indent << " currentMouseMode=\"" << "MousePan" << "\"";    
    }
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MouseZoom )
    {
    of << indent << " currentMouseMode=\"" << "MouseZoom" << "\"";    
    }
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MouseRotate )
    {
    of << indent << " currentMouseMode=\"" << "MouseRotate" << "\"";    
    }  
  else if ( this->GetCurrentMouseMode() == vtkMRMLInteractionNode::MouseTransform )
    {
    of << indent << " currentMouseMode=\"" << "MouseTransform" << "\"";    
    }  

  if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MouseSelect )
    {
    of << indent << " lastMouseMode=\"" << "MouseSelect" << "\"";    
    }
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MouseSelectRegion )
    {
    of << indent << " lastMouseMode=\"" << "MouseSelectRegion" << "\"";    
    }
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MousePut )
    {
    of << indent << " lastMouseMode=\"" << "MousePut" << "\"";    
    }
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MousePan )
    {
    of << indent << " lastMouseMode=\"" << "MousePan" << "\"";    
    }
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MouseZoom )
    {
    of << indent << " lastMouseMode=\"" << "MouseZoom" << "\"";    
    }
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MouseRotate )
    {
    of << indent << " lastMouseMode=\"" << "MouseRotate" << "\"";    
    }  
  else if ( this->GetLastMouseMode() == vtkMRMLInteractionNode::MouseTransform )
    {
    of << indent << " lastMouseMode=\"" << "MouseTransform" << "\"";    
    }  

}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);


    if (!strcmp(attName, "currentMouseMode")) 
      {
      if (!strcmp(attValue,"MouseSelect")) 
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MouseSelect;
        }
      else if ( !strcmp (attValue, "MouseSelectRegion" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MouseSelectRegion;
        }
      else if ( !strcmp (attValue, "MousePut" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MousePut;
        }
      else if ( !strcmp (attValue, "MousePan" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MousePan;
        }
      else if ( !strcmp (attValue, "MouseZoom" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MouseZoom;
        }
      else if ( !strcmp (attValue, "MouseRotate" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MouseRotate;
        }
      else if ( !strcmp (attValue, "MouseTransform" ))
        {
        this->CurrentMouseMode = vtkMRMLInteractionNode::MouseTransform;
        }
      }
    else if (!strcmp(attName, "lastMouseMode"))
      {
      if (!strcmp(attValue,"MouseSelect")) 
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MouseSelect;
        }
      else if ( !strcmp (attValue, "MouseSelectRegion" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MouseSelectRegion;
        }
      else if ( !strcmp (attValue, "MousePut" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MousePut;
        }
      else if ( !strcmp (attValue, "MousePan" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MousePan;
        }
      else if ( !strcmp (attValue, "MouseZoom" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MouseZoom;
        }
      else if ( !strcmp (attValue, "MouseRotate" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MouseRotate;
        }
      else if ( !strcmp (attValue, "MouseTransform" ))
        {
        this->LastMouseMode = vtkMRMLInteractionNode::MouseTransform;
        }
      }
    
    }
}




//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLInteractionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLInteractionNode *node = (vtkMRMLInteractionNode *) anode;

  this->SetCurrentMouseMode (node->GetCurrentMouseMode());
  this->SetLastMouseMode ( node->GetLastMouseMode());
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  os << indent << "CurrentMouseMode:        " << this->CurrentMouseMode << "\n";
  os << indent << "LastMouseMode:        " << this->LastMouseMode << "\n";
}

